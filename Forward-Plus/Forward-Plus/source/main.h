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

const glm::ivec2 SCREEN_SIZE(1280, 720);

const int NUM_LIGHTS = 100;
const float LIGHT_RADIUS = 5.5f;

GLFWwindow* gWindow;
GLuint gVAO = 0;
GLuint gBufLightPosition = 0;
GLuint gBufLightColor = 0;
GLuint gBufLightRadius = 0;

GLuint lightBuffer = 0; // point lights in scene
GLuint visibleLightIndicesBuffer = 0; // visible lights after culling=

GLuint workGroupsX = 0;
GLuint workGroupsY = 0;

glm::vec3 directionalLightPosition = glm::vec3(0.0f, 100.0, 0.0f);
glm::vec3 directionalLightDirection = glm::normalize(glm::vec3(0.0, 0.0, -1.0));

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// These bounds are going to need to be tweaked
glm::vec3 lightMinBounds = glm::vec3(-135.0f, 0.0f, -60.0f);
glm::vec3 lightMaxBounds = glm::vec3(135.0f, 120.0f, 60.0f);
float lightDeltaTime = -0.6; // negative?

struct PointLight {
	glm::vec4 color;
	glm::vec4 position;
	//float radius;
	glm::vec4 paddingAndRadius;
};

struct VisibleIndex {
	int index;
};

void initGLFW(int argc, char* argv[]);

void initShaders();

// This stuff should be moved to a scene class later
void InitScene();

// Returns a random position for the light in the scene
glm::vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen);
void SetupLights();
void UpdateLights(float deltaTime);

//GLuint loadTexture(GLchar* imagepath);

void printShaderInfoLog(int shader);
void printLinkInfoLog(int program);

std::string TextFileRead(const char *filename);

void Movement();

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static void mouseCallback(GLFWwindow* window, double x, double y);

static void scrollCallback(GLFWwindow* window, double x, double y);

int main(int argc, char **argv);

void RenderQuad();

void LoadLights(Shader shader);
void DrawLights(Shader shader);

