#pragma once

#include "FileIO.h"
#include "../../Celestarria/Defines.h"
#include <Windows.h>

namespace Game {
	bool updateAndRenderStub(Game::ThreadContext* thread, Game::Memory* memory, Game::Backbuffer* backbuffer, Game::Input* input, float dT) {
		return false;
	}
	
	void getSoundSamplesStub(Game::ThreadContext* thread, Game::Memory* memory, Game::SoundBuffer* soundBuffer) {}
}

inline uint32_t SafeUInt32Truncate(uint64_t size) {
	LOG_ASSERT(size <= 0xFFFFFF, "file size too big, truncating");
	return static_cast<uint32_t>(size);
}

void DEBUGPlatformFreeFileMemory(Game::ThreadContext* thread, void* memory) {
	if (memory) {
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

Platform::DEBUGReadFileResult DEBUGPlatformReadEntireFile(Game::ThreadContext* thread, const char* filename) {
	HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		return {};
	}

	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(fileHandle, &fileSize)) {
		CloseHandle(fileHandle);
		return {};
	}

	void* fileContents = VirtualAlloc(0, static_cast<size_t>(fileSize.QuadPart), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!fileContents) {
		CloseHandle(fileHandle);
		return {};
	}

	uint32_t fileSize32 = SafeUInt32Truncate(fileSize.QuadPart);
	DWORD bytesRead;
	if (!ReadFile(fileHandle, fileContents, fileSize32, &bytesRead, 0)) {
		CloseHandle(fileHandle);
		DEBUGPlatformFreeFileMemory(nullptr, fileContents);
		return {};
	}

	if (bytesRead != fileSize32) {
		CloseHandle(fileHandle);
		DEBUGPlatformFreeFileMemory(nullptr, fileContents);
		return {};
	}

	CloseHandle(fileHandle);
	Platform::DEBUGReadFileResult file{};
	file.contents = fileContents;
	file.size = fileSize32;
	return file;
}

bool DEBUGPlatformWriteEntireFile(Game::ThreadContext* thread, const char* filename, uint32_t memSize, void* memory) {
	HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		return true;
	}

	DWORD bytesWritten;
	if (!WriteFile(fileHandle, memory, memSize, &bytesWritten, 0)) {
		CloseHandle(fileHandle);
		return true;
	}

	if (bytesWritten != memSize) {
		CloseHandle(fileHandle);
		return true;	
	}

	CloseHandle(fileHandle);
	return false;
}

GameDLL loadGameDLL(char* gameDLLPath, char* gameIntermediaryDLLPath) {
	GameDLL gameDLL{};

	// so that we can compile the dll while the intermediary is being used
	CopyFileA(gameDLLPath, gameIntermediaryDLLPath, FALSE);
	gameDLL.dll = LoadLibraryA(gameIntermediaryDLLPath);

	if (gameDLL.dll) {
		gameDLL.updateAndRender = reinterpret_cast<Game::UpdateAndRender*>(GetProcAddress(gameDLL.dll, "updateAndRender"));
		gameDLL.getSoundSamples = reinterpret_cast<Game::GetSoundSamples*>(GetProcAddress(gameDLL.dll, "getSoundSamples"));

		gameDLL.isValid = gameDLL.updateAndRender && gameDLL.getSoundSamples;
	}

	if (!gameDLL.isValid) {
		gameDLL.getSoundSamples = Game::getSoundSamplesStub;
		gameDLL.updateAndRender = Game::updateAndRenderStub;
	}

	gameDLL.lastWriteTime = getLastWriteTime(gameDLLPath);

	return gameDLL;
}

void unloadGameDLL(GameDLL* gameDLL) {
	if (gameDLL->dll) {
		FreeLibrary(gameDLL->dll);
	}
	gameDLL->isValid = false;
	gameDLL->getSoundSamples = Game::getSoundSamplesStub;
	gameDLL->updateAndRender = Game::updateAndRenderStub;
}
