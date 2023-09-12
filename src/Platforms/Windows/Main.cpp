#include <Windows.h>

#include "../../Celestarria/Game.h"
#include "../../Celestarria/Defines.h"

#include "Audio.h"
#include "Window.h"
#include "Input.h"
#include "FileIO.h"
#include "Utils.h"
#include "History.h"
#include "Time.h"
#include "Debug.h"

#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#define HID_USAGE_PAGE_GAME         ((USHORT) 0x05)

#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#define HID_USAGE_GENERIC_KEYBOARD        ((USHORT) 0x06)
#define HID_USAGE_GENERIC_GAMEPAD        ((USHORT) 0x05)

// TODO: maybe make more procedural?
_Use_decl_annotations_ int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int) {
	char gameDLLFilePath[WIN32_FILENAME_MAX_COUNT];
	char gameIntermediaryDLLFilePath[WIN32_FILENAME_MAX_COUNT];

	char gameDLLFilename[] = "Celestarria.dll";
	char gameIntermediaryDLLFilename[] = "Celestarria_intermediary.dll";

	History gameHistory{};
	getEXEPathAndDirectory(&gameHistory);
	buildFileInEXEDirectory(&gameHistory, gameDLLFilename, WIN32_FILENAME_MAX_COUNT, gameDLLFilePath);
	buildFileInEXEDirectory(&gameHistory, gameIntermediaryDLLFilename, WIN32_FILENAME_MAX_COUNT, gameIntermediaryDLLFilePath);

	TIMECAPS timecaps{};
	timeGetDevCaps(&timecaps, sizeof(TIMECAPS));

	uint32_t targetSchedulerMS{ timecaps.wPeriodMin };
	bool sleepIsGranular = timeBeginPeriod(targetSchedulerMS) == TIMERR_NOERROR;

	const int64_t perfCountFreq = getPerfCountFreq();

	WNDCLASSW windowClass{};

	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = L"gameClass";
	// windowClass.hIcon;
	windowClass.lpfnWndProc = windowProc;

	RegisterClassW(&windowClass);
	
	RECT windowRect{};
	windowRect.right = (1920 / 2);
	windowRect.bottom = (1080 / 2);
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, 0, 0);

	HWND windowHandle = CreateWindowExW(
		0,
        L"gameClass",
        L"Celestarria",
        WS_OVERLAPPEDWINDOW, 
        CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
        0,
        0,
        instance,
        0
	);

	// HACK: to prevent white borders when resize, also causes startup to be transparent
	ShowWindow(windowHandle, SW_SHOWMINIMIZED);
	ShowWindow(windowHandle, SW_RESTORE);

	if (!windowHandle) {
		ASSERT(false);
		return 1;
	}

	{
		RAWINPUTDEVICE rid[3]{};
		rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		rid[0].dwFlags = RIDEV_DEVNOTIFY;
		rid[0].hwndTarget = windowHandle;

		rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		rid[1].dwFlags = RIDEV_DEVNOTIFY;
		rid[1].hwndTarget = windowHandle;

		rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[2].usUsage = HID_USAGE_GENERIC_GAMEPAD;
		rid[2].dwFlags = RIDEV_DEVNOTIFY;
		rid[2].hwndTarget = windowHandle;
		RegisterRawInputDevices(rid, sizeof(rid) / sizeof(rid[0]), sizeof(rid[0]));
	}

	Game::Memory gameMemory{};
	gameMemory.permanentMemorySize = Megabytes(1);
	gameMemory.transientMemorySize = Megabytes(10);

