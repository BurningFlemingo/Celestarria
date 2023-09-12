#include "Utils.h"
#include "../../Celestarria/Defines.h"

uint32_t stringLength(char* string) {
	ASSERT(string);

	if (string) {
		uint32_t count{};
		while (*string++) {
			count++;
		}
		return count;
	}

	return 0;
}

void catString(char* srcA, size_t srcACount, char* srcB, size_t srcBCount, char* dst, size_t dstCount) {
	if ((srcACount + srcBCount + 1) > dstCount) {
		ASSERT(false);
		return;
	}
	for (int i{ 0 }; i < srcACount; i++) {
		*dst++ = *srcA++;
	}
	for (int i{ 0 }; i < srcBCount; i++) {
		*dst++ = *srcB++;
	}

	*dst = 0;
}


