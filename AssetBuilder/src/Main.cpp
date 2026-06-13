
#include "Utils/Log.h"

#include "Application.h"

int main(int argc, char** argv) {
	Log::Init("AssetBuilder");

	Application app;
	return app.Run(argc, argv);
}
