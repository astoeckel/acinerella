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

#include <stdlib.h>
#include "acinerella.h"
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

#define AUDIO_BUFFER_BASE_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2)

//This struct represents one Acinerella video object.
//It contains data needed by FFMpeg.
struct _ac_data {
  ac_instance instance;
  
  AVFormatContext *pFormatCtx;
  
  void *sender;
  ac_openclose_callback open_proc;
  ac_read_callback read_proc; 
  ac_openclose_callback close_proc; 
  
  URLProtocol protocol;
  char protocol_name[9];  
};

typedef struct _ac_data ac_data;
typedef ac_data* lp_ac_data;

struct _ac_video_decoder {
  ac_decoder decoder;
  AVCodec *pCodec;
  AVCodecContext *pCodecCtx;
  AVFrame *pFrame;
  AVFrame *pFrameRGB;  
};

typedef struct _ac_video_decoder ac_video_decoder;
typedef ac_video_decoder* lp_ac_video_decoder;

struct _ac_audio_decoder {
  ac_decoder decoder;
  int max_buffer_size;
  AVCodec *pCodec;
  AVCodecContext *pCodecCtx;
};

typedef struct _ac_audio_decoder ac_audio_decoder;
typedef ac_audio_decoder* lp_ac_audio_decoder;

struct _ac_package_data {
  ac_package package;
  AVPacket ffpackage;
};

typedef struct _ac_package_data ac_package_data;
typedef ac_package_data* lp_ac_package_data;

//
//--- Memory manager ---
//

ac_malloc_callback mgr_malloc = &malloc;
ac_realloc_callback mgr_realloc =  &realloc;
ac_free_callback mgr_free = &free;

void CALL_CONVT ac_mem_mgr(ac_malloc_callback mc, ac_realloc_callback rc, ac_free_callback fc) {
  mgr_malloc = mc;
  mgr_realloc = rc;
  mgr_free = fc;
}

//
//--- Initialization and Stream opening---
//

lp_ac_instance CALL_CONVT ac_init(void) {
  //Initialize FFMpeg libraries
  av_register_all();
  
  //Allocate a new instance of the videoplayer data and return it
  lp_ac_data ptmp;  
  ptmp = (lp_ac_data)mgr_malloc(sizeof(ac_data));
  ptmp->instance.opened = 0;
  ptmp->instance.stream_count = 0;
  ptmp->instance.output_format = AC_OUTPUT_RGB24;
  return (lp_ac_instance)ptmp;  
}

void CALL_CONVT ac_free(lp_ac_instance pacInstance) {
  if (pacInstance != NULL) {
    mgr_free((lp_ac_data)pacInstance);
  }
}

int protocol_number;
lp_ac_data last_instance;

//Function called by FFMpeg when opening an ac stream.
static int file_open(URLContext *h, const char *filename, int flags)
{
  h->priv_data = last_instance;
  h->is_streamed = 1;
  
  if (last_instance->open_proc != NULL) {
    last_instance->open_proc(last_instance->sender);
  }
    
  return 0;
}

//Function called by FFMpeg when reading from the stream
static int file_read(URLContext *h, unsigned char *buf, int size)
{
  if (((lp_ac_data)(h->priv_data))->read_proc != NULL) {
    return ((lp_ac_data)(h->priv_data))->read_proc(((lp_ac_data)(h->priv_data))->sender, buf, size);
  }
  
  return -1;
}

//Function called by FFMpeg when the stream should be closed
static int file_close(URLContext *h)
{
  if (((lp_ac_data)(h->priv_data))->close_proc != NULL) {
    return ((lp_ac_data)(h->priv_data))->close_proc(((lp_ac_data)(h->priv_data))->sender);
  }
    
  return 0;
}

