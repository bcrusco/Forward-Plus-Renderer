#pragma once

#include "model.h"
#include "shader.h"
#include "camera.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <cstdlib>
#include <fstream>
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "SOIL.h"

using namespace std;

const glm::ivec2 SCREEN_SIZE(800, 600);

const int NUM_LIGHTS = 10;

GLFWwindow* gWindow;
GLuint gVAO = 0;
GLuint gBufPos = 0;
GLuint gBufCol = 0;
GLuint gBufSiz = 0;

GLuint headBuffer = 0; // head of linked lists
GLuint nodeBuffer = 0; // nodes of the linked lists
GLuint lightBuffer = 0; // point lights in scene
GLuint counterBuffer = 0;

GLuint workGroupsX = 0;
GLuint workGroupsY = 0;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

struct PointLight {
	glm::vec3 color;
	glm::vec4 previous;
	glm::vec4 current;
	glm::vec3 velocity;
	float radius;
};

void initGLFW(int argc, char* argv[]);

void initShaders();

// THis stuff should be moved to a scene class later
void InitScene();

void UpdateLights(float deltaTime);

GLuint loadTexture(GLchar* imagepath);

void printShaderInfoLog(int shader);
void printLinkInfoLog(int program);

std::string TextFileRead(const char *filename);

void Movement();

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static void mouseCallback(GLFWwindow* window, double x, double y);

static void scrollCallback(GLFWwindow* window, double x, double y);

int main(int argc, char **argv);