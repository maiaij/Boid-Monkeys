/************************************************
 *
 *  CSCI 4110 Assignment 3
 *
 *  Maia Johnson
 *  100739773
 *
 * 
 ************************************************/

#include <Windows.h>
#include <gl/glew.h>
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include <stdio.h>
#include "tiny_obj_loader.h"
#include <iostream>
#include "glm/gtx/string_cast.hpp"

 /*
  * The following structure represents a single mesh in the scene
 */
struct Mesh {
	GLuint vbuffer;		// vertex buffer name
	GLuint ibuffer;		// index buffer name
	GLuint triangles;	// number of triangles
	GLuint vbytes;		// size of vertices in bytes
	GLuint program;		// program for drawing the mesh
	glm::mat4 model;	// model transformation for the mesh
	glm::vec3 velocity;  // the translation num
	glm::vec3 position;  // the position on the ground
	std::string tribe;   // the tribe the monkey belongs to
};

std::vector<Mesh*> tribes;

std::vector<glm::vec3>ground_vert;

GLuint groundVAO;   // the data to be displayed 
int striangles;   // number of triangles 
GLuint sibuffer;

GLuint program;			// shader programs
GLuint objVAO;			// the data to be displayed -> monkeys
int triangles;			// number of triangles
int window;

char *vertexName;
char *fragmentName;

double theta, phi;
double r;

float dist = 95.0f;
float cx, cy, cz;

glm::vec3 MAX_VELOCITY = glm::vec3(0.0, 0.0, 0.05);

// collision avoidance
double dx, dy, dz;

glm::mat4 projection;	// projection matrix
float eyex, eyey, eyez;	// eye position

// physical properties
double p[3];
double v[3];
double a[3];



/*
 *  The init procedure creates the OpenGL data structures
 *  that contain the triangle geometry, compiles our
 *  shader program and links the shader programs to
 *  the data.
 */

GLuint loadProgram(char* vertex, char* fragment) {
	GLuint vs;
	GLuint fs;
	GLuint program;

	vs = buildShader(GL_VERTEX_SHADER, vertex);
	fs = buildShader(GL_FRAGMENT_SHADER, fragment);
	program = buildProgram(vs, fs, 0);

	return(program);
}

// use to load plane and monkey
Mesh* loadOBJ(char* filename) {
	GLuint vbuffer;
	GLuint ibuffer;
	GLuint objVAO;
	GLfloat* vertices;
	GLfloat* normals;
	GLuint* indices;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	int nv;
	int nn;
	int ni;
	int i;
	float xmin, ymin, zmin;
	float xmax, ymax, zmax;
	float dx, dy, dz;
	float scale;
	int triangles;
	Mesh* mesh;

	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);

	/*  Load the obj file */

	std::string err = tinyobj::LoadObj(shapes, materials, filename, 0);

	if (!err.empty()) {
		std::cerr << err << std::endl;
		return(NULL);
	}

	/*  Retrieve the vertex coordinate data */

	nv = shapes[0].mesh.positions.size();
	vertices = new GLfloat[nv];
	for (i = 0; i < nv; i++) {
		vertices[i] = shapes[0].mesh.positions[i];
	}

	/*
	 *  Find the range of the x, y and z
	 *  coordinates.
	 */

	xmin = ymin = zmin = 1000000.0;
	xmax = ymax = zmax = -1000000.0;
	for (i = 0; i < nv / 3; i++) {
		if (vertices[3 * i] < xmin)
			xmin = vertices[3 * i];
		if (vertices[3 * i] > xmax)
			xmax = vertices[3 * i];
		if (vertices[3 * i + 1] < ymin)
			ymin = vertices[3 * i + 1];
		if (vertices[3 * i + 1] > ymax)
			ymax = vertices[3 * i + 1];
		if (vertices[3 * i + 2] < zmin)
			zmin = vertices[3 * i + 2];
		if (vertices[3 * i + 2] > zmax)
			zmax = vertices[3 * i + 2];
	}

	/*
	 *  Scales the vertices so the longest axis is unit length
	*/

	dx = xmax - xmin;
	dy = ymax - ymin;
	dz = zmax - zmin;

	scale = dx;
	if (dy > scale)
		scale = dy;
	if (dz > scale)
		scale = dz;

	

	scale = 1.0 / scale;

	int test = 0;
	if (!std::string(filename).compare("ground.obj")) {
		for (i = 0; i < nv; i++) {
			vertices[i] = vertices[i] * scale;
		}
	}

	else {
		for (i = 0; i < nv; i+=3) {
			if(vertices[i]>0 && vertices[i+1] < 0 && vertices[i+2] < 0){
				test++;
				ground_vert.push_back(glm::vec3(vertices[i] * scale, vertices[i + 1] * scale, vertices[i + 2] * scale));
			}
		}
	}
	

	/*  Retrieve the vertex normals */

	nn = shapes[0].mesh.normals.size();
	normals = new GLfloat[nn];
	for (i = 0; i < nn; i++) {
		normals[i] = shapes[0].mesh.normals[i];
	}

	/*  Retrieve the triangle indices */

	ni = shapes[0].mesh.indices.size();
	triangles = ni / 3;
	indices = new GLuint[ni];
	for (i = 0; i < ni; i++) {
		indices[i] = shapes[0].mesh.indices[i];
	}

	/*
	 *  load the vertex coordinate data
	 */
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, (nv + nn) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nn * sizeof(GLfloat), normals);

	/*
	 *  load the vertex indexes
	 */
	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indices, GL_STATIC_DRAW);

	/*
	 *  Allocate the mesh structure and fill it in
	*/
	mesh = new Mesh();
	mesh->vbuffer = vbuffer;
	mesh->ibuffer = ibuffer;
	mesh->triangles = triangles;
	mesh->vbytes = nv * sizeof(GLfloat);
	//mesh->velocity = glm::vec3(0.0, 0.0, 0.01);

	delete[] vertices;
	delete[] normals;
	delete[] indices;

	return(mesh);
}

