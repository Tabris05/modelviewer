#ifdef NDEBUG
#define main WinMain
#endif

#include "renderer.h"

int main() {
	Renderer::make().run();
}