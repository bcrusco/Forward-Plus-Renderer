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
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE); // THis won't mess with transparancy right?

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
	workGroupsX = (SCREEN_SIZE.x + (SCREEN_SIZE.x % 16)) / 16;
	workGroupsY = (SCREEN_SIZE.y + (SCREEN_SIZE.y % 16)) / 16;

	size_t numberOfTiles = workGroupsX * workGroupsY;

	glGenBuffers(1, &lightBuffer);
	glGenBuffers(1, &visibleLightIndicesBuffer);

	// TODO: Double check all of these sizes
	// bind light buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

	// bind visible light indices buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * (size_t)16 * 1024, 0, GL_STATIC_DRAW);
	// TODO: Remember to pay attention to if this ends up being signed or unsigned int (right now its signed to mark the end)
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleIndex) * 1024, 0, GL_DYNAMIC_DRAW); //TODO: Dynamic or static draw?

	SetupLights();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

glm::vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen) {
	glm::vec3 position = glm::vec3(0.0);
	for (int i = 0; i < 3; i++) {
		float min = lightMinBounds[i];
		float max = lightMaxBounds[i];
		position[i] = dis(gen) * (max - min) + min;
	}

	return position;
}

void SetupLights() {
	if (lightBuffer == 0) {
		return;
	}
	
	// Set the random seed
	//srand(time(NULL));
	random_device rd;
	mt19937 gen(rd());
	uniform_real_distribution<> dis(0, 1);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < NUM_LIGHTS; i++) {
		PointLight &light = pointLights[i];
		light.position = glm::vec4(RandomPosition(dis, gen), 1.0);
		light.color = glm::vec4(1.0f + dis(gen), 1.0f + dis(gen), 1.0f + dis(gen), 1.0f);
		light.paddingAndRadius = glm::vec4(glm::vec3(0.0f), LIGHT_RADIUS);
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void UpdateLights(float deltaTime) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *pointLights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	for (int i = 0; i < NUM_LIGHTS; i++) {
		PointLight &light = pointLights[i];
		float min = lightMinBounds[1];
		float max = lightMaxBounds[1];

		light.position.y = fmod((light.position.y + lightDeltaTime - min + max), max) + min;
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

/*
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
*/

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
	//camera.ProcessMouseScroll(y);
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

	Shader particleShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\particle.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\particle.frag.glsl", NULL);

	/*
	Shader depthShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.vert.glsl",
	"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.frag.glsl",
	"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\shadow_map_depth.geom.glsl");
	*/

	// Set texture samples
	//shader.Use();
	//computeShader.Use(); // Wait, is accum going to use this or compute? Compute should.. So add it there and use that shader
	// TODO: Add this uniform to accum shader
	// Should be set to 4 I believe, as 0-3 are taken by assimp
	//glUniform1i(glGetUniformLocation(computeShader.Program, "u_depthMap"), 4);


	// So we need to create a depth map FBO
	// This will be used in the depth pass
	const GLuint SCREEN_WIDTH = SCREEN_SIZE.x, SCREEN_HEIGHT = SCREEN_SIZE.y;
	// TODO: Have I actually even confirmed that this is 100% working now?
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
	//Model testModel("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\nanosuit\\nanosuit.obj");
	Model testModel("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\crytek-sponza\\sponza.obj");
	// init scene stuff (set up buffers for culling)
	InitScene();
	particleShader.Use();
	LoadLights(particleShader);




	// run while the window is open
	while (!glfwWindowShouldClose(gWindow)) {
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Movement();

		UpdateLights(0.0f);

		glm::mat4 model;
		//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glm::mat4 projection = glm::perspective(camera.zoom, (float)SCREEN_SIZE.x / (float)SCREEN_SIZE.y, 0.1f, 300.0f);
		glm::mat4 view = camera.GetViewMatrix();
		// Step 1: Render the depth of the scene to texture
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


		// What texture do I have to bind? nothing for now?
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(computeShader.Program, "u_depthTexture"), 4);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(glGetUniformLocation(computeShader.Program, "lightCount"), NUM_LIGHTS);


		glm::vec2 screenSize = SCREEN_SIZE;
		glUniform2fv(glGetUniformLocation(computeShader.Program, "screenSize"), 1, &screenSize[0]);
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 viewProjection = projection * view;
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightIndicesBuffer);


		// do the compute dispatch
		glDispatchCompute(workGroupsX, workGroupsY, 1);

		//glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);





		// Does it wait for the compute to be finished before proceeding?

		/*
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
		VisibleIndex *visibleIndices = (VisibleIndex*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		size_t numberOfTiles = workGroupsX * workGroupsY;
		for (int i = 0; i < numberOfTiles; i++) {

			VisibleIndex &test = visibleIndices[i * 1024];

			if (test.index != -1) {
				int debu1 = 2;
				for (int j = 0; j < 1024; j++) {
					VisibleIndex &test1 = visibleIndices[(i * 1024) + j];
					if (test1.index != -1) {
						int debug = 1;
					}
					
				}
			}
		}
		*/
		
		/*
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
		PointLight *lights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		size_t numberOfTiles = workGroupsX * workGroupsY;
		for (int i = 0; i < NUM_LIGHTS; i++) {

			PointLight &test = lights[i];
			int debu1 = 2;
		}
		*/



		// TODO: Triple look into to this stuff and whether it is being used correctly or is needed to do the accumulate stuff

		// Accumulate the light and render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

		glUniform1i(glGetUniformLocation(shader.Program, "numberOfTilesX"), workGroupsX);
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(shader.Program, "u_viewPosition"), 1, &camera.position[0]);

		// Testing. Disable normal maps by default (will be enabled if they exist) by sending -1
		glUniform1i(glGetUniformLocation(shader.Program, "texture_normal1"), -1);

		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));

		testModel.Draw(shader);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		//glDisable(GL_BLEND);




		/*
		// render lights as billboarded particles
		particleShader.Use();

		glUniformMatrix4fv(glGetUniformLocation(particleShader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(particleShader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(particleShader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(shader.Program, "u_viewPosition"), 1, &camera.position[0]);

		DrawLights(particleShader);
		*/




		// render the light sources/
		/*
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
		PointLight *lights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

		for (int i = 0; i < NUM_LIGHTS; i++) {

			PointLight &test = lights[i];

			//glm::vec3 pos = glm::vec3(-400.0, 5.0, 0.0);
			model = glm::mat4();
			model = glm::translate(model, glm::vec3(test.position));
			//model = glm::scale(model, glm::vec3(0.1f));
			glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
			RenderQuad();
		}
		
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		*/


		glfwSwapBuffers(gWindow);
	}

	// clean up and exit
	glfwTerminate();

	return 0;
}

