//--------------------------------------------------------------------------
//
//    This file is part of ClaudeReader (based on BDReader, Copyright (C) 2014-2016  Vincent Crocher).
//
//    ClaudeReader is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    ClaudeReader is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with ClaudeReader. If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------
/**
 * \file StaticImports.cpp
 * \brief Static build only: MSYS2 compiled some static libraries (libarchive, MuPDF) for the zlib and OpenJPEG DLLs.
 *
 * Their code calls these functions through the DLL import pointers (__imp_name); with the static zlib and OpenJPEG,
 * the pointers are defined here, to the statically linked functions.
 */

//No header: zlib.h would declare these functions as imported too
#define STATIC_IMPORT(name) extern "C" void name(); extern "C" void *__imp_##name; void *__imp_##name=(void*)&name;

//zlib
STATIC_IMPORT(crc32)
STATIC_IMPORT(deflate)
STATIC_IMPORT(deflateEnd)
STATIC_IMPORT(deflateInit2_)
STATIC_IMPORT(deflateInit_)
STATIC_IMPORT(deflateReset)
STATIC_IMPORT(inflate)
STATIC_IMPORT(inflateEnd)
STATIC_IMPORT(inflateInit2_)
STATIC_IMPORT(inflateInit_)
STATIC_IMPORT(inflateReset)
STATIC_IMPORT(inflateSetDictionary)

//OpenJPEG
STATIC_IMPORT(opj_create_decompress)
STATIC_IMPORT(opj_decode)
STATIC_IMPORT(opj_destroy_codec)
STATIC_IMPORT(opj_image_destroy)
STATIC_IMPORT(opj_read_header)
STATIC_IMPORT(opj_set_default_decoder_parameters)
STATIC_IMPORT(opj_set_error_handler)
STATIC_IMPORT(opj_set_info_handler)
STATIC_IMPORT(opj_set_warning_handler)
STATIC_IMPORT(opj_setup_decoder)
STATIC_IMPORT(opj_stream_default_create)
STATIC_IMPORT(opj_stream_destroy)
STATIC_IMPORT(opj_stream_set_read_function)
STATIC_IMPORT(opj_stream_set_seek_function)
STATIC_IMPORT(opj_stream_set_skip_function)
STATIC_IMPORT(opj_stream_set_user_data)
STATIC_IMPORT(opj_stream_set_user_data_length)
