
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

#include <GL/freeglut.h>
#include <optional>
#include <chrono>
#include <set>
#include <functional>
#include <thread>
#include <cmath>

typedef std::chrono::high_resolution_clock clk;

class time
{
private:
    static constexpr float s_FPS = 144.0f;
    static clk::time_point s_start_time_point;
    static clk::time_point s_frame_begin_time_point;
    static clk::time_point s_frame_end_time_point;
    static float s_delta_time_s;

public:
    static const float &FPS();
    static inline int64_t frame_time_ns();
    static void initialise();
    static void start_frame();
    static void end_frame();
    static const float &get_delta_time_s();
    static int64_t get_now_ns();
    static float get_now_s();
};

clk::time_point time::s_start_time_point;
clk::time_point time::s_frame_begin_time_point;
clk::time_point time::s_frame_end_time_point;
float time::s_delta_time_s = 0.0f;

const float &time::FPS()
{
    return s_FPS;
}

int64_t time::frame_time_ns()
{
    return (int64_t)(1000000000 / s_FPS);
}

void time::initialise()
{
    s_start_time_point = clk::now();
}

void time::start_frame()
{
    s_frame_begin_time_point = clk::now();
}

void time::end_frame()
{
    s_frame_end_time_point = clk::now();
    clk::duration nanoseconds = s_frame_end_time_point - s_frame_begin_time_point;

    if(nanoseconds.count() < frame_time_ns())
    {
        clk::duration sleep_time(frame_time_ns() - nanoseconds.count());

        std::this_thread::sleep_for(sleep_time);

        s_frame_end_time_point = clk::now();
        nanoseconds = s_frame_end_time_point - s_frame_begin_time_point;
    }

    s_delta_time_s = nanoseconds.count() / 1e9f;
}

const float &time::get_delta_time_s()
{
    return s_delta_time_s;
}

int64_t time::get_now_ns()
{
    clk::time_point now_time_point(clk::now());
    clk::duration time_point_ns(now_time_point - s_start_time_point);

    return time_point_ns.count();
}

float time::get_now_s()
{
    return get_now_ns() / 1e9f;
}



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




struct Point cam(5, 0, 2);
struct Point look(-1, 0, 0);
struct Point right(0, 1, 0);
struct Point up(0, 0, 1);
double zRot = 45;


constexpr size_t circle_segments = 50;
Point center0(0.0f, 0.0f, 0.0f);
constexpr float radius0 = 0.5f;
constexpr float radius1 = 0.25f;
constexpr float radius2 = 0.1f;
constexpr float frequency0 = 0.25f;
constexpr float frequency1 = 0.5f;
size_t frame_count = 0;

double radius = 0.7;
double angle = 0;
double speed = 0;



void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}



void drawAxes() {
    glLineWidth(5);
    glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f(-5, 0, 0);
        glVertex3f(5, 0, 0);

        glColor3f(1, 1, 1);
        glVertex3f(0, -5, 0);
        glVertex3f(0, 5, 0);

        glColor3f(1, 1, 1);
        glVertex3f(0, 0, -5);
        glVertex3f(0, 0, 5);
    glEnd();
}




void drawSquare(double angle) {
    glBegin(GL_QUADS);
        glVertex3f(radius*std::cos(angle)-0.1f, radius*std::sin(angle)-0.1f, -0.3f);
        glVertex3f(radius*std::cos(angle)-0.1f, radius*std::sin(angle)+0.1f, -0.3f);
        glVertex3f(radius*std::cos(angle)+0.1f, radius*std::sin(angle)+0.1f, -0.3f);
        glVertex3f(radius*std::cos(angle)+0.1f, radius*std::sin(angle)-0.1f, -0.3f);
    glEnd();
}

void drawLines(double angle) {
    glLineWidth(2);
    glBegin(GL_LINES);
        glVertex3f(radius*std::cos(angle), radius*std::sin(angle), 0);
        glVertex3f(radius*std::cos(angle)-0.1f, radius*std::sin(angle), -0.3f);
    glEnd();
    glBegin(GL_LINES);
        glVertex3f(radius*std::cos(angle), radius*std::sin(angle), 0);
        glVertex3f(radius*std::cos(angle)+0.1f, radius*std::sin(angle), -0.3f);
    glEnd();
}

void draw_circle(const Point &_center, const double &_radius)
{
    glBegin(GL_LINES);
    glLineWidth(3);
    glColor3f(1, 1, 1);


    for(size_t i = 0; i < circle_segments; ++i)
    {
        const size_t start_idx = i;
        const size_t end_idx = (i + 1) % circle_segments;
        const float line_start_x = std::cos(start_idx * 2.0f * M_PI / circle_segments) * _radius + _center.x;
        const float line_start_y = std::sin(start_idx * 2.0f * M_PI / circle_segments) * _radius + _center.y;
        const float line_end_x = std::cos(end_idx * 2.0f * M_PI / circle_segments) * _radius + _center.x;
        const float line_end_y = std::sin(end_idx * 2.0f * M_PI / circle_segments) * _radius + _center.y;
        glVertex3f(line_start_x, line_start_y, 0);
        glVertex3f(line_end_x, line_end_y, 0);
    }

    glEnd();
}

void display_callback()  // draw each frame
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    time::start_frame();

    gluLookAt(cam.x, cam.y, cam.z,
                cam.x + look.x, cam.y + look.y, cam.z + look.z,
                up.x, up.y, up.z);

    glRotatef(zRot, 0, 0, 1);

    
    angle = frame_count*speed/time::FPS();
    drawAxes();
    draw_circle(Point(0, 0, 0), radius);
    for (int i = 0; i < 6; i++) {
        drawSquare((angle+60*i)*pi/180);
        drawLines((angle+60*i)*pi/180);
    }

    glutSwapBuffers();
    glutPostRedisplay();    // draw next frame
    time::end_frame();

    ++frame_count;
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
            speed = speed+10;
            if (speed > 50) speed = 50;
            break;
        case '2':
            speed = speed-10;
            if (speed < 0) speed = 0;
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

    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshapeListener);
    glutKeyboardFunc(keyboardListener);
    glutSpecialFunc(specialKeyListener);

    initGL();
    glutMainLoop();


    return 0;
}



