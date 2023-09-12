#pragma once

#include <stdint.h>

#include "Game.h"

struct BMP {
	void* file;	 // needed to release the bmp

	uint32_t* pixels;
	int32_t width;
	int32_t height;
};

#pragma pack(push, 1)
struct BitmapHeader {
	uint16_t fileType;
	uint32_t fileSize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t bitmapOffset;
	uint32_t size;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bitsPerPixel;
	uint32_t compressionMethod;
	uint32_t sizeofBitmap;
	int32_t horizontalResolution;
	int32_t verticalResolution;
	uint32_t nColorsUsed;
	uint32_t nColorsImportant;	// generally ignored

	uint32_t redMask;
	uint32_t greenMask;
	uint16_t blueMask;
};
#pragma pack(pop)

// 	windows bmp:  0XAARRGGBB or BB GG RR AA and top-down
BMP DEBUGLoadBPM(
	Game::ThreadContext* thread, Platform::DEBUGReadEntireFile* readEntireFile,
	const char* filePath
);
