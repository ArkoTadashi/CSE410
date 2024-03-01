#include <iostream>
#include <iomanip>
#include <fstream>
#include <stack>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include "bitmap.hpp"
#include "1905109_classes.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#elif __linux__
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#elif WIN32
#include <windows.h>
#include <glut.h>
#endif


#define pi (2*acos(0.0))
#define dim 4L
#define GAP ' '

using namespace std;


bool grid;
bool axes;
double angle;
double cameraHeight;
double cameraAngle;


int recLevel;
int imageHeight, imageWidth;
bitmap_image img;

vector <Object*> objects;
vector <PointLight*> pointLights;
vector <SpotLight*> spotLights;

void loadData() {
	ifstream in("scene.txt");
	in >> recLevel >> imageHeight;

	imageWidth = imageHeight;

	int objCount;
	in >> objCount;

	for(int i = 0; i < objCount; i++) {
		string objType;
		in >> objType;

		Object *obj;

		if(objType == "sphere") {
			obj = new Sphere();
			in >> *((Sphere *)obj);
		}
		else if(objType == "triangle") {
			obj = new Triangle();
			in >> *((Triangle *)obj);
		}
		else if(objType == "general") {
			obj = new Quadratic();
			in >> *((Quadratic *)obj);
		}
		objects.push_back(obj);
	}

	int lightCount;
	in >> lightCount;

	for(int i=0;i<lightCount;i++) {
		PointLight *light = new PointLight();
		in >> *light;
		pointLights.push_back(light);
	}

	int spotlightCount;
	in >> spotlightCount;

	for(int i = 0; i < spotlightCount; i++) {
		SpotLight *spotlight = new SpotLight();
		in >> *spotlight;
		spotLights.push_back(spotlight);
	}


	Object *floor;
	floor = new Floor(400, 10);
	floor->setColor(Color(0.5, 0.5, 0.5));
	vector <double> coefficients;
	coefficients.push_back(0.4);
	coefficients.push_back(0.2);
	coefficients.push_back(0.2);
	coefficients.push_back(0.2);
	floor->setCoefficients(coefficients);
	objects.push_back(floor);
}

int imageCount = 1;
Point cam(150, 0, 10);

Point up(0, 0, 1);
Point rig(-1 / sqrt(2), 1 / sqrt(2), 0);
Point look(-1 / sqrt(2), -1 / sqrt(2), 0);

int seg;
double rotAngle = pi/180;

double getDegree(double angle) {
	return angle*pi/180;
}

void drawAxes() {
	if(axes == 1) {
		glBegin(GL_LINES); {
			glColor3f(1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(100, 0, 0);

			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 100, 0);

			glColor3f(0, 0, 1);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 100);
		}
		glEnd();
	}
}

void drawGrid() {
	if(grid == 1) {
		glColor3f(0.6, 0.6, 0.6);
		glBegin(GL_LINES); {
			for(int i = -8; i <= 8; i++) {

				if(i == 0) continue;

				glVertex3f(i*10, -90, 0);
				glVertex3f(i*10, 90, 0);
				glVertex3f(-90, i*10, 0);
				glVertex3f(90, i*10, 0);
			}
		}
		glEnd();
	}
}

void rodriguez(Point &p, Point &axis, double ang) {
	p = p*cos(ang)+(axis^p)*sin(ang);
}

double windowWidth = 720, windowHeight = 600;
double viewAngle = 80;

