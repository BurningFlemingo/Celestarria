#include "Tiles.h"

#include "Scratch.h"
#include "utils/math.h"
#include "Arena.h"

// TODO: make functions take chunk instead of calculating it
bool isAbsTileInBounds(Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY) {
	if (absTileX < tilemap->chunkTilesWide * tilemap->chunksWide &&
		absTileY < tilemap->chunkTilesTall * tilemap->chunksTall) {
		return true;
	}
	return false;
}

bool isChunkInBounds(Tilemap* tilemap, uint32_t chunkX, uint32_t chunkY) {
	if (chunkX < tilemap->chunksWide && chunkY < tilemap->chunksTall) {
		return true;
	}
	return false;
}

void realignCoord(Tilemap* tilemap, uint32_t* packedTile, float* tileRel) {
	int32_t tileOffset{ roundToInt(*tileRel / tilemap->tileTxSize) };
	*packedTile += tileOffset;

	*tileRel -= tileOffset * static_cast<int>(tilemap->tileTxSize);
}

TilemapPos realignPosition(
	Tilemap* tilemap, TilemapPos pos, float inBoundsXOff, float inBoundsYOff
) {
	pos.tileRelX += inBoundsXOff;
	pos.tileRelY += inBoundsYOff;
	realignCoord(tilemap, &pos.absTileX, &pos.tileRelX);
	realignCoord(tilemap, &pos.absTileY, &pos.tileRelY);

	return pos;
}

RelTilePos getRelTilePos(
	Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY
) {
	uint32_t chunkTileX{ absTileX & tilemap->chunkMask };
	uint32_t chunkTileY{ absTileY & tilemap->chunkMask };

	uint32_t chunkIndexX{ absTileX >> tilemap->chunkShift };
	uint32_t chunkIndexY{ absTileY >> tilemap->chunkShift };

	RelTilePos pos{};
	pos.relTileX = chunkTileX;
	pos.relTileY = chunkTileY;
	pos.chunkX = chunkIndexX;
	pos.chunkY = chunkIndexY;

	return pos;
}

Chunk* getChunk(
	Tilemap* tilemap, uint32_t inBoundsChunkX, uint32_t inBoundsChunkY
) {
	return &tilemap->chunks
				[(inBoundsChunkY * tilemap->chunksWide) + inBoundsChunkX];
}

uint32_t* getTile_Nullptr(
	Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY
) {
	bool inBounds{isAbsTileInBounds(tilemap, absTileX, absTileY)};
	if (!inBounds) {
		return nullptr;
	}

	RelTilePos chunkPos{
		getRelTilePos(tilemap, absTileX, absTileY)
	};
	Chunk* chunk{ getChunk(tilemap, chunkPos.chunkX, chunkPos.chunkY) };

	if (!chunk->tiles) {
		return nullptr;
	}
	uint32_t* tile{ &chunk->tiles
				[(chunkPos.relTileY * tilemap->chunkTilesWide) +
				 chunkPos.relTileX]};
	return tile;

}

void setTileValue(
	MemoryArena* arena, Tilemap* tilemap, uint32_t absTileX, uint32_t absTileY,
	uint32_t tileValue
) {
	bool inBounds{isAbsTileInBounds(tilemap, absTileX, absTileY)};
	if (!inBounds) {
		return;
	}

	RelTilePos chunkPos{
		getRelTilePos(tilemap, absTileX, absTileY)
	};
	Chunk* chunk{ getChunk(tilemap, chunkPos.chunkX, chunkPos.chunkY) };
	if (!chunk->tiles) {
		chunk->tiles = pushArray<uint32_t>(arena, tilemap->chunkTilesTall * tilemap->chunkTilesWide);
		for (uint32_t tile{}; tile < tilemap->chunkTilesWide * tilemap->chunkTilesTall; tile++) {
			chunk->tiles[tile] = 1;
		}
	}

	uint32_t* tile{ &chunk->tiles
				[(chunkPos.relTileY * tilemap->chunkTilesWide) +
				 chunkPos.relTileX]};
	*tile = tileValue;
}

uint32_t getOffsettedPointTileValue(
	Tilemap* tilemap, TilemapPos pos, float inBoundsXOff, float inBoundsYOff
) {
	pos.tileRelX += inBoundsXOff;
	pos.tileRelY += inBoundsYOff;
	realignCoord(tilemap, &pos.absTileX, &pos.tileRelX);
	realignCoord(tilemap, &pos.absTileY, &pos.tileRelY);

	PRECOND(isAbsTileInBounds(tilemap, pos.absTileX, pos.absTileY));

	uint32_t tileValue{ *getTile_Nullptr(tilemap, pos.absTileX, pos.absTileY) };
	return tileValue;
}
