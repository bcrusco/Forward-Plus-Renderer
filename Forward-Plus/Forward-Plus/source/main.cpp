#include "main.h"



GLuint shaderProgram;

GLuint locationPosition;
GLuint locationColor;
GLuint locationNormal;
GLuint locationSize;

GLuint unifProjection;
GLuint unifView;
GLuint unifModel;
GLuint unifModelInvTr;
GLuint unifLightPosition;
GLuint unifLightColor;
GLuint unifCamPosition;

// Camera setup
glm::vec3 camEye = glm::vec3(0, 0, 3);
glm::vec3 camDir = glm::vec3(0, -5, 0);
glm::vec3 camUp = glm::vec3(0, -1, 0);
GLfloat yaw1 = -90.0f;
GLfloat pitch1 = 0.0f;

// Temp
GLuint woodTexture;

void initGLFW(int argc, char* argv[]) {
	if (!glfwInit()) {
		throw std::runtime_error("glfwInit failed");
	}

	// open a window with GLFW
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	gWindow = glfwCreateWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y, "Forward+ Renderer", NULL, NULL);
	if (!gWindow) {
		throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 4.5?");
	}
	glfwMakeContextCurrent(gWindow);

	glewExperimental = GL_TRUE; //TODO: Was for an OSX bug. Can I remove?
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("glewInit failed");
	}
	// print out some info about the graphics drivers
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	// make sure OpenGL version 4.5 API is available
	if (!GLEW_VERSION_4_5) {
		throw std::runtime_error("OpenGL 4.5 API is not available.");
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glfwSetKeyCallback(gWindow, keyCallback);
	glfwSetCursorPosCallback(gWindow, mouseCallback);
	glfwSetScrollCallback(gWindow, scrollCallback);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void printShaderInfoLog(int shader) {
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_TRUE) {
		return;
	}
	std::cerr << "GLSL COMPILE ERROR" << std::endl;

	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

	if (infoLogLen > 0) {
		infoLog = new GLchar[infoLogLen];
		glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
		std::cerr << "InfoLog:" << std::endl << infoLog << std::endl;
		delete[] infoLog;
	}
	// Throw here to allow the debugger to track error.
	throw;
}

void printLinkInfoLog(int program) {
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (linked == GL_TRUE) {
		return;
	}
	std::cerr << "GLSL LINK ERROR" << std::endl;

	int infoLogLen = 0;
	int charsWritten = 0;
	GLchar *infoLog;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);

	if (infoLogLen > 0) {
		infoLog = new GLchar[infoLogLen];
		glGetProgramInfoLog(program, infoLogLen, &charsWritten, infoLog);
		std::cerr << "InfoLog:" << std::endl << infoLog << std::endl;
		delete[] infoLog;
	}
	// Throw here to allow the debugger to track error.
	throw;
}


void initShaders() {
	// sets up all the different shaders we need.
	// need at least: blinn phong, ambient, shadow map ones, compute for light culling, and light acumulation

	// then do camera setup






	std::string vertSourceS = TextFileRead("./shaders/blinn_phong.vert.glsl");
	std::string fragSourceS = TextFileRead("./shaders/blinn_phong.frag.glsl");

	const char *vertSource = vertSourceS.c_str();
	const char *fragSource = fragSourceS.c_str();

	GLuint shadVert = glCreateShader(GL_VERTEX_SHADER);
	GLuint shadFrag = glCreateShader(GL_FRAGMENT_SHADER);
	shaderProgram = glCreateProgram();

	// Load and compiler each shader program
	// Then check to make sure the shaders complied correctly

	// - Vertex shader
	glShaderSource(shadVert, 1, &vertSource, NULL);
	glCompileShader(shadVert);
	printShaderInfoLog(shadVert);
	// - Diffuse fragment shader
	glShaderSource(shadFrag, 1, &fragSource, NULL);
	glCompileShader(shadFrag);
	printShaderInfoLog(shadFrag);

	// Link the shader programs together from compiled bits
	glAttachShader(shaderProgram, shadVert);
	glAttachShader(shaderProgram, shadFrag);
	glLinkProgram(shaderProgram);
	printLinkInfoLog(shaderProgram);

	// Clean up the shaders now that they are linked
	glDetachShader(shaderProgram, shadVert);
	glDetachShader(shaderProgram, shadFrag);
	glDeleteShader(shadVert);
	glDeleteShader(shadFrag);

	// Find out what the GLSL locations are, since we can't pre-define these
	locationPosition = glGetAttribLocation(shaderProgram, "vs_Position");
	//    locationNor    = glGetAttribLocation (shaderProgram, "vs_Normal");
	locationColor = glGetAttribLocation(shaderProgram, "vs_Color");
	locationSize = glGetAttribLocation(shaderProgram, "vs_Size");

	unifProjection = glGetUniformLocation(shaderProgram, "u_projection");
	unifView = glGetUniformLocation(shaderProgram, "u_view");
	unifModel = glGetUniformLocation(shaderProgram, "u_model");

	// then do camera setup
	glUseProgram(shaderProgram);

	glm::mat4 view = glm::lookAt(camEye, camEye + camDir, camUp);
	glUniformMatrix4fv(unifView, 1, GL_FALSE, &view[0][0]);

	glm::mat4 projection = glm::perspective<float>(50.0, (float)SCREEN_SIZE.x / SCREEN_SIZE.y, 0.1f, 2000.0f);
	glUniformMatrix4fv(unifProjection, 1, GL_FALSE, &projection[0][0]);

	glUseProgram(0);
}

