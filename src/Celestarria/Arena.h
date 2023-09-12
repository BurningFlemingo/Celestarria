#pragma once
#include <stdint.h>
#include "Defines.h"

struct MemoryArena {
	void* base{};
	size_t size{};
	size_t used{};
};


MemoryArena constructArena(void* baseAddress, size_t size);
void* pushSize(MemoryArena* arena, uint32_t size);

template <typename T>
T* pushStruct(MemoryArena* arena) {
	PRECOND(arena->size >= sizeof(T) + arena->used);

	void* data{ pushSize(arena, sizeof(T)) };
	return static_cast<T*>(data);
}

template <typename T>
T* pushArray(MemoryArena* arena, uint32_t count) {
	PRECOND(arena->size >= (sizeof(T) * count) + arena->used);

	void* dataPtr{ static_cast<uint8_t*>(arena->base) + arena->used };

	for (uint32_t i{}; i < count; i++) {
		arena->used += sizeof(T);
	}

	return static_cast<T*>(dataPtr);
}

