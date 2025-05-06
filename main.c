#include "audio_processing.h"
#include <stdio.h>


int main(int argc, char** argv)
{
    init_audio_device();

    drawWindow();

    uninit_audio_device();
}
