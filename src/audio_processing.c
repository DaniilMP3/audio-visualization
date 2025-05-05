#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include "audio_processing.h"

static ma_device device;

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    float* pInputF32 = (float*)pInput;

    (void)pOutput;
}

bool init_audio_device() {
    ma_result result;
    ma_context context;

    ma_backend backends[] = {ma_backend_pulseaudio};

    if (ma_context_init(backends, 1, NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize context.\n");
        return false;
    }

    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    ma_device_info* pCaptureDeviceInfos;
    ma_uint32 captureDeviceCount;

    //Enumarate devices
    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
    if(result != MA_SUCCESS) {
        printf("Failed to retrieve device information.\n");
        return false;
    }
    
    //Set up capture device config
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.pDeviceID = &pCaptureDeviceInfos[0].id; //hardcoded value for now
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate = 44100;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = NULL;


    //Initialize device with config
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device.\n");
        return false;
    }

    //Start device
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        ma_device_uninit(&device);
        printf("Failed to start device.\n");
        return false;
    }

    return true;
}

void uninit_audio_device() {
    ma_device_uninit(&device);
}