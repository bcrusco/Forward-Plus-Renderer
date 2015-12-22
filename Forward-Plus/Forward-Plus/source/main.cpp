#include "main.h"

void InitGLFW(int argc, char* argv[]) {
	if (!glfwInit()) {
		throw std::runtime_error("glfwInit failed");
	}

	// Open and configure a window with GLFW
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

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
	glEnable(GL_MULTISAMPLE);

	// Set mouse and keyboard callback functions
	glfwSetKeyCallback(gWindow, KeyCallback);
	glfwSetCursorPosCallback(gWindow, MouseCallback);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void InitScene() {
	// Define work group sizes in x and y direction based off screen size and tile size (in pixels)
	workGroupsX = (SCREEN_SIZE.x + (SCREEN_SIZE.x % 16)) / 16;
	workGroupsY = (SCREEN_SIZE.y + (SCREEN_SIZE.y % 16)) / 16;
	size_t numberOfTiles = workGroupsX * workGroupsY;

	// Generate our shader storage buffers
	glGenBuffers(1, &lightBuffer);
	glGenBuffers(1, &visibleLightIndicesBuffer);

	// Bind light buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), 0, GL_DYNAMIC_DRAW);

	// Bind visible light indices buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleLightIndicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleIndex) * 1024, 0, GL_STATIC_DRAW);

	// Set the default values for the light buffer
	SetupLights();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

glm::vec3 RandomPosition(uniform_real_distribution<> dis, mt19937 gen) {
	glm::vec3 position = glm::vec3(0.0);
	for (int i = 0; i < 3; i++) {
		float min = LIGHT_MIN_BOUNDS[i];
		float max = LIGHT_MAX_BOUNDS[i];
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
		light.position = glm::vec4(RandomPosition(dis, gen), 1.0f);
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
		float min = LIGHT_MIN_BOUNDS[1];
		float max = LIGHT_MAX_BOUNDS[1];

		light.position.y = fmod((light.position.y + (-4.5f * deltaTime) - min + max), max) + min;
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

	GLfloat xOffset = (GLfloat)x - lastX;
	GLfloat yOffset = lastY - (GLfloat)y;

	lastX = (GLfloat)x;
	lastY = (GLfloat)y;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

void DrawQuad() {
	if (quadVAO == 0) {
		GLfloat quadVertices[] = {
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

GLuint LoadCubemap(vector<const GLchar*> faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);

	ilInit();
	ILuint imageID;
	ilGenImages(1, &imageID);

	int width, height;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++) {
		// TODO: replace with devil 

		ilBindImage(imageID);
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

		ilLoadImage((ILstring)faces[i]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ilGetData());
		ilDeleteImages(1, &imageID);

		//image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		//glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		//SOIL_free_image_data(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

int main(int argc, char **argv) {
	InitGLFW(argc, argv);

	// TODO: Make these directories relative to root directory of project
	Shader depthShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth.frag.glsl", NULL);
	Shader lightCullingShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_culling.comp.glsl");

#if defined(DEPTH_DEBUG)
	Shader depthDebugShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth_debug.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\depth_debug.frag.glsl", NULL);
#elif defined(LIGHT_DEBUG)
	Shader lightDebugShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_debug.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_debug.frag.glsl", NULL);
#else
	Shader lightAccumulationShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_accumulation.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\light_accumulation.frag.glsl", NULL);
	Shader skyboxShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\skybox.vert.glsl", 
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\skybox.frag.glsl", NULL);
	Shader hdrShader("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\hdr.vert.glsl",
		"D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\source\\shaders\\hdr.frag.glsl", NULL);
#endif

	// So we need to create a depth map FBO
	// This will be used in the depth pass
	// Create a depth map frame buffer object and texture
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCREEN_SIZE.x, SCREEN_SIZE.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if defined(DEPTH_DEBUG)
#elif defined(LIGHT_DEBUG)
#else

	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	// Set up and load the cubemap / skybox
	vector<const GLchar*> faces;
	faces.push_back("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\skybox\\starfield_rt.jpg");
	faces.push_back("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\skybox\\starfield_lf.jpg");
	faces.push_back("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\skybox\\starfield_up.jpg");
	faces.push_back("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\skybox\\starfield_dn.jpg");
	faces.push_back("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\skybox\\starfield_bk.jpg");
	faces.push_back("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\skybox\\starfield_ft.jpg");
	GLuint cubemapTexture = LoadCubemap(faces);


	// Create a floating point HDR frame buffer and a floating point color buffer (as a texture)
	GLuint hdrFBO;
	glGenFramebuffers(1, &hdrFBO);

	GLuint colorBuffer;
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_SIZE.x, SCREEN_SIZE.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// It will also need a depth component as a render buffer, attached to the hdrFBO
	GLuint rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_SIZE.x, SCREEN_SIZE.y);

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif

	// Load in our scene models
	// TODO: Want to replace this path thing with a pay to do this agnostic of what the file path of the project is
	// Modified the Crytek Sponza model for use in our scene
	// http://www.crytek.com/cryengine/cryengine3/downloads
	Model sponzaModel("D:\\Git\\Forward-Plus-Renderer\\Forward-Plus\\Forward-Plus\\crytek-sponza\\sponza.obj");

	// Initialize the scene by setting up the buffers and assigning default values
	InitScene();

	// Scale and get the model view transformation matrix
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));

	// For each shader, bind the uniforms that will during the program's execution
	depthShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCullingShader.Use();
	glUniform1i(glGetUniformLocation(lightCullingShader.Program, "lightCount"), NUM_LIGHTS);
	glUniform2iv(glGetUniformLocation(lightCullingShader.Program, "screenSize"), 1, &SCREEN_SIZE[0]);

#if defined(DEPTH_DEBUG)
	depthDebugShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(depthDebugShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(glGetUniformLocation(depthDebugShader.Program, "near"),  NEAR_PLANE);
	glUniform1f(glGetUniformLocation(depthDebugShader.Program, "far"),  FAR_PLANE);
#elif defined(LIGHT_DEBUG)
	lightDebugShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(lightDebugShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(lightDebugShader.Program, "totalLightCount"), NUM_LIGHTS);
	glUniform1i(glGetUniformLocation(lightDebugShader.Program, "numberOfTilesX"), workGroupsX);
#else
	lightAccumulationShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(lightAccumulationShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(lightAccumulationShader.Program, "numberOfTilesX"), workGroupsX);
#endif

	// Set viewport dimensions and background color
	glViewport(0, 0, SCREEN_SIZE.x, SCREEN_SIZE.y);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Run while the window is open
	while (!glfwWindowShouldClose(gWindow)) {
		// Calculate the delta time used for movement (not for animating the lights)
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check and call events
		glfwPollEvents();
		Movement();

		// Update light positions
		UpdateLights();

		// Calculate new projection and view matrices and save current cameraPosition
		glm::mat4 projection = glm::perspective(camera.zoom, (float)SCREEN_SIZE.x / (float)SCREEN_SIZE.y, NEAR_PLANE, FAR_PLANE);
		glm::mat4 view = camera.GetViewMatrix();
		glm::vec3 cameraPosition = camera.position;

		// Step 1: Render the depth of the scene to a depth map
		depthShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(depthShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		
		// Bind the depth map's frame buffer and draw the depth map to it
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		sponzaModel.Draw(depthShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Step 2: Perform light culling on point lights in the scene
		lightCullingShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(lightCullingShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(lightCullingShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		// Bind depth map texture to texture location 4 (which will not be used by any model texture)
		glActiveTexture(GL_TEXTURE4);
		glUniform1i(glGetUniformLocation(lightCullingShader.Program, "depthMap"), 4);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		
		// Bind shader storage buffer objects for the light and indice buffers
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, visibleLightIndicesBuffer);


		// Dispatch the compute shader, using the workgroup values calculated earlier
		glDispatchCompute(workGroupsX, workGroupsY, 1);

		// Unbind the depth map
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Step 3: Accumulate the remaining lights after culling and render (or execute one of the debug views of a flag is enabled
		// (Or run one of the debug shaders)
		
		// Depth debug shader
#if defined(DEPTH_DEBUG)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		depthDebugShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(depthDebugShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(depthDebugShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

		sponzaModel.Draw(depthDebugShader);
#elif defined(LIGHT_DEBUG)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightDebugShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(lightDebugShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(lightDebugShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(lightDebugShader.Program, "viewPosition"), 1, &cameraPosition[0]);

		sponzaModel.Draw(lightDebugShader);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
#else
		// We render the scene into the floating point HDR frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lightAccumulationShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(lightAccumulationShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(lightAccumulationShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform3fv(glGetUniformLocation(lightAccumulationShader.Program, "viewPosition"), 1, &cameraPosition[0]);

		// Blending might be needed if we add additional passes, but may not work correctly with objects with transparency masks
		// Disabled for now
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);

		sponzaModel.Draw(lightAccumulationShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//glDisable(GL_BLEND);


		// Draw skybox after scene (should it be before or after HDR?)
		glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);

		//glUniform1i(glGetUniformLocation(shader.Program, "skybox"), 0);

		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // Set depth function back to default


		// Tonemap the HDR colors to the default framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Weirdly, moving this call drops performance into the floor
		hdrShader.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glUniform1f(glGetUniformLocation(hdrShader.Program, "exposure"), exposure);
		DrawQuad();

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
#endif

		glfwSwapBuffers(gWindow);
	}

	// Clean up and exit
	glfwTerminate();

	return 0;
}