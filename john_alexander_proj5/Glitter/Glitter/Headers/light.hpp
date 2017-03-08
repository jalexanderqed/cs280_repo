#ifndef LIGHT_H
#define LIGHT_H

#include <glad\glad.h>

#include <string>
#include <sstream>
#include <iostream>

class Light {
private:
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	int index;
	std::string myPre;

public:
	Light() :
		position(glm::vec3()),
		ambient(glm::vec3()),
		diffuse(glm::vec3()),
		specular(glm::vec3()),
		index(0)
	{}

	Light(glm::vec3 pos, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, int ind) :
		position(pos),
		ambient(amb),
		diffuse(diff),
		specular(spec),
		index(ind)
	{
		std::ostringstream pred;
		pred << "pointLights[" << index << "]";
		myPre = pred.str();
	}

	void update(const Shader& shader) {
		glUniform3fv(glGetUniformLocation(shader.Program, (myPre + ".position").c_str()), 1, glm::value_ptr(position));
		glUniform3fv(glGetUniformLocation(shader.Program, (myPre + ".ambient").c_str()), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(shader.Program, (myPre + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(shader.Program, (myPre + ".specular").c_str()), 1, glm::value_ptr(specular));
	}

	glm::vec3 getPos() {
		return position;
	}

	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime, glm::vec3 front, glm::vec3 right)
	{
		GLfloat velocity = 3 * deltaTime;
		if (direction == FORWARD)
			this->position += front * velocity;
		if (direction == BACKWARD)
			this->position -= front * velocity;
		if (direction == LEFT)
			this->position -= right * velocity;
		if (direction == RIGHT)
			this->position += right * velocity;
	}
};

#endif
