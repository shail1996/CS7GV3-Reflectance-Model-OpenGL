#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "maths_funcs.h"
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "teapot.h"

#include <windows.h> 
#include <mmsystem.h>

int width = 1000;
int height = 1000;
int specularStrength = 0;
float specularStrengthCook = 0.0;

GLuint ObjectShaderProgramID;
GLuint ObjectShaderProgramRefractionID;
GLuint ObjectShaderProgramRefractionID_Temp;
GLuint textureID;

GLuint skyboxVAO;
GLuint skyboxVBO;
GLuint objectVAO = 0;
GLuint objectVBO = 0;
GLuint objectNormalVBO = 0;
GLuint objectloc1;
GLuint objectloc2;
GLuint skyboxloc1;

GLfloat rotatez = 0.0f;



std::string readShaderSource(const std::string& fileName)
{
	std::ifstream file(fileName.c_str());
	if (file.fail()) {
		std::cerr << "Error loading shader called " << fileName << std::endl;
		exit(EXIT_FAILURE);
	}

	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	return stream.str();
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
	GLuint ShaderObj = glCreateShader(ShaderType);
	if (ShaderObj == 0) {
		std::cerr << "Error creating shader type " << ShaderType << std::endl;
		exit(EXIT_FAILURE);
	}

	/* bind shader source code to shader object */
	std::string outShader = readShaderSource(pShaderText);
	const char* pShaderSource = outShader.c_str();
	glShaderSource(ShaderObj, 1, (const GLchar * *)& pShaderSource, NULL);

	/* compile the shader and check for errors */
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling shader type " << ShaderType << ": " << InfoLog << std::endl;
		exit(EXIT_FAILURE);
	}
	glAttachShader(ShaderProgram, ShaderObj); /* attach compiled shader to shader programme */
}

GLuint CompileShaders(const char* pVShaderText, const char* pFShaderText)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	GLuint ShaderProgramID = glCreateProgram();
	if (ShaderProgramID == 0) {
		std::cerr << "Error creating shader program" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(ShaderProgramID, pVShaderText, GL_VERTEX_SHADER);
	AddShader(ShaderProgramID, pFShaderText, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(ShaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		exit(EXIT_FAILURE);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(ShaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(ShaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		exit(EXIT_FAILURE);
	}
	return ShaderProgramID;
}

void generateObjectBufferTeapot(GLuint TempObjectShaderProgramID)
{
	GLuint vp_vbo = 0;

	objectloc1 = glGetAttribLocation(TempObjectShaderProgramID, "vertex_position");
	objectloc2 = glGetAttribLocation(TempObjectShaderProgramID, "vertex_normals");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_vertex_points, GL_STATIC_DRAW);
	GLuint vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * teapot_vertex_count * sizeof(float), teapot_normals, GL_STATIC_DRAW);

	glGenVertexArrays(1, &objectVAO);
	glBindVertexArray(objectVAO);

	glEnableVertexAttribArray(objectloc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(objectloc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(objectloc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(objectloc2, 3, GL_FLOAT, GL_FALSE, 0, NULL); 
}



void init(void) {

	// Toon Reflectance Model
	ObjectShaderProgramID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-1/Shaders/Toon Reflectance Model/objectVS-Toon.vert",
		"C:/Users/lenovo/source/repos/Rendering Assignment-1/Shaders/Toon Reflectance Model/objectFS-Toon.frag");
 	generateObjectBufferTeapot(ObjectShaderProgramID);

	// Phong Reflectance Model
	ObjectShaderProgramRefractionID = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-1/Shaders/Phong Reflectance Model/objectVS-Phong.vert",
		"C:/Users/lenovo/source/repos/Rendering Assignment-1/Shaders/Phong Reflectance Model/objectFS-Phong.frag");
	generateObjectBufferTeapot(ObjectShaderProgramRefractionID);

	// Cook-Torrance Reflectance Model
	ObjectShaderProgramRefractionID_Temp = CompileShaders("C:/Users/lenovo/source/repos/Rendering Assignment-1/Shaders/Cook-Torrance Reflectance Model/objectVS-Cook.vert",
		"C:/Users/lenovo/source/repos/Rendering Assignment-1/Shaders/Cook-Torrance Reflectance Model/objectFS-Cook.frag");
	generateObjectBufferTeapot(ObjectShaderProgramRefractionID_Temp);
}

void display() {


	// Toon Reflectance Model
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS); 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(ObjectShaderProgramID);

	int matrix_location = glGetUniformLocation(ObjectShaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(ObjectShaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(ObjectShaderProgramID, "proj");
	GLuint specularStrengthLocation = glGetUniformLocation(ObjectShaderProgramID, "specularChangeVal");
	glUniform1f(specularStrengthLocation, specularStrength);
	mat4 view = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	mat4 persp_proj = perspective(65.0, (float)width / (float)height, 0.1, 100.0);
	mat4 model = rotate_y_deg(identity_mat4(), rotatez);

	glViewport(30, (height / 5), (width/2 + 30), height/2);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// Phong Reflectance Model
	glUseProgram(ObjectShaderProgramRefractionID);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

	matrix_location = glGetUniformLocation(ObjectShaderProgramRefractionID, "model");
	view_mat_location = glGetUniformLocation(ObjectShaderProgramRefractionID, "view");
	proj_mat_location = glGetUniformLocation(ObjectShaderProgramRefractionID, "proj");

	view = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	persp_proj = perspective(65.0, (float)width / (float)height, 0.1, 100.0);
	model = rotate_y_deg(identity_mat4(), rotatez);

	glViewport(width/2, (height/2 - 50), width / 2, (height / 2 - 50));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);

	// Cook-Torrance Reflectance Model
	glUseProgram(ObjectShaderProgramRefractionID_Temp);

	matrix_location = glGetUniformLocation(ObjectShaderProgramRefractionID_Temp, "model");
	view_mat_location = glGetUniformLocation(ObjectShaderProgramRefractionID_Temp, "view");
	proj_mat_location = glGetUniformLocation(ObjectShaderProgramRefractionID_Temp, "proj");
	GLuint specularStrengthLocationCook = glGetUniformLocation(ObjectShaderProgramRefractionID_Temp, "specularChangeValCook");
	glUniform1f(specularStrengthLocationCook, specularStrengthCook);
	view = translate(identity_mat4(), vec3(0.0, 0.0, -40.0));
	persp_proj = perspective(65.0, (float)width / (float)height, 0.1, 100.0);
	model = rotate_y_deg(identity_mat4(), rotatez);

	glViewport(width / 2, 0, width / 2, (height / 2 - 50));
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, teapot_vertex_count);


	glutSwapBuffers();
}

void keyPress(unsigned char key, int xmouse, int ymouse) {
	std::cout << specularStrength << "Keypress: " << key << std::endl;
	switch (key) {
	case('a'):
		specularStrength += 5;
		break;
	case('q'):
		if(specularStrength > 0)
			specularStrength -= 5;
		break;
	case ('w'):
		specularStrengthCook += 0.1;
		break;
	case ('s'): 
		if (specularStrengthCook > 0.1)
			specularStrengthCook -= 0.1;
		break;
	}
};



void updateScene() {

	rotatez += 0.5f;
	// Draw the next frame
	glutPostRedisplay();
}


int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow(argv[1]);

	glutDisplayFunc(display);

	glutIdleFunc(updateScene);
	glutKeyboardFunc(keyPress);
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(res) << std::endl;
		return EXIT_FAILURE;
	}

	init();
	glutMainLoop();
	return 0;
}