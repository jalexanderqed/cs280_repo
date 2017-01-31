#define cimg_use_jpeg
#define cimg_use_tiff

#include <GL/gl.h>
#include <GL/glut.h>
#include <string>
#include <fstream>
#include "CImg/CImg.h"
#include <vector>
#include <limits>
#include <cmath>

#include <iostream>

using namespace std;
using namespace cimg_library;

float rot = 0;

// OpenGL values
GLsizei winWidth = 640;
GLsizei winHeight = 480;
int window;

// Textures
vector<string> textureNames;
vector<GLuint> textureIds;

// Player position
float px, py, pz;
float pPitch, pYaw;
float moveMult = 0.5f;

// Stuff for reading input file
int lineCount = 1;
struct Maze {
    int cellSize;
    int wallHeight;
    int width;
    int height;
    int numTextures;
    vector<int> vertWalls;
    vector<int> horizWalls;
} maze;

void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(px, py, pz,
              px + sin(pYaw) * cos(pPitch),
              py + sin(pPitch),
              pz + cos(pYaw) * cos(pPitch),
              0, 1, 0);

    for(size_t i = 0; i < maze.horizWalls.size(); i++){
        if(maze.horizWalls[i] == 0) continue;
        
        int col = i % maze.width;
        int row = i / maze.width;
        float xMin = col * maze.cellSize;
        float xMax = xMin + maze.cellSize;
        float z = row * maze.cellSize;
        glBindTexture(GL_TEXTURE_2D,
                      textureIds[maze.horizWalls[i] - 1]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(xMin, 0, z);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(xMax, 0, z);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(xMax, maze.wallHeight, z);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(xMin, maze.wallHeight, z);
        glEnd();
    }

    for(size_t i = 0; i < maze.vertWalls.size(); i++){
        if(maze.vertWalls[i] == 0) continue;
        
        int col = i % maze.width;
        int row = i / maze.width;
        float x = col * maze.cellSize;
        float zMin = row * maze.cellSize;
        float zMax = zMin + maze.cellSize;

        glBindTexture(GL_TEXTURE_2D,
                      textureIds[maze.vertWalls[i] - 1]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(x, 0, zMin);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(x, 0, zMax);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(x, maze.wallHeight, zMax);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(x, maze.wallHeight, zMin);
        glEnd();
    }

    glutSwapBuffers();
}

void idle() {
    glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y) {
    switch(key){
    case 27:
    case 'q':
        glutDestroyWindow(window);
        exit(0);
        break;
    case 'w':
        px += moveMult * sin(pYaw);
        pz += moveMult * cos(pYaw);
        break;
    case 's':
        px -= moveMult * sin(pYaw);
        pz -= moveMult * cos(pYaw);
        break;
    case 'a':
        px += moveMult * sin(pYaw + (M_PI / 2));
        pz += moveMult * cos(pYaw + (M_PI / 2));
        break;
    case 'd':
        px += moveMult * sin(pYaw - (M_PI / 2));
        pz += moveMult * cos(pYaw - (M_PI / 2));
        break;
    }
}

void loadTextures() {
    for(size_t i = 0; i < textureNames.size(); i++) {
        CImg<unsigned char> image(textureNames[i].c_str());

        int width = image.width();
        int height = image.height();
        image.mirror("y");
        image.permute_axes("cxyz");

        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D,
                     0, GL_RGB,
                     width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, image.data());

        textureIds.push_back(id);
    }
}

int init(GLvoid) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,
                   (GLfloat) winWidth / (GLfloat) winHeight,
                   0.1f, 1000.0f);
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

GLvoid resizeWindow(GLsizei w, GLsizei h) {
    winWidth = w;
    winHeight = h;

    if (winHeight == 0) {
        winHeight = 1;
    }

    glViewport(0, 0, winWidth, winHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f,
                   (GLfloat) winWidth / (GLfloat) winHeight,
                   0.1f, 1000.0f);
}

void takeLine(ifstream& file){
    file.ignore(numeric_limits<streamsize>::max(), '\n');
    lineCount++;
}

