#include <GL/gl.h>
#include <GL/glut.h>
#include "SOIL.h"

#include <iostream>

using namespace std;

GLsizei winWidth = 640;
GLsizei winHeight = 480;
int window;

void draw(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    

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

int init(GLvoid){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,
                   (GLfloat)winWidth / (GLfloat)winHeight,
                   0.1f,1000.0f);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    int width, height;
    unsigned char* image =
        SOIL_load_image("floor.tif", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);

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
