#version 330 core
out vec4 FragColor;

in vec3 v_normal;
in vec3 v_pos;
in vec2 tex_coord;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
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
uniform float time;

void main() {
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coord));

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(light.v_pos - v_pos);
    float cos_theta = max(0.0, dot(norm, light_dir));
    vec3 diffuse = cos_theta * light.diffuse * texture(material.diffuse, tex_coord).rgb;

    vec3 camera_dir = normalize(-v_pos);
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), material.shininess);
    vec3 specular = spec * light.specular * texture(material.specular, tex_coord).rgb;

    vec2 e_coord = tex_coord;
    e_coord.y = fract(e_coord.y + time);
    float d_to_edge = 1.0;
    d_to_edge = min(d_to_edge, tex_coord.x);
    d_to_edge = min(d_to_edge, 1.0 - tex_coord.x);
    d_to_edge = min(d_to_edge, tex_coord.y);
    d_to_edge = min(d_to_edge, 1.0 - tex_coord.y);

    vec3 emission = vec3(0.0);
    emission = d_to_edge < 0.085 ? vec3(0.0) : texture(material.emission, e_coord).rgb;

    vec3 res = ambient + diffuse + specular + emission;
    FragColor = vec4(res, 1.0);
}