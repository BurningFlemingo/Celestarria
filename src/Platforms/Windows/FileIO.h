#pragma once

#include <Windows.h>
#include "../../Celestarria/Game.h"

struct GameDLL {
	Game::UpdateAndRender* updateAndRender{};
	Game::GetSoundSamples* getSoundSamples{};

	HMODULE dll{};
	FILETIME lastWriteTime{};
	bool isValid{ false };
};

void DEBUGPlatformFreeFileMemory(Game::ThreadContext* thread, void* memory);
Platform::DEBUGReadFileResult DEBUGPlatformReadEntireFile(Game::ThreadContext* thread, const char* filename);
bool DEBUGPlatformWriteEntireFile(Game::ThreadContext* thread, const char* filename, uint32_t memSize, void* memory);

inline FILETIME getLastWriteTime(char* filename) {
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	FILETIME lastWriteTime{};

	bool found = GetFileAttributesExA(filename, GetFileExInfoStandard, &fileData);
	if (found) {
		lastWriteTime = fileData.ftLastWriteTime;
	}

	return lastWriteTime;
}
GameDLL loadGameDLL(char* gameDLLPath, char* gameIntermediaryDLLPath);
void unloadGameDLL(GameDLL* gameDLL);
