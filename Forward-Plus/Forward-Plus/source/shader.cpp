#include "shader.h"

// Based on: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/shader.h

Shader::Shader(const GLchar* computePath) {
	std::string computeCode;
	std::ifstream computeShaderFile;

	// Ensure that ifstream objects can throw exceptions
	computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		// Open file
		computeShaderFile.open(computePath);
		std::stringstream cShaderStream;

		// Read file;s buffer contents into streams
		cShaderStream << computeShaderFile.rdbuf();

		// close file handlers
		computeShaderFile.close();

		// Convert stream to string
		computeCode = cShaderStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	// Then compile shader
	const GLchar* computeShaderCode = computeCode.c_str();

	GLuint compute;

	// Compute shader
	compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &computeShaderCode, NULL);
	glCompileShader(compute);
	CheckCompileErrors(compute, "COMPUTE");

	// Create the shader program
	this->Program = glCreateProgram();
	glAttachShader(this->Program, compute);

	glLinkProgram(this->Program);
	CheckCompileErrors(this->Program, "PROGRAM");

	// No longer need the shaders, delete them
	glDeleteShader(compute);
}

Shader::Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr) {
	// First retrieve the vertex and fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vertexShaderFile;
	std::ifstream fragmentShaderFile;
	std::ifstream geometryShaderFile;

	// Ensure that ifstream objects can throw exceptions
	vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	geometryShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		// Open files
		vertexShaderFile.open(vertexPath);
		fragmentShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;

		// Read file's buffer contents into streams
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();

		// Close file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();

		// Convert stream to string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		// Load geometryPath if one was provided
		if (geometryPath != nullptr) {
			geometryShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}

	// Then compile shaders
	const GLchar* vertexShaderCode = vertexCode.c_str();
	const GLchar* fragmentShaderCode = fragmentCode.c_str();
	
	GLuint vertex, fragment;

	// Vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexShaderCode, NULL);
	glCompileShader(vertex);
	CheckCompileErrors(vertex, "VERTEX");

	// Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
	glCompileShader(fragment);
	CheckCompileErrors(fragment, "FRAGMENT");

	// Compile the geometry shader if we have one
	GLuint geometry;
	if (geometryPath != nullptr) {
		const GLchar * geometryShaderCode = geometryCode.c_str();

		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometryShaderCode, NULL);
		glCompileShader(geometry);
		CheckCompileErrors(geometry, "GEOMETRY");
	}

	// Create the shader program
	this->Program = glCreateProgram();
	glAttachShader(this->Program, vertex);
	glAttachShader(this->Program, fragment);

	if (geometryPath != nullptr) {
		glAttachShader(this->Program, geometry);
	}

	glLinkProgram(this->Program);
	CheckCompileErrors(this->Program, "PROGRAM");

	// No longer need the shaders, delete them
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr) {
		glDeleteShader(geometry);
	}
}

void Shader::Use() {
	glUseProgram(this->Program);
}

void Shader::CheckCompileErrors(GLuint shader, std::string type) {
	GLint success;
	GLchar infoLog[1024];

	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "| ERROR::::SHADER-COMPILATION-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);

		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "| ERROR::::PROGRAM-LINKING-ERROR of type: " << type << "|\n" << infoLog << "\n| -- --------------------------------------------------- -- |" << std::endl;
		}
	}
}