#include "Localization.h"

#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>
#include <fstream>
#include <filesystem>

#include "Utils/Log.h"
#include "fmt/base.h"
#include "simdjson.h"
#include "fmt/format.h"

namespace Localization {
	enum PluralCategory : uint8_t {
		One,
		Other,
		Count
	};

	struct Entry {
		uint32_t firstForm = 0;
		uint32_t formCount = 0;
	};

	struct Form {
		uint32_t category = 0;
		uint32_t offset = 0;
		uint32_t length = 0;
	};

	struct ParsedForm {
		std::string text;
		PluralCategory pluralCategory = PluralCategory::Other;
	};

	struct ParsedEntry {
		std::string key;
		std::vector<ParsedForm> forms;
	};

	struct BinHeader {
		char magic[4] {};
		uint32_t version = 0;
		uint32_t entryCount = 0;
		uint32_t formCount = 0;
		uint32_t stringPoolSize = 0;
	};

	static Entry* s_entries = nullptr;
	static Form* s_forms = nullptr;
	static char* s_stringData = nullptr;

	static uint32_t s_entryCount = 0;
	static uint32_t s_formCount = 0;
	static uint32_t s_stringDataSize = 0;

	static bool ParseFile(const char* filePath, std::vector<ParsedEntry>& entries);
	static bool ExportFile(const char* filePath, const std::vector<ParsedEntry>& entries);
	static bool ExportTextIds(const char* sourceDir, const std::vector<ParsedEntry>& entries);

	// TODO: Should handle multiple languages.
	static PluralCategory ConvertToPluralCategory(uint32_t count) {
		if (count == 1)
			return PluralCategory::One;

		return PluralCategory::Other;
	}

	bool CompileStrings(const char* inputFile, const char* outputFile, const char* sourceDir) {
		std::vector<ParsedEntry> parsedEntries;
		if (!ParseFile(inputFile, parsedEntries)) {
			LOG_ERROR("Failed to parse localization file: \"{}\"", inputFile);
			return false;
		}

		if (!ExportFile(outputFile, parsedEntries)) {
			LOG_ERROR("Failed to export localization file \"{}\" to \"{}\"", inputFile, outputFile);
			return false;
		}

		if (!ExportTextIds(sourceDir, parsedEntries)) {
			LOG_ERROR("Failed to generate TextId source files");
			return false;
		}

		return true;
	}

	bool Load(const char* filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file) {
			LOG_ERROR("Failed to open localization file \"{}\"", filePath);
			return false;
		}

		BinHeader header;
		file.read((char*)&header, sizeof(BinHeader));

		if (memcmp(header.magic, "LOCB", 4) != 0) {
			LOG_ERROR("Localization file \"{}\" doesn't have a valid signature", filePath);
			return false;
		}

		Destroy();

		s_entries = new Entry[header.entryCount];
		s_entryCount = header.entryCount;

		file.read((char*)s_entries, header.entryCount * sizeof(Entry));

		s_forms = new Form[header.formCount];
		s_formCount = header.formCount;

		file.read((char*)s_forms, header.formCount * sizeof(Form));

		s_stringData = new char[header.stringPoolSize];
		s_stringDataSize = header.stringPoolSize;

		file.read(s_stringData, header.stringPoolSize);

		return true;
	}

	void Destroy() {
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

	const char* GetString(uint32_t id) {
		assert(s_entries != nullptr && "Localization entries not loaded");
		assert(s_forms != nullptr && "Localization forms not loaded");
		assert(s_stringData != nullptr && "Localization string data not loaded");
		assert(id < s_entryCount && "Invalid localization ID");

		const Entry& entry = s_entries[id];
		const Form& form = s_forms[entry.firstForm];

		return s_stringData + form.offset;
	}

	std::string GetPlural(uint32_t id, uint32_t count) {
		assert(s_entries != nullptr && "Localization entries not loaded");
		assert(s_forms != nullptr && "Localization forms not loaded");
		assert(s_stringData != nullptr && "Localization string data not loaded");
		assert(id < s_entryCount && "Invalid localization ID");

		const PluralCategory category = ConvertToPluralCategory(count);

		const Entry& entry = s_entries[id];

		for (uint32_t i = 0; i < entry.formCount; ++i) {
			const Form& form = s_forms[entry.firstForm + i];

			if (form.category == category) {
				const char* const str = s_stringData + form.offset;
				return fmt::format(fmt::runtime(str), fmt::arg("count", count));
			}
		}

		return {};
	}

	static bool ParseFile(const char* filePath, std::vector<ParsedEntry>& entries) {
		simdjson::ondemand::parser parser;
		auto json = simdjson::padded_string::load(filePath);
		auto doc = parser.iterate(json);

		for (auto field : doc.get_object()) {
			std::string_view key = field.unescaped_key();
			auto value = field.value();

			switch (value.type()) {
			// Singular text entry
			case simdjson::ondemand::json_type::string: {
				ParsedEntry entry;
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
				ParsedEntry entry;
				entry.key = std::string(key);

				for (auto pluralField : value.get_object()) {
					std::string_view pluralKey = pluralField.unescaped_key();
					auto pluralValue = pluralField.value();

					if (pluralValue.type() != simdjson::ondemand::json_type::string) {
						LOG_ERROR("Invalid plural form value for key '{}.{}'", key, pluralKey);
						return false;
					}

					std::string_view text = pluralValue.get_string();

					ParsedForm form;
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

	static bool ExportFile(const char* filePath, const std::vector<ParsedEntry>& entries) {
		std::vector<Entry> locEntries;
		std::vector<Form> locForms;
		std::vector<char> stringPool;

		for (const auto& parsedEntry : entries) {
			Entry entry;
			entry.firstForm = (uint32_t)std::size(locForms);
			entry.formCount = (uint32_t)std::size(parsedEntry.forms);

			for (const auto& parsedForm : parsedEntry.forms) {
				Form form;
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

		BinHeader header;
		memcpy(header.magic, "LOCB", 4);
		header.entryCount = (uint32_t)std::size(locEntries);
		header.formCount = (uint32_t)std::size(locForms);
		header.stringPoolSize = (uint32_t)std::size(stringPool);

		file.write((char*)&header, sizeof(BinHeader));
		file.write((char*)std::data(locEntries), std::size(locEntries) * sizeof(Entry));
		file.write((char*)std::data(locForms), std::size(locForms) * sizeof(Form));
		file.write(std::data(stringPool), std::size(stringPool));

		return true;
	}

	static bool ExportTextIds(const char* sourceDir, const std::vector<ParsedEntry>& entries) {
		std::filesystem::path dir = sourceDir;
		std::filesystem::create_directories(dir);

		std::filesystem::path headerPath = dir / "TextId.h";
		std::ofstream headerFile(headerPath);
		if (!headerFile) {
			LOG_ERROR("Failed to open header file for generated ids: {}", headerPath.c_str());
			return false;
		}

		headerFile << "/*\n";
		headerFile << " * This header is auto generated\n";
		headerFile << " */\n";
		headerFile << "#pragma once\n\n";
		headerFile << "#include <cstdint>\n\n";

		headerFile << "enum TextId : uint32_t {\n";

		for (const auto& entry : entries) {
			headerFile << "\t" << entry.key << ",\n";
		}

		headerFile << "\tCount\n";
		headerFile << "};\n";

		return true;
	}
}
