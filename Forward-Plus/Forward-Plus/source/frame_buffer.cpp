#include "frame_buffer.h"

GLint internalFormat[14] = {
	0,
	GL_RGB8,
	GL_RGBA8,
	GL_SRGB8_ALPHA8,
	GL_DEPTH_STENCIL,
	GL_DEPTH_COMPONENT,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	GL_R16F,
	GL_RG16F,
	GL_RGBA16F_ARB,
	GL_R32F,
	GL_RG32F,
	GL_RGBA32F_ARB
};

GLenum formatFormat[14] = {
	0,
	GL_RGB,
	GL_RGBA,
	GL_RGBA,
	GL_DEPTH_STENCIL,
	GL_DEPTH_COMPONENT,
	GL_RGBA,
	GL_RGBA,
	GL_RED,
	GL_RG,
	GL_RGBA,
	GL_RED,
	GL_RG,
	GL_RGBA
};

GLenum formatType[14] = {
	0,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT_8_8_8_8_REV,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT_24_8,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_BYTE,
	GL_FLOAT,
	GL_HALF_FLOAT,
	GL_HALF_FLOAT,
	GL_HALF_FLOAT,
	GL_FLOAT,
	GL_FLOAT,
	GL_FLOAT
};

FrameBuffer::FrameBuffer(GLuint width, GLuint height) {
	glGenFramebuffers(1, &frameBufferId);
	this->width = width;
	this->height = height;
}

FrameBuffer::~FrameBuffer() {
	for (int i = 0; i < 8; i++) {
		if (renderTargets[i].id != 0) {
			if (renderTargets[i].type == 0) {
				glDeleteRenderbuffers(1, &renderTargets[i].id);
			}
			else {
				glDeleteTextures(1, &renderTargets[i].id);
			}
		}
	}

	if (depthStencil.id != 0) {
		if (depthStencil.type == 0) {
			glDeleteRenderbuffers(1, &depthStencil.id);
		}
		else {
			glDeleteTextures(1, &depthStencil.id);
		}
	}

	glDeleteFramebuffers(1, &frameBufferId);
}

bool FrameBuffer::AttachRenderBuffer(GLenum target, Format format, GLsizei samples) {
	Attachment *attachment = 0;

	if (target == GL_DEPTH_ATTACHMENT || target == GL_DEPTH_STENCIL_ATTACHMENT) {
		attachment = &depthStencil;
	}
	else if (target < GL_COLOR_ATTACHMENT8 && target >= GL_COLOR_ATTACHMENT0) {
		attachment = &renderTargets[target - GL_COLOR_ATTACHMENT0];
	}
	else {
		// Invalid target
		return false;
	}

	if (attachment->id != 0) {
		// There's already something attached to this target
		return false;
	}

	glGenRenderbuffers(1, &attachment->id);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, attachment->id);

	if (samples > 1) {
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalFormat[format], width, height);
	}
	else {
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat[format], width, height);
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, target, GL_RENDERBUFFER, attachment->id);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	attachment->type = 0;

	return true;
}

bool FrameBuffer::AttachTexture(GLenum target, Format format, GLenum filter) {
	Attachment *attachment = 0;

	if (target == GL_DEPTH_ATTACHMENT || target == GL_DEPTH_STENCIL_ATTACHMENT) {
		attachment = &depthStencil;
	}
	else if (target < GL_COLOR_ATTACHMENT8 && target >= GL_COLOR_ATTACHMENT0) {
		attachment = &renderTargets[target - GL_COLOR_ATTACHMENT0];
	}
	else {
		// Invalid target
		return false;
	}

	if (attachment->id != 0) {
		// There's already something attached to this target
		return false;
	}

	glGenTextures(1, &attachment->id);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
	glBindTexture(GL_TEXTURE_2D, attachment->id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat[format], width, height, 0, formatFormat[format], formatType[format], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, target, GL_TEXTURE_2D, attachment->id, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	attachment->type = 1;

	return true;
}

void FrameBuffer::Set() {
	GLenum buffers[8];
	GLsizei count = 0;

	for (int i = 0; i < 8; i++) {
		if (renderTargets[i].id != 0) {
			buffers[i] = GL_COLOR_ATTACHMENT0 + i;
			count = i;
		}
		else {
			buffers[i] = GL_NONE;
		}
	}

	if (count > 0) {
		glDrawBuffers(count + 1, buffers);
	}

	glViewport(0, 0, width, height);
}

void FrameBuffer::Unset() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_BACK);
}

bool FrameBuffer::Validate() {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	// TODO: Check status?

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return status == GL_FRAMEBUFFER_COMPLETE;
}

GLuint FrameBuffer::GetColorAttachment(int index) {
	return renderTargets[index].id;
}

GLuint FrameBuffer::GetDepthAttachment() {
	return depthStencil.id;
}