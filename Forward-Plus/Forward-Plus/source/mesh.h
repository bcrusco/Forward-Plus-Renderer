#pragma once

#include <iostream>
#include <vector>
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <assimp\Scene.h>

struct Vertex {
	glm::vec3 vertexCoordinate;
	glm::vec3 normal;

	Vertex();
	Vertex(glm::vec3 v, glm::vec3 n) {
		vertexCoordinate = v;
		normal = n;
	}
};


class Mesh {
public:
	Mesh() {
		
	}

	~Mesh() {

	}

	void LoadMesh(const std::string& filename);

	void Render();

	int GetNumVertices(int index);
	std::vector<glm::vec3> GetTriangles(int index);
	std::vector<glm::vec3> GetNormals(int index);

private:
	void InitFromScene(const aiScene* pScene);
	void InitMesh(unsigned int index, const aiMesh* paiMesh);
	void Clear();

	struct MeshEntry {
		GLuint vb;
		GLuint ib;
		unsigned int numIndices;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		MeshEntry();

		~MeshEntry();

		void Init(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	};

	std::vector<MeshEntry> m_Entries;
};