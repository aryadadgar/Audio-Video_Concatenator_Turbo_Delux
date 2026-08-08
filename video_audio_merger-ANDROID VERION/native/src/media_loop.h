#ifndef MEDIA_LOOP_H
#define MEDIA_LOOP_H

#include <string>

int create_scaled_pcm_from_audio(const std::string &audioPath, double volumeFactor, const std::string &outPcmPath);

#endif // MEDIA_LOOP_H
