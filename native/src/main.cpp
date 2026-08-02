#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <media/NdkMediaExtractor.h>
#include <media/NdkMediaFormat.h>

using std::string;
using std::vector;

static vector<string> list_files_in_dir(const string &dirpath) {
    vector<string> out;
    DIR *d = opendir(dirpath.c_str());
    if (!d) return out;
    struct dirent *ent;
    while ((ent = readdir(d)) != nullptr) {
        if (ent->d_name[0] == '.') continue;
        string p = dirpath + "/" + ent->d_name;
        out.push_back(p);
    }
    closedir(d);
    return out;
}

static bool ends_with(const string &s, const string &suffix) {
    if (s.size() < suffix.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Sort by path length then lexicographically to mirror VB.NET ordering in Form1.vb
static void vb_like_sort(vector<string> &v) {
    std::sort(v.begin(), v.end(), [](const string &a, const string &b) {
        if (a.size() != b.size()) return a.size() < b.size();
        return a < b;
    });
}

// Probe duration (in microseconds) using AMediaExtractor; returns -1 on failure
static int64_t probe_duration_us(const string &path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }
    AMediaExtractor *ext = AMediaExtractor_new();
    if (!ext) {
        close(fd);
        return -1;
    }
    off_t fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    media_status_t status = AMediaExtractor_setDataSourceFd(ext, fd, 0, fileSize);
    if (status != AMEDIA_OK) {
        std::cerr << "AMediaExtractor_setDataSourceFd failed for " << path << "\n";
        AMediaExtractor_delete(ext);
        close(fd);
        return -1;
    }

    size_t numTracks = AMediaExtractor_getTrackCount(ext);
    int64_t durationUs = -1;
    for (size_t i = 0; i < numTracks; ++i) {
        AMediaFormat *format = AMediaExtractor_getTrackFormat(ext, i);
        if (!format) continue;
        int64_t d = 0;
        if (AMediaFormat_getInt64(format, AMEDIAFORMAT_KEY_DURATION, &d) && d > 0) {
            // Keep the maximum duration found among tracks
            if (d > durationUs) durationUs = d;
        }
        AMediaFormat_delete(format);
    }

    AMediaExtractor_delete(ext);
    close(fd);
    return durationUs; // microseconds
}

static void print_usage(const char *prog) {
    std::cout << "Usage: " << prog << " <directory-or-list-of-files>\n";
    std::cout << "If a directory is given, all files inside are scanned.\n";
}

// Placeholder for the actual per-pair processing:
// - decode audio
// - loop audio by re-feeding decoded samples (or using codec-level looping)
// - encode audio and video to target profile
// - mux into segment_i.mp4
// For now this is a stub which only prints what it would do.
static int process_pair_stub(const string &videoPath, const string &audioPath, int pairIndex) {
    int64_t vdur = probe_duration_us(videoPath);
    int64_t adur = probe_duration_us(audioPath);
    std::cout << "Pair " << pairIndex << ":\n";
    std::cout << "  video: " << videoPath << " duration_us=" << vdur << "\n";
    std::cout << "  audio: " << audioPath << " duration_us=" << adur << "\n";
    if (vdur <= 0) {
        std::cerr << "  [WARN] Could not probe video duration. Skipping pair.\n";
        return -1;
    }
    if (adur <= 0) {
        std::cerr << "  [WARN] Could not probe audio duration. Will attempt to mux video without audio.\n";
        return 0;
    }

    if (adur < vdur) {
        std::cout << "  [INFO] Audio shorter than video: will loop audio to match video duration (per-pair).\n";
    } else if (adur > vdur) {
        std::cout << "  [INFO] Audio longer than video: will trim audio to video duration.\n";
    } else {
        std::cout << "  [INFO] Audio matches video duration.\n";
    }

    // TODO: Implement decoding audio, looping, encoding, muxing using AMediaCodec & AMediaMuxer.
    // This will be implemented in the next commit.

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    vector<string> files;
    string firstArg = argv[1];
    struct stat st;
    if (stat(firstArg.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        files = list_files_in_dir(firstArg);
    } else {
        // treat all args as files
        for (int i = 1; i < argc; ++i) files.emplace_back(argv[i]);
    }

    if (files.empty()) {
        std::cerr << "No files found.\n";
        return 1;
    }

    // Partition into video (.mp4) and audio (others)
    vector<string> vFiles, aFiles;
    for (auto &f : files) {
        string lower = f;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (ends_with(lower, ".mp4")) vFiles.push_back(f);
        else aFiles.push_back(f);
    }

    vb_like_sort(vFiles);
    vb_like_sort(aFiles);

    size_t totalPairs = std::min(vFiles.size(), aFiles.size());
    if (totalPairs == 0) {
        std::cerr << "Need at least one .mp4 video and one audio file.\n";
        return 1;
    }

    std::cout << "Found " << vFiles.size() << " videos and " << aFiles.size() << " audio files.\n";
    std::cout << "Processing " << totalPairs << " pairs.\n";

    for (size_t i = 0; i < totalPairs; ++i) {
        int r = process_pair_stub(vFiles[i], aFiles[i], (int)i + 1);
        if (r != 0) std::cerr << "Pair " << i + 1 << " returned error code " << r << "\n";
    }

    std::cout << "Prototype run finished (stub). Next: implement encode/mux loop.\n";
    return 0;
}
