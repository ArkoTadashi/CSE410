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
extern bitmap_image image;


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
    Point operator -() {
        return Point(-x, -y, -z);
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

    friend ostream& operator <<(ostream &out, Point p) {
        out << "(" << p.x << "," << p.y << "," << p.z << ")" << " : " << p.n;
        return out;
    }

    friend istream& operator >>(istream &in, Point &p) {
        in >> p.x >> p.y >> p.z;
        return in;
    }

    friend ofstream& operator <<(ofstream &out, Point &p) {
        out << fixed << setprecision(7) << p.x << " " << p.y << " " << p.z;
        return out;
    }

};

struct Color {
    double r, g, b;
    Color() {
        r = g = b = 0.0;
    }

    Color(double r, double g, double b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
};


struct PointLight{
    Point pos;
    Color color;

    void draw() {
        glPointSize(3);
        glBegin(GL_POINTS); {
            glColor3f(color.r, color.g, color.b);
            glVertex3f(pos.x, pos.y, pos.z);
        } glEnd();
    }

    friend istream& operator >>(istream &in, PointLight &light) {
        in >> light.pos.x >> light.pos.y >> light.pos.z;
        in >> light.color.r >> light.color.g >> light.color.b;
        return in;
    }

};

struct SpotLight{
    PointLight pointLight;
    Point dir;
    double cutoffAngle;

    void draw() {
        Color color = pointLight.color;
        Point pos = pointLight.pos;

        glPointSize(12);
        glBegin(GL_POINTS); {
            glColor3f(color.r, color.g, color.b);
            glVertex3f(pos.x, pos.y, pos.z);
        } glEnd();
    }

    friend istream& operator >>(istream &in, SpotLight &light) {
        in >> light.pointLight.pos;
        in >> light.pointLight.color.r >> light.pointLight.color.g >> light.pointLight.color.b;
        in >> light.dir;
        in >> light.cutoffAngle;
        return in;
    }

};


struct Ray {
    Point ori, dir;
    
    Ray(Point ori, Point dir) {
        this->ori = ori;
        dir.normalize();
        this->dir = dir;
    }

    friend ostream& operator<<(ostream &out, Ray r) {
        out << "Origin : " << r.ori << ", Direction : " << r.dir;
        return out;
    }
};

class Object;

extern vector <PointLight*> pointLights;
extern vector <SpotLight*> spotLights;
extern vector <Object*> objects;
extern int recLevel;

class Object {
public:
    Point refPoint;
    double height = 0, width = 0, length = 0;
    Color color;
    vector <double> coefficients; // ambient, diffuse, specular, reflection coefficients
    int shine;
    
    Object() {
        coefficients = vector <double> (4, 0);
    }

    void setColor(Color color) {
        this->color = color;
    }

    virtual Color getColorAt(Point point) {
        return Color(this->color.r, this->color.g, this->color.b);
    }
    
    void setShine(int shine) {
        this->shine = shine;
    }

    void setCoefficients(vector<double> coefficients) {
        this->coefficients = coefficients;
    }   

