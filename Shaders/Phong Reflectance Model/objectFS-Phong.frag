#version 330

in vec3 n_eye;
in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;
out vec4 frag_colour;

void main(){
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * vec3(1.0,1.0,1.0);    
    
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = LightPos;
    //vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0,1.0,1.0);
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(-FragPos); 
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0,1.0,1.0); 
    
    vec3 result = (ambient + diffuse + specular) * n_eye;
    frag_colour = vec4(result, 1.0);
}