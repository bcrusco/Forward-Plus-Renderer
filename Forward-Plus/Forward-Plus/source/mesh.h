#pragma once

#include "shader.h"

#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <assimp\Scene.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

// Based on: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/mesh.h

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoordinates;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct Texture {
	GLuint id;
	string type;
	aiString path;
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<GLuint> indices;
	int num_indices;
	vector<Texture> textures;
	GLuint VAO;

	Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures);

	void Draw(Shader shader);

private:
	GLuint VBO, EBO;

	void SetupMesh();
};