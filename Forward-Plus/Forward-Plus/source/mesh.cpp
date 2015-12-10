#include "mesh.h"

// Based on: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/mesh.h

Mesh::Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures) {
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	// Set vertex buffers and attribute pointers
	this->SetupMesh();
}

void Mesh::Draw(Shader shader) {
	// Bind appropriate textures
	GLuint diffuseNumber = 1;
	GLuint specularNumber = 1;
	GLuint normalNumber = 1;
	GLuint heightNumber = 1;

	for (GLuint i = 0; i < this->textures.size(); i++) {
		// Activate proper texture unit and retreive texture number
		glActiveTexture(GL_TEXTURE0 + i);
		stringstream stream;
		string number;
		string name = this->textures[i].type;

		// Transfer texture data to stream
		if (name == "texture_diffuse") {
			stream << diffuseNumber++;
		}
		else if (name == "texture_specular") {
			stream << specularNumber++;
		}
		else if (name == "texture_normal") {
			stream << normalNumber++;
		}
		else if (name == "texture_height") {
			stream << heightNumber++;
		}
		number = stream.str();

		// Set sampler to the correct texture unit and bind the texture
		glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}

	// Draw mesh
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, (GLsizei)this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Reset to defaults
	for (GLuint i = 0; i < this->textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Mesh::SetupMesh() {
	// Create buffers and arrays
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	// Load data into vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

	// Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, textureCoordinates));

	// Tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));

	// Bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, bitangent));

	glBindVertexArray(0);
}