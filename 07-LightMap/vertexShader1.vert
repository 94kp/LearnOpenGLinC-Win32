#version 450 core

in vec4 a_position;
in vec3 a_normal;
in vec2 a_texcoord;
in vec2 a_texcoord_specular;
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;
out vec3 FragPos;
out vec3 a_normal_out;
out vec2 a_texcoord_out;

out vec3 eyeCoordinates;

void main(void)
{
    gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * a_position; 

    FragPos = vec3(u_modelMatrix * a_position); // moving position to world space coordinates
    // vec4 Normal = mat4(transpose(inverse(u_modelMatrix))) * vec4(a_normal, 1.0);
    mat3 normalMatrix = mat3(u_viewMatrix * u_modelMatrix);
    vec3 transformedNormals = normalMatrix * a_normal;
    
    a_texcoord_out = a_texcoord;
    eyeCoordinates = vec3(u_viewMatrix * u_modelMatrix * a_position);
    a_normal_out = vec3(transformedNormals);
}
