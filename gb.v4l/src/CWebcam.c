/***************************************************************************

  CWebcam.c

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

#define __CWEBCAM_C

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <png.h>
#include <jpeglib.h>

#ifdef HAVE_STDLIB_H
#undef HAVE_STDLIB_H
#endif

#include "main.h"
#include "CWebcam.h"

#define DEF_WIDTH	320
#define DEF_HEIGHT	240
// default colour depth (changing this will break everything)
#define DEF_DEPTH	3

#define FMT_UNKNOWN	0
#define FMT_PPM		1
#define FMT_PNG		2
#define FMT_JPEG	3
#define FMT_DEFAULT	2

#define IN_TV			0
#define IN_COMPOSITE1	1
#define IN_COMPOSITE2	2
#define IN_SVIDEO		3
#define IN_DEFAULT		1

#define NORM_PAL		0
#define NORM_NTSC		1
#define NORM_SECAM		2
#define NORM_AUTO		3
#define NORM_DEFAULT	0

#define QUAL_DEFAULT	80

GB_STREAM_DESC VideoStream = {
	open: Video_stream_open,
	close: Video_stream_close,
	read: Video_stream_read,
	write: Video_stream_write,
	seek: Video_stream_seek,
	tell: Video_stream_tell,
	flush: Video_stream_flush,
	eof: Video_stream_eof,
	lof: Video_stream_lof,
	handle: Video_stream_handle
};

extern 	int gv4l2_debug_mode; // ++

/***********************************************************************************

 Camera setup

************************************************************************************/
int	vd_ioctl(video_device_t *vd, int cmd, void *arg)
{
	return ioctl(vd->dev, cmd, arg);
}

void vd_close(video_device_t *vd)
{

	if (vd->frame_buffer)
	{
		if (vd->use_mmap) munmap(vd->frame_buffer, vd->vmbuf.size);
		else GB.Free(POINTER(&vd->frame_buffer));
	}
	close(vd->dev);
}

video_device_t	*vd_setup(int width, int height, int depth, int dev)
{
	video_device_t *vd;

	GB.Alloc(POINTER(&vd),sizeof(video_device_t));

	vd->width = width;
	vd->height = height;
	vd->depth = depth;
	vd->buffer_size = width * height * depth;
	vd->dev = dev;
	vd->use_mmap = 0;
	vd->capturing = 0;
	vd->frame_buffer=NULL;

	return vd;
}

int	vd_get_capabilities(video_device_t *vd)
{
	if (vd_ioctl(vd, VIDIOCGCAP, &vd->vcap)) return 0;

	if (!(vd->vcap.type & VID_TYPE_CAPTURE)) vd->use_mmap = 0;
	else  vd->use_mmap = 1;

	if (vd->width > vd->vcap.maxwidth) vd->width=vd->vcap.maxwidth;
	if (vd->width < vd->vcap.minwidth) vd->width=vd->vcap.minwidth;
	if (vd->height > vd->vcap.maxheight) vd->height=vd->vcap.maxheight;
	if (vd->height < vd->vcap.minheight) vd->height=vd->vcap.minheight;

	return 1;
}


// -- int	vd_setup_capture_mode(video_device_t *vd)

int	vd_setup_capture_mode(CWEBCAM * _object) // ++
{
	video_device_t *vd = DEVICE;

	if (!vd_get_capabilities(vd)) return 0;

	// See if we can use mmap (to avoid copying data around)
	// VIDIOCGMBUF tells us how many frames it is going to buffer
	// for us, and we have to use them all!!! ???
	if (vd_ioctl(vd, VIDIOCGMBUF, &vd->vmbuf))
	{
		if (vd->use_mmap)
		{
			if (vd->frame_buffer)
			{
				munmap(vd->frame_buffer, vd->vmbuf.size);
				vd->frame_buffer=NULL;
			}
			vd->use_mmap = 0;
		}

		// Issue VIDIOCGWIN to tell the driver what geometry we
		// expect from read()
		if (!vd_ioctl(vd, VIDIOCGWIN, &vd->vwin))
		{
			vd->vwin.width = vd->width;
			vd->vwin.height = vd->height;
			if (vd_ioctl(vd, VIDIOCSWIN, &vd->vwin)) return 0;
			if (vd_ioctl(vd, VIDIOCSWIN, &vd->vwin)) return 0;
			vd->buffer_size = vd->height * vd->width;
		}

		if (vd->frame_buffer) GB.Free(POINTER(&vd->frame_buffer));
		if (THIS->frame)      GB.Free(POINTER(&THIS->frame)); // ++
		GB.Alloc(POINTER(&vd->frame_buffer),vd->buffer_size);
		GB.Alloc(POINTER(&THIS->frame),vd->height * vd->width * 4); // ++
		return 1;
	}

	// mmap is okay!
	if (!vd->use_mmap)
	{
		if (vd->frame_buffer) GB.Free(POINTER(&vd->frame_buffer));
		vd->use_mmap = 1;
	}

	vd->frame_buffer = mmap(0, vd->vmbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, vd->dev, 0);
	vd->vmmap.format = VIDEO_PALETTE_RGB24;		// KLUDGE ...
	vd->vmmap.frame = 0;				// Start at frame 0
	vd->vmmap.width = vd->width;
	vd->vmmap.height = vd->height;

	if (THIS->frame)      GB.Free(POINTER(&THIS->frame)); // ++
	GB.Alloc(&THIS->frame, vd->height * vd->width * 4); // ++

        ioctl(vd->dev, VIDIOCGPICT, &vd->videopict); //++ Recover camera palette
        vd->vmmap.format = vd->videopict.palette;    //++ Save for future ref
	return 1;
}

