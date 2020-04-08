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
float deltaMove = 0.0f;
//x, y, z for 3d Model
float xModel = 0.0f;
float yModel = 0.0f;
float zModel = 0.0f;

//x, y, z for cube
float xCube = -20.0f;
float yCube = 0.0f;
float zCube = 0.0f;

//bool to display cube 
bool cubeAppeared = true;

//bool to check crashed
bool isCrashed = false;

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
Model3D cube;
Model3D suzanne;
Model3D canape;
Model3D car;
Model3D table;

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

void drawTriangles()
{	
	glLoadIdentity();									// Reset The Current Modelview Matrix
	//glTranslatef(-1.5f,0.0f,-6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
	//glRotatef(rtri,0.0f,1.0f,0.0f);

	//glLoadIdentity();									// Reset The Current Modelview Matrix
	//glTranslatef(-1.5f,0.0f,-6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
	//glRotatef(rtri,0.0f,1.0f,0.0f);						// Rotate The Triangle On The Y axis ( NEW )
	//rtri+=0.02f;											// Increase The Rotation Variable For The Triangle ( NEW )
	//cout << "Rotate angle " << rtri<< endl;

	//glBegin(GL_TRIANGLES);								// Start Drawing A Triangle
	//	glColor3f(1.0f,0.0f,0.0f);						// Red
	//	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Front)
	//	glColor3f(0.0f,1.0f,0.0f);						// Green
	//	glVertex3f(-1.0f,-1.0f, 1.0f);					// Left Of Triangle (Front)
	//	glColor3f(0.0f,0.0f,1.0f);						// Blue
	//	glVertex3f( 1.0f,-1.0f, 1.0f);					// Right Of Triangle (Front)
	//	glColor3f(1.0f,0.0f,0.0f);						// Red
	//	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Right)
	//	glColor3f(0.0f,0.0f,1.0f);						// Blue
	//	glVertex3f( 1.0f,-1.0f, 1.0f);					// Left Of Triangle (Right)
	//	glColor3f(0.0f,1.0f,0.0f);						// Green
	//	glVertex3f( 1.0f,-1.0f, -1.0f);					// Right Of Triangle (Right)
	//	glColor3f(1.0f,0.0f,0.0f);						// Red
	//	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Back)
	//	glColor3f(0.0f,1.0f,0.0f);						// Green
	//	glVertex3f( 1.0f,-1.0f, -1.0f);					// Left Of Triangle (Back)
	//	glColor3f(0.0f,0.0f,1.0f);						// Blue
	//	glVertex3f(-1.0f,-1.0f, -1.0f);					// Right Of Triangle (Back)
	//	glColor3f(1.0f,0.0f,0.0f);						// Red
	//	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Left)
	//	glColor3f(0.0f,0.0f,1.0f);						// Blue
	//	glVertex3f(-1.0f,-1.0f,-1.0f);					// Left Of Triangle (Left)
	//	glColor3f(0.0f,1.0f,0.0f);						// Green
	//	glVertex3f(-1.0f,-1.0f, 1.0f);					// Right Of Triangle (Left)
	//glEnd();
}


void drawTeapot()
{
	//glDepthFunc(GL_ALWAYS);     // to avoid visual artifacts with grid lines
	//glDisable(GL_LIGHTING);
	glPushMatrix();             //NOTE: There is a bug on Mac misbehaviours of

	glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();		
	// Reset The Current Modelview Matrix
	
	glTranslatef(0.0f, 0.0f, .0f);
	glColor3f(0.0, 1.0, 0.0);
	glutWireTeapot(2.0);
	//glutWireTeapot(2.0);
	//glutSolidSphere(1, 8, 8);
	//glColor3f(1.0, 1.0, 0.0);
	//glutWireTeapot(2.0);
	glEnd();

	// restore default settings
	glPopMatrix();
	//glEnable(GL_LIGHTING);
	//glDepthFunc(GL_LEQUAL);
}

