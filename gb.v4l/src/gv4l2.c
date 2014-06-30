/***************************************************************************

  gv4l2.c

  (C) 2009 Gareth Bult <gareth@encryptec.net>

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

#define __CWEBCAM_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>



#ifdef HAVE_STDLIB_H
#undef HAVE_STDLIB_H
#endif

#include "main.h"
#include "CWebcam.h"

#ifdef OS_LINUX
  #include <asm/types.h>
#else
  #include <stdint.h>
#endif

//
bool gv4l2_debug_mode = TRUE;
//
//=============================================================================
//
//	gv4l2_available()
//
//	Test for V4L2 availability
//
int gv4l2_available(CWEBCAM * _object)
{
	char dummy[256];

	return(!(ioctl( THIS->io , VIDIOC_QUERYCAP , dummy ) == -1 ));
}
//=============================================================================
//
//	v4l2_debug( string )
//	
//	Debugging routine for V4L2
//
void gv4l2_debug(const char *s) 
{
	if (!gv4l2_debug_mode) return;
	fprintf(stderr, "gb.v4l: v4l2: %s: %s\n", s, strerror(errno));
}

//=============================================================================
//
//	xioctl( fd,request,arg )
//
//	Local swapper for ioctl to repeat on EINTR's
//	
int gv4l2_xioctl( int fd,int request,void * arg)
{
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);
	return r;
}
//
//=============================================================================
//
//	gv4l2_hue( THIS, value )
//
void gv4l2_camera_setup(CWEBCAM *_object,int id,int *min,int *max,int *def)
{
	struct v4l2_queryctrl	query;

	memset (&query, 0, sizeof (query));
	query.id = id;

	if(gv4l2_xioctl(THIS->io,VIDIOC_QUERYCTRL,&query)==-1)return;

	//printf("Name=%s,Min=%d,Max=%d,Value=%d\n",
	//	query.name,
	//	query.minimum,
	//	query.maximum,
	//	query.default_value);
	//fflush(stdout);

	*max = query.maximum;
	*min = query.minimum;
	*def = query.default_value;
}
//
int gv4l2_camera_get( CWEBCAM * _object , int id , int value )
{
	struct 	v4l2_control 	control;	
	int 			command;
	int			result;

	memset (&control, 0, sizeof (control));
	control.id = id;
       	control.value = value;

	if( value != -1 )
		command = VIDIOC_S_CTRL;
	else	command = VIDIOC_G_CTRL;

       	result = gv4l2_xioctl (THIS->io, command , &control);

	if( result == -1 ) return result;
	return control.value;
}
//
void gv4l2_hue_setup( CWEBCAM * _object )
{
	gv4l2_camera_setup( THIS ,
		V4L2_CID_HUE , &THIS->hue_min , &THIS->hue_max ,&THIS->hue_def);
}
//
int gv4l2_hue( CWEBCAM * _object , int value )
{
	return gv4l2_camera_get( THIS, V4L2_CID_HUE , value );
}
//
//=============================================================================
//
//	gv4l2_brightness( THIS, value )
//
void gv4l2_brightness_setup( CWEBCAM * _object )
{
	gv4l2_camera_setup( THIS,
		V4L2_CID_BRIGHTNESS , &THIS->bright_min ,&THIS->bright_max,&THIS->bright_def);
}

int gv4l2_brightness( CWEBCAM * _object , int value )
{
	return gv4l2_camera_get( THIS, V4L2_CID_BRIGHTNESS , value );
}
//
//=============================================================================
//
//	gv4l2_contrast( THIS, value )
//
void gv4l2_contrast_setup( CWEBCAM * _object )
{
	gv4l2_camera_setup( THIS , 
		V4L2_CID_CONTRAST, &THIS->contrast_min , &THIS->contrast_max,&THIS->contrast_def);
}

int gv4l2_contrast( CWEBCAM * _object , int value )
{
	return gv4l2_camera_get( THIS, V4L2_CID_CONTRAST , value );
}
//
//=============================================================================
//
//	gv4l2_color( THIS, value )
//
void gv4l2_color_setup( CWEBCAM * _object )
{
	gv4l2_camera_setup( THIS , 
		V4L2_CID_SATURATION, &THIS->color_min, &THIS->color_max,&THIS->color_def );
}

int gv4l2_color( CWEBCAM * _object , int value )
{
	return gv4l2_camera_get( THIS, V4L2_CID_SATURATION , value );
}
//
//=============================================================================
//
//	gv4l2_color( THIS, value )
//
void gv4l2_whiteness_setup( CWEBCAM * _object )
{
	gv4l2_camera_setup( THIS , 
		V4L2_CID_WHITENESS , &THIS->whiteness_min,&THIS->whiteness_max,&THIS->whiteness_def);
}
//
int gv4l2_whiteness( CWEBCAM * _object , int value )
{
	return gv4l2_camera_get( THIS, V4L2_CID_WHITENESS , value );
}
//=============================================================================
//
//
//=============================================================================
//
//	v4l2_open_device( device_name )
//
//	Open the raw device (typically /dev/video?) , note that we're not
//	using non-blocking mode as (a) it's not needed given we're recovering
//	specific frames and (b) camera's are often "not ready" so it would
//	require retries under Gambas.
//
//	FIXME:: what happens when you unplug a camera when active ??
//
int gv4l2_open_device( char* name )
{
	struct stat file_info;
	int status;

	//
	//	See if the file is there ...
	//
	status = stat(name,&file_info);
	if( status == -1 ) {
		gv4l2_debug("failed to stat device");
		return status;
	}
	//
	//	Make sure it's a character device (/dev/video?)
	//
	if( !S_ISCHR(file_info.st_mode) ) {
		gv4l2_debug("not a character device");
		return status;
	}
	//
	//	Finally, try to open the file ..
	//
	return open( name,O_RDWR /* |O_NONBLOCK */ ,0 );
}
//============================================================================
//
//	v4l2_close_device( id )
//
//	Close the device, got to be done and can't get much easier !  ;-)
//
void gv4l2_close_device( int id )
{
	if( close( id ) == -1 ) {
		gv4l2_debug("error closing device");
	}
}
//============================================================================
//
//	v4l2_init_device( THIS , Width , Height )
//
//	Initialise the device and associated data structures, this is the most
//	complex operation in the code and has to cope with it's own MMAP
//	handling whereas V4L did a lot of this for us.
//
//	FIXME:: test the READ interface, I only use MMAP cameras ...
//
int gv4l2_init_device(CWEBCAM * _object , int width , int height )
{
	unsigned int min;
	static unsigned int n_buffers = 0;
	int save;

	if ( gv4l2_xioctl (THIS->io, VIDIOC_QUERYCAP, &THIS->cap) == -1 ) {
		gv4l2_debug("VIDIOC_QUERYCAP error");
		return 0;
	}

	if (!(THIS->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		gv4l2_debug("not video capture device");
		return 0;
	}
	//
	//	We need to choose which IO method to use, well try MMAP and
	//	if that fails, fall back to READ
	//
	if (!(THIS->cap.capabilities & V4L2_CAP_STREAMING)) {
		//
		//	No MMAP support!
		//
		THIS->use_mmap = 0;
		if (!(THIS->cap.capabilities & V4L2_CAP_READWRITE)) {
			gv4l2_debug("device does not support mmap or read");
			return 0;
			}
		} 
	else	
		THIS->use_mmap = 1;

	MCLEAR(THIS->cropcap);
	THIS->cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (!gv4l2_xioctl (THIS->io, VIDIOC_CROPCAP, &THIS->cropcap)) {
		THIS->crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		THIS->crop.c = THIS->cropcap.defrect;

		if ( gv4l2_xioctl (THIS->io, VIDIOC_S_CROP, &THIS->crop) == -1 ) 
		{
			if( errno == EINVAL ) {
				gv4l2_debug("cropping not supported");
			}
		}
	}

	MCLEAR(THIS->fmt);
	THIS->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if( gv4l2_xioctl( THIS->io, VIDIOC_G_FMT, &THIS->fmt ) == -1 ) {
		gv4l2_debug("Unable to get Video formats");
		return 0;
	}

	THIS->fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	THIS->fmt.fmt.pix.width       = width;
	THIS->fmt.fmt.pix.height      = height;
	THIS->fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	
	save = THIS->fmt.fmt.pix.pixelformat;
	
	//
	//	Camera format should be picked up from VIDIOC_G_FMT above
	//	FIXME:: do cameras support multiple formats and so we want
	//	to be able to pick the format??
	//
	//	Try the supported formats;
	//		a. YUYV
	//		b. YUV420
	//		c. revert to whatever the camera was set to
	//
	THIS->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	if ( gv4l2_xioctl ( THIS->io, VIDIOC_S_FMT, &THIS->fmt) == -1) {
		gv4l2_debug("VIDIOC_S_FMT, can't set YUYV, trying YUV 420");
		THIS->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
		if ( gv4l2_xioctl ( THIS->io, VIDIOC_S_FMT, &THIS->fmt) == -1) {
			gv4l2_debug("VIDIOC_S_FMT, can't set YUV420, defaulting ");
			THIS->fmt.fmt.pix.pixelformat = save;
		}
	}
				
	// BM: Final image gb.image format
	THIS->format = GB_IMAGE_BGR; //IMAGE.GetDefaultFormat();
	
	// BM: Conversion structure
	THIS->convert = v4lconvert_create(THIS->io);
	
	//THIS->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	//if ( gv4l2_xioctl ( THIS->io, VIDIOC_S_FMT, &THIS->fmt) == -1) {
	//	gv4l2_debug("VIDIOC_S_FMT, unable to set format");
	//	return 0;
	//}
	// THIS->fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	// gv4l2_xioctl ( THIS->io, VIDIOC_S_FMT, &THIS->fmt);

	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	min = THIS->fmt.fmt.pix.width * 2;
	if (THIS->fmt.fmt.pix.bytesperline < min) 
		THIS->fmt.fmt.pix.bytesperline = min;
	min = THIS->fmt.fmt.pix.bytesperline * THIS->fmt.fmt.pix.height;
	if (THIS->fmt.fmt.pix.sizeimage < min)    
		THIS->fmt.fmt.pix.sizeimage = min;

	GB.Alloc(&THIS->frame, THIS->fmt.fmt.pix.width * THIS->fmt.fmt.pix.height * (GB_IMAGE_FMT_IS_24_BITS(THIS->format) ? 3 : 4));

	gv4l2_brightness_setup( THIS );
	gv4l2_contrast_setup( THIS );
	gv4l2_color_setup( THIS );
	gv4l2_whiteness_setup( THIS );
	gv4l2_hue_setup( THIS );

	if( !THIS->use_mmap ) {
		GB.Alloc( POINTER(&THIS->buffers) ,sizeof(*THIS->buffers));
		if( !THIS->buffers ) {
			gv4l2_debug("Failed to allocate buffer space");
			return 0;
		}
		THIS->buffers[0].length = THIS->fmt.fmt.pix.sizeimage;
		GB.Alloc( POINTER(&THIS->buffers[0].start),THIS->fmt.fmt.pix.sizeimage);
		if( !THIS->buffers[0].start ) {
			gv4l2_debug("Failed to allocate buffer space");
			return 0;
		}
		return 1;			
	}
	//	We don't support USERPTR in Gambas (!)
	//	So now we initialise MMAP
	//
	struct v4l2_requestbuffers req;

	MCLEAR(req);
	req.count	= 2;
	req.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory 	= V4L2_MEMORY_MMAP;

	if ( gv4l2_xioctl (THIS->io, VIDIOC_REQBUFS, &req) == -1 ) {
		gv4l2_debug("mmap not supported or error");
		return 0;
	}
	if (req.count < 2) {
		gv4l2_debug("not enough memory for mmap");
		return 0;
	}
	GB.Alloc ( POINTER(&THIS->buffers),req.count * sizeof (*THIS->buffers));
	if (!THIS->buffers) {
		gv4l2_debug("not memory for mmap");
		return 0;
		}
	THIS->buffer_count = req.count;
	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		MCLEAR(buf);

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if( gv4l2_xioctl (THIS->io, VIDIOC_QUERYBUF, &buf) == -1 ) {
			gv4l2_debug("VIDIOC_QUERYBUF");
			return 0;
		}

		THIS->buffers[n_buffers].length = buf.length;
		THIS->buffers[n_buffers].start =
		mmap (NULL /* start anywhere */,
					buf.length,
					PROT_READ | PROT_WRITE /* required */,
					MAP_SHARED /* recommended */,
					THIS->io, buf.m.offset);

		if (MAP_FAILED == THIS->buffers[n_buffers].start) {
			gv4l2_debug("mmap failed");
			return 0;
		}
	}
	return 1;
}

