#version 330 core
layout(location = 0) in vec3 l_pos;
layout(location = 1) in vec3 l_normal;

out float intensity;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 v_light_pos;

void main() {
    vec3 v_pos = (view * model * vec4(l_pos, 1.0)).xyz;
    mat3 model_normal = transpose(inverse(mat3(view * model))); // slow, for learning only!
    vec3 v_normal = model_normal * l_normal;
    gl_Position = projection * view * model * vec4(l_pos, 1.0);

    float ambient_strength = 0.1;
    float ambient = ambient_strength;

    float diffuse_strength = 0.5;
    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(v_light_pos - v_pos);
    float cos_theta = max(0.0, dot(norm, light_dir));
    float diffuse = diffuse_strength * cos_theta;

    float specular_strength = 0.5;
    vec3 camera_dir = normalize(-v_pos);
    vec3 bounce_dir = reflect(-light_dir, norm);
    float spec = pow(max(0.0, dot(camera_dir, bounce_dir)), 32);
    float specular = specular_strength * spec;

    // explanation for brightness along triangle edges:
    // * the vertex shader is evaluated only at the vertices,
    // if a vertex is bright due to some non-linear effect
    // (eg: the "ring" in phong lighting),
    // they will get linearly interpolated to the vertices,
    // yielding bright stripes
    intensity = ambient + diffuse + specular;
}