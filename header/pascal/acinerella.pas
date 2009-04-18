{
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
}

{Pascal header translation for the Acinerella media decoder system. Acinerella
 wraps around FFMpeg and allows easy use in other programming languages.}
unit acinerella;

{$IFDEF FPC}
  {$MODE DELPHI}
{$ENDIF}

interface

{$MINENUMSIZE 1}
{$IFDEF WIN32}
  {$ALIGN 8}
{$ELSE}
  {$ALIGN 4}
{$ENDIF}

const
  {$IFDEF WIN32}
    ac_dll = 'acinerella.dll';
  {$ELSE}
    {$IFDEF UNIX}
      ac_dll = 'libacinerella.so';
    {$ELSE}
      {$MESSAGE Error 'Plattform not supported by Acinerella.'}
    {$ENDIF}
  {$ENDIF}

type 
  {Defines the type of an Acinerella media stream. Currently only video and
   audio streams are supported, subtitle and data streams will be marked as
   "unknown".}
  TAc_stream_type = (
    {The type of the media stream is not known. This kind of stream can not be
     decoded.}
    AC_STREAM_TYPE_UNKNOWN = -1,
    {This media stream is a video stream.}
    AC_STREAM_TYPE_VIDEO = 0,
    {This media stream is an audio stream.}
    AC_STREAM_TYPE_AUDIO = 1
  );
  
  {Defines the type of an Acinerella media decoder.}
  TAc_decoder_type = (
    {This decoder is used to decode a video stream.}
    AC_DECODER_TYPE_VIDEO = 0,
    {This decoder is used to decode an audio stram.}
    AC_DECODER_TYPE_AUDIO = 1
  );
  
  {Defines the format video/image data is outputted in}
  TAc_output_format = (
    AC_OUTPUT_RGB24 = 0,
    AC_OUTPUT_BGR24 = 1,
    AC_OUTPUT_RGBA32 = 2,
    AC_OUTPUT_BGRA32 = 3
  );

  {TAc_instance represents an Acinerella instance. Each instance can open and
   decode one file at once. There can be only 26 Acinerella instances opened at
   once.}
  TAc_instance = record
    {If true, the instance currently opened a media file.}
    opened: boolean;
    {Contains the count of streams the media file has. This value is available
     after calling the ac_open function.}
    stream_count: integer;
    {Set this value to change the image output format}
    output_format: TAc_output_format;
  end;         
  {Pointer on the Acinerella instance record.}
  PAc_instance = ^TAc_instance;
  
  {Contains information about an Acinerella audio stream.}
  TAc_audio_stream_info = record
    {Samples per second. Default values are 44100 or 48000.}
    samples_per_second: integer;
    {Bits per sample. Can be 8 or 16 Bit.}
    bit_depth: integer;
    {Count of channels in the audio stream.}
    channel_count: integer;
  end;
  
  {Contains information about an Acinerella video stream.}
  TAc_video_stream_info = record
    {The width of one frame.}
    frame_width: integer;
    {The height of one frame.}
    frame_height: integer;
    {The width of one pixel. 1.07 for 4:3 format, 1,42 for the 16:9 format}
    pixel_aspect: single;
    {Frames per second that should be played.}
    frames_per_second: double;
  end;

  {Contains additional info about the video stream. Use "video_info" when the stream
     is an video stream, "audio_info" when the stream is an audio stream.}
  TAd_additional_info = record
    case byte of
      0: (video_info: TAc_video_stream_info);
      1: (audio_info: TAc_audio_stream_info);
  end;

  {Contains information about an Acinerella stream.}
  TAc_stream_info = record
    {Contains the type of the stream.}
    stream_type: TAc_stream_type;
    {Additional info about the stream}
    additional_info: TAd_additional_info;
  end;
  {Pointer on TAc_stream_info}
  PAc_stream_info = ^TAc_stream_info;

  {Contains information about an Acinerella video/audio decoder.}
  TAc_decoder = record
    {Pointer on the Acinerella instance}
    pAcInstance: pAc_instance;
    {Contains the type of the decoder.}
    dec_type: TAc_decoder_type;

    {The timecode of the currently decoded picture in seconds.}
    timecode: double;

    {Contains information about the stream the decoder is attached to.}
    stream_info: TAc_stream_info;
    {The index of the stream the decoder is attached to.}
    stream_index: integer;

    {Pointer to the buffer which contains the data.}
    buffer: PByte;
    {Size of the data in the buffer.}
    buffer_size: integer;
  end;
  {Pointer on TAc_decoder.}
  PAc_decoder = ^TAc_decoder;

  {Contains information about an Acinerella package.}
  TAc_package = record
    {The data of the package. This data may not be accessible, because
     currently FFMpeg doesn't reserve this memory area using the Acinerella
     memory manager.}
    data: PByte;
    {The size of the package data.}
    size: integer;
    {The stream the package belongs to.}
    stream_index: integer;
  end;
  {Pointer on TAc_package}
  PAc_package = ^TAc_package;

  {Callback function used to ask the application to read data. Should return
   the number of bytes read or an value smaller than zero if an error occured.}
  TAc_read_callback = function(sender: Pointer; buf: PByte; size: integer): integer; cdecl;

  {Callback function used to ask the application to seek. return 0 if succeed , -1 on failure.}
  TAc_seek_callback = function(sender: Pointer; pos: int64; whence: integer): int64; cdecl;

  {Callback function that is used to notify the application when the data stream
   is opened or closed. For example the file pointer should be resetted to zero
   when the "open" function is called.}
  TAc_openclose_callback = function(sender: Pointer): integer; cdecl;

