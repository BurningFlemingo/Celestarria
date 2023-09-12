#pragma once
#include <stdint.h>

#include "Defines.h"

namespace Game {
struct ThreadContext;
};

namespace Platform {
struct DEBUGReadFileResult {
	void* contents{};
	uint32_t size{};
};

// imported functions
typedef DEBUGReadFileResult DEBUGReadEntireFile(
	Game::ThreadContext* thread, const char* filename
);
typedef bool DEBUGWriteEntireFile(
	Game::ThreadContext* thread, const char* filename, uint32_t memSize,
	void* memory
);
typedef void DEBUGFreeFileMemory(Game::ThreadContext* thread, void* memory);

}  // namespace Platform

namespace Game {
/* pixels are always bb gg rr aa or 0xAARRGGBB, and 32 bit wide
 * and top down
 */
struct Backbuffer {
	void* memory{};
	uint32_t width{};
	uint32_t height{};
};

// always 2 channels and 32 bits per sample (64 bits total for both channels)
struct SoundBuffer {
	int32_t* memory;
	uint32_t samplesPerSecond{};
	uint32_t nSamples{};
};

enum class MouseButton : uint8_t {
	left,
	right,
	middle,
	mouse4,
	mouse5,
};

enum class ControllerButton : char {
	// for ps4 / xbox compatibility, these are the xyab, shape buttons
	up,
	down,
	left,
	right,
	leftStickPush,
	rightStickPush,
	leftTrigger,
	rightTrigger,
	leftShoulder,
	rightShoulder,
	dpadUp,
	dpadDown,
	dpadLeft,
	dpadRight
};

struct ScreenPos {
	short x;
	short y;
};

// arr size = double the ammount of keys that can resonably be pressed at once
struct KeyboardInput {
	// used for up and down arrays
	static constexpr uint32_t maxSize{ 30 };
	static constexpr uint32_t maxTypedSize{ 30 };

	Scancode up[maxSize]{};
	Scancode down[maxSize]{};
	uint32_t typed[maxTypedSize]{};

	uint8_t upSize{};
	uint8_t downSize{};
	uint8_t typedSize{};
};

// abs positions use OS cursor position (cursor balistics)
struct MouseInput {
	static constexpr uint32_t maxSize{ 10 };

	ScreenPos cursorAbsEndPos{};

	MouseButton up[maxSize]{};
	MouseButton down[maxSize]{};

	ScreenPos upAbsPos[maxSize]{};
	ScreenPos downAbsPos[maxSize]{};

	uint8_t upSize{};
	uint8_t downSize{};

	int wheelDelta{};
};

struct ControllerInput {
	static constexpr uint32_t maxSize{ 20 };

	ControllerButton up[maxSize]{};
	ControllerButton down[maxSize]{};
	uint8_t upSize{};
	uint8_t downSize{};

	ScreenPos lsPosUp[maxSize]{};
	ScreenPos lsPosDown[maxSize]{};
	ScreenPos rsUpPos[maxSize]{};
	ScreenPos rsDownPos[maxSize]{};

	ScreenPos lsEndPos{};
	ScreenPos rsEndPos{};
};

struct Input {
	bool isAnalog{};
	bool resetState{};

	KeyboardInput keyboard{};
	MouseInput mouse{};
	ControllerInput controller{};
};

struct Memory {
	bool isInitialized{};
	size_t permanentMemorySize{};
	void* permanentMemory;	// REQUIRED: cleared to zero

	size_t transientMemorySize{};
	void* transientMemory;	// REQUIRED: cleared to zero

	struct PlatformFunctions {
		Platform::DEBUGReadEntireFile* debugReadEntireFile{};
		Platform::DEBUGWriteEntireFile* debugWriteEntireFile{};
		Platform::DEBUGFreeFileMemory* debugFreeFileMemory{};
	} platform;
};

struct Clock {
	float secondsElapsed{};
};

struct ThreadContext {
	int placeholder{};
};

// exported functions
typedef bool UpdateAndRender(
	ThreadContext* thread, Game::Memory* memory, Game::Backbuffer* backbuffer,
	Game::Input* input, float dT
);

typedef void GetSoundSamples(
	ThreadContext* thread, Game::Memory* memory, Game::SoundBuffer* soundBuffer
);
}  // namespace Game
