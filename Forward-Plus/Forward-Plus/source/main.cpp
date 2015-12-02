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

glm::vec4 ambient = glm::vec4(glm::vec3(0.3f), 1.0f);

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

	glewExperimental = GL_TRUE;
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

void InitScene() {


	// Need to create render targets (but how many?)
	frameBuffer = new FrameBuffer(SCREEN_SIZE.x, SCREEN_SIZE.y);
	frameBuffer->AttachTexture(GL_COLOR_ATTACHMENT0, GLFMT_A16B16G16R16F);
	frameBuffer->AttachTexture(GL_DEPTH_ATTACHMENT, GLFMT_D32F);


	workGroupsX = (SCREEN_SIZE.x + (SCREEN_SIZE.x % 16)) / 16;
	workGroupsY = (SCREEN_SIZE.y + (SCREEN_SIZE.y % 16)) / 16;

	size_t numberOfTiles = workGroupsX * workGroupsY;

	glGenBuffers(1, &headBuffer);
	glGenBuffers(1, &nodeBuffer);
	glGenBuffers(1, &lightBuffer);
	glGenBuffers(1, &counterBuffer);

	//TODO: Double check all these sizes
	// bind head buffer

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, headBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * (size_t)16, 0, GL_STATIC_DRAW);

	// bind node buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodeBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * (size_t)16 * 1024, 0, GL_STATIC_DRAW);

	// bind light buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

	// TODO: assign values to lights (in future call simulation)
	UpdateLights(0.0f);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	// bind counter buffer
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counterBuffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void UpdateLights(float deltaTime) {
	// this will do some update later, for now just generate lights

	if (lightBuffer == 0) {
		return;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	glm::vec3 lightPosition(2.3f, 10.0f, -3.0f);
	// generate the lights
	int segments = sqrt(NUM_LIGHTS);

	for (int i = 0; i < segments; i++) {
		for (int j = 0; j < segments; j++) {
			PointLight &light = pointLights[i * segments + j];

			light.previous[0] = lightPosition[0];
			light.previous[1] = lightPosition[1];
			light.previous[2] = lightPosition[2];
			light.previous[3] = 1.0f;

			light.velocity = glm::vec3(0.0f);

			light.current = light.previous;

			light.radius = 1.5f;
			light.color = glm::vec3(1.0f);
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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


	// Test compute shader
	Shader computeShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_cull.comp.glsl");

	Shader depthShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth.frag.glsl", NULL);

	Shader shader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\vertex.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\final.frag.glsl", NULL);

	/*
	Shader depthShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.vert.glsl", 
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.frag.glsl", 
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.geom.glsl");
		*/

	// Set texture samples
	//shader.Use();
	computeShader.Use(); // Wait, is accum going to use this or compute? Compute should.. So add it there and use that shader
	// TODO: Add this uniform to accum shader
	// Should be set to 4 I believe, as 0-3 are taken by assimp
	glUniform1i(glGetUniformLocation(computeShader.Program, "u_depthMap"), 4);


	// So we need to create a depth map FBO
	// This will be used in the depth pass
	const GLuint SCREEN_WIDTH = SCREEN_SIZE.x, SCREEN_HEIGHT = SCREEN_SIZE.y;
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// - Create depth texture
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	

	// Load in our scene models
	// TODO: Want to replace this path thing with a pay to do this agnostic of what the file path of the project is
	Model testModel("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\nanosuit\\nanosuit.obj");

	// init scene stuff (set up buffers for culling)
	InitScene();

	// set up culling uniforms
	computeShader.Use();
	glUniform1i(glGetUniformLocation(computeShader.Program, "depthSampler"), 0);
	// What texture do I have to bind? nothing for now?
	//glBindTexture(GL_TEXTURE_2D, computeShader.textures[i].id);
	glUniform1i(glGetUniformLocation(computeShader.Program, "numberOfLights"), NUM_LIGHTS);

	

	// run while the window is open
	while (!glfwWindowShouldClose(gWindow)) {
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Movement();

		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		glm::mat4 projection = glm::perspective(camera.zoom, (float)SCREEN_SIZE.x / (float)SCREEN_SIZE.y, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		// Step 1: Rener the depth of the scene to texture
		depthShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));

		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		testModel.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		


		// Step 2: do light culling
		
		// TODO: test alpha values
		computeShader.Use();

		glUniform1f(glGetUniformLocation(computeShader.Program, "alpha"), 1.0f);
		glm::vec2 clipPlanes = glm::vec2(100.0, -100.0); // TODO: Check this value
		glUniform2fv(glGetUniformLocation(computeShader.Program, "clipPlanes"), 1, &clipPlanes[0]);
		glm::vec2 screenSize = SCREEN_SIZE;
		glUniform2fv(glGetUniformLocation(computeShader.Program, "screenSize"), 1, &screenSize[0]);
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 viewProjection = projection * view;
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));

		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counterBuffer);
		GLuint *counter = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_WRITE_ONLY);

		*counter = 0;
		glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, headBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, nodeBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, counterBuffer);

		// TODO: This especially needs to be changed
		glBindTexture(GL_TEXTURE_2D, frameBuffer->GetDepthAttachment());

		
		// do the compute dispatch
		glDispatchCompute(workGroupsX, workGroupsY, 1);


		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
		glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);


		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(GL_FALSE);

		frameBuffer->Set();

		// TODO: Why is this one here?
		glActiveTexture(GL_TEXTURE0);

		// Accumulate the light and render
		shader.Use();
		glUniform1f(glGetUniformLocation(shader.Program, "alpha"), 1.0f);
		glUniform1i(glGetUniformLocation(shader.Program, "numberOfTilesX"), workGroupsX);
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		// Set light uniforms
		//glUniform3fv(glGetUniformLocation(shader.Program, "u_lightPosition"), 1, &lightPos[0]);
		glUniform3fv(glGetUniformLocation(shader.Program, "u_viewPosition"), 1, &camera.position[0]);

		//glUniform1i(glGetUniformLocation(shader.Program, "u_shadows"), 1);
		//glUniform1f(glGetUniformLocation(shader.Program, "u_farPlane"), far);

		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(shader.Program, "u_reverseNormals"), 0);
		testModel.Draw(shader);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

		frameBuffer->Unset();

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);


		glfwSwapBuffers(gWindow);
	}

	// clean up and exit
	glfwTerminate();

	return 0;
}