int	vd_setup_video_source(video_device_t *vd, int input, int norm) {

	vd->vchan.channel = input;	// Query desired channel

	if (vd_ioctl(vd, VIDIOCGCHAN, &vd->vchan)) return 0;

	// Now set the channel and the norm for this channel

	vd->vchan.norm = norm;
	if (vd_ioctl(vd, VIDIOCSCHAN, &vd->vchan)) return 0;

	// KLUDGE ... the API leaves colour settings and tuning undefined
	// after a channel change
	return 1;
}

/***********************************************************************************

 Image capture

************************************************************************************/

void put_image_jpeg (char *image, int width, int height, int quality, int frame,FILE *fd)
{

	int y, x, line_width;
	JSAMPROW row_ptr[1];
	struct jpeg_compress_struct cjpeg;
	struct jpeg_error_mgr jerr;
	char *line;

	GB.Alloc( POINTER(&line) ,width * 3);
	if (!line)
		return;
	cjpeg.err = jpeg_std_error(&jerr);
	jpeg_create_compress (&cjpeg);
	cjpeg.image_width = width;
	cjpeg.image_height= height;
	cjpeg.input_components = 3;
	cjpeg.in_color_space = JCS_RGB;
	jpeg_set_defaults (&cjpeg);

	jpeg_set_quality (&cjpeg, quality, TRUE);
	cjpeg.dct_method = JDCT_FASTEST;
	jpeg_stdio_dest (&cjpeg, fd);

	jpeg_start_compress (&cjpeg, TRUE);

	row_ptr[0] = (JSAMPROW)line;
	line_width = width * 3;
	for ( y = 0; y < height; y++) {
	for (x = 0; x < line_width; x+=3) {
			line[x]   = image[x+2];
			line[x+1] = image[x+1];
			line[x+2] = image[x];
		}
		jpeg_write_scanlines (&cjpeg, row_ptr, 1);
		image += line_width;
	}
	jpeg_finish_compress (&cjpeg);
	jpeg_destroy_compress (&cjpeg);
	GB.Free( POINTER(&line) );

}

void put_image_png (char *image, int width, int height, int frame,FILE *fd)
{

	int y;
	char *p;
	png_infop info_ptr;
	png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
						NULL, NULL, NULL);
	if (!png_ptr)
		return;
	info_ptr = png_create_info_struct (png_ptr);
	if (!info_ptr)
		return;

	png_init_io (png_ptr, fd);
	png_set_IHDR (png_ptr, info_ptr, width, height,
					8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_bgr (png_ptr);
	png_write_info (png_ptr, info_ptr);
	p = image;
	for (y = 0; y < height; y++) {
		png_write_row (png_ptr, (unsigned char*)p);
		p+=width*3;
	}
	png_write_end (png_ptr, info_ptr);

}

/*
 * write ppm image to stdout
 */
#define WRITE_MODE	"w"

void put_image_ppm_buffer (char *image, int width, int height, int frame,long *len,void *_object)
{
	int x;
	int htot=width*height;
	unsigned char *p = (unsigned char *)image;
	unsigned char	*bp;

	*len=(3 * htot)+15;
	if (!THIS->membuf)	GB.Alloc(POINTER(&THIS->membuf),((*len)*sizeof(unsigned char*)));
	sprintf((char*)THIS->membuf, "P6\n%d %d\n%d\n", width, height, 255);
	bp=THIS->membuf+strlen((const char*)THIS->membuf);
	for (x = 0; x < htot; x++) {
		*bp++ = p[2];
		*bp++ = p[1];
		*bp++ = p[0];
		p += 3;
	}


}

void put_image_ppm (char *image, int width, int height, int binary, int frame,FILE *out_fp)
{
	int x, y, ls=0;
	unsigned char *p = (unsigned char *)image;



	if (!binary)
	{
		fprintf(out_fp, "P3\n%d %d\n%d\n", width, height, 255);
		for (x = 0; x < width; x++)
		{
			for (y = 0; y < height; y++)
			{
				fprintf(out_fp, "%03d %03d %03d  ", p[2], p[1], p[0]);
				p += 3;
				if (ls++ > 4)
				{
					fprintf(out_fp, "\n");
					ls = 0;
				}
			}
		}
		fprintf(out_fp, "\n");

	}
	else
	{
		unsigned char	buff[3 * width * height];
		unsigned char	*bp = buff;

		fprintf(out_fp, "P6\n%d %d\n%d\n", width, height, 255);
		for (x = 0; x < width * height; x++) {
			*bp++ = p[2];
			*bp++ = p[1];
			*bp++ = p[0];
			p += 3;
		}
		fwrite(buff, width * height, 3, out_fp);
	}

}



//unsigned char *	vd_get_image(video_device_t *vd)

