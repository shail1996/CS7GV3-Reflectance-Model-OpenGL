#version 330

in vec3 vertex_position;
in vec3 vertex_normals;

out vec3 n_eye;
out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;


void main(){
 	//destColor = doColor();
	n_eye = (view * vec4 (0.0039, 0.1961, 0.1255, 0.0)).xyz;
	gl_Position =  proj * view * model * vec4 (vertex_position, 1.0);
	FragPos = vec3(view*model*vec4(vertex_position, 1.0));
	Normal = mat3(transpose(inverse(view * model)))*vertex_normals;
//	LightPos = vec3(view * vec4(vec3(10.0,10.0,10.0),1.0));
	LightPos = vec3(5.2f, 4.0f, 2.0f);
}
