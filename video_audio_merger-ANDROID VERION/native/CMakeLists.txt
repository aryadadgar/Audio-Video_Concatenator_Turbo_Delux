cmake_minimum_required(VERSION 3.10)
project(native_concat LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Expected to be built with Android NDK toolchain; to force ABI from command-line:
# cmake -DANDROID_ABI=x86 -DANDROID_PLATFORM=android-21 -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake ..

if (ANDROID)
    add_executable(native_concat src/main.cpp src/media_loop.cpp)
else()
    add_executable(native_concat src/main.cpp src/media_loop.cpp src/media_ndk_stubs.cpp)
endif()

# Link against NDK media APIs and android/log when building for Android
if (ANDROID)
    find_library(LOG-lib log)
    target_include_directories(native_concat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(native_concat PRIVATE mediandk ${LOG-lib})
else()
    target_include_directories(native_concat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif()