unsigned char *	vd_get_image(CWEBCAM * _object)
{
	int	len;
	video_device_t *vd;

	vd = DEVICE;

	if (vd->use_mmap) {
		if (!vd->capturing) {

			int i;
			// Queue requests to capture successive frames
			for (i = 0; i < vd->vmbuf.frames; ++i) {
				vd->vmmap.frame = i;
				if(vd_ioctl(vd, VIDIOCMCAPTURE, &vd->vmmap))
					return 0;
			}
			// And start reading from zero
			vd->vmmap.frame = 0;
			vd->capturing = 1;
		}
		// VIDIOCSYNC causes the driver to block until the specified
		// frame is completely received
		if (ioctl(vd->dev, VIDIOCSYNC, &vd->vmmap.frame)) return 0;
		gv4l1_process_image (THIS,vd->frame_buffer + vd->vmbuf.offsets[vd->vmmap.frame]);

		//vd_post_process(vd,vd->frame_buffer + vd->vmbuf.offsets[vd->vmmap.frame]);
		return THIS->frame;

		// Return the buffer, cause it should contain an image
		//return vd->frame_buffer + vd->vmbuf.offsets[vd->vmmap.frame];
	}

	// Otherwise, we have to read the right number of bytes

	len = read(vd->dev, vd->frame_buffer, vd->buffer_size);
	if (len <= 0) {
		return 0;
	}

	if (len != vd->buffer_size) return 0;

	return vd->frame_buffer;
}

int	vd_image_done(video_device_t *vd)
{
	if (vd->use_mmap)
		{
		// vd->vmmap.frame contains the index of the recently-used buffer
		// So tell the driver to reuse this one for the next frame

		if (ioctl(vd->dev, VIDIOCMCAPTURE, &vd->vmmap)) return 0;

		// Now cycle the frame number, so we sync the next frame
		if (++vd->vmmap.frame >= vd->vmbuf.frames) {
			vd->vmmap.frame = 0;
		}
	}

	return 1;
}
/***********************************************************************************

Stream interface

************************************************************************************/
int fill_buffer(void *_object)
{
	char *buf;
	int w,h;

	// -- buf=(char*)vd_get_image(DEVICE);
	buf=(char*)vd_get_image(THIS); // ++
	if (!buf) return -1;
	w=DEVICE->vmmap.width;
	h=DEVICE->vmmap.height;
	vd_image_done(DEVICE);
	put_image_ppm_buffer (buf,w,h,0,&THIS->gotframe,_object);
	THIS->posframe=0;
	return 0;
}

int Video_stream_read(GB_STREAM *stream, char *buffer, int len)
{
	void *_object=(void*)((VIDEO_STREAM*)stream)->handle;

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe)
		if ( fill_buffer(_object) ) return -1;

	if ((len+THIS->posframe)>THIS->gotframe) return -1;
	memcpy (buffer,THIS->membuf+THIS->posframe,len);
	THIS->posframe+=len;
	return 0;
}

int Video_stream_eof(GB_STREAM *stream)
{
	void *_object=(void*)((VIDEO_STREAM*)stream)->handle;

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe) return 0;

	if (THIS->gotframe<=THIS->posframe) return -1;
	return 0;
}

int Video_stream_lof(GB_STREAM *stream, int64_t *len)
{
	void *_object=(void*)((VIDEO_STREAM*)stream)->handle;

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe)
		if ( fill_buffer(_object) ) return -1;

	*len=(long long)THIS->gotframe;
	return 0;
}

int Video_stream_seek(GB_STREAM *stream, int64_t pos, int whence)
{
	void *_object=(void*)((VIDEO_STREAM*)stream)->handle;

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe)
		if ( fill_buffer(_object) ) return -1;

	if (pos<0) return -1;
	THIS->posframe=pos;
	return 0;
}

int Video_stream_tell(GB_STREAM *stream, int64_t *pos)
{
	void *_object=(void*)((VIDEO_STREAM*)stream)->handle;

	if (!_object) return -1;
	if (!DEVICE) return -1;

	*pos=(long long)THIS->posframe;
	return 0;

}

int Video_stream_flush(GB_STREAM *stream)
{
	void *_object=(void*)((VIDEO_STREAM*)stream)->handle;

	if (!_object) return -1;
	if (!DEVICE) return -1;

	THIS->gotframe=0;
	THIS->posframe=0;
	return 0;
}

int Video_stream_open(GB_STREAM *stream, const char *path, int mode, void *data)
{
	return -1;
}

int Video_stream_close(GB_STREAM *stream)
{
	return -1;
}

int Video_stream_write(GB_STREAM *stream, char *buffer, int len)
{
	return -1;
}

int Video_stream_handle(GB_STREAM *stream)
{
	return 0;
}

/***********************************************************************************

 Gambas interface

************************************************************************************/
int CWEBCAM_check(void *_object)
{
	//if((!DEVICE)&&(!THIS->is_v4l2)) return TRUE;
	if(!THIS->device) return TRUE;	// ++ V4L2
  	return FALSE;
}

