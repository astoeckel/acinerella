/*
 * Acinerella -- ffmpeg Wrapper Library
 * Copyright (C) 2008-2018  Andreas Stöckel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Acinerella.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
WARNING:
Depending on the implementation of libc, reading of files larger than 1GB might
not work. This is not a problem with Acinerella and can be solved by using
stream objects delivered by your OS.
*/

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "config.h"
#include "acinerella.h"

#if __PLATFORM == PLATFORM_LINUX
#define O_BINARY 0
#endif

char *filename;
int source;

int CALL_CONVT read_callback(void *sender, uint8_t *buf, int size)
{
	return read(source, buf, size);
}

int64_t CALL_CONVT seek_callback(void *sender, int64_t pos, int whence)
{
	int64_t res = lseek(source, pos, whence);
	return res;
}

int CALL_CONVT open_callback(void *sender)
{
	source = open(filename, O_RDONLY | O_BINARY);
	if (source < 0) {
		perror("Open");
		return -1;
	}
	printf("Open '%s' \n", filename);
	return 0;
}

int CALL_CONVT close_callback(void *sender)
{
	printf("Closing file\n");
	return close(source);
}

void SaveFrame(uint8_t *buffer, int width, int height, int iFrame)
{
	FILE *pFile;
	char szFilename[32];

	// Open file
	sprintf(szFilename, "frame%05d.ppm", iFrame);
	pFile = fopen(szFilename, "wb");
	if (pFile == NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	fwrite(buffer, 1, height * width * 3, pFile);

	// Close file
	fclose(pFile);
}

int main(int argc, char *argv[])
{
	lp_ac_instance pData;
	lp_ac_decoder pVideoDecoder = NULL;
	lp_ac_decoder pAudioDecoder = NULL;
	int i;
	int framenb = 0;
	int audiofile;

	if (argc != 2) {
		printf("Usage: %s <MEDIA FILE>\n", argv[0]);
		exit(1);
	}

// Open a file for raw audio output
#if __PLATFORM == PLATFORM_LINUX
	audiofile = open("acin_test.raw", O_WRONLY | O_CREAT | O_TRUNC, 0777);
#else
	audiofile = open("acin_test.raw", O_WRONLY | O_CREAT | O_BINARY);
#endif

	// Save the filename of the file that should be opened in a string
	filename = argv[1];

	// Initialize an instance of Acinerella
	pData = ac_init();

	// Set the output format to one that is compilant with the ppm picture
	// format byte order
	pData->output_format = AC_OUTPUT_RGB24;

	// Open the video/audio file by passing the function pointers to the open,
	// read and close callbacks to Acinerella.
	// Only the read callback is neccessary, all other callbacks may be 0
	if (ac_open(pData, 0, &open_callback, &read_callback, &seek_callback,
	            &close_callback, NULL) < 0) {
		printf("Error opening media file!\n");
		exit(1);
	}

	// Display the count of the found data streams.
	printf("Count of Datastreams:  %d \n", pData->stream_count);

	// Print file info
	printf("File duration: %ld \n", pData->info.duration);
	printf("Title: %s \n", pData->info.title);
	printf("Author: %s \n", pData->info.author);
	printf("Album: %s \n", pData->info.album);

	// Go through every stream and fetch information about it.
	for (i = pData->stream_count - 1; i >= 0; --i) {
		printf("\nInformation about stream %d: \n", i);

		ac_stream_info info;

		// Get information about stream no. i
		ac_get_stream_info(pData, i, &info);

		switch (info.stream_type) {
			// Stream is a video stream - display information about it
			case AC_STREAM_TYPE_VIDEO:
				printf(
				    "Stream is an video "
				    "stream.\n--------------------------\n\n");
				printf(" * Width            : %d\n",
				       info.additional_info.video_info.frame_width);
				printf(" * Height           : %d\n",
				       info.additional_info.video_info.frame_height);
				printf(" * Pixel aspect     : %f\n",
				       info.additional_info.video_info.pixel_aspect);
				printf(" * Frames per second: %lf \n",
				       info.additional_info.video_info.frames_per_second);

				// If we don't have a video decoder now, try to create a video
				// decoder for this video stream
				if (pVideoDecoder == NULL) {
					pVideoDecoder = ac_create_decoder(pData, i);
				}
				break;

			// Stream is an audio stream - display information about it
			case AC_STREAM_TYPE_AUDIO:
				printf(
				    "Stream is an audio "
				    "stream.\n--------------------------\n\n");
				printf("  * Samples per Second: %d\n",
				       info.additional_info.audio_info.samples_per_second);
				printf("  * Channel count     : %d\n",
				       info.additional_info.audio_info.channel_count);
				printf("  * Bit depth         : %d\n",
				       info.additional_info.audio_info.bit_depth);

				// If we don't have an audio decoder now, try to create an audio
				// decoder for this audio stream
				if (pAudioDecoder == NULL) {
					pAudioDecoder = ac_create_decoder(pData, i);
				}
				break;
			default:
				break;
		}
	}

	// Check whether the audio file was opened properly
	if (!pData->opened) {
		printf("No video/audio information found.\n\n");
		return 0;
	}

	// Read all packets from the stream and try to decode them
	printf("\nReading packet data...\n\n");

	lp_ac_package pckt = NULL;
	do {
		pckt = ac_read_package(pData);
		if (pckt != NULL) {
			printf("Found packet for stream %d.\r", pckt->stream_index);

			if ((pVideoDecoder != NULL) &&
			    (pckt->stream_index == pVideoDecoder->stream_index)) {
				// The packet is for the video stream, try to decode it
				if (ac_decode_package(pckt, pVideoDecoder)) {
					// Save every 100th video frame to a ppm file
					if (framenb % 100 == 0) {
						SaveFrame(pVideoDecoder->pBuffer,
						          pVideoDecoder->stream_info.additional_info
						              .video_info.frame_width,
						          pVideoDecoder->stream_info.additional_info
						              .video_info.frame_height,
						          framenb);
					}
					++framenb;
				}
			}

			if ((pAudioDecoder != NULL) &&
			    (pckt->stream_index == pAudioDecoder->stream_index)) {
				// The packet is for the audio stream, decode it and write the
				// data to the raw audio file
				if (ac_decode_package(pckt, pAudioDecoder)) {
					write(audiofile, pAudioDecoder->pBuffer,
					      pAudioDecoder->buffer_size);
				}
			}
			ac_free_package(pckt);
		}
	} while (pckt != NULL);

	if (pVideoDecoder != NULL) {
		ac_free_decoder(pVideoDecoder);
	}

	if (pAudioDecoder != NULL) {
		ac_free_decoder(pAudioDecoder);
	}

	printf("End of stream reached                    \n");

	close(audiofile);

	ac_close(pData);
	ac_free(pData);
}
