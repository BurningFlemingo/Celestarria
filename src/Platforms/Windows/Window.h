#pragma once
#include <Windows.h>
#include <stdint.h>

struct BitmapBuffer {
	BITMAPINFO info{};
	int width{};
	int height{};
	uint32_t bytesPerPixel{};
	void* memory{};
};

struct WindowDimensions {
	int width{};
	int height{};
};

LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);

void resizeBackbuffer(BitmapBuffer* bitmap, int width, int height);
// will not stretch (FOR DEBUG / prototyping)
void blitBackbuffer(const BitmapBuffer* bitmap, HDC deviceContext, int windowWidth, int windowHeight);
WindowDimensions getWindowDims(HWND window);

namespace Global {
	extern BitmapBuffer backbuffer;
	extern bool running;
	extern bool windowFocused;
	extern bool windowChanged;
	extern bool resetInput;
}

