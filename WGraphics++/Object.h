/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Object.h
Purpose: Class definition for Object.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/

#ifndef _OBJECT_H
#define _OBJECT_H

class Object
{
public:
	Object(OBJ* obj, 
		const glm::vec3& pos   = glm::vec3(0.0f), 
		const glm::vec3& scl   = glm::vec3(1.0f), 
		const glm::vec3& color = glm::vec3(1.0f));
	~Object(void);

	void Initialize	(void);
	void Update		(void);
	void Render		(void);
	void Destroy	(void);

	glm::vec3& GetPosition(void);
	glm::vec3& GetScale(void);
	glm::vec3& GetColor(void);

	void SetPosition	(const glm::vec3& pos);
	void SetScale		(const glm::vec3& scl);
	void SetColor		(const glm::vec3& color);
	void SetOBJFile		(OBJ& obj);
	void SetRotation	(float degree, const glm::vec3& axis);

	glm::mat4& GetObjectMatrix(void);
	glm::mat4& GetRotationMatrix(void);
	OBJ* GetOBJ(void);

private:
	OBJ* m_obj;				// loaded from .obj file.
	glm::vec3 m_pos;		// position of the object.
	glm::vec3 m_scl;		// scale of the object.
	glm::vec3 m_color;		// color of the object.
	glm::mat4 m_mat;		// tranform matrix
	glm::mat4 m_rot;		// rotation matrix (need for render normal)

	glm::vec3 m_rotAxis;
	float m_rotDegree;
		
};

#endif