#include "Application.h"

#include <string>

#include "CLI/CLI.hpp"
#include "Utils/Log.h"
#include "Localization.h"

Application::~Application() {
	Localization::Destroy();
}

int Application::Run(int argc, char** argv) {
	CLI::App app("Program for building and converting game assets");
	app.set_version_flag("-V,--version", "1.0.0");

	std::string resDir;
	std::string outputDir;

	auto* localization = app.add_subcommand("localization", "Commands for working with localization files");
	localization->add_option("-i,--input", resDir, "Input resource directory")->required();
	localization->add_option("-o,--output", outputDir, "Output resource directory")->required();

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
		Localization::BuildResources(resDir, outputDir);
		//Localization::CompileStrings(inputLocalizationFile.c_str(), outputBinaryFile.c_str());
	}

	return 0;
}
