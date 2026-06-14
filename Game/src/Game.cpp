#include "Game.h"

#include "Utils/Log.h"
#include "Localization.h"

Game::~Game() {
	Localization::Destroy();
}

int Game::Run(int argc, char** argv) {
	LOG_INFO("Started Game...");

	constexpr const char* locFile = "res/strings/en.locbin";

	if (!Localization::Load(locFile)) {
		LOG_ERROR("Failed to load locbin file \"{}\"", locFile);
		return -1;
	}

	LOG_INFO("TextId {}: {}", ToString(TextId::MENU_FILE), Localization::GetString(TextId::MENU_FILE));
	LOG_INFO("TextId {}: {}", ToString(TextId::MENU_HELP), Localization::GetString(TextId::MENU_HELP));
	LOG_INFO("TextId {} (count 1): {}", ToString(TextId::ITEM_COUNT), Localization::GetPlural(TextId::ITEM_COUNT, 1));
	LOG_INFO("TextId {} (count 2): {}", ToString(TextId::ITEM_COUNT), Localization::GetPlural(TextId::ITEM_COUNT, 2));

	return 0;
}
