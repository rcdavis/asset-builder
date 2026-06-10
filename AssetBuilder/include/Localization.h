#pragma once

#include <cstdint>
#include <string>

bool Localization_CompileStrings(const char* inputFile, const char* outputFile);

bool Localization_Load(const char* filePath);

void Localization_Destroy();

// TODO: Create TextId
const char* Localization_GetString(uint32_t id);

std::string Localization_GetPlural(uint32_t id, uint32_t count);