#ifdef INTERNAL
		LPVOID permanentBaseAddress = reinterpret_cast<void*>(Terabytes(1));

		size_t totalMemorySize = gameMemory.permanentMemorySize + gameMemory.transientMemorySize;
		gameHistory.gameMemorySize = totalMemorySize;

		// TODO: add support for large pages
		gameMemory.permanentMemory = VirtualAlloc(permanentBaseAddress, totalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		gameMemory.transientMemory = reinterpret_cast<void*>(static_cast<uint8_t*>(permanentBaseAddress) + gameMemory.permanentMemorySize);
		gameHistory.gameMemory = gameMemory.permanentMemory;

		for (int i{ 0 }; i < History::replayBufferCount; i++) {
			ReplayBuffer* replayBuffer{ &gameHistory.replayBuffers[i] };

			getInputFileLocation(&gameHistory, false, i, replayBuffer->filePath, sizeof(replayBuffer->filePath));
			replayBuffer->fileHandle = CreateFileA(replayBuffer->filePath,
				GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
				0,
				CREATE_ALWAYS,
				0, 0
			);

			LARGE_INTEGER gameMemSize;
			gameMemSize.QuadPart = gameHistory.gameMemorySize;
			replayBuffer->memoryHandle = CreateFileMappingA(
				replayBuffer->fileHandle,
				0, PAGE_READWRITE,
				gameMemSize.HighPart,
				gameMemSize.LowPart,
				0
			);

			replayBuffer->memory = MapViewOfFile(
				replayBuffer->memoryHandle,
				FILE_MAP_ALL_ACCESS,
				0, 0,
				gameHistory.gameMemorySize
			);
			LOG_ASSERT(gameHistory.replayBuffers[i].memory, "not enough memory for replay buffers\n");
			ASSERT(gameHistory.replayBuffers[i].memory);
		}
#else
		size_t totalMemorySize = static_cast<size_t>(gameMemory.permanentMemorySize + gameMemory.transientMemorySize);
		gameHistory.gameMemorySize = totalMemorySize;

		gameHistory.gameMemory = VirtualAlloc(0, totalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		gameMemory.permanentMemory = gameHistory.gameMemory;
		gameMemory.transientMemory = static_cast<uint8_t*>(gameMemory.permanentMemory) + gameMemory.permanentMemorySize;
#endif
	ASSERT(gameMemory.transientMemory && gameMemory.permanentMemory);

	gameMemory.platform.debugFreeFileMemory = DEBUGPlatformFreeFileMemory;
	gameMemory.platform.debugWriteEntireFile = DEBUGPlatformWriteEntireFile;
	gameMemory.platform.debugReadEntireFile = DEBUGPlatformReadEntireFile;

	Game::Input playerInput{};

	Global::windowFocused = (GetForegroundWindow() == windowHandle);

	uint32_t renderRefreshHz{ 60 };
	{
		HDC deviceContext{ GetDC(windowHandle) };
		int monitorRefreshRate = GetDeviceCaps(deviceContext, VREFRESH);
		if (monitorRefreshRate > 0) {
			renderRefreshHz = monitorRefreshRate;
		}
		ReleaseDC(windowHandle, deviceContext);
	}
	renderRefreshHz = 30;
	uint32_t updateRefreshHz{ static_cast<uint32_t>(renderRefreshHz / 2) };

	const float targetSecPerFrame{ (1.f / renderRefreshHz) };
	constexpr float msBufferSize{ 1000.f };
	AudioOutputInfo audioInfo = initWASAPI(msBufferSize);
	writeSilence((audioInfo.samplesPerSecond / renderRefreshHz) * 5);

	uint32_t minSamplesToPlayPerFrame{ static_cast<uint32_t>(audioInfo.samplesPerSecond) / renderRefreshHz };
	AudioBufferState bufferState = getBufferState();
	Game::SoundBuffer soundBuffer{};

	const uint32_t bufferSizeInBytes = (bufferState.bufferSize * 2) * audioInfo.bytesPerSample;
	soundBuffer.memory = static_cast<int*>(VirtualAlloc(0, bufferSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
	soundBuffer.samplesPerSecond = audioInfo.samplesPerSecond;
	uint32_t samplesQueuedNextFrame{ bufferState.samplesQueued };
	uint32_t writeCursorJitter{ (uint32_t)((audioInfo.samplesPerSecond / 300) + 1) };

#ifdef INTERNAL
	int dCursorIndex{};
	const int dCursorCount{ 20 };
	DEBUGCursor dCursors[dCursorCount]{};
	DEBUGCursor pc{};
#endif

	GameDLL gameDLL = loadGameDLL(gameDLLFilePath, gameIntermediaryDLLFilePath);
	Game::Backbuffer gameBackbuffer{};

	// TODO: make this work for any window size


	Game::ThreadContext thread{};

	float lastFrameTime{};
	LARGE_INTEGER lastClock = getClock();
	while (Global::running) {
		LARGE_INTEGER firstClock = getClock();

		{
			WindowDimensions winDims = getWindowDims(windowHandle);
			if (Global::windowChanged) {
				resizeBackbuffer(&Global::backbuffer, winDims.width, winDims.height);
				gameBackbuffer.width = Global::backbuffer.width;
				gameBackbuffer.height = Global::backbuffer.height;
				gameBackbuffer.memory = Global::backbuffer.memory;

				Global::windowChanged = false;
			}
		}

		{
			FILETIME newGameDLLFiletimeWritten{ getLastWriteTime(gameDLLFilePath) };
			if (CompareFileTime(&newGameDLLFiletimeWritten, &gameDLL.lastWriteTime) != 0) {
				unloadGameDLL(&gameDLL);
				gameDLL = loadGameDLL(gameDLLFilePath, gameIntermediaryDLLFilePath);
			}
		}

		{
			int32_t writeCursor = samplesQueuedNextFrame;
			int32_t writeFrameBoundary = minSamplesToPlayPerFrame - writeCursor + writeCursorJitter;

			uint32_t samplesToFill{};
			if (writeFrameBoundary < -1 * (int)(2 * writeCursorJitter)) {
				LOG_MSG("writeCursor is in front of frame boundary, < 1 frame of audio samples was written\n");
				// samplesToFill = minSamplesToPlayPerFrame - (writeCursor % minSamplesToPlayPerFrame);
				samplesToFill = 0;
			}
			else if (writeFrameBoundary > (int)(writeCursorJitter)) {
				LOG_MSG("writeCursor is before frameBoundary, this should only happen on startup\n");
				samplesToFill = minSamplesToPlayPerFrame + writeFrameBoundary;
			}
			else {
				samplesToFill = minSamplesToPlayPerFrame;
			}
			soundBuffer.nSamples = (uint32_t)samplesToFill;
			LOG_ASSERT((samplesToFill + samplesQueuedNextFrame) <= ((minSamplesToPlayPerFrame + writeCursorJitter) * 2), "too many samples\n");
		}

		updatePlayerInput(&playerInput, &gameHistory, true);
		ASSERT(!(gameHistory.recording && gameHistory.playing));
#ifdef INTERNAL
		if (gameHistory.recording) {
			recordInput(&gameHistory, playerInput);
		}
		if (gameHistory.playing) {
			playbackInput(&gameHistory, &playerInput);
		}
#endif
		DEBUGVerifyInputBounds(&playerInput);

		Global::running = gameDLL.updateAndRender(&thread, &gameMemory, &gameBackbuffer, &playerInput, lastFrameTime) && Global::running;
		if (!Global::running) {
			break;
		}
		gameDLL.getSoundSamples(&thread, &gameMemory, &soundBuffer);

		writeAudioBuffer(static_cast<void*>(soundBuffer.memory), soundBuffer.nSamples);
		resetGameInput(&playerInput);

		uint32_t nSpinlocked{};
		float workSecondsElapsed = getSecondsElapsed(perfCountFreq, lastClock, getClock());
		float frameSecondsElapsed{ workSecondsElapsed };
		if (frameSecondsElapsed < targetSecPerFrame) {
			if (sleepIsGranular) {
				uint32_t sleepMS{ static_cast<uint32_t>(1000.f * (targetSecPerFrame - frameSecondsElapsed)) };
				uint32_t sleepInnacuracy{ 3 };
				if (sleepMS >= (targetSchedulerMS + sleepInnacuracy)) {
					Sleep(sleepMS - sleepInnacuracy);
				}

				frameSecondsElapsed = getSecondsElapsed(perfCountFreq, lastClock, getClock());
				// ASSERT(frameSecondsElapsed < targetSecPerFrame)
			}

			while (frameSecondsElapsed < targetSecPerFrame) {
				frameSecondsElapsed = getSecondsElapsed(perfCountFreq, lastClock, getClock());
				nSpinlocked++;
			}
		}
		else {
			LOG_MSG("frame took longer took longer than target ms per frame\n");
		}
#if 0
		DEBUGSyncDisplay(&Global::backbuffer, dCursors, dCursorIndex, dCursorCount, &audioInfo, &bufferState, writeCursorJitter * 2);
#endif
		{
			WindowDimensions windowDims = getWindowDims(windowHandle);
			HDC deviceContext{ GetDC(windowHandle) };
			blitBackbuffer(&Global::backbuffer, deviceContext, Global::backbuffer.width, Global::backbuffer.height);
			lastClock = getClock();
			lastFrameTime = getSecondsElapsed(perfCountFreq, firstClock, lastClock);
			ReleaseDC(windowHandle, deviceContext);
		}

		startAudio();
		bufferState = getBufferState();
		samplesQueuedNextFrame = bufferState.samplesQueued;
		LOG_ASSERT(samplesQueuedNextFrame >= minSamplesToPlayPerFrame - (writeCursorJitter * 2), "too few samples after blit\n");
		LOG_ASSERT(samplesQueuedNextFrame <= minSamplesToPlayPerFrame + (writeCursorJitter * 2), "too many samples after blit\n");
#if 0
		bufferState = getBufferState();

		char wsPrintfBuffer[256];
		wsprintfA(wsPrintfBuffer, "%dus/f | %dsl %dsq %dpp %dsw\n", (int)(frameSecondsElapsed * 1000000), nSpinlocked, bufferState.samplesQueued, bufferState.playbackPosition, soundBuffer.nSamples);
		OutputDebugStringA(wsPrintfBuffer);

		pc.playSamplePosition = bufferState.playbackPosition;
		pc.writeSamplePosition = samplesQueuedNextFrame;

		ASSERT((dCursorIndex + 1) <= dCursorCount);
		dCursors[dCursorIndex++] = pc;
		if (dCursorIndex >= dCursorCount) {
			dCursorIndex = 0;
		}
 #endif
	}

	stopAudio();

	return 0;
}
