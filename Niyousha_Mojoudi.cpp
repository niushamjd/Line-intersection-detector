/*********

ADDITIONAL FEATURES: The lightning on coordinate system, the circle showing the mouse position, turning on and off of the sentences to guide you to the next step.
*********/
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define TIMER_PERIOD  1000 // Period for the timer.
#define TIMER_ON      1    // 0:disable timer, 1:enable timer

#define D2R 0.0174532

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false;
int  winWidth, winHeight,timecnt; // current Window width and height
bool firtPointDeclared = false, endPointDeclared = false;

int linecnt = 0, clickcnt = 0;

/* Programmer variables and definitions */
typedef struct {
    float x, y;
} point_t;

typedef struct {
    point_t start, end;
    float A, B, C,t;
} line_t;

typedef struct {
    point_t center;
    float radius;
} circle_t;

typedef struct {
    bool hit;
    float dist;
    point_t nearest;
    point_t ixect;
} result_t;

circle_t circ[20];

line_t line;
int vertNo = 0;

line_t perp;
result_t res;
line_t ln[10];
point_t mouse;

int circleno = 4;//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
    float angle;
    glBegin(GL_POLYGON);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
    float angle;

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 100; i++)
    {
        angle = 2 * PI * i / 100;
        glVertex2f(x + r * cos(angle), y + r * sin(angle));
    }
    glEnd();
}

void print(int x, int y, const char* string, void* font)
{
    int len, i;

    glRasterPos2f(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }
}

// display text with variables.
void vprint(int x, int y, void* font, const char* string, ...)
{
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);

    int len, i;
    glRasterPos2f(x, y);
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, str[i]);
    }
}

void vprint2(int x, int y, float size, const char* string, ...) {
    va_list ap;
    va_start(ap, string);
    char str[1024];
    vsprintf_s(str, string, ap);
    va_end(ap);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(size, size, 1);

    int len, i;
    len = (int)strlen(str);
    for (i = 0; i < len; i++)
    {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
    }
    glPopMatrix();
}

float testPoint(line_t line, point_t p) {
    float dx = line.end.x - line.start.x;
    float dy = line.end.y - line.start.y;
    // Ix = Mx + dx.t => t = (Ix - Mx) / dx
    if (dx != 0) {
        return (p.x - line.start.x) / dx;
    }
    // Iy = My + dy.t => t = (Iy - My) / dy
    return (p.y - line.start.y) / dy;
}

float distance(point_t p, point_t q) {
    return sqrt((p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y));
}


void drawFirstPoint() {
    glColor3f(0.7, 0.7, 0.8);
    circle(line.start.x, line.start.y, 20);
}


void drawLine(point_t s, point_t e, float size) {
    glLineWidth(size);
    glBegin(GL_LINES);
    glVertex2f(s.x, s.y);
    glVertex2f(e.x, e.y);
    glEnd();
    glLineWidth(1.0f);
}

void drawLineSegment() {
    glColor3f(0.7, 0.7, 0.8);
    circle(line.end.x, line.end.y, 20);
    glColor3f(0, 0, 0.2);
    drawLine(line.start, line.end, 3);
}

point_t  findPoint(point_t start, point_t end, float t) {
    point_t p = { start.x + (end.x - start.x) * t, start.y + (end.y - start.y) * t };
    return p;
}





void drawCircularObject() {
    glColor4f(0, 1, 0, 0.7);
    if (res.hit)
        glColor4f(1, 0, 0, 0.7);
    circle(circ[vertNo].center.x, circ[vertNo].center.y, circ[vertNo].radius);
}


