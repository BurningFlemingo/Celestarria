#pragma once

#include "Window.h"

namespace Global {
	BitmapBuffer backbuffer{};
	bool running{ true };
	bool windowFocused{};
	bool windowChanged{};
	bool resetInput{};
}

	// TODO: assert that no input related messages come through
LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
	LRESULT lResult{};
	switch (msg) {
	case WM_CLOSE: {
		Global::running = false;
	} break;
	case WM_QUIT: {
		Global::running = false;
	} break;
	case WM_DESTROY: {
		Global::running = false;
	} break;
	case WM_PAINT: {
		PAINTSTRUCT paint;
		HDC deviceContext = BeginPaint(window, &paint);
		WindowDimensions windowDims = getWindowDims(window);
		EndPaint(window, &paint);
	} break;
	case WM_SYSCOMMAND: {
		// remove beeping sound when alts + key is pressed
		if (wParam == SC_KEYMENU) {
			lResult = 0;
		}
		else {
			lResult = DefWindowProc(window, msg, wParam, lParam);
		}
	} break;
	case WM_SETFOCUS: {
		Global::windowFocused = true;
		Global::resetInput = false;

	} break;
	case WM_KILLFOCUS: {
		Global::windowFocused = false;
		Global::resetInput = true;
	} break;
	case WM_SIZE: {
		Global::windowChanged = true;

		HDC deviceContext = GetDC(window);
		WindowDimensions dims = getWindowDims(window);
		blitBackbuffer(&Global::backbuffer, deviceContext, dims.width, dims.height);
		ReleaseDC(window, deviceContext);

	} break;
	default: {
		lResult = DefWindowProc(window, msg, wParam, lParam);
	} break;
	}
	return  lResult;
}

void resizeBackbuffer(BitmapBuffer* bitmap, int width, int height) {
	constexpr int bytesPerPixel = 4;
	const uint32_t bitmapSize = (width * height) * bytesPerPixel;

	if (bitmap->memory) {
		VirtualFree(bitmap->memory, 0, MEM_RELEASE);
	}
	bitmap->memory = VirtualAlloc(0, bitmapSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	bitmap->width = width;
	bitmap->height = abs(height);

	bitmap->info.bmiHeader.biSize = sizeof(bitmap->info.bmiHeader);
	bitmap->info.bmiHeader.biWidth = bitmap->width;
	bitmap->info.bmiHeader.biHeight = -bitmap->height;
	bitmap->info.bmiHeader.biPlanes = 1;
	bitmap->info.bmiHeader.biBitCount = 32;
	bitmap->info.bmiHeader.biCompression = BI_RGB;

	bitmap->bytesPerPixel = bitmap->info.bmiHeader.biBitCount / 8;
}

void blitBackbuffer(const BitmapBuffer* bitmap, HDC deviceContext, int windowWidth, int windowHeight) {
	// NOTE: this is only for debug
	StretchDIBits(deviceContext,
		0, 0, bitmap->width, bitmap->height,
		0, 0, bitmap->width, bitmap->height,
		bitmap->memory,
		&bitmap->info,
		DIB_RGB_COLORS, SRCCOPY);
}

WindowDimensions getWindowDims(HWND window) {
	RECT windowRect{};
	GetClientRect(window, &windowRect);

	int windowWidth = windowRect.right - windowRect.left;
	int windowHeight = windowRect.bottom - windowRect.top;

	return { windowWidth, windowHeight };
}


