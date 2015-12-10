#include "main.h"

void InitGLFW(int argc, char* argv[]) {
	if (!glfwInit()) {
		throw std::runtime_error("glfwInit failed");
	}

	// Open and configure a window with GLFW
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	//glfwWindowHint(GLFW_SAMPLES, 4);

	gWindow = glfwCreateWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y, "Forward+ Renderer", NULL, NULL);
	if (!gWindow) {
		throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 4.5?");
	}
	glfwMakeContextCurrent(gWindow);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		throw std::runtime_error("glewInit failed");
	}

	// Print out some info about the graphics drivers
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

	// Make sure OpenGL version 4.5 API is available
	if (!GLEW_VERSION_4_5) {
		throw std::runtime_error("OpenGL 4.5 API is not available.");
	}

	// Enable any OpenGL features we want to use
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_MULTISAMPLE);

	// Set mouse and keyboard callback functions
	glfwSetKeyCallback(gWindow, KeyCallback);
	glfwSetCursorPosCallback(gWindow, MouseCallback);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleIndex) * 1024, 0, GL_STATIC_DRAW);

	SetupLights();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

glm::vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen) {
	glm::vec3 position = glm::vec3(0.0);
	for (int i = 0; i < 3; i++) {
		float min = lightMinBounds[i];
		float max = lightMaxBounds[i];
		position[i] = (GLfloat)dis(gen) * (max - min) + min;
	}

	return position;
}

void SetupLights() {
	if (lightBuffer == 0) {
		return;
	}
	
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

void UpdateLights() {
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

void Movement() {
	// Camera controls
	if (keys[GLFW_KEY_W]) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (keys[GLFW_KEY_S]) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (keys[GLFW_KEY_A]) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (keys[GLFW_KEY_D]) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

static void KeyCallback(GLFWwindow *window, int key, int scanCode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key <= 1024) {
		if (action == GLFW_PRESS) {
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			keys[key] = false;
			keysPressed[key] = false;
		}
	}
}

static void MouseCallback(GLFWwindow *window, double x, double y) {
	if (firstMouse) {
		lastX = (GLfloat)x;
		lastY = (GLfloat)y;
		firstMouse = false;
	}

	GLfloat xoffset = (GLfloat)x - lastX;
	GLfloat yoffset = lastY - (GLfloat)y;

	lastX = (GLfloat)x;
	lastY = (GLfloat)y;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

int main(int argc, char **argv) {
	InitGLFW(argc, argv);

	Shader depthShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth.frag.glsl", NULL);
	Shader computeShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_cull.comp.glsl");
	Shader shader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\vertex.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\final.frag.glsl", NULL);


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
	Model sponzaModel("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\crytek-sponza\\sponza.obj");
	// init scene stuff (set up buffers for culling)
	InitScene();

	// Scale and get the model view transformation matrix
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

	// For each shader, bind the uniforms that will during the program's execution
	depthShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));

	computeShader.Use();
	glUniform1i(glGetUniformLocation(computeShader.Program, "lightCount"), NUM_LIGHTS);
	glUniform2iv(glGetUniformLocation(computeShader.Program, "screenSize"), 1, &SCREEN_SIZE[0]);

	shader.Use();
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader.Program, "numberOfTilesX"), workGroupsX);

	// Run while the window is open
	while (!glfwWindowShouldClose(gWindow)) {
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Movement();

		// Update light positions
		UpdateLights();

		// Calculate new projection and view matrices and save current cameraPosition
		glm::mat4 projection = glm::perspective(camera.zoom, (float)SCREEN_SIZE.x / (float)SCREEN_SIZE.y, 0.1f, 300.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::vec3 cameraPosition = camera.position;

		// Step 1: Render the depth of the scene to texture
		depthShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		sponzaModel.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Step 2: Perform light culling on point lights in the scene
		computeShader.Use();
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

		// What texture do I have to bind? nothing for now?
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(computeShader.Program, "depthMap"), 4);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		


		
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(computeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightIndicesBuffer);


		// do the compute dispatch
		glDispatchCompute(workGroupsX, workGroupsY, 1);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);



		// TODO: Triple look into to this stuff and whether it is being used correctly or is needed to do the accumulate stuff

		// Step 3: Accumulate the remaining lights after culling and render
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.Use();
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

		
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "u_view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(shader.Program, "u_viewPosition"), 1, &cameraPosition[0]);
		

		sponzaModel.Draw(shader);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		//glDisable(GL_BLEND);
		
		glfwSwapBuffers(gWindow);
	}

	// Clean up and exit
	glfwTerminate();

	return 0;
}