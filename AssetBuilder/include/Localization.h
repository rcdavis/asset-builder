#pragma once

#include <cstdint>
#include <string>

namespace Localization {
	bool CompileStrings(const char* inputFile, const char* outputFile);

	bool Load(const char* filePath);

	void Destroy();

	// TODO: Create TextId
	const char* GetString(uint32_t id);

	std::string GetPlural(uint32_t id, uint32_t count);
}
