#pragma once

#include <GL\glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Based on: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/shader.h

class Shader {
public:
	GLuint Program;

	// Compile shader on the fly
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath);

	// Use this shader
	void Use();

private:
	void CheckCompileErrors(GLuint shader, std::string type);
};