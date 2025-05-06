#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include "audio_processing.h"
#define RAYLIB_IMPLEMENTATION
#include <raylib.h>

static ma_device device;

static ma_pcm_rb rb;

bool init_ring_buffer();

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_result result;
    ma_uint32 framesWritten;

    (void)pOutput;

    framesWritten = 0;
    while(framesWritten < frameCount) {
        void* pMappedBuffer;
        ma_uint32 framesToWrite = frameCount - framesWritten;

        result = ma_pcm_rb_acquire_write(&rb, &framesToWrite, &pMappedBuffer);
        if (result != MA_SUCCESS) {
            break;
        }

        if (framesToWrite == 0) {
            break;
        }

        /* Copy the data from the capture buffer to the ring buffer. */
        ma_copy_pcm_frames(pMappedBuffer, ma_offset_pcm_frames_const_ptr_f32((const float*)pInput, framesWritten, pDevice->capture.channels), framesToWrite, pDevice->capture.format, pDevice->capture.channels);

        result = ma_pcm_rb_commit_write(&rb, framesToWrite);
        if (result != MA_SUCCESS) {
            break;
        }

        framesWritten += framesToWrite;
    }
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
    deviceConfig.capture.format = CAPTURE_FORMAT;
    deviceConfig.capture.channels = CAPTURE_CHANNELS;
    deviceConfig.sampleRate = SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = NULL;


    //Initialize device with config
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize capture device.\n");
        return false;
    }

    //Init ring buffer
    init_ring_buffer();

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

    //Uninit ring buffer
    ma_pcm_rb_uninit(&rb);
}

bool init_ring_buffer() {
    ma_result result = ma_pcm_rb_init(CAPTURE_FORMAT, CAPTURE_CHANNELS, SAMPLE_RATE, NULL, NULL, &rb);

    if (result != MA_SUCCESS) {
        printf("Failed to initialize ring buffer.\n");
        return false;
    }
    return true;
}

void printSamples() {
    ma_result result;
    ma_uint32 framesAvailable;
    ma_uint32 framesToRead = 256;
    float* pMappedBuffer;

    framesAvailable = ma_pcm_rb_available_read(&rb);
    if (framesAvailable == 0) {
        return;  // No new data to read
    }

    if (framesAvailable < framesToRead) {
        framesToRead = framesAvailable;
    }

    result = ma_pcm_rb_acquire_read(&rb, &framesToRead, (void**)&pMappedBuffer);
    if (result != MA_SUCCESS || framesToRead == 0) {
        return;
    }

    for (ma_uint32 i = 0; i < framesToRead; i++) {
        printf("Sample %u: %f\n", i, pMappedBuffer[i]);
    }

    ma_pcm_rb_commit_read(&rb, framesToRead);
}

void drawWindow() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        printSamples();
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();  
}