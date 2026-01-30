#include "audio.h"
#include "graphics.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "winmm.lib")

#pragma warning(disable : 4996)

AudioEngine::AudioEngine() {
	XAudio = 0;
	XAudioMaster = 0;
	XAudioSource = 0;
	ZeroMemory(&wfx, sizeof(WAVEFORMATEXTENSIBLE));
	ZeroMemory(&buffer, sizeof(XAUDIO2_BUFFER));
	number = 0.0f;
	sampleRate = 44100;
	amplitude = 32767;
	attackSamples = 0;
	sustainSamples = 0;
	releaseSamples = 0;
	totalSamples = 0;
	currentFrequency = 0.0f;
	frequencySlide = 0.0f;
	deltaSlide = 0.0f;
	attackTime = 0.0f;
	sustainTime = 0.1f;
	releaseTime = 0.1f;
    vibrato = 0.0f;
	phase = 0.0f;
	envelope = 1.0f;
	waveData = nullptr;
};

AudioEngine::~AudioEngine() {
	if (XAudioSource) {
		XAudioSource->Stop(0);
		XAudioSource->FlushSourceBuffers();
		XAudioSource->DestroyVoice();
	}
	if (XAudioMaster) {
		XAudioMaster->DestroyVoice();
	}
	if (XAudio) {
		XAudio->Release();
	}
};

HRESULT AudioEngine::InitAudio() {
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to initialize COM!", "Time to debug", MB_OK);
		return 1;
	}
	hr = XAudio2Create(&XAudio, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to initialize XAudio2!", "Time to debug", MB_OK);
		return 1;
	}
	hr = XAudio->CreateMasteringVoice(&XAudioMaster);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create master voice!", "Time to debug", MB_OK);
		return 1;
	}
}

void AudioEngine::GenerateWaveform(
    WaveformType type,
    float attackTime,
    float sustainTime,
    float releaseTime,
    float frequency,
    float frequencySlide,
    float deltaSlide,
    float repeatSpeed,
    float flangerOffset,
    float flangerSweep,
    float vibratoDepth,
    float vibratoSpeed,
    float volume
) {
    attackSamples = (int)(attackTime * sampleRate);
    sustainSamples = (int)(sustainTime * sampleRate);
    releaseSamples = (int)(releaseTime * sampleRate);

    totalSamples = attackSamples + sustainSamples + releaseSamples;

    if (waveData) {
        delete[] waveData;
    }
    waveData = new short[totalSamples];

    std::vector<float> floatBuffer(totalSamples, 0.0f);

    float localPhase = 0.0f;
    float localEnvelope = 1.0f;
    float localFrequency = frequency;

    for (int i = 0; i < totalSamples; i++) {
        if (i < attackSamples) {
            localEnvelope = (float)i / (attackSamples > 0 ? attackSamples : 1);
        }
        else if (i >= attackSamples + sustainSamples) {
            int relPos = i - attackSamples - sustainSamples;
            localEnvelope = 1.0f - (float)relPos / (releaseSamples > 0 ? releaseSamples : 1);
            if (localEnvelope < 0.0f) localEnvelope = 0.0f;
        }
        else {
            localEnvelope = 1.0f;
        }

        float t;
        if (repeatSpeed > 0.0f) {
            int repeatSamples = (int)(sampleRate / repeatSpeed);
            int posInRepeat = i % repeatSamples;
            t = (float)posInRepeat / repeatSamples;
        } else {
            t = (float)i / totalSamples;
        }

        vibrato = 0.0f;
        if (vibratoDepth > 0.0f && vibratoSpeed > 0.0f) {
            vibrato = vibratoDepth * sinf(2.0f * 3.14159265f * vibratoSpeed * ((float)i / sampleRate));
        }

        localFrequency = frequency + (frequencySlide * t) + (deltaSlide * t * t) + vibrato;
        localFrequency = max(localFrequency, 1.0f);

        localPhase += (2.0f * 3.14159265f * localFrequency) / sampleRate;

        float sample = 0.0f;
        switch (type)
        {
        case SINE:
            sample = sinf(localPhase);
            break;
        case SQUARE:
            sample = (sinf(localPhase) >= 0 ? 1.0f : -1.0f);
            break;
        case TRIANGLE:
            sample = (2.0f * fabs(2.0f * (localPhase / (2.0f * 3.14159265f) - floor(localPhase / (2.0f * 3.14159265f) + 0.5f))) - 1.0f);
            break;
        case SAWTOOTH:
            sample = (2.0f * (localPhase / (2.0f * 3.14159265f) - floor(localPhase / (2.0f * 3.14159265f) + 0.5f)));
            break;
        case WHITENOISE:
            {
                static float lastWhite = 0.0f;
                static int lastSampleIndex = -1;
                int freqStep = (int)(sampleRate / localFrequency);
                if (freqStep < 1) freqStep = 1;
                if (i % freqStep == 0 || lastSampleIndex == -1) {
                    lastWhite = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
                    lastSampleIndex = i;
                }
                sample = lastWhite;
            }
            break;
        case BITNOISE:
            {
                static int lastSampleIndex = -1;
                static float lastBit = 1.0f;
                int freqStep = (int)(sampleRate / localFrequency);
                if (freqStep < 1) freqStep = 1;
                if (i % freqStep == 0 || lastSampleIndex == -1) {
                    int bit = rand() & 1;
                    lastBit = bit ? 1.0f : -1.0f;
                    lastSampleIndex = i;
                }
                sample = lastBit * localEnvelope * volume;
            }
            break;
        default:
            sample = 0.0f;
            break;
        }

        floatBuffer[i] = amplitude * localEnvelope * sample * volume;
    }

    int maxDelaySamples = (int)((fabs(flangerOffset) + fabs(flangerSweep)) * sampleRate / 1000.0f) + 2;

    for (int i = 0; i < totalSamples; i++) {
        float t;
        if (repeatSpeed > 0.0f) {
            int repeatSamples = (int)(sampleRate / repeatSpeed);
            int posInRepeat = i % repeatSamples;
            t = (float)posInRepeat / repeatSamples;
        } else {
            t = (float)i / totalSamples;
        }

        float elapsedSeconds = (float)i / sampleRate;
        float delayMs = flangerOffset + flangerSweep * elapsedSeconds;
        if (delayMs < -20.0f) delayMs = -20.0f;
        if (delayMs > 20.0f + fabs(flangerSweep)) delayMs = 20.0f + fabs(flangerSweep);

        int delaySamples = (int)(delayMs * sampleRate / 1000.0f);

        int delayedIndex = i - delaySamples;
        float delayedSample = 0.0f;
        if (delayedIndex >= 0 && delayedIndex < totalSamples) {
            delayedSample = floatBuffer[delayedIndex];
        }

        float mixed = floatBuffer[i] * 0.7f + delayedSample * 0.3f;

        if (mixed > 32767.0f) mixed = 32767.0f;
        if (mixed < -32768.0f) mixed = -32768.0f;

        waveData[i] = (short)mixed;
    }

    wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
    wfx.Format.nChannels = 1;
    wfx.Format.nSamplesPerSec = sampleRate;
    wfx.Format.wBitsPerSample = 16;
    wfx.Format.nBlockAlign = (wfx.Format.nChannels * wfx.Format.wBitsPerSample) / 8;
    wfx.Format.nAvgBytesPerSec = wfx.Format.nSamplesPerSec * wfx.Format.nBlockAlign;
    wfx.Format.cbSize = 0;
    buffer.AudioBytes = totalSamples * sizeof(short);
    buffer.pAudioData = (BYTE*)waveData;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    if (XAudioSource) {
        XAudioSource->Stop(0);
        XAudioSource->FlushSourceBuffers();
        XAudioSource->DestroyVoice();
    }
    HRESULT hr = XAudio->CreateSourceVoice(&XAudioSource, (WAVEFORMATEX*)&wfx);
    if (FAILED(hr)) {
        MessageBoxA(NULL, "Failed to create source voice!", "Time to debug", MB_OK);
        return;
    }
    hr = XAudioSource->SubmitSourceBuffer(&buffer);
    if (FAILED(hr)) {
        MessageBoxA(NULL, "Failed to submit source buffer!", "Time to debug", MB_OK);
        return;
    }
}

