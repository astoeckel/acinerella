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

#include "acinerella.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>

char *filename;
int fd;

int CALL_CONVT read_callback(void *sender, char *buf, int size) {
  //printf("Lese %d Bytes. \n", size);
  return read(fd, buf, size);
}

int CALL_CONVT open_callback(void *sender) {
  fd = open(filename, O_RDONLY, 0666);  
  printf("÷ffne '%s' \n", filename);
  return 0;
}

int CALL_CONVT close_callback(void *sender) {
  printf("Schlieﬂe... \n");  
  return close(fd);  
}

void SaveFrame(char *buffer, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  fwrite(buffer, 1, height*width*3, pFile);
  
  // Close file
  fclose(pFile);
}

int main(int argc, char *argv[]) {
  lp_ac_instance pData;
  lp_ac_decoder pVideoDecoder = NULL;
  lp_ac_decoder pAudioDecoder = NULL;    
  int i;
  int framenb = 0;
  FILE *pAudiofile;
  
  pAudiofile = fopen("test.raw", "w");
  
  filename = argv[1];
  
  pData = ac_init();
  ac_open(pData, 0, &open_callback, &read_callback, &close_callback);
  
  printf("Anzahl der Video-/Audiodatenstrˆme: %d \n", pData->stream_count);
  
  for (i = pData->stream_count - 1; i >= 0; --i) {
    printf("\nInformationen f¸r Datenstrom %d: \n", i);
    ac_stream_info info;
    ac_get_stream_info(pData, i, &info);    
    
    switch (info.stream_type) {
      case AC_STREAM_TYPE_VIDEO:
        printf("Strom ist ein Videodatenstrom. \n");
        printf("Bild Breite: %d, Frame Hˆhe: %d \n", 
          info.additional_info.video_info.frame_width,
          info.additional_info.video_info.frame_height);
        printf("Bildpunktseitenverh‰ltnis: %f \n", 
          info.additional_info.video_info.pixel_aspect);
        printf("Bilder pro Sekunde: %lf \n", 
          1.0 / info.additional_info.video_info.frames_per_second);          
          
        if (pVideoDecoder == NULL) {
          printf("\nSuche Decoder f¸r Videodatenstrom...\n");
          pVideoDecoder = ac_create_decoder(pData, i);
        }
      break;
      
      case AC_STREAM_TYPE_AUDIO:
        printf("Strom ist ein Audiodatenstrom. \n");
        printf("Anzahl der Abtastpunkte pro Sekunde: %d \n", 
          info.additional_info.audio_info.samples_per_second);
        printf("Bittiefe eines Abtastpunkts: %d \n", 
          info.additional_info.audio_info.bit_depth);
        printf("Anzahl der Audiokan‰le: %d \n", 
          info.additional_info.audio_info.channel_count);
          
        if (pAudioDecoder == NULL) {
          printf("\nSuche Decoder f¸r Audiodatenstrom...\n");
          pAudioDecoder = ac_create_decoder(pData, i);
        }          
      break;      
    }
  }
  
  if (pVideoDecoder != NULL) {
    printf("\nVideodecoder f¸r ersten Videodatenstrom gefunden.\n");
  }
  
  if (pAudioDecoder != NULL) {
    printf("\nAudiodecoder f¸r ersten Audiodatenstrom gefunden.\n");
  }  
  
  printf("\nLese Paketdaten...\n");
  
  lp_ac_package pckt = NULL;
  do {
    pckt = ac_read_package(pData);
    if (pckt != NULL) {
      printf("Paket f¸r Strom %d gefunden. \r", pckt->stream_index);
      
      if ((pVideoDecoder != NULL) && (pckt->stream_index == pVideoDecoder->stream_index)) {
        if (ac_decode_package(pckt, pVideoDecoder)) {
          if (framenb % 100 == 0) {
          printf("\nSpeichere Videoframe Nr. %d.\n", framenb);
            SaveFrame(
               pVideoDecoder->pBuffer, 
               pVideoDecoder->stream_info.additional_info.video_info.frame_width, 
               pVideoDecoder->stream_info.additional_info.video_info.frame_height, framenb);
          }
          ++framenb;
        }      
      }
      
      if ((pAudioDecoder != NULL) && (pckt->stream_index == pAudioDecoder->stream_index)) {
        if (ac_decode_package(pckt, pAudioDecoder)) {
          fwrite(
            pAudioDecoder->pBuffer, 1, 
            pAudioDecoder->buffer_size, 
            pAudiofile);
        }
      }        
      ac_free_package(pckt);
    }
  } while(pckt != NULL);
  
  if (pVideoDecoder != NULL) {
    printf("\nSchlieﬂe Videodecoder.\n");
    ac_free_decoder(pVideoDecoder);
  }
  
  if (pAudioDecoder != NULL) {
    printf("\nSchlieﬂe Audiodecoder.\n");
    ac_free_decoder(pAudioDecoder);
  }  
  
  fclose(pAudiofile);
  
  ac_close(pData);
  ac_free(pData);  
}
