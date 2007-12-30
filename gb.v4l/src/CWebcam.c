/***************************************************************************

  CWebcam.c

  Webcam capture component

  (C) 2005 Daniel Campos Fernández <danielcampos@netcourrier.com>

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
	Video_stream_open,
	Video_stream_close,
	Video_stream_read,
	Video_stream_write,
	Video_stream_seek,
	Video_stream_tell,
	Video_stream_flush,
	Video_stream_eof,
	Video_stream_lof,
	Video_stream_handle
};

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
		else GB.Free((void**)&vd->frame_buffer);
	}
	close(vd->dev);
}

video_device_t	*vd_setup(int width, int height, int depth, int dev)
{
	video_device_t *vd;

	GB.Alloc((void**)&vd,sizeof(video_device_t));

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


int	vd_setup_capture_mode(video_device_t *vd)
{
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

		if (vd->frame_buffer) GB.Free((void**)&vd->frame_buffer);
		GB.Alloc ((void**)&vd->frame_buffer,vd->buffer_size);
		return 1;
	}

	// mmap is okay!
	if (!vd->use_mmap)
	{
		if (vd->frame_buffer) GB.Free((void**)&vd->frame_buffer);
		vd->use_mmap = 1;
	}

	vd->frame_buffer = mmap(0, vd->vmbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED, vd->dev, 0);
	vd->vmmap.format = VIDEO_PALETTE_RGB24;		// KLUDGE ...
	vd->vmmap.frame = 0;				// Start at frame 0
	vd->vmmap.width = vd->width;
	vd->vmmap.height = vd->height;
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

	line = malloc (width * 3);
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
	free (line);

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
	if (!THIS->membuf)	GB.Alloc((void**)&THIS->membuf,((*len)*sizeof(unsigned char*)));
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



unsigned char *	vd_get_image(video_device_t *vd)
{
	int	len;

	if (vd->use_mmap) {

		if (!vd->capturing) {

			int i;
			// Queue requests to capture successive frames
			for (i = 0; i < vd->vmbuf.frames; ++i) {
				vd->vmmap.frame = i;
				if (vd_ioctl(vd, VIDIOCMCAPTURE, &vd->vmmap)) return 0;
			}

			// And start reading from zero
			vd->vmmap.frame = 0;

			vd->capturing = 1;
		}

		// VIDIOCSYNC causes the driver to block until the specified
		// frame is completely received
		if (ioctl(vd->dev, VIDIOCSYNC, &vd->vmmap.frame)) return 0;

		// Return the buffer, cause it should contain an image
		return vd->frame_buffer + vd->vmbuf.offsets[vd->vmmap.frame];
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

	buf=(char*)vd_get_image(DEVICE);
	if (!buf) return -1;
	w=DEVICE->vmmap.width;
	h=DEVICE->vmmap.height;
	vd_image_done(DEVICE);
	put_image_ppm_buffer (buf,w,h,0,&THIS->gotframe,_object);
	THIS->posframe=0;
	return 0;
}

int Video_stream_read(GB_STREAM *stream, char *buffer, long len)
{
	void *_object=(void*)stream->_free[0];

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
	void *_object=(void*)stream->_free[0];

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe) return 0;

	if (THIS->gotframe<=THIS->posframe) return -1;
	return 0;
}

int Video_stream_lof(GB_STREAM *stream, long long *len)
{
	void *_object=(void*)stream->_free[0];

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe)
		if ( fill_buffer(_object) ) return -1;

	*len=(long long)THIS->gotframe;
	return 0;
}

int Video_stream_seek(GB_STREAM *stream, long long pos, int whence)
{
	void *_object=(void*)stream->_free[0];

	if (!_object) return -1;
	if (!DEVICE) return -1;

	if (!THIS->gotframe)
		if ( fill_buffer(_object) ) return -1;

	if (pos<0) return -1;
	THIS->posframe=pos;
	return 0;
}

int Video_stream_tell(GB_STREAM *stream, long long *pos)
{
	void *_object=(void*)stream->_free[0];

	if (!_object) return -1;
	if (!DEVICE) return -1;

	*pos=(long long)THIS->posframe;
	return 0;

}

int Video_stream_flush(GB_STREAM *stream)
{
	void *_object=(void*)stream->_free[0];

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

int Video_stream_write(GB_STREAM *stream, char *buffer, long len)
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
	if (!DEVICE) return TRUE;
  	return FALSE;
}

BEGIN_PROPERTY(CWEBCAM_bright)

	vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(DEVICE->videopict.brightness);
		return;
	}

	DEVICE->videopict.brightness=VPROP(GB_INTEGER);
	vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_contrast)

	vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(DEVICE->videopict.contrast);
		return;
	}

	DEVICE->videopict.contrast=VPROP(GB_INTEGER);
	vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_colour)

	vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(DEVICE->videopict.colour);
		return;
	}

	DEVICE->videopict.colour=VPROP(GB_INTEGER);
	vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_whiteness)

	vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(DEVICE->videopict.whiteness>>8);
		return;
	}

	DEVICE->videopict.whiteness=VPROP(GB_INTEGER);
	vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_hue)

	vd_ioctl (DEVICE, VIDIOCGPICT, &DEVICE->videopict);
	if (READ_PROPERTY)
	{
		GB.ReturnInteger(DEVICE->videopict.hue>>8);
		return;
	}

	DEVICE->videopict.hue=VPROP(GB_INTEGER);
	vd_ioctl (DEVICE, VIDIOCSPICT, &DEVICE->videopict);

END_PROPERTY


BEGIN_PROPERTY(CWEBCAM_width)

	GB.ReturnInteger(DEVICE->width);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_height)

	GB.ReturnInteger(DEVICE->height);

END_PROPERTY

BEGIN_METHOD (CWEBCAM_new,GB_STRING Device;)

	int mydev;
	struct video_tuner vtuner;

	mydev=open (GB.FileName(STRING(Device),LENGTH(Device)),O_RDWR);
	if (mydev==-1)
	{
		GB.Error("Unable to open device");
		return;
	}
	DEVICE=vd_setup(DEF_WIDTH,DEF_HEIGHT,DEF_DEPTH,mydev);

	if (!vd_setup_capture_mode(DEVICE))
	{
		close(mydev);
		GB.Free((void**)&DEVICE);
		GB.Error("Unable to setup capture mode");
		return;
	}

	vd_setup_video_source(DEVICE,IN_DEFAULT,NORM_DEFAULT);
	GB.Alloc((void**)&THIS->device,sizeof(char)*(LENGTH(Device)+1));
	strcpy(THIS->device,STRING(Device));

	if (vd_ioctl (DEVICE, VIDIOCGTUNER, &vtuner)) DEVICE->Freq2=1;

	THIS->stream.desc=&VideoStream;
	THIS->stream._free[0]=(long)THIS;

END_METHOD

BEGIN_METHOD_VOID(CWEBCAM_free)

	if (THIS->device) GB.Free((void**)&THIS->device);
	if (THIS->membuf) GB.Free((void**)&THIS->membuf);

	if (DEVICE)
	{
		vd_close(DEVICE);
		GB.Free((void**)&DEVICE);
	}

END_METHOD

BEGIN_METHOD(CWEBCAM_size,GB_INTEGER Width; GB_INTEGER Height;)

	struct video_tuner vtuner;
	long w=VARG(Width);
	long h=VARG(Height);
	int mydev;
	int norm;
	int channel;
	int colour,hue,whiteness,contrast,brightness;

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

	if (THIS->membuf) GB.Free((void**)&THIS->membuf);
	vd_close(DEVICE);
	GB.Free((void**)&DEVICE);

	mydev=open(THIS->device,O_RDWR);
	if (mydev==-1)
	{
		GB.Error("Unable to open device");
		return;
	}
	DEVICE=vd_setup(w,h,DEF_DEPTH,mydev);

	if (!vd_setup_capture_mode(DEVICE))
	{
		close(mydev);
		GB.Free((void**)&DEVICE);
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

BEGIN_PROPERTY(CWEBCAM_image)

	GB_IMAGE ret=NULL;
	char *buf;
	int w,h;
	long bucle;

	buf=(char*)vd_get_image(DEVICE);
	if (!buf)
	{
		GB.Error("Unable to get image");
		GB.ReturnNull();
		return;
	}

	w=DEVICE->vmmap.width;
	h=DEVICE->vmmap.height;
	vd_image_done(DEVICE);

	GB.Image.Create(&ret,(void*)buf,w,h,GB_IMAGE_BGR);

	GB.ReturnObject((void*)ret);

END_PROPERTY

BEGIN_PROPERTY(CWEBCAM_picture)

	GB_PICTURE ret=NULL;
	char *buf;
	int w,h;
	long bucle;

	buf=(char*)vd_get_image(DEVICE);
	if (!buf)
	{
		GB.Error("Unable to get image");
		GB.ReturnNull();
		return;
	}

	w=DEVICE->vmmap.width;
	h=DEVICE->vmmap.height;
	vd_image_done(DEVICE);

	GB.Picture.Create(&ret,(void*)buf,w,h,GB_IMAGE_BGR);

	GB.ReturnObject((void*)ret);

END_PROPERTY

BEGIN_METHOD(CWEBCAM_save,GB_STRING File; GB_INTEGER Format; GB_INTEGER Quality;)

	char *File;
	char *ext=NULL;
	long b;
	FILE *fd;
	char *buf;
	int format=2;
	int quality=80;
	int w,h;

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

	if (!MISSING(Format))
	{
		switch(VARG(Format))
		{
			case 1:
			case 2:
			case 3: format=VARG(Format); break;
			default : GB.Error("Unknown format"); return;
		}
	}
	else
	{
		for (b=strlen(File)-1;b>=0;b--)
			if (File[b]=='.') { ext=File+b+1; break; }

		if (ext)
		{
			if (!strcasecmp(ext,"jpeg")) format=3;
			else if (!strcasecmp(ext,"jpg")) format=3;
			else if (!strcasecmp(ext,"png")) format=2;
			else if (!strcasecmp(ext,"ppm")) format=1;
			else { GB.Error("Unknown format"); return; }
		}
	}

	fd=fopen(File, "w");
	if (!fd)
	{
		GB.Error("Unable to open file for writting");
		return;
	}
	buf=(char*)vd_get_image(DEVICE);
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

	fclose(fd);
	vd_image_done(DEVICE);

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

	return_array(DEVICE->vcap.name,32);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_driver)

	struct v4l2_capability vcap;

	if ( vd_ioctl(DEVICE,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}

	return_array((char*)vcap.driver,16);


END_PROPERTY

BEGIN_PROPERTY(CFEATURES_bus)

	struct v4l2_capability vcap;

	if ( vd_ioctl(DEVICE,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}

	return_array((char*)vcap.bus_info,32);


END_PROPERTY

BEGIN_PROPERTY(CFEATURES_version)

	char arr[12];
	struct v4l2_capability vcap;

	if ( vd_ioctl(DEVICE,VIDIOC_QUERYCAP,&vcap)!=0 )
	{
		GB.ReturnNull();
		return;
	}

	sprintf (arr,"%u.%u.%u",(vcap.version>>16)&0xFF,(vcap.version>> 8)&0xFF,vcap.version&0xFF);
	GB.ReturnNewZeroString(arr);


END_PROPERTY

BEGIN_PROPERTY(CFEATURES_maxWidth)

	GB.ReturnInteger(DEVICE->vcap.maxwidth);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_minWidth)

	GB.ReturnInteger(DEVICE->vcap.minwidth);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_maxHeight)

	GB.ReturnInteger(DEVICE->vcap.maxheight);

END_PROPERTY

BEGIN_PROPERTY(CFEATURES_minHeight)

	GB.ReturnInteger(DEVICE->vcap.minheight);

END_PROPERTY

/**************************************************************************

TV tuner

***************************************************************************/

