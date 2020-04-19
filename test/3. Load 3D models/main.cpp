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
float FoV = 100.0f;

float deltaAngle = 0.0f;
float deltaMove = 1.5f;
//x, y, z for 3d Model
float xModel = 10.0f;
float yModel = 0.0f;
float zModel = 0.0f;

//x, y, z for obj
float xObj1 = -20.0f;
float yObj1 = 0.0f;
float zObj1 = 0.0f;

//x road
float xRoad = -10.0f;

//x forest
float xForest = -20.0f;
float yForest = 0.0f;
float zForest = 5.5f;

//sign
float xSign = 0.0f;
float ySign = 0.0f;
float zSign = -6.0f;

//traffic light
float xLightSign = -100.0f;
float yLightSign = 0.0f;
float zLightSign = 7.5f;

//watch tower
float xWatchTower = -250.0f;
float yWatchTower = 0.0f;
float zWatchTower = 10.0f;

//wood fence
float xWoodFence = -80.0f;
float yWoodFence = 0.0f;
float zWoodFence = 6.0f;

//plam tree
float xPalmTree = 0.0f;
float yPalmTree = 0.0f;
float zPalmTree = 0.0f;

//palm trees
float xPalmTrees = -20.0f;
float zPalmTrees = -22.0f;

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
Model3D tree;
Model3D stopSign;
Model3D lightSign;
Model3D watchTower;
Model3D woodFence;
Model3D palmTree;
Model3D bus;
Model3D airBalloon;

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
	gluPerspective(100, ratio,1, 2000);//fovy, ratio, near, far

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

void drawCar(float x, float y, float z) {
	glm::mat4 ModelMatrix2 = glm::mat4(1.0);
	glm::vec3 carScale = glm::vec3(1.8f, 1.8f, 1.8f);
	ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(x, y, z));
	ModelMatrix2 = glm::scale(ModelMatrix2, carScale);
	ModelMatrix2 = glm::rotate(ModelMatrix2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	draw3DModel(car, ModelMatrix2, GL_TRIANGLES);
}

void drawBus(float x, float y, float z) {
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glm::vec3 busScale = glm::vec3(2.5f, 3.0f, 3.0f);
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(x, y, z));
	ModelMatrix = glm::scale(ModelMatrix, busScale);
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	draw3DModel(bus, ModelMatrix, GL_LINES);
}

void drawRoad(float x, float y, float z) {
	glm::mat4 ModelMatrix3 = glm::mat4(1.0);
	glm::vec3 roadScale = glm::vec3(15.5f, 0.5f, 3.0f);
	ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(x, y, z));
	ModelMatrix3 = glm::scale(ModelMatrix3, roadScale);
	ModelMatrix3 = glm::rotate(ModelMatrix3, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	draw3DModel(road, ModelMatrix3, GL_LINES);

}

void drawTree(float x, float y, float z) {
	glm::mat4 ModelMatrix4 = glm::mat4(1.0);
	ModelMatrix4 = glm::translate(ModelMatrix4, glm::vec3(x, y, z));
	//ModelMatrix4 = glm::rotate(ModelMatrix4, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	draw3DModel(tree, ModelMatrix4, GL_LINES);
}