BEGIN_PROPERTY(CWEBCAM_contrast)

	if( !THIS->is_v4l2 ) {

		vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
		if (READ_PROPERTY)
		{
			GB.ReturnInteger(DEVICE->videopict.contrast);
			return;
		}

		DEVICE->videopict.contrast=VPROP(GB_INTEGER);
		vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);
		return;
	}
	if (READ_PROPERTY)
		GB.ReturnInteger(gv4l2_contrast(THIS,-1));
	else	gv4l2_contrast(THIS,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_contrast_max)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->contrast_max);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_contrast_min)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->contrast_min);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_colour)

	if( !THIS->is_v4l2 ) {

		vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
		if (READ_PROPERTY)
		{
			GB.ReturnInteger(DEVICE->videopict.colour);
			return;
		}
	
		DEVICE->videopict.colour=VPROP(GB_INTEGER);
		vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);
		return;
	}
	if (READ_PROPERTY)
		GB.ReturnInteger(gv4l2_color(THIS,-1));
	else	gv4l2_color(THIS,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_color_max)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->color_max);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_color_min)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->color_min);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_whiteness)

	if( !THIS->is_v4l2 ) {

		vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
		if (READ_PROPERTY)
		{
			GB.ReturnInteger(DEVICE->videopict.whiteness>>8);
			return;
		}
	
		DEVICE->videopict.whiteness=VPROP(GB_INTEGER);
		vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);
		return;
	}
	if (READ_PROPERTY)
		GB.ReturnInteger(gv4l2_whiteness(THIS,-1));
	else	gv4l2_whiteness(THIS,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_whiteness_max)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->whiteness_max);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_whiteness_min)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->whiteness_min);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_hue)

	if( !THIS->is_v4l2 ) {

	        vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	        if (READ_PROPERTY)
	        {
	                GB.ReturnInteger(DEVICE->videopict.hue>>8);
	                return;
	        }
	
	        DEVICE->videopict.hue=VPROP(GB_INTEGER);
	        vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);
		return;
	}
	if (READ_PROPERTY)
		GB.ReturnInteger(gv4l2_hue(THIS,-1));
	else	gv4l2_hue(THIS,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_hue_max)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->hue_max);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_hue_min)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->hue_min);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_bright)

	if( !THIS->is_v4l2 ) {

		vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
		if (READ_PROPERTY)
		{
			GB.ReturnInteger(DEVICE->videopict.brightness);
			return;
		}
	
		DEVICE->videopict.brightness=VPROP(GB_INTEGER);
		vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);
		return;
	}
	if (READ_PROPERTY)
		GB.ReturnInteger(gv4l2_brightness(THIS,-1));
	else	gv4l2_brightness(THIS,VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_bright_max)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->bright_max);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_bright_min)

	if( !THIS->is_v4l2 )
		GB.ReturnInteger(65535);
	else	GB.ReturnInteger(THIS->bright_min);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_width)

	if( THIS->is_v4l2 )	// ++
		GB.ReturnInteger(THIS->fmt.fmt.pix.width);
	else	GB.ReturnInteger(DEVICE->width);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_height)

	if( THIS->is_v4l2 )	// ++
		GB.ReturnInteger(THIS->fmt.fmt.pix.height);
	else	GB.ReturnInteger(DEVICE->height);

END_PROPERTY
//
//=============================================================================
//
//	_new( device name)
//
//	Default constructor
//
BEGIN_METHOD (CWEBCAM_new,GB_STRING Device; GB_INTEGER Compat;)

	int mydev;
	struct video_tuner vtuner;
	VIDEO_STREAM *str;

	// ++ V4L2
	//
	//	Open the device
	//
	mydev = gv4l2_open_device(STRING(Device));
	if( mydev == -1) {
		GB.Error("Unable to open device");
		return;
	}
	THIS->io = mydev;

	if(MISSING(Compat)) 
		THIS->is_v4l2 = gv4l2_available( THIS );
	else
		switch( VARG(Compat) ) {
			case gv4l2_V4L:	
				THIS->is_v4l2 = 0;
				break;
			case gv4l2_V4L2:	
				THIS->is_v4l2 = 1;
				break;
			default:
				close(mydev);
				GB.Error("Invalid compatibility flag");
		}
			

	GB.Alloc(POINTER(&THIS->device),sizeof(char)*(LENGTH(Device)+1)); // ++
	strcpy(THIS->device,STRING(Device));				  // ++

	if( THIS->is_v4l2 ) {
		//
		gv4l2_debug("Device is V4L2!");
		//
		//	Initialise the device
		//
		if( !gv4l2_init_device(THIS,DEF_WIDTH,DEF_HEIGHT) ) {
			close(mydev);
			GB.Error("Unable to initialise the device");
			return;
		}
		//
		THIS->stream.desc=&VideoStream;
		str=(VIDEO_STREAM*)POINTER(&THIS->stream);
		str->handle=(void*)THIS;
		//
		gv4l2_start_capture(THIS);        
		return;
	}

	gv4l2_debug("Device is V4L!");
	// mydev=open (GB.FileName(STRING(Device),LENGTH(Device)),O_RDWR);
	// if (mydev==-1)
	// {
	// 	GB.Error("Unable to open device");
	//	return;
	//}
	// -- V4L2

	DEVICE=vd_setup(DEF_WIDTH,DEF_HEIGHT,DEF_DEPTH,mydev);

//--	if (!vd_setup_capture_mode(DEVICE))
	if (!vd_setup_capture_mode(THIS)) // ++
	{
		close(mydev);
		GB.Free(POINTER(&DEVICE));
		GB.Error("Unable to setup capture mode");
		return;
	}

	vd_setup_video_source(DEVICE,IN_DEFAULT,NORM_DEFAULT);

	// -- GB.Alloc(POINTER(&THIS->device),sizeof(char)*(LENGTH(Device)+1));
	// -- strcpy(THIS->device,STRING(Device));

	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)) DEVICE->Freq2=1;

	THIS->stream.desc=&VideoStream;
	str=(VIDEO_STREAM*)POINTER(&THIS->stream);
	str->handle=(void*)THIS;

