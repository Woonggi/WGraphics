/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Game.h
Purpose: Header for Game.cpp
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#ifndef _GAME_H
#define _GAME_H

class Graphics;

class Game
{
public:
	Game(void);
	~Game(void);
	
	bool Initialize		(void);
	void Update			(void);
	void Shutdown		(void);

	void ShutdownGame(void) { m_isShutdown = true; }

private:
	Graphics* m_graphics;
	bool m_isShutdown;
	
};

#endif