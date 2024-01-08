
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

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

#include <iostream>

using namespace std;


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



Point cam(25, 25, 25);
Point cLook(0, 0, 0);
Point cUp(0, 1, 0);
Point cRight(1, 0, 1);
double yRot = 0;

Point ball(0, 0, 0);
Point bLook(1, 0, 0);
Point bRight(0, 0, 1);
Point bUp(0, 1, 0);
double radius = 1.5;
double angle = 0;
double moveSpeed = 1;
double rotateSpeed = 10;

bool sim = false;

Point dir, norm;

double bLen = 40;
double depLen = 3;

clock_t start = clock();

void initGL(){
    glClearColor(0,0,0,0);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80, 1, 1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}


void drawAxes() {
    glLineWidth(5);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(bLen, 0, 0);

        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(0, bLen, 0);

        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, bLen);
    glEnd();
}

void boundary() {
    for(int i = 0; i < 4; i++){
        glPushMatrix();
            glRotatef(90*i, 0, 1, 0);
        
            glColor3f(0.4f, 0.5f, 0.6f);
            glBegin(GL_QUADS);
                glVertex3f(-bLen/2, 0, bLen/2);
                glVertex3f(-bLen/2, depLen, bLen/2);
                glVertex3f(bLen/2 , depLen, bLen/2);
                glVertex3f(bLen/2 , 0, bLen/2);
            glEnd();
        
            glLineWidth(3);
            glColor3f(1, 1, 1);
            glBegin(GL_LINES);
                glVertex3f(-bLen/2, depLen, bLen/2);
                glVertex3f(bLen/2, depLen, bLen/2);
                glVertex3f(-bLen/2, depLen, bLen/2);
                glVertex3f(-bLen/2, 0, bLen/2);
                glVertex3f(bLen/2, depLen, bLen/2);
                glVertex3f(bLen/2, 0, bLen/2);
                glVertex3f(-bLen/2, 0, bLen/2);
                glVertex3f(bLen/2, 0, bLen/2);
            glEnd();
        glPopMatrix();
    }
}

void drawCheckSquare(double x) {
    glBegin(GL_QUADS);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, x);
        glVertex3f(x, 0, x);
        glVertex3f(x, 0, 0);
    glEnd();
}

void drawCheckboard() {
    double len = bLen/8;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            glPushMatrix();
                glTranslatef(-bLen/2 + i*len, 0, -bLen/2 + j*len);
                glColor3f(0.5f+((i+j)%2)*0.3f, 0.5f+((i+j)%2)*0.3f, 0.5f+((i+j)%2)*0.3f);
                drawCheckSquare(len);
            glPopMatrix();
        }
    }
}

void drawArrow() {
    glLineWidth(3);
    glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(5, 0, 0);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex3d(5, 0, 1);
        glVertex3d(7, 0, 0);
        glVertex3d(5, 0, -1);
    glEnd();
}



void drawBall(double rad, int slices, int stacks) {
    Point points[stacks+1][stacks+1];
    double h, r;
    
    for(int i = 0; i <= stacks; i++) {
        h=radius*sin(((double)i/(double)stacks)*(pi/2));
        r=rad*cos(((double)i/(double)stacks)*(pi/2));
        for(int j = 0; j <= slices; j++) {
            points[i][j].x = r*cos(((double)j/(double)slices)*2*pi);
            points[i][j].y = r*sin(((double)j/(double)slices)*2*pi);
            points[i][j].z = h;
        }
    }

    for(int i = 0; i < stacks; i++) {
        for(int j = 0; j < slices; j++) {
            glBegin(GL_QUADS);
                glColor3f((j%2)*0.7f, ((j+1)%2)*0.8f, 1.0f);
                glVertex3f(points[i][j].x, points[i][j].y, points[i][j].z);
                glVertex3f(points[i][j+1].x, points[i][j+1].y, points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x, points[i+1][j+1].y, points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x, points[i+1][j].y, points[i+1][j].z);
                glColor3f(((j+1)%2)*0.7f, (j%2)*0.8f, 1.0f);
                glVertex3f(points[i][j].x, points[i][j].y, -points[i][j].z);
                glVertex3f(points[i][j+1].x, points[i][j+1].y, -points[i][j+1].z);
                glVertex3f(points[i+1][j+1].x, points[i+1][j+1].y, -points[i+1][j+1].z);
                glVertex3f(points[i+1][j].x, points[i+1][j].y, -points[i+1][j].z);
            glEnd();
        }
    }
}