void capture() {
	cout << "Capturing Image" << endl;
	for(int i = 0; i < imageWidth; i++) {
		for(int j = 0; j < imageHeight; j++) {
			img.set_pixel(i, j, 0, 0, 0);
        }
    }

	double planeDistance = (windowHeight / 2.0) / tan(getDegree(viewAngle/2.0));

	Point topLeft = cam + (look * planeDistance) + (up * (windowHeight / 2.0)) - (rig * (windowWidth / 2.0));

	double du = windowWidth / (imageWidth*1.0);
	double dv = windowHeight / (imageHeight*1.0);
	topLeft = topLeft + (rig * du / 2.0) - (up * dv / 2.0);

	int nearIndex = -1;
	double t, tMin;

	for(int i = 0; i < imageWidth; i++) {
		for(int j = 0; j < imageHeight; j++) {
			Point pixel = topLeft + (rig * du * i) - (up * dv * j);

			Ray ray(cam, pixel-cam);
			Color color;
			tMin = -1;
			nearIndex = -1;
			for(int k = 0; k < (int)objects.size(); k++) {
				t = objects[k]->intersect(ray,color, 0);
				if(t>0 && (nearIndex == -1 || t<tMin) )
					tMin = t , nearIndex = k;
			}

			if(nearIndex != -1) {
				color = Color(0,0,0);
				double t = objects[nearIndex]->intersect(ray, color, 1);

				if(color.r > 1) color.r = 1;
				if(color.g > 1) color.g = 1;
				if(color.b > 1) color.b = 1;

				if(color.r < 0) color.r = 0;
				if(color.g < 0) color.g = 0;
				if(color.b < 0) color.b = 0;
				
				img.set_pixel(i, j, 255*color.r, 255*color.g, 255*color.b);
			}
		}
	}

	img.save_image("Output_"+to_string(imageCount)+".bmp");
	imageCount++;
	cout << "Image Saved" << endl;		
}

void keyboardListener(unsigned char key, int x, int y) {
	switch(key) {
		case '0':
			capture();
			break;
		case '1':
			rodriguez(rig, up, rotAngle);
			rodriguez(look, up, rotAngle);
			break;
		case '2':
			rodriguez(rig, up, -rotAngle);
			rodriguez(look, up, -rotAngle);
			break;
		case '3':
			rodriguez(up, rig, rotAngle);
			rodriguez(look, rig, rotAngle);
			break;
		case '4':
			rodriguez(up, rig, -rotAngle);
			rodriguez(look, rig, -rotAngle);
			break;
		case '5':
			rodriguez(rig, look, rotAngle);
			rodriguez(up, look, rotAngle);
			break;
		case '6':
			rodriguez(rig, look, -rotAngle);
			rodriguez(up, look, -rotAngle);
			break;
		default:
			break;
	}
}


void specialKeyListener(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_DOWN:
			cam = cam - look * 2;
			break;
		case GLUT_KEY_UP:
			cam = cam + look * 2;
			break;
		case GLUT_KEY_RIGHT:
			cam = cam + rig * 2;
			break;
		case GLUT_KEY_LEFT:
			cam = cam - rig * 2;
			break;
		case GLUT_KEY_PAGE_UP:
			cam = cam + up * 2;
			break;
		case GLUT_KEY_PAGE_DOWN:
			cam = cam - up * 2;
			break;
		default:
			break;
	}
}


void mouseListener(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:
			if(state == GLUT_DOWN) {
				axes = !axes;
			}
			break;
		default:
			break;
	}
}



void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(cam.x, cam.y, cam.z, 
			cam.x + look.x, cam.y + look.y, cam.z + look.z, 
			up.x, up.y, up.z);


	glMatrixMode(GL_MODELVIEW);


	drawAxes();
	drawGrid();

    for (int i = 0; i < objects.size(); i++) objects[i]->draw();
	for (int i = 0; i < pointLights.size(); i++) pointLights[i]->draw();
	for(int i = 0; i < spotLights.size(); i++) spotLights[i]->draw();
	

	glutSwapBuffers();
}


void animate() {
	glutPostRedisplay();
}



void initGL() {
	grid = true;
	axes = true;
	cameraHeight = 150.0;
	cameraAngle = 1.0;
	angle = 0;
	seg = 36;

	loadData();
	img = bitmap_image(imageWidth, imageHeight);

	glClearColor(0, 0, 0, 0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(80, 1, 1, 1000);
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(720, 600);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("1905109");
	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutIdleFunc(animate);

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);
	glutMouseFunc(mouseListener);

	initGL();
	glutMainLoop();

	objects.clear();
	objects.shrink_to_fit();
	pointLights.clear();
	pointLights.shrink_to_fit();
	spotLights.clear();
	spotLights.shrink_to_fit();

	return 0;
}
