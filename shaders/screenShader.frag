// 独立实现
#version 450 core

layout(location = 0) out vec3 pixelColor;

uniform int frameCnt;
uniform sampler2D colorBuffer;

vec3 gamma_correction(vec3 color) {
    return vec3(pow(color.x, 0.6), pow(color.y, 0.6), pow(color.z, 0.6));
}

void main() {
    pixelColor = gamma_correction(texelFetch(colorBuffer, ivec2(gl_FragCoord.xy), 0).xyz);
}