int CALL_CONVT ac_open(
  lp_ac_instance pacInstance,
  void *sender, 
  ac_openclose_callback open_proc,
  ac_read_callback read_proc, 
  ac_openclose_callback close_proc) {
  
  pacInstance->opened = 0;
  
  //Set last instance
  last_instance = (lp_ac_data)pacInstance;
  
  //Store the given parameters in the ac Instance
  ((lp_ac_data)pacInstance)->sender = sender;
  ((lp_ac_data)pacInstance)->open_proc = open_proc;  
  ((lp_ac_data)pacInstance)->read_proc = read_proc;
  ((lp_ac_data)pacInstance)->close_proc = close_proc;    
  
  //Increase protocol number for having an unique identifier
  ++protocol_number;  
  
  //Create a new protocol name
  strcpy(((lp_ac_data)pacInstance)->protocol_name, "acinx");
  ((lp_ac_data)pacInstance)->protocol_name[4] = (char)(protocol_number + 65);
 
  //Create a new protocol
  ((lp_ac_data)pacInstance)->protocol.name = ((lp_ac_data)pacInstance)->protocol_name;
  ((lp_ac_data)pacInstance)->protocol.url_open = &file_open;
  ((lp_ac_data)pacInstance)->protocol.url_read = &file_read;
  ((lp_ac_data)pacInstance)->protocol.url_write = NULL;
  ((lp_ac_data)pacInstance)->protocol.url_seek = NULL;
  ((lp_ac_data)pacInstance)->protocol.url_close = &file_close;
  
  //Register the generated protocol
  register_protocol(&((lp_ac_data)pacInstance)->protocol);
  
  //Generate a unique filename
  char filename[50];
  strcpy(filename, ((lp_ac_data)pacInstance)->protocol_name);
  strcat(filename, "://dummy.file");
  
  if(av_open_input_file(
      &(((lp_ac_data)pacInstance)->pFormatCtx), filename, NULL, 0, NULL) != 0 ) {
    return -1;
  }
 
  //Retrieve stream information
  if(av_find_stream_info(((lp_ac_data)pacInstance)->pFormatCtx)<0) {
    return -1;
  }
  
  //Set some information in the instance variable
  pacInstance->stream_count = ((lp_ac_data)pacInstance)->pFormatCtx->nb_streams;
  pacInstance->opened = pacInstance->stream_count > 0;  
}

