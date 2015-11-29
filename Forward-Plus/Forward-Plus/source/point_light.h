#pragma once

#include <glm/glm.hpp>

class PointLight {
public:
	glm::vec3 color;
	glm::vec4 previous;
	glm::vec4 current;
	glm::vec3 velocity;
	float radius;
};