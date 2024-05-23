#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture samplers
uniform sampler2D wallSampler;
uniform sampler2D smileySampler;

void main() {
    vec4 c1 = texture(wallSampler, TexCoord);
    vec4 c2 = texture(smileySampler, TexCoord);
    FragColor = mix(c1, c2, 0.3);
}