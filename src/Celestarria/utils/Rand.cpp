#include "Rand.h"

uint32_t rand(uint64_t* index) {
	*index = *index * 11-3515245 + 12345;
	uint32_t randNum = (uint32_t)(*index / 65536) % 32768;
	return randNum;
}

