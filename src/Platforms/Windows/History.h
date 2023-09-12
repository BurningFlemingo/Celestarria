#pragma once

#include "../../Celestarria/Defines.h"
#include <Windows.h>
#include <stdint.h>

#define WIN32_FILENAME_MAX_COUNT MAX_PATH

struct ReplayBuffer {
	HANDLE fileHandle{};
	HANDLE memoryHandle{};
	void* memory{};
	char filePath[WIN32_FILENAME_MAX_COUNT];
	bool isValid{};
};

struct History {
	void* gameMemory{};
	size_t gameMemorySize{};
	HANDLE replayHandle{};

	uint32_t replaySlot{};

	static constexpr uint32_t replayBufferCount{ 4 };
	ReplayBuffer replayBuffers[replayBufferCount];

	char exeFilePath[WIN32_FILENAME_MAX_COUNT];
	char* exeDirectory{};

	bool recording{};
	bool playing{};
};

// ansi
void getEXEPathAndDirectory(History* history);
void buildFileInEXEDirectory(History* history, char* filename, int dstCount, char* dst);
