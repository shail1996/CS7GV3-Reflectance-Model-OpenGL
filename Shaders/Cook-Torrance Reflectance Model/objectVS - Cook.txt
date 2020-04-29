#version 330

in vec3 vertex_position;
in vec3 vertex_normals;

out vec3 vsNormals;
out vec3 vsViewDir;
out vec3 vsLightDir;
out float vsLightDistance;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 ortho = mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
uniform mat4 model;
uniform vec3 lightPos = vec3(0.0f,50.0f,100.0f);
uniform vec3 viewPos = vec3(0.0f,0.0f,40.0f);


void main(){
	vec4 fragPos = (model * vec4(vertex_position, 1.0));
	vsNormals = (model * vec4(vertex_normals, 0)).xyz;
	vsViewDir = normalize(viewPos - fragPos.xyz);
	vsLightDir = lightPos - fragPos.xyz;
	vsLightDistance = length(vsLightDir);
	vsLightDir /= vsLightDistance;
	vsLightDistance *= vsLightDistance;
	gl_Position =  proj * view * model * ortho* vec4 (vertex_position, 1.0);
}