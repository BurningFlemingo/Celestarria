#include "Rendering.h"

void drawRect(
	Backbuffer* backbuffer, float x, float y, float width, float height,
	float r, float g, float b
) {
	uint32_t color{ (roundToUInt(r * 255) << 16) | (roundToUInt(g * 255) << 8) |
					(roundToUInt(b * 255) << 0) };

	int screenX{ (int)(roundToUInt(x)) };
	int screenY{ (int)(roundToUInt(y)) };
	int screenWidth{ (int)(roundToUInt(width)) };
	int screenHeight{ (int)(roundToUInt(height)) };

	screenWidth = min(screenX + screenWidth, backbuffer->width);
	screenHeight = min(screenY + screenHeight, backbuffer->height);

	screenX = clampInt(0, screenX, backbuffer->width);
	screenY = clampInt(0, screenY, backbuffer->height);

	uint32_t* basePixel{ reinterpret_cast<uint32_t*>(backbuffer->memory) };
	for (int yPos{ screenY }; yPos < screenHeight; yPos++) {
		for (int xPos{ screenX }; xPos < screenWidth; xPos++) {
			basePixel[(yPos * backbuffer->width) + xPos] = color;
		}
	}
}

void drawBMP(
	Backbuffer* backbuffer, int screenOffsetX, int screenOffsetY,
	uint32_t bmpOffsetX, uint32_t bmpOffsetY, uint32_t bmpWidth,
	uint32_t bmpHeight, BMP* bmp
) {
	uint32_t* baseScreenPixel{ reinterpret_cast<uint32_t*>(backbuffer->memory
	) };

	if (bmpWidth == 0) {
		bmpWidth = bmp->width;
	} else {
		int endXTexel = min(bmpWidth + bmpOffsetX, bmp->width);
		bmpWidth = min(endXTexel - bmpOffsetX, 0);
	}

	if (bmpHeight == 0) {
		bmpHeight = bmp->height;
	} else {
		int endYTexel = min(bmpHeight + bmpOffsetY, bmp->height);
		bmpHeight = min(endYTexel - bmpOffsetY, 0);
	}

	int width{};
	int height{};
	if (screenOffsetX < (int)backbuffer->width &&
		screenOffsetY < (int)backbuffer->height) {
		int endXPixel{ min(bmpWidth + screenOffsetX, backbuffer->width) };
		int endYPixel{ min(bmpHeight + screenOffsetY, backbuffer->height) };

		// when screen offset is negative, the bmp should be cut off
		// respectively, therefore offset right, ONLY when its to the left or
		// top
		bmpOffsetX = bmpOffsetX + (-1 * min(screenOffsetX, 0));
		bmpOffsetY = bmpOffsetY + (-1 * min(screenOffsetY, 0));

		screenOffsetX = max(screenOffsetX, 0);
		screenOffsetY = max(screenOffsetY, 0);

		width += max(endXPixel - screenOffsetX, 0);
		height += max(endYPixel - screenOffsetY, 0);
	} else {
		return;
	}

	int xTexel{ (int)bmpOffsetX };
	int yTexel{ (int)bmpOffsetY };
	int firstXTexel{
		xTexel
	};	// to that x start is perserved when moving y up and down

	// -1 because when y is flipped, the max value is bmp->height, which doesnt
	// exist, so shift down by 1
	yTexel = bmp->height - yTexel - 1;

	for (uint32_t y{}; y < height; y++) {
		for (uint32_t x{}; x < width; x++) {
			uint32_t* screenPixel{
				&baseScreenPixel
					[(screenOffsetY + y) * backbuffer->width +
					 (screenOffsetX + x)]
			};

			uint32_t bmpTexel{ bmp->pixels
								   [(yTexel % bmp->height) * bmp->width +
									(xTexel % bmp->width)] };

			float a{ (bmpTexel >> 24) / 255.f };

			float sB{ (float)((bmpTexel >> 16) & 0xFF) };
			float sG{ (float)((bmpTexel >> 8) & 0xFF) };
			float sR{ (float)((bmpTexel >> 0) & 0xFF) };

			float dB{ (float)((*screenPixel >> 16) & 0xFF) };
			float dG{ (float)((*screenPixel >> 8) & 0xFF) };
			float dR{ (float)((*screenPixel >> 0) & 0xFF) };

			// linear blend
			// TODO: premultiplied alpha
			dB = (1.f - a) * dB + (a * sB);
			dG = (1.f - a) * dG + (a * sG);
			dR = (1.f - a) * dR + (a * sR);
			uint32_t color1{ (((uint32_t)(dB)&0xFF) << 16) |
							 (((uint32_t)(dG)&0xFF) << 8) |
							 (((uint32_t)(dR)&0xFF)) };

			*screenPixel = color1;
			xTexel++;
		}
		xTexel = firstXTexel;
		yTexel--;
	}
}
