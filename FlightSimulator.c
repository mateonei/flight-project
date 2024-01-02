/************************************************************
*
* Mateo Neira
* B00836322
*
* CSCI 3161 - Final project, Flight Simulator
*
************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <freeglut.h>
#include "array_list.h"

// the ratio of the circumference to the diameter of a circle  
#define PI 3.14159265

// conversion multiplier for converting from degrees to Radians
#define DEG_TO_RAD PI/180.0

#define GRID_SIZE 60
#define MOUNT_SIZE 20

#define NUM_VERTEX_CESSNA 6763
#define NUM_SUBOBJECTS_CESSNA 33

#define NUM_VERTEX_PROPELLER 6763
#define NUM_SUBOBJECTS_PROPELLER 2

#define SEA_IMAGE_WIDTH 1600
#define SEA_IMAGE_HEIGHT 1200

#define SKY_IMAGE_WIDTH 896
#define SKY_IMAGE_HEIGHT 385

#define MOUNT_IMAGE_WIDTH 1280
#define MOUNT_IMAGE_HEIGHT 1104

#define MOUNT_DIVISIONS 5

typedef GLdouble point3d[3];
typedef GLfloat point3f[3];
typedef GLfloat rgba4f[4];

typedef struct _Vector3 {
	GLfloat x;
	GLfloat y;
	GLfloat z;
}Vector3 ;

// window dimensions
int originalWidth = 1000;
int originalHeight = 600;

// variable dimensions through runtime
int width = 1000;
int height = 600;

// initial grid
point3f gridLimits[4] = { {-GRID_SIZE, 0, -GRID_SIZE}, {GRID_SIZE, 0, -GRID_SIZE},  {GRID_SIZE, 0, GRID_SIZE}, {-GRID_SIZE, 0, GRID_SIZE}};

GLUquadric* sphere = NULL;
GLUquadric* disk = NULL;
GLUquadric* cylinder = NULL;

// The next set of global arrays will hold the data read from text files
point3d cessnaVertex[NUM_VERTEX_CESSNA];
point3d cessnaNormals[NUM_VERTEX_CESSNA];
ArrayList* cessnaSubobjects[NUM_SUBOBJECTS_CESSNA];
rgba4f cessnaColors[NUM_SUBOBJECTS_CESSNA];

point3d propellerVertex[NUM_VERTEX_PROPELLER];
point3d propellerNormals[NUM_VERTEX_PROPELLER];
ArrayList* propellerSubobjects[NUM_SUBOBJECTS_PROPELLER];
rgba4f propellerColors[NUM_SUBOBJECTS_PROPELLER];

rgba4f fogColor = { 1, 0.753, 0.796, 0.25 };
GLfloat fogDensity = 0.01;

// texture images in memory
GLubyte seaImage[SEA_IMAGE_WIDTH][SEA_IMAGE_HEIGHT][3];
int seaImageID;

GLubyte skyImage[SKY_IMAGE_WIDTH][SKY_IMAGE_HEIGHT][3];
int skyImageID;

GLubyte mountImage[MOUNT_IMAGE_WIDTH][MOUNT_IMAGE_HEIGHT][3];
int mountImageID;

// mountains limit
point3f pyramid[] = { {0, 15, 0}, {-MOUNT_SIZE, 0, -MOUNT_SIZE}, {0, 0, -MOUNT_SIZE}, {MOUNT_SIZE, 0, -MOUNT_SIZE}, {MOUNT_SIZE, 0, 0}, {MOUNT_SIZE, 0, MOUNT_SIZE}, {0, 0, MOUNT_SIZE}, {-MOUNT_SIZE, 0, MOUNT_SIZE}, {-MOUNT_SIZE, 0, 0} };
ArrayList* mountVertexes1 = NULL;
ArrayList* mountVertexes2 = NULL;
ArrayList* mountVertexes3 = NULL;

bool wireframe = false;
bool fullscreen = false;
bool grid = true;
bool texture = true;
bool fog = false;
bool mountains = false;

point3f cameraPosition = { 0, 3, 15 };
GLfloat cameraRotation = 0;
Vector3 cameraDirection;

GLfloat lightPosition[] = { 25, 150, 0, 0 };

// define the light color and intensity
GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat specularLight[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat ambientLight[] = { 0.0, 0.0, 0.0, 1.0 };

//  the global ambient light level
GLfloat globalAmbientLight[] = { 0.4, 0.4, 0.4, 1.0 };

// a material that is all zeros
GLfloat zeroMaterial[] = { 0.0, 0.0, 0.0, 1.0 };

// a white specular material
GLfloat whiteSpecular[] = { 1.0, 1.0, 1.0, 1.0 };

// shininess plane
GLfloat shininess = 100.0;

GLfloat whiteEmission[] = { 1, 1, 1, 1.0 };
GLfloat noEmission[] = { 0, 0, 0, 1.0 };

// plane movement
point3f cessnaPosition = { 0, -0.75, -2.5 };
point3f cessnaRotation = { 0, -90, 0 };

point3f propellerPosition = { -0.55, -0.15, 0.35 };
GLfloat propellerRotation = 0;

GLfloat speed = 0.01;

bool isRight;
bool isLeft;
GLfloat rightPercent = 0;
GLfloat leftPercent = 0;
GLfloat rotationSensitivity = 0.5;

// Generates a random number in the range; (-range, range)
GLfloat getRandNumber(int range) {
	GLfloat randNum = (GLfloat)(rand() % (2 * range));
	return randNum > range ? (GLfloat)(range - randNum) : randNum;
}

// for loading ppm image data to global arrays, adapted from professors example, displayPPM.c
void loadImage(char* fileName) {
	// the ID of the image file
	FILE* fileID;

	// maxValue
	int  maxValue;

	// temporary character
	char tempChar;

	// array for reading in header information
	char headerLine[100];

	// temporary variables for reading in the red, green and blue data of each pixel
	int red, green, blue;

	int imageWidth, imageHeight;

	// open the image file for reading
	fileID = fopen(fileName, "r");
	if (fileID == NULL) {
		printf("Image file could not be read\n");
		return;
	}

	// read in the first header line
	//    - "%[^\n]"  matches a string of all characters not equal to the new line character ('\n')
	//    - so we are just reading everything up to the first line break
	fscanf(fileID, "%[^\n] ", headerLine);

	// make sure that the image begins with 'P3', which signifies a PPM file
	if ((headerLine[0] != 'P') || (headerLine[1] != '3'))
	{
		printf("This is not a PPM file!\n");
		exit(0);
	}

	// we have a PPM file
	printf("This is a PPM file\n");

	// read in the first character of the next line
	fscanf(fileID, "%c", &tempChar);

	// while we still have comment lines (which begin with #)
	while (tempChar == '#')
	{
		// read in the comment
		fscanf(fileID, "%[^\n] ", headerLine);

		// print the comment
		printf("%s\n", headerLine);

		// read in the first character of the next line
		fscanf(fileID, "%c", &tempChar);
	}

	// the last one was not a comment character '#', so we need to put it back into the file stream (undo)
	ungetc(tempChar, fileID);

	// read in the image hieght, width and the maximum value
	fscanf(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

	// print out the information about the image file
	printf("%d rows  %d columns  max value= %d\n", imageHeight, imageWidth, maxValue);

	// if the maxValue is 255 then we do not need to scale the 
	//    image data values to be in the range or 0 to 255
	for (int i = imageWidth - 1; i >= 0; i--) {
		for (int j = imageHeight - 1; j >= 0; j--) {
			// read in the current pixel from the file
			fscanf(fileID, "%d %d %d", &red, &green, &blue);

			// store the red, green and blue data of the current pixel in the data array
			if (imageWidth == 1600) {
				seaImage[i][j][0] = (GLubyte)red;
				seaImage[i][j][1] = (GLubyte)green;
				seaImage[i][j][2] = (GLubyte)blue;
			}
			else if (imageWidth == 896) {
				skyImage[i][j][0] = (GLubyte)red;
				skyImage[i][j][1] = (GLubyte)green;
				skyImage[i][j][2] = (GLubyte)blue;
			}
			else {
				mountImage[i][j][0] = (GLubyte)red;
				mountImage[i][j][1] = (GLubyte)green;
				mountImage[i][j][2] = (GLubyte)blue;
			}
		}
	}


	// close the image file
	fclose(fileID);
}

void fillColors(void) {
	int i = 0;
	for (; i < 4; i++) {
		cessnaColors[i][0] = 1;
		cessnaColors[i][1] = 1;
		cessnaColors[i][2] = 0;
		cessnaColors[i][3] = 1;
	}

	for (; i < 6; i++) {
		cessnaColors[i][0] = 0;
		cessnaColors[i][1] = 0;
		cessnaColors[i][2] = 0;
		cessnaColors[i][3] = 1;
	}

	cessnaColors[++i][0] = 0.796078431;
	cessnaColors[i][1] = 0.76470588;
	cessnaColors[i][2] = 0.89019608;
	cessnaColors[i][3] = 1;

	cessnaColors[++i][0] = 0;
	cessnaColors[i][1] = 0;
	cessnaColors[i][2] = 1;
	cessnaColors[i][3] = 1;

	for (; i < 14; i++) {
		cessnaColors[i][0] = 1;
		cessnaColors[i][1] = 1;
		cessnaColors[i][2] = 0;
		cessnaColors[i][3] = 1;
	}

	for (; i < 26; i++) {
		cessnaColors[i][0] = 0;
		cessnaColors[i][1] = 0;
		cessnaColors[i][2] = 1;
		cessnaColors[i][3] = 1;
	}

	for (; i < 33; i++) {
		cessnaColors[i][0] = 1;
		cessnaColors[i][1] = 1;
		cessnaColors[i][2] = 0;
		cessnaColors[i][3] = 1;
	}

	propellerColors[0][0] = 1;
	propellerColors[0][1] = 1;
	propellerColors[0][2] = 1;
	propellerColors[0][3] = 1;

	propellerColors[1][0] = 1;
	propellerColors[1][1] = 0;
	propellerColors[1][2] = 0;
	propellerColors[1][3] = 1;
}

// reads cessna subobjects from file and stores them in arraylist
void readSubojects(FILE* file, int numSubojects, ArrayList* subobjects[]) {
	char* buffer = malloc(256);
	const char delim[3] = "  ";

	fscanf(file, "\n");
	for (int i = 0; i < numSubojects; i++) {
		subobjects[i] = alist_initialize(16, sizeof(ArrayList), "ArrayList");

		fgets(buffer, 256, file);
		fgets(buffer, 256, file);
		while (buffer[0] == 'f') {
			buffer[0] = '0';
			buffer[1] = '0';
			buffer[strlen(buffer) - 1] = '\0';

			ArrayList* faceIndxs = alist_initialize(24, sizeof(int), "int");
			char* faceIndxStr = strtok(buffer, delim);
			while (faceIndxStr != NULL) {
				int index = atoi(faceIndxStr) - 1;
				alist_add(faceIndxs, &index);
				faceIndxStr = strtok(NULL, delim);
			}
			alist_add(subobjects[i], faceIndxs);

			fgets(buffer, 256, file);
		}
	}

	free(buffer);
}

// reads cessna vertexes and indexes and stores them in arraylists
void readModel(char* fileName, int numVertex, int numSubojects, point3d vertexArr[], point3d normalArr[], ArrayList* subobjects[]) {
	FILE* file = fopen(fileName, "r");
	if (file == NULL) {
		printf("Model text file could not be read\n");
		return;
	}

	GLdouble x;
	GLdouble y;
	GLdouble z;

	// reads vertexes
	for (int i = 0; i < numVertex; i++) {
		fscanf(file, "%*c %lf %lf %lf\n", &x, &y, &z);
		vertexArr[i][0] = x;
		vertexArr[i][1] = y;
		vertexArr[i][2] = z;
	}

	// reads normals
	for (int i = 0; i < numVertex; i++) {
		fscanf(file, "%*c %lf %lf %lf\n", &x, &y, &z);
		normalArr[i][0] = x;
		normalArr[i][1] = y;
		normalArr[i][2] = z;
	}

	readSubojects(file, numSubojects, subobjects);

	fclose(file);
}

// normalizes a vector
void normalize(Vector3* vector) {
	GLfloat length = sqrt(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z);

	vector->x /= length;
	vector->y /= length;
	vector->z /= length;
}

// computes cross-product u x v
GLfloat* crossProduct(point3f u, point3f v) {
	point3f product;

	product[0] = u[1] * v[2] - u[2] * v[1];
	product[1] = u[2] * v[0] - u[0] * v[2];
	product[2] = u[0] * v[1] - u[1] * v[0];
	
	return product;
}

// computes the vertex normals
GLfloat* computeVertexNormal(point3f vertex, point3f ref1, point3f ref2) {
	point3f u;
	point3f v;

	for (int i = 0; i < 3; i++) {
		u[i] = ref1[i] - vertex[i];
		v[i] = ref2[i] - vertex[i];
	}
	
	return crossProduct(u, v);
}

// parameterizes texture coordinates based on a given polygon vertex
GLfloat texParameter(GLfloat polygonVertex) {
	GLfloat texParameter = 0.0625 * polygonVertex;
	texParameter /= 2.5;
	return texParameter;
}

// draws a square
void drawSquare(point3f topLeft, point3f topRight, point3f bottomRight, point3f bottomLeft) {
	glBegin(GL_POLYGON);
	glNormal3fv(computeVertexNormal(topLeft, bottomRight, topRight));
	glTexCoord2f(texParameter(topLeft[0] + 20), texParameter(topLeft[1] + 20));
	glVertex3fv(topLeft);

	glNormal3fv(computeVertexNormal(topRight, topLeft, bottomRight));
	glTexCoord2f(texParameter(topRight[0] + 20), texParameter(topRight[1] + 20));
	glVertex3fv(topRight);

	glNormal3fv(computeVertexNormal(bottomRight, topRight, bottomLeft));
	glTexCoord2f(texParameter(bottomRight[0] + 20), texParameter(bottomRight[1] + 20));
	glVertex3fv(bottomRight);

	glNormal3fv(computeVertexNormal(bottomLeft, bottomRight, topLeft));
	glTexCoord2f(texParameter(bottomLeft[0] + 20), texParameter(bottomLeft[1] + 20));
	glVertex3fv(bottomLeft);
	glEnd();
}

// draws a grid
void drawGrid(point3f a, point3f b, point3f c, point3f d, int divisions) {
	point3f m0, m1, m2, m3, m4;
	if (divisions > 0) {
		for (int i = 0; i < 3; i++) { m0[i] = (a[i] + b[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m1[i] = (a[i] + d[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m2[i] = (a[i] + c[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m3[i] = (b[i] + c[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m4[i] = (d[i] + c[i]) / 2.0; }

		drawGrid(a, m0, m2, m1, divisions - 1);
		drawGrid(m0, b, m3, m2, divisions - 1);
		drawGrid(m2, m3, c, m4, divisions - 1);
		drawGrid(m1, m2, m4, d, divisions - 1);
	}
	else {
		drawSquare(a, b, c, d);
	}
}

// divides mountain levels and adds random moun to midpoints
void divideSquare(point3f a, point3f b, point3f c, point3f d, int level, GLfloat randAmount, ArrayList* mountVertexes) {
	point3f m0, m1, m2, m3, m4;
	int range = 20;
	if (level > 0) {
		for (int i = 0; i < 3; i++) { m0[i] = (a[i] + b[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m1[i] = (a[i] + d[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m2[i] = (a[i] + c[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m3[i] = (b[i] + c[i]) / 2.0; }
		for (int i = 0; i < 3; i++) { m4[i] = (d[i] + c[i]) / 2.0; }

		m2[1] += getRandNumber(range) / randAmount;

		divideSquare(a, m0, m2, m1, level - 1, randAmount * 2, mountVertexes);
		divideSquare(m0, b, m3, m2, level - 1, randAmount * 2, mountVertexes);
		divideSquare(m2, m3, c, m4, level - 1, randAmount * 2, mountVertexes);
		divideSquare(m1, m2, m4, d, level - 1, randAmount * 2, mountVertexes);
	}
	else {
		alist_add(mountVertexes, a);
		alist_add(mountVertexes, b);
		alist_add(mountVertexes, c);
		alist_add(mountVertexes, d);
	}
}

// generates a mountain and stores it into arraylist
void generateMount(int level, ArrayList* mountVertexes) {
	divideSquare(pyramid[1], pyramid[2], pyramid[0], pyramid[8], level, 2, mountVertexes);
	divideSquare(pyramid[2], pyramid[3], pyramid[4], pyramid[0], level, 2, mountVertexes);
	divideSquare(pyramid[0], pyramid[4], pyramid[5], pyramid[6], level, 2, mountVertexes);
	divideSquare(pyramid[8], pyramid[0], pyramid[6], pyramid[7], level, 2, mountVertexes);
}

// draws a mountain stored in arraylist
void drawMount(ArrayList* mountVertexes) {
	for (int i = 0; i < mountVertexes->size; i++) {
		GLfloat* a = alist_get(mountVertexes, i);
		GLfloat* b = alist_get(mountVertexes, ++i);
		GLfloat* c = alist_get(mountVertexes, ++i);
		GLfloat* d = alist_get(mountVertexes, ++i);
		drawSquare(a, b, c, d);
	}
}

// draws 3 mountains in scene with textures if enabled
void drawMountains(void) {
	if (texture) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mountImageID);
	}
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseLight);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientLight);
	glMaterialf(GL_FRONT, GL_SHININESS, 5);
	glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);

	glPushMatrix();
	glTranslatef(100, -2.5, -50);
	glScalef(2.5, 1, 2);
	drawMount(mountVertexes1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-50, -7.5, -100);
	glScalef(3.5, 1.5, 2);
	drawMount(mountVertexes2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-40, -7.5, 75);
	glScalef(3, 2, 4);
	drawMount(mountVertexes3);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
}

// draws the sky with textures
void drawSky(void) {
	glPushMatrix();
	glTranslatef(0, -5, 0);
	glRotatef(-90, 1, 0, 0);

	glBindTexture(GL_TEXTURE_2D, skyImageID);
	gluQuadricNormals(cylinder, GLU_SMOOTH);
	gluQuadricTexture(cylinder, GL_TRUE);

	gluCylinder(cylinder, 195, 195, 400, 50, 100);
	glPopMatrix();
}

// draws the sea with textures
void drawSea(void) {
	glPushMatrix();
	glRotatef(90, 1, 0, 0);

	glBindTexture(GL_TEXTURE_2D, seaImageID);
	gluQuadricNormals(disk, GLU_SMOOTH);
	gluQuadricTexture(disk, GL_TRUE);

	if (fog) {
		glEnable(GL_FOG);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, fogDensity);
	}

	gluDisk(disk, 0, 200, 50, 25);

	glDisable(GL_FOG);

	glPopMatrix();
}

// draws the sea and the sky with textures, wrapper for above functions
void drawEnvironment(void) {
	glEnable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT, GL_EMISSION, whiteEmission);

	drawSea();
	drawSky();

	glDisable(GL_TEXTURE_2D);
}

// draws plane propeller
void drawPropeller(void) {
	for (int i = 0; i < NUM_SUBOBJECTS_PROPELLER; i++) {
		glMaterialfv(GL_FRONT, GL_DIFFUSE, propellerColors[i]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecular);
		glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);

		for (int j = 0; j < propellerSubobjects[i]->size; j++) {
			ArrayList* facesIndex = alist_get(propellerSubobjects[i], j);

			glPushMatrix();
			glTranslatef(0.5586, 0.15, -0.355); // moving to origin
			glBegin(GL_POLYGON);
			for (int i = 0; i < facesIndex->size; i++) {
				int* index = alist_get(facesIndex, i);
				glNormal3dv(propellerNormals[*index]);
				glVertex3dv(propellerVertex[*index]);
			}
			glEnd();
			glPopMatrix();
		}
	}
}

// draws cessna
void drawCessna(void) {
	for (int i = 0; i < NUM_SUBOBJECTS_CESSNA; i++) {
		glMaterialfv(GL_FRONT, GL_DIFFUSE, cessnaColors[i]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, whiteSpecular);
		glMaterialfv(GL_FRONT, GL_AMBIENT, zeroMaterial);
		glMaterialf(GL_FRONT, GL_SHININESS, shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);

		for (int j = 0; j < cessnaSubobjects[i]->size; j++) {
			ArrayList* facesIndex = alist_get(cessnaSubobjects[i], j);

			glBegin(GL_POLYGON);
			for (int i = 0; i < facesIndex->size; i++) {
				int* index = alist_get(facesIndex, i);
				glNormal3dv(cessnaNormals[*index]);
				glVertex3dv(cessnaVertex[*index]);
			}
			glEnd();
		}
	}
}

// draws the reference at origin
void drawReference(void) {
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseLight);
	glMaterialfv(GL_FRONT, GL_EMISSION, whiteEmission);
	gluSphere(sphere, 0.1, 50, 50);

	glLineWidth(5.0);

	point3f color1 = { 1, 0, 0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, color1);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	glEnd();

	point3f color2 = { 0, 1, 0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, color2);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	glEnd();

	point3f color3 = { 0, 0, 1 };
	glMaterialfv(GL_FRONT, GL_EMISSION, color3);
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();

	glLineWidth(1.0);
}

// rotates camera depending on where mouse is on windows.
// moves camera/plane forward
// rotates propellers
void myIdle(void) {
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (isRight) {
		cameraRotation = cameraRotation >= 360 ? 0 : cameraRotation + rightPercent;
	}
	else if (isLeft) {
		cameraRotation = cameraRotation < 0 ? 360 : cameraRotation - leftPercent;
	}

	cameraDirection.x = -cos(DEG_TO_RAD * (cameraRotation + 90));
	cameraDirection.z = -sin(DEG_TO_RAD * (cameraRotation + 90));
	normalize(&cameraDirection);

	cameraPosition[0] += cameraDirection.x * speed;
	cameraPosition[2] += cameraDirection.z * speed;
	propellerRotation = propellerRotation >= 360 ? 0 : propellerRotation + 9;
	glutPostRedisplay();
}

// Function that draws to display 
void myDisplay(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2], cameraPosition[0] + cameraDirection.x, cameraPosition[1], cameraPosition[2] + cameraDirection.z, 0, 1, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180);

	// cessna
	glPushMatrix();
	glTranslatef(cameraPosition[0], cameraPosition[1], cameraPosition[2]); // camera becomes parent
	glRotatef(-cameraRotation, 0, 1, 0);

	glTranslatef(cessnaPosition[0], cessnaPosition[1], cessnaPosition[2]); // positions cessna relative to camera
	glRotatef(cessnaRotation[1], 0, 1, 0);
	glRotatef(cessnaRotation[0], 1, 0, 0);
	drawCessna();
	glPopMatrix();

	// propellers
	for (int i = 0; i < 2; i++) {
		glPushMatrix();
		glTranslatef(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
		glRotatef(-cameraRotation, 0, 1, 0);
		glTranslatef(cessnaPosition[0], cessnaPosition[1], cessnaPosition[2]);
		glRotatef(cessnaRotation[1], 0, 1, 0);
		glRotatef(cessnaRotation[0], 1, 0, 0);

		glTranslatef(propellerPosition[0], propellerPosition[1], i < 1 ? propellerPosition[2] : -propellerPosition[2]);
		glRotatef(propellerRotation, 1, 0, 0);
		drawPropeller();
		glPopMatrix();
	}

	// environment
	if (grid) {
		drawReference();
		drawGrid(gridLimits[0], gridLimits[1], gridLimits[2], gridLimits[3], 5);
	}
	else {
		drawEnvironment();
	}

	if (mountains) {
		drawMountains();
	}

	glutSwapBuffers();
}

// deallocates memory from subobject arraylists
void destroySubobjectLists(int numSubobjects, ArrayList* faces[]) {
	for (int i = 0; i < numSubobjects; i++) {
		for (int j = 0; j < faces[i]->size; j++) {
			alist_clear(alist_get(faces[i], j));
		}
		alist_destroy(faces[i]);
	}
}

// deallocates memory from arraylists and deletes quadrics
void clean(void) {
	destroySubobjectLists(NUM_SUBOBJECTS_CESSNA, cessnaSubobjects);
	destroySubobjectLists(NUM_SUBOBJECTS_PROPELLER, propellerSubobjects);
	alist_destroy(mountVertexes1);
	alist_destroy(mountVertexes2);
	alist_destroy(mountVertexes3);
	gluDeleteQuadric(sphere);
	gluDeleteQuadric(disk);
	gluDeleteQuadric(cylinder);
}

// handles window resize
void myResize(int newWidth, int newHeight) {
	// update dimensions
	width = newWidth;
	height = newHeight;

	// update viewport
	glViewport(0, 0, width, height);

	// update camera
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60, (double)width / (double)height, 0.1, 300);

	glMatrixMode(GL_MODELVIEW);
}

// handles mouse position on window
void myMouseMotionPass(int x, int y) {
	isRight = x > ((GLfloat)width / 2.0);
	isLeft = x < ((GLfloat)width / 2.0);

	rightPercent = (((GLfloat)x / ((GLfloat)width / 2.0)) - 1.0) * rotationSensitivity;
	leftPercent = (1 - ((GLfloat)x / ((GLfloat)width / 2.0))) * rotationSensitivity;
	
	GLfloat percentageX = (GLfloat)x / (GLfloat)width;
	GLfloat cessnaAngle = (percentageX * 120.0) - 60.0;
	cessnaRotation[0] = -cessnaAngle;
}

// handles keyboard presses behaviour
void myKey(unsigned char key, int x, int y) {
	if (key == 'w' || key == 'W') {
		wireframe = !wireframe;
	}
	else if (key == 'f' || key == 'F') {
		fullscreen = !fullscreen;

		switch (fullscreen) {
		case true:
			originalWidth = width;
			originalHeight = height;
			glutFullScreen();
			break;
		case false:
			glutReshapeWindow(originalWidth, originalHeight);
			break;
		}
	}
	else if (key == 's' || key == 'S') {
		grid = !grid;
	}
	else if (key == 't' || key == 'T') {
		texture = !texture;
	}
	else if (key == 'b' || key == 'B') {
		fog = !fog;
	}
	else if (key == 'm' || key == 'M') {
		mountains = !mountains;
	}
	else if (key == 'q' || key == 'Q') {
		clean();
		exit(0);
	}
}

// handles special keyboard key presses
void mySpecialKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_PAGE_UP:
		speed += 0.001;
		//rotationSensitivity += 0.01;
		break;
	case GLUT_KEY_PAGE_DOWN:
		speed = speed <= 0.005 ? 0.005 : speed - 0.001;
		//rotationSensitivity = rotationSensitivity <= 0.05 ? 0.05 : rotationSensitivity - 0.01;
		break;
	case GLUT_KEY_UP:
		cameraPosition[1] += 0.1;
		break;
	case GLUT_KEY_DOWN:
		cameraPosition[1] -= 0.1;
		break;
	}
}

// Initializing OpenGL window and display
void initializeGL(void) {
	glClearColor(0, 0, 0, 1);

	// set the global ambient light level
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

	// define the color and intensity for light 0
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60, (double)width / (double)height, 0.1, 1000); // using perpective camera
	glMatrixMode(GL_MODELVIEW);

	sphere = gluNewQuadric();
	gluQuadricNormals(sphere, GLU_SMOOTH);

	disk = gluNewQuadric();
	cylinder = gluNewQuadric();

	// setting textures
	glGenTextures(1, &seaImageID);
	glBindTexture(GL_TEXTURE_2D, seaImageID);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, SEA_IMAGE_WIDTH, SEA_IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, seaImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenTextures(1, &skyImageID);
	glBindTexture(GL_TEXTURE_2D, skyImageID);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, SKY_IMAGE_WIDTH, SKY_IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, skyImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenTextures(1, &mountImageID);
	glBindTexture(GL_TEXTURE_2D, mountImageID);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, MOUNT_IMAGE_WIDTH, MOUNT_IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, mountImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

int main(int argc, char** argv) {

	// initialize the toolkit
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(width, height);

	// open the screen window
	glutCreateWindow("FlightSimulator");

	// register redraw function
	glutDisplayFunc(myDisplay);

	// register idle function
	glutIdleFunc(myIdle);

	// register mouse functions
	glutPassiveMotionFunc(myMouseMotionPass);

	// register keyboard function
	glutKeyboardFunc(myKey);
	glutSpecialFunc(mySpecialKey);

	// register the resize function
	glutReshapeFunc(myResize);

	srand(7);

	fillColors();
	readModel("cessna.txt", NUM_VERTEX_CESSNA, NUM_SUBOBJECTS_CESSNA, cessnaVertex, cessnaNormals, cessnaSubobjects);
	readModel("propeller.txt", NUM_VERTEX_PROPELLER, NUM_SUBOBJECTS_PROPELLER, propellerVertex, propellerNormals, propellerSubobjects);

	loadImage("sea02.ppm");
	loadImage("sky08.ppm");
	loadImage("mount03.ppm");

	mountVertexes1 = alist_initialize(1024, sizeof(point3f), "point3f");
	generateMount(5, mountVertexes1);
	mountVertexes2 = alist_initialize(1024, sizeof(point3f), "point3f");
	generateMount(5, mountVertexes2);
	mountVertexes3 = alist_initialize(1024, sizeof(point3f), "point3f");
	generateMount(5, mountVertexes3);
	

	initializeGL();

	// go into a perpetual loop
	glutMainLoop();

	// doesnt execute if closed without pressing 'q', may not deallocate allocated memory
	clean();
	return 0;
}