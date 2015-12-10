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

// Renderer constants
const glm::ivec2 SCREEN_SIZE(1920, 1080);
const unsigned int NUM_LIGHTS = 500;
const float LIGHT_RADIUS = 30.0f;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 300.0f;

// Constants for light animations
const glm::vec3 LIGHT_MIN_BOUNDS = glm::vec3(-135.0f, 0.0f, -60.0f);
const glm::vec3 LIGHT_MAX_BOUNDS = glm::vec3(135.0f, 120.0f, 60.0f);
const float LIGHT_DELTA_TIME = -0.6f;

GLFWwindow* gWindow;

// Mouse and keyboard variables
bool keys[1024];
bool keysPressed[1024];
bool firstMouse = true;
GLfloat lastX = 400.0f, lastY = 300.0f;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Used for storage buffer objects to hold light data and visible light indicies data
GLuint lightBuffer = 0;
GLuint visibleLightIndicesBuffer = 0;

// structures defining the data of both buffers
struct PointLight {
	glm::vec4 color;
	glm::vec4 position;
	glm::vec4 paddingAndRadius;
};

struct VisibleIndex {
	int index;
};

// X and Y work group dimension variables for compute shader
GLuint workGroupsX = 0;
GLuint workGroupsY = 0;

// Camera object
Camera camera(glm::vec3(-40.0f, 10.0f, 0.0f));

// Creates window and initializes GLFW
void InitGLFW(int argc, char* argv[]);

// Initializes buffers and scene data
void InitScene();

// Returns a random position in the scene confined to the lightMinBounds and lightMaxBounds
glm::vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen);

// Fills the lightBuffer with lights in random positions and colors
void SetupLights();

// Updates light position based on lightDeltaTime. Called each frame
void UpdateLights();

// Mouse and keyboard callback functions
void Movement();
static void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods);
static void MouseCallback(GLFWwindow *window, double x, double y);

int main(int argc, char **argv);