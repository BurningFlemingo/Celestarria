#include "Arena.h"
#include "Defines.h"

MemoryArena constructArena(void* baseAddress, size_t size) {
	MemoryArena arena;
	arena.base = baseAddress;
	arena.size = size;
	arena.used = 0;
	return arena;
}

void* pushSize(MemoryArena* arena, uint32_t size) {
	PRECOND(arena->size >= size + arena->used);
	void* dataPtr{ static_cast<uint8_t*>(arena->base) + arena->used };
	arena->used += size;

	return dataPtr;
}