END_METHOD
//
//=============================================================================
//
//	_free()
//
//	Default destructor
//
BEGIN_METHOD_VOID(CWEBCAM_free)

	// ++ V4L2
	if (THIS->device) GB.Free(POINTER(&THIS->device));		// ++
	if (THIS->frame)  GB.Free(POINTER(&THIS->frame));		// ++

	if( THIS->is_v4l2 ) {
		gv4l2_stop_capture( THIS );
		gv4l2_uninit_device( THIS );
		gv4l2_close_device( THIS->io );
		return;
	}
	
	// --if (THIS->device) GB.Free(POINTER(&THIS->device));
	// -- V4L2

	if (THIS->membuf) GB.Free(POINTER(&THIS->membuf));

	if (DEVICE)
	{
		vd_close(DEVICE);
		GB.Free(POINTER(&DEVICE));
	}

END_METHOD
//
//=============================================================================
//

BEGIN_METHOD(CWEBCAM_size,GB_INTEGER Width; GB_INTEGER Height;)

	struct video_tuner vtuner;
	long w=VARG(Width);
	long h=VARG(Height);
	int mydev;
	int norm;
	int channel;
	int colour,hue,whiteness,contrast,brightness;

	// ++ V4L2
	if( THIS->is_v4l2 ) {
		gv4l2_resize( THIS , VARG(Width) , VARG(Height) );
		return;
	}
	// -- V4L2

	if (h<DEVICE->vcap.minheight) h=DEVICE->vcap.minheight;
	if (h>DEVICE->vcap.maxheight) h=DEVICE->vcap.maxheight;
	if (w<DEVICE->vcap.minwidth) w=DEVICE->vcap.minwidth;
	if (w>DEVICE->vcap.maxwidth) w=DEVICE->vcap.maxwidth;

	if ( (w==DEVICE->width) && (h==DEVICE->height) ) return;

	norm=DEVICE->vchan.norm;
	channel=DEVICE->vchan.channel;

	vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	hue=DEVICE->videopict.hue;
	contrast=DEVICE->videopict.contrast;
	brightness=DEVICE->videopict.brightness;
	colour=DEVICE->videopict.colour;
	whiteness=DEVICE->videopict.whiteness;

	if (THIS->membuf) GB.Free(POINTER(&THIS->membuf));
	vd_close(DEVICE);
	GB.Free(POINTER(&DEVICE));

	mydev=open(THIS->device,O_RDWR);
	if (mydev==-1)
	{
		GB.Error("Unable to open device");
		return;
	}
	DEVICE=vd_setup(w,h,DEF_DEPTH,mydev);

//--	if (!vd_setup_capture_mode(DEVICE))
	if (!vd_setup_capture_mode(THIS)) // ++
	{
		close(mydev);
		GB.Free(POINTER(&DEVICE));
		GB.Error("Unable to setup capture mode");
		return;
	}

	vd_setup_video_source(DEVICE,channel,norm);

	DEVICE->videopict.hue=hue;
	DEVICE->videopict.contrast=contrast;
	DEVICE->videopict.brightness=brightness;
	DEVICE->videopict.colour=colour;
	DEVICE->videopict.whiteness=whiteness;
	vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);

	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)) DEVICE->Freq2=1;

END_METHOD

BEGIN_PROPERTY(CWEBCAM_source)

	long Source=0,Norm=0;

	if( THIS->is_v4l2 ) {
		gv4l2_debug("'Source' not currently implemented for V4L2");
		return;
	}

	if (READ_PROPERTY)
	{
		if (!vd_ioctl(DEVICE, VIDIOCGCHAN, &DEVICE->vchan))
		{
			switch (DEVICE->vchan.channel)
			{
				case IN_TV: Source=0; break;
				case IN_COMPOSITE1:  Source=1; break;
				case IN_COMPOSITE2: Source=2; break;
				case IN_SVIDEO: Source=3; break;
			}
			switch(DEVICE->vchan.norm)
			{
				case NORM_PAL: Norm=0; break;
				case NORM_NTSC: Norm=4; break;
				case NORM_SECAM: Norm=8; break;
				case NORM_AUTO: Norm=12; break;
			}
		}
		GB.ReturnInteger(Source+Norm);
		return;
	}

	Source=VPROP(GB_INTEGER) & 3;
	Norm=(VPROP(GB_INTEGER)>>2) & 3;

	switch( Source )
	{
		case 0: Source=IN_TV; break;
		case 1: Source=IN_COMPOSITE1; break;
		case 2: Source=IN_COMPOSITE2; break;
		case 3: Source=IN_SVIDEO; break;
	}

	switch( Norm )
	{
		case 0: Norm=NORM_PAL; break;
		case 1: Norm=NORM_NTSC; break;
		case 2: Norm=NORM_SECAM; break;
		case 3: Norm=NORM_AUTO; break;
	}

	vd_setup_video_source(DEVICE,Source,Norm);

END_METHOD
//
//=============================================================================
//
//	CWEBCAM_debug()
//
BEGIN_PROPERTY(CWEBCAM_debug)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger( gv4l2_debug_mode );
		return;
	}
	gv4l2_debug_mode = VPROP(GB_INTEGER);

