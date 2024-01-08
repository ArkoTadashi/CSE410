
#include<stdio.h>
#include<stdlib.h>
#include<math.h>


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


double maxTLen = 1.5;
double tLen = 1.5;
double maxSRadius = maxTLen/sqrt(3.0);
double sRadius = 0;
double sStep = maxSRadius/15;



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




void drawTriangle() {
glBegin(GL_TRIANGLES);
    glVertex3f(1, 0, 0);
    glVertex3f(0, 1, 0);
    glVertex3f(0, 0, 1);
glEnd();
}

void drawCylinderSection(double h, double r, int seg) {
    struct Point points[seg+1];
    double angle = 70.5287794*pi/180;

    for (int i = 0; i < seg + 1; i++) {
        double alp = -angle/2 + angle*i/seg;
        points[i].x = r*cos(alp);
        points[i].y = r*sin(alp);
    }

    glBegin(GL_QUADS);
    for (int i = 0; i < seg; i++) {
        glVertex3f(points[i].x, points[i].y, h/2);
        glVertex3f(points[i].x, points[i].y, -h/2);
        glVertex3f(points[i+1].x, points[i+1].y, -h/2);
        glVertex3f(points[i+1].x, points[i+1].y, h/2);
    }
    glEnd();
}

void drawSphereSection(double r, int seg) {
    struct Point points[seg+1][seg+1];
    double x, y;

    for (int i = 0; i <= seg; i++) {
        for (int j = 0; j <= seg; j++) {
            x = -1 + 2.0*i/seg;
            y = -1 + 2.0*j/seg;
            points[i][j].x = x;
            points[i][j].y = y;
            points[i][j].z = 1;

            points[i][j] = normalizedVector(points[i][j]);
            points[i][j] = points[i][j] * r;
        }
    }

    for (int i = 0; i < seg; i++) {
        for (int j = 0; j < seg; j++) {
            glBegin(GL_QUADS);
                glVertex3f(points[i][j].x, points[i][j].y, points[i][j].z);
                glVertex3f(points[i][j+1].x, points[i][j+1].y, points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x, points[i+1][j+1].y, points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x, points[i+1][j].y, points[i+1][j].z);
            glEnd();
        }
    }
}





void drawCylinders() {
    glColor3f(1, 1, 1);

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glRotatef(45 + i*90, 0, 1, 0);
            glTranslatef(tLen / sqrt(2), 0, 0);
            drawCylinderSection(tLen * sqrt(2), sRadius, 100);
        glPopMatrix();
    }

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glRotatef(90, 1, 0, 0);
            glRotatef(45 + i*90, 0, 1, 0);
            glTranslatef(tLen / sqrt(2), 0, 0);
            drawCylinderSection(tLen * sqrt(2), sRadius, 100);
        glPopMatrix();
    }

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glRotatef(90, 0, 0, 1);
            glRotatef(45 + i*90, 0, 1, 0);
            glTranslatef(tLen / sqrt(2), 0, 0);
            drawCylinderSection(tLen * sqrt(2), sRadius, 100);
        glPopMatrix();
    }
}

void drawSpheres() {
    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glColor3f(0.5f, i%2, (i+1)%2);
            glRotatef(90*i, 0, 1, 0);
            glTranslatef(0, 0, tLen);
            drawSphereSection(sRadius, 100);
        glPopMatrix();
    }

    for (int i = 0; i < 2; i++) {
        glPushMatrix();
            glColor3f(1, 0.5f, 0);
            glRotatef(90+180*i, 1, 0, 0);
            glTranslatef(0, 0, tLen);
            drawSphereSection(sRadius, 100);
        glPopMatrix();
    }
}

void drawPyramids() {
    double diff = maxTLen - tLen;
    diff = diff / 3.0;

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glColor3f((i+1) % 2, i % 2, 0.5f);
            glRotatef(90*i, 0, 1, 0);
            glTranslatef(diff, diff, diff);
            glScaled(tLen, tLen, tLen);
            drawTriangle();
        glPopMatrix();
    }

    for (int i = 0; i < 4; i++) {
        glPushMatrix();
            glColor3f((i+1) % 2, i % 2, 0.5f);
            glRotatef(90*i, 0, 1, 0);
            glRotatef(180, 0, 0, 1);
            glTranslatef(diff, diff, diff);
            glScaled(tLen, tLen, tLen);
            drawTriangle();
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
    drawPyramids();
    drawSpheres();
    drawCylinders();


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
        case ',':
            tLen -= v;
            sRadius += sStep;
            if (tLen < 0) {
                tLen = 0;
                sRadius = maxSRadius;
            }
            break;
        case '.':
            tLen += v;
            sRadius -= sStep;
            if (tLen > maxTLen) {
                tLen = maxTLen;
                sRadius = 0;
            }
            break;
        case 27:
            exit(0);
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
    glutCreateWindow("Magic Cube");

    glutDisplayFunc(display);
    glutReshapeFunc(reshapeListener);
    glutKeyboardFunc(keyboardListener);

    initGL();
    glutMainLoop();


    return 0;
}



