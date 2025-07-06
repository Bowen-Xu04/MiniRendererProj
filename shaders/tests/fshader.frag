#version 450 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {

    //FragColor = vec4(0.1, 0.2, 0.3, 1.0);
    FragColor = texture(texture1, TexCoord);
    //ivec2 texSize = textureSize(texture1, 0);
    //FragColor = texelFetch(texture1, ivec2(TexCoord.x * texSize.x, TexCoord.y * texSize.y), 0);
    //FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2) * vec4(ourColor, 1.0);
}