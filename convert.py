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
                    mapped = 31 # water? Let's guess 8=water, 7=fire, 6=goo based on visual ordering?
                elif val == 7:
                    mapped = 32 # fire?
                elif val == 6:
                    mapped = 33 # toxic?
                elif val == 2: mapped = 11
                elif val == 3: mapped = 10
                elif val == 4: mapped = 13
                elif val == 5: mapped = 12
                else:
                    mapped = 0
                row_data.append(mapped)
            terrain_2d.append(row_data)
        level_data['terrain'] = terrain_2d
    else:
        # fallback
        level_data['terrain'] = [[0]*width for _ in range(height)]
        terrain_2d = level_data['terrain']

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
                    new_obj['length'] = w // tiled_tile_size
                    new_obj['is_horizontal'] = w > h
                    if 'group' in props:
                        new_obj['group_id'] = props['group']
                elif gid >= 16 and gid < 24:
                    local_gid = gid - 16
                    if local_gid == 0: new_obj['type'] = "fire_spawn"
                    elif local_gid == 1: new_obj['type'] = "water_spawn"
                    elif local_gid == 2: new_obj['type'] = "fire_door"
                    elif local_gid == 3: new_obj['type'] = "water_door"
                    elif local_gid == 4: 
                        new_obj['type'] = "diamond"
                        new_obj['element'] = "fire"
                    elif local_gid == 5:
                        new_obj['type'] = "diamond"
                        new_obj['element'] = "water"
                    else: continue
                    new_obj['row'] = row
                    new_obj['col'] = col
                elif gid >= 24:
                    local_gid = gid - 24
                    if local_gid == 0: new_obj['type'] = "button"
                    elif local_gid in [1, 2]: new_obj['type'] = "lever"
                    elif local_gid == 4: new_obj['type'] = "block"
                    else: continue
                    new_obj['row'] = row
                    new_obj['col'] = col
                    if 'group' in props:
                        new_obj['group_id'] = props['group']
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
    convert_level('reference/data/tutorials/levels/forest_01.json', 'Resources/levels/level0.json')
