#pragma once

#include <GL\glew.h>
#include <GLFW\glfw3.h>

enum Format {
	GLFMT_UNKNOWN = 0,
	GLFMT_R8G8B8,
	GLFMT_A8R8G8B8,
	GLFMT_sA8R8G8B8,
	GLFMT_D24S8,
	GLFMT_D32F,
	GLFMT_DXT1,
	GLFMT_DXT5,
	GLFMT_R16F,
	GLFMT_G16R16F,
	GLFMT_A16B16G16R16F,
	GLFMT_R32F,
	GLFMT_G32R32F,
	GLFMT_A32B32G32R32F
};

class FrameBuffer {
	struct Attachment {
		GLuint id;
		int type;

		Attachment() : id(0), type(0) {}
	};

private:
	GLuint frameBufferId, width, height;
	Attachment renderTargets[8];
	Attachment depthStencil;

public:
	FrameBuffer(GLuint width, GLuint height);
	~FrameBuffer();

	bool AttachRenderBuffer(GLenum target, Format format, GLsizei samples);
	bool AttachTexture(GLenum target, Format format, GLenum filter = GL_NEAREST);

	void Set();
	void Unset();
	bool Validate();

	GLuint GetColorAttachment(int index);
	GLuint GetDepthAttachment();
};