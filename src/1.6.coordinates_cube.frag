#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D wallSampler;
uniform sampler2D smileySampler;

void main() {
    vec4 c1 = vec4(0.0, 1.0, 0.0, 1.0) * texture(wallSampler, TexCoord);
    vec4 c2 = vec4(1.0, 0.0, 0.0, 1.0) * (1.0 - texture(smileySampler, TexCoord));
    FragColor = TexCoord.x > 0.5 ? c1 : c2;
}