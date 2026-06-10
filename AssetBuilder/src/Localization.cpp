#include "Localization.h"

#include <cassert>

#include "simdjson.h"

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

static LocalizationEntry* s_entries = nullptr;
static LocalizationForm* s_forms = nullptr;
static char* s_stringData = nullptr;

static uint32_t s_entryCount = 0;
static uint32_t s_formCount = 0;
static uint32_t s_stringDataSize = 0;

bool Localization_Load(const char* filePath) {
	return true;
}

void Localization_Destroy() {
	delete[] s_entries;
	s_entries = nullptr;

	delete[] s_forms;
	s_forms = nullptr;

	delete[] s_stringData;
	s_stringData = nullptr;

	s_entryCount = 0;
	s_formCount = 0;
	s_stringDataSize = 0;
}

const char* Localization_GetString(uint32_t id) {
	assert(s_entries != nullptr && "Localization entries not loaded");
	assert(s_forms != nullptr && "Localization forms not loaded");
	assert(s_stringData != nullptr && "Localization string data not loaded");
	assert(id < s_entryCount && "Invalid localization ID");

	const LocalizationEntry& entry = s_entries[id];
	const LocalizationForm& form = s_forms[entry.firstForm];

	return s_stringData + form.offset;
}
