/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Game.cpp
Purpose: Managing main loop for the application for the future scalability
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#include "Game.h"
#include "Graphics.h"
#include "InputManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// temporary
#include <iostream>
#include "OBJLoader.h"
#include "Object.h"
#include "ObjectManager.h"

Game::Game(void)
	: m_graphics(new Graphics(1280, 720))
	, m_isShutdown(false)
{

}

Game::~Game(void)
{
	
}

bool Game::Initialize(void)
{	
	OBJ* bunny = LoadOBJFromFile("obj/bunny.obj");
	Object* obj1  = new Object(bunny);
	obj1->SetScale(glm::vec3(1.0f));

	ObjectManager::getInstance().AddObject(obj1);

	return m_graphics->Initialize();
}

void Game::Update(void)
{
	while (m_isShutdown == false)
	{
		double currTime;
		static double lastTime = glfwGetTime();
		currTime = glfwGetTime();
		float dt = (float)(currTime - lastTime);
		lastTime = currTime;

		m_graphics->Update(dt);
		if (InputManager::getInstance().isKeyPressed(KEY_ESCAPE))
		{
			m_isShutdown = true;
		}
	}
}

void Game::Shutdown(void)
{
	m_graphics->Shutdown();
	ObjectManager::getInstance().ClearObjects();
	delete m_graphics;
}

