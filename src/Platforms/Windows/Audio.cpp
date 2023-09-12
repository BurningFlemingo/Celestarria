#pragma once

#include "Audio.h"
#include "../../Celestarria/Defines.h"
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <AudioSessionTypes.h>
#include <mmdeviceapi.h>
#include <devicetopology.h>
#include <endpointvolume.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "winmm.lib")

namespace {
    IMMDeviceEnumerator* pEnumerator{};
    IMMDevice* pDevice{};
    IAudioClient* pAudioClient{};
    IAudioClock* audioClock;
    IAudioRenderClient* pRenderClient{};
    WAVEFORMATEX* pMixFormat{};
    DWORD audioFlags;
    UINT32 bufferFrameCount{};
    REFERENCE_TIME bufferDuration{};
    float bufferDurationInSeconds{};
    bool playing{ false };

    inline void safeRelease();
}

// TODO: change asserts into log asserts
// TODO: bulletproof
AudioOutputInfo initWASAPI(float msBufferSize) {
    HRESULT hr{};
    hr = CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY);
    if (FAILED(hr)) {
        return {};
    }

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), 0,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pEnumerator)
    );
    if (FAILED(hr)) {
        return {};
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return {};
    }

    hr = pDevice->Activate(
        __uuidof(IAudioClient), CLSCTX_INPROC_SERVER,
        0, reinterpret_cast<void**>(&pAudioClient)
    );
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return {};
    }

    pAudioClient->GetMixFormat(&pMixFormat);
    ASSERT(pMixFormat->nChannels == 2);

    bufferDuration = static_cast<REFERENCE_TIME>(static_cast<double>(msBufferSize) * 10000.f);
    audioFlags =
        AUDCLNT_STREAMFLAGS_NOPERSIST |
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM;

    hr = pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        audioFlags,
        bufferDuration,
        0,
        pMixFormat,
        0
    );

    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return {};
    }

    bufferDurationInSeconds = static_cast<float>(bufferFrameCount) / static_cast<float>(pMixFormat->nSamplesPerSec);

    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();

        return {};
    }

    hr = pAudioClient->GetService(IID_PPV_ARGS(&pRenderClient));
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();

        return {};
    }

    hr = pAudioClient->GetService(__uuidof(IAudioClock), reinterpret_cast<void**>(&audioClock));
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return {};
    }

    AudioOutputInfo audioOutputInfo{};
    audioOutputInfo.bytesPerSample = pMixFormat->nBlockAlign;
    audioOutputInfo.nChannels = static_cast<uint8_t>(pMixFormat->nChannels);
    audioOutputInfo.samplesPerSecond = pMixFormat->nSamplesPerSec;

    return audioOutputInfo;
}

// buffer will be copied
bool writeAudioBuffer(void* samples, uint32_t nSamplesToPlay) {
    HRESULT hr{};

    AudioBufferState bufferState = getBufferState();
    uint32_t nSamplesPlayed = min(nSamplesToPlay, bufferState.samplesFree);
    //ASSERT(nSamplesToPlay == nSamplesPlayed);
    if (bufferState.samplesFree == 0 || nSamplesPlayed != nSamplesToPlay || !samples) {
        return true;
    }

    BYTE* buffer{};
    hr = pRenderClient->GetBuffer(nSamplesPlayed, &buffer);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        return true;
    }

    memcpy(buffer, samples, static_cast<size_t>(pMixFormat->nBlockAlign) * nSamplesPlayed);

    hr = pRenderClient->ReleaseBuffer(nSamplesPlayed, 0);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        return true;
    }

    AudioBufferState bufferState2 = getBufferState();

    return false;
}

AudioBufferState getBufferState() {
    HRESULT hr{};
    AudioBufferState bufferState{};

    if (!pAudioClient) {
        return {};
    }

    hr = pAudioClient->GetCurrentPadding(&bufferState.samplesQueued);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        return {};
    }

    bufferState.samplesFree = bufferFrameCount - bufferState.samplesQueued;

    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        return {};
    }

    bufferState.bufferSize = bufferFrameCount;
    bufferState.bufferTime = bufferDurationInSeconds;

    uint64_t audioPlaybackFreq{};
    uint64_t audioPlaybackPos{};

    hr = audioClock->GetFrequency(&audioPlaybackFreq);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        return {};
    }

    hr = audioClock->GetPosition(&audioPlaybackPos, 0);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        return {};
    }

    bufferState.totalPlaybackTime = static_cast<float>(audioPlaybackPos) / static_cast<float>(audioPlaybackFreq);
    bufferState.totalPlaybackPosition = static_cast<uint32_t>(bufferState.totalPlaybackTime * static_cast<float>(pMixFormat->nSamplesPerSec));

    bufferState.playbackPosition = bufferState.totalPlaybackPosition % bufferState.bufferSize;

    return bufferState;
}

void startAudio() {
    static bool audioStarted{ false };

    if (!audioStarted && pAudioClient) {
        HRESULT hr{};
        hr = pAudioClient->Start();

        ASSERT(SUCCEEDED(hr));
        audioStarted = SUCCEEDED(hr);
    }
}
void stopAudio() {
    static bool audioStarted{ false };

    if (!audioStarted && pAudioClient) {
        HRESULT hr{};
        hr = pAudioClient->Stop();

        ASSERT(SUCCEEDED(hr));
        audioStarted = SUCCEEDED(hr);
    }
}

void releaseAudio() {
    safeRelease();
    stopAudio();
}

bool writeSilence(uint32_t nSamples) {
    HRESULT hr{};
    BYTE* data{};

    uint32_t padding{};
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);
    hr = pAudioClient->GetCurrentPadding(&padding);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return false;
    }

    if (nSamples > (bufferFrameCount - padding)) {
        return false;
    }
    
    hr = pRenderClient->GetBuffer(nSamples, &data);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return false;
    }

    hr = pRenderClient->ReleaseBuffer(nSamples, AUDCLNT_BUFFERFLAGS_SILENT);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
        safeRelease();
        return false;
    }

    return true;
}

namespace {
    void safeRelease() {
        if (pDevice) {
            pDevice->Release();
            pDevice = 0;
        }
        if (pAudioClient) {
            pAudioClient->Release();
            pAudioClient = 0;
        }
        if (audioClock) {
            audioClock->Release();
            audioClock = 0;
        }
        if (pRenderClient) {
            pRenderClient->Release();
            pRenderClient = 0;
        }
    }
}

