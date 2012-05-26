#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <portaudio.h>

#include "generators.h"

void sound_play(struct sound_params *sp,
                int samples);
void sound_play_seconds(struct sound_params *sp,
                        float seconds);

int audio_pa_callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData);

int audio_init();
int audio_create_stream(int sample_rate);
int audio_shutdown();

#endif  /* __AUDIO_H__ */
