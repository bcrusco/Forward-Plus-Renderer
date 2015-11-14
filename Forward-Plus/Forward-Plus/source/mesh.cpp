#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

//Reference : http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

Mesh::MeshEntry::MeshEntry() {
	numIndices = 0;
};

Mesh::MeshEntry::~MeshEntry() {
	glDeleteBuffers(1, &vb);
	glDeleteBuffers(1, &ib);
}

void Mesh::MeshEntry::Init(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
	numIndices = static_cast<unsigned int>(indices.size());

	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, &indices[0], GL_STATIC_DRAW);
}

void Mesh::Clear() {
	// TODO: Implement. Needed for if we want to overwrite the existing mesh
}

void Mesh::LoadMesh(const std::string& filename) {
	// Release the previously loaded mesh
	Clear();

	// How mesh is read from file can be changed to match requirements
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
	InitFromScene(pScene);
}

void Mesh::InitFromScene(const aiScene* pScene) {
	m_Entries.resize(pScene->mNumMeshes);

	// Initialize each mesh in the scene
	for (unsigned int i = 0; i < m_Entries.size(); i++) {
		const aiMesh* paiMesh = pScene->mMeshes[i];
		InitMesh(i, paiMesh);
	}
}

void Mesh::InitMesh(unsigned int index, const aiMesh* paiMesh) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	const aiVector3D zero3D(0.0f, 0.0f, 0.0f);

	for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
		const aiVector3D* pPos = &(paiMesh->mVertices[i]);
		const aiVector3D* pNormal = paiMesh->HasNormals() ? &(paiMesh->mNormals[i]) : &zero3D;

		Vertex v(glm::vec3(pPos->x, pPos->y, pPos->z),
			glm::vec3(pNormal->x, pNormal->y, pNormal->z));

		vertices.push_back(v);
	}

	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		const aiFace& face = paiMesh->mFaces[i];

		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	m_Entries[index].vertices = vertices;
	m_Entries[index].indices = indices;
	m_Entries[index].Init(vertices, indices);
}

// TODO: Just basic rendering. Needs to be built into shader pipeline?
void Mesh::Render() {
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	for (unsigned int i = 0; i < m_Entries.size(); i++) {
		glBindBuffer(GL_ARRAY_BUFFER, m_Entries[i].vb);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)20);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Entries[i].ib);

		glDrawElements(GL_TRIANGLES, m_Entries[i].numIndices, GL_UNSIGNED_INT, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

int Mesh::GetNumVertices(int index) {
	return m_Entries[index].numIndices;
}

std::vector<glm::vec3> Mesh::GetTriangles(int index) {
	std::vector<glm::vec3> triangleCoordinate;

	for (unsigned int i = 0; i < m_Entries[index].numIndices; ++i) {
		triangleCoordinate.push_back(m_Entries[index].vertices[i].vertexCoordinate);
	}

	return triangleCoordinate;
}

std::vector<glm::vec3> Mesh::GetNormals(int index) {
	std::vector<glm::vec3> normalCoordinate;
	glm::vec3 v1, v2, v3;

	for (unsigned int i = 0; i < m_Entries[index].numIndices; i += 3) {
		v1 = m_Entries[index].vertices[i].vertexCoordinate;
		v2 = m_Entries[index].vertices[i + 1].vertexCoordinate;
		v3 = m_Entries[index].vertices[i + 2].vertexCoordinate;

		normalCoordinate.push_back(glm::normalize(glm::cross((v2 - v1), (v3 - v1))));
	}

	return normalCoordinate;
}