END_PROPERTY
//
//=============================================================================
//
//	cwebcam_image
//
//	Raw "get_image" routine that can be used elsewhere regardless of the
//	version of V4L2 in play. Necessary refactoring I'm afraid ...
//
int cwebcam_image(CWEBCAM * _object)
{
	if( THIS->is_v4l2 ) {

		if( !gv4l2_read_frame( THIS )) return 0;
		THIS->w=THIS->fmt.fmt.pix.width;
		THIS->h=THIS->fmt.fmt.pix.height;
	}
	else
	{
		if( !vd_get_image( THIS )) return 0;
		THIS->w = DEVICE->vmmap.width;
		THIS->h = DEVICE->vmmap.height;
		vd_image_done(DEVICE);
	}
	return 1;
}
//
//	CWEBCAM_image()
//
//	Hopefully you will agree, that not only is the raw _image routine
//	required, but the resulting code is much nicer .. :)
//
BEGIN_PROPERTY(CWEBCAM_image)

	if( !cwebcam_image(THIS) ) {
		GB.Error("Unable to get image");
		GB.ReturnNull();
		return;
	}
	GB.ReturnObject(IMAGE.Create(THIS->w,THIS->h,GB_IMAGE_BGR,THIS->frame));
/*
	// Ok, this lot has been refactored, sorry
	// Once I got to "save" it became more efficient ..

	// -- GB_IMAGE ret=NULL;
	unsigned char *buf;
	int w, h;

	// ++ V4L2
	if( THIS->is_v4l2 ) {

		if( !gv4l2_read_frame( THIS ))
		{
			GB.Error("Unable to get image");
			GB.ReturnNull();
			return;
		}
		w=THIS->fmt.fmt.pix.width;
		h=THIS->fmt.fmt.pix.height;
		GB.ReturnObject(IMAGE.Create(w, h, GB_IMAGE_BGR, THIS->frame));
		return;
	}
	// -- V4L2

	// -- buf = (unsigned char*)vd_get_image(DEVICE);
	buf = (unsigned char*)vd_get_image(THIS); // ++
	if (!buf)
	{
		GB.Error("Unable to get image");
		GB.ReturnNull();
		return;
	}

	w = DEVICE->vmmap.width;
	h = DEVICE->vmmap.height;
	vd_image_done(DEVICE);

	GB.ReturnObject(IMAGE.Create(w, h, GB_IMAGE_BGR, buf));
*/

END_PROPERTY

BEGIN_METHOD(CWEBCAM_save,GB_STRING File; GB_INTEGER Quality;)

	char *File;
	char *ext=NULL;
	long b;
	FILE *fd;
	// -- char *buf;
	int format=2;
	int quality=80;
	// -- int w,h;

	File=GB.FileName(STRING(File),LENGTH(File));

	if (!File)
	{
		GB.Error("Unable to open file for writting");
		return;
	}

	if (!MISSING(Quality))
	{
		quality=VARG(Quality);
		if (quality<0) quality=0;
		if (quality>100) quality=100;
	}

	format = 0;
	
	for (b=strlen(File)-1;b>=0;b--)
		if (File[b]=='.') { ext=File+b+1; break; }

	if (ext)
	{
		if (!strcasecmp(ext,"jpeg")) format=3;
		else if (!strcasecmp(ext,"jpg")) format=3;
		else if (!strcasecmp(ext,"png")) format=2;
		else if (!strcasecmp(ext,"ppm")) format=1;
	}
	
	if (!format)
	{ 
		GB.Error("Unknown format (jpeg|jpg|png|ppm"); 
		return; 
	}

	fd=fopen(File, "w");
	if (!fd)
	{
		GB.Error("Unable to open file for writting");
		return;
	}

/*	V4L2 Refactoring

	// -- buf=(char*)vd_get_image(DEVICE);
	buf=(char*)vd_get_image(THIS); // ++
	if (!buf)
	{
		fclose(fd);
		GB.Error("Unable to get image");
		return;
	}

	w=DEVICE->vmmap.width;
	h=DEVICE->vmmap.height;

	switch(format)
	{
		case 1: put_image_ppm (buf,w,h,quality,0,fd); break;
		case 2: put_image_png (buf,w,h,0,fd); break;
		case 3: put_image_jpeg (buf,w,h,quality,0,fd); break;
	}
*/

	//
	//	V4L2 ++
	//
	if( !cwebcam_image(THIS) ) {	
		fclose(fd);
		GB.Error("Unable to get image");
		return;
	}
	switch(format)
	{
		case 1: 
			put_image_ppm (THIS->frame,THIS->w,THIS->h,quality,0,fd); 
			break;
		case 2: 
			put_image_png (THIS->frame,THIS->w,THIS->h,0,fd); 
			break;
		case 3: 
			put_image_jpeg(THIS->frame,THIS->w,THIS->h,quality,0,fd); 
			break;
	}
	//
	//	V4L2 --
	//

	fclose(fd);
	// -- (Ooops!) vd_image_done(DEVICE);

END_METHOD

/**************************************************************************

Features

***************************************************************************/
void return_array(char *array,long mmax)
{
	long bucle;
	long max=mmax;

	for (bucle=0;bucle<max;bucle++)
	{
		if (array[bucle]==0) break;
		mmax--;
	}
	GB.ReturnNewString(array,max-mmax);


}

