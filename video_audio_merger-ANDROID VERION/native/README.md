# Native concat/merge prototype (x86, minSdk 21)

This branch contains an NDK-native prototype for the audio-video concatenation tool.

Goals
- x86-only native CLI built with Android NDK (minSdk 21)
- Per-pair behavior: for each mp4 video and its paired music, if music is shorter than the video, loop the music to match the video duration; if music is longer, trim it.
- No ffmpeg; use Android NDK Media APIs (AMediaExtractor / AMediaCodec / AMediaMuxer)

Current status
- Scaffold committed: CMakeLists + README + main.cpp stub that scans directory, pairs files, and probes durations.
- Added CLI flags to control attenuation:
  --attenuate-percent <0-100>  : reduce volume by this percent (e.g., 96 means leave 4% of original)
  --volume-factor <0.0-1.0>    : directly specify multiplier to apply to PCM (e.g., 0.04)

Default behavior
- If neither flag provided, default is --attenuate-percent 96 (i.e., leave 4% of original volume).
- Attenuation is applied after looping/trimming and before encoding the audio track.

How to build
1. Set NDK path (example):
   export ANDROID_NDK_HOME=/path/to/android-ndk
2. Create build directory and run cmake with the NDK toolchain:
   mkdir -p native/build && cd native/build
   cmake -DANDROID_ABI=x86 -DANDROID_PLATFORM=android-21 -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake ..
   cmake --build . -- -j4

How to run (example)
- Reduce audio by 96% (leave 4%):
  adb push native/build/native_concat /data/local/tmp/native_concat
  adb shell
  su
  chmod +x /data/local/tmp/native_concat
  /data/local/tmp/native_concat --attenuate-percent 96 /sdcard/Media/Clips

Next steps
- Implement the full media pipeline: audio decode+loop+scale+encode and muxing with video (copy or re-encode). This is the next commit.
