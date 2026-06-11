#include "Application.h"

#include <string>

#include "CLI/CLI.hpp"
#include "TextId.h"
#include "Utils/Log.h"
#include "Localization.h"

Application::~Application() {
	Localization::Destroy();
}

int Application::Run(int argc, char** argv) {
	CLI::App app("Program for building and converting game assets");
	app.set_version_flag("-V,--version", "1.0.0");

	std::string inputLocalizationFile;
	std::string outputBinaryFile;
	std::string sourceDir;

	auto* localization = app.add_subcommand("localization", "Commands for working with localization files");
	localization->add_option("-i,--input", inputLocalizationFile, "Input localization file")->required();
	localization->add_option("-o,--output", outputBinaryFile, "Output binary file")->required();
	localization->add_option("--src-dir", sourceDir, "Output directory for generated source files")->required();

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError& e) {
		// Exception will be thrown with exit code 0 if using --help or --version. This isn't a real error.
		if (e.get_exit_code() != 0) {
			LOG_ERROR("Error parsing command line arguments: {}", e.what());
		}

		return app.exit(e);
	}

	if (localization->parsed()) {
		Localization::CompileStrings(inputLocalizationFile.c_str(), outputBinaryFile.c_str(), sourceDir.c_str());
		Localization::Load(outputBinaryFile.c_str());

		LOG_INFO("TextId {}: {}", ToString(TextId::MENU_FILE), Localization::GetString(TextId::MENU_FILE));
		LOG_INFO("TextId {}: {}", ToString(TextId::MENU_HELP), Localization::GetString(TextId::MENU_HELP));
		LOG_INFO("TextId {} (count 1): {}", ToString(TextId::ITEM_COUNT), Localization::GetPlural(TextId::ITEM_COUNT, 1));
		LOG_INFO("TextId {} (count 2): {}", ToString(TextId::ITEM_COUNT), Localization::GetPlural(TextId::ITEM_COUNT, 2));
	}

	return 0;
}