    virtual void draw() = 0;
    virtual double intersectHelper(Ray ray, Color &color, int level) = 0;
    virtual Ray getNormal(Point point, Ray incidentRay) = 0;
    virtual double intersect(Ray ray, Color &color, int level) {
        double t = intersectHelper(ray, color, level);

        if(t < 0) return -1;
        if(level == 0) return t;

        Point intersectionPoint = ray.ori + ray.dir*t;
        Color colorAtIntersection = getColorAt(intersectionPoint);

        color.r = colorAtIntersection.r * coefficients[0];
        color.g = colorAtIntersection.g * coefficients[0];
        color.b = colorAtIntersection.b * coefficients[0];


        for(int i = 0; i < pointLights.size(); i++) {

            Point lightPosition = pointLights[i]->pos;
            Point lightDirection = intersectionPoint - lightPosition;
            lightDirection.normalize();
            
            Ray lightRay = Ray(lightPosition, lightDirection);
            Ray normal = getNormal(intersectionPoint, lightRay);

            double t2 = (intersectionPoint - lightPosition).length();
            if(t2 < 1e-5) continue;

            bool obscured = false;

            for(Object *obj : objects) {
                double t3 = obj->intersectHelper(lightRay, color, 0);
                if(t3 > 0 && t3 + 1e-5 < t2) {
                    obscured = true;
                    break;
                }
            }

            if(!obscured) {
                double val = max(0.0, -lightRay.dir*normal.dir);
                
                Ray reflection = Ray(intersectionPoint, lightRay.dir - normal.dir*2*(lightRay.dir*normal.dir));
                double phong = max(0.0,-ray.dir*reflection.dir);
                
                color.r += pointLights[i]->color.r * coefficients[1] * val * colorAtIntersection.r;
                color.r += pointLights[i]->color.r * coefficients[2] * pow(phong,shine) * colorAtIntersection.r;

                color.g += pointLights[i]->color.g * coefficients[1] * val * colorAtIntersection.g;
                color.g += pointLights[i]->color.g * coefficients[2] * pow(phong,shine) * colorAtIntersection.g;

                color.b += pointLights[i]->color.b * coefficients[1] * val * colorAtIntersection.b;
                color.b += pointLights[i]->color.b * coefficients[2] * pow(phong,shine) * colorAtIntersection.b;

            }
        }

        for(int i = 0; i < spotLights.size(); i++) {

            Point lightPosition = spotLights[i]->pointLight.pos;
            Point lightDirection = intersectionPoint - lightPosition;
            lightDirection.normalize();

            double dot = lightDirection*spotLights[i]->dir;
            double angle = acos(dot/(lightDirection.length()*spotLights[i]->dir.length())) * (180.0/pi);

            if(fabs(angle)<spotLights[i]->cutoffAngle) {

                Ray lightRay = Ray(lightPosition, lightDirection);
                Ray normal = getNormal(intersectionPoint,lightRay);
                Ray reflection = Ray(intersectionPoint, lightRay.dir - normal.dir*2*(lightRay.dir*normal.dir));
                
                double t2 = (intersectionPoint - lightPosition).length();
                if(t2 < 1e-5) continue;
                
                bool obscured = false;
                
                for(Object *obj : objects){
                    double t3 = obj->intersectHelper(lightRay, color, 0);
                    if(t3 > 0 && t3 + 1e-5 < t2){
                        obscured = true;
                        break;
                    }
                }
                
                if(!obscured) {
                    double phong = max(0.0,-(ray.dir*reflection.dir));
                    double val = max(0.0, -(lightRay.dir*normal.dir));
                    
                    color.r += spotLights[i]->pointLight.color.r * coefficients[1] * val * colorAtIntersection.r;
                    color.r += spotLights[i]->pointLight.color.r * coefficients[2] * pow(phong,shine) * colorAtIntersection.r;
                    
                    color.g += spotLights[i]->pointLight.color.g * coefficients[1] * val * colorAtIntersection.g;
                    color.g += spotLights[i]->pointLight.color.g * coefficients[2] * pow(phong,shine) * colorAtIntersection.g;
                    
                    color.b += spotLights[i]->pointLight.color.b * coefficients[1] * val * colorAtIntersection.b;
                    color.b += spotLights[i]->pointLight.color.b * coefficients[2] * pow(phong,shine) * colorAtIntersection.b;
                    
                }
            }
        }

        if(level < recLevel) {
            Ray normal = getNormal(intersectionPoint,ray);
            Ray reflectionRay = Ray(intersectionPoint, ray.dir - normal.dir*2*(ray.dir*normal.dir));
            reflectionRay.ori = reflectionRay.ori + reflectionRay.dir*1e-5;
            
            int nearIndex = -1;
            double t = -1, tMin = 1e9;

            for(int k = 0; k < (int)objects.size(); k++) {
                t = objects[k]->intersect(reflectionRay, color, 0);
                if(t > 0 && t < tMin) {
                    tMin = t, nearIndex = k;
                }
            }

            if(nearIndex != -1) {
                Color colorTemp(0, 0, 0);
                double t = objects[nearIndex]->intersect(reflectionRay, colorTemp, level+1);             
                color.r += colorTemp.r * coefficients[3];
                color.g += colorTemp.g * coefficients[3];
                color.b += colorTemp.b * coefficients[3];
            }
        }

        return t;
    }

