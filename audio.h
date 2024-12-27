#ifndef audio_h
#define audio_h

#include <stdint.h>

// Audio Registers
#define NR10_REG 0xFF10
#define NR11_REG 0xFF11
#define NR12_REG 0xFF12
#define NR13_REG 0xFF13
#define NR14_REG 0xFF14

#define NR21_REG 0xFF16
#define NR22_REG 0xFF17
#define NR23_REG 0xFF18
#define NR24_REG 0xFF19

#define NR30_REG 0xFF1A
#define NR31_REG 0xFF1B
#define NR32_REG 0xFF1C
#define NR33_REG 0xFF1D
#define NR34_REG 0xFF1E

#define NR41_REG 0xFF1F
#define NR42_REG 0xFF20
#define NR43_REG 0xFF21
#define NR44_REG 0xFF22

#define NR50_REG 0xFF24
#define NR51_REG 0xFF25
#define NR52_REG 0xFF26

// PCM Registers
#define PCM12_REGISTER 0xFF76
#define PCM34_REGISTER 0xFF77

#define DIV_APU_FREQUENCY 512

#define ENVELOPE_SWEEP_RATE 8   // 64 Hz
#define SOUND_LENGTH_RATE 2      // 256 Hz
#define CH1_FREQ_SWEEP_RATE 4    // 128 Hz

#define DAC_ENABLED_MASK 0xF8

#define MAX_VOLUME 7            // Maximum vol
#define MIN_VOLUME 0            // Minimum vol


typedef struct {
    uint8_t sweep;
    uint8_t duty;
    uint8_t volume;
    uint8_t length;
    uint16_t frequency;
}audio_config;

// Function Prototypes
void audio_init();
void audio_set_channel1(audio_config *config);
void audio_set_channel2(uint8_t duty, uint8_t volume, uint8_t length, uint16_t frequency);
void audio_set_channel3(uint8_t dac, uint8_t length, uint8_t output_level, uint16_t frequency);
void audio_set_channel4(uint8_t length, uint8_t volume, uint8_t randomness, uint8_t trigger);
void audio_set_master_volume(uint8_t left_volume, uint8_t right_volume);
void audio_enable_channel(uint8_t channel);
void audio_disable_channel(uint8_t channel);
void audio_trigger_channel(uint8_t channel);

uint8_t read_pcm12(void);
uint8_t read_pcm34(void);


void update_audio(void);
void reset_audio(void);


void audio_toggle_sound(uint8_t enable);

#endif
