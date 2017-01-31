#define cimg_use_jpeg
#define cimg_use_tiff

#include <GL/gl.h>
#include <GL/glut.h>
#include "CImg/CImg.h"

#include <iostream>

using namespace std;
using namespace cimg_library;

GLsizei winWidth = 640;
GLsizei winHeight = 480;
int window;
GLuint textureIndex;

float rot = 0;

void draw(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f,0.0f,-5.0f);

    glRotatef(rot,0.0f,1.0f,0.0f);

    glBindTexture(GL_TEXTURE_2D, textureIndex);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    glTexCoord2f(5.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    glTexCoord2f(5.0f, 5.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    glTexCoord2f(0.0f, 5.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    glEnd();

    //    rot += 0.4f;

    glutSwapBuffers();
}

void idle(){
    glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y){
    if (key == 27 || key == 'q') // Escape or q
    { 
        glutDestroyWindow(window); 
        exit(0);                   
    }
}

void loadTextures(){
    CImg<unsigned char> image("floor.tif");

    int width = image.width();
    int height = image.height();
    image.permute_axes("cxyz");
    
    glGenTextures(1, &textureIndex);
    glBindTexture(GL_TEXTURE_2D, textureIndex);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGB,
                 width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image.data());
}

int init(GLvoid){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,
                   (GLfloat)winWidth / (GLfloat)winHeight,
                   0.1f,1000.0f);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    loadTextures();

    return true;
}

GLvoid resizeWindow(GLsizei w, GLsizei h){
    winWidth = w;
    winHeight = h;
    
    if(winHeight == 0){
        winHeight = 1;
    }

    glViewport(0, 0, winWidth, winHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,
                   (GLfloat)winWidth / (GLfloat)winHeight,
                   0.1f,1000.0f);
}

int main(int argc, char** argv){
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE |
                        GLUT_ALPHA | GLUT_DEPTH);

    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(0, 0);
    window = glutCreateWindow("Maze Viewer");
    glutDisplayFunc(&draw);
    glutIdleFunc(&idle);
    glutReshapeFunc(&resizeWindow);
    glutKeyboardFunc(&keyDown);
    init();
    glutMainLoop();
    
    return 0;
}
