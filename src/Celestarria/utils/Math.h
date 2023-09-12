#pragma once

#include <stdint.h>
#include "../Defines.h"
#include <intrin.h>

inline int roundToInt(float x) {
	if (x > 0) {
		return static_cast<int>(x + 0.5f);
	}
	return static_cast<int>(x - 0.5f);
};

inline uint32_t roundToUInt(float x) {
	return static_cast<uint32_t>(x + 0.5f);
};

inline float abs(float a) {
	if (a > 0) {
		return a;
	}
	return a * -1;
}

inline int abs(int a) {
	if (a > 0) {
		return a;
	}
	return a * -1;
}

inline float mod(float a, float b) {
	if (a < 0) {
		int i{};
	}
	float result{ a - (b * static_cast<int>(a / b)) };
	return result;
}

inline int clampInt(int low, int x, int high) {
	if (low > x) {
		return low;
	}
	if (high < x) {
		return high;
	}
	return x;
};

inline int min(int x, int y) {
	if (x > y) {
		return y;
	}
	return x;
};

inline int max(int a, int b) {
	if (a > b) {
		return a;
	}
	return b;
}

inline float ceil(float a) {
	float result;
	int truncatedA{(int)a};
	if (a == truncatedA) {
		result = (float)truncatedA;
		return result;
	}
	if (a >= 0) {
		result = (float)(truncatedA + 1);
		return result;
	}
	result = (float)(truncatedA);
	return result;
}

struct FirstSetBit {
	char32_t found;
	uint32_t shift;
};


// example, 00100 would be 3, because 1 << 3 = 100
inline FirstSetBit findFirstSetBit(uint32_t val) {
	FirstSetBit result{};

#if COMPILER_MSVC
	result.found = _BitScanForward((unsigned long*)&result.shift, val);
#else
	constexpr uint32_t nBitsInUInt{ 32 };
	for (int i{}; i < nBitsInUInt; i++) {
		if (val & (1 << i)) {
			result.found = true;
			result.shift = i;
			break;
		}
	}

#endif
	return result;
}
