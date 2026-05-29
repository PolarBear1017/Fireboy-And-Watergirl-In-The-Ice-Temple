#version 410 core

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 localPos;

layout(location = 0) out vec4 fragColor;

uniform sampler2D surface;
uniform vec4 colorTint = vec4(1.0, 1.0, 1.0, 1.0);

uniform bool useClockMask = false;
uniform float u_TimeRatio = 1.0;

void main() {
    if (useClockMask) {
        vec2 pixelPos = localPos * vec2(118.0, 148.0);
        vec2 dialCenter = vec2(0.0, -27.0);
        if (length(pixelPos - dialCenter) <= 22.0) {
            float angle = atan(pixelPos.x - dialCenter.x, pixelPos.y - dialCenter.y);
            if (angle < 0.0) {
                angle += 2.0 * 3.14159265;
            }
            if (angle < 2.0 * 3.14159265 * (1.0 - u_TimeRatio)) {
                discard;
            }
        }
    }

    vec4 texColor = texture(surface, uv);
    if (texColor.a < 0.01)
        discard;
    fragColor = texColor * colorTint;
}
