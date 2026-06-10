#pragma once

class Application {
public:
	Application() = default;
	~Application();

	int Run(int argc, char** argv);
};
