// Local Headers
#include "glitter.hpp"
// System Headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Standard Headers
#include <cstdio>
#include <cstdlib>
#include <random>
#include <time.h>

//// Helper functions
#include <camera.hpp>
#include <shader.hpp>
#include <light.hpp>
#include <model.hpp>
#include <filesystem.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

#define NUM_LIGHTS 3
#define NUM_AMB_PROBES 30

glm::vec3 ambProbes[NUM_AMB_PROBES];

Light lights[NUM_LIGHTS];
int activeLights = 0;
bool lightMoveMode = false;
int lightMoveInd = 0;

glm::vec3 testCoord(-10, 12, -3);

bool wPressed = false;
bool sPressed = false;
bool dPressed = false;
bool aPressed = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void updatePosition();
void drawMainScene(Shader* shader);
void renderShadowMaps();
void RenderCube(const Shader& shader);
void renderDepthTex();
void setupAmbProbes();
void renderMainMap();

Shader fullShader;
Model sampleModel;
GLuint shadowMaps[NUM_LIGHTS];
GLuint shadowFrameBuffers[NUM_LIGHTS];
Shader shadowShader;
Shader ssaoShader;
Shader ssdoShader;
Shader depthShader;
Shader mapShader;
GLuint ambPositionTex;
GLuint ambPositionBuffer;
GLuint ambPositionRenderBuffer;

GLuint mainMapTex;

float shadowFar = 100.0f;
float shadowNear = 1.0f;

bool useTextures = true;

Camera camera(glm::vec3(0.f, 0.f, 2.f));

uniform_real_distribution<GLfloat> rands(0, 1);
default_random_engine eng(time(NULL));

int renderMode = 0;

char* shadowTexNames[] = {
	"shadowMap0",
	"shadowMap1",
	"shadowMap2"
};

