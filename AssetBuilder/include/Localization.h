#pragma once

#include <filesystem>

namespace Localization {
	bool CompileStrings(const char* inputFile, const char* outputFile);

	bool BuildResources(const std::filesystem::path& resDir, const std::filesystem::path& outputDir);

	void Destroy();
}
