#include <stdio.h>
#include <stdlib.h>

#include "audio.h"

#define NUM_SECONDS (2)
#define SAMPLE_RATE (44100/2)

int main(int argc, char *argv[])
{
    //struct sound_params *sp = sg_create_sound_params(SQUARE, sg_create_square_wave_parameters(440, 0.5, SAMPLE_RATE));
    struct sound_params *sp2 = sg_create_sound_params(SINE, sg_create_sine_parameters(440, SAMPLE_RATE));
    //struct sound_params *sp3 = sg_create_sound_params(SQUARE, sg_create_square_wave_parameters(840, 0.5, SAMPLE_RATE));
    
    audio_init();

    audio_create_stream(SAMPLE_RATE);

    //sound_play_seconds(sp, 0.5);
    sound_play_seconds(sp2, 1.5);
    //sound_play_seconds(sp3, 1.5);

    audio_sleep(1.5*1000);

    //sp2 = sg_create_sound_params(SQUARE, sg_create_square_wave_parameters(440, 0.5, SAMPLE_RATE));
    //sound_play_seconds(sp2, 1);
    
    //audio_sleep(1*1000);

    //sw1.setFrequency(300);

    audio_shutdown();

    return 0;
}