//=============================================================================
//
//	v4l2_start_capture()
//
//	Start capture mode, this should turn on the little green light on your
//	camera.
//
//	FIXME:: make sure we check the return status on this call
//
int gv4l2_start_capture(CWEBCAM * _object)
{
	int i;
        enum v4l2_buf_type type;
	//
	gv4l2_debug("Capture ON");
	//
	//	Nothing to do unless we're using MMAP
	//
	if( !THIS->use_mmap) return 1;
	//
	for( i=0; i<THIS->buffer_count; i++ ) {
		struct v4l2_buffer buf;

		MCLEAR (buf);
	        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = i;

                if( gv4l2_xioctl( THIS->io, VIDIOC_QBUF, &buf) == -1 ) {
			gv4l2_debug("VIDIOC_QBUF error starting capture");
			return 0;
                }
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( gv4l2_xioctl( THIS->io, VIDIOC_STREAMON, &type) == -1 ) {
		gv4l2_debug("VIDIOC_STREAMON error starting capture");
		return 0;
	}
	return 1;
}
//=============================================================================
//
//	v4l2_stop_capture()
//
//	Stop Capturing on device (turn little green light off!)
//
//	FIXME:: check return status on this call!
//
int gv4l2_stop_capture(CWEBCAM * _object)
{
        enum v4l2_buf_type type;

	if( !THIS->use_mmap) return 1;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if( gv4l2_xioctl( THIS->io, VIDIOC_STREAMOFF, &type) == -1)
	{
		gv4l2_debug("VIDIOC_STREAMOFF error");
		return 0;
	}
	return 1;
}
//=============================================================================
//
//	gv4l2_uninit_device(THIS)
//
//	Uninitialise the device and free all the associated memory
//
void gv4l2_uninit_device(CWEBCAM * _object)
{
	unsigned int i;

	GB.Free( POINTER(&THIS->frame) );

	v4lconvert_destroy(THIS->convert);
	
	if( !THIS->use_mmap) { 
		GB.Free ( POINTER(&THIS->buffers[0].start));
		GB.Free ( POINTER(&THIS->buffers));
		return;
	}
	for (i = 0; i < THIS->buffer_count; ++i )
		if(munmap(THIS->buffers[i].start,THIS->buffers[i].length)==-1) 
			gv4l2_debug("MUNMAP Error");

	GB.Free ( POINTER(&THIS->buffers));
}
//=============================================================================
//
//	g4vl_process_image(THIS,start)
//
//	Process the image found in start and dump the resulting RGB frame into
//	our local frame buffer (THIS->frame). Width, Height and Image size can
//	all be found in THIS->fmt.fmt
//
//	FIXME:: there are lots of formats, I can *only* test YUYV.
//              I'm *assuming* RGB32 is "raw" mode (no conversion)
//		Do "other" RGB formats work without conversion?
//		What other conversion routines do we need?
//		Will BM be moving any/all of these to Image/Picture objects?
//
void gv4l1_process_image (CWEBCAM * _object, void *start)
{
	int format,w,h;
	long size;

	format = THIS->dev->videopict.palette;
	w = THIS->dev->width;
	h = THIS->dev->height;
	size = THIS->dev->buffer_size;

	switch(format) 
	{
		case VIDEO_PALETTE_YUV411P:	gv4l2_debug("YUV411P"); break;
		case VIDEO_PALETTE_YUV420P:	
			//gv4l2_debug("YUV420P");
		case VIDEO_PALETTE_YUV420:
			//gv4l2_debug("YUV420");
			yuv420p_to_rgb (start,THIS->frame,w,h,3);
			return;
		case VIDEO_PALETTE_YUYV:
			//gv4l2_debug("YUYV");
			convert_yuv_to_rgb_buffer(start,THIS->frame,w,h);
			return;

		case VIDEO_PALETTE_GREY:	gv4l2_debug("GREY");	break;
		case VIDEO_PALETTE_HI240:	gv4l2_debug("HI240");	break;
		case VIDEO_PALETTE_RGB565:	gv4l2_debug("RGB5656");	break;
		case VIDEO_PALETTE_RGB24:	gv4l2_debug("RGB24");	break;
		case VIDEO_PALETTE_RGB32:	/* DEFAULT */		break;
		case VIDEO_PALETTE_RGB555:	gv4l2_debug("RGB555");	break;
		case VIDEO_PALETTE_UYVY:	gv4l2_debug("UYVY");	break;
		case VIDEO_PALETTE_YUV411:	gv4l2_debug("YUV411");	break;
		case VIDEO_PALETTE_RAW:		gv4l2_debug("RAW");	break;
		case VIDEO_PALETTE_YUV422P:	gv4l2_debug("YUV422P");	break;
		case VIDEO_PALETTE_YUV410P:	gv4l2_debug("YUV410P");	break;
		case VIDEO_PALETTE_COMPONENT:	gv4l2_debug("COMPONENT");break;
    
		default:
			gv4l2_debug("Frame in unknown format");
			break;
	}
	memcpy(THIS->frame,start,size);
}
//
//	v4l2 version (!)
//
void gv4l2_process_image (CWEBCAM * _object, void *start)
{
	struct v4l2_format dest = THIS->fmt;

	if (THIS->format != GB_IMAGE_BGR)
		gv4l2_debug("Destination format not supported");
	
	dest.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
	dest.fmt.pix.sizeimage = THIS->fmt.fmt.pix.width * THIS->fmt.fmt.pix.height * 3;

	if (v4lconvert_convert(THIS->convert, &THIS->fmt, &dest, start, THIS->fmt.fmt.pix.sizeimage, THIS->frame, dest.fmt.pix.sizeimage) != dest.fmt.pix.sizeimage)
		gv4l2_debug("Unable to convert webcam image to BGR24");
}

#if 0
	switch(format) 
	{
/*
  For adding new formats use the #ifdef method.
  It is not safe to asume every videodev2.h has the V4L2_PIX_FMT_
  V4L2_PIX_FMT_Y16 was not aviable in Ubuntu 8.04 but exists in Ubuntu 8.10
  Some other where also missing
  Add by:R.Onstenk 22-feb-2009
*/
		case V4L2_PIX_FMT_RGB332: 	gv4l2_debug("RGB332");	break;
		case V4L2_PIX_FMT_RGB444: 	gv4l2_debug("RGB444"); 	break;
		case V4L2_PIX_FMT_RGB555: 	gv4l2_debug("RGB555"); 	break;
		case V4L2_PIX_FMT_RGB565: 	gv4l2_debug("YRGB565");	break;
		case V4L2_PIX_FMT_RGB555X: 	gv4l2_debug("YRGB555X");break;
		case V4L2_PIX_FMT_RGB565X: 	gv4l2_debug("RGB565X"); break;
		case V4L2_PIX_FMT_BGR24: 	gv4l2_debug("BGR24"); 	break;  
		case V4L2_PIX_FMT_RGB24: 	gv4l2_debug("RGB24"); 	break;
		case V4L2_PIX_FMT_BGR32: 	gv4l2_debug("BGR32"); 	break;  
		case V4L2_PIX_FMT_RGB32:	/* DEFAULT - NO CONV */	break;
		case V4L2_PIX_FMT_GREY: 	gv4l2_debug("GREY"); 	break;  

#ifdef V4L2_PIX_FMT_Y16
		case V4L2_PIX_FMT_Y16: 		gv4l2_debug("Y16"); 	break;   
#endif
		case V4L2_PIX_FMT_PAL8: 	gv4l2_debug("PAL8"); 	break;   
		case V4L2_PIX_FMT_YVU410: 	gv4l2_debug("YVU410"); 	break;
		case V4L2_PIX_FMT_YVU420: 	gv4l2_debug("YVU420"); 	break; 

		case V4L2_PIX_FMT_YUV420: 	
			//gv4l2_debug("YUV420");
			yuv420p_to_rgb (start,THIS->frame,w,h,3);
			return;
		case V4L2_PIX_FMT_YUYV: 	
			//gv4l2_debug("YUYV");
			convert_yuv_to_rgb_buffer(start,THIS->frame,w,h);
			return;

		case V4L2_PIX_FMT_UYVY: 	gv4l2_debug("UYVY"); 	break;   
		case V4L2_PIX_FMT_YUV422P: 	gv4l2_debug("YUV422P"); break;
		case V4L2_PIX_FMT_YUV411P: 	gv4l2_debug("YUV411P"); break;
		case V4L2_PIX_FMT_Y41P: 	gv4l2_debug("Y41P"); 	break;   
		case V4L2_PIX_FMT_YUV444: 	gv4l2_debug("YUV444"); 	break; 
		case V4L2_PIX_FMT_YUV555: 	gv4l2_debug("YUV555"); 	break; 
		case V4L2_PIX_FMT_YUV565: 	gv4l2_debug("YUV565"); 	break; 
		case V4L2_PIX_FMT_YUV32: 	gv4l2_debug("YUV32"); 	break;  
		case V4L2_PIX_FMT_NV12: 	gv4l2_debug("NV12"); 	break;   
		case V4L2_PIX_FMT_NV21: 	gv4l2_debug("NV21"); 	break;   
		case V4L2_PIX_FMT_YUV410: 	gv4l2_debug("YUV410"); 	break; 
		case V4L2_PIX_FMT_YYUV: 	gv4l2_debug("YYUV"); 	break;   
		case V4L2_PIX_FMT_HI240: 	gv4l2_debug("HI240"); 	break;  
		case V4L2_PIX_FMT_HM12: 	gv4l2_debug("HM12"); 	break;   
		case V4L2_PIX_FMT_SBGGR8: 	gv4l2_debug("SBGGR8"); 	break; 
#ifdef V4L2_PIX_FMT_SGBRG8
		case V4L2_PIX_FMT_SGBRG8: 	gv4l2_debug("SBGRG8"); 	break; 
#endif
#ifdef V4L2_PIX_FMT_SBGGR16
		case V4L2_PIX_FMT_SBGGR16: 	gv4l2_debug("SBGGR16"); break;
#endif

		case V4L2_PIX_FMT_MJPEG: 	gv4l2_debug("MJPEG"); 	break;  
		case V4L2_PIX_FMT_JPEG: 	gv4l2_debug("JPEG"); 	break;   
		case V4L2_PIX_FMT_DV: 		gv4l2_debug("DV"); 	break;     
		case V4L2_PIX_FMT_MPEG: 	gv4l2_debug("MPEG"); 	break;   
		case V4L2_PIX_FMT_WNVA: 	gv4l2_debug("WNVA"); 	break;   
		case V4L2_PIX_FMT_SN9C10X: 	gv4l2_debug("SN9C10X"); break;
		case V4L2_PIX_FMT_PWC1: 	gv4l2_debug("PWC1"); 	break;   
		case V4L2_PIX_FMT_PWC2: 	gv4l2_debug("PWC2"); 	break;   
		case V4L2_PIX_FMT_ET61X251: 	gv4l2_debug("ET61X251");break;
#ifdef V4L2_PIX_FMT_SPCA501
		case V4L2_PIX_FMT_SPCA501: 	gv4l2_debug("SPCA501"); break; 
#endif
#ifdef V4L2_PIX_FMT_SPCA505
		case V4L2_PIX_FMT_SPCA505: 	gv4l2_debug("SPCA505"); break; 
#endif
#ifdef V4L2_PIX_FMT_SPCA508
		case V4L2_PIX_FMT_SPCA508: 	gv4l2_debug("SPCA508"); break; 
#endif
#ifdef V4L2_PIX_FMT_SPCA561
		case V4L2_PIX_FMT_SPCA561: 	gv4l2_debug("SPCA561"); break; 
#endif
#ifdef V4L2_PIX_FMT_PAC207
		case V4L2_PIX_FMT_PAC207: 	gv4l2_debug("PAC207"); 	break;  
#endif
#ifdef V4L2_PIX_FMT_PJPG
		case V4L2_PIX_FMT_PJPG: 	gv4l2_debug("PJPG"); 	break;    
#endif
#ifdef V4L2_PIX_FMT_YVYU
		case V4L2_PIX_FMT_YVYU: 	gv4l2_debug("YVYU"); 	break;    
#endif

		default:
			gv4l2_debug("Frame in unknown format. Format:0x" + format);
			break;
	}
	memcpy(THIS->frame,start,size);
}
#endif