BEGIN_PROPERTY(CTUNER_name)

	struct video_tuner vtuner;
	long bucle,mmax=32;


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
	GB_DECLARE(".VideoDevFeatures", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Name","s",CFEATURES_name),
	GB_PROPERTY_READ("Driver","s",CFEATURES_driver),
	GB_PROPERTY_READ("Bus","s",CFEATURES_bus),
	GB_PROPERTY_READ("Version","s",CFEATURES_version),
	GB_PROPERTY_READ("MaxWidth","i",CFEATURES_maxWidth),
	GB_PROPERTY_READ("MinWidth","i",CFEATURES_minWidth),
	GB_PROPERTY_READ("MaxHeight","i",CFEATURES_maxHeight),
	GB_PROPERTY_READ("MinHeight","i",CFEATURES_minHeight),


	GB_END_DECLARE
};

GB_DESC CTunerDesc[] =
{
	GB_DECLARE(".VideoDevTuner", 0),
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

  GB_CONSTANT("JPEG","i",FMT_JPEG),
  GB_CONSTANT("PPM","i",FMT_PPM),
  GB_CONSTANT("PNG","i",FMT_PNG),

  GB_CONSTANT("Hz","i",1),
  GB_CONSTANT("Khz","i",0),

  GB_CONSTANT("PAL","i",0),//NORM_PAL),
  GB_CONSTANT("NTSC","i",4),//NORM_NTSC),
  GB_CONSTANT("SECAM","i",8),//NORM_SECAM),
  GB_CONSTANT("AUTO","i",12),//NORM_AUTO),

  GB_CONSTANT("TV","i",0), //IN_TV),
  GB_CONSTANT("COMPOSITE1","i",1), //IN_COMPOSITE1),
  GB_CONSTANT("COMPOSITE2","i",2), //IN_COMPOSITE2),
  GB_CONSTANT("SVIDEO","i",3), //IN_SVIDEO),

  GB_METHOD("_new",NULL,CWEBCAM_new,"(Device)s"),
  GB_METHOD("_free",NULL,CWEBCAM_free,NULL),

  GB_PROPERTY_SELF("Tuner",".VideoDevTuner"),
  GB_PROPERTY_SELF("Features",".VideoDevFeatures"),

  GB_PROPERTY_READ("Width","i",CWEBCAM_width),
  GB_PROPERTY_READ("Height","i",CWEBCAM_height),

  GB_PROPERTY("Source","i",CWEBCAM_source),
  GB_PROPERTY("Bright","i",CWEBCAM_bright),
  GB_PROPERTY("Contrast","i",CWEBCAM_contrast),
  GB_PROPERTY("Color","i",CWEBCAM_colour),
  GB_PROPERTY("Whiteness","i",CWEBCAM_whiteness),
  GB_PROPERTY("Hue","i",CWEBCAM_hue),

  GB_PROPERTY("Image","Image",CWEBCAM_image),
  GB_PROPERTY("Picture","Picture",CWEBCAM_picture),

  GB_METHOD("Resize",NULL,CWEBCAM_size,"(Width)i(Height)i"),
  GB_METHOD("Save",NULL,CWEBCAM_save,"(File)s[(Format)i(Quality)i]"),

  GB_END_DECLARE
};