void readString(const char *str, ifstream& file) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (file.get() != str[i]) {
            cout << "Parsing error on line "
                 << lineCount << endl;
            exit(1);
        }
    }
}

void readTextures(ifstream& file){
    for(int i = 1; i <= maze.numTextures; i++){
        if(file.peek() == '#'){
            i--;
            takeLine(file);
            continue;
        }

        int id;
        file >> id;
        if(id != i){
            cout << "Texture numbering error on line "
                 << lineCount << endl;
            exit(1);
        }
        file.ignore(1);
        string textureName;
        getline(file, textureName);
        lineCount++;

        int commentBegin = textureName.find_first_of('#');
        textureName = textureName.substr(0, commentBegin);

        char c;
        while((c = textureName.back()) == ' ' || c == '\n'){
            textureName =
                    textureName.substr(0, textureName.length() - 1);
        }

        textureNames.push_back(textureName);
    }
}

void readRow(int length, vector<int>& vec, ifstream& file){
    int val;
    for(int i = 0; i < length; i++){
        file >> val;
        if(file.fail() || val > maze.numTextures || val < 0){
            cout << "Error in floorplan at line " << lineCount << endl;
            cout << val << endl;
            exit(1);
        }
        vec.push_back(val);
    }
}

void readMazeShape(ifstream& file){
    for(int i = 0; i < maze.height; i++){
        if(file.peek() == '#'){
            i--;
            takeLine(file);
            continue;
        }
        readRow(maze.width, maze.horizWalls, file);
        takeLine(file);
        readRow(maze.width, maze.vertWalls, file);
        takeLine(file);
    }
}

void readMazeFile(char *fileName) {
    ifstream file;
    file.open(fileName);
    if (!file.is_open()) {
        cout << "Could not open file "
             << fileName << endl;
        exit(1);
    }

    maze.cellSize = -1;
    maze.wallHeight = -1;
    maze.width = -1;
    maze.height = -1;

    while (!file.eof()) {
        switch (file.peek()) {
            case '#':
                takeLine(file);
                break;
            case '\n':
                lineCount++;
            case ' ':
            case '\t':
                file.get();
                break;
            case 'C':
                readString("CELL", file);
                file >> maze.cellSize;
                break;
            case 'D':
                readString("DIMENSIONS", file);
                file >> maze.width;
                file >> maze.height;
                break;
            case 'H':
                readString("HEIGHT", file);
                file >> maze.wallHeight;
                break;
            case 'T':
                readString("TEXTURES", file);
                file >> maze.numTextures;
                takeLine(file);
                readTextures(file);
                break;
            case 'F':
                readString("FLOORPLAN", file);
                takeLine(file);
                readMazeShape(file);
                break;
            case EOF:
                takeLine(file);
                break;
            default:
                cout << "Unrecognized start of line at line "
                     << lineCount << endl;
                exit(1);
                break;
        }
    }

    file.close();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);

    if(argc != 2){
        cout << "Usage:\t./MazeViewer <maze-file-name>" << endl;
        return 0;
    }

    readMazeFile(argv[1]);

    /*for(int i = 0; i < maze.height; i++){
        for(int j = 0; j < maze.width; j++){
            cout << maze.horizWalls[i * maze.width + j] << ' ';
        }
        cout << endl;
        for(int j = 0; j < maze.width; j++){
            cout << maze.vertWalls[i * maze.width + j] << ' ';
        }
        cout << endl;
    }

    for(int i = 0; i < textureNames.size(); i++){
        cout << textureNames[i] << endl;
    }

    cout << "cs: " << maze.cellSize << endl;
    cout << "wh: " << maze.wallHeight << endl;
    cout << "w: " << maze.width << endl;
    cout << "h: " << maze.height << endl;
    cout << "nt: " << maze.numTextures << endl;
    cout << "vw: " << maze.vertWalls.size() << endl;
    cout << "hw: " << maze.horizWalls.size() << endl;

    return 0;*/

    px = -3 * maze.cellSize;
    py = maze.wallHeight / 2.0f;
    pz = -3 * maze.cellSize;

    pPitch = 0;
    pYaw = M_PI / 4;

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
