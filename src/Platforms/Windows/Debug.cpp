#pragma once

#include "Debug.h"

void DEBUGSyncDisplay(
	BitmapBuffer* backbuffer,
	DEBUGCursor* cursors,
	uint32_t latestCursorIndex,
	uint32_t cursorCount,
	AudioOutputInfo* audioOutputInfo,
	AudioBufferState* audioBufferState,
	uint32_t jitter
) {
	int padY{ 16 };

	float C = (backbuffer->width) / (float)(audioBufferState->bufferSize);

	for (uint32_t i{}; i < cursorCount; i++) {
		uint32_t playCursorPos = cursors[i].playSamplePosition;
		uint32_t writeCursorPos = (playCursorPos + cursors[i].writeSamplePosition) % audioBufferState->bufferSize;

		float playCursorXFloat = C * (float)playCursorPos;
		float writeCursorXFloat = C * (float)writeCursorPos;
		float leftJitterWindowXFloat = C * ((float)playCursorPos - jitter);
		float rightJitterWindowXFloat = C * ((float)playCursorPos + jitter);

		int playCursorX = (int)playCursorXFloat;
		int writeCursorX = (int)writeCursorXFloat;
		int leftJitterWindowX = (int)leftJitterWindowXFloat - 1;
		int rightJitterWindowX = (int)rightJitterWindowXFloat + 1;

		int top = padY;
		int bottom = top + padY * 5;

		uint32_t writeColor{ 0x00FF00FF };
		uint32_t playColor{ 0xFFFFFFFF };
		uint32_t jitterWindowColor{ 0x00000000 };

		if (i == latestCursorIndex - 1) {
			top = bottom + padY;
			bottom = top + padY * 5;
		}

		DEBUGDrawLine(backbuffer, playCursorX, bottom, top, playColor);
		DEBUGDrawLine(backbuffer, writeCursorX, bottom, top, writeColor);
		if (leftJitterWindowX >= 0) {
			DEBUGDrawLine(backbuffer, leftJitterWindowX, bottom, top, jitterWindowColor);
		}
		if (rightJitterWindowX < (backbuffer->width + 1) || rightJitterWindowX >= 0) {
			DEBUGDrawLine(backbuffer, rightJitterWindowX, bottom, top, jitterWindowColor);
		}
	}
}

void DEBUGDrawLine(
	BitmapBuffer* backbuffer,
	int32_t x,
	int32_t bottom,
	int32_t top,
	uint32_t color
) {
	if (x < 0 || x > backbuffer->width) {
		return;
	}
	if (bottom < 0) {
		bottom = 0;
	}
	if (bottom > backbuffer->height) {
		bottom = backbuffer->height;
	}
	if (top < 0) {
		top = 0;
	}
	if (top > bottom) {
		top = bottom;
	}
	uint8_t* pixel = static_cast<uint8_t*>(backbuffer->memory) + x * backbuffer->bytesPerPixel + top * backbuffer->width * backbuffer->bytesPerPixel;
	for (int32_t y{ top }; y < bottom; y++) {
		*(uint32_t*)pixel = color;
		pixel += backbuffer->width * 4;
	}
}