{Initializes an Acinerella instance.}
function ac_init(): PAc_instance; cdecl; external ac_dll;
{Frees an Acinerella instance.}
procedure ac_free(inst: PAc_instance); cdecl; external ac_dll;

{Opens a media file.
 @param(inst specifies the Acinerella Instance the stream should be opened for)
 @param(sender specifies a pointer that is sent to all callback functions to
  allow you to do object orientated programming. May be NULL.)
 @param(open_proc specifies the callback function that is called, when the
  media file is opened. May be NULL.)
 @param(close_proc specifies the callback function that is called when the media
  file is closed. May be NULL.)}
function ac_open(
  inst: PAc_instance;
  sender: Pointer;
  open_proc: TAc_openclose_callback;
  read_proc: TAc_read_callback;
  seek_proc: TAc_seek_callback;
  close_proc: TAc_openclose_callback): integer; cdecl; external ac_dll;

{Closes an opened media file.}
procedure ac_close(inst: PAc_instance);cdecl; external ac_dll;

{Stores information in "pInfo" about stream number "nb".}
procedure ac_get_stream_info(
  inst: PAc_instance; nb: integer; pinfo: PAc_stream_info); cdecl; external ac_dll;

{Reads a package from an opened media file.}
function ac_read_package(inst: PAc_instance): PAc_package; cdecl; external ac_dll;
{Frees a package that has been read.}
procedure ac_free_package(package: PAc_package); cdecl; external ac_dll;

{Creates an decoder for the specified stream number. Returns NIL if no decoder
 could be found.}
function ac_create_decoder(pacInstance: PAc_instance; nb: integer): PAc_decoder; cdecl; external ac_dll;
{Frees an created decoder.}
procedure ac_free_decoder(pDecoder: PAc_decoder); cdecl; external ac_dll;
{Decodes a package using the specified decoder. The decodec data is stored in the
 "buffer" property of the decoder.}
function ac_decode_package(pPackage: PAc_package; pDecoder: PAc_decoder): integer; cdecl; external ac_dll;


implementation

{Connect the library memory management to the host application. This happens
 automatically when the application gets initialized and nobody has to care
 about it.}

function ac_mem_mgr(
  ptr_malloc: Pointer;
  ptr_realloc: Pointer;
  ptr_free: Pointer): PAc_instance; cdecl; external ac_dll;

function malloc(size: integer): Pointer; cdecl;
begin
  result := GetMemory(size);
end;

function realloc(ptr: Pointer; size: integer): pointer; cdecl;
begin
  result := ReallocMemory(ptr, size);
end;

procedure free(ptr: Pointer); cdecl;
begin
  FreeMemory(ptr);
end;

initialization
  ac_mem_mgr(@malloc, @realloc, @free);      

end.


