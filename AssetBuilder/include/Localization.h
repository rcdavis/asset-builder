#pragma once

#include <cstdint>

bool Localization_Load(const char* filePath);

void Localization_Destroy();

// TODO: Create TextId
const char* Localization_GetString(uint32_t id);
