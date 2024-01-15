#version 450 core

in vec4 a_position;
uniform mat4 u_mvpMatrix;
in vec4 a_color;
out vec4 a_color_out;

void main(void)
{
    gl_Position = u_mvpMatrix * a_position;
    a_color_out = a_color;
}
