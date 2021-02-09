/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ObjectManager.h
Purpose: Class declaration for ObjectManager.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/

#ifndef _OBJECT_MANAGER_H
#define _OBJECT_MANAGER_H

#include <vector>

class Object;

class ObjectManager
{
public:
	static ObjectManager& getInstance(void)
	{
		static ObjectManager instance;
		return instance;
	}

	void AddObject(Object* obj);
	void DeleteObject(Object* obj);
	void ClearObjects(void);

	const std::vector<Object*>& GetObjects(void);

private:
	ObjectManager(void);
	std::vector<Object*> m_objects;
};

#endif