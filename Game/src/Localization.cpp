#include "Localization.h"

#include <fstream>
#include <cassert>

#include "Utils/Log.h"
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

	// TODO: Should handle multiple languages. As of now, only English.
	static PluralCategory ConvertToPluralCategory(uint32_t count) {
		if (count == 1)
			return PluralCategory::One;

		return PluralCategory::Other;
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

	const char* GetString(TextId id) {
		assert(s_entries != nullptr && "Localization entries not loaded");
		assert(s_forms != nullptr && "Localization forms not loaded");
		assert(s_stringData != nullptr && "Localization string data not loaded");
		assert(id < s_entryCount && "Invalid localization ID");

		const Entry& entry = s_entries[id];
		const Form& form = s_forms[entry.firstForm];

		return s_stringData + form.offset;
	}

	std::string GetPlural(TextId id, uint32_t count) {
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
}
