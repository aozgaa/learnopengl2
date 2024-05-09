#version 330 core
out vec4 FragColor;
  
in vec3 attrColor; // the input variable from the vertex shader (same name and same type)  

void main()
{
    FragColor = vec4(attrColor, 1.0);
}