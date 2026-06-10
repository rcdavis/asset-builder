
#include "Utils/Log.h"

#include "CLI/CLI.hpp"

int main(int argc, char** argv) {
	Log::Init();

	CLI::App app("Asset Builder for building and converting game assets");
	app.set_version_flag("-V,--version", "1.0.0");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError& e) {
		// Exit code will be 0 if using --help or --version. This isn't a real error.
		if (e.get_exit_code() != 0) {
			LOG_ERROR("Error parsing command line arguments: {}", e.what());
		}
		return app.exit(e);
	}

	return 0;
}