void keyboardListener(unsigned char key, int x, int y) {
    switch (key) {
        case ' ':
            sim = !sim;
        case 'i':
            if (sim) break;
            dir.setPoint(cos(yRot*pi/180), 0, -sin(yRot*pi/180));
            norm.setPoint(0, 0, 0);
            if (ball.x+dir.x*moveSpeed+radius >= bLen/2) {
                norm.setPoint(-1, 0, 0);
            }
            if (ball.x+dir.x*moveSpeed-radius <= -bLen/2) {
                norm.setPoint(1, 0, 0);
            }
            if (ball.z+dir.z*moveSpeed+radius >= bLen/2) {
                norm.setPoint(0, 0, -1);
            }
            if (ball.z+dir.z*moveSpeed-radius <= -bLen/2) {
                norm.setPoint(0, 0, 1);
            }
            dir = dir-norm*(dir%norm)*2;
            
            if (dir.x == 0) {
                yRot = (dir.z > 0? 270 : 90);
            }
            else if (dir.z == 0) {
                yRot = (dir.x > 0? 0 : 180);
            }
            else if (dir.x < 0 and dir.z < 0) {
                yRot = 180-atan(dir.z/dir.x)*180/pi;
            }
            else if (dir.x < 0 and dir.z > 0) {
                yRot = 180+atan(-dir.z/dir.x)*180/pi;
            }
            else if (dir.x > 0 and dir.z < 0) {
                yRot = atan(-dir.z/dir.x)*180/pi;
            }
            else if (dir.x > 0 and dir.z > 0) {
                yRot = atan(-dir.z/dir.x)*180/pi;
            }
            
            
            ball = ball+dir*moveSpeed;
            break;
        case 'k':
            if (sim) break;
            dir.setPoint(-cos(yRot*pi/180), 0, sin(yRot*pi/180));
            norm.setPoint(0, 0, 0);
            if (ball.x+dir.x*moveSpeed+radius >= bLen/2) {
                norm.setPoint(-1, 0, 0);
            }
            if (ball.x+dir.x*moveSpeed-radius <= -bLen/2) {
                norm.setPoint(1, 0, 0);
            }
            if (ball.z+dir.z*moveSpeed+radius >= bLen/2) {
                norm.setPoint(0, 0, -1);
            }
            if (ball.z+dir.z*moveSpeed-radius <= -bLen/2) {
                norm.setPoint(0, 0, 1);
            }
            dir = dir-norm*(dir%norm)*2;
            
            if (dir.x == 0) {
                yRot = (dir.z > 0? 270 : 90);
            }
            else if (dir.z == 0) {
                yRot = (dir.x > 0? 0 : 180);
            }
            else if (dir.x < 0 and dir.z < 0) {
                yRot = 180-atan(dir.z/dir.x)*180/pi;
            }
            else if (dir.x < 0 and dir.z > 0) {
                yRot = 180+atan(-dir.z/dir.x)*180/pi;
            }
            else if (dir.x > 0 and dir.z < 0) {
                yRot = atan(-dir.z/dir.x)*180/pi;
            }
            else if (dir.x > 0 and dir.z > 0) {
                yRot = atan(-dir.z/dir.x)*180/pi;
            }
            
            yRot = yRot+180;
            
            ball = ball+dir*moveSpeed;
            break;
        case 'j':
            yRot += rotateSpeed;
            break;
        case 'l':
            yRot -= rotateSpeed;
            break;
        case 27:
            exit(0);
            break;
    }

    glutPostRedisplay();
}


void simulate() {
    if (((double)(clock()-start)) / CLOCKS_PER_SEC > 0.005) {
        start = clock();
        dir.setPoint(cos(yRot*pi/180), 0, -sin(yRot*pi/180));
        norm.setPoint(0, 0, 0);
        if (ball.x+dir.x*moveSpeed+radius >= bLen/2) {
            norm.setPoint(-1, 0, 0);
        }
        if (ball.x+dir.x*moveSpeed-radius <= -bLen/2) {
            norm.setPoint(1, 0, 0);
        }
        if (ball.z+dir.z*moveSpeed+radius >= bLen/2) {
            norm.setPoint(0, 0, -1);
        }
        if (ball.z+dir.z*moveSpeed-radius <= -bLen/2) {
            norm.setPoint(0, 0, 1);
        }
        dir = dir-norm*(dir%norm)*2;
        
        if (dir.x == 0) {
            yRot = (dir.z > 0? 270 : 90);
        }
        else if (dir.z == 0) {
            yRot = (dir.x > 0? 0 : 180);
        }
        else if (dir.x < 0 and dir.z < 0) {
            yRot = 180-atan(dir.z/dir.x)*180/pi;
        }
        else if (dir.x < 0 and dir.z > 0) {
            yRot = 180+atan(-dir.z/dir.x)*180/pi;
        }
        else if (dir.x > 0 and dir.z < 0) {
            yRot = atan(-dir.z/dir.x)*180/pi;
        }
        else if (dir.x > 0 and dir.z > 0) {
            yRot = atan(-dir.z/dir.x)*180/pi;
        }
        
        
        ball = ball+dir*moveSpeed;
    }
    
    glutPostRedisplay();
    
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    
    gluLookAt(cam.x, cam.y, cam.z,
                cLook.x, cLook.y, cLook.z,
                cUp.x, cUp.y, cUp.z);
    
    
    drawCheckboard();
    boundary();
    drawAxes();
    
    glPushMatrix();
        glColor3f(0.9f, 0.2f, 0.6f);
        glTranslatef(ball.x, radius, ball.z);
        glRotatef(yRot, 0, 1, 0);
        drawArrow();
        drawBall(radius, 24, 60);
    glPopMatrix();
    
    glutSwapBuffers();
}

void timer(int value) {
    if (sim) simulate();
    glutTimerFunc(50, timer, 0);
}

int main(int argc, char** argv) {
    
    glutInit(&argc,argv);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(50, 50);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutCreateWindow("Rolling Ball");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardListener);

    initGL();
    glutTimerFunc(50, timer, 0);
    glutMainLoop();

    return 0;
}

