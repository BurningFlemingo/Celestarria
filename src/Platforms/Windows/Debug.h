#pragma once

#include "../../Celestarria/Game.h"
#include "Window.h"
#include "Audio.h"
#include <stdint.h>

struct DEBUGCursor {
	uint32_t playSamplePosition{};  // in bufferSize
	uint32_t writeSamplePosition{};  // in bufferSize
};

void DEBUGSyncDisplay(
	BitmapBuffer* backbuffer,
	DEBUGCursor* cursors,
	uint32_t latestCursorIndex,
	uint32_t cursorCount,
	AudioOutputInfo* audioOutputInfo,
	AudioBufferState* audioBufferState,
	uint32_t jitter
);

void DEBUGDrawLine(
	BitmapBuffer* backbuffer,
	int32_t x,
	int32_t bottom,
	int32_t top,
	uint32_t color
);

inline void DEBUGVerifyInputBounds(Game::Input* input) {
	ASSERT(input->keyboard.downSize <= input->keyboard.maxSize);
	ASSERT(input->keyboard.upSize <= input->keyboard.maxSize);
	ASSERT(input->keyboard.typedSize <= input->keyboard.maxTypedSize);

	ASSERT(input->mouse.upSize <= input->mouse.maxSize);
	ASSERT(input->mouse.downSize <= input->mouse.maxSize);

	ASSERT(input->controller.downSize <= input->controller.maxSize);
	ASSERT(input->controller.upSize <= input->controller.maxSize);
}