Mesh* copyObject(Mesh* obj) {
	Mesh* result;

	result = new Mesh();
	*result = *obj;
	return(result);
}

void addModels() {
	Mesh* mesh;
	GLuint program;
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	int colourLoc;

	glm::mat4 trans = glm::mat4(1.0f);
	
	// plane
	program = loadProgram((char*)"a3.vs", (char*)"sand.fs"); //set fragment shader
	mesh = loadOBJ((char*)"ground.obj");
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(100.0, 100.0, 100.0));
	mesh->program = program;
	tribes.push_back(mesh);

	// red tribe
	program = loadProgram((char*)"a3.vs", (char*)"red.fs"); //set fragment shader

	mesh = loadOBJ((char*)"monkey.obj");
	mesh->tribe = "red";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 1.57f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(0.0, 0.0, -7.5));
	mesh->position = glm::vec3(0.0, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.002);
	mesh->program = program;
	tribes.push_back(mesh);

	mesh = copyObject(mesh);
	mesh->tribe = "red";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 1.57f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(-3.0, 0.0, -7.5));
	mesh->position = glm::vec3(-3, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.001);
	mesh->program = program;
	tribes.push_back(mesh);

	mesh = copyObject(mesh);
	mesh->tribe = "red";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 1.57f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(-2.0, 0.0, -7.5));
	mesh->position = glm::vec3(-2, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.001);
	mesh->program = program;
	tribes.push_back(mesh);
	///*
	mesh = copyObject(mesh);
	mesh->tribe = "red";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 1.57f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(1.0, 0.0, -7.5));
	mesh->position = glm::vec3(1, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.001);
	mesh->program = program;
	tribes.push_back(mesh);
	//*/

	// blue tribe
	program = loadProgram((char*)"a3.vs", (char*)"blue.fs"); //set fragment shader
	
	mesh = loadOBJ((char*)"monkey.obj");
	mesh->tribe = "blue";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 4.71f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(1.0, 0.0, -7.5));
	mesh->position = glm::vec3(1.0, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.001);
	mesh->program = program;
	tribes.push_back(mesh);

	mesh = copyObject(mesh);
	mesh->tribe = "blue";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 4.71f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(3.0, 0.0, -7.5));
	mesh->position = glm::vec3(3.0, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.002);
	mesh->program = program;
	tribes.push_back(mesh);

	mesh = copyObject(mesh);
	mesh->tribe = "blue";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 4.71f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(-2.0, 0.0, -7.5));
	mesh->position = glm::vec3(-2, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.001);
	mesh->program = program;
	tribes.push_back(mesh);
	///*
	mesh = copyObject(mesh);
	mesh->tribe = "blue";
	mesh->model = glm::translate(trans, glm::vec3(0.0, 0.0, 0.0));
	mesh->model = glm::rotate(mesh->model, 3 * 4.71f, glm::vec3(0.0, 1.0, 0.0));
	mesh->model = glm::scale(mesh->model, glm::vec3(5.0, 5.0, 5.0));
	mesh->model = glm::translate(mesh->model, glm::vec3(1.0, 0.0, -7.5));
	mesh->position = glm::vec3(1, 0.0, 0.0);
	mesh->velocity = glm::vec3(0.0, 0.0, 0.001);
	mesh->program = program;
	tribes.push_back(mesh);
	//*/

}

