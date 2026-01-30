#pragma once

#ifndef AUDIO_H
#define AUDIO_H

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

#include "window.h"
#include <xaudio2.h>
#include <iostream>
#include <vector>

enum WaveformType {
	SINE,
	SQUARE,
	TRIANGLE,
	SAWTOOTH,
	WHITENOISE,
	BITNOISE
};

class AudioEngine {
protected:
	IXAudio2* XAudio;
	IXAudio2MasteringVoice* XAudioMaster;
	IXAudio2SourceVoice* XAudioSource;
	WAVEFORMATEXTENSIBLE wfx;
	XAUDIO2_BUFFER buffer;	
	//
	int sampleRate;
	int amplitude;
	//
	int attackSamples;
	int sustainSamples;
	int releaseSamples;
	int totalSamples;
	//
	float currentFrequency;
	float frequencySlide;
	float deltaSlide;
	//
	float attackTime;
	float sustainTime;
	float releaseTime;
	//
	float vibrato;
	//
	float number;
	//
	float phase;
	float envelope;
	short* waveData;
public:
	AudioEngine();
	~AudioEngine();
	HRESULT InitAudio();
	void GenerateWaveform(WaveformType type, float attackTime, 
		float sustainTime, float releaseTime, 
		float frequency, float frequencySlide, 
		float deltaSlide, float repeatSpeed, 
		float flangerOffset, float flangerSweep, 
		float vibratoDepth, float vibratoSpeed,
		float volume);
	bool ExportWaveform(const char* filename);
	float GetNumber();
	void PlayGeneratedSound();
};


#endif // AUDIO_H