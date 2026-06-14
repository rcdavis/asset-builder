#pragma once

#include <string>

#include "TextId.h"

namespace Localization {
	bool Load(const char* filePath);

	void Destroy();

	const char* GetString(TextId id);

	std::string GetPlural(TextId id, uint32_t count);
}
