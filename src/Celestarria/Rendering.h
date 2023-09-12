#pragma once

#include "utils/Math.h"
#include "Game.h"
#include "FileIO.h"
#include <stdint.h>

using namespace Game;

void drawRect(
	Backbuffer* backbuffer, float x, float y, float width, float height,
	float r, float g, float b
);
void drawBMP(
	Backbuffer* backbuffer, int screenOffsetX, int screenOffsetY,
	uint32_t bmpOffsetX, uint32_t bmpOffsetY, uint32_t bmpWidth, uint32_t bmpHeight, BMP* bmp
);