BEGIN_PROPERTY(CFEATURES_name)

	if( THIS->is_v4l2 ) 
		GB.ReturnNewString(THIS->device,strlen(THIS->device));
	else	return_array(DEVICE->vcap.name,32);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_driver)

	struct v4l2_capability vcap;
	int dev;

	if( THIS->is_v4l2 ) 
		dev = THIS->io;
	else	dev = DEVICE->dev;
	if ( ioctl(dev,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}
	return_array((char*)vcap.driver,16);


END_PROPERTY

BEGIN_PROPERTY(CFEATURES_bus)

	struct v4l2_capability vcap;

	int dev;

	if( THIS->is_v4l2 ) 
		dev = THIS->io;
	else	dev = DEVICE->dev;

	if ( ioctl(dev,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}

	return_array((char*)vcap.bus_info,32);


END_PROPERTY

BEGIN_PROPERTY(CFEATURES_card)

	struct v4l2_capability vcap;

	int dev;

	if( THIS->is_v4l2 ) {
		return_array((char*)THIS->cap.card,32);		
		return;
	}
	dev = DEVICE->dev;

	if ( ioctl(dev,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}
	return_array((char*)vcap.driver,16);


END_PROPERTY


BEGIN_PROPERTY(CFEATURES_version)

	char arr[12];
	struct v4l2_capability vcap;

	int dev;

	if( THIS->is_v4l2 ) 
		dev = THIS->io;
	else	dev = DEVICE->dev;

	if ( ioctl(dev,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}

	sprintf (arr,"%u.%u.%u",(vcap.version>>16)&0xFF,(vcap.version>> 8)&0xFF,vcap.version&0xFF);
	GB.ReturnNewZeroString(arr);


END_PROPERTY



BEGIN_PROPERTY(CFEATURES_maxWidth)

	if( THIS->is_v4l2 ) {					// ++ V4L2
		gv4l2_debug("maxWidth not implemented in V4l2");
		GB.ReturnInteger(1024);
		return;						// ++ V4L2
	}
	GB.ReturnInteger(DEVICE->vcap.maxwidth);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_minWidth)

	if( THIS->is_v4l2 ) {					// ++ V4L2
		gv4l2_debug("minWidth not implemented in V4l2");
		GB.ReturnInteger(0);
		return;						// ++ V4L2
	}
	GB.ReturnInteger(DEVICE->vcap.minwidth);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_maxHeight)

	if( THIS->is_v4l2 ) {					// ++ V4L2	
		gv4l2_debug("maxHeight not implemented in V4l2");
		GB.ReturnInteger(768);
		return;						// ++ V4L2
	}
	GB.ReturnInteger(DEVICE->vcap.maxheight);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_minHeight)

	if( THIS->is_v4l2 ) {					// ++ V4L2
		gv4l2_debug("minHeight not implemented in V4l2");
		GB.ReturnInteger(0);
		return;						// ++ V4L2
	}
	GB.ReturnInteger(DEVICE->vcap.minheight);

END_PROPERTY

/**************************************************************************

TV tuner

***************************************************************************/

BEGIN_PROPERTY(CTUNER_name)

	struct video_tuner vtuner;
	long bucle,mmax=32;
	char * tuner = "'tuner' not currently implemented on V4L2";

	if( THIS->is_v4l2 ) {
		GB.ReturnNewString(tuner,strlen(tuner));
		return;
	}

	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)!=0)
	{
		GB.ReturnNull();
		return;
	}

	for (bucle=0;bucle<32;bucle++)
	{
		if (vtuner.name[bucle]==0) break;
		mmax--;
	}

	GB.ReturnNewString(vtuner.name,32-mmax);

END_PROPERTY

BEGIN_PROPERTY(CTUNER_signal)

	struct video_tuner vtuner;

	if( THIS->is_v4l2 ) {
		GB.ReturnInteger(0);
		return;
	}

	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)!=0)
	{
		GB.ReturnInteger(0);
		return;
	}
	GB.ReturnInteger(vtuner.signal);

END_PROPERTY

BEGIN_PROPERTY(CTUNER_low)

	struct video_tuner vtuner;
	struct v4l2_frequency vfreq;

	if( THIS->is_v4l2 ) {
		GB.ReturnBoolean(0);
		return;
	}

	if (DEVICE->Freq2)
	{
		if (READ_PROPERTY)
		{
			if (vd_ioctl (DEVICE, VIDIOC_G_FREQUENCY, &vfreq)!=0)
			{
				GB.ReturnBoolean(0);
				return;
			}
			GB.ReturnBoolean(vfreq.type & V4L2_TUNER_CAP_LOW);
			return;
		}
	}

	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)!=0)
	{
		GB.ReturnBoolean(0);
		return;
	}
	GB.ReturnBoolean(vtuner.flags & VIDEO_TUNER_LOW);


END_PROPERTY


