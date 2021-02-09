/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ObjectManager.h
Purpose: To manage objects easily.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_2
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/11/10
End Header --------------------------------------------------------*/

#include "ObjectManager.h"
#include "OBJLoader.h"
#include "Object.h"

ObjectManager::ObjectManager(void) 
	: m_objects()
{
}

void ObjectManager::AddObject(Object* obj)
{
	m_objects.push_back(obj);
}

void ObjectManager::DeleteObject(Object* /*obj*/)
{
}

void ObjectManager::ClearObjects(void)
{
	for (auto& it : m_objects)
	{
		delete it;
	}
}

const std::vector<Object*>& ObjectManager::GetObjects(void)
{
	return m_objects;
}
