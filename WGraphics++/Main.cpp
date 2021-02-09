/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Main.cpp
Purpose: Main function that runs application.
Language: C++, Visual studio 2019
Platform: Visual Studio 2019, C++ Standard to Default, Windows.
Project: woonggi.eun_CS300_1
Author: Richard Eun, woonggi.eun, 180003119
Creation date: 2019/09/18
End Header --------------------------------------------------------*/

#include "Game.h"

int main(void)
{
	Game* game = new Game;
	// Game initialize, update, shutdown should be here.
	if (game->Initialize() == true)
	{
		game->Update();
	}
	
	game->Shutdown();

	return 0;
}