void LoadLights(Shader shader) {
	// Put the three verticies into the VBO

	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	std::vector<float> lightRadii;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *lights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	// Need to get all the data
	for (int i = 0; i < NUM_LIGHTS; i++) {
		PointLight &light = lights[i];
		glm::vec3 posiion = glm::vec3(light.position);
		glm::vec3 color = glm::vec3(light.color);
		float radius = light.paddingAndRadius.w;

		lightPositions.push_back(posiion);
		lightColors.push_back(color);
		lightRadii.push_back(radius);
	}


	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);

	glGenBuffers(1, &gBufLightPosition);
	glBindBuffer(GL_ARRAY_BUFFER, gBufLightPosition);
	glBufferData(GL_ARRAY_BUFFER, NUM_LIGHTS * sizeof(glm::vec3), \
		&lightPositions, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shader.Program, "position"));
	glVertexAttribPointer(glGetAttribLocation(shader.Program, "position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gBufLightColor);
	glBindBuffer(GL_ARRAY_BUFFER, gBufLightColor);
	glBufferData(GL_ARRAY_BUFFER, NUM_LIGHTS * sizeof(glm::vec3), \
		&lightColors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shader.Program, "color"));
	glVertexAttribPointer(glGetAttribLocation(shader.Program, "color"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gBufLightRadius);
	glBindBuffer(GL_ARRAY_BUFFER, gBufLightRadius);
	glBufferData(GL_ARRAY_BUFFER, NUM_LIGHTS * sizeof(float), \
		&lightRadii, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shader.Program, "radii"));
	glVertexAttribPointer(glGetAttribLocation(shader.Program, "radii"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void DrawLights(Shader shader) {
	std::vector<glm::vec3> lightPositions;
	std::vector<glm::vec3> lightColors;
	std::vector<float> lightRadii;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	PointLight *lights = (PointLight*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

	// Need to get all the data
	for (int i = 0; i < NUM_LIGHTS; i++) {
		PointLight &light = lights[i];
		glm::vec3 posiion = glm::vec3(light.position);
		glm::vec3 color = glm::vec3(light.color);
		float radius = light.paddingAndRadius.w;

		lightPositions.push_back(posiion);
		lightColors.push_back(color - 1.0f);
		lightRadii.push_back(radius);
	}


	// need a new shader program
	//shader.Use();

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// TODO: set model, view and projection here or outside?


	glBindVertexArray(gVAO);


	//bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, gBufLightPosition);
	glBufferData(GL_ARRAY_BUFFER, lightPositions.size() * sizeof(glm::vec3), &lightPositions[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shader.Program, "position"));
	glVertexAttribPointer(glGetAttribLocation(shader.Program, "position"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, gBufLightRadius);
	glBufferData(GL_ARRAY_BUFFER, lightRadii.size() * sizeof(float), &lightRadii[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shader.Program, "radii"));
	glVertexAttribPointer(glGetAttribLocation(shader.Program, "radii"), 1, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, gBufLightColor);
	glBufferData(GL_ARRAY_BUFFER, lightColors.size() * sizeof(glm::vec3), &lightColors[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(glGetAttribLocation(shader.Program, "color"));
	glVertexAttribPointer(glGetAttribLocation(shader.Program, "color"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_POINTS, 0, NUM_LIGHTS);

	// unbind the VAO
	glBindVertexArray(0);

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


// RenderQuad() Renders a 1x1 quad in NDC
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad() {
	if (quadVAO == 0) {
		// positions
		glm::vec3 pos1(-1.0, 1.0, 0.0);
		glm::vec3 pos2(-1.0, -1.0, 0.0);
		glm::vec3 pos3(1.0, -1.0, 0.0);
		glm::vec3 pos4(1.0, 1.0, 0.0);
		// texture coordinates
		glm::vec2 uv1(0.0, 1.0);
		glm::vec2 uv2(0.0, 0.0);
		glm::vec2 uv3(1.0, 0.0);
		glm::vec2 uv4(1.0, 1.0);
		// normal vector
		glm::vec3 nm(0.0, 0.0, 1.0);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// - triangle 1
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// - triangle 2
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);


		GLfloat quadVertices[] = {
			// Positions            // normal         // TexCoords  // Tangent                          // Bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (GLvoid*)(11 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
