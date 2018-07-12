{*
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
 *}

program acinerella_demo;

{$APPTYPE CONSOLE}

{$IFDEF FPC}
  {$MODE DELPHI}
{$ENDIF}

uses
  {$IFDEF FPC}Interfaces,{$ENDIF}{$IFDEF WIN32}Windows,{$ENDIF}Forms, Graphics, Classes, SysUtils, acinerella;

type
  TWAVHdr = packed Record
    riff: array[0..3] of Char;
    len: DWord;
    cWavFmt: array[0..7] of Char;
    dwHdrLen: DWord;
    wFormat: Word;
    wNumChannels: Word;
    dwSampleRate: DWord;
    dwBytesPerSec: DWord;
    wBlockAlign: Word;
    wBitsPerSample: Word;
    cData: array[0..3] of Char;
    dwDataLen: DWord;
  end;

var
  inst: PAc_instance;
  pack: PAc_package;
  wave: TFileStream;
  wave_hdr: TWAVHdr;
  info: TAc_stream_info;
  audiodecoder: PAc_decoder;
  videodecoder: PAc_decoder;
  i: integer;
  frm: TForm;
  bmp: TBitmap;
  fs: TFileStream;
  read_cnt: integer;

function read_proc(sender: Pointer; buf: PChar; size: integer): integer; cdecl;
begin
  inc(read_cnt);
  result := fs.Read(buf^, size);
end;

function seek_proc(sender: Pointer; pos: int64; whence: integer): int64; cdecl;
begin
  result := fs.Seek(pos, TSeekOrigin(whence))
end;

begin
  videodecoder := nil;
  audiodecoder := nil;
  wave := nil;

  if not FileExists(ParamStr(1)) then
  begin
    Writeln('Source file not specified. Simply drag and drop a video or an audio ' +
      'file on the executable. Press return to close the program.');
    Readln;
    halt;
  end;

  Application.Initialize;
  frm := TForm.Create(nil);

  bmp := TBitmap.Create;
  bmp.PixelFormat := pf24Bit;

  fs := TFileStream.Create(ParamStr(1), fmOpenRead);
  fs.Position := 0;

  Writeln('Acinerella Pascal Test Program');
  Writeln('------------------------------');
  Writeln;

  inst := ac_init();
  ac_open(inst, nil, nil, @read_proc, nil, nil, nil);

  Writeln('Count of Datastreams: ', inst^.stream_count);
  Writeln('Length of the file: ', inst^.info.duration);
  Writeln('Title: ', PChar(@(inst^.info.title[0])));
  Writeln('Author: ', PChar(@(inst^.info.author[0])));
  Writeln('Album: ', PChar(@(inst^.info.album[0])));
  Writeln('Genre: ', PChar(@(inst^.info.genre[0])));
  for i := 0 to inst^.stream_count - 1 do
  begin
    Writeln;
    ac_get_stream_info(inst, i, @info);
    Writeln('Information about stream ', i, ':');
    case info.stream_type of
      AC_STREAM_TYPE_AUDIO:
      begin
        Writeln('Stream is an audio stream.');
        Writeln('--------------------------');
        Writeln;
        Writeln(' * Samples per Second: ', info.additional_info.audio_info.samples_per_second);
        Writeln(' * Channel count     : ', info.additional_info.audio_info.channel_count);
        Writeln(' * Bit depth         : ', info.additional_info.audio_info.bit_depth);

        if audiodecoder = nil then
        begin
          audiodecoder := ac_create_decoder(inst, i);
          wave := TFileStream.Create(ExtractFilePath(ParamStr(0))+'out.wav', fmCreate);

          with info.additional_info.audio_info do
          begin
            wave_hdr.riff := 'RIFF';
            wave_hdr.len := 36;
            wave_hdr.cWavFmt := 'WAVEfmt ';
            wave_hdr.dwHdrLen := 16;
            wave_hdr.wFormat := 1;
            wave_hdr.wNumChannels := channel_count;
            wave_hdr.dwSampleRate := samples_per_second;
            wave_hdr.wBlockAlign := (channel_count * bit_depth) div 8;
            wave_hdr.dwBytesPerSec := (samples_per_second * bit_depth * channel_count) div 8;
            wave_hdr.wBitsPerSample := bit_depth;
            wave_hdr.cData := 'data';
            wave_hdr.dwDataLen := 0; //!Unknown
          end;

          wave.Write(wave_hdr, SizeOf(wave_hdr));
        end;
      end;

      AC_STREAM_TYPE_VIDEO:
      begin
        Writeln('Stream is an video stream.');
        Writeln('--------------------------');
        Writeln;
        Writeln(' * Width             : ', info.additional_info.video_info.frame_width, 'px');
        Writeln(' * Height            : ', info.additional_info.video_info.frame_height, 'px');
        Writeln(' * Pixel aspect      : ', FormatFloat('#.##', info.additional_info.video_info.pixel_aspect));
        Writeln(' * Frames per second : ', FormatFloat('#.##', info.additional_info.video_info.frames_per_second));

        if videodecoder = nil then
        begin
          videodecoder := ac_create_decoder(inst, i);
          bmp.Height := videodecoder^.stream_info.additional_info.video_info.frame_height;
          bmp.Width := videodecoder^.stream_info.additional_info.video_info.frame_width;
          frm.ClientWidth :=
            round(bmp.Width * videodecoder^.stream_info.additional_info.video_info.pixel_aspect);
          frm.ClientHeight := bmp.Height;
          frm.Show;
        end;
      end;
    end;
  end;

  Writeln;

  if not inst^.opened then
  begin
    Writeln('No video/audio information found. Press return to leave.');
    Readln;
    exit;
  end;

  repeat
    Application.ProcessMessages;

    pack := ac_read_package(inst);
    if pack <> nil then
    begin
      if (videodecoder <> nil) and (videodecoder^.stream_index = pack^.stream_index) then
      begin
        if (ac_decode_package(pack, videodecoder) > 0) then
        begin
          //This demo uses the GDI to draw the video data - this is a very simple
          //way but produces a very bad quality. You should use a video overlay or
          //OpenGL/Direct3D to draw your video data.
          {$IFNDEF FPC}
          Move(videodecoder^.buffer^, bmp.Scanline[bmp.Height-1]^, videodecoder^.buffer_size);
          SetStretchBltMode(frm.Canvas.Handle, HALFTONE); 
          StretchBlt(frm.Canvas.Handle, 0, frm.Height, frm.Width, -frm.Height, bmp.Canvas.Handle,
            0, 0, bmp.Width, bmp.Height, SRCCOPY);
          {$ENDIF}
        end;
      end;
      if (audiodecoder <> nil) and (audiodecoder^.stream_index = pack^.stream_index) then
        if (ac_decode_package(pack, audiodecoder) > 0) then
          wave.Write(audiodecoder^.buffer^, audiodecoder^.buffer_size);

      ac_free_package(pack);
    end;
  until (pack = nil) or ((not frm.Visible) and (videodecoder <> nil));

  if videodecoder <> nil then
    ac_free_decoder(videodecoder);

  if audiodecoder <> nil then
    ac_free_decoder(audiodecoder);

  ac_close(inst);

  ac_free(inst);

  fs.Free;

  if wave <> nil then
  begin
    wave_hdr.len := wave.Size;
    wave_hdr.dwDataLen := wave.Size - sizeof(wave_hdr);

    wave.Position := 0;
    wave.Write(wave_hdr, sizeof(wave_hdr));

    wave.Free;
  end;      

  frm.Free;
  bmp.Free;
end.
