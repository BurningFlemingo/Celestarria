#pragma once

#include "Game.h"

#include "Arena.h"
#include "FileIO.h"
#include "Rendering.h"
#include "Scratch.h"
#include "utils/Math.h"
#include "utils/Rand.h"

using namespace Game;

void fillSoundBuffer(State* state, SoundBuffer* soundBuffer, float toneHz) {
	float wave_period =
		static_cast<float>(soundBuffer->samplesPerSecond) / toneHz;

	float* sampleOut = reinterpret_cast<float*>(soundBuffer->memory);
	if (!sampleOut) {
		return;
	}
	for (uint32_t sampleIndex{ 0 }; sampleIndex < soundBuffer->nSamples;
		 sampleIndex++) {
		// float sine_value = sinf(state->tSine);
		// float sample_value = sine_value * 0.05f;
		// *sampleOut++ = sample_value;
		// *sampleOut++ = sample_value;

		// state->tSine += (PI32_t * 2) / wave_period;
		// if (state->tSine >= (PI32_t * 2)) {
		// 	state->tSine -= (PI32_t * 2);
		// }

		*sampleOut++ = 0;
		*sampleOut++ = 0;
	}
}

extern "C" bool updateAndRender(
	ThreadContext* thread, Game::Memory* memory, Game::Backbuffer* backbuffer,
	Game::Input* input, float dT
) {
	State* state = static_cast<State*>(memory->permanentMemory);
	ASSERT(sizeof(*state) <= memory->permanentMemorySize);

	constexpr float screenTilesAcross{ 16 };
	Tilemap* tilemap{ &state->world.tilemap };
	if (!memory->isInitialized) {
		state->playerSpriteBMP = DEBUGLoadBPM(
			thread, memory->platform.debugReadEntireFile, "assets/DUCK.bmp"
		);
		state->backgroundBMP = DEBUGLoadBPM(
			thread, memory->platform.debugReadEntireFile, "assets/background.bmp"
		);
		state->dirtBMP = DEBUGLoadBPM(
			thread, memory->platform.debugReadEntireFile, "assets/dirt.bmp"
		);
		state->pxPerTx = 1.f;

		memory->isInitialized = true;
		state->playerPos.absTileX = 5;
		state->playerPos.absTileY = 5;

		state->worldArena = constructArena(
			static_cast<uint8_t*>(memory->permanentMemory) + sizeof(*state),
			memory->permanentMemorySize - sizeof(*state)
		);
		tilemap->chunks = reinterpret_cast<Chunk*>(state->worldArena.base);

		tilemap->chunkShift = 4;
		tilemap->chunkMask = (1 << tilemap->chunkShift) - 1;
		tilemap->chunksWide = 100;
		tilemap->chunksTall = 100;

		tilemap->chunkTilesWide = tilemap->chunkMask + 1;
		tilemap->chunkTilesTall = tilemap->chunkMask + 1;
		tilemap->chunks = pushArray<Chunk>(
			&state->worldArena, tilemap->chunksWide * tilemap->chunksTall
		);

		tilemap->tileTxSize = 8;

		uint64_t rngSeed{ 2 };

		int32_t randNum{};
		int32_t yOffset{};
		int32_t dirtYStart{ 30 };
		for (uint32_t tileX{};
			 tileX < tilemap->chunksWide * tilemap->chunkTilesWide; tileX++) {
			uint32_t tileValue{ 2 };

			yOffset += -1 + rand(&rngSeed) % 3;
			if (yOffset < 0) {
				yOffset = 0;
			}
			int ogTileY{ dirtYStart + yOffset };
			for (int tileY{ ogTileY }; tileY > -1; tileY--) {
				if (static_cast<uint32_t>(tileY) >=
					tilemap->chunksTall * tilemap->chunkTilesTall) {
					continue;
				}
				setTileValue(
					&state->worldArena, tilemap, tileX, tileY, tileValue
				);
				setTileValue(
					&state->worldArena, tilemap, tileX,
					dirtYStart + tilemap->chunkTilesTall, 1
				);
			}
		}
	}
	uint32_t playerWidth{ 10 };
	uint32_t playerHeight{ 21 };
	float playerR{ 0.5f };
	float playerG{ 0.f };
	float playerB{ 0.5f };

	if (input->resetState) {
		state->down = false;
		state->left = false;
		state->right = false;
		state->up = false;

		input->resetState = false;
	}

	float speed = 100;  // tx per second
	if (input->isAnalog == false) {
		for (int i = 0; i < input->keyboard.downSize; i++) {
			switch (input->keyboard.down[i]) {
			case Scancode::w: {
				state->up = true;
			} break;
			case Scancode::a: {
				state->left = true;
			} break;
			case Scancode::d: {
				state->right = true;
			} break;
			case Scancode::s: {
				state->down = true;
			} break;
			case Scancode::shiftLeft: {
				state->running = true;
			} break;
			case Scancode::tab: {
				return false;
			} break;
			default: break;
			}
		}

		for (int i = 0; i < input->mouse.downSize; i++) {
			switch (input->mouse.down[i]) {
			case MouseButton::left: {
			} break;
			default: break;
			}
		}
	}

	if (input->mouse.wheelDelta > 0) {
		state->pxPerTx *= 2.f;
	} else if (input->mouse.wheelDelta < 0) {
		state->pxPerTx *= 0.5f;
	}

	float dPlayerX{};
	float dPlayerY{ -speed / (30.f * 6) };
	if (state->running) {
		speed *= 8.f;
	}
	if (state->up) {
		dPlayerY += speed * dT;
	}
	if (state->down) {
		dPlayerY -= speed * dT;
	}
	if (state->left) {
		dPlayerX -= speed * dT;
	}
	if (state->right) {
		dPlayerX += speed * dT;
	}

	bool isValid{};
	TilemapPos newTilemapPos{
		realignPosition(tilemap, state->playerPos, dPlayerX, dPlayerY)
	};
	uint32_t newPlayerLeftX{ realignPosition(
								 tilemap, newTilemapPos, playerWidth / 2.f,
								 (float)newTilemapPos.absTileY
	)
								 .absTileX };
	uint32_t newPlayerRightX{ realignPosition(
								  tilemap, newTilemapPos, playerWidth / -2.f,
								  (float)newTilemapPos.absTileY
	)
								  .absTileX };
	isValid =
		(getTileValue_Defaulted(
			 tilemap, newPlayerLeftX, newTilemapPos.absTileY
		 ) == 1 &&
		 getTileValue_Defaulted(
			 tilemap, newPlayerRightX, newTilemapPos.absTileY
		 ) == 1);
	if (state->running) {
		isValid = true;
	}
	if (isValid) {
		state->playerPos = newTilemapPos;
	}

	drawRect(backbuffer, 0.f, 0.f, 100000.f, 100000.f, 0.75f, 0.f, 0.75f);

	float pxPerTile{ state->pxPerTx * tilemap->tileTxSize };
	drawBMP(
		backbuffer, 0, 0,
		(state->playerPos.absTileX * 8 + state->playerPos.tileRelX + 4) / 10.f,
		300, 0, 0, &state->backgroundBMP
	);

	float screenTilesWide{ backbuffer->width / pxPerTile };
	float screenTilesTall{ backbuffer->height / pxPerTile };

	const int32_t halfScreenTilesWide{ (int)(ceil(screenTilesWide) / 2.f) };
	const int32_t halfScreenTilesTall{ (int)(ceil(screenTilesTall) / 2.f) };

	/*
	 * there are 2 offsets being applied here
	 * 1: the offset of an odd number of tiles in width or height
	 * 2: the "half" tiles caused by not completely round screenTilesWidth or
	 * height
	 *
	 * the way the loop works is by getting the tile the players on, and getting
	 * screenTilesWidth / 2 tiles left and right, and centering it by using
	 * playerRelTile since the player screen position is the center of the
	 * screen, even tiles work automatically but odd tiles need to have half a
	 * tile width added to be in the center, so for even numbers itll be 0, and
	 * for odd itll be 0.5, which does exactly this
	 *
	 * the way 2: works is by simply taking the excess tiles on one edge, and
	 * moving it the oppisite direction by 0.5 times aka making sure theres the
	 * same ammount on both the left and right, the offset will never be more
	 * than 1 tile so whats happening here is basacally halfing the decimal
	 * difference, example, screenTilesTall = 5.6 -> 0.3 * pxPerTile, a 0.3
	 * shift up makes a 0.3 border above and below, and 5 tiles rendered in the
	 * center
	 *
	 * form floor(ceil(x) / 2) - (x / 2) no clue how this works, but it does :D
	 */
	int screenOffsetX = roundToInt(
		(((int)(ceil(screenTilesWide) / 2.f)) - (screenTilesWide / 2.f)) *
		pxPerTile
	);
	int screenOffsetY = roundToInt(
		(((int)(ceil(screenTilesTall) / 2.f)) - (screenTilesTall / 2.f)) *
		pxPerTile
	);

	// adding 1 to edges for padding, needed for smooth scrolling and offsets
	// theres a bit of overdraw for even screen offsets
	for (int screenTileY{ -1 }; screenTileY < ceil(screenTilesTall) + 1;
		 screenTileY++) {
		for (int screenTileX{ -1 }; screenTileX < ceil(screenTilesWide) + 1;
			 screenTileX++) {
			int gameTileXOffset{ screenTileX - halfScreenTilesWide };
			int gameTileYOffset{ screenTileY - halfScreenTilesTall };

			uint32_t gameTileX{ state->playerPos.absTileX + gameTileXOffset };
			uint32_t gameTileY{ state->playerPos.absTileY + gameTileYOffset };

			uint32_t tileVal{
				getTileValue_Defaulted(tilemap, gameTileX, gameTileY)
			};

			if (tileVal == 0 || tileVal == 1) {
				continue;
			}

			int pxPerTileRounded{ (int)ceil(pxPerTile) };

			int screenPixelX{ pxPerTileRounded * screenTileX };
			int screenPixelY{ pxPerTileRounded * screenTileY };

			screenPixelX -= screenOffsetX;
			screenPixelY -= screenOffsetY;

			// tileRel goes from -4 to 4, so +4 to make it 0 to 8
			float tileRelXToTx{ state->playerPos.tileRelX + 4.f };
			float tileRelYToTx{ state->playerPos.tileRelY + 4.f };

			screenPixelX -= roundToInt(tileRelXToTx * state->pxPerTx);
			screenPixelY -= roundToInt(tileRelYToTx * state->pxPerTx);

			/*
			 * y is flipped
			 *
			 * -1 because pixel at backbuffer->height doesnt exist
			 *  but pixel at 0 does, therefore the -1 shifts it all down to
			 * account
			 */
			screenPixelY = backbuffer->height - screenPixelY - pxPerTileRounded;

			drawBMP(backbuffer, screenPixelX, screenPixelY, 0, 0, 0, 0, &state->dirtBMP);
		}
	}
	float screenPlayerWidth{ playerWidth * state->pxPerTx };
	float screenPlayerHeight{ playerHeight * state->pxPerTx };

	drawBMP(
		backbuffer, backbuffer->width / 2.f - (state->playerSpriteBMP.width / 2.f),
		(backbuffer->height / 2.f) - state->playerSpriteBMP.height, 0, 0, 0, 0, &state->playerSpriteBMP
	);
	BMP transpTest = DEBUGLoadBPM(
		thread, memory->platform.debugReadEntireFile, "assets/transptest.bmp"
	);
	drawBMP(
		backbuffer, backbuffer->width / 2.f - (transpTest.width / 2.f),
		(backbuffer->height / 2.f) - transpTest.height, 120, 0, 0, 0, &transpTest
	);
	// drawRect(
	// 	backbuffer,
	// 	backbuffer->width / 2.f - ((playerWidth * state->pxPerTx) / 2.f),
	// 	(backbuffer->height / 2.f) - (playerHeight * state->pxPerTx),
	// 	playerWidth * state->pxPerTx, playerHeight * state->pxPerTx, 0.f, 1.f,
	// 	1.f
	// );

	// drawRect(
	// 	backbuffer, backbuffer->width / 2.f - (screenPlayerWidth / 2),
	// 	backbuffer->height / 2.f - screenPlayerHeight, screenPlayerWidth,
	// 	screenPlayerHeight, playerR, playerG, playerB
	// );
	// drawRect(
	// 	backbuffer, backbuffer->width / 2.f,
	// 	backbuffer->height / 2.f,
	// 	1,
	// 	1,
	// 	1.f, 0, 0
	// );

	if (input->isAnalog == false) {
		for (int i = 0; i < input->keyboard.upSize; i++) {
			switch (input->keyboard.up[i]) {
			case Scancode::w: state->up = false; break;
			case Scancode::a: {
				state->left = false;
			} break;
			case Scancode::d: {
				state->right = false;
			} break;
			case Scancode::s: {
				state->down = false;
			} break;
			case Scancode::shiftLeft: {
				state->running = false;
			} break;
			default: break;
			}
		}

		for (int i = 0; i < input->mouse.upSize; i++) {
			switch (input->mouse.down[i]) {
			case MouseButton::left: {
			};
			default: break;
			}
		}
	}

	return true;
}

extern "C" void getSoundSamples(
	ThreadContext* thread, Game::Memory* memory, Game::SoundBuffer* soundBuffer
) {
	State* gameState = static_cast<State*>(memory->permanentMemory);
	fillSoundBuffer(gameState, soundBuffer, 20);
}
