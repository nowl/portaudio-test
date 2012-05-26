#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "mempool.h"
#include "generators.h"

struct sound_params
{
    enum sound_generator_type type;
    void *params;
};

static struct mempool *sound_params_mem = NULL;

struct sound_params *sg_create_sound_params(int type, void *params)
{
    if( !sound_params_mem )
        sound_params_mem = mp_create(sizeof(struct sound_params), 2);

    struct sound_params * sc = mp_alloc(sound_params_mem);
    sg_set_params(sc, type, params);
    return sc;
}

void sg_cleanup_sound_params(struct sound_params *sc)
{
    switch(sc->type)
    {
    case SQUARE:
        sg_free_square_wave_parameters(sc->params);
        break;
    default:
        break;
    }
    
    mp_free(sound_params_mem, sc);
}

/* TODO free sound_params_mem */

void sg_set_params(struct sound_params *sc,
                   int type,
                   void *params)
{
    sc->type = type;
    sc->params = params;
}

/* square wave */

struct square_wave_parameters
{
    float frequency;
    float sample_rate;
    int ticks_per_rev;
    float current_value;
    float duty_cycle;
    int counter;
};

static struct mempool *square_wave_parameter_mem = NULL;

struct square_wave_parameters *sg_create_square_wave_parameters(float frequency,
                                                                float duty_cycle,
                                                                float sample_rate)
{
    if( !square_wave_parameter_mem )
        square_wave_parameter_mem = mp_create(sizeof(struct square_wave_parameters), 16);

    struct square_wave_parameters *p = mp_alloc(square_wave_parameter_mem);
    p->frequency = frequency;
    p->ticks_per_rev = sample_rate/frequency;
    p->counter = 0;
    p->sample_rate = sample_rate;
    p->current_value = 0;
    p->duty_cycle = duty_cycle;
    return p;
}

void sg_free_square_wave_parameters(struct square_wave_parameters *params)
{
    mp_free(square_wave_parameter_mem, params);
}

/* TODO free square_wave_parameter_mem */

/* sawtooth wave */

static struct mempool *sawtooth_parameter_mp = NULL;

struct sawtooth_parameters
{
    float frequency;
    float sample_rate;
    int ticks_per_rev;
    float current_value;
    int counter;
    float slope;
};

struct sawtooth_parameters *sg_create_sawtooth_parameters(float frequency,
                                                          float sample_rate)
{
    if( !sawtooth_parameter_mp )
        sawtooth_parameter_mp = mp_create(sizeof(struct sawtooth_parameters), 16);

    struct sawtooth_parameters *p = mp_alloc(sawtooth_parameter_mp);
    p->frequency = frequency;
    p->ticks_per_rev = sample_rate/frequency;
    p->counter = 0;
    p->sample_rate = sample_rate;
    p->current_value = -1;
    p->slope = 2.0 / p->ticks_per_rev;
    return p;
}

void sg_free_sawtooth_parameters(struct sawtooth_parameters *params)
{
    mp_free(sawtooth_parameter_mp, params);
}

/* triangle wave */

static struct mempool *triangle_parameter_mp = NULL;

struct triangle_parameters
{
    float frequency;
    float sample_rate;
    int ticks_per_rev;
    float current_value;
    int counter;
    float slope;
};

struct triangle_parameters *sg_create_triangle_parameters(float frequency,
                                                          float sample_rate)
{
    if( !triangle_parameter_mp )
        triangle_parameter_mp = mp_create(sizeof(struct triangle_parameters), 16);

    struct triangle_parameters *p = mp_alloc(triangle_parameter_mp);
    p->frequency = frequency;
    p->ticks_per_rev = sample_rate/frequency;
    p->counter = 0;
    p->sample_rate = sample_rate;
    p->current_value = 0;
    p->slope = 1 / (p->ticks_per_rev/4.0);
    return p;
}

void sg_free_triangle_parameters(struct triangle_parameters *params)
{
    mp_free(triangle_parameter_mp, params);
}

/* sine wave */

static struct mempool *sine_parameter_mp = NULL;

#define PI 3.1415926535

struct sine_parameters
{
    float frequency;
    float sample_rate;
    int ticks_per_rev;
    float current_value;
    float rev;
    int counter;
};

struct sine_parameters *sg_create_sine_parameters(float frequency,
                                                  float sample_rate)
{
    if( !sine_parameter_mp )
        sine_parameter_mp = mp_create(sizeof(struct sine_parameters), 16);

    struct sine_parameters *p = mp_alloc(sine_parameter_mp);
    p->frequency = frequency;
    p->ticks_per_rev = sample_rate/frequency;
    p->rev = (sample_rate/frequency) / (2*PI);
    p->counter = 0;
    p->sample_rate = sample_rate;
    p->current_value = 0;
    return p;
}

void sg_free_sine_parameters(struct sine_parameters *params)
{
    mp_free(sine_parameter_mp, params);
}

/* advance */

static float advance_square(struct square_wave_parameters *p)
{
    if(p->counter < p->ticks_per_rev * p->duty_cycle)
        p->current_value = 1.0;
    else
        p->current_value = -1.0;
        
    if(p->counter++ >= p->ticks_per_rev)
        p->counter = 0;
    
    return p->current_value;
}

static float advance_sawtooth(struct sawtooth_parameters *p)
{
    if(p->counter < p->ticks_per_rev)
        p->current_value += p->slope;
    
    if(p->counter++ >= p->ticks_per_rev)
    {
        p->counter = 0;
        p->current_value = -1;
    }
    
    return p->current_value;
}

static float advance_triangle(struct triangle_parameters *p)
{
    if(p->counter < p->ticks_per_rev/4.0 || p->counter >= 3*p->ticks_per_rev/4.0)
        p->current_value += p->slope;
    else
        p->current_value -= p->slope;
    
    if(p->counter++ >= p->ticks_per_rev)
    {
        p->counter = 0;
        p->current_value = 0;
    }

    return p->current_value;
}

static float advance_sine(struct sine_parameters *p)
{
    p->current_value = sin(p->counter / p->rev);
    
    if(p->counter++ >= p->ticks_per_rev)
        p->counter = 0;

    return p->current_value;
}

float sg_advance_generator(struct sound_params *sc)
{
    enum sound_generator_type type = sc->type;
    void *params = sc->params;

    switch(type)
    {
    case SQUARE:
        return advance_square(params);
    case SAWTOOTH:
        return advance_sawtooth(params);
    case TRIANGLE:
        return advance_triangle(params);
    case SINE:
        return advance_sine(params);
    default:
        printf("bad value for sound advance\n");
        return 0;
    }
}