//=============================================================================
//
//	gv4l2_read_frame( THIS )
//
//	Read a frame from the camera / video device
//
//	FIXME:: test non mmap mode!
//
int gv4l2_read_frame( CWEBCAM * _object )
{
	struct v4l2_buffer buf;

	if( !THIS->use_mmap ) {
		gv4l2_debug("Using READ interface");
		if( read (THIS->io, THIS->buffers[0].start, THIS->buffers[0].length) == -1) {
                        switch (errno) {
	                        case EAGAIN:
	                                return 0;
                        case EIO:
                                /* Could ignore EIO, see spec. */
                                /* fall through */
                        default:
				gv4l2_debug("READ ERROR");
                        }
                }
                gv4l2_process_image (THIS,THIS->buffers[0].start);
                return 1;
	}
	//
	//	This is the MMAP based read code
	//
	MCLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if( gv4l2_xioctl( THIS->io, VIDIOC_DQBUF, &buf) == -1 ) {
		gv4l2_debug("DQBUF Error");
		switch (errno) {
			case EAGAIN:
				gv4l2_debug("EAGAIN");
				return 0;
                        case EIO:
                                /* Could ignore EIO, see spec. */
                                /* fall through */
                        default:
				gv4l2_debug("VIDIOC_DQBUF READ ERROR");
		}
	}
	assert (buf.index < THIS->buffer_count);
	gv4l2_process_image (THIS,THIS->buffers[buf.index].start);

	if( gv4l2_xioctl( THIS->io, VIDIOC_QBUF, &buf) == -1 ) {
		gv4l2_debug("VIDIOC_QBUF READ ERROR");		
		return 0;
	}
	return 1;
}
//=============================================================================
//
//	gv4l2_resize( THIS , Width , Height )
//
//	Resize the display.
//	Going to cheat a little here, easy way is to completely deactivate
//	and let it start up with a new width and height .. :)
//
int gv4l2_resize( CWEBCAM * _object , int width , int height )
{
	if(! gv4l2_stop_capture( THIS ) ) {
		GB.Error("Failed to stop capturing on device");
		return 0;
	}
	gv4l2_uninit_device( THIS );

	if( close(THIS->io  ) == -1 ) {
		gv4l2_debug("error closing device");
	}

	if ( !gv4l2_open_device( THIS->device )){
	  GB.Error("Unable to reopen the device");
		return 0;
	}
	
	if( !gv4l2_init_device(THIS , width , height ) ) {
		GB.Error("Unable to initialise the device");
		return 0;
	}
	
	gv4l2_start_capture( THIS );        
	return 1;
}
