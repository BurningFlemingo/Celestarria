#pragma once
#include <stdint.h>

#include "Arena.h"
#include "Defines.h"
#include "FileIO.h"
#include "Tiles.h"

struct Pixel {
	uint8_t b{};
	uint8_t g{};
	uint8_t r{};
	uint8_t a{};
};

struct World {
	Tilemap tilemap{};
};

struct State {
	float pxPerTx;
	TilemapPos playerPos;
	MemoryArena worldArena;

	BMP playerSpriteBMP;
	BMP dirtBMP;
	BMP backgroundBMP;

	bool up;
	bool down;
	bool left;
	bool right;
	bool running;

	World world;
};
