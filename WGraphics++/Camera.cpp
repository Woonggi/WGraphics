/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.cpp
Purpose: Implementation of camera in the scene.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glfw3.h>

namespace 
{
	const float PI = 3.141592f;
}

Camera::Camera(GLFWwindow* window, int width, int height) : 
	m_window(window),
	m_position(0.0f),
	m_direction(glm::vec3(0.0f, 0.0f, -1.0f)),
	m_up(0.0f),
	m_view(0.0f),
	m_hAngle(PI),
	m_vAngle(0.0f),
	m_width(width),
	m_height(height)
{
}

Camera::Camera(GLFWwindow* window, const glm::vec3& direction, const glm::vec3& up) :
	m_window(window),
	m_position(0.0f),
	m_direction(direction),
	m_up(up),
	m_view(0.0f),
	m_hAngle(glm::radians(90.0f)),
	m_vAngle(0.0f),
	m_width(1.0f),
	m_height(1.0f)
{
}

Camera::~Camera(void)
{

}

void Camera::Initialize(GLFWwindow* window, int width, int height)
{
	m_window = window;
	m_width = width;
	m_height = height;
}

const glm::mat4& Camera::GetViewMatrix(void) 
{	
	m_view = glm::lookAt(
			m_position,
			m_position + m_direction,
			m_up
		);

	return m_view;
}

glm::vec3& Camera::GetPosition(void)
{
	return m_position;
}

void Camera::SetPosition(const glm::vec3& pos)
{
	m_position = pos;
}

void Camera::Move(const glm::vec3& dir, float speed)
{
	m_position += (dir * speed);
}

void Camera::Control(float mouseSpeed, float dt)
{
	double xPos, yPos;

	glfwGetCursorPos(m_window, &xPos, &yPos);
	//glfwSetCursorPos(m_window, m_width / 2, m_height / 2);

	m_hAngle += mouseSpeed * dt * (float)(m_width / 2 - xPos);
	m_vAngle += mouseSpeed * dt * (float)(m_height / 2 - yPos);

	m_direction = glm::vec3(
		cos(m_vAngle) * sin(m_hAngle),
		sin(m_vAngle),
		cos(m_vAngle) * cos(m_hAngle)
	);

	m_right = glm::vec3(
		sin(m_hAngle - (PI / 2.f)),
		0,
		cos(m_hAngle - (PI / 2.f))
	);

	m_up = glm::cross(m_right, m_direction);
}