/*
 *  Executed each time the window is resized,
 *  usually once at the start of the program.
 */
void framebufferSizeCallback(GLFWwindow *window, int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0f * w / h;

	glfwMakeContextCurrent(window);

	glViewport(0, 0, w, h);

	projection = glm::perspective(0.7f, ratio, 1.0f, 800.0f);

}

glm::vec3 groupTogether(Mesh* monkey, int start, int end) {
	// only work with pos[0]
	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	for (int i = start; i <= end; i++) {
		if (monkey != tribes[i] && tribes[i]->tribe == monkey->tribe) {
			pos += tribes[i]->position;
		}
	}

	//std::cout << glm::to_string(monkey->position) << std::endl;

	//pos[0] = (pos[0] / (end - start + 1));
	pos[0] = (pos[0] / (end - start + 1)) / 2;

	return (pos - monkey->position) * 0.01f;
}

glm::vec3 avoidMonkeys(Mesh* monkey, int start, int end) {
	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	for (int i = start; i <= end; i++) {
		if (monkey != tribes[i]) {
			//if (glm::abs(tribes[i]->position[0] - monkey->position[0]) < 1 ) {
			if (glm::abs(tribes[i]->position[0] - monkey->position[0]) < 0.5 ){
				//pos += tribes[i]->position + monkey->position;
				//pos -= tribes[i]->position - monkey->position;
				pos += glm::vec3(0.1, 0.0, 0.0);
			}

			if (glm::abs(tribes[i]->position[0] - monkey->position[0]) < 1 && monkey->tribe == tribes[i]->tribe) {
				//pos += tribes[i]->position + monkey->position;
				//pos -= tribes[i]->position - monkey->position;
				pos += glm::vec3(0.1, 0.0, 0.0);
			}

			if ((glm::abs(tribes[i]->velocity[2] - monkey->velocity[2]) < 0.1) ) {
				//pos -= tribes[i]->position - monkey->position;
				//pos += glm::vec3(0.1, 0.0, 0.0);
				pos -= tribes[i]->velocity - monkey->velocity;
				
			}
		}
	}

	return pos;
}

glm::vec3 matchMotion(Mesh* monkey, int start, int end) {
	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	for (int i = start; i <= end; i++) {
		if (monkey != tribes[i] && tribes[i]->tribe == monkey->tribe) {
			pos += tribes[i]->velocity;
			//if (pos[2] > MAX_VELOCITY[2]) {
				//pos = MAX_VELOCITY;
			//}
		}
	}
	pos[2] = (pos[2] / (end - start + 1)) / 2;
	//pos[2] = pos[2] / (end - start + 1);
	//pos = pos * (float)(1/ tribes.size()-1);

	return (pos - monkey->velocity) * 0.125f;
}

glm::vec3 groundCollision(Mesh* monkey) {
	glm::vec3 pos = glm::vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < ground_vert.size(); i++) {
		if (ground_vert[i] == monkey->position) {
			pos -= monkey->position - ground_vert[i];
			pos += monkey->velocity;
		}
	}
	return pos;
}


