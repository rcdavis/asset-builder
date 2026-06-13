
#include "Utils/Log.h"
#include "Game.h"

int main(int argc, char** argv) {
	Log::Init("Game");

	Game game;
	return game.Run(argc, argv);
}
