#version 330 core
layout(location = 0) in vec3 l_pos;
layout(location = 1) in vec3 l_normal;

out vec3 v_pos;
out vec3 v_normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    v_pos = (view * model * vec4(l_pos, 1.0)).xyz;
    mat3 model_normal = transpose(inverse(mat3(view * model))); // slow, for learning only!
    v_normal = model_normal * l_normal;
    gl_Position = projection * view * model * vec4(l_pos, 1.0);
}