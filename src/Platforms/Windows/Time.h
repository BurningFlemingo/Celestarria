#pragma once
#include <Windows.h>
#include <stdint.h>

inline int64_t getPerfCountFreq() {
	LARGE_INTEGER perfCountRes;
	QueryPerformanceFrequency(&perfCountRes);
	return perfCountRes.QuadPart;	
}

inline float getSecondsElapsed(const int64_t perfCountFreq, LARGE_INTEGER startCount, LARGE_INTEGER endCount) {
	ASSERT(endCount.QuadPart >= startCount.QuadPart);

	int64_t counterElapsed = endCount.QuadPart - startCount.QuadPart;
	float secondsElapsed = ((static_cast<float>(counterElapsed)) / perfCountFreq);
	return secondsElapsed;
}

// time in queryperformancefrequency / second
LARGE_INTEGER getClock() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter;
}
