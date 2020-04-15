#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <GL/glew.h>//OpenGL Extension Wrangler Library 
#include <GL/glut.h>//Utilities e.g: setting camera view and projection
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
using namespace glm;

#include "shader.h"

using namespace std;
struct Vector3f
{
	float x, y, z;
	Vector3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
	{
	}
};

glm::vec3 eye = glm::vec3(20.0f, 3.0f, 0.0f);
float lx = -1.0f, lz = -1.0f;
float FoV = 45.0f;

float deltaAngle = 0.0f;
float deltaMove = 0.2f;
//x, y, z for 3d Model
float xModel = 0.0f;
float yModel = 0.0f;
float zModel = 0.0f;

//x, y, z for cube
float xObj1 = -20.0f;
float yObj1 = 0.0f;
float zObj1 = 0.0f;

//bool to display cube 
bool isShown = true;

//bool to check crashed
bool isCrashed = false;

//score
float score = 0.0;
//level
float level = 0.1f;

Vector3f lightPos(-3, 5, 5); // lightDir vector
Vector3f lightColor(1, 1, 1); 
bool fullScreenMode = true; // Full-screen or windowed mode?
int windowWidth = 1024;     // Windowed mode's width
int windowHeight = 720;     // Windowed mode's height
int windowPosX = 100;      // Windowe	d mode's top-left corner x
int windowPosY = 100;      // Windowed mode's top-left corner y
float camAngle = 0.0;		  // angle of rotation for the camera direction

// actual vector representing the camera's direction
float MaterialSh = 120; 

GLfloat	rtri = 0;				// Angle For The Triangle ( NEW )
GLfloat	rquad = 0;				// Angle For The Quad ( NEW )
GLuint progShader;

GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

//GLuint TextureID;
//GLuint Texture;
//
//std::vector<glm::vec3> vertices;
//std::vector<glm::vec2> uvs;
//std::vector<glm::vec3> normals;
struct Model3D {
	GLuint TextureID;
	GLuint Texture;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	GLuint vertexbuffer;	
	GLuint uvbuffer;
	GLuint nmbuffer;
};

Model3D car;
Model3D table;
Model3D road;

GLuint loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file = fopen(imagepath, "rb");
	if (!file) {
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
		return 0;
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    fclose(file); return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    fclose(file); return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

										 // Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file can be closed.
	fclose(file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

bool loadOBJ(const char * path, 	std::vector<glm::vec3> & out_vertices,	std::vector<glm::vec2> & out_uvs,	std::vector<glm::vec3> & out_normals)
{
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}
	fclose(file);
	return true;
}

void computePos(float deltaMove) {

	eye.x += deltaMove * lx * 0.1f;

}

void computeDir(float deltaAngle) {
	camAngle += deltaAngle;
	lx = sin(camAngle);
	lz = -cos(camAngle);
}

void changeSize(int w, int h) {
	cout << "changeSize called" << endl;
	if(h == 0)	h = 1; // Prevent a divide by zero, when window is too short
	float ratio = 1.0* w / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);
	// Set the correct perspective.
	gluPerspective(70, ratio,1, 1000);//fovy, ratio, near, far

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt (eye.x, eye.y, eye.z, eye.x + lx, 1.0f,	eye.z + lz, 0.0, 1.0, 0.0);//eye, at, up
}

void updateShaderResources()
{
	GLint loc;
	loc = glGetUniformLocation(progShader, "materialSh");
	glUniform1f(loc, MaterialSh);

	loc = glGetUniformLocation(progShader, "Eye");
	glUniform3f(loc, eye.x, eye.y, eye.z);

	loc = glGetUniformLocation(progShader, "LightPosition_worldspace");
	glUniform3f(loc, lightPos.x, lightPos.y, lightPos.z);

	loc = glGetUniformLocation(progShader, "lightColor");
	glUniform3f(loc, lightColor.x, lightColor.y, lightColor.z);

	MatrixID = glGetUniformLocation(progShader, "MVP");	
	ViewMatrixID = glGetUniformLocation(progShader, "V");
	ModelMatrixID = glGetUniformLocation(progShader, "M");
}

GLuint VertexArrayID;

void draw3DModel(Model3D model3D, glm::mat4 modelMatrix, GLenum mode)
{
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * modelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	//glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, model3D.Texture);
	glUniform1i(model3D.TextureID, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model3D.vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	//2nd attribute buffer : colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model3D.uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	//2nd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, model3D.nmbuffer);
	glVertexAttribPointer(
		3,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// Draw the triangle !
	//glColor3f(1, 1, 0);
	glDrawArrays(mode, 0, model3D.vertices.size()); // 3 indices starting at 0 -> 1 triangle

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	// restore default settings
	//glPopMatrix();
}

void drawTable(float x, float y, float z) {
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glm::vec3 tableScale = glm::vec3(0.05f, 0.05f, 0.05f);
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, z));
	ModelMatrix = glm::scale(ModelMatrix, tableScale);
	draw3DModel(table, ModelMatrix, GL_TRIANGLES);
}

void drawRoad(float x, float y, float z) {
	glm::mat4 ModelMatrix3 = glm::mat4(1.0);
	ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(x, y, z));
	ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	draw3DModel(road, ModelMatrix3, GL_TRIANGLES);
}

float randomPos(float min, float max) {
	float range = max - min + 1;
	float num = rand() % int(range) + min;
	return num;
}

void displayCar() {
	glm::mat4 ModelMatrix2 = glm::mat4(1.0);
	if (deltaMove > 0) {
		xModel -= deltaMove / 10;
	}
	/*glm::vec3 modelScale = glm::vec3(0.1f, 0.1f, 0.1f);
	ModelMatrix2 = glm::scale(ModelMatrix2, modelScale);*/
	glm::vec3 carScale = glm::vec3(1.5f, 1.5f, 1.5f);
	ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(xModel, yModel, zModel));
	ModelMatrix2 = glm::scale(ModelMatrix2, carScale);
	ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	draw3DModel(car, ModelMatrix2, GL_TRIANGLES);
}

void displayTable() {
	if (isShown) {
		xObj1 -= randomPos(20.0 + level * 4, 45.0 + level * 3);
		zObj1 = randomPos(-3.0, 3.0);
		drawTable(xObj1, yObj1, zObj1);
	}
}

void nextLevel() {
	switch (int(score))
	{
	case 200:
		deltaMove += 0.02f;
		level += 0.01f;
		break;
	case 500:
		deltaMove += 0.02f;
		break;
	case 1000:
		deltaMove += 0.03f;
		break;
	case 1500:
		deltaMove += 0.04f;
		break;
	case 2000:
		deltaMove += 0.05f;
		break;
	case 2500:
		deltaMove += 0.07f;
		break;
	case 3000:
		deltaMove += 0.09f;
		break;
	case 3200:
		deltaMove += 0.1f;
		break;
	default:
		break;
	}
}

void getCrashed(float xCar, float xObject) {
	if (int(xCar) - int(xObject) - 2 == 0 && int(zModel) - int(zObj1) == 0) {
		deltaMove = 0.0f;
	}
	else {
		if (deltaMove > 0) {
			score += 0.1;
			nextLevel();
		}
		printf("score: %f\n", score);
		printf("level: %f\n", level);
		printf("delta move: %f\n", deltaMove);

	}
}

void drawObjects(GLvoid)									// Here's Where We Do All The Drawing
{
	if (deltaMove)
		computePos(deltaMove);
	if (deltaAngle)
		computeDir(deltaAngle);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	ViewMatrix = glm::lookAt(eye, eye + glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	//display car
	displayCar();
	//first, display an object, then random its position
	drawTable(xObj1, yObj1, zObj1);
	drawRoad(0.0f, 0.0f, zObj1);
	if (xModel <  xObj1 - 15.0f) {
		isShown != isShown;
		displayTable();
	}
	//detect crashed
	getCrashed(xModel, xObj1);

	//glm::mat4 ModelMatrix3 = glm::mat4(1.0);
	//ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(4.0f, 0.0f, 0.0f));
	//draw3DModel(car, ModelMatrix3, GL_TRIANGLES);

	/*glm::vec3 modelScale = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::mat4 ModelMatrix3 = glm::mat4(1.0);
	ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(4.0f, 3.0f, 0.0f));
	ModelMatrix3 = glm::scale(ModelMatrix3, modelScale);
	draw3DModel(canape, ModelMatrix3, GL_TRIANGLES);*/
}

/* Callback handler for normal-key event */
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:     // ESC key
		exit(0);
		break;
	}
}

void pressKey(int key, int xx, int yy) {

	switch (key) {
	case GLUT_KEY_LEFT: deltaAngle = -0.01f; break;
	case GLUT_KEY_RIGHT: deltaAngle = 0.01f; break;
	case GLUT_KEY_UP: deltaMove = 0.5f; break;
	case GLUT_KEY_DOWN: deltaMove = -0.5f; break;

	}
}

void releaseKey(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT: deltaAngle = 0.0f; break;
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN: deltaMove = 0; break;

	}
}

void controlCar(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		if (zModel <= 5) {
			zModel += deltaMove + 0.2f;
		}
		break;
	case 'd':
		if (-5 <= zModel) {
			zModel -= deltaMove + 0.2f;
		}
		break;
	default: break;
	}

}

void renderScene(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	drawObjects();
	updateShaderResources();
	
	glutSwapBuffers();
	gluLookAt(eye.x, eye.y, eye.z, eye.x + lx, 1.0f, eye.z + lz, 0.0, 1.0, 0.0);//eye, at, up

}

Model3D loadModel3D(const char* file, const char* textureFile) {
	Model3D result;	
	bool res = loadOBJ(file, result.vertices, result.uvs, result.normals);

	glGenBuffers(1, &result.vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, result.vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, result.vertices.size() * sizeof(glm::vec3), &result.vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &result.uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, result.uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, result.uvs.size() * sizeof(glm::vec2), &result.uvs[0], GL_STATIC_DRAW);

	glGenBuffers(1, &result.nmbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, result.nmbuffer);
	glBufferData(GL_ARRAY_BUFFER, result.normals.size() * sizeof(glm::vec3), &result.normals[0], GL_STATIC_DRAW);

	result.Texture = loadBMP_custom(textureFile);
	result.TextureID = glGetUniformLocation(progShader, "myTextureSampler");
	return result;
}

void initGL(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);//The mode parameter is a Boolean combination
	glutInitWindowPosition(100,100);//-1,-1 up to the windows manager
	glutInitWindowSize(windowWidth,windowHeight);
	glutCreateWindow("Lighting");

	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);

	glutSpecialFunc(pressKey); // Register callback handler for special-key event
	glutSpecialUpFunc(releaseKey);
	glutKeyboardFunc(keyboard);   // Register callback handler for special-key event
	glutKeyboardFunc(controlCar);
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0,1.0,1.0,1.0);
	glEnable(GL_CULL_FACE);	

	glewInit();

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	
	car = loadModel3D("carModel.obj", "suzanne.bmp");
	table = loadModel3D("table.obj", "suzanne.bmp");
	road = loadModel3D("roadV2.obj", "suzanne.bmp");

	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}
}

int main(int argc, char **argv) 
{
	initGL(argc, argv);

	//shader
	progShader = initShaders("lighting.vert", "lighting.frag");
	//progShader = initShaders("perFrag.vert", "perFrag.frag");
	updateShaderResources();

	glutMainLoop();//says: we're ready to get in the event processing loop

	//glDeleteBuffers(1, &vertexbuffer);
	//glDeleteBuffers(1, &colorbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	return 0;
}

