
#include <iostream>
#include <cstdlib>
#include <cmath>


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

struct Point {
    double x, y, z;

    Point() {
        x = y = z = 0;
    }
    Point(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Point(const Point &p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }

    Point operator +(Point p) {
        return Point(x+p.x, y+p.y, z+p.z);
    }
    Point operator -(Point p) {
        return Point(x-p.x, y-p.y, z-p.z);
    }
    Point operator *(double c) {
        return Point(x*c, y*c, z*c);
    }
    Point operator /(double c) {
        return Point(x/c, y/c, z/c);
    }
    Point operator *(Point p) {
        return Point(y*p.z - z*p.y, z*p.x - x*p.z, x*p.y - y*p.x);
    }
    double operator %(Point p) {
        return (x*p.x + y*p.y + z*p.z);
    }
    
    void setPoint(Point &p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }
    void setPoint(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }


};

double length(Point a) {
    return sqrt(a%a);
}
Point normalizedVector(Point a) {
    return a / length(a);
}




struct Point cam(5, 0, 1);
struct Point look(-1, 0, 0);
struct Point right(0, 1, 0);
struct Point up(0, 0, 1);
double zRot = 45;



void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}



void drawAxes() {
    glLineWidth(5);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(5, 0, 0);

        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 5, 0);

        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, 5);
    glEnd();
}




void drawSquare() {
    glBegin(GL_QUADS);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
    glEnd();
}

void drawCube() {
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glColor3f(0.5f, i%2, (i+1)%2);
            glRotatef(90*i, 0, 1, 0);
            drawSquare();
        glPopMatrix();
    }

    for (int i = 0; i < 2; i++) {
        glPushMatrix();
            glColor3f(1, 0.5f, 0);
            glRotatef(90+180*i, 1, 0, 0);
            drawSquare();
        glPopMatrix();
    }
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(cam.x, cam.y, cam.z,
                cam.x + look.x, cam.y + look.y, cam.z + look.z,
                up.x, up.y, up.z);

    glRotatef(zRot, 0, 0, 1);

    drawAxes();
    drawCube();


    glutSwapBuffers();
}

void reshapeListener(GLsizei width, GLsizei height) {
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80, 1, 1, 1000.0);
}

void keyboardListener(unsigned char key, int x, int y) {
    double v = 0.1;

    switch (key) {
        case '1':
            right = right*cos(v) + look*sin(v);
            look = look*cos(v)-right*sin(v);
            break;
        case '2':
            right = right*cos(-v) + look*sin(-v);
            look = look*cos(-v)-right*sin(-v);
            break;
        case '3':
            look = look*cos(v)+up*sin(v);
            up = up*cos(v)-look*sin(v);
            break;
        case '4':
            look = look*cos(-v)+up*sin(-v);
            up = up*cos(-v)-look*sin(-v);
            break;
        case '5':
            up = up*cos(v)+right*sin(v);
            right = right*cos(v)-up*sin(v);
            break;
        case '6':
            up = up*cos(-v)+right*sin(-v);
            right = right*cos(-v)-up*sin(-v);
            break;
        case 'd':
            zRot -= 5.0f;
            break;
        case 'a':
            zRot += 5.0f;
            break;
        case 'w':
            cam.z += v;
            break;
        case 's':
            cam.z -= v;
            break;
        case 27:
            exit(0);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void specialKeyListener(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            cam = cam+look;
            break;
        case GLUT_KEY_DOWN:
            cam = cam-look;
            break;
        case GLUT_KEY_RIGHT:
            cam = cam+right;
            break;
        case GLUT_KEY_LEFT:
            cam = cam-right;
            break;
        case GLUT_KEY_PAGE_UP:
            cam = cam+up;
            break;
        case GLUT_KEY_PAGE_DOWN:
            cam = cam-up;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {

    glutInit(&argc, argv);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow("Movement");

    glutDisplayFunc(display);
    glutReshapeFunc(reshapeListener);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);

    initGL();
    glutMainLoop();


    return 0;
}



