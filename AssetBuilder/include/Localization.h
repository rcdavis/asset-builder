#pragma once

#include <cstdint>
#include <string>

#include "TextId.h"

namespace Localization {
	bool CompileStrings(const char* inputFile, const char* outputFile);

	bool Load(const char* filePath);

	void Destroy();

	// TODO: Create TextId
	const char* GetString(TextId id);

	std::string GetPlural(TextId id, uint32_t count);
}
