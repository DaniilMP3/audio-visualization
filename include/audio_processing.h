#ifndef AUDIO_PROCESSING_H
#define AUDIO_PROCESSING_H

#include <stdbool.h>
#include "miniaudio.h"

#define CAPTURE_FORMAT ma_format_f32
#define CAPTURE_CHANNELS 1
#define SAMPLE_RATE 44100

bool init_audio_device();
void uninit_audio_device();
void peek_from_ring_buffer(size_t sizeInBytes, void* pTargetBuffer);

void drawWindow();
#endif