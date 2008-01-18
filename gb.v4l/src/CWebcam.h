/***************************************************************************

  CWebcam.h

  Webcam capture component

  (C) 2005-2008 Daniel Campos Fernández <dcamposf@gmail.com>

  Based on the GPL code from "video-capture":

      (#) video-capture.c - modified from vidcat.c
      Copyright (C) 1998 Rasca, Berlin
      EMail: thron@gmx.de
      Modifications (C) 2001, Nick Andrew <nick@nick-andrew.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#ifndef __CWEBCAM_H
#define __CWEBCAM_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <linux/videodev2.h>

#include "gambas.h"

#ifndef __CWEBCAM_C

extern GB_DESC CWebcamDesc[];
extern GB_DESC CTunerDesc[];
extern GB_DESC CFeaturesDesc[];
extern GB_STREAM_DESC VideoStream;

#else

#define THIS ((CWEBCAM *)_object)
#define DEVICE (THIS->dev)

#endif

typedef struct	video_device {
	int	width;
	int	height;
	int	depth;		// colour depth
	int	buffer_size;	// always width * height * depth
	int	use_mmap;	// mmap() available for capturing a frame
	int	capturing;	// our device is capturing frames for us

	struct video_capability vcap;
	struct video_channel vchan;
	struct video_mbuf vmbuf;
	struct video_mmap vmmap;
	struct video_window vwin;
	struct video_picture videopict;

	unsigned char	*frame_buffer;	// not the video memory, but one image
	int	dev;		// fd of the physical device
	int Freq2;

} video_device_t;


typedef struct  
{
    GB_STREAM_DESC *desc;
    int _reserved;
    void *handle;
}  VIDEO_STREAM;

typedef  struct
{
	GB_BASE ob;
	GB_STREAM stream;

	char *device;
	video_device_t *dev;
	unsigned char *membuf;
	long gotframe;
	long posframe;

}  CWEBCAM;


int Video_stream_read(GB_STREAM *stream, char *buffer, int len);
int Video_stream_write(GB_STREAM *stream, char *buffer, int len);
int Video_stream_eof(GB_STREAM *stream);
int Video_stream_lof(GB_STREAM *stream, int64_t *len);
int Video_stream_open(GB_STREAM *stream, const char *path, int mode, void *data);
int Video_stream_seek(GB_STREAM *stream, int64_t pos, int whence);
int Video_stream_tell(GB_STREAM *stream, int64_t *pos);
int Video_stream_flush(GB_STREAM *stream);
int Video_stream_close(GB_STREAM *stream);
int Video_stream_handle(GB_STREAM *stream);

#endif
