#pragma once

#include "Input.h"

#include <Windows.h>

#include "../../Celestarria/Defines.h"
#include "Utils.h"

/*
	shouts out to andrew chronister:
	https://handmade.network/forums/articles/t/2823-keyboard_inputs_-_scancodes%252C_raw_input%252C_text_input%252C_key_names
	for making scancodes / keyboard input less painfull
*/

namespace {
#ifdef INTERNAL
void handleHistoryPlayback(History* history, uint32_t slot);
void handleHistoryRecording(History* history, uint32_t slot);

void beginRecordingInput(History* gameHistory, uint32_t replaySlot);
void endRecordingInput(History* gameHistory);
void beginInputPlayback(History* gameHistory, uint32_t replaySlot);
void endInputPlayback(History* gameHistory);
#endif
Scancode processScancode(uint32_t scancode, bool* pauseScancodeRead);
}  // namespace

// must define unicode window
void getInputFileLocation(
	History* gameHistory, bool inputStream, uint32_t slot, char* dst,
	uint32_t dstCount
) {
	ASSERT(slot < gameHistory->replayBufferCount);

	char filename[WIN32_FILENAME_MAX_COUNT];
	wsprintfA(
		filename, "Celestarria_replay_history_%d_%s.ch", slot,
		inputStream ? "input" : "state"
	);
	buildFileInEXEDirectory(gameHistory, filename, dstCount, dst);
}

