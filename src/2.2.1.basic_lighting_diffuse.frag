#version 330 core
out vec4 FragColor;

in vec3 ws_normal;
in vec3 ws_pos;

uniform vec3 ws_light_pos;
uniform vec3 object_color;
uniform vec3 light_color;

void main() {
    float ambient_intensity = 0.1;
    vec3 ambient = ambient_intensity * light_color;

    float diffuse_intensity = 1.0;
    vec3 norm = normalize(ws_normal);
    vec3 light_dir = normalize(ws_light_pos - ws_pos);
    float cos_theta = max(0.0, dot(norm, light_dir));
    vec3 diffuse = diffuse_intensity * cos_theta * light_color;

    vec3 res = (ambient + diffuse) * object_color;
    FragColor = vec4(res, 1.0);
}