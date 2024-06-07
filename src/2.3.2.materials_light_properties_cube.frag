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

struct Light {
    vec3 v_pos;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main() {
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(light.v_pos - v_pos);
    float cos_theta = max(0.0, dot(norm, light_dir));
    vec3 diffuse = light.diffuse * material.diffuse * cos_theta;

    vec3 camera_dir = normalize(-v_pos);
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), material.shininess);
    vec3 specular = light.specular * material.specular * spec;

    vec3 res = (ambient + diffuse + specular);
    FragColor = vec4(res, 1.0);
}