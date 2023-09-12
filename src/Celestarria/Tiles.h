#pragma once
#include <stdint.h>

struct MemoryArena;

struct Chunk {
	uint32_t* tiles{};
};

// tiles are unpacked, as in, they only contain tile index
struct RelTilePos {
	uint32_t chunkX{};
	uint32_t chunkY{};

	uint32_t relTileX{};
	uint32_t relTileY{};
};

struct TilemapPos {
	uint32_t absTileX{};
	uint32_t absTileY{};

	float tileRelX{};
	float tileRelY{};
};

struct Tilemap {
	uint32_t chunksWide{};
	uint32_t chunksTall{};

	uint32_t chunkShift{};	// 2^chunkShift bits reserved for tile location
							// inside chunk
	uint32_t chunkMask{};

	uint32_t chunkTilesWide{};
	uint32_t chunkTilesTall{};

	uint32_t tileTxSize{};
	Chunk* chunks{};
};

// may realign to an invalid tile
void realignCoord(Tilemap* tilemap, uint32_t* packedTile, float* tileRel);

TilemapPos realignPosition(
	Tilemap* tilemap, TilemapPos pos, float inBoundsXOff, float inBoundsYOff
);

RelTilePos getRelTilePos(
	Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY
);

bool isChunkInBounds(Tilemap* tilemap, uint32_t chunkX, uint32_t chunkY);

Chunk* getChunk(
	Tilemap* tilemap, uint32_t inBoundsChunkX, uint32_t inBoundsChunkY
);

bool isAbsTileInBounds(Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY);

uint32_t* getTile_Nullptr(
	Tilemap* tilemap, uint32_t inBoundsAbsTileX, uint32_t inBoundsAbsTileY
);

inline uint32_t getTileValue_Defaulted(
	Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY
) {
	uint32_t* tile{ getTile_Nullptr(tilemap, absTileX, absTileY) };
	if (tile) {
		return *tile;
	}
	return 0;
}

void setTileValue(
	MemoryArena* arena, Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY,
	uint32_t tileValue
);

void setTileValue(
	Tilemap* tilemap, Chunk* chunk, uint32_t absTileX, uint32_t absTileY,
	uint32_t tileValue
);

uint32_t getOffsettedPointTileValue(
	Tilemap* tilemap, TilemapPos pos, float inBoundsXOff, float inBoundsYOff
);

bool isOffsettedPointTileZero_Defaulted(
	Tilemap* tilemap, TilemapPos pos, float inBoundsXOff, float inBoundsYOff
);
