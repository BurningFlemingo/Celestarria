#pragma once

#include <stdint.h>

uint32_t stringLength(char* string);
void catString(char* srcA, size_t srcACount, char* srcB, size_t srcBCount, char* dst, size_t dstCount);
