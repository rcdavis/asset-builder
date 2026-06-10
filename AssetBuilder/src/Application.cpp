#include "Application.h"

#include <string>

#include "CLI/CLI.hpp"
#include "Utils/Log.h"
#include "Localization.h"

Application::~Application() {
	Localization_Destroy();
}

int Application::Run(int argc, char** argv) {
	CLI::App app("Program for building and converting game assets");
	app.set_version_flag("-V,--version", "1.0.0");

	std::string inputLocalizationFile;
	std::string outputBinaryFile;

	auto* localization = app.add_subcommand("localization", "Commands for working with localization files");
	localization->add_option("-i,--input", inputLocalizationFile, "Input localization file")->required();
	localization->add_option("-o,--output", outputBinaryFile, "Output binary file")->required();

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
		Localization_CompileStrings(inputLocalizationFile.c_str(), outputBinaryFile.c_str());
		Localization_Load(outputBinaryFile.c_str());

		LOG_INFO("TextId 0: {}", Localization_GetString(0));
		LOG_INFO("TextId 1: {}", Localization_GetString(1));
	}

	return 0;
}
