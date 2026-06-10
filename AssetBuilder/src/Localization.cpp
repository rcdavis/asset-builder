#include "Localization.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <fstream>

#include "Utils/Log.h"
#include "simdjson.h"

enum PluralCategory : uint8_t {
	One,
	Other,
	Count
};

struct LocalizationEntry {
	uint32_t firstForm = 0;
	uint32_t formCount = 0;
};

struct LocalizationForm {
	uint32_t category = 0;
	uint32_t offset = 0;
	uint32_t length = 0;
};

struct ParsedLocalizationForm {
	std::string text;
	PluralCategory pluralCategory = PluralCategory::Other;
};

struct ParsedLocalizationEntry {
	std::string key;
	std::vector<ParsedLocalizationForm> forms;
};

struct LocalizationBinHeader {
	char magic[4] = "LOC";
	uint32_t version = 0;
	uint32_t entryCount = 0;
	uint32_t formCount = 0;
	uint32_t stringPoolSize = 0;
};

static LocalizationEntry* s_entries = nullptr;
static LocalizationForm* s_forms = nullptr;
static char* s_stringData = nullptr;

static uint32_t s_entryCount = 0;
static uint32_t s_formCount = 0;
static uint32_t s_stringDataSize = 0;

static bool Localization_ParseFile(const char* filePath, std::vector<ParsedLocalizationEntry>& entries);
static bool Localization_ExportFile(const char* filePath, const std::vector<ParsedLocalizationEntry>& entries);

bool Localization_CompileStrings(const char* inputFile, const char* outputFile) {
	std::vector<ParsedLocalizationEntry> parsedEntries;
	if (!Localization_ParseFile(inputFile, parsedEntries)) {
		LOG_ERROR("Failed to parse localization file: {}", inputFile);
		return false;
	}

	if (!Localization_ExportFile(outputFile, parsedEntries)) {
		LOG_ERROR("Failed to export localization file \"{}\" to \"{}\"", inputFile, outputFile);
		return false;
	}

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

static bool Localization_ParseFile(const char* filePath, std::vector<ParsedLocalizationEntry>& entries) {
	simdjson::ondemand::parser parser;
	auto json = simdjson::padded_string::load(filePath);
	auto doc = parser.iterate(json);

	for (auto field : doc.get_object()) {
		std::string_view key = field.unescaped_key();
		auto value = field.value();

		switch (value.type()) {
		// Singular text entry
		case simdjson::ondemand::json_type::string: {
			ParsedLocalizationEntry entry;
			entry.key = std::string(key);
			const std::string_view text = value.get_string();

			entry.forms.push_back({
				.text = std::string(text),
				.pluralCategory = PluralCategory::Other,
			});

			entries.push_back(std::move(entry));
		}
		break;

		// Plural text entry
		case simdjson::ondemand::json_type::object: {
			ParsedLocalizationEntry entry;
			entry.key = std::string(key);

			for (auto pluralField : value.get_object()) {
				std::string_view pluralKey = pluralField.unescaped_key();
				auto pluralValue = pluralField.value();

				if (pluralValue.type() != simdjson::ondemand::json_type::string) {
					LOG_ERROR("Invalid plural form value for key '{}.{}'", key, pluralKey);
					return false;
				}

				std::string_view text = pluralValue.get_string();

				ParsedLocalizationForm form;
				form.text = std::string(text);

				if (pluralKey == "one") {
					form.pluralCategory = PluralCategory::One;
				} else if (pluralKey == "other") {
					form.pluralCategory = PluralCategory::Other;
				} else {
					LOG_ERROR("Unsupported plural category '{}' for key '{}'", pluralKey, key);
					return false;
				}

				entry.forms.push_back(std::move(form));
			}

			entries.push_back(std::move(entry));
		}
		break;

		default:
			LOG_ERROR("Unsupported JSON value type for key '{}'", key);
			return false;
		}
	}

	return true;
}

static bool Localization_ExportFile(const char* filePath, const std::vector<ParsedLocalizationEntry>& entries) {
	std::vector<LocalizationEntry> locEntries;
	std::vector<LocalizationForm> locForms;
	std::vector<char> stringPool;

	for (const auto& parsedEntry : entries) {
		LocalizationEntry entry;
		entry.firstForm = (uint32_t)std::size(locForms);
		entry.formCount = (uint32_t)std::size(parsedEntry.forms);

		for (const auto& parsedForm : parsedEntry.forms) {
			LocalizationForm form;
			form.category = (uint32_t)parsedForm.pluralCategory;
			form.offset = (uint32_t)std::size(stringPool);
			form.length = (uint32_t)std::size(parsedForm.text);

			stringPool.insert(std::end(stringPool), std::cbegin(parsedForm.text), std::cend(parsedForm.text));
			stringPool.push_back('\0');

			locForms.push_back(std::move(form));
		}

		locEntries.push_back(std::move(entry));
	}

	std::ofstream file(filePath, std::ios::binary);
	if (!file) {
		LOG_ERROR("Failed to create file \"{}\"", filePath);
		return false;
	}

	LocalizationBinHeader header;
	header.entryCount = (uint32_t)std::size(locEntries);
	header.formCount = (uint32_t)std::size(locForms);
	header.stringPoolSize = (uint32_t)std::size(stringPool);

	file.write((char*)&header, sizeof(LocalizationBinHeader));
	file.write((char*)std::data(locEntries), std::size(locEntries) * sizeof(LocalizationEntry));
	file.write((char*)std::data(locForms), std::size(locForms) * sizeof(LocalizationForm));
	file.write(std::data(stringPool), std::size(stringPool));

	return true;
}
