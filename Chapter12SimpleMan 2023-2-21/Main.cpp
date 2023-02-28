// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <iostream>
#include "Game.h"

int main(int argc, char** argv)
{
	//running things from AssimpPlusDemoApp.cc in here

	HINSTANCE appInstance = GetModuleHandle(nullptr);
	
	AssimpPlus::AssimpPlusDemoApp app(appInstance); 
	Game game(app, appInstance);
	
	bool successGame   = game.Initialize();

	if (!successGame) std::cout << "Can't start the Madhav demo!\n";

	if (successGame)
	{
		std::cout << "running the game.";
		game.RunLoop();
	}
	game.Shutdown();

	return 0;
}
