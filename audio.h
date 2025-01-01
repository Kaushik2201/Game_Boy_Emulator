#ifndef audio_h
#define audio_h

#include <stdint.h>
#include "memory.h"

#define DIV_APU_FREQUENCY 512

#define ENVELOPE_SWEEP_RATE 8   // 64 Hz
#define SOUND_LENGTH_RATE 2      // 256 Hz
#define CH1_FREQ_SWEEP_RATE 4

#define DAC_ENABLED_MASK 0xF8
#define AUDIO_ON_MASK 0x80
#define CHANNEL_ENABLE_MASK 0x0F
#define VOLUME_MASK 0x07
#define LENGTH_MASK 0x3F

#define DUTY_CYCLE_MASK 0xC0

#define SWEEP_PACE_MASK 0x07
#define SWEEP_DIRECTION_MASK 0x08

#define ENVELOPE_PACE_MASK 0x0E
#define ENVELOPE_DIRECTION_MASK 0x10
#define ENVELOPE_VOLUME_MASK 0x07

#define LENGTH_ENABLE_MASK 0x40
#define TRIGGER_MASK 0x80
#define CH3_LENGTH_MASK 0xFF

#define RANDOMNESS_MASK 0x01
#define CH4_NOISE_CONTROL_MASK 0x0F
#define MASTER_VOLUME_MASK 0x77

typedef struct {
    uint8_t volume;
    uint8_t length_counter;
    uint8_t envelope_direction;
    uint8_t envelope_pace
    uint8_t waveform_id;
    uint8_t initial_length_timer;
    uint16_t frequency;
    uint16_t current_frequency;
    uint8_t randomness;
    uint32_t phase_accumulator;
} gbc_audio_channel;


typedef struct {
gbc_channel *channel1;
gbc_audio_channel channel2;
    gbc_audio_channel channel3;
    gbc_audio_channel channel4;
    uint8_t master_volume_left;
    uint8_t master_volume_right;
}gbc_audio;

#define FREQUENCY_TO_PERIOD(freq) (DIV_APU_FREQUENCY / (freq))

#define PERIOD_TO_FREQUENCY(period) (DIV_APU_FREQUENCY / (period))

#define DUTY_VALUE_TO_PERCENTAGE(duty) ((duty) * 25)

#define SWEEP_RATE_TO_PACE(sweep_rate) ((sweep_rate) * 8)

#define GET_SWEEP_DIRECTION(sweep) (((sweep) & SWEEP_DIRECTION_MASK) >> 3)

#define GET_SWEEP_STEPS(sweep) ((sweep) & SWEEP_PACE_MASK)

#define GET_ENVELOPE_PACE(envelope) (((envelope) & ENVELOPE_PACE_MASK) >> 1)

#define GET_ENVELOPE_DIRECTION(envelope) (((envelope) & ENVELOPE_DIRECTION_MASK) >> 4)

#define GET_ENVELOPE_VOLUME(envelope) ((envelope) & ENVELOPE_VOLUME_MASK)

#define IS_DAC_ENABLED(channel) (((channel) & 0xF8) != 0)

#define ENABLE_DAC(channel) ((channel) |= 0xF8)

#define DISABLE_DAC(channel) ((channel) &= ~0xF8)

#define TRIGGER_CHANNEL(channel) ((channel) |= 0x80)

#define SET_CHANNEL_VOLUME(channel, volume) \
    ((channel) = ((channel) & ~VOLUME_MASK) | ((volume) & VOLUME_MASK))

#define SET_CHANNEL_DUTY(channel, duty) \
    ((channel) = ((channel) & ~DUTY_CYCLE_MASK) | (((duty) << 6) & DUTY_CYCLE_MASK))


void audio_init();
void audio_set_channel1(audio_config *config);
void audio_set_channel2(uint8_t duty, uint8_t volume, uint8_t length, uint16_t frequency);
void audio_set_channel3(uint8_t dac, uint8_t length, uint8_t output_level, uint16_t frequency);
void audio_set_channel4(uint8_t length, uint8_t volume, uint8_t randomness, uint8_t trigger);

#endif
