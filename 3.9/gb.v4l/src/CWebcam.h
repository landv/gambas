/***************************************************************************

  CWebcam.h

  (C) 2005-2008 Daniel Campos Fernández <dcamposf@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#ifndef __CWEBCAM_H
#define __CWEBCAM_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "config.h"
#include <linux/videodev2.h>
#include <libv4lconvert.h>

#ifdef OS_LINUX
  #include <linux/types.h>
  #include "videodev.h"
#else
  #include <linux/videodev.h>
#endif

#include "gambas.h"

#ifndef __CWEBCAM_C

extern GB_DESC CWebcamDesc[];
extern GB_DESC CTunerDesc[];
extern GB_STREAM_DESC VideoStream;

#else

#define THIS ((CWEBCAM *)_object)
#define DEVICE (THIS->dev)
// ++ V4L2
#define MCLEAR(x) memset (&(x), 0, sizeof (x))
#define MODE_ANY 0
#define MODE_V4L 1
#define MODE_V4L2 2
// --
#endif

typedef 
	struct video_device 
	{
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
	}
	video_device_t;


typedef 
	struct  
	{
		GB_STREAM_BASE stream;
		void *handle;
	}
	VIDEO_STREAM;

// ++ V4L2 
typedef 
	struct gv4l2_buffer
	{
		void*   start;
		size_t  length;
	} 
	gv4l2_buffer_t;
//--

typedef 
	struct
	{
		GB_BASE ob;
		GB_STREAM stream;

		char *device;
		video_device_t *dev;
		unsigned char *membuf;
		int gotframe;
		int posframe;

		// ++ YUYV->RGB conversion
		void *frame;		// "current" frame buffer
		//--
		// ++ V4L2 
		//
		// There is some duplication here but we really don't want to use
		// the v4l video_device_t structure ...
		//
		struct v4l2_capability cap;
		struct v4l2_cropcap cropcap;
		struct v4l2_crop crop;
		struct v4l2_format fmt;
		struct gv4l2_buffer *buffers;
		//
		int is_v4l2;	// which version is this dev
		int io;		// raw device handle for V2
		int use_mmap;	// is MMAP available
		int buffer_count;	// number of buffers
		int w, h;		// "current" dimensions
		int format; // gb.image format
		//
		int bright_max;
		int hue_max;
		int contrast_max;
		int whiteness_max;
		int color_max;
		//
		int bright_min;
		int hue_min;
		int contrast_min;
		int whiteness_min;
		int color_min;
		//
		int bright_def;
		int hue_def;
		int contrast_def;
		int whiteness_def;
		int color_def;
		// --
		struct v4lconvert_data *convert;
	}
	CWEBCAM;


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


// ++ YUYV->RGB conversion
int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
void yuv420p_to_rgb (unsigned char *image, unsigned char *image2, int x, int y, int z);
// --

// ++ V4L2 
int 		gv4l2_available(CWEBCAM * _object);
void 		gv4l2_debug(const char *s);
int 		gv4l2_xioctl( int fd,int request,void * arg);
int 		gv4l2_open_device( char* name );
void 		gv4l2_close_device( int id );
int 		gv4l2_init_device(CWEBCAM * _object , int width , int height );
int 		gv4l2_start_capture(CWEBCAM * _object);
int 		gv4l2_stop_capture(CWEBCAM * _object);
void 		gv4l2_uninit_device(CWEBCAM * _object);
void 		gv4l1_process_image (CWEBCAM * _object, void *start);
void 		gv4l2_process_image (CWEBCAM * _object, void *start);
int 		gv4l2_read_frame( CWEBCAM * _object );
int 		gv4l2_resize( CWEBCAM * _object , int width , int height );
int 		gv4l2_hue( CWEBCAM * _object , int hue );
int 		gv4l2_brightness( CWEBCAM * _object , int hue );
int 		gv4l2_contrast( CWEBCAM * _object , int value );
int 		gv4l2_color( CWEBCAM * _object , int value );
int 		gv4l2_whiteness( CWEBCAM * _object , int value );
// -- V4L2

#endif
