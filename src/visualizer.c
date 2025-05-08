#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define CAPTURE_FORMAT ma_format_f32
#define CAPTURE_CHANNELS 1
#define SAMPLE_RATE 44100
#define VISUAL_BUFFER_SIZE 1024
#define WAVE_AMPLITUDE 300.0f 

#define MAX_OPTION_STRING 1024

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

static ma_pcm_rb rb;
static float visualBuffer[VISUAL_BUFFER_SIZE];

static int captureChannels;
static int sampleRate;

static ma_device device;
static ma_context context;
static ma_device_info* pPlaybackDeviceInfos;
static ma_uint32 playbackDeviceCount;
static ma_device_info* pCaptureDeviceInfos;
static ma_uint32 captureDeviceCount;


char dropdownOptions[MAX_OPTION_STRING] = { 0 };

static bool enumerate_capture_devices() {
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize context.\n");
        return false;
    }

    if(ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount) != MA_SUCCESS) {
        printf("Failed to retrieve device information.\n");
        ma_context_uninit(&context);
        return false;
    }

    return true;
}


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

bool init_ring_buffer() {
    ma_result result = ma_pcm_rb_init(CAPTURE_FORMAT, CAPTURE_CHANNELS, SAMPLE_RATE, NULL, NULL, &rb);

    if (result != MA_SUCCESS) {
        printf("Failed to initialize ring buffer.\n");
        return false;
    }
    return true;
}

bool uninit_ring_buffer() {
    ma_pcm_rb_uninit(&rb);
}

static bool init_capture_device_from_index(int selectedIndex) {
    ma_result result;

    if (selectedIndex < 0 || (ma_uint32)selectedIndex >= captureDeviceCount) {
        printf("Invalid device index.");
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.pDeviceID = &pCaptureDeviceInfos[selectedIndex].id; //hardcoded value for now
    deviceConfig.capture.format = CAPTURE_FORMAT;
    deviceConfig.capture.channels = CAPTURE_CHANNELS;
    deviceConfig.sampleRate = SAMPLE_RATE;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = NULL;

    if (ma_device_init(&context, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to initialize capture device.");
        return false;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start capture device.");
        ma_device_uninit(&device);
        return false;
    }

    // Clear the ring buffer when switching devices
    ma_pcm_rb_reset(&rb);

    return true;
}

static void uninit_capture_device() {
    ma_result result = ma_device_stop(&device);
    if (result != MA_SUCCESS) {
        printf("Warning: Failed to stop device before uninitializing. Error code: %d\n", result);
    }

    ma_device_uninit(&device);
}


void BuildCaptureDevicesDropdown() {
    dropdownOptions[0] = '\0';
    for (ma_uint32 i = 0; i < captureDeviceCount; ++i) {
        strcat(dropdownOptions, pCaptureDeviceInfos[i].name);
        if (i < captureDeviceCount - 1) strcat(dropdownOptions, ";");
    }
}



void DrawDeviceDropdown(int* activeIndex, bool* editMode)
{
    // Store current active index before dropdown interaction
    int previousActiveIndex = *activeIndex;
    
    if (GuiDropdownBox((Rectangle){ 0, 0, 400, 30 }, dropdownOptions, activeIndex, *editMode)) {
        *editMode = !(*editMode);  // Toggle dropdown state
        printf("Dropdown clicked - editMode toggled to: %s\n", *editMode ? "OPEN" : "CLOSED");
    }

    if (*editMode == false && previousActiveIndex != *activeIndex) {
        printf("New device selected: %d\n", *activeIndex);
    }

    // Display selected device info
    if (captureDeviceCount > 0 && *activeIndex >= 0 && (ma_uint32)*activeIndex < captureDeviceCount) {
        const char* deviceName = pCaptureDeviceInfos[*activeIndex].name;
        DrawText(TextFormat("Selected Device: [%d] %s", *activeIndex, deviceName), 0, 50, 20, DARKGRAY);
    } else {
        DrawText("No device selected or invalid selection", 100, 150, 20, RED);
    }
}


void updateAudioVisualization() {
    ma_result result;
    ma_uint32 framesAvailable;
    ma_uint32 framesToRead = VISUAL_BUFFER_SIZE;
    float* pMappedBuffer;
    framesAvailable = ma_pcm_rb_available_read(&rb);
    // Update only if all requested frames are available
    if (framesAvailable >= VISUAL_BUFFER_SIZE) {
        result = ma_pcm_rb_acquire_read(&rb, &framesToRead, (void**)&pMappedBuffer);
        if (result == MA_SUCCESS && framesToRead > 0) {
            for (ma_uint32 i = 0; i < framesToRead; i++) {
                if (CAPTURE_CHANNELS > 1) {
                    // Take only first channel if multichannel
                    visualBuffer[i] = pMappedBuffer[i * CAPTURE_CHANNELS];
                } else {
                    visualBuffer[i] = pMappedBuffer[i];
                }
            }
            ma_pcm_rb_commit_read(&rb, framesToRead);
        }
    } 
}

void drawAudioWaveform(int screenWidth, int screenHeight) {
    const int centerY = screenHeight / 2;
    const float xStep = (float)screenWidth / VISUAL_BUFFER_SIZE;
    for (int i = 0; i < VISUAL_BUFFER_SIZE-1; i++) {
        float x1 = i * xStep;
        float x2 = (i + 1) * xStep;
        float y1 = centerY - (visualBuffer[i] * WAVE_AMPLITUDE);
        float y2 = centerY - (visualBuffer[i+1] * WAVE_AMPLITUDE);  
        DrawLine((int)x1, (int)y1, (int)x2, (int)y2, RED);
    }
}


void start_visualization() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Audio Wave Visualizer");
    SetTargetFPS(60);

    init_ring_buffer();

    if(!enumerate_capture_devices()){
        return;
    }

    BuildCaptureDevicesDropdown();

    int selectedIndex = 0;
    int lastSelectedIndex = -1;  // Set to -1 to force initialization on first loop
    bool dropdownEditMode = false;
    bool deviceStarted = false;

    while (!WindowShouldClose())
    {
        updateAudioVisualization();

        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawDeviceDropdown(&selectedIndex, &dropdownEditMode);
            
            // Only reinitialize device if selection changed
            if (selectedIndex != lastSelectedIndex) {
                if (deviceStarted) {
                    uninit_capture_device();
                }

                deviceStarted = init_capture_device_from_index(selectedIndex);
                lastSelectedIndex = selectedIndex;
            }
            
            drawAudioWaveform(SCREEN_WIDTH, SCREEN_HEIGHT);

        EndDrawing();
    }

    // De-Initialization
    if (deviceStarted) {
        uninit_capture_device();
    }
    
    uninit_ring_buffer();
    ma_context_uninit(&context);
    CloseWindow();
}