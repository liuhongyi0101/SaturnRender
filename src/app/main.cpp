/*
*
* See http://graphicrants.blogspot.de/2013/08/specular-brdf-reference.html for a good reference to the different functions that make up a specular BRDF
*/

#include <stdio.h>
#include "renderer/renderer.h"

Renderer *renderer;																		\
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						\
{																									\
if (renderer != NULL)																		\
{																								\
renderer->handleMessages(hWnd, uMsg, wParam, lParam);									\
}																								\
return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
}																									\
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)									\
{																									\
for (int32_t i = 0; i < __argc; i++) { Renderer::args.push_back(__argv[i]); };  			\
	renderer = new Renderer();															\
	renderer->initVulkan();																	\
	renderer->setupWindow(hInstance, WndProc);													\
	renderer->prepare();																		\
	renderer->renderLoop();																	\
	delete(renderer);																			\
	return 0;																						\
}