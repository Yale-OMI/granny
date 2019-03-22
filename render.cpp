/*

TEST FM-SYNTHESIS
sam l - 02/12

*/

#include <Bela.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "ADSR.h"

#define MAX_GRAINS 10

int gCount;
int gTrig;
int gOffset;

float gPhase;
float gModPhase;
float gInverseSampleRate;

float sig[44800*2];

float gGrainsPS;
float gGLength;
float gGRate;
int gGPos;
int gIndex;

float gScale = 0.2;

int lastActiveIndex;

ADSR envs[MAX_GRAINS];
int pos[MAX_GRAINS];
bool active[MAX_GRAINS];

int gAudioFramesPerAnalogFrame = 0;

bool setup(BelaContext *context, void *userData)
{
	// Setup random stuff
	srand(time(NULL));

	if(context->analogFrames)
		gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

	gInverseSampleRate = 1.0 / context->audioSampleRate;
	gPhase = 0.0;
	gModPhase = 0.0;

	/*
	 * setup granular variables
	 *
	 * TODO
	 * setup all analog knobs and whatnot
	 */
	gGrainsPS = 1.;
	gGLength = 0.1;
	gGRate = 1.;
	gGPos = 0;
	lastActiveIndex = 0;
	gIndex = 0;

	for(int i=0; i<MAX_GRAINS; i++) pos[i] = gGPos; active[0] = false;

	/*
	 * We want to populate a bank with a certain amount
	 * of frequency modulated signal
	 * - for now make 2 seconds
	 */

	/*
	 * TODO
	 * implement sample buffer class that handles access,
	 * loading, allocation
	 */

	float p_mod = 0.0; float p_car = 0.0;
	float a_mod = 1000.; float a_car = 0.3;
	float f_out = 0.0; float f_car = 440.; float f_mod = 440.;
	for (int i=0; i<context->audioSampleRate*2; i++) {
		f_out = f_car - (a_mod * sinf(p_mod));
		sig[i] = a_car * sinf(p_car);

		p_car += 2.0 * M_PI * f_out * gInverseSampleRate;
		if (p_car > 2.0 * M_PI) p_car -= 2.0 * M_PI;	
		p_mod += 2.0 * M_PI * f_mod * gInverseSampleRate;
		if (p_mod > 2.0 * M_PI) p_mod -= 2.0 * M_PI;
	}

	return true;
}

void render(BelaContext *context, void *userData)
{

	for(unsigned int n = 0; n < context->audioFrames; n++) {
		gGrainsPS = map(analogRead(context, n/gAudioFramesPerAnalogFrame, 0), 0, 1, 5., 100.);
		gGLength = map(analogRead(context, n/gAudioFramesPerAnalogFrame, 1), 0, 1, 0.2, 0.1); 
		gGPos = int(map(analogRead(context, n/gAudioFramesPerAnalogFrame, 2), 0, 1, 0, 44100));

		if (gCount >= context->audioSampleRate / gGrainsPS) {
			envs[gIndex].reset();
			envs[gIndex].gate(1);
			envs[gIndex].setAttackRate((gGLength / 2) * context->analogSampleRate);
			envs[gIndex].setDecayRate((gGLength / 2) * context->analogSampleRate);
			envs[gIndex].setSustainLevel(0.0);
			
			pos[gIndex] = gGPos;
			active[gIndex] = true;

			gIndex = (gIndex + 1) % MAX_GRAINS;
			gCount = 0;
		}

		float out = 0.;

		for(unsigned int i = 0; i < MAX_GRAINS; i++) {
			if (!active[i]) continue;
			out += envs[i].getOutput() * sig[pos[i]] * gScale;
			pos[i] = (pos[i] + 1) % ((int) context->audioSampleRate * 2);
			envs[i].process(1);	
		}
		
		for(unsigned int channel = 0; channel < context->audioOutChannels; channel++) {
			// Two equivalent ways to write this code
			// The long way, using the buffers directly:
			// context->audioOut[n * context->audioOutChannels + channel] = out;
			// Or using the macros:
			audioWrite(context, n, channel, out);
		}
		
		gCount += 1;
	}
}

void cleanup(BelaContext *context, void *userData)
{
}