void drawLine2(float x1, float y1, float x2, float y2) {
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void drawCoordinateSystem() {
    glColor3f(0.5, 0.7, 0.6);


    for (int i = -350; i < 400; i += 100) {
        if(timecnt%2==0)
            glColor3f(0.7, 0.3, 0.5);
        drawLine2(i, 250, i, -250);
    }
    glColor3f(0.7, 0.3, 0.5);
    for (int i = -300; i < 400; i += 100) {
        if (timecnt % 2 == 0)
            glColor3f(0.5, 0.7, 0.6);
        drawLine2(i, 250, i, -250);
    }


    for (int z = -250; z < 300; z += 100) {
        drawLine2(350, z, -350, z);
    } glColor3f(0.5, 0.6, 0.2);
    for (int z = -200; z < 300; z += 100) {
        drawLine2(350, z, -350, z);
    }

}


//
// To display onto window using OpenGL commands
//
int checkIntersect(line_t line2)
{
  

    line.t = (((line2.end.x - line2.start.x) * (line2.start.y - line.start.y)) - ((line2.end.y - line2.start.y) * (line2.start.x - line.start.x))) /(( (line2.end.x - line2.start.x) * (line.end.y - line.start.y)) -( (line.end.x - line.start.x) * (line2.end.y - line2.start.y)));
    line2.t = (((line.end.x - line.start.x) * (line2.start.y - line.start.y)) - ((line2.start.x - line.start.x) * (line.end.y - line.start.y))) / (((line2.end.x - line2.start.x) * (line.end.y - line.start.y)) - ((line.end.x - line.start.x) * (line2.end.y - line2.start.y)));
    if (line.t >= 0 && line.t <= 1 && line2.t >= 0 && line2.t <= 1)
        return 1;
    return 0;
}
void display() {

    //
    // clear window to black
    //
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(1, 1, 1);
    glRectf(-350, -250, 350, 250);

    glColor3f(0.7, 0.7, 0.8);
    if (circleno == 1) {
        glColor3f(0.8, 0.8, 0.9);
        circle(mouse.x, mouse.y, 20);
        circle(-340, -275, 15);
    }
    if (circleno == 2) {
        glColor3f(0.8, 0.8, 0.9);
        circle(mouse.x, mouse.y, 20);
        circle(-110, -275, 15);
    }

    if (circleno == 3) {

        circle(140, -275, 15);
        {
            glColor3f(0.4, 0.7, 0.99);
            circle(mouse.x, mouse.y, 3);
        }
    }
    glColor3f(0.6, 0.3, 0.7);
    if (circleno == 4) {

        if (timecnt % 2 == 0)

            vprint(-340, -280, GLUT_BITMAP_8_BY_13, "F1 : Set Start Position");
    }else vprint(-340, -280, GLUT_BITMAP_8_BY_13, "F1 : Set Start Position");

    if (circleno == 1) {
        if (timecnt % 2 == 0)
            vprint(-110, -280, GLUT_BITMAP_8_BY_13, "F2 : Set  End Position");
    }else  vprint(-110, -280, GLUT_BITMAP_8_BY_13, "F2 : Set  End Position");
        

    if (circleno == 2) {
        if (timecnt % 2 == 0)
            vprint(140, -280, GLUT_BITMAP_8_BY_13, "F3 : Add Lines ");
    }else   vprint(140, -280, GLUT_BITMAP_8_BY_13, "F3 : Add Lines ");

    drawCoordinateSystem();


    glColor3f(0.6, 0.3, 0.7);
    vprint(-250, 280, GLUT_BITMAP_8_BY_13, "Niyousha Mojoudi - 22001297     HW #2 � 2020�2021 Spring�");


    if (firtPointDeclared) {
        drawFirstPoint();
        glColor3f(0, 0, 0);
        vprint(line.start.x-2, line.start.y-2, GLUT_BITMAP_8_BY_13, "S");
    }
    if (endPointDeclared) {
        drawLineSegment();
        glColor3f(0, 0, 0);
        vprint(line.end.x-2, line.end.y-2, GLUT_BITMAP_8_BY_13, "E");
    }
    if (clickcnt % 2 != 0) {
       
        glColor3f(0.2, 0.5, 0.8);
        circle(ln[linecnt].start.x, ln[linecnt].start.y, 3);
    }
    for (int i = 0; i <linecnt; i++)
    {

        glColor3f(0, 0.5, 0.1);
        if (checkIntersect(ln[i]))
            glColor3f(0.8, 0, 0);
            drawLine(ln[i].start, ln[i].end, 3);
            glColor3f(0.2, 0.5, 0.8);
            circle(ln[i].start.x, ln[i].start.y, 3);
            circle(ln[i].end.x, ln[i].end.y, 3);
       
        
    }
  
    
    
    
    


    glutSwapBuffers();

}

//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
    // exit when ESC is pressed.
    if (key == 27)
        exit(0);

    // to refresh the window it calls display() function
    glutPostRedisplay();
}
//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{

    if (key == GLUT_KEY_F1) {

        circleno = 1;

    }
    if (key == GLUT_KEY_F2) {

        circleno = 2;

    }
    if (key == GLUT_KEY_F3) {

        circleno = 3;

    }
    // to refresh the window it calls display() function
    glutPostRedisplay();

}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
    // Write your codes here.
    switch (key) {
    case GLUT_KEY_UP: up = false; break;
    case GLUT_KEY_DOWN: down = false; break;
    case GLUT_KEY_LEFT: left = false; break;
    case GLUT_KEY_RIGHT: right = false; break;
    }
    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
    point_t click = { x - winWidth / 2 , winHeight / 2 - y };

    if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN) {
        if (circleno == 1) {
            line.start = click;
            firtPointDeclared = true;

        }
    }
    //starting point of the line segment

    if (circleno == 2) { //end point of the line segment
        if (line.start.x == click.x && line.start.y == click.y)
            return; //if the same point is clicked (not a line segment; a point)
                    //   do nothing; wait for another point
        line.end = click;
        endPointDeclared = true;

    }
   
    if (circleno == 3&&clickcnt<=20)
    { 
        if (button == GLUT_LEFT_BUTTON && stat == GLUT_DOWN) {
            clickcnt++;
            if (clickcnt % 2 != 0)
                ln[linecnt].start = click;
            else {
                ln[linecnt].end = click;
                linecnt++;
            }
        }

    }



    // to refresh the window it calls display() function
    glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
    winWidth = w;
    winHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // refresh window.
}

void onMoveDown(int x, int y) {
    // Write your codes here.



    // to refresh the window it calls display() function   
    glutPostRedisplay();
}

void onMove(int x, int y) {
    mouse.x = x - winWidth / 2;
    mouse.y = winHeight / 2 - y;
    
    // to refresh the window it calls display() function
    glutPostRedisplay();
}

void onTimer(int v) {

    glutTimerFunc(TIMER_PERIOD, onTimer, 1000);
    // Write your codes here.

    timecnt++;

    // to refresh the window it calls display() function
    glutPostRedisplay(); // display()

}


void Init() {

    // Smoothing shapes
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

int main(int argc, char* argv[]) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Line Intersection");

    glutDisplayFunc(display);
    glutReshapeFunc(onResize);

    //
    // keyboard registration
    //
    glutKeyboardFunc(onKeyDown);
    glutSpecialFunc(onSpecialKeyDown);

    glutKeyboardUpFunc(onKeyUp);
    glutSpecialUpFunc(onSpecialKeyUp);

    //
    // mouse registration
    //
    glutMouseFunc(onClick);
    glutMotionFunc(onMoveDown);
    glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
    // timer event
    glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

    Init();

    glutMainLoop();
}