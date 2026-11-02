#!/bin/sh
# Windows installer of the single exe (MSYS2 UCRT64, NSIS: pacman -S mingw-w64-ucrt-x86_64-nsis)
#   installer/build-installer.sh [exe] [output]
# Default: build-static/ClaudeReader.exe (./build.sh --static) -> dist/ClaudeReader-windows-x64-setup.exe
set -e
cd "$(dirname "$0")/.."

exe=${1:-build-static/ClaudeReader.exe}
out=${2:-dist/ClaudeReader-windows-x64-setup.exe}
version=$(sed -n 's/^project(ClaudeReader VERSION \([0-9.]*\).*/\1/p' CMakeLists.txt)
# Windows file versions have 4 numbers: 1.2 -> 1.2.0.0
vi=$(echo "$version.0.0.0" | cut -d. -f1-4)

if [ ! -f "$exe" ]; then
    echo "Missing $exe: build it first (./build.sh --static)"
    exit 1
fi

stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT
cp "$exe" "$stage/ClaudeReader.exe"
cp README.md "$stage/README.md"
cp CHANGES "$stage/CHANGES.txt"
cp doc/License.txt "$stage/LICENSE.txt"
cp src/Icon2_48.ico "$stage/ClaudeReader.ico"
mkdir -p "$(dirname "$out")"

makensis -V2 -DVERSION="$version" -DVIVERSION="$vi" -DSTAGE="$(cygpath -w "$stage")" \
    -DOUTFILE="$(cygpath -wa "$out")" installer/ClaudeReader.nsi
echo "$out (ClaudeReader $version)"
