#!/bin/sh
# Configure and build with the MSYS2 UCRT64 toolchain (run from Git Bash or MSYS2)
#   ./build.sh            build/ClaudeReader.exe and the DLLs it needs
#   ./build.sh --static   build-static/ClaudeReader.exe alone (needs the static OpenCV, see deps/build-opencv.sh)
set -e
export PATH=/c/msys64/ucrt64/bin:$PATH
cd "$(dirname "$0")"

if [ "$1" = "--static" ]; then
    cmake -S . -B build-static -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-Release} -DCLAUDEREADER_STATIC=ON
    cmake --build build-static
    exit 0
fi

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-Release}
cmake --build build

# Copy the MSYS2 DLLs ClaudeReader.exe depends on, so it runs outside the MSYS2 environment
ldd build/ClaudeReader.exe | awk '$3 !~ /^\/c\/WINDOWS/ && $3 ~ /\.dll$/ {print $3}' | sort -u | xargs -I{} cp -u {} build/ 2>/dev/null || true
