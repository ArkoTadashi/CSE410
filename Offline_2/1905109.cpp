


#include <iostream>
#include <iomanip>
#include <fstream>
#include <stack>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include "bitmap.hpp"

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


static unsigned long int g_seed = 1;
inline int randoms() {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}


struct Point {
    double x, y, z, n;

    Point() {
        x = y = z = 0;
        n = 1;
    }
    Point(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->n = 1;
    }
    Point(double x, double y, double z, double n) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->n = n;
    }
    Point(const Point &p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->n = p.n;
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
    Point operator ^(Point p) {
        return Point(y*p.z - z*p.y, z*p.x - x*p.z, x*p.y - y*p.x);
    }
    double operator *(Point p) {
        return (x*p.x + y*p.y + z*p.z);
    }

    double length() {
        return sqrt(x*x + y*y + z*z);
    }

    void normalize() {
        double len = length();
        x /= len;
        y /= len;
        z /= len;
    }

    void scale() {
        x /= n;
        y /= n;
        z /= n;
        n = 1;
    }

    
    void setPoint(Point &p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->n = p.n;
    }
    void setPoint(double x, double y, double z) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->n = 1;
    }
    void setPoint(double x, double y, double z, double n) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->n = n;
    }

};

struct Triangle{
    Point points[3];
    int col[3];

    Triangle() {
        col[0] = randoms()%255+1;
        col[1] = randoms()%255+1;
        col[2] = randoms()%255+1;
    }

    Triangle(Point p, Point q, Point r) {
        points[0] = p;
        points[1] = q;
        points[2] = r;
        col[0] = randoms()%255+1;
        col[1] = randoms()%255+1;
        col[2] = randoms()%255+1;
    }

    void setPoints(Point p, Point q, Point r) {
        points[0] = p;
        points[1] = q;
        points[2] = r;
    }
};


struct Matrix{
    double mat[dim][dim];

    Matrix() {
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                mat[i][j] = 0;
            }
        }
    }

    Matrix operator*(Matrix p) {
        Matrix ret;
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                for (int k = 0; k < dim; k++) {
                    ret.mat[i][j] += mat[i][k]*p.mat[k][j];
                }
            }
        }
        return ret;
    }

    Point operator*(Point p) {
        Point ret;
        double c[dim] = {0};
        double pArr[dim] = {p.x, p.y, p.z, p.n};

        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                c[i] += mat[i][j]*pArr[j];
            }
        }

        ret.setPoint(c[0], c[1], c[2], c[3]);
        ret.scale();
        return ret;
    }

    void identity() {
        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < dim; j++) {
                if (i == j) {
                    mat[i][j] = 1;
                    continue;
                }
                mat[i][j] = 0;
            }
        }
    }

    void translation(Point p) {
        identity();
        mat[0][3] = p.x;
        mat[1][3] = p.y;
        mat[2][3] = p.z;
    }

    void scaling(Point p) {
        identity();
        mat[0][0] = p.x;
        mat[1][1] = p.y;
        mat[2][2] = p.z;
    }

    Point rodrigues(Point p, Point axis, double angle) {
        double theta = angle*pi/180;
        return p*cos(theta) + axis*(axis*p)*(1-cos(theta)) + (axis^p)*sin(theta);   
    }

    void rotate(Point axis, double angle) {
        identity();
        Point a = axis;
        a.normalize();
        
        Point x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
        x = rodrigues(x, a, angle);
        y = rodrigues(y, a, angle);
        z = rodrigues(z, a, angle);

        mat[0][0] = x.x;
        mat[1][0] = x.y;
        mat[2][0] = x.z;
        mat[0][1] = y.x;
        mat[1][1] = y.y;
        mat[2][1] = y.z;
        mat[0][2] = z.x;
        mat[1][2] = z.y;
        mat[2][2] = z.z;
    }


    void viewMatrix(Point cam, Point look, Point up) {
        identity();
        Point z = look-cam; z.normalize();
        Point x = z^up; x.normalize();
        Point y = x^z; y.normalize(); 

        mat[0][0] = x.x;
        mat[0][1] = x.y;
        mat[0][2] = x.z;
        mat[1][0] = y.x;
        mat[1][1] = y.y;
        mat[1][2] = y.z;
        mat[2][0] = -z.x;
        mat[2][1] = -z.y;
        mat[2][2] = -z.z;

        Matrix m;
        m.translation(Point(-cam.x, -cam.y, -cam.z));

        m = m*(*this);
        *this = m;
    }

    void projectionMatrix(double fovY, double aspect, double near, double far) {
        identity();
        double fovX = fovY*aspect;
        double angleR = (fovX/2)*pi/180;
        double r = near*tan(angleR);
        double angleT = (fovY/2)*pi/180;
        double t = near*tan(angleT);

        mat[0][0] = near/r;
        mat[1][1] = near/t;
        mat[2][2] = -(far+near)/(far-near);
        mat[3][2] = -1;
        mat[2][3] = -2*far*near/(far-near);
        mat[3][3] = 0;
    }

    
};



