#pragma once

#include "model.h"
#include "shader.h"
#include "camera.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>

#define GLM_FORCE_RADIANS
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

using namespace std;

const glm::ivec2 SCREEN_SIZE(1920, 1080);
const int NUM_LIGHTS = 500;
const float LIGHT_RADIUS = 40.0f;

GLFWwindow* gWindow;

bool keys[1024];
bool keysPressed[1024];

GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;


GLuint lightBuffer = 0; // point lights in scene
GLuint visibleLightIndicesBuffer = 0; // visible lights after culling=

GLuint workGroupsX = 0;
GLuint workGroupsY = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Bounds for lights animating in the scene
const glm::vec3 lightMinBounds = glm::vec3(-135.0f, 0.0f, -60.0f);
const glm::vec3 lightMaxBounds = glm::vec3(135.0f, 120.0f, 60.0f);
const float lightDeltaTime = -0.6f;

struct PointLight {
	glm::vec4 color;
	glm::vec4 position;
	glm::vec4 paddingAndRadius;
};

struct VisibleIndex {
	int index;
};

void InitGLFW(int argc, char* argv[]);

void InitScene();

// Returns a random position for the light in the scene
glm::vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen);
void SetupLights();
void UpdateLights();

void Movement();

static void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);

static void MouseCallback(GLFWwindow *window, double x, double y);

int main(int argc, char **argv);