#include "Localization.h"

enum PluralCategory : uint8_t {
	One,
	Other,
	Count
};

struct LocalizationEntry {
	uint32_t firstForm = 0;
	uint32_t formMask = 0;
};

struct LocalizationForm {
	uint32_t offset = 0;
	uint32_t length = 0;
};

bool Localization_Load(const char* filePath) {
	return true;
}

void Localization_Destroy() {

}

const char* Localization_GetString(uint32_t id) {
	return nullptr;
}