/////////


int main() {
    ifstream in;
    in.open("scene.txt");
    ofstream out;
    out.open("stage1.txt");

    Point cam, look, up;
    in >> cam.x >> cam.y >> cam.z;
    in >> look.x >> look.y >> look.z;
    in >> up.x >> up.y >> up.z;

    double fovY, aspect, near, far;
    in >> fovY >> aspect >> near >> far;

    stack<Matrix> st;
    Matrix mat1;
    mat1.identity();
    st.push(mat1);

    int count = 0;

    while (true) {
        string s;
        in >> s;

        if (s == "triangle") {
            Point p1, p2, p3;
            in >> p1.x >> p1.y >> p1.z;
            in >> p2.x >> p2.y >> p2.z;
            in >> p3.x >> p3.y >> p3.z;

            p1 = st.top()*p1;
            p2 = st.top()*p2; 
            p3 = st.top()*p3;

            out << setprecision(6) << fixed;
            out << p1.x << GAP << p1.y << GAP << p1.z << endl;
            out << p2.x << GAP << p2.y << GAP << p2.z << endl;
            out << p3.x << GAP << p3.y << GAP << p3.z << endl;
            out << endl;

            count++; 
        }
        else if (s == "translate") {
            Point p;
            in >> p.x >> p.y >> p.z;

            Matrix m;
            m.translation(p);

            Matrix t = st.top()*m;
            st.pop();
            st.push(t);
        }
        else if (s == "scale") {
            Point p;
            in >> p.x >> p.y >> p.z;

            Matrix m;
            m.scaling(p);
            Matrix t = st.top()*m;
            st.pop();
            st.push(t);
        }
        else if (s == "rotate") {
            double angle;
            Point p;
            in >> angle >> p.x >> p.y >> p.z;

            Matrix m;
            m.rotate(p, angle);
            Matrix t = st.top()*m;
            st.pop();
            st.push(t);
        }
        else if (s == "push") {
            st.push(st.top());
        }
        else if (s == "pop") {
            if(!st.empty()) {
                st.pop();
            }
        }
        else if (s == "end") {
            break;
        }

    }

    in.close();
    out.close();





    in.open("stage1.txt");
    out.open("stage2.txt");

    Matrix mat2;
    mat2.viewMatrix(cam, look, up);

    for(int i = 0; i < count; i++) {
        Point p1, p2, p3;
        in >> p1.x >> p1.y >> p1.z;
        in >> p2.x >> p2.y >> p2.z;
        in >> p3.x >> p3.y >> p3.z;

        p1 = mat2*p1;
        p2 = mat2*p2;
        p3 = mat2*p3;

        out << setprecision(6) << fixed;
        out << p1.x << GAP << p1.y << GAP << p1.z << endl;
        out << p2.x << GAP << p2.y << GAP << p2.z << endl;
        out << p3.x << GAP << p3.y << GAP << p3.z << endl;
        out << endl;
    }

    in.close();
    out.close();





    in.open("stage2.txt");
    out.open("stage3.txt");

    Matrix mat3;
    mat3.projectionMatrix(fovY, aspect, near, far);

    for (int i = 0; i < count; i++) {
        Point p1, p2, p3;
        in >> p1.x >> p1.y >> p1.z;
        in >> p2.x >> p2.y >> p2.z;
        in >> p3.x >> p3.y >> p3.z;

        p1 = mat3*p1;
        p2 = mat3*p2;
        p3 = mat3*p3;

        out << setprecision(6) << fixed;
        out << p1.x << GAP << p1.y << GAP << p1.z << endl;
        out << p2.x << GAP << p2.y << GAP << p2.z << endl;
        out << p3.x << GAP << p3.y << GAP << p3.z << endl;
        out << endl;
    }

    in.close();
    out.close();





    in.open("config.txt");

    int screenWidth = 100, screenHeight = 100;
    in >> screenWidth >> screenHeight;

    in.close();


    in.open("stage3.txt");
    out.open("z_buffer.txt");

    double dx = 2.0/screenWidth;
    double dy = 2.0/screenHeight;

    double topY = 1-dy/2;
    double bottomY = -1+dy/2;
    double leftX = -1+dx/2;
    double rightX = 1-dx/2;

    vector<vector<double>> z_buffer(screenHeight, vector<double>(screenWidth, 1.0));
    

    bitmap_image image(screenWidth, screenHeight);
    for (int i = 0; i < screenWidth; i++) {
        for (int j = 0; j < screenHeight; j++) {
            image.set_pixel(i, j, 0, 0, 0);
        }
    }

    for(int t = 0; t < count; t++) {
        Point p1, p2, p3;
        in >> p1.x >> p1.y >> p1.z;
        in >> p2.x >> p2.y >> p2.z;
        in >> p3.x >> p3.y >> p3.z;

        Triangle tr(p1, p2, p3);

        double minX, maxX, minY, maxY;

        minX = min(min(p1.x, p2.x), p3.x);
        maxX = max(max(p1.x, p2.x), p3.x);
        minY = min(min(p1.y, p2.y), p3.y);
        maxY = max(max(p1.y, p2.y), p3.y);

        minX = max(minX, leftX);
        maxX = min(maxX, rightX);
        minY = max(minY,bottomY);
        maxY = min(maxY,topY);

        int startY = round((topY-minY)/dy);
        int endY = round((topY-maxY)/dy);

        for(int i = endY; i <= startY; i++) {
            double y = topY - i*dy;

            vector<double> xx(2), zz(2);
            int cnt = 0;

            for(int k = 0; k < 3; k++) {
                int l = (k+1)%3;

                if(tr.points[k].y == tr.points[l].y) continue;

                if(y >= min(tr.points[k].y, tr.points[l].y) && y <= max(tr.points[k].y, tr.points[l].y)) {
                    xx[cnt] = tr.points[k].x - (tr.points[k].x - tr.points[l].x)*(tr.points[k].y - y)/(tr.points[k].y - tr.points[l].y);
                    zz[cnt] = tr.points[k].z - (tr.points[k].z - tr.points[l].z)*(tr.points[k].y - y)/(tr.points[k].y - tr.points[l].y);
                    cnt++;
                }
            }

            vector<double> tempx(2);
            tempx = xx;

            for(int k = 0; k < 2; k++) {
                if(xx[k] < minX) xx[k] = minX;
                if(xx[k] > maxX) xx[k] = maxX;
            }

            zz[0] = zz[1] - (zz[1] - zz[0])*(tempx[1] - xx[0])/(tempx[1] - tempx[0]);
            zz[1] = zz[1] - (zz[1] - zz[0])*(tempx[1] - xx[1])/(tempx[1] - tempx[0]);

            double xa, za, xb, zb;
            xa = xx[0];
            xb = xx[1];
            za = zz[0];
            zb = zz[1];

            if(xx[0] >= xx[1]) {
                swap(xa, xb);
                swap(za, zb);
                swap(tempx[0], tempx[1]);
            }

            int startX = round((xa-leftX)/dx);
            int endX = round((xb-leftX)/dx);
            
            for(int j = startX; j <= endX; j++) {
                double xp = leftX + j*dx;

                double zp = zb - (zb-za)*((xb-xp)/(xb-xa));

                if (zp < -1) continue;
                if (zp < z_buffer[j][i]) {
                    z_buffer[j][i] = zp;
                    image.set_pixel(j, i, tr.col[0], tr.col[1], tr.col[2]);
                }
            }
        }
    }

    for (int i = 0; i < screenHeight; i++) {
        for (int j = 0; j < screenWidth; j++) {
            if (z_buffer[i][j] < 1.0) {
                out << setprecision(6) << fixed;
                out << z_buffer[i][j] << "\t";
            }
        }
        out << endl;
    }

    in.close();
    out.close();
    image.save_image("out.bmp");

    z_buffer.clear();
    image.clear();

    return 0;
}