    virtual ~Object(){
        coefficients.clear();
        coefficients.shrink_to_fit();
    }
};


struct Quadratic : public Object{
    double A,B,C,D,E,F,G,H,I,J;

    Quadratic() {

    }

    virtual void draw() {
        return;
    }

    virtual Ray getNormal(Point point, Ray incidentRay) {
        Point dir(2*A*point.x + D*point.y + E*point.z + G,
               2*B*point.y + D*point.x + F*point.z + H,
               2*C*point.z + E*point.x + F*point.y + I);

        return Ray(point, dir);
    }

    bool check(Point point) {
        if(fabs(length) > 1e-5) {
            if(point.x < refPoint.x) return false;
            if(point.x > refPoint.x + length) return false;
        }
        if(fabs(width) > 1e-5) {
            if(point.y < refPoint.y) return false;
            if(point.y > refPoint.y + width) return false;
        }
        if(fabs(height) > 1e-5) {
            if(point.z < refPoint.z) return false;
            if(point.z > refPoint.z + height) return false;
        }
    
        return true;
    }


    virtual double intersectHelper(Ray ray, Color &color, int level) {
        double X0 = ray.ori.x;
        double Y0 = ray.ori.y;
        double Z0 = ray.ori.z;

        double X1 = ray.dir.x;
        double Y1 = ray.dir.y;
        double Z1 = ray.dir.z;

        double C0 = A*X1*X1 + B*Y1*Y1 + C*Z1*Z1 + D*X1*Y1 + E*X1*Z1 + F*Y1*Z1;
        double C1 = 2*A*X0*X1 + 2*B*Y0*Y1 + 2*C*Z0*Z1 + D*(X0*Y1 + X1*Y0) + E*(X0*Z1 + X1*Z0) + F*(Y0*Z1 + Y1*Z0) + G*X1 + H*Y1 + I*Z1;
        double C2 = A*X0*X0 + B*Y0*Y0 + C*Z0*Z0 + D*X0*Y0 + E*X0*Z0 + F*Y0*Z0 + G*X0 + H*Y0 + I*Z0 + J;

        double dis = C1*C1 - 4*C0*C2;
        if(dis < 0) return -1;
        if(fabs(C0) < 1e-5) {
            return -C2/C1;
        }
        double t1 = (-C1 - sqrt(dis))/(2*C0);
        double t2 = (-C1 + sqrt(dis))/(2*C0);

        if(t1 < 0 && t2 < 0) return -1;
        if(t2 < t1) swap(t1,t2);

        if(t1 > 0) {
            Point intersectionPoint = ray.ori + ray.dir*t1;
            if(check(intersectionPoint)) {
                return t1;
            }
        }
        if(t2 > 0) {
            Point intersectionPoint = ray.ori + ray.dir*t2;
            if(check(intersectionPoint)) {
                return t2;
            }
        }

        return -1;

    }
    
    friend istream& operator>>(istream &in, Quadratic &q) {
        in >> q.A >> q.B >> q.C >> q.D >> q.E >> q.F >> q.G >> q.H >> q.I >> q.J;
        in >> q.refPoint >> q.length >> q.width >> q.height;

        in >> q.color.r >> q.color.g >> q.color.b;
        for(int i = 0; i < 4; i++) {
            in >> q.coefficients[i];
        }
        in >> q.shine;

        return in;
    }

};

double determinant(double mat[3][3]) {
	double x = mat[0][0] * (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]);
	double y = mat[0][1] * (mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]);
	double z = mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
	return x-y+z;
}


