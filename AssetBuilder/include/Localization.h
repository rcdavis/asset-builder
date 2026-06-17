#pragma once

#include <filesystem>

namespace Localization {
	bool BuildResources(const std::filesystem::path& resDir, const std::filesystem::path& outputDir, const std::filesystem::path& genDir);
}
