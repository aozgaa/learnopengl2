#version 330 core
out vec4 FragColor;

in vec3 v_normal;
in vec3 v_pos;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform vec3 v_light_pos;
uniform vec3 light_color;

uniform Material material;

void main() {
    vec3 ambient = material.ambient * light_color;

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(v_light_pos - v_pos);
    float cos_theta = max(0.0, dot(norm, light_dir));
    vec3 diffuse = material.diffuse * cos_theta * light_color;

    vec3 camera_dir = normalize(-v_pos); // fixme: negate?
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), material.shininess);
    vec3 specular = material.specular * spec * light_color;

    vec3 res = (ambient + diffuse + specular);
    FragColor = vec4(res, 1.0);
}