void updatePlayerInput(
	Game::Input* playerInput, History* gameHistory, bool windowFocused
) {
#ifdef INTERNAL
	static bool lPressed{ false };
	static bool pPressed{ false };
	static bool mPressed{ false };
#endif
	static bool pauseScancodeRead{ false };

	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		switch (msg.message) {
		case WM_MOUSEMOVE: {
			if (windowFocused) {
				short xPos = static_cast<short>(msg.lParam);
				short yPos = static_cast<short>(msg.lParam >> 16);

				playerInput->mouse.cursorAbsEndPos.x = xPos;
				playerInput->mouse.cursorAbsEndPos.y = yPos;
			}
		} break;
		case WM_INPUT: {
			UINT dwSize = sizeof(RAWINPUT);
			static BYTE lpb[sizeof(RAWINPUT)];

			GetRawInputData(
				(HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize,
				sizeof(RAWINPUTHEADER)
			);

			RAWINPUT* raw = (RAWINPUT*)lpb;

			// TOPDO: change this to use WM events?
			if (raw->header.dwType == RIM_TYPEMOUSE) {
				if (playerInput->mouse.downSize < playerInput->mouse.maxSize) {
					switch (raw->data.mouse.usButtonFlags) {
					case RI_MOUSE_LEFT_BUTTON_DOWN: {
						playerInput->mouse.down[playerInput->mouse.downSize] =
							Game::MouseButton::left;
						playerInput->mouse
							.downAbsPos[playerInput->mouse.downSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_RIGHT_BUTTON_DOWN: {
						playerInput->mouse.down[playerInput->mouse.downSize] =
							Game::MouseButton::right;
						playerInput->mouse
							.downAbsPos[playerInput->mouse.downSize++] =
							playerInput->mouse.cursorAbsEndPos;

					} break;
					case RI_MOUSE_MIDDLE_BUTTON_DOWN: {
						playerInput->mouse.down[playerInput->mouse.downSize] =
							Game::MouseButton::middle;
						playerInput->mouse
							.downAbsPos[playerInput->mouse.downSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_BUTTON_4_DOWN: {
						playerInput->mouse.down[playerInput->mouse.downSize] =
							Game::MouseButton::mouse4;
						playerInput->mouse
							.downAbsPos[playerInput->mouse.downSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_BUTTON_5_DOWN: {
						playerInput->mouse.down[playerInput->mouse.downSize] =
							Game::MouseButton::mouse5;
						playerInput->mouse
							.downAbsPos[playerInput->mouse.downSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					}
				} else {
					LOG_ASSERT(false, "mouse key down missed, maxSize reached");
				}

				if (playerInput->mouse.upSize < playerInput->mouse.maxSize) {
					switch (raw->data.mouse.usButtonFlags) {
					case RI_MOUSE_LEFT_BUTTON_UP: {
						playerInput->mouse.up[playerInput->mouse.upSize] =
							Game::MouseButton::left;
						playerInput->mouse
							.upAbsPos[playerInput->mouse.upSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_RIGHT_BUTTON_UP: {
						playerInput->mouse.up[playerInput->mouse.upSize] =
							Game::MouseButton::right;
						playerInput->mouse
							.upAbsPos[playerInput->mouse.upSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_MIDDLE_BUTTON_UP: {
						playerInput->mouse.up[playerInput->mouse.upSize] =
							Game::MouseButton::middle;
						playerInput->mouse
							.upAbsPos[playerInput->mouse.upSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_BUTTON_4_UP: {
						playerInput->mouse.up[playerInput->mouse.upSize] =
							Game::MouseButton::mouse4;
						playerInput->mouse
							.upAbsPos[playerInput->mouse.upSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					case RI_MOUSE_BUTTON_5_UP: {
						playerInput->mouse.up[playerInput->mouse.upSize] =
							Game::MouseButton::mouse5;
						playerInput->mouse
							.upAbsPos[playerInput->mouse.upSize++] =
							playerInput->mouse.cursorAbsEndPos;
					} break;
					}
				} else {
					LOG_ASSERT(false, "mouse key up missed, maxSize reached");
				}

				break;
			}

			// TODO: add support for controller
		} break;
		case WM_MOUSEWHEEL: {
			playerInput->mouse.wheelDelta = static_cast<int16_t>(msg.wParam >> 16);
		} break;
		case WM_KEYUP: {
			uint32_t scancode = msg.lParam & (255 << 16);
			scancode = scancode >> 16;
			Scancode key = processScancode(scancode, &pauseScancodeRead);
			if (key == Scancode::null) {
				break;
			}
			if (playerInput->keyboard.upSize < playerInput->keyboard.maxSize) {
				playerInput->keyboard.up[playerInput->keyboard.upSize] =
					static_cast<Scancode>(scancode);
				playerInput->keyboard.upSize++;
#ifdef INTERNAL
				if (key == Scancode::l) {
					lPressed = false;
				}
				if (key == Scancode::p) {
					pPressed = false;
				}
				if (key == Scancode::m) {
					mPressed = false;
				}
#endif
			} else {
				LOG_ASSERT(false, "key up missed, maxSize reached");
			}
		} break;
		case WM_KEYDOWN: {
			playerInput->isAnalog = false;
			bool previouslyPressed = msg.lParam & (1 << 30);
			if (!previouslyPressed) {
				uint32_t scancode = msg.lParam & (255 << 16);
				scancode = scancode >> 16;
				Scancode key = processScancode(scancode, &pauseScancodeRead);
				if (key != Scancode::null) {
					if (playerInput->keyboard.downSize <
						playerInput->keyboard.maxSize) {
						playerInput->keyboard
							.down[playerInput->keyboard.downSize++] = key;
					} else {
						LOG_ASSERT(false, "key down missed, maxSize reached");
					}
#ifdef INTERNAL
					if (key == Scancode::l) {
						lPressed = true;
					}
					if (key == Scancode::p) {
						pPressed = true;
					}
					if (key == Scancode::m) {
						mPressed = true;
					}
					if (key == Scancode::escape && gameHistory->recording) {
						endRecordingInput(gameHistory);
					}
					if (key == Scancode::escape && gameHistory->playing) {
						endInputPlayback(gameHistory);
						playerInput->resetState = true;
					}
					uint32_t maxKeyForHistoryRecording{
						static_cast<uint32_t>(Scancode::escape) +
						gameHistory->replayBufferCount + 1
					};
					if ((static_cast<uint32_t>(key) <
							 maxKeyForHistoryRecording &&
						 key > Scancode::escape) &&
						(lPressed || pPressed || mPressed)) {
						uint32_t slot{ (uint32_t)key -
									   (uint32_t)Scancode::one };
						if (slot < gameHistory->replayBufferCount) {
							if (lPressed && (!gameHistory->recording ||
											 slot != gameHistory->replaySlot)) {
								playerInput->resetState = gameHistory->playing;
								handleHistoryRecording(gameHistory, slot);
							} else {
								handleHistoryPlayback(gameHistory, slot);
							}
						}
						if (mPressed) {
							endRecordingInput(gameHistory);
							endInputPlayback(gameHistory);
							gameHistory->recording = false;
							gameHistory->playing = false;
							playerInput->resetState = true;
						}
					}
#endif
				}
			}
			TranslateMessage(&msg);
		}
		case WM_CHAR: {
			// even though wParam is 32 bit, it only sends the surrogates one at
			// a time

			uint32_t character = static_cast<uint32_t>(msg.wParam);

			uint16_t highSurrogateStart{ 0xD800 };
			uint16_t lowSurrogateStart{ 0xDC00 };
			uint16_t surrogateEnd{ 0xFFF };

			static uint16_t highSurrogate{ 0 };
			if (lowSurrogateStart <= character &&
				character <= highSurrogateStart) {
				highSurrogate = static_cast<uint16_t>(character);
			} else {
				if (highSurrogateStart <= character &&
					character <= surrogateEnd) {
					uint16_t lowSurrogate = static_cast<uint16_t>(character);
					character = (highSurrogate - highSurrogateStart) << 10;
					character |= (lowSurrogate - lowSurrogateStart);
					character += 0x10000;

					highSurrogate = 0;
				}
			}

			if (highSurrogate == 0) {
				if (playerInput->keyboard.typedSize <
					playerInput->keyboard.maxTypedSize) {
					playerInput->keyboard
						.typed[playerInput->keyboard.typedSize++] = character;
				} else {
					LOG_ASSERT(false, "key typed missed, maxSize reached");
				}
			}
		} break;
		default: {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} break;
		}
	}
}

#ifdef INTERNAL
void recordInput(History* gameHistory, const Game::Input input) {
	DWORD bytesWritten{};
	WriteFile(
		gameHistory->replayHandle, &input, sizeof(input), &bytesWritten, 0
	);
}
void playbackInput(History* gameHistory, Game::Input* input) {
	DWORD bytesRead{};
	ReadFile(gameHistory->replayHandle, input, sizeof(*input), &bytesRead, 0);

	if (bytesRead != sizeof(*input)) {
		uint32_t inputPlayingIndex{ gameHistory->replaySlot };
		endInputPlayback(gameHistory);
		beginInputPlayback(gameHistory, inputPlayingIndex);

		ReadFile(
			gameHistory->replayHandle, input, sizeof(*input), &bytesRead, 0
		);
	}
}
#endif

namespace {
// CHECK: do make codes and break codes produce the same scancode given by
// windows?
Scancode processScancode(uint32_t scancode, bool* pauseScancodeRead) {
	ASSERT(scancode <= 0xFF);
	if (*pauseScancodeRead) {
		if (scancode == 0x45) {
			scancode = 0xE11D45;
		}
		*pauseScancodeRead = false;
	} else if (scancode == 0xE11D) {
		*pauseScancodeRead = true;
	} else if (scancode == 0x54) {
		/* Alt + print screen return scancode 0x54 but we want it to return
		 * 0xE037 because 0x54 will not return a name for the key. */
		scancode = 0xE037;
	}

	/*
		Some scancodes we can ignore:
		- 0xE11D: first part of the Pause scancode (handled above);
		- 0xE02A: first part of the Print Screen scancode if no Shift, Control
	   or Alt keys are pressed;
		- 0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Insert,
	   Delete, Home, End, Page Up, Page Down, Up, Down, Left, Right when num
	   lock is on; or when num lock is off but one or both shift keys are
	   pressed;
		- 0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Numpad Divide
	   and one or both Shift keys are pressed;
		- Some of those a break scancode;

		When holding a key down, the pre/postfix (0xE02A) is not repeated.
	*/
	if (scancode != 0xE11D && scancode != 0xE02A && scancode != 0xE0AA &&
		scancode != 0xE0B6 && scancode != 0xE036) {
		return static_cast<Scancode>(scancode);
	}
	ASSERT(false);
	return Scancode::null;
}
#ifdef INTERNAL
inline ReplayBuffer* getReplayBuffer(History* history, uint32_t index) {
	ASSERT(index < history->gameMemorySize);
	return &history->replayBuffers[index];
}

// TODO: maybe enforce recording and playing invarient with a class / lambda
// magic? TOPDO: maybe rename to something more concrete like
// "setHandleToRecording"?
void beginRecordingInput(History* gameHistory, uint32_t replaySlot) {
	ASSERT(gameHistory->replayBuffers[replaySlot].memory);
	ASSERT(replaySlot < gameHistory->replayBufferCount);
	ASSERT(gameHistory->gameMemorySize < 0xFFFFFFFF);

	ReplayBuffer* replayBuffer = getReplayBuffer(gameHistory, replaySlot);
	gameHistory->replaySlot = replaySlot;
	gameHistory->recording = true;
	gameHistory->playing = false;

	char filePath[WIN32_FILENAME_MAX_COUNT];
	getInputFileLocation(
		gameHistory, true, replaySlot, filePath, sizeof(filePath)
	);
	gameHistory->replayHandle = CreateFileA(
		filePath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0
	);
	replayBuffer->isValid = true;

	CopyMemory(
		replayBuffer->memory, gameHistory->gameMemory,
		gameHistory->gameMemorySize
	);
}
void beginInputPlayback(History* gameHistory, uint32_t replaySlot) {
	ASSERT(gameHistory->replayBuffers[replaySlot].memory);
	ASSERT(replaySlot < gameHistory->replayBufferCount);

	ReplayBuffer* replayBuffer = getReplayBuffer(gameHistory, replaySlot);
	gameHistory->replaySlot = replaySlot;
	gameHistory->playing = true;
	gameHistory->recording = false;

	char filePath[WIN32_FILENAME_MAX_COUNT];
	getInputFileLocation(
		gameHistory, true, replaySlot, filePath, sizeof(filePath)
	);
	gameHistory->replayHandle = CreateFileA(
		filePath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0
	);

	CopyMemory(
		gameHistory->gameMemory, replayBuffer->memory,
		gameHistory->gameMemorySize
	);
}

void endRecordingInput(History* gameHistory) {
	if (gameHistory->recording) {
		ReplayBuffer* replayBuffer =
			getReplayBuffer(gameHistory, gameHistory->replaySlot);
		CloseHandle(gameHistory->replayHandle);
		gameHistory->recording = false;
	}
}
void endInputPlayback(History* gameHistory) {
	if (gameHistory->playing) {
		ReplayBuffer* replayBuffer =
			getReplayBuffer(gameHistory, gameHistory->replaySlot);
		CloseHandle(gameHistory->replayHandle);
		gameHistory->playing = false;
	}
}

// TODO: maybe do something more sensible?
/*
	the reason for this all in one ugly function is because i cant early return
   on the switch statent since it has to flow to WM_CHAR, could possibly split
   it into stop and start calls, and return success, and let the caller check
   idunno, im too tired
*/
void handleHistoryPlayback(History* history, uint32_t slot) {
	if (history->playing) {
		endInputPlayback(history);
	}
	if (slot >= history->replayBufferCount) {
		return;
	}

	if (!history->replayBuffers[slot].isValid) {
		return;
	}
	if (history->recording && (history->replaySlot != slot)) {
		return;
	}

	endRecordingInput(history);
	beginInputPlayback(history, slot);
}
void handleHistoryRecording(History* history, uint32_t slot) {
	if (history->recording) {
		endRecordingInput(history);
	}
	if (slot >= history->replayBufferCount) {
		return;
	}

	endInputPlayback(history);
	beginRecordingInput(history, slot);
}
#endif
}  // namespace
