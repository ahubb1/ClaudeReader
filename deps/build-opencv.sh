#!/bin/sh
# Static OpenCV for the single exe (./build.sh --static): only the modules ClaudeReader uses, the image codecs
# from the MSYS2 static libraries (shared with FLTK and MuPDF), installed in deps/opencv-static
set -e
export PATH=${MINGW_PREFIX:-/c/msys64/ucrt64}/bin:$PATH
cd "$(dirname "$0")"
VERSION=5.0.0
if [ ! -d src/opencv-$VERSION ]; then
    mkdir -p src
    curl -L -o src/opencv-$VERSION.tar.gz https://github.com/opencv/opencv/archive/refs/tags/$VERSION.tar.gz
    tar xzf src/opencv-$VERSION.tar.gz -C src
fi
LIB=${MINGW_PREFIX:-/c/msys64/ucrt64}/lib
cmake -S src/opencv-$VERSION -B build-opencv -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD/opencv-static" \
    -DCMAKE_CXX_FLAGS="-DOPJ_STATIC -DLZMA_API_STATIC -DLERC_STATIC" \
    -DBUILD_SHARED_LIBS=OFF -DBUILD_LIST=core,imgproc,imgcodecs,geometry \
    -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_opencv_apps=OFF -DBUILD_JAVA=OFF -DBUILD_opencv_python3=OFF -DBUILD_DOCS=OFF \
    -DWITH_IPP=OFF -DWITH_ITT=OFF -DWITH_OPENCL=OFF -DWITH_LAPACK=OFF -DWITH_EIGEN=OFF -DWITH_PROTOBUF=OFF -DWITH_FLATBUFFERS=OFF -DWITH_ADE=OFF \
    -DWITH_KLEIDICV=OFF -DWITH_FASTCV=OFF -DWITH_MLAS=OFF -DWITH_OPENVINO=OFF -DWITH_FFMPEG=OFF -DWITH_GSTREAMER=OFF -DWITH_MSMF=OFF -DWITH_DSHOW=OFF \
    -DWITH_WIN32UI=OFF -DWITH_QT=OFF -DWITH_VTK=OFF \
    -DBUILD_ZLIB=OFF -DBUILD_PNG=OFF -DBUILD_JPEG=OFF -DBUILD_WEBP=OFF -DBUILD_TIFF=OFF -DBUILD_OPENJPEG=OFF -DBUILD_JASPER=OFF \
    -DWITH_TIFF=ON -DWITH_WEBP=ON -DWITH_PNG=ON -DWITH_JPEG=ON -DWITH_OPENJPEG=ON -DWITH_IMGCODEC_GIF=ON \
    -DWITH_SPNG=OFF -DWITH_JASPER=OFF -DWITH_OPENEXR=OFF -DWITH_AVIF=OFF -DWITH_JPEGXL=OFF -DWITH_GDAL=OFF -DWITH_GDCM=OFF \
    -DZLIB_LIBRARY=$LIB/libz.a -DPNG_LIBRARY=$LIB/libpng.a -DJPEG_LIBRARY=$LIB/libjpeg.a -DTIFF_LIBRARY=$LIB/libtiff.a -DWEBP_LIBRARY=$LIB/libwebp.a
cmake --build build-opencv
cmake --install build-opencv
