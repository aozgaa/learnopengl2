#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord); // * 3.0); //  * 3.0 * vec4(ourColor, 1.0);
}