void CALL_CONVT ac_close(lp_ac_instance pacInstance) {
  if (pacInstance->opened) {
    av_close_input_file(((lp_ac_data)(pacInstance))->pFormatCtx);
  }
}
void CALL_CONVT ac_get_stream_info(lp_ac_instance pacInstance, int nb, lp_ac_stream_info info) {
  if (!(pacInstance->opened)) { 
    return;
  }
  
  switch (((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->codec_type) {
    case CODEC_TYPE_VIDEO:
      //Set stream type to "VIDEO"
      info->stream_type = AC_STREAM_TYPE_VIDEO;
      
      //Store more information about the video stream
      info->additional_info.video_info.frame_width = 
        ((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->width;
      info->additional_info.video_info.frame_height = 
        ((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->height;
      info->additional_info.video_info.pixel_aspect = 
        av_q2d(((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->sample_aspect_ratio);
      //Sometime "pixel aspect" may be zero. Correct this.
      if (info->additional_info.video_info.pixel_aspect == 0.0) {
        info->additional_info.video_info.pixel_aspect = 1.0;
      }
      
      info->additional_info.video_info.frames_per_second = 
        av_q2d(((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->time_base);
    break;
    case CODEC_TYPE_AUDIO:
      //Set stream type to "AUDIO"
      info->stream_type = AC_STREAM_TYPE_AUDIO;
      
      //Store more information about the video stream
      info->additional_info.audio_info.samples_per_second = 
        ((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->sample_rate;        
      info->additional_info.audio_info.channel_count = 
        ((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->channels;
      
      // Set bit depth      
      switch (((lp_ac_data)pacInstance)->pFormatCtx->streams[nb]->codec->sample_fmt) {
        //8-Bit
        case SAMPLE_FMT_U8:
          info->additional_info.audio_info.bit_depth = 
            8;                
        break;
        
        //16-Bit
        case SAMPLE_FMT_S16:
          info->additional_info.audio_info.bit_depth = 
              16;                            
        break;
        
        //24-Bit
        case SAMPLE_FMT_S24:
          info->additional_info.audio_info.bit_depth = 
              24;                                          
        break;
        
        //32-Bit
        case SAMPLE_FMT_S32: case SAMPLE_FMT_FLT:
          info->additional_info.audio_info.bit_depth = 
              32;                                          
        break;       
         
        //Unknown format, return zero
        default:
          info->additional_info.audio_info.bit_depth = 
            0;        
      }
        
    break;
    default:
      info->stream_type = AC_STREAM_TYPE_UNKNOWN;
  }
}

//
//---Package management---
//

lp_ac_package CALL_CONVT ac_read_package(lp_ac_instance pacInstance) {
  //Try to read package
  AVPacket Package;  
  if (av_read_frame(((lp_ac_data)(pacInstance))->pFormatCtx, &Package) >= 0) {
    //Reserve memory
    lp_ac_package_data pTmp = (lp_ac_package_data)(mgr_malloc(sizeof(ac_package_data)));
    
    //Set package data
    pTmp->package.data = Package.data;
    pTmp->package.size = Package.size;
    pTmp->package.stream_index = Package.stream_index;
    pTmp->ffpackage = Package;
    
    return (lp_ac_package)(pTmp);
  } else {
    return NULL;
  }
}

//Frees the currently loaded package
void CALL_CONVT ac_free_package(lp_ac_package pPackage) {
  //Free the packet
  if (pPackage != NULL) {    
    av_free_packet(&((lp_ac_package_data)pPackage)->ffpackage);
    mgr_free((lp_ac_package_data)pPackage);
  }
}

//
//--- Decoder management ---
//

//Init a video decoder
void* ac_create_video_decoder(lp_ac_instance pacInstance, lp_ac_stream_info info, int nb) {
  //Allocate memory for a new decoder instance
  lp_ac_video_decoder pDecoder;  
  pDecoder = (lp_ac_video_decoder)(mgr_malloc(sizeof(ac_video_decoder)));
  
  //Set a few properties
  pDecoder->decoder.pacInstance = pacInstance;
  pDecoder->decoder.type = AC_DECODER_TYPE_VIDEO;
  pDecoder->decoder.stream_index = nb;
  pDecoder->pCodecCtx = ((lp_ac_data)(pacInstance))->pFormatCtx->streams[nb]->codec;
  pDecoder->decoder.stream_info = *info;  
  
  //Find correspondenting codec
  if (!(pDecoder->pCodec = avcodec_find_decoder(pDecoder->pCodecCtx->codec_id))) {
    return NULL; //Codec could not have been found
  }
  
  //Open codec
  if (avcodec_open(pDecoder->pCodecCtx, pDecoder->pCodec) < 0) {
    return NULL; //Codec could not have been opened
  }
  
  //Reserve frame variables
  pDecoder->pFrame = avcodec_alloc_frame();
  pDecoder->pFrameRGB = avcodec_alloc_frame();
  
  //Reserve buffer memory
  pDecoder->decoder.buffer_size = avpicture_get_size(PIX_FMT_RGB24, 
    pDecoder->pCodecCtx->width, pDecoder->pCodecCtx->height);
  pDecoder->decoder.pBuffer = (uint8_t*)mgr_malloc(pDecoder->decoder.buffer_size);

  //Link decoder to buffer
  avpicture_fill(
    (AVPicture*)(pDecoder->pFrameRGB), 
    pDecoder->decoder.pBuffer, PIX_FMT_RGB24,
    pDecoder->pCodecCtx->width, pDecoder->pCodecCtx->height);
    
  return (void*)pDecoder;
}

//Init a audio decoder
void* ac_create_audio_decoder(lp_ac_instance pacInstance, lp_ac_stream_info info, int nb) {
  //Allocate memory for a new decoder instance
  lp_ac_audio_decoder pDecoder;
  pDecoder = (lp_ac_audio_decoder)(mgr_malloc(sizeof(ac_audio_decoder)));
  
  //Set a few properties
  pDecoder->decoder.pacInstance = pacInstance;
  pDecoder->decoder.type = AC_DECODER_TYPE_AUDIO;
  pDecoder->decoder.stream_index = nb;
  pDecoder->decoder.stream_info = *info;
  
  //Temporary store codec context pointer
  AVCodecContext *pCodecCtx = ((lp_ac_data)(pacInstance))->pFormatCtx->streams[nb]->codec;
  pDecoder->pCodecCtx = pCodecCtx;  
  
  //Find correspondenting codec
  if (!(pDecoder->pCodec = avcodec_find_decoder(pCodecCtx->codec_id))) {
    return NULL;
  }
  
  //Open codec
  if (avcodec_open(pCodecCtx, pDecoder->pCodec) < 0) {
    return NULL;
  }

  //Reserve a buffer
  pDecoder->max_buffer_size = AUDIO_BUFFER_BASE_SIZE;
  pDecoder->decoder.pBuffer = (uint8_t*)(mgr_malloc(pDecoder->max_buffer_size));
  pDecoder->decoder.buffer_size = 0;
  
  return (void*)pDecoder;
}

lp_ac_decoder CALL_CONVT ac_create_decoder(lp_ac_instance pacInstance, int nb) {
  //Get information about the chosen data stream and create an decoder that can
  //handle this kind of stream.
  ac_stream_info info;
  ac_get_stream_info(pacInstance, nb, &info);
  
  if (info.stream_type == AC_STREAM_TYPE_VIDEO) {
    return ac_create_video_decoder(pacInstance, &info, nb);
  } 
  else if (info.stream_type == AC_STREAM_TYPE_AUDIO) {
    return ac_create_audio_decoder(pacInstance, &info, nb);  
  }
  
  return NULL;
}

enum PixelFormat convert_pix_format(ac_output_format fmt) {
  switch (fmt) {
    case AC_OUTPUT_RGB24: return PIX_FMT_RGB24;
    case AC_OUTPUT_BGR24: return PIX_FMT_BGR24;
    case AC_OUTPUT_RGBA32: return PIX_FMT_RGB32;
    case AC_OUTPUT_BGRA32: return PIX_FMT_BGR32;        
  }
  return PIX_FMT_RGB24;
}

int ac_decode_video_package(lp_ac_package pPackage, lp_ac_video_decoder pDecoder) {
  int finished;
  avcodec_decode_video(
    pDecoder->pCodecCtx, pDecoder->pFrame, &finished, 
    pPackage->data, pPackage->size);
  
  if (finished) {
    img_convert(
      (AVPicture*)(pDecoder->pFrameRGB), convert_pix_format(pDecoder->decoder.pacInstance->output_format), 
      (AVPicture*)(pDecoder->pFrame), pDecoder->pCodecCtx->pix_fmt, 
			pDecoder->pCodecCtx->width, pDecoder->pCodecCtx->height);
      
    return 1;
  }
  
  return 0;
}

int ac_decode_audio_package(lp_ac_package pPackage, lp_ac_audio_decoder pDecoder) {
  int len1;
  int dest_buffer_size = pDecoder->max_buffer_size;
  int dest_buffer_pos = 0;
  int size;
  uint8_t *src_buffer = pPackage->data;
  int src_buffer_size = pPackage->size;
  
  pDecoder->decoder.buffer_size = 0;
  
  while (src_buffer_size > 0) {
    //Set the size of bytes that can be written to the current size of the destination buffer
    size = dest_buffer_size;
    
    //Decode a piece of the audio buffer. len1 contains the count of bytes read from the soure buffer.
    len1 = avcodec_decode_audio2(
      pDecoder->pCodecCtx, (uint16_t*)((uint8_t*)pDecoder->decoder.pBuffer + dest_buffer_pos), &size, 
      src_buffer, src_buffer_size);
      
    src_buffer_size -= len1;
    src_buffer      += len1;
    
    dest_buffer_size -= size;
    dest_buffer_pos += size;
    pDecoder->decoder.buffer_size = dest_buffer_pos;
    
    if (dest_buffer_size <= AUDIO_BUFFER_BASE_SIZE) {
      pDecoder->decoder.pBuffer = mgr_realloc(pDecoder->decoder.pBuffer, pDecoder->max_buffer_size * 2);
      dest_buffer_size += pDecoder->max_buffer_size;
      pDecoder->max_buffer_size *= 2;
    }
    
    if (len1 <= 0) {    
      return 1;
    }
  }
  
  return 1;
}

int CALL_CONVT ac_decode_package(lp_ac_package pPackage, lp_ac_decoder pDecoder) {
  if (pDecoder->type == AC_DECODER_TYPE_VIDEO) {
    return ac_decode_video_package(pPackage, (lp_ac_video_decoder)pDecoder);
  } 
  else if (pDecoder->type == AC_DECODER_TYPE_AUDIO) {
    return ac_decode_audio_package(pPackage, (lp_ac_audio_decoder)pDecoder);  
  }
  return 0;
}

//Free video decoder
void ac_free_video_decoder(lp_ac_video_decoder pDecoder) {
  av_free(pDecoder->decoder.pBuffer);  
  av_free(pDecoder->pFrame);
  av_free(pDecoder->pFrameRGB);    
  avcodec_close(pDecoder->pCodecCtx);
  
  //Free reserved memory for decoder record
  mgr_free(pDecoder);
}

//Free video decoder
void ac_free_audio_decoder(lp_ac_audio_decoder pDecoder) {
  av_free(pDecoder->decoder.pBuffer);
  avcodec_close(pDecoder->pCodecCtx);

  //Free reserved memory for decoder record
  mgr_free(pDecoder);
}

void CALL_CONVT ac_free_decoder(lp_ac_decoder pDecoder) {
  if (pDecoder->type == AC_DECODER_TYPE_VIDEO) {
    ac_free_video_decoder((lp_ac_video_decoder)pDecoder);
  }
  else if (pDecoder->type == AC_DECODER_TYPE_AUDIO) {
    ac_free_audio_decoder((lp_ac_audio_decoder)pDecoder);  
  }
}
