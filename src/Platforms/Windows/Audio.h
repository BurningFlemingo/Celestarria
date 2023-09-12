#pragma once

#include <stdint.h>

struct AudioOutputInfo {
    uint32_t samplesPerSecond{};
    uint32_t bytesPerSample{};  // including nChannels
    uint8_t nChannels{};
};

struct AudioBufferState {
    uint32_t samplesQueued{};
    uint32_t samplesFree{};
    // in samples
    uint32_t bufferSize{};
    // in seconds
    float bufferTime{};
    // in samples
    uint32_t playbackPosition{};
    // in samples
    uint32_t totalPlaybackPosition{};
    // in ms
    float totalPlaybackTime{};
};

AudioOutputInfo initWASAPI(float msBufferSize);
AudioBufferState getBufferState();
bool writeAudioBuffer(void* samples, uint32_t nSamplesToPlay);
void startAudio();
void stopAudio();
bool writeSilence(uint32_t nSamples);
