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
    vec3 v_pos;
    vec3 direction;
    float spotlight_cos_inner;
    float spotlight_cos_outer;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main() {
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coord));

    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(light.v_pos - v_pos); // towards light source
    float cos_theta_surface = max(0.0, dot(norm, light_dir));
    vec3 diffuse = cos_theta_surface * light.diffuse * vec3(texture(material.diffuse, tex_coord));

    vec3 camera_dir = normalize(-v_pos);
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), material.shininess);
    vec3 specular = spec * light.specular * vec3(texture(material.specular, tex_coord));

    vec3 res = ambient + diffuse + specular;

    float d_2 = dot(v_pos - light.v_pos, v_pos - light.v_pos); // squared distance
    float d = sqrt(d_2);
    float k_0 = 1.0;
    float k_1 = 0.009;
    float k_2 = 0.0032;
    float f_att = 1.0/(k_0 + k_1 * d + k_2 * d_2);
    res *= f_att;

    float cos_theta_spotlight = dot(light_dir, -normalize(light.direction)); // away from spotlight center
    res *= smoothstep(light.spotlight_cos_outer, light.spotlight_cos_inner, cos_theta_spotlight);

    FragColor = vec4(res, 1.0);
}