int main(int argc, char * argv[]) {
    // Load GLFW and Create a Window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    auto mWindow = glfwCreateWindow(mWidth, mHeight, "OpenGL", nullptr, nullptr);

    // Check for Valid Context
    if (mWindow == nullptr) {
        fprintf(stderr, "Failed to Create OpenGL Context");
        return EXIT_FAILURE;
    }

    // Create Context and Load OpenGL Functions
    glfwMakeContextCurrent(mWindow);
    gladLoadGL();
    fprintf(stderr, "OpenGL %s\n", glGetString(GL_VERSION));

	// Set callback functions
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);
	glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	float am = .05f;

	lights[0] = Light(glm::vec3(-30, 30, -3), glm::vec3(am, am, am), glm::vec3(1, .1, .1), glm::vec3(1, .1, .1), 0);
	lights[1] = Light(glm::vec3(0, 10, 3), glm::vec3(am, am, am), glm::vec3(.1, .1, 1), glm::vec3(.1, .1, 1), 1);
	lights[2] = Light(glm::vec3(30, 30, -3), glm::vec3(am, am, am), glm::vec3(.1, 1, .1), glm::vec3(.1, 1, .1), 2);
	activeLights = 3;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_DEPTH_TEST);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 

	// Create a sample shader that displays normal
	fullShader = Shader(FileSystem::getPath("Shaders/geometry.vert.glsl").c_str(),
		FileSystem::getPath("Shaders/geometry.frag.glsl").c_str());
	shadowShader = Shader(FileSystem::getPath("Shaders/shadow_geometry.vert.glsl").c_str(),
		FileSystem::getPath("Shaders/shadow_geometry.frag.glsl").c_str(),
		FileSystem::getPath("Shaders/shadow_geometry.geom.glsl").c_str());
	ssaoShader = Shader(FileSystem::getPath("Shaders/geometry.vert.glsl").c_str(),
		FileSystem::getPath("Shaders/ssao_geometry.frag.glsl").c_str());
	ssdoShader = Shader(FileSystem::getPath("Shaders/geometry.vert.glsl").c_str(),
		FileSystem::getPath("Shaders/ssdo_geometry.frag.glsl").c_str());
	depthShader = Shader(FileSystem::getPath("Shaders/pre_ssao_geometry.vert.glsl").c_str(),
		FileSystem::getPath("Shaders/pre_ssao_geometry.frag.glsl").c_str());
	mapShader = Shader(FileSystem::getPath("Shaders/map_geometry.vert.glsl").c_str(),
		FileSystem::getPath("Shaders/map_geometry.frag.glsl").c_str(),
		FileSystem::getPath("Shaders/shadow_geometry.geom.glsl").c_str());

	// Load a model from obj file
	sampleModel = Model(FileSystem::getPath("Resources/crytek_sponza/sponza.obj").c_str());

	fullShader.Use();
	for (int i = 0; i < activeLights; i++) {
		lights[i].update(fullShader);
	}

	for (int i = 0; i < NUM_LIGHTS; i++) {
		glGenTextures(1, &shadowMaps[i]);
		glGenFramebuffers(1, &shadowFrameBuffers[i]);
	}

	setupAmbProbes();

	glGenTextures(1, &ambPositionTex);
	glBindTexture(GL_TEXTURE_2D, ambPositionTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
		mWidth, mHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glGenFramebuffers(1, &ambPositionBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ambPositionBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ambPositionTex, 0);
	GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachments);
	glGenRenderbuffers(1, &ambPositionRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, ambPositionRenderBuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ambPositionRenderBuffer);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "GBuffer Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//renderMainMap();

	Shader* currentShader;
    // Rendering Loop
    while (glfwWindowShouldClose(mWindow) == false) {
		glfwPollEvents();
		updatePosition();
        // Background Fill Color
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (renderMode == 0) {
			currentShader = &fullShader;
			renderShadowMaps();
			renderDepthTex();
			currentShader->Use();
			glUniform1f(glGetUniformLocation(currentShader->Program, "farClip"), shadowFar);
		}
		else if (renderMode == 1) {
			currentShader = &ssaoShader;
			renderDepthTex();
			currentShader->Use();
		}
		else {
			currentShader = &ssdoShader;
			currentShader->Use();
		}

		glUniform1i(glGetUniformLocation(currentShader->Program, "positionTex"), 5);
		glActiveTexture(GL_TEXTURE0 + 5);
		glBindTexture(GL_TEXTURE_2D, ambPositionTex);

		glActiveTexture(GL_TEXTURE0 + 8);
		glUniform1i(glGetUniformLocation(currentShader->Program, "mainMapTex"), 8);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mainMapTex);

		for (int i = 0; i < NUM_AMB_PROBES; i++) {
			glUniform3fv(glGetUniformLocation(currentShader->Program, ("probes[" + to_string(i) + "]").c_str()), 1, glm::value_ptr(ambProbes[i]));
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)mWidth / (GLfloat)mHeight, mNear, mFar);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4();
		glUniformMatrix4fv(glGetUniformLocation(currentShader->Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(currentShader->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(currentShader->Program, "pPos"), 1, glm::value_ptr(camera.Position));

		if (renderMode == 0) {
			for (int i = 0; i < NUM_LIGHTS; i++) {
				glActiveTexture(GL_TEXTURE0 + i);
				glUniform1i(glGetUniformLocation(currentShader->Program, shadowTexNames[i]), i);
				glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps[i]);
			}
		}
		
		useTextures = (renderMode == 0);
		drawMainScene(currentShader);

		model = glm::mat4();
		for (int i = 0; i < activeLights; i++) {
			glUniformMatrix4fv(glGetUniformLocation(currentShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(model, lights[i].getPos())));
			RenderCube(*currentShader);
		}

        // Flip Buffers and Draw
        glfwSwapBuffers(mWindow);
    }   glfwTerminate();
    return EXIT_SUCCESS;
}