struct Triangle: public Object {
    Point a, b, c;
    Triangle(){

    }

    Triangle(Point a, Point b, Point c) {
        this->a = a;
        this->b = b;
        this->c = c;
    }

    virtual Ray getNormal(Point point, Ray incidentRay) {
        Point normal = (b-a)^(c-a);
        normal.normalize();
        
        if(incidentRay.dir*normal < 0) {
            return Ray(point, -(normal));
        }
        else {
            return Ray(point, normal);
        }
    }

    virtual void draw() {
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_TRIANGLES); {
            glVertex3f(a.x, a.y, a.z);
            glVertex3f(b.x, b.y, b.z);
            glVertex3f(c.x, c.y, c.z);
        }
        glEnd();
    }

    virtual double intersectHelper(Ray ray, Color &color, int level) {

        double betaMat[3][3] = {
            {a.x - ray.ori.x, a.x - c.x, ray.dir.x},
            {a.y - ray.ori.y, a.y - c.y, ray.dir.y},
            {a.z - ray.ori.z, a.z - c.z, ray.dir.z}
        };
        double gammaMat[3][3] = {
            {a.x - b.x, a.x - ray.ori.x, ray.dir.x},
            {a.y - b.y, a.y - ray.ori.y, ray.dir.y},
            {a.z - b.z, a.z - ray.ori.z, ray.dir.z}
        };
        double tMat[3][3] = {
            {a.x - b.x, a.x - c.x, a.x - ray.ori.x},
            {a.y - b.y, a.y - c.y, a.y - ray.ori.y},
            {a.z - b.z, a.z - c.z, a.z - ray.ori.z}
        };
        double AMat[3][3] = {
            {a.x - b.x, a.x - c.x, ray.dir.x},
            {a.y - b.y, a.y - c.y, ray.dir.y},
            {a.z - b.z, a.z - c.z, ray.dir.z}
        };

        double Adet = determinant(AMat);
        double beta = determinant(betaMat) / Adet;
        double gamma = determinant(gammaMat) / Adet;
        double t = determinant(tMat) / Adet;

        if (beta + gamma < 1 && beta > 0 && gamma > 0 && t > 0) {
            return t;
        }

        return -1;
    }

    friend istream& operator>>(istream &in, Triangle &t) {
        in >> t.a >> t.b >> t.c;
        in >> t.color.r >> t.color.g >> t.color.b;
        for(int i = 0; i < 4; i++) in >> t.coefficients[i];
        in >> t.shine;

        return in;
    }
};

struct Sphere : public Object {

    Sphere() {}

    Sphere(Point center, double radius) {
        refPoint = center;
        length = radius;
    }

    virtual Ray getNormal(Point point, Ray incidentRay) {
        return Ray(point, point - refPoint);
    }

