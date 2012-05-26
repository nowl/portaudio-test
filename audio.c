#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "mempool.h"
#include "audio.h"

struct sound_container
{
    struct sound_container *next, *prev;

    struct sound_params *params;

    int counter;
    int duration;
};

static struct mempool *sound_container_mp = NULL;
static pthread_mutex_t container_lock = PTHREAD_MUTEX_INITIALIZER;

static struct sound_container *sound_container = NULL;
static unsigned int sample_rate = 44100;

void sound_play(struct sound_params *sp,
                int duration)
{
    if( !sound_container_mp )
    {
        sound_container_mp = mp_create(sizeof(struct sound_container), 2);
        pthread_mutex_init(&container_lock, NULL);
    }

    struct sound_container *s = mp_alloc(sound_container_mp);
    s->params = sp;
    
    pthread_mutex_lock(&container_lock);

    s->prev = NULL;
    s->next = sound_container;
    
    if(s->next)
        s->next->prev = s;

    pthread_mutex_unlock(&container_lock);

    s->counter = 0;
    s->duration = duration;
    
    sound_container = s;
}

void sound_play_seconds(struct sound_params *sp,
                        float seconds)
{
    sound_play(sp, sample_rate * seconds);
}

static void sound_container_remove_dead()
{
    pthread_mutex_lock(&container_lock);
    
    struct sound_container *n = sound_container;
    struct sound_container *ret = sound_container;

    while(n)
    {
        struct sound_container *next = n->next;

        if(n->counter > n->duration)
        {
            if(ret == n)
                ret = next;

            if(n->prev)
                n->prev->next = next;
            if(n->next)
                n->next->prev = n->prev;

            sg_cleanup_sound_params(n->params);
            mp_free(sound_container_mp, n);
        }

        n = next;
    }

    pthread_mutex_unlock(&container_lock);

    sound_container = ret;
}

int audio_pa_callback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData)
{
    float *out = outputBuffer;
    unsigned int i;
    (void)inputBuffer;

    sound_container_remove_dead();

    for(i=0; i<framesPerBuffer; i++)
    {
        double total = 0;
        int numChannels = 0;
        struct sound_container *n = sound_container;
        while(n)
        {
            total += sg_advance_generator(n->params);
            numChannels++;
            n->counter++;

            n = n->next;
        }

        *out++ = total/numChannels;
        //*out++ = total/numChannels;
    }

    return 0;
}

static PaStream *stream = NULL;

static int pa_error(PaError err)
{
    Pa_Terminate();
    printf("Error occured while using portaudio\n");
    printf("Error number: %d\n", err);
    printf("Error message: %s\n", Pa_GetErrorText(err));

    return err;
}

int audio_init()
{
    PaError err = Pa_Initialize();
    if(err != paNoError) return pa_error(err);

    printf("audio started\n");

    return 0;
}

int audio_create_stream(int sr)
{
    sample_rate = sr;

    PaError err = Pa_OpenDefaultStream(&stream,
                                       0,
                                       1,
                                       paFloat32,
                                       sample_rate,
                                       256,
                                       audio_pa_callback,
                                       NULL);
    if(err != paNoError) return pa_error(err);

    err = Pa_StartStream(stream);
    if(err != paNoError) return pa_error(err);

    return 0;
}

int audio_shutdown()
{
    PaError err = Pa_StopStream(stream);
    if(err != paNoError) return pa_error(err);
    
    err = Pa_CloseStream(stream);
    if(err != paNoError) return pa_error(err);
    
    Pa_Terminate();
    
    printf("audio shutdown\n");

    return 0;
}
