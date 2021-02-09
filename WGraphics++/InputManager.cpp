/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: InputManager.cpp
Purpose: Getting simple keyboard inputs from user.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/
#include "InputManager.h"
#include <windows.h>

InputManager::InputManager(void) :
	m_keyWasPressed {false}
{
	for (int i = 0; i < 256; ++i)
	{
		m_keyWasPressed[i] = false;
	}
}

InputManager::~InputManager(void)
{

}

bool InputManager::isKeyDown(int keyID)
{
	if ((GetAsyncKeyState(keyID) & 0x0001) == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool InputManager::isKeyPressed(int keyID)
{
	// Check if key is pressed or not.
	bool isPressed = (GetAsyncKeyState(keyID) & 0x8000) != 0;

	if (!m_keyWasPressed[keyID] && isPressed)
	{
		m_keyWasPressed[keyID] = isPressed;
		return true;
	}

	// 8 or 0 - being pressed.
	// 0 or 1 - have been pressed.
	m_keyWasPressed[keyID] = isPressed;
	return false;
}
