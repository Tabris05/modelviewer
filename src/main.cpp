#ifdef NDEBUG
#define main WinMain
#endif

#include "renderer.h"

// force discrete gpu if applicable
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main() {
	Renderer::make().run();
}