void display() {
	glm::mat4 view;
	glm::mat4 modelViewPerspective;
	glm::mat3 normal;
	int modelLoc;
	int normalLoc;
	int i;
	Mesh* mesh;
	GLuint vPosition;
	GLuint vNormal;

	//view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
		glm::vec3(cx, cy, cz),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// monkey animation

	for (i = 0; i < tribes.size(); i++) {
		mesh = tribes[i];
		glUseProgram(mesh->program);

		// movement and avoidance
		if ((i > 0) && (i <= (tribes.size() - 1))) {

			//red

			glm::vec3 res, pos1, pos2, pos3, pos4;
			res = mesh->velocity;

			pos1 = groupTogether(mesh, 1, (tribes.size() - 1));
			pos2 = avoidMonkeys(mesh, 1, (tribes.size() - 1));
			pos3 = matchMotion(mesh, 1, (tribes.size() - 1));
			//pos4 = groundCollision(mesh);
			res += pos1 + pos2 + pos3 += pos4;


			//std::cout << glm::to_string(res) << std::endl;
			mesh->position += res;
			//mesh->velocity = res;

			if (mesh->position[2] > 17) {
				// monkeys stop at destination
				res = glm::vec3(0.0, 0.0, 0.0);
			}


			mesh->model = glm::translate(mesh->model, res);
			//std::cout << glm::to_string(mesh->position) << std::endl;
		}
		

		// Set up the position and normal attributes

		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbuffer);
		vPosition = glGetAttribLocation(mesh->program, "vPosition");
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vPosition);
		vNormal = glGetAttribLocation(mesh->program, "vNormal");
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)(mesh->vbytes));
		glEnableVertexAttribArray(vNormal);

		// Set up the transformation matrices for the vertices and normal vectors

		modelLoc = glGetUniformLocation(mesh->program, "model");
		modelViewPerspective = projection * view * mesh->model;
		glUniformMatrix4fv(modelLoc, 1, 0, glm::value_ptr(modelViewPerspective));
		normalLoc = glGetUniformLocation(mesh->program, "normalMat");
		normal = glm::transpose(glm::inverse(glm::mat3(view * mesh->model)));
		glUniformMatrix3fv(normalLoc, 1, 0, glm::value_ptr(normal));

		// draw the mesh models

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibuffer);
		glDrawElements(GL_TRIANGLES, 3 * mesh->triangles, GL_UNSIGNED_INT, NULL);
	}
}

/*
 *  Called each time a key is pressed on
 *  the keyboard.
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		cx -= 0.5;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		cx += 0.5;
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		cz += 0.5;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		cz -= 0.5;
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		dist -= 0.02; // zoom in
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		dist += 0.02; // zoom out

	//eyex = (float)(r*sin(theta)*cos(phi));
	//eyey = (float)(r*sin(theta)*sin(phi));
	//eyez = (float)(r*cos(theta));

}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char **argv) {
	GLFWwindow *window;
	
	//glfwSwapInterval(3);
	
	/*
	p[0] = 0.0;
	p[1] = -5.0;
	p[2] = 1.0;

	v[0] = 0.0;
	v[1] = 0.1;
	v[2] = 0.0;

	a[0] = 0.0;
	a[1] = 0.0;
	a[2] = 0.0;

	r = sqrt(dx * dx + dy * dy + dz * dz);
	if (r < 2.0) {
		if (dx < 0)
			a[0] -= 0.03 / r;
		else
			a[0] += 0.03 / r;
	}

	if(argc > 1) {
		vertexName = argv[1];
	} else {
		vertexName = "a";
	}
	if(argc > 2) {
		fragmentName = argv[2];
	} else {
		fragmentName = "a";
	}*/

	// start by setting error callback in case something goes wrong

	glfwSetErrorCallback(error_callback);

	// initialize glfw

	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
	}

	// create the window used by our application

	window = glfwCreateWindow(512, 512, "assignment three", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// establish framebuffer size change and input callbacks

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, key_callback);
	/*
	 *  initialize glew
	 */
	glfwMakeContextCurrent(window);
	GLenum error = glewInit();
	if(error != GLEW_OK) {
		printf("Error starting GLEW: %s\n",glewGetErrorString(error));
		exit(0);
	}

	eyex = 0.0;
	eyey = 150.0;
	eyez = 200.0;

	theta = 0.5;
	phi = 1.5;
	r = 20.0;

	addModels();

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0,1.0,1.0,1.0);
	glViewport(0, 0, 512, 512);

	glfwSwapInterval(1);

	projection = glm::perspective(dist, 1.0f, 1.0f, 800.0f);

	

	// GLFW main loop, display model, swapbuffer and check for input

	while (!glfwWindowShouldClose(window)) {
		projection = glm::perspective(dist, 1.0f, 1.0f, 700.0f);
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		
		/*
		a[0] = a[1] = a[2] = 0.0;
		dx = p[0];
		dy = p[1];
		dz = p[2];
		r = sqrt(dx * dx + dy * dy + dz * dz);
		if (r < 2.0) {
			if (dx < 0)
				a[0] -= 0.01 / r*r;
			else
				a[0] += 0.01 / r*r;
		}
		dx = p[0] - 4.0;
		dy = p[1] - 2.0;
		dz = p[2];

		v[0] += a[0];
		v[1] += a[1];
		v[2] += a[2];
		p[0] += v[0];
		p[1] += v[1];
		p[2] += v[2];
		*/
	}

	glfwTerminate();

}