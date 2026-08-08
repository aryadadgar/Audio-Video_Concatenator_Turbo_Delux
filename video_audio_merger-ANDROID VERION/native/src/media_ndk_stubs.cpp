#include <cstring>
#include <sys/types.h>
#include "media/NdkMediaExtractor.h"
#include "media/NdkMediaFormat.h"

struct AMediaExtractor {
    int fd;
    off_t length;
};

struct AMediaFormat {
    int64_t duration;
};

AMediaExtractor* AMediaExtractor_new(void) {
    AMediaExtractor* ext = new AMediaExtractor();
    ext->fd = -1;
    ext->length = 0;
    return ext;
}

void AMediaExtractor_delete(AMediaExtractor* ext) {
    delete ext;
}

media_status_t AMediaExtractor_setDataSourceFd(AMediaExtractor* ext, int fd, off_t offset, off_t length) {
    if (!ext) return -1;
    ext->fd = fd;
    ext->length = length;
    return AMEDIA_OK;
}

size_t AMediaExtractor_getTrackCount(AMediaExtractor* ext) {
    return ext ? 0 : 0;
}

AMediaFormat* AMediaExtractor_getTrackFormat(AMediaExtractor* ext, size_t idx) {
    (void)ext;
    (void)idx;
    return nullptr;
}

bool AMediaFormat_getInt64(AMediaFormat* format, const char* name, int64_t* out) {
    (void)format;
    (void)name;
    if (out) *out = 0;
    return false;
}

void AMediaFormat_delete(AMediaFormat* format) {
    delete format;
}
