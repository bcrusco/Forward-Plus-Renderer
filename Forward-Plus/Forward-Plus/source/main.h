#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <cstdlib>
#include <iostream>

#include <glm\glm.hpp>
#include "SOIL.h"

using namespace std;

const glm::vec2 SCREEN_SIZE(800, 600);

GLFWwindow* gWindow;
GLuint gVAO = 0;
GLuint gBufPos = 0;
GLuint gBufCol = 0;
GLuint gBufSiz = 0;

void initGLFW(int argc, char* argv[]);

void initShaders();

GLuint loadTexture(GLchar* imagepath);

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static void mouseCallback(GLFWwindow* window, double x, double y);

int main(int argc, char **argv);