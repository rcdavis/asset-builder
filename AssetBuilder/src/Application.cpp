#include "Application.h"

#include "CLI/CLI.hpp"
#include "Utils/Log.h"
#include "Localization.h"

Application::~Application() {
	Localization_Destroy();
}

int Application::Run(int argc, char** argv) {
	CLI::App app("Program for building and converting game assets");
	app.set_version_flag("-V,--version", "1.0.0");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError& e) {
		// Exception will be thrown with exit code 0 if using --help or --version. This isn't a real error.
		if (e.get_exit_code() != 0) {
			LOG_ERROR("Error parsing command line arguments: {}", e.what());
		}

		return app.exit(e);
	}

	Localization_Load("res/strings/en.json");

	return 0;
}
