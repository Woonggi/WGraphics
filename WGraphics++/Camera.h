/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ObjectManager.h
Purpose: Class definition for camera.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

struct GLFWwindow;
class Camera
{
public:
	Camera(GLFWwindow* window, int width, int height);
	Camera(GLFWwindow* window,
		const glm::vec3& direction,  const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));
	~Camera(void);

	void Initialize(GLFWwindow* window, int width, int height);

	const glm::mat4& GetViewMatrix(void);

	glm::vec3& GetPosition(void);

	void Move(const glm::vec3& dir, float speed);
	void SetPosition(const glm::vec3& pos);
	void Control(float mouseSpeed, float dt);

private:
	GLFWwindow* m_window;
	glm::vec3 m_position;
	glm::vec3 m_direction;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::mat4 m_view;

	float m_hAngle;
	float m_vAngle;
	int	  m_width, m_height;
};

#endif