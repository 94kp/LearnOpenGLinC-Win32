#version 450 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
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
in vec3 eyeCoordinates;

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
    vec3 lightDir = normalize(light.position - eyeCoordinates); // was FragPos earlier, replaced it with eyeCoordinates
    float diff = max(dot(norm, lightDir), 0.0);
    // vec3 diffuse = light.diffuse * (diff * material.diffuse);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, a_texcoord_out).rgb * 1.5;
    vec3 ambient = light.ambient * texture(material.diffuse, a_texcoord_out).rgb;
    
    // specular
    // vec3 viewDir = normalize(light.position - eyeCoordinates);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 viewerVector = -eyeCoordinates.xyz;
    vec3 normalizedViewerVector = normalize(viewerVector);
    float spec = pow(max(dot(reflectDir, normalizedViewerVector), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.specular, a_texcoord_out).rgb;
    
    vec3 result = ambient + diffuse + specular;
    // FragColor = vec4(result, 1.0);

    FragColor = vec4(result, 1.0);
} 

