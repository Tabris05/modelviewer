#ifdef NDEBUG
#define main WinMain
#endif

#include <filesystem>
#include "renderer.h"

// force discrete gpu if applicable
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main() {
#ifdef NDEBUG
	if (__argc != 2) return 0;
	const char* modelPath = __argv[1];
	std::string executablePath(__argv[0]);
	std::filesystem::current_path(executablePath.substr(0, executablePath.find_last_of('\\')));
	constexpr int windowW = 1280, windowH = 720;
#else
	const char* modelPath = "models\\backpack\\scene.gltf";
	constexpr int windowW = 1920, windowH = 1080;
#endif
	Renderer(windowW, windowH, modelPath).run();
}