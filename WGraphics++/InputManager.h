/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: InputManager.h
Purpose: Header for InputManager.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#ifndef _INPUTMANAGER_H
#define _INPUTMANAGER_H

#include "KeyCodes.h"

class InputManager
{
public:
	static InputManager& getInstance(void)
	{
		static InputManager instance;
		return instance;
	}

	bool isKeyDown		(int keyID);
	bool isKeyPressed	(int keyID);

private:
	InputManager		(void);
	~InputManager		(void);

private:
	bool m_keyWasPressed[256];
};

#endif