/*
    This file is part of Acinerella.

    Acinerella is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Acinerella is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Acinerella.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VIDEOPLAY_H
#define VIDEOPLAY_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef _WIN32
#define CALL_CONVT __cdecl
#else
#define CALL_CONVT  
#endif

//Enumeration that represents the type of an Acinerella stream
enum _ac_stream_type {
  AC_STREAM_TYPE_VIDEO = 0,
  AC_STREAM_TYPE_AUDIO = 1,
  AC_STREAM_TYPE_UNKNOWN = -1
};

typedef enum _ac_stream_type ac_stream_type;

enum _ac_decoder_type {
  AC_DECODER_TYPE_VIDEO = 0,
  AC_DECODER_TYPE_AUDIO = 1
};

typedef enum _ac_decoder_type ac_decoder_type;

typedef long long int64;

struct _ac_instance {
  bool opened;
  int stream_count;
};

typedef struct _ac_instance ac_instance;
typedef ac_instance* lp_ac_instance;

struct _ac_audio_stream_info {
  int samples_per_second;
  int bit_depth;
  int channel_count;
};

struct _ac_video_stream_info {
  int frame_width;
  int frame_height;
  float pixel_aspect;
  double frames_per_second;
};

union _ac_additional_stream_info {
  struct _ac_audio_stream_info audio_info;
  struct _ac_video_stream_info video_info;
};

struct _ac_stream_info {
  ac_stream_type stream_type;
  union _ac_additional_stream_info additional_info;
};

typedef struct _ac_stream_info ac_stream_info;
typedef ac_stream_info* lp_ac_stream_info;

struct _ac_decoder {
  lp_ac_instance pacInstance;
  ac_decoder_type type;
  
  ac_stream_info stream_info;
  int stream_index;
  
  char *pBuffer;  
  int buffer_size;
};

typedef struct _ac_decoder ac_decoder;
typedef ac_decoder* lp_ac_decoder;

struct _ac_package {
  char *data;
  int size;
  int stream_index;
};

typedef struct _ac_package ac_package;
typedef ac_package* lp_ac_package;

typedef int CALL_CONVT (*ac_read_callback)(void *sender, char *buf, int size);
typedef int CALL_CONVT (*ac_openclose_callback)(void *sender);
typedef void* CALL_CONVT (*ac_malloc_callback)(size_t size);
typedef void* CALL_CONVT (*ac_realloc_callback)(void *ptr, size_t size);
typedef void CALL_CONVT (*ac_free_callback)(void *ptr);

//Memory manager function
extern void CALL_CONVT ac_mem_mgr(ac_malloc_callback, ac_realloc_callback, ac_free_callback);

//Initialization function
extern lp_ac_instance CALL_CONVT ac_init(void);
extern void CALL_CONVT ac_free(lp_ac_instance pacInstance);

//Open function
extern int CALL_CONVT ac_open(
  lp_ac_instance pacInstance,
  void *sender, 
  ac_openclose_callback open_proc,
  ac_read_callback read_proc,  
  ac_openclose_callback close_proc);
  
extern void CALL_CONVT ac_close(lp_ac_instance pacInstance);
  
//Retrieves information about a specific video/audio stream
extern void CALL_CONVT ac_get_stream_info(lp_ac_instance pacInstance, int nb, lp_ac_stream_info info);

//Loads the next package from the stream and returns the number of the stream the package belongs to. If the result is -1, we are at the end of the video
extern lp_ac_package CALL_CONVT ac_read_package(lp_ac_instance pacInstance);
//Frees the currently loaded package
extern void CALL_CONVT ac_free_package(lp_ac_package pPackage);

//Get stream decoder
extern lp_ac_decoder CALL_CONVT ac_create_decoder(lp_ac_instance pacInstance, int nb);
//Free stream decoder
extern void CALL_CONVT ac_free_decoder(lp_ac_decoder pDecoder);
//Decode the currently loaded package
extern int CALL_CONVT ac_decode_package(lp_ac_package pPackage, lp_ac_decoder pDecoder);

#endif /*VIDEOPLAY_H*/
