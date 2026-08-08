#ifndef NDK_MEDIA_EXTRACTOR_H
#define NDK_MEDIA_EXTRACTOR_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AMediaExtractor AMediaExtractor;
typedef struct AMediaFormat AMediaFormat;
typedef int media_status_t;

#define AMEDIA_OK 0

AMediaExtractor* AMediaExtractor_new(void);
void AMediaExtractor_delete(AMediaExtractor* ext);
media_status_t AMediaExtractor_setDataSourceFd(AMediaExtractor* ext, int fd, off_t offset, off_t length);
size_t AMediaExtractor_getTrackCount(AMediaExtractor* ext);
AMediaFormat* AMediaExtractor_getTrackFormat(AMediaExtractor* ext, size_t idx);

#ifdef __cplusplus
}
#endif

#endif // NDK_MEDIA_EXTRACTOR_H
