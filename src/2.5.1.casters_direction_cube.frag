#version 330 core
out vec4 FragColor;

in vec3 v_normal;
in vec3 v_pos;
in vec2 tex_coord;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct Light {
    vec3 direction; // from light towards object
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main() {
    vec3 light_dir = normalize(-light.direction); // from object towards source

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coord));

    vec3 norm = normalize(v_normal);
    float cos_theta = max(0.0, dot(norm, light_dir));
    vec3 diffuse = cos_theta * light.diffuse * vec3(texture(material.diffuse, tex_coord));

    vec3 camera_dir = normalize(-v_pos);
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), material.shininess);
    vec3 specular = spec * light.specular * vec3(texture(material.specular, tex_coord));

    vec3 res = (ambient + diffuse + specular);
    FragColor = vec4(res, 1.0);
}