void drawCube(float x, float y, float z)
{
	glPushMatrix();
	//glLoadIdentity();									// Reset The Current Modelview Matrix
	glTranslatef(x, y, z);
	// Move Right 1.5 Units And Into The Screen 7.0
	/*glRotatef(randomPos(45, 90),0,0,1.0f);		*/			// Rotate The Quad On The X axis ( NEW )
	//rquad-=0.015f;										// Decrease The Rotation Variable For The Quad ( NEW )
	glBegin(GL_QUADS);									// Draw A Quad
	glColor3f(0.0f, 1.0f, 0.0f);						// Set The Color To Green
	glVertex3f(1.0f, 1.0f, -1.0f);					// Top Right Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f, -1.0f);					// Top Left Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f, 1.0f);					// Bottom Left Of The Quad (Top)
	glVertex3f(1.0f, 1.0f, 1.0f);					// Bottom Right Of The Quad (Top)
	glColor3f(1.0f, 0.5f, 0.0f);						// Set The Color To Orange
	glVertex3f(1.0f, -1.0f, 1.0f);					// Top Right Of The Quad (Bottom)
	glVertex3f(-1.0f, -1.0f, 1.0f);					// Top Left Of The Quad (Bottom)
	glVertex3f(-1.0f, -1.0f, -1.0f);					// Bottom Left Of The Quad (Bottom)
	glVertex3f(1.0f, -1.0f, -1.0f);					// Bottom Right Of The Quad (Bottom)
	glColor3f(1.0f, 0.0f, 0.0f);						// Set The Color To Red
	glVertex3f(1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Front)
	glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Front)
	glVertex3f(-1.0f, -1.0f, 1.0f);					// Bottom Left Of The Quad (Front)
	glVertex3f(1.0f, -1.0f, 1.0f);					// Bottom Right Of The Quad (Front)
	glColor3f(1.0f, 1.0f, 0.0f);						// Set The Color To Yellow
	glVertex3f(1.0f, -1.0f, -1.0f);					// Top Right Of The Quad (Back)
	glVertex3f(-1.0f, -1.0f, -1.0f);					// Top Left Of The Quad (Back)
	glVertex3f(-1.0f, 1.0f, -1.0f);					// Bottom Left Of The Quad (Back)
	glVertex3f(1.0f, 1.0f, -1.0f);					// Bottom Right Of The Quad (Back)
	glColor3f(0.0f, 0.0f, 1.0f);						// Set The Color To Blue
	glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Left)
	glVertex3f(-1.0f, 1.0f, -1.0f);					// Top Left Of The Quad (Left)
	glVertex3f(-1.0f, -1.0f, -1.0f);					// Bottom Left Of The Quad (Left)
	glVertex3f(-1.0f, -1.0f, 1.0f);					// Bottom Right Of The Quad (Left)
	glColor3f(1.0f, 0.0f, 1.0f);						// Set The Color To Violet
	glVertex3f(1.0f, 1.0f, -1.0f);					// Top Right Of The Quad (Right)
	glVertex3f(1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Right)
	glVertex3f(1.0f, -1.0f, 1.0f);					// Bottom Left Of The Quad (Right)
	glVertex3f(1.0f, -1.0f, -1.0f);					// Bottom Right Of The Quad (Right)
	glEnd();
	glPopMatrix();
	// Done Drawing The Quad										// Done Drawing The Quad
}

void drawTriangle() {
	//glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_TRIANGLES);
	glVertex3f(-0.5, -0.5, 0.0);
	glVertex3f(0.5, 0.0, 0.0);
	glVertex3f(0.0, 0.5, 0.0);
	glEnd();
}

void drawAxis(float size)
{

	glm::mat4 ModelMatrix = glm::mat4(1.0);
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

	glLineWidth(3);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(size, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, size, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, size);
	glEnd();
	glLineWidth(1);

	// draw arrows(actually big square dots)
	glPointSize(5);
	glBegin(GL_POINTS);
	glColor3f(1, 0, 0);
	glVertex3f(size, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, size, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, size);
	glEnd();
	glPointSize(1);
}

