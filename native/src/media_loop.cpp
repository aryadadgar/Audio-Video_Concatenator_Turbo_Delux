#include "media_loop.h"
#include <iostream>
#include <string>
#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>

int create_scaled_pcm_from_audio(const std::string &audioPath, double volumeFactor, const std::string &outPcmPath) {
    // Minimal behavior for the scaffold: verify the file is readable and return success/failure.
    // Real implementation will:
    //  - open AMediaExtractor on audioPath
    //  - configure AMediaCodec for the audio decoder
    //  - decode frames, apply PCM scaling (volumeFactor)
    //  - write PCM to outPcmPath

    FILE *f = fopen(audioPath.c_str(), "rb");
    if (!f) {
        std::cerr << "create_scaled_pcm_from_audio: could not open audio file " << audioPath << "\n";
        return -1;
    }
    fclose(f);

    std::cout << "[media_loop] (stub) would decode " << audioPath << " -> " << outPcmPath << " with volumeFactor=" << volumeFactor << "\n";
    // create an empty file to signal progress
    FILE *o = fopen(outPcmPath.c_str(), "wb");
    if (o) fclose(o);
    return 0;
}
