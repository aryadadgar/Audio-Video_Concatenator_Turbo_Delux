#ifndef NDK_MEDIA_FORMAT_H
#define NDK_MEDIA_FORMAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AMediaFormat AMediaFormat;

bool AMediaFormat_getInt64(AMediaFormat* format, const char* name, int64_t* out);
void AMediaFormat_delete(AMediaFormat* format);

#ifdef __cplusplus
}
#endif

#define AMEDIAFORMAT_KEY_DURATION "durationUs"

#endif // NDK_MEDIA_FORMAT_H