void drawAxes() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1.0, 0.0, 0.0); // red x
	glBegin(GL_LINES);
	// x aix

	glVertex3f(-4.0, 0.0f, 0.0f);
	glVertex3f(4.0, 0.0f, 0.0f);

	// arrow
	glVertex3f(4.0, 0.0f, 0.0f);
	glVertex3f(3.0, 1.0f, 0.0f);

	glVertex3f(4.0, 0.0f, 0.0f);
	glVertex3f(3.0, -1.0f, 0.0f);
	glEnd();
	glFlush();

	// y 
	glColor3f(0.0, 1.0, 0.0); // green y
	glBegin(GL_LINES);
	glVertex3f(0.0, -4.0f, 0.0f);
	glVertex3f(0.0, 4.0f, 0.0f);

	// arrow
	glVertex3f(0.0, 4.0f, 0.0f);
	glVertex3f(1.0, 3.0f, 0.0f);

	glVertex3f(0.0, 4.0f, 0.0f);
	glVertex3f(-1.0, 3.0f, 0.0f);
	glEnd();
	glFlush();

	// z 
	glColor3f(0.0, 0.0, 1.0); // blue z
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0f, -4.0f);
	glVertex3f(0.0, 0.0f, 4.0f);

	// arrow
	//glVertex3f(0.0, 0.0f, 4.0f);
	//glVertex3f(0.0, 1.0f, 3.0f);

	//glVertex3f(0.0, 0.0f, 4.0f);
	//glVertex3f(0.0, -1.0f, 3.0f);
	glEnd();
	glFlush();
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

void displayModel() {
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	if (deltaMove > 0) {
		xModel -= deltaMove / 10;
	}

	std::cout << "x pos: \n" << xModel;
	//std::cout << glm::to_string(ModelMatrix) << std::endl;
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(xModel, yModel, zModel));
	draw3DModel(cube, ModelMatrix, GL_TRIANGLES);
}

void displayCube() {
	if (cubeAppeared) {
		drawCube(xCube, yCube, zCube);
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
	drawTeapot();
	ViewMatrix = glm::lookAt(eye, eye + glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	drawAxes();

	glm::mat4 ModelMatrix2 = glm::mat4(1.0);
	glm::vec3 modelScale = glm::vec3(0.1f, 0.1f, 0.1f);
	ModelMatrix2 = glm::scale(ModelMatrix2, modelScale);
	ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(-2.0f, 0.0f, 0.0f));
	draw3DModel(car, ModelMatrix2, GL_TRIANGLES);

	//glm::mat4 ModelMatrix3 = glm::mat4(1.0);
	//ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(4.0f, 0.0f, 0.0f));
	//draw3DModel(car, ModelMatrix3, GL_TRIANGLES);

	/*glm::vec3 modelScale = glm::vec3(0.1f, 0.1f, 0.1f);
	glm::mat4 ModelMatrix3 = glm::mat4(1.0);
	ModelMatrix3 = glm::translate(ModelMatrix3, glm::vec3(4.0f, 3.0f, 0.0f));
	ModelMatrix3 = glm::scale(ModelMatrix3, modelScale);
	draw3DModel(canape, ModelMatrix3, GL_TRIANGLES);*/
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

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0,1.0,1.0,1.0);
	glEnable(GL_CULL_FACE);	

	glewInit();

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	cube = loadModel3D("cube.obj", "uvtemplate.bmp");
	suzanne = loadModel3D("suzanna2.obj", "suzanne.bmp");
	canape = loadModel3D("table.obj", "Canape.bmp");
	car = loadModel3D("carModel.obj", "suzanne.bmp");
	table = loadModel3D("table.obj", "suzanne.bmp");

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

