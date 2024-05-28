#version 330 core
out vec4 FragColor;

in vec3 ws_normal;
in vec3 ws_pos;

uniform vec3 ws_camera_pos;
uniform vec3 ws_light_pos;
uniform vec3 object_color;
uniform vec3 light_color;

void main() {
    float ambient_intensity = 0.1;
    vec3 ambient = ambient_intensity * light_color;

    float diffuse_intensity = 0.5;
    vec3 norm = normalize(ws_normal);
    vec3 light_dir = normalize(ws_light_pos - ws_pos);
    float cos_theta = max(0.0, dot(norm, light_dir));
    vec3 diffuse = diffuse_intensity * cos_theta * light_color;

    float specular_intensity = 0.5;
    vec3 camera_dir = normalize(ws_camera_pos - ws_pos);
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), 32);
    vec3 specular = specular_intensity * spec * light_color;

    vec3 res = (ambient + diffuse + specular) * object_color;
    FragColor = vec4(res, 1.0);
}