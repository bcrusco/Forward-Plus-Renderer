#pragma once

#include "mesh.h"

#include <GL/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <SOIL.h>
#include <IL/il.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

GLint TextureFromFile(const char* path, string directory, bool gamma = false);

// Based on: https://raw.githubusercontent.com/JoeyDeVries/LearnOpenGL/master/includes/learnopengl/model.h

class Model {
public:
	vector<Texture> texturesLoaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	// Takes a file path to a 3D model
	Model(string const & path, bool gamma = false) : gammaCorrection(gamma) {
		this->LoadModel(path);
	}

	// Draws model
	void Draw(Shader shader);

private:
	// Loads a model with supported ASSIMP extensions from a file and stores the resulting meshes in the meshes vector.
	void LoadModel(string path);

	void ProcessNode(aiNode* node, const aiScene* scene);

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	// Checks all material textures of a given type and loads the textures if they're not loaded yet
	vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};