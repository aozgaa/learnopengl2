#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
    vec4 c1 = vec4(0.0, 1.0, 0.0, 1.0) * texture(texture1, TexCoord);
    vec4 c2 = vec4(1.0, 0.0, 0.0, 1.0) * (1.0 - texture(texture2, TexCoord));
    FragColor = TexCoord.x > 0.5 ? c1 : c2;
}
