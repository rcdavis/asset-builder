#pragma once

namespace Localization {
	bool CompileStrings(const char* inputFile, const char* outputFile);

	bool Load(const char* filePath);

	void Destroy();
}
