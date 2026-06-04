import json
import sys

def convert_level(input_path, output_path):
    with open(input_path, 'r', encoding='utf-8') as f:
        tiled_data = json.load(f)

    # basic info
    width = tiled_data.get('width', 39)
    height = tiled_data.get('height', 29)
    tiled_tile_size = tiled_data.get('tilewidth', 32)
    output_tile_size = 32  # Changed back to 32 to perfectly fit 1248x928 window

    level_data = {
        "tileSize": output_tile_size,
        "width": width,
        "height": height,
        "terrain": [],
        "objects": []
    }

    # terrain
    raw_terrain = None
    for layer in tiled_data.get('layers', []):
        if layer.get('type') == 'tilelayer' and layer.get('name') == 'Ground':
            raw_terrain = layer.get('data', [])
            break

    if raw_terrain:
        # map 1D to 2D
        # 1 -> 1, 0 -> 0. Tiled data might have other things, let's map them.
        # Wait, the Ground layer has:
        # 1: wall
        # 2: ? 3: ? 4: ? 6: ? 7: ? 8: ?
        # In level1.json:
        # 10: ramp
        # 11: ramp
        # 12: ramp
        # 13: ramp
        # 31: water, 32: fire, 33: goo
        # Let's map Tiled terrain:
        # 2 -> 10, 3 -> 11, 4 -> 12, 8 -> 31(water), 7 -> 32(fire), 6 -> 33(goo)?
        # Wait, let's check Ground.json for Tiled properties!
        terrain_2d = []
        for r in range(height):
            row_data = []
            for c in range(width):
                val = raw_terrain[r * width + c]
                # Default mapping
                if val == 1:
                    mapped = 1
                elif val == 8:
                    mapped = 33 # toxic
                elif val == 7:
                    mapped = 32 # fire
                elif val == 6:
                    mapped = 31 # water
                elif val == 2: mapped = 11
                elif val == 3: mapped = 10
                elif val == 4: mapped = 13
                elif val == 5: mapped = 12
                elif val == 12: mapped = 21 # SnowSlopeBL
                elif val == 13: mapped = 20 # SnowSlopeBR
                elif val == 15: mapped = 30 # SnowBlock
                elif val == 14: mapped = 22 # Ice
                else:
                    mapped = 0
                row_data.append(mapped)
            terrain_2d.append(row_data)
        level_data['terrain'] = terrain_2d
    else:
        # fallback
        level_data['terrain'] = [[0]*width for _ in range(height)]
        terrain_2d = level_data['terrain']

    def get_tileset_and_local_gid(gid):
        matching_ts = None
        for ts in sorted(tiled_data.get('tilesets', []), key=lambda x: x.get('firstgid', 0)):
            if ts.get('firstgid', 0) <= gid:
                matching_ts = ts
            else:
                break
        if matching_ts:
            return matching_ts.get('source', '').lower(), gid - matching_ts.get('firstgid', 0)
        return "", gid

    # objects
    # GID mappings
    # Chars (firstgid=16): 0:fire_spawn, 1:water_spawn, 2:fire_door, 3:water_door, 4:fire_diamond, 5:water_diamond
    # Objects (firstgid=24): 0:button, 1:lever, 2:lever, 4:block
    for layer in tiled_data.get('layers', []):
        if layer.get('type') == 'objectgroup':
            for obj in layer.get('objects', []):
                # if it's text, skip
                if 'text' in obj:
                    continue

                new_obj = {}
                obj_type = obj.get('type')
                gid = obj.get('gid', 0)
                props = {p['name']: p['value'] for p in obj.get('properties', [])} if isinstance(obj.get('properties'), list) else obj.get('properties', {})

                x = obj.get('x', 0)
                y = obj.get('y', 0)
                w = obj.get('width', 32)
                h = obj.get('height', 32)

                col = x // tiled_tile_size
                # tiled uses bottom-left for gid objects, top-left for rect objects
                if gid > 0:
                    row = (y - h) // tiled_tile_size
                else:
                    row = y // tiled_tile_size

                if obj_type == "platform":
                    new_obj['type'] = "elevator"
                    new_obj['row'] = row
                    new_obj['col'] = col
                    new_obj['target_row'] = row - props.get('dy', 0)
                    new_obj['target_col'] = col + props.get('dx', 0)
                    new_obj['is_horizontal'] = w > h
                    new_obj['length'] = (w if new_obj['is_horizontal'] else h) // tiled_tile_size
                    if 'group' in props:
                        new_obj['group_id'] = props['group']
                elif gid > 0:
                    ts_source, local_gid = get_tileset_and_local_gid(gid)
                    if 'chars' in ts_source:
                        if local_gid == 0:
                            new_obj['type'] = "fire_spawn"
                            col += 1
                        elif local_gid == 1:
                            new_obj['type'] = "water_spawn"
                            col += 1
                        elif local_gid == 2:
                            new_obj['type'] = "fire_door"
                            col += 1
                        elif local_gid == 3:
                            new_obj['type'] = "water_door"
                            col += 1
                        elif local_gid == 4: 
                            new_obj['type'] = "diamond"
                            new_obj['element'] = "fire"
                            row += 1
                            col += 1
                        elif local_gid == 5:
                            new_obj['type'] = "diamond"
                            new_obj['element'] = "water"
                            row += 1
                            col += 1
                        else: continue
                        new_obj['row'] = row
                        new_obj['col'] = col
                    elif 'objects' in ts_source and 'large' not in ts_source:
                        if local_gid == 0:
                            new_obj['type'] = "button"
                            col += 1
                        elif local_gid in [1, 2]:
                            new_obj['type'] = "lever"
                            col += 1
                        elif local_gid == 14:
                            new_obj['type'] = "timed_button"
                            col += 1
                            new_obj['time'] = float(props.get('time', 2000.0))
                        elif local_gid == 4: new_obj['type'] = "block"
                        else: continue
                        new_obj['row'] = row
                        new_obj['col'] = col
                        if 'group' in props:
                            new_obj['group_id'] = props['group']
                    else:
                        continue
                else:
                    continue

                level_data['objects'].append(new_obj)

    level_data_copy = level_data.copy()
    level_data_copy['terrain'] = "TERRAIN_PLACEHOLDER"
    json_str = json.dumps(level_data_copy, indent=2)
    
    terrain_lines = []
    for row in terrain_2d:
        terrain_lines.append("    [" + ", ".join(str(x) for x in row) + "]")
    terrain_str = "[\n" + ",\n".join(terrain_lines) + "\n  ]"
    
    json_str = json_str.replace('"TERRAIN_PLACEHOLDER"', terrain_str)

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(json_str)

if __name__ == '__main__':
    if len(sys.argv) >= 3:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    else:
        input_file = 'reference/data/tutorials/levels/forest_01.json'
        output_file = 'Resources/levels/level0.json'
    
    print(f"Converting {input_file} -> {output_file}...")
    convert_level(input_file, output_file)
    print("Done!")