GLuint loadTexture(GLchar* imagepath) {
	GLuint textureId;
	glGenTextures(1, &textureId);
	int width, height;
	unsigned char* image = SOIL_load_image(imagepath, &width, &height, 0, SOIL_LOAD_RGB);

	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);

	return textureId;
}

bool keys[1024];
bool keysPressed[1024];

void Movement() {
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key <= 1024) {
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE) {
			keys[key] = false;
			keysPressed[key] = false;
		}
	}
}

static void mouseCallback(GLFWwindow* window, double x, double y) {
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;

	lastX = x;
	lastY = y;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

static void scrollCallback(GLFWwindow* window, double x, double y) {
	camera.ProcessMouseScroll(y);
}

std::string TextFileRead(const char *filename) {
	std::ifstream in(filename, std::ios::in);
	if (!in) {
		std::cerr << "Error reading file" << std::endl;
		throw (errno);
	}
	return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

int main(int argc, char **argv) {
	initGLFW(argc, argv);

	// Need to do some sort of scene loading

	// need to display the plane. do I load a shader?

	// need to load the lights and all that jazz


	Shader shader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\vertex.vert.glsl", 
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\fragment.frag.glsl", nullptr);
	Shader depthShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.vert.glsl", 
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.frag.glsl", 
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.geom.glsl");

	shader.Use();
	//glUniform1i(glGetUniformLocation(shader.Program, "u_diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader.Program, "u_depthMap"), 1);


	// light
	glm::vec3 lightPos(2.3f, -1.6f, -3.0f);

	//woodTexture = loadTexture("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\wood.png");

	// Configure depth map FBO
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// Create depth cubemap texture
	GLuint depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (GLuint i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// Attach cubemap as depth map FBO's color buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


	// TODO: Want to replace this path thing with a pay to do this agnostic of what the file path of the project is
	// This whole path thing might be wrong
	Model testModel("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\nanosuit\\nanosuit.obj");

	// run while the window is open
	while (!glfwWindowShouldClose(gWindow)) {

		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		// TODO add any movement call backs
		Movement();

		// 0. Create depth cubemap transformation matrices
		GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
		GLfloat near = 1.0f;
		GLfloat far = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

		// 1. Render scene to depth cubemap
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		depthShader.Use();
		for (GLuint i = 0; i < 6; ++i)
			glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, ("u_shadowTransforms[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
		glUniform1f(glGetUniformLocation(depthShader.Program, "u_farPlane"), far);
		glUniform3fv(glGetUniformLocation(depthShader.Program, "u_lightPosition"), 1, &lightPos[0]);

		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// It's a bit too big for our scene, so scale it down
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shader.Program, "u_reverseNormals"), 0);
		testModel.Draw(depthShader);
		//RenderScene(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Render scene as normal 
		glViewport(0, 0, SCREEN_SIZE.x, SCREEN_SIZE.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();
		glm::mat4 projection = glm::perspective(camera.zoom, (float)SCREEN_SIZE.x / (float)SCREEN_SIZE.y, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		// Set light uniforms
		glUniform3fv(glGetUniformLocation(shader.Program, "u_lightPosition"), 1, &lightPos[0]);
		glUniform3fv(glGetUniformLocation(shader.Program, "u_viewPosition"), 1, &camera.position[0]);
		// Enable/Disable shadows by pressing 'SPACE'
		glUniform1i(glGetUniformLocation(shader.Program, "u_shadows"), 1);
		glUniform1f(glGetUniformLocation(shader.Program, "u_farPlane"), far);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, woodTexture);
		//glActiveTexture(GL_TEXTURE1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shader.Program, "u_reverseNormals"), 0);
		testModel.Draw(shader);
		//RenderScene(shader);

		glfwSwapBuffers(gWindow);
	}

	// clean up and exit
	glfwTerminate();

	return 0;
}