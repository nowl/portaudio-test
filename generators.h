#ifndef __GENERATORS_H__
#define __GENERATORS_H__

struct sound_params;
struct square_wave_parameters;
struct sawtooth_parameters;
struct triangle_parameters;
struct sine_parameters;

enum sound_generator_type
{
    SQUARE,
    SAWTOOTH,
    TRIANGLE,
    SINE
};

struct sound_params *sg_create_sound_params(int type, void *params);
void sg_cleanup_sound_params(struct sound_params *sc);
void sg_set_params(struct sound_params *sc,
                   int type,
                   void *params);

struct square_wave_parameters *sg_create_square_wave_parameters(float frequency,
                                                                float duty_cycle,
                                                                float sample_rate);
void sg_free_square_wave_parameters(struct square_wave_parameters *params);

struct sawtooth_parameters *sg_create_sawtooth_parameters(float frequency, float sample_rate);
void sg_free_sawtooth_parameters(struct sawtooth_parameters *params);

struct triangle_parameters *sg_create_triangle_parameters(float frequency, float sample_rate);
void sg_free_triangle_parameters(struct triangle_parameters *params);

struct sine_parameters *sg_create_sine_parameters(float frequency, float sample_rate);
void sg_free_sine_parameters(struct sine_parameters *params);

float sg_advance_generator(struct sound_params *sc);

#endif  /* __GENERATORS_H__ */
