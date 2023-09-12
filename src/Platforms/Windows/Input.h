#pragma once

#include "../../Celestarria/Game.h"
#include "History.h"
#include <stdint.h>

// must define unicode window
void getInputFileLocation(History* gameHistory, bool inputStream, uint32_t slot, char* dst, uint32_t dstCount);
void updatePlayerInput(Game::Input* playerInput, History* gameHistory, bool windowFocused);

#ifdef INTERNAL
void recordInput(History* gameHistory, const Game::Input input);
void playbackInput(History* gameHistory, Game::Input* input);
#endif

inline void resetGameInput(Game::Input* input) {
	input->keyboard.downSize = 0;
	input->keyboard.upSize = 0;
	input->keyboard.typedSize = 0;

	input->mouse.downSize = 0;
	input->mouse.upSize = 0;

	input->controller.downSize = 0;
	input->controller.upSize = 0;

	input->mouse.wheelDelta = 0;
}
