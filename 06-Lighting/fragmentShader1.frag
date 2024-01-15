#version 450 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    vec3 specular;
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec2 a_texcoord_out;
in vec3 a_normal_out;

uniform sampler2D u_textureSampler;
uniform vec3 u_viewPos;
uniform Material material;
uniform Light light;

void main()
{
    // ambient
    // vec3 ambient = light.ambient * material.ambient;
  	
    // diffuse
    vec3 norm = normalize(a_normal_out);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    // vec3 diffuse = light.diffuse * (diff * material.diffuse);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, a_texcoord_out));
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, a_texcoord_out));
    
    // specular
    vec3 viewDir = normalize(u_viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
    
    vec3 result = ambient + diffuse + specular;
    // FragColor = vec4(result, 1.0);

    FragColor = vec4(result, 1.0);
} 

