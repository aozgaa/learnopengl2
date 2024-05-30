#version 330 core
out vec4 FragColor;

in float intensity;

uniform vec3 object_color;
uniform vec3 light_color;

void main() {
    vec3 res = intensity * light_color * object_color;
    FragColor = vec4(res, 1.0);
}