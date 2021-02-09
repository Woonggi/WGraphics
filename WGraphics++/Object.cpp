/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Object.cpp
Purpose: To be managed by object manager.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/
#include "OBJLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <algorithm>

#include "Object.h"

Object::Object(OBJ* obj, const glm::vec3& pos, const glm::vec3& scl, const glm::vec3& color)
	: m_obj(obj), m_pos(pos), m_scl(scl), m_color(color), m_mat(1.0f), m_rotAxis(glm::vec3(0.0f, 1.0f, 0.0f))
	, m_rotDegree(0.0f)
{

}

Object::~Object(void)
{

}

void Object::Initialize(void)
{

}

void Object::Update(void)
{

}

void Object::Render(void)
{

}

void Object::Destroy(void)
{
	delete m_obj;
}

void Object::SetPosition(const glm::vec3& pos)
{
	m_pos = pos;
}
void Object::SetScale(const glm::vec3& scl)
{
	m_scl = scl;
}
void Object::SetColor(const glm::vec3& color)
{
	m_color = color;
}

void Object::SetRotation(float degree, const glm::vec3& axis)
{
	m_rotDegree = degree;
	m_rotAxis = axis;
}

glm::vec3& Object::GetPosition(void)
{
	return m_pos;
}

glm::vec3& Object::GetScale(void)
{
	return m_scl;
}

glm::vec3& Object::GetColor(void)
{
	return m_color;
}

glm::mat4& Object::GetObjectMatrix(void)
{
	glm::mat4 translate = glm::mat4(1.0f);
	glm::mat4 rotation  = glm::mat4(1.0f);
	glm::mat4 scaler    = glm::mat4(1.0f);

	scaler    = glm::scale(m_scl);
	rotation  = glm::rotate(glm::radians(m_rotDegree), m_rotAxis);
	translate = glm::translate(translate, m_pos);

	m_rot = rotation;
	m_mat = translate * rotation * scaler;

	return m_mat;
}

glm::mat4& Object::GetRotationMatrix(void)
{
	return m_rot;
}

OBJ* Object::GetOBJ(void)
{
	return m_obj;
}

void Object::SetOBJFile(OBJ& obj)
{
	m_obj = &obj;
}
