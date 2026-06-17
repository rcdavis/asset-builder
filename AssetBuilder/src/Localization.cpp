#include "Localization.h"

#include <cassert>
#include <cstdint>
#include <string_view>
#include <vector>
#include <fstream>
#include <filesystem>

#include "Utils/Log.h"
#include "simdjson.h"

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

	static bool ParseStringsFile(const char* filePath, std::vector<ParsedEntry>& entries);
	static bool ExportLocbinFile(const char* filePath, const std::vector<ParsedEntry>& entries);
	static bool ExportTextIdsHeader(const std::filesystem::path& headerPath, const std::vector<ParsedEntry>& entries);
	static bool ExportTextIdsSource(const std::filesystem::path& sourcePath, const std::vector<ParsedEntry>& entries);

	// TODO: Should handle multiple languages.
	static PluralCategory ConvertToPluralCategory(uint32_t count) {
		if (count == 1)
			return PluralCategory::One;

		return PluralCategory::Other;
	}

	bool BuildResources(const std::filesystem::path& resDir, const std::filesystem::path& outputDir) {
		if (!std::filesystem::is_directory(resDir)) {
			LOG_ERROR("Passed in path isn't a directory: {}", resDir.c_str());
			return false;
		}

		// TODO: Handle multiple languages
		const std::filesystem::path stringsFile = resDir / "strings/en.json";

		if (!std::filesystem::exists(stringsFile)) {
			LOG_ERROR("Strings file doesn't exist: {}", stringsFile.c_str());
			return false;
		}

		std::vector<ParsedEntry> parsedEntries;
		if (!ParseStringsFile(stringsFile.c_str(), parsedEntries)) {
			LOG_ERROR("Failed to parse localization file: \"{}\"", stringsFile.c_str());
			return false;
		}

		const std::filesystem::path locbinFile = outputDir / "Assets/strings/en.locbin";
		std::filesystem::create_directories(locbinFile.parent_path());

		if (!ExportLocbinFile(locbinFile.c_str(), parsedEntries)) {
			LOG_ERROR("Failed to export localization file \"{}\" to \"{}\"", stringsFile.c_str(), locbinFile.c_str());
			return false;
		}

		std::filesystem::create_directories(outputDir / "Generated");

		const std::filesystem::path headerPath = outputDir / "Generated/TextId.h";
		if (!ExportTextIdsHeader(headerPath, parsedEntries)) {
			LOG_ERROR("Failed to generate TextId header file: {}", headerPath.c_str());
			return false;
		}

		const std::filesystem::path sourcePath = outputDir / "Generated/TextId.cpp";
		if (!ExportTextIdsSource(sourcePath, parsedEntries)) {
			LOG_ERROR("Failed to generate TextId source file: {}", sourcePath.c_str());
			return false;
		}

		return true;
	}

	static bool ParseStringsFile(const char* filePath, std::vector<ParsedEntry>& entries) {
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

	static bool ExportLocbinFile(const char* filePath, const std::vector<ParsedEntry>& entries) {
		std::vector<Entry> locEntries;
		locEntries.reserve(std::size(entries));

		std::vector<Form> locForms;
		locForms.reserve(std::size(entries));

		std::vector<char> stringPool;
		stringPool.reserve(std::size(entries));

		for (const auto& parsedEntry : entries) {
			Entry entry;
			entry.firstForm = (uint32_t)std::size(locForms);
			entry.formCount = (uint32_t)std::size(parsedEntry.forms);

			for (const auto& parsedForm : parsedEntry.forms) {
				Form form;
				form.category = (uint32_t)parsedForm.pluralCategory;
				form.offset = (uint32_t)std::size(stringPool);

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

	static bool ExportTextIdsHeader(const std::filesystem::path& headerPath, const std::vector<ParsedEntry>& entries) {
		std::ofstream file(headerPath);
		if (!file) {
			LOG_ERROR("Failed to open header file for generated ids: {}", headerPath.c_str());
			return false;
		}

		file << "/**\n";
		file << " * This file is auto generated. Any manual changes will be overridden.\n";
		file << " */\n";
		file << "#pragma once\n\n";
		file << "#include <cstdint>\n\n";

		file << "enum TextId : uint32_t {\n";

		for (const auto& entry : entries) {
			file << "\t" << entry.key << ",\n";
		}

		file << "\tCount\n";
		file << "};\n\n";

		file << "const char* TextIdToString(TextId id);\n";

		return true;
	}

	static bool ExportTextIdsSource(const std::filesystem::path& sourcePath, const std::vector<ParsedEntry>& entries) {
		std::ofstream file(sourcePath);
		if (!file) {
			LOG_ERROR("Failed to open source file for generated ids: {}", sourcePath.c_str());
			return false;
		}

		file << "/**\n";
		file << " * This file is auto generated. Any manual changes will be overridden.\n";
		file << " */\n";
		file << "#include \"TextId.h\"\n\n";

		file << "const char* TextIdToString(TextId id) {\n";
		file << "\tswitch(id) {\n";

		for (const auto& entry : entries) {
			file << "\tcase " << entry.key << ":\n";
			file << "\t\treturn \"" << entry.key << "\";\n\n";
		}

		file << "\tdefault:\n";
		file << "\t\treturn nullptr;\n";
		file << "\t}\n\n";
		file << "\treturn nullptr;\n";
		file << "}\n";

		return true;
	}
}