BEGIN_PROPERTY(CTUNER_frequency)

	struct video_tuner vtuner;
	struct v4l2_frequency vfreq;

	if( THIS->is_v4l2 ) {
		GB.ReturnInteger(0);
		return;
	}

	if (DEVICE->Freq2)
	{
		if (READ_PROPERTY)
		{
			if (vd_ioctl (DEVICE, VIDIOC_G_FREQUENCY, &vfreq)!=0)
			{
				GB.ReturnInteger(0);
				return;
			}
			GB.ReturnInteger(vfreq.frequency);
			return;
		}

		if (vd_ioctl (DEVICE, VIDIOC_G_FREQUENCY, &vfreq)!=0) return;
		vfreq.frequency=VPROP(GB_INTEGER);
		vd_ioctl (DEVICE, VIDIOC_S_FREQUENCY, &vfreq);
		return;
	}

	if (READ_PROPERTY)
	{
		if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)!=0)
		{

			GB.ReturnInteger(0);
			return;
		}
		GB.ReturnInteger(vtuner.signal);
		return;
	}
	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)!=0) return;
	vtuner.signal=VPROP(GB_INTEGER);
	vd_ioctl (DEVICE, VIDIOCSTUNER, &vtuner);

END_PROPERTY

GB_DESC CFeaturesDesc[] =
{
	GB_DECLARE(".VideoDeviceFeatures", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Name","s",CFEATURES_name),
	GB_PROPERTY_READ("Driver","s",CFEATURES_driver),
	GB_PROPERTY_READ("Bus","s",CFEATURES_bus),
	GB_PROPERTY_READ("Card","s",CFEATURES_card),  // ++ V4L2
	GB_PROPERTY_READ("Version","s",CFEATURES_version),
	GB_PROPERTY_READ("MaxWidth","i",CFEATURES_maxWidth),
	GB_PROPERTY_READ("MinWidth","i",CFEATURES_minWidth),
	GB_PROPERTY_READ("MaxHeight","i",CFEATURES_maxHeight),
	GB_PROPERTY_READ("MinHeight","i",CFEATURES_minHeight),

	GB_END_DECLARE
};

GB_DESC CTunerDesc[] =
{
	GB_DECLARE(".VideoDeviceTuner", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Name","s",CTUNER_name),
	GB_PROPERTY_READ("Quality","i",CTUNER_signal),
	GB_PROPERTY_READ("Low","b",CTUNER_low),

	GB_PROPERTY("Frequency","i",CTUNER_frequency),

	GB_END_DECLARE
};

GB_DESC CWebcamDesc[] =
{

  GB_DECLARE("VideoDevice", sizeof(CWEBCAM)),
  GB_INHERITS("Stream"),

  GB_HOOK_CHECK(CWEBCAM_check),

  GB_CONSTANT("Hz","i",1),
  GB_CONSTANT("Khz","i",0),

  GB_CONSTANT("Pal","i",0),//NORM_PAL),
  GB_CONSTANT("Ntsc","i",4),//NORM_NTSC),
  GB_CONSTANT("Secam","i",8),//NORM_SECAM),
  GB_CONSTANT("Auto","i",12),//NORM_AUTO),

  GB_CONSTANT("Tv","i",0), //IN_TV),
  GB_CONSTANT("Composite1","i",1), //IN_COMPOSITE1),
  GB_CONSTANT("Composite2","i",2), //IN_COMPOSITE2),
  GB_CONSTANT("SVideo","i",3), //IN_SVIDEO),
  GB_CONSTANT("V4L","i",1),		// ++ force V4L1
  GB_CONSTANT("V4L2","i",2),		// ++ force V4L2

  GB_METHOD("_new",NULL,CWEBCAM_new,"(Device)s[(V4L|V4L2)i]"),
  GB_METHOD("_free",NULL,CWEBCAM_free,NULL),

  GB_PROPERTY_SELF("Features",".VideoDeviceFeatures"),

  GB_PROPERTY_SELF("Tuner",".VideoDeviceTuner"),
  GB_PROPERTY("Source","i",CWEBCAM_source),

  GB_PROPERTY_READ("Width","i",CWEBCAM_width),
  GB_PROPERTY_READ("Height","i",CWEBCAM_height),

  GB_PROPERTY("Contrast","i",CWEBCAM_contrast),
  GB_PROPERTY("Contrast_Max","i",CWEBCAM_contrast_max),
  GB_PROPERTY("Contrast_Min","i",CWEBCAM_contrast_min),
  GB_PROPERTY("Color","i",CWEBCAM_colour),
  GB_PROPERTY("Color_Max","i",CWEBCAM_color_max),
  GB_PROPERTY("Color_Min","i",CWEBCAM_color_min),
  GB_PROPERTY("Whiteness","i",CWEBCAM_whiteness),
  GB_PROPERTY("Whiteness_Max","i",CWEBCAM_whiteness_max),
  GB_PROPERTY("Whiteness_Min","i",CWEBCAM_whiteness_min),
  GB_PROPERTY("Bright","i",CWEBCAM_bright),
  GB_PROPERTY("Bright_Max","i",CWEBCAM_bright_max),
  GB_PROPERTY("Bright_Min","i",CWEBCAM_bright_min),
  GB_PROPERTY("Hue","i",CWEBCAM_hue),
  GB_PROPERTY("Hue_Max","i",CWEBCAM_hue_max),
  GB_PROPERTY("Hue_Min","i",CWEBCAM_hue_min),

  GB_PROPERTY("Image","Image",CWEBCAM_image),
  GB_PROPERTY("Debug","i",CWEBCAM_debug),

  GB_METHOD("Resize",NULL,CWEBCAM_size,"(Width)i(Height)i"),
  GB_METHOD("Save",NULL,CWEBCAM_save,"(File)s[(Quality)i]"),

  GB_END_DECLARE
};


