#include "camera.h"

glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(this->position, this->position + this->front, this->up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime) {
	GLfloat velocity = this->movementSpeed * deltaTime;

	if (direction == FORWARD) {
		this->position += this->front * velocity;
	}
	if (direction == BACKWARD) {
		this->position -= this->front * velocity;
	}
	if (direction == LEFT) {
		this->position -= this->right * velocity;
	}
	if (direction == RIGHT) {
		this->position += this->right * velocity;
	}
}

void Camera::ProcessMouseMovement(GLfloat xOffset, GLfloat yOffset, GLboolean constrainPitch) {
	xOffset *= this->mouseSensitivity;
	yOffset *= this->mouseSensitivity;

	this->yaw += xOffset;
	this->pitch += yOffset;

	// Make sure when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch) {
		if (this->pitch > 89.0f) {
			this->pitch = 89.0f;
		}
		if (this->pitch < -89.0f) {
			this->pitch = -89.0f;
		}
	}

	// Update the camera
	this->Update();
}

// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(GLfloat yOffset) {
	if (this->zoom >= 1.0f && this->zoom <= 45.0f) {
		this->zoom -= yOffset;
	}
	if (this->zoom <= 1.0f) {
		this->zoom = 1.0f;
	}
	if (this->zoom >= 45.0f) {
		this->zoom = 45.0f;
	}
}

void Camera::Update() {
	glm::vec3 front;
	front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	front.y = sin(glm::radians(this->pitch));
	front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
	this->front = glm::normalize(front);
	this->right = glm::normalize(glm::cross(this->front, this->worldUp));
	this->up = glm::normalize(glm::cross(this->right, this->front));
}