void drawSign(float x, float y, float z) {
	glm::mat4 ModelMatrix5 = glm::mat4(1.0);
	ModelMatrix5 = glm::translate(ModelMatrix5, glm::vec3(x, y, z));
	glm::vec3 signScale = glm::vec3(0.02f, 0.02f, 0.02f);
	ModelMatrix5 = glm::scale(ModelMatrix5, signScale);
	ModelMatrix5 = glm::rotate(ModelMatrix5, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix5 = glm::rotate(ModelMatrix5, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	draw3DModel(stopSign, ModelMatrix5, GL_LINES);
}

void drawLightSign(float x, float y, float z) {
	glm::mat4 ModelMatrix6 = glm::mat4(1.0);
	ModelMatrix6 = glm::translate(ModelMatrix6, glm::vec3(x, y, z));
	glm::vec3 lightScale = glm::vec3(0.06f, 0.06f, 0.06f);
	ModelMatrix6 = glm::scale(ModelMatrix6, lightScale);
	ModelMatrix6 = glm::rotate(ModelMatrix6, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix6 = glm::rotate(ModelMatrix6, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	draw3DModel(lightSign, ModelMatrix6, GL_LINES);
}

void drawWatchTower(float x, float y, float z) {
	glm::mat4 ModelMatrix7 = glm::mat4(1.0);
	ModelMatrix7 = glm::translate(ModelMatrix7, glm::vec3(x, y, z));
	glm::vec3 towerScale = glm::vec3(2.5f, 2.5f, 2.5f);
	ModelMatrix7 = glm::scale(ModelMatrix7, towerScale);
	/*ModelMatrix7 = glm::rotate(ModelMatrix7, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	ModelMatrix7 = glm::rotate(ModelMatrix7, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));*/
	draw3DModel(watchTower, ModelMatrix7, GL_LINES);
}

void drawWoodFence(float x, float y, float z) {
	glm::mat4 ModelMatrix8 = glm::mat4(1.0);
	ModelMatrix8 = glm::translate(ModelMatrix8, glm::vec3(x, y, z));
	glm::vec3 lightScale = glm::vec3(0.02f, 0.02f, 0.02f);
	ModelMatrix8 = glm::scale(ModelMatrix8, lightScale);
	ModelMatrix8 = glm::rotate(ModelMatrix8, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	draw3DModel(woodFence, ModelMatrix8, GL_LINES);
}

void drawPalmTree(float x, float y, float z, float treeScale, float treeRotate, glm::vec3 vector) {
	glm::mat4 ModelMatrix9 = glm::mat4(1.0);
	ModelMatrix9 = glm::translate(ModelMatrix9, glm::vec3(x, y, z));
	glm::vec3 lightScale = glm::vec3(treeScale, treeScale, treeScale);
	ModelMatrix9 = glm::scale(ModelMatrix9, lightScale);
	ModelMatrix9 = glm::rotate(ModelMatrix9, glm::radians(treeRotate), vector);
	draw3DModel(palmTree, ModelMatrix9, GL_LINES);
}

void drawAirBalloon(float x, float y, float z) {
	glm::mat4 ModelMatrix10 = glm::mat4(1.0);
	ModelMatrix10 = glm::translate(ModelMatrix10, glm::vec3(x, y, z));
	glm::vec3 lightScale = glm::vec3(0.5f, 0.5f, 0.5f);
	ModelMatrix10 = glm::scale(ModelMatrix10, lightScale);
	draw3DModel(airBalloon, ModelMatrix10, GL_LINES);
}


float randomPos(float min, float max) {
	float range = max - min + 1;
	float num = rand() % int(range) + min;
	return num;
}

void displayCar() {
	if (deltaMove > 0) {
		xModel -= deltaMove / 10;
	}
	/*glm::vec3 modelScale = glm::vec3(0.1f, 0.1f, 0.1f);
	ModelMatrix2 = glm::scale(ModelMatrix2, modelScale);*/
	drawCar(xModel, yModel, zModel);
}

void displayTrees(float x, float z) {
	
	drawPalmTree(x - 20.0f, 0.0f, z - 8.0f, 0.013, -90.0f, glm::vec3(1.0, 0.0, 0.0));
	drawPalmTree(x - 16.0f, 0.0f, z - 6.0f, 0.017, -90.0f, glm::vec3(1.0, 0.0, 0.0));
	drawPalmTree(x - 13.0f, 0.0f, z - 7.0f, 0.025, -90.0f, glm::vec3(1.0, 0.0, 0.0));
	drawPalmTree(x - 18.0f, 0.0f, z - 7.5f, 0.0155, -90.0f, glm::vec3(1.0, 0.0, 0.0));
	drawPalmTree(x - 11.0f, 0.0f, z - 9.0f, 0.02, -90.0f, glm::vec3(1.0, 0.0, 0.0));


}

void displayBus() {
	xObj1 -= randomPos(80.0 , 120.0);
	zObj1 = randomPos(-3.0, 3.0);
	if (deltaMove > 0) {
		xObj1 -= deltaMove / 15;

	}
	drawBus(xObj1, yObj1, zObj1);
}

void displaySideObj() {
	float a = randomPos(-5.0, 5.0);
	float b = randomPos(1.0, 8.0);
	xSign -= randomPos(300.0, 350.0);
	xLightSign -= randomPos(300.0, 360.0);
	xWatchTower -= randomPos(300.0, 320.0);
	
	drawLightSign(xLightSign, yLightSign, zLightSign);
	drawSign(xSign, ySign, zSign);
	drawWatchTower(xWatchTower, yWatchTower, -1.0 * zWatchTower);
}

void nextLevel() {
	switch (int(score))
	{
	case 50:
		deltaMove += 0.1f;
		break;
	case 100:
		deltaMove += 0.2f;
		break;
	case 150:
		deltaMove += 0.3f;
		break;
	case 200:
		deltaMove += 0.4f;
		break;
	case 250:
		deltaMove += 0.5f;
		break;
	case 300:
		deltaMove += 0.6f;
		break;
	case 350:
		deltaMove += 0.7f;
		break;
	case 400:
		deltaMove += 0.8f;
		break;
	case 450:
		deltaMove += 0.9f;
		break;
	case 500:
		deltaMove += 1.0f;
		break;
	default:
		break;
	}
}

void getCrashed(float xCar, float xObject, float zCar, float zObject, float radiusCar, float radiusObj) {
	float xd, zd, distance;
	xd = xCar - xObject;
	zd = zCar - zObject;
	distance = sqrt(xd * xd + zd * zd);
	
	if (radiusCar + radiusObj >= distance) {
		deltaMove = 0.0f;
	}else {
		if (deltaMove > 0) {
			score += 0.1;
			nextLevel();
			printf("score: %f\n", score);
		}
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
	
	drawRoad(eye.x - 55.0f, -1.0f, 0.0f);
	drawSign(xSign, ySign, zSign);
	drawLightSign(xLightSign, yLightSign, zLightSign);
	drawWatchTower(xWatchTower, yWatchTower, zWatchTower);
	drawAirBalloon(0.0f, 0.0f, 0.0f);
	displayTrees(xPalmTrees, zPalmTrees);
	
	//display car
	displayCar();
	//first, display an object, then random its position
	drawBus(xObj1, yObj1, zObj1);

	if (xModel <  xObj1 - 2.0f) {
		displayBus();
	}
	if (xModel < xWatchTower - 15.0f) {
		displaySideObj();
	}
	if (xModel < xPalmTrees - 25.0f) {
		xPalmTrees -= 500.0f;
		zPalmTrees = -zPalmTrees;
		displayTrees(xPalmTrees, zPalmTrees);
	}
	//printf("x model: %f\n", xModel);
	
	/*detect crashed*/
	getCrashed(xModel, xObj1, zModel, zObj1, 2.0f, 3.5f);

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
			zModel += 0.7f;
		}
		break;
	case 'd':
		if (-5 <= zModel) {
			zModel -= 0.7f;
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

	car = loadModel3D("model/car2.obj", "model/suzanne.bmp");
	bus = loadModel3D("model/van.obj", "model/suzanne.bmp");
	road = loadModel3D("model/old road.obj", "model/suzanne.bmp");
	tree = loadModel3D("model/Tree.obj", "model/suzanne.bmp");
	stopSign = loadModel3D("model/stopSign.obj", "model/suzanne.bmp");
	lightSign = loadModel3D("model/lightSign.obj", "model/suzanne.bmp");
	watchTower = loadModel3D("model/watchTower.obj", "model/suzanne.bmp");
	palmTree = loadModel3D("model/palmTree.obj", "model/suzanne.bmp");

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