bool AudioEngine::ExportWaveform(const char* filename) {
	if (!XAudioSource) return false;
	FILE* file = fopen( filename, "wb");
	if (!file) return false;
	int totalSamples = buffer.AudioBytes / sizeof(short);
	int dataSize = totalSamples * sizeof(short);
	int fileSize = 36 + dataSize;
	fwrite("RIFF", 1, 4, file);
	fwrite(&fileSize, 4, 1, file);
	fwrite("WAVE", 1, 4, file);
	int fmtChunkSize = 16;
	short audioFormat = 1; 
	short numChannels = wfx.Format.nChannels;
	int sampleRate = wfx.Format.nSamplesPerSec;
	short bitsPerSample = wfx.Format.wBitsPerSample;
	short blockAlign = (numChannels * bitsPerSample) / 8;
	int byteRate = sampleRate * blockAlign;
	fwrite("fmt ", 1, 4, file);
	fwrite(&fmtChunkSize, 4, 1, file);
	fwrite(&audioFormat, 2, 1, file);
	fwrite(&numChannels, 2, 1, file);
	fwrite(&sampleRate, 4, 1, file);
	fwrite(&byteRate, 4, 1, file);
	fwrite(&blockAlign, 2, 1, file);
	fwrite(&bitsPerSample, 2, 1, file);
	fwrite("data", 1, 4, file);
	fwrite(&dataSize, 4, 1, file);
	fwrite(buffer.pAudioData, sizeof(short), totalSamples, file);
	fclose(file);
	return true;
}

float AudioEngine::GetNumber() {
	return XAudioSource ? (float)buffer.AudioBytes / (float)(wfx.Format.nAvgBytesPerSec) : 0.0f;
}

void AudioEngine::PlayGeneratedSound() {
	XAudioSource->Start(0);
}