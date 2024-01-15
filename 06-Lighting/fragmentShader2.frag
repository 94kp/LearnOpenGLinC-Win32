#version 450 core

in vec4 a_color_out;
uniform vec3 u_objectColor;
uniform vec3 u_lightColor;
out vec4 FragColor;

void main(void)
{
    FragColor = vec4(1.0);    
}
