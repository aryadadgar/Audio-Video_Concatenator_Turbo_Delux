# Native concat/merge prototype (x86, minSdk 21)

This branch contains an NDK-native prototype for the audio-video concatenation tool.

Goals
- x86-only native CLI built with Android NDK (minSdk 21)
- Per-pair behavior: for each mp4 video and its paired audio, if audio is shorter than the video, loop the audio to match the video duration; if longer, trim it.
- No ffmpeg; use Android NDK Media APIs (AMediaExtractor / AMediaCodec / AMediaMuxer / avfilter not used)

What is included
- native/CMakeLists.txt — CMake file for building the native CLI
- native/src/main.cpp — implementation scaffold: scanning, pairing, duration probing and placeholders for the actual mux/encode loop

How to build (local Android NDK)
1. Set NDK path (example):
   export ANDROID_NDK_HOME=/path/to/android-ndk
2. Create build directory and run cmake with the NDK toolchain:
   mkdir -p native/build && cd native/build
   cmake -DANDROID_ABI=x86 -DANDROID_PLATFORM=android-21 -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake ..
   cmake --build . -- -j4

This will produce a native_concat executable under native/build/

How to run on an Android device (adb)
- Push binary to device and run in shell:
  adb push native/build/native_concat /data/local/tmp/native_concat
  adb shell
  su
  chmod +x /data/local/tmp/native_concat
  /data/local/tmp/native_concat /sdcard/Media/Clips

Notes & next steps
- The current implementation is a scaffold: main.cpp probes file durations and performs pairing. The core media processing (decoding audio, looping, encoding, muxing) is marked as TODO and will be implemented next.
- After you verify the build works on your environment, I'll implement the media processing loop, using AMediaCodec decoders/encoders and AMediaMuxer for output.