    virtual void draw() {
        int stacks = 30;
        int slices = 20;

        Point points[100][100];
        double h, r;
        for (int i = 0; i <= stacks; i++) {
            h = length * sin(((double)i / (double)stacks) * (pi / 2));
            r = length * cos(((double)i / (double)stacks) * (pi / 2));
            for (int j = 0; j <= slices; j++) {
                points[i][j].x = r * cos(((double)j / (double)slices) * 2 * pi);
                points[i][j].y = r * sin(((double)j / (double)slices) * 2 * pi);
                points[i][j].z = h;
            }
        }
        
        for (int i = 0; i < stacks; i++) {
            glPushMatrix();
            glTranslatef(refPoint.x, refPoint.y, refPoint.z);
            glColor3f(color.r, color.g, color.b);
            for (int j = 0; j < slices; j++) {
                glBegin(GL_QUADS); {
                    glVertex3f(points[i][j].x, points[i][j].y, points[i][j].z);
                    glVertex3f(points[i][j + 1].x, points[i][j + 1].y, points[i][j + 1].z);
                    glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, points[i + 1][j + 1].z);
                    glVertex3f(points[i + 1][j].x, points[i + 1][j].y, points[i + 1][j].z);
                    
                    glVertex3f(points[i][j].x, points[i][j].y, -points[i][j].z);
                    glVertex3f(points[i][j + 1].x, points[i][j + 1].y, -points[i][j + 1].z);
                    glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, -points[i + 1][j + 1].z);
                    glVertex3f(points[i + 1][j].x, points[i + 1][j].y, -points[i + 1][j].z);
                }
                glEnd();
            }
            glPopMatrix();
        }
    }

    virtual double intersectHelper(Ray ray, Color &color, int level) {
        ray.ori = ray.ori - refPoint;
        
        double a = 1;
        double b = 2 * (ray.dir*ray.ori);
        double c = (ray.ori*ray.ori) - (length*length);

        double dis = pow(b, 2) - 4 * a * c;
        double t = -1;
        if (dis < 0) {
            t = -1;
        }
        else {
            
            if(fabs(a) < 1e-5) {
                t = -c/b;
                return t;
            }

            double t1 = (-b - sqrt(dis)) / (2 * a);
            double t2 = (-b + sqrt(dis)) / (2 * a);

            if(t2 < t1) swap(t1, t2);

            if (t1 > 0){
                t = t1;
            }
            else if (t2 > 0){
                t = t2;
            }
            else{
                t = -1;
            }
        }

        return t;
    }

    friend std::istream& operator>>(std::istream& in, Sphere& s) {
        in >> s.refPoint >> s.length;
        in >> s.color.r >> s.color.g >> s.color.b;
        for(int i = 0; i < 4; i++) in >> s.coefficients[i];
        in >> s.shine;
        return in;
    }
};

struct Floor : public Object {
    int tiles;

    Floor() {
        tiles = 1;
    }

    Floor(int floorWidth,int tileWidth){
        tiles = floorWidth / tileWidth;
        refPoint = Point(-floorWidth / 2, -floorWidth / 2, 0);
        length = tileWidth;
    }

    virtual Color getColorAt(Point point) {
        int tileX = (point.x - refPoint.x) / length;
		int tileY = (point.y - refPoint.y) / length;

        if(tileX < 0 or tileX >= tiles or tileY < 0 or tileY >= tiles) {
            return Color(0,0,0);
        }

		if (((tileX + tileY) % 2) == 0) {
			return Color(1,1,1);
		}
		else {
			return Color(0,0,0);
		}
    }

    virtual Ray getNormal(Point point, Ray incidentRay) {
        if(incidentRay.dir.z > 0) return Ray(point, Point(0, 0, 1));
        else return Ray(point, Point(0, 0, -1));
    }

    virtual void draw() {
        for (int i = 0; i < tiles; i++) {
			for (int j = 0; j < tiles; j++) {
				if (((i + j) % 2) == 0) glColor3f(1, 1, 1);
				else glColor3f(0, 0, 0);

				glBegin(GL_QUADS); {
					glVertex3f(refPoint.x + i * length, refPoint.y + j * length, 0);
					glVertex3f(refPoint.x + (i + 1) * length, refPoint.y + j * length, 0);
					glVertex3f(refPoint.x + (i + 1) * length, refPoint.y + (j + 1) * length, 0);
					glVertex3f(refPoint.x + i * length, refPoint.y + (j + 1) * length, 0);
				}
				glEnd();
			}
		}
    }

    virtual double intersectHelper(Ray ray, Color &color, int level) {
        Point normal = Point(0, 0, 1);
        double dotP = normal * ray.dir;
        
        if (round(dotP * 100) == 0) return -1;

        double t = -(normal * ray.ori) / dotP;
        Point p = ray.ori + ray.dir * t;

        if(p.x <= refPoint.x or p.x >= abs(refPoint.x) and p.y <= refPoint.y and p.y >= abs(refPoint.y)) {
            return -1;
        }
        
        return t;
    }
};