void renderMainMap() {
	mapShader.Use();
	float mapRes = 100.0f;
	glViewport(0, 0, mapRes, mapRes);
	glm::mat4 perspective = glm::perspective(glm::radians(90.0f), 1.0f, mNear, mFar);
	glm::vec3 center(0, 10, 0);
	glUniformMatrix4fv(glGetUniformLocation(mapShader.Program, "shadowMat[0]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(center,
				center + glm::vec3(1, 0, 0),
				glm::vec3(0, -1, 0))
		));
	glUniformMatrix4fv(glGetUniformLocation(mapShader.Program, "shadowMat[1]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(center,
				center + glm::vec3(-1, 0, 0),
				glm::vec3(0, -1, 0))
		));
	glUniformMatrix4fv(glGetUniformLocation(mapShader.Program, "shadowMat[2]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(center,
				center + glm::vec3(0, 1, 0),
				glm::vec3(0, 0, 1))
		));
	glUniformMatrix4fv(glGetUniformLocation(mapShader.Program, "shadowMat[3]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(center,
				center + glm::vec3(0, -1, 0),
				glm::vec3(0, 0, -1))
		));
	glUniformMatrix4fv(glGetUniformLocation(mapShader.Program, "shadowMat[4]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(center,
				center + glm::vec3(0, 0, 1),
				glm::vec3(0, -1, 0))
		));
	glUniformMatrix4fv(glGetUniformLocation(mapShader.Program, "shadowMat[5]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(center,
				center + glm::vec3(0, 0, -1),
				glm::vec3(0, -1, 0))
		));
	glUniform3fv(glGetUniformLocation(shadowShader.Program, "lightPos"), 1, glm::value_ptr(center));
	glUniform1f(glGetUniformLocation(shadowShader.Program, "farClip"), shadowFar);

	glGenTextures(1, &mainMapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mainMapTex);
	for (GLuint i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB,
			mapRes, mapRes, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	GLuint mapFB;
	glGenFramebuffers(1, &mapFB);
	glBindFramebuffer(GL_FRAMEBUFFER, mapFB);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mainMapTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glClear(GL_DEPTH_BUFFER_BIT);

	useTextures = true;
	drawMainScene(&mapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glViewport(0, 0, mWidth, mHeight);
}

void setupAmbProbes() {
	for (GLuint i = 0; i < NUM_AMB_PROBES; i++) {
		ambProbes[i] = glm::vec3(
			rands(eng) * 2 - 1,
			rands(eng) * 2 - 1,
			rands(eng)
		);
		ambProbes[i] = glm::normalize(ambProbes[i]);
		GLfloat ratio = ((GLfloat)i) / NUM_AMB_PROBES;
		ambProbes[i] *= .1 + .9 * ratio * ratio;
	}
}

void renderDepthTex() {
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (GLfloat)mWidth / (GLfloat)mHeight, mNear, mFar);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4();
	depthShader.Use();
	glViewport(0, 0, mWidth, mHeight);
	glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(glGetUniformLocation(depthShader.Program, "farClip"), mFar);

	glBindFramebuffer(GL_FRAMEBUFFER, ambPositionBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	useTextures = false;
	drawMainScene(&depthShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	useTextures = true;
}

void setupShadowMatrices(int lightIndex) {
	glm::mat4 perspective = glm::perspective(glm::radians(90.0f), 1.0f, shadowNear, shadowFar);
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMat[0]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(lights[lightIndex].getPos(),
				lights[lightIndex].getPos() + glm::vec3(1, 0, 0),
				glm::vec3(0, -1, 0))
		));
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMat[1]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(lights[lightIndex].getPos(),
				lights[lightIndex].getPos() + glm::vec3(-1, 0, 0),
				glm::vec3(0, -1, 0))
		));
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMat[2]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(lights[lightIndex].getPos(),
				lights[lightIndex].getPos() + glm::vec3(0, 1, 0),
				glm::vec3(0, 0, 1))
		));
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMat[3]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(lights[lightIndex].getPos(),
				lights[lightIndex].getPos() + glm::vec3(0, -1, 0),
				glm::vec3(0, 0, -1))
		));
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMat[4]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(lights[lightIndex].getPos(),
				lights[lightIndex].getPos() + glm::vec3(0, 0, 1),
				glm::vec3(0, -1, 0))
		));
	glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMat[5]"), 1, GL_FALSE,
		glm::value_ptr(
			perspective * glm::lookAt(lights[lightIndex].getPos(),
				lights[lightIndex].getPos() + glm::vec3(0, 0, -1),
				glm::vec3(0, -1, 0))
		));
	glUniform3fv(glGetUniformLocation(shadowShader.Program, "lightPos"), 1, glm::value_ptr(lights[lightIndex].getPos()));
	glUniform1f(glGetUniformLocation(shadowShader.Program, "farClip"), shadowFar);
}

void renderShadowMaps() {
	useTextures = false;
	shadowShader.Use();
	GLuint shadowRes = 1024;
	glViewport(0, 0, shadowRes, shadowRes);

	for (int lightNum = 0; lightNum < NUM_LIGHTS; lightNum++) {
		setupShadowMatrices(lightNum);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMaps[lightNum]);
		for (GLuint i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
				shadowRes, shadowRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffers[lightNum]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMaps[lightNum], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glClear(GL_DEPTH_BUFFER_BIT);

		drawMainScene(&shadowShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glViewport(0, 0, mWidth, mHeight);
	useTextures = true;
}

void drawMainScene(Shader* currentShader) {
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(0.05f));    // The sponza model is too big, scale it first
	glUniformMatrix4fv(glGetUniformLocation(currentShader->Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	sampleModel.Draw(*currentShader);
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
GLuint cubeTex = 0;
void RenderCube(const Shader& shader)
{
	// Initialize (if necessary)
	if (cubeVAO == 0)
	{
		GLfloat vertices[] = {
			// Back face
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f, // Bottom-left
			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f, // bottom-right         
			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f,  // top-right
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  // bottom-left
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,// top-left
																 // Front face
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f, // bottom-left
			0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 0.0f,  // bottom-right
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,  // top-right
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 1.0f, 1.0f, // top-right
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 1.0f,  // top-left
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,  // bottom-left
																   // Left face
			-0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-0.5f,  0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-left
			-0.5f, -0.5f, -0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-0.5f, -0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
			-0.5f,  0.5f,  0.5f, 1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																  // Right face
			0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
			0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-left
			0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
																 // Bottom face
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f, // top-right
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 1.0f, // top-left
			0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,// bottom-left
			0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-0.5f, -0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f, // top-right
																  // Top face
			-0.5f,  0.5f, -0.5f,  0.0f,  -1.0f,  0.0f, 0.0f, 1.0f,// top-left
			0.5f,  0.5f , 0.5f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			0.5f,  0.5f, -0.5f,  0.0f,  -1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			0.5f,  0.5f,  0.5f,  0.0f,  -1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-0.5f,  0.5f, -0.5f,  0.0f,  -1.0f,  0.0f, 0.0f, 1.0f,// top-left
			-0.5f,  0.5f,  0.5f,  0.0f,  -1.0f,  0.0f, 0.0f, 0.0f // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// Fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// Link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		char texture[] = {
			255, 255, 255};
		glGenTextures(1, &cubeTex);
		glBindTexture(GL_TEXTURE_2D, cubeTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &(texture[0]));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	// Render Cube

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse1"), 0);
	glBindTexture(GL_TEXTURE_2D, cubeTex);
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

double speed = 6;
double lastTime = -1.0;
void updatePosition() {
	if (lastTime == -1.0) {
		lastTime = glfwGetTime();
		return;
	}

	double deltaTime = glfwGetTime() - lastTime;
	lastTime = glfwGetTime();

	if(lightMoveMode){
		if (wPressed) {
			lights[lightMoveInd].ProcessKeyboard(FORWARD, speed * deltaTime, camera.Front, camera.Right);
		}
		if (sPressed) {
			lights[lightMoveInd].ProcessKeyboard(BACKWARD, speed * deltaTime, camera.Front, camera.Right);
		}
		if (aPressed) {
			lights[lightMoveInd].ProcessKeyboard(LEFT, speed * deltaTime, camera.Front, camera.Right);
		}
		if (dPressed) {
			lights[lightMoveInd].ProcessKeyboard(RIGHT, speed * deltaTime, camera.Front, camera.Right);
		}
		lights[lightMoveInd].update(fullShader);
	}
	else {
		if (wPressed) {
			camera.ProcessKeyboard(FORWARD, speed * deltaTime);
		}
		if (sPressed) {
			camera.ProcessKeyboard(BACKWARD, speed * deltaTime);
		}
		if (aPressed) {
			camera.ProcessKeyboard(LEFT, speed * deltaTime);
		}
		if (dPressed) {
			camera.ProcessKeyboard(RIGHT, speed * deltaTime);
		}
	}
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Camera movements
	switch(key){
	case GLFW_KEY_W:
		wPressed = action != GLFW_RELEASE;
		break;
	case GLFW_KEY_S:
		sPressed = action != GLFW_RELEASE;
		break;
	case GLFW_KEY_A:
		aPressed = action != GLFW_RELEASE;
		break;
	case GLFW_KEY_D:
		dPressed = action != GLFW_RELEASE;
		break;
	case GLFW_KEY_L:
		if (action == GLFW_PRESS) {
			lightMoveMode = !lightMoveMode;
		}
		break;
	case GLFW_KEY_M:
		if (action == GLFW_PRESS) {
			renderMode = (renderMode + 1) % 2;
		}
		break;
	case GLFW_KEY_SPACE:
		if (lightMoveMode && action == GLFW_PRESS) {
			lightMoveInd = (lightMoveInd + 1) % activeLights;
		}
		break;
	}
	//cout << glm::to_string(camera.Position) << endl;
}

GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	//TODO: changed
	//if (state == GLFW_PRESS) {
	if (true) {
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		GLfloat xoffset = xpos - lastX;
		GLfloat yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	/*if (state == GLFW_RELEASE) {
		firstMouse = true;
	}*/
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}