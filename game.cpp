#include "game.h"
#include "timer.h"
#include "audio.h"
#include "graphics.h"

AudioEngine* Audio = new AudioEngine;
GraphicsEngine* Graphics = new GraphicsEngine;
float parameter = 0.0f;

int Game::Init(void* parms, int num_parms, HWND hWnd) {
	ResetTime();
	Graphics->InitDevice(hWnd);
	Audio->InitAudio();
	//Audio->GenerateSineWave(0.0f, 0.4f, .4f, 200.0f, -600, 0.0f, 0.5f);
	//Audio->GenerateWaveform(WHITENOISE, 0.0f, 1, 1.f, 225 * 32, 0, 0.0, 0.0f, 0.0f, 1344 / 1000, .4f);
	/*This one is cool*/ Audio->GenerateWaveform(WHITENOISE, 0.0f, 0.6, 1, 100 * 32, -50 * 32, 0.0f, 8, 0, 3, 0.0f, 0.0f, 0.4f);
	//Audio->GenerateWaveform(SQUARE, 0.0f, 0.0, 0.2, 440, 0, 0.0f, 0, 0, 0, 0.0f, 0.0f, 0.4f);
	Audio->PlayGeneratedSound();
	//Audio->ExportWaveform("start.wav");
	return 0;
};

int Game::Logic(void* parms, int num_parms) {
	Graphics->Render(parms, num_parms, parameter);
	parameter = Audio->GetNumber();
	return 0;
};

int Game::Shutdown(void* parms, int num_parms) {
	Audio->~AudioEngine();
	Graphics->~GraphicsEngine();
	return 0;
};