#pragma once

#include <cstdint>

bool Localization_CompileStrings(const char* inputFile, const char* outputFile);

void Localization_Destroy();

// TODO: Create TextId
const char* Localization_GetString(uint32_t id);
