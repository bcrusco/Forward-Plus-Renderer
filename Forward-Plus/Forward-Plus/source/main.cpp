#include "main.h"

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
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_PROGRAM_POINT_SIZE); // enable point size to be changed in vertex shader

	glfwSetKeyCallback(gWindow, keyCallback);
	glfwSetCursorPosCallback(gWindow, mouseCallback);
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initShaders() {
	// sets up all the different shaders we need.
	// need at least: blinn phong, ambient, shadow map ones, compute for light culling, and light acumulation

	// then do camera setup
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

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

}

static void mouseCallback(GLFWwindow* window, double x, double y) {

}

int main(int argc, char **argv) {
	initGLFW(argc, argv);

	// Need to do some sort of scene loading

	// need to display the plane. do I load a shader?

	// need to load the lights and all that jazz

	// run while the window is open
	while (!glfwWindowShouldClose(gWindow)) {
		// process pending events
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// TODO: Draw one frame, update simulation

		glfwSwapBuffers(gWindow);
	}

	// clean up and exit
	glfwTerminate();

	return 0;
}
	