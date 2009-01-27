/***************************************************************************

  image.c

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __IMAGE_C

#include "image.h"

#define DEBUG_CONVERT

static int _default_format = GB_IMAGE_RGBA;

static inline unsigned char *GET_END_POINTER(GB_IMG *image)
{
	return &image->data[IMAGE_size(image)];
}

static inline uint PREMUL(uint x) 
{
	uint a = x >> 24;
	
	if (a == 0)
		return 0;
	else if (a == 0xFF)
		return x;
	
	uint t = (x & 0xff00ff) * a;
	t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
	t &= 0xff00ff;

	x = ((x >> 8) & 0xff) * a;
	x = (x + ((x >> 8) & 0xff) + 0x80);
	x &= 0xff00;
	x |= t | (a << 24);
	return x;
}

static inline uint INV_PREMUL(uint p)
{
	if (ALPHA(p) == 0)
		return 0;
	else if (ALPHA(p) == 0xFF)
		return p;
	else
	return
		((ALPHA(p) << 24)
		| (((255*RED(p))/ ALPHA(p)) << 16)
		| (((255*GREEN(p)) / ALPHA(p)) << 8)
		| ((255*BLUE(p)) / ALPHA(p)));
}

static inline uint SWAP(uint p)
{
	return RGBA(ALPHA(p), BLUE(p), GREEN(p), RED(p));
}

static inline uint SWAP_RED_BLUE(uint p)
{
	return RGBA(BLUE(p), GREEN(p), RED(p), ALPHA(p));
}

static uint from_GB_COLOR(GB_COLOR col, int format)
{
	col ^= 0xFF000000;
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = PREMUL(col);
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	if (GB_IMAGE_FMT_IS_RGBA(format))
		col = SWAP_RED_BLUE(col);
	return col;
}

static GB_COLOR to_GB_COLOR(uint col, int format)
{
	if (GB_IMAGE_FMT_IS_RGBA(format))
		col = SWAP_RED_BLUE(col);
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = INV_PREMUL(col);
	return col ^ 0xFF000000;
}

static inline uint BGRA_to_format(uint col, int format)
{
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = PREMUL(col);
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	return col;
}

static inline uint BGRA_from_format(uint col, int format)
{
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = INV_PREMUL(col);
	return col;
}

static inline bool is_valid(GB_IMG *img, int x, int y)
{
	return !(x >= img->width || y >= img->height || x < 0 || y < 0);
}

static void free_image(GB_IMG *img, void *image)
{
	//fprintf(stderr, "free_image: %p %p\n", img, img->data);
	GB.Free(POINTER(&img->data));
}

static GB_IMG_OWNER _image_owner = {
	"gb.image",
	0,
	free_image,
	free_image,
	NULL
	};


// Only converts to the following formats:
// - GB_IMAGE_BGRA
// - GB_IMAGE_BGRX
// - GB_IMAGE_RGBA
// - GB_IMAGE_RGBX

static void convert_image(uchar *dst, int dst_format, uchar *src, int src_format, int w, int h)
{
	unsigned char *s = src;
	unsigned char *d = dst;
	int len;
	unsigned char *dm;
	uint *p, *pm;
	bool psrc, pdst;

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: src_format = %d dst_format = %d\n", src_format, dst_format);
	#endif

	len = w * h * sizeof(uint);
	dm = &d[len];

	psrc = GB_IMAGE_FMT_IS_PREMULTIPLIED(src_format);
	src_format = GB_IMAGE_FMT_CLEAR_PREMULTIPLIED(src_format);

	pdst = GB_IMAGE_FMT_IS_PREMULTIPLIED(dst_format);
	dst_format = GB_IMAGE_FMT_CLEAR_PREMULTIPLIED(dst_format);

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: after: src_format = %d dst_format = %d\n", src_format, dst_format);
	#endif

  if (dst_format == GB_IMAGE_BGRA || dst_format == GB_IMAGE_BGRX)
  {
    switch (src_format)
    {
      case GB_IMAGE_BGRA: case GB_IMAGE_BGRX: 
        goto __0123;
  
      case GB_IMAGE_ARGB: case GB_IMAGE_XRGB:
        goto __3210;
  
      case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
        goto __2103;
  
      case GB_IMAGE_ABGR: case GB_IMAGE_XBGR:
        goto __1230;
        
      case GB_IMAGE_BGR:
        goto __012X;
  
      case GB_IMAGE_RGB:
        goto __210X;
    }
  }
  else if (dst_format == GB_IMAGE_RGBA || dst_format == GB_IMAGE_RGBX)
  {
    switch (src_format)
    {
      case GB_IMAGE_RGBA: case GB_IMAGE_RGBX:
        goto __0123;
  
      case GB_IMAGE_ABGR: case GB_IMAGE_XBGR:
        goto __3210;
  
      case GB_IMAGE_BGRA: case GB_IMAGE_BGRX:
        goto __2103;
  
      case GB_IMAGE_ARGB: case GB_IMAGE_XRGB:
        goto __1230;
  
      case GB_IMAGE_RGB:
        goto __012X;
      
      case GB_IMAGE_BGR:
        goto __210X;  
    }  
  }
  
__0123:         

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: 0123\n");
	#endif
  memcpy(dst, src, len);
  goto __PREMULTIPLIED;

__3210:

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: 3210\n");
	#endif
  while (d != dm)
  {
    d[0] = s[3];
    d[1] = s[2];
    d[2] = s[1];
    d[3] = s[0];
    s += 4;
    d += 4;
  }
  goto __PREMULTIPLIED;

__2103:

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: 2103\n");
	#endif

  while (d != dm)
  {
    d[0] = s[2];
    d[1] = s[1];
    d[2] = s[0];
    d[3] = s[3];
    s += 4;
    d += 4;
  }
  goto __PREMULTIPLIED;
  
__1230:

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: 1230\n");
	#endif
	
  while (d != dm)
  {
    d[0] = s[1];
    d[1] = s[2];
    d[2] = s[3];
    d[3] = s[0];
    s += 4;
    d += 4;
  }
  goto __PREMULTIPLIED;

__012X:

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: 012X\n");
	#endif
	
  while (d != dm)
  {
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = 0xFF;
    s += 3;
    d += 4;
  }
  return;

__210X:

	#ifdef DEBUG_CONVERT
	fprintf(stderr, "convert_image: 210X\n");
	#endif
	
  while (d != dm)
  {
    d[0] = s[2];
    d[1] = s[1];
    d[2] = s[0];
    d[3] = 0xFF;
    s += 3;
    d += 4;
  }
  return;
  
__PREMULTIPLIED:

	if (psrc == pdst)
		return;
	
	p = (uint *)dst;
	pm = (uint *)dm;
	
	if (psrc)
	{
		#ifdef DEBUG_CONVERT
		fprintf(stderr, "convert_image: premultiplied -> normal\n");
		#endif
		// convert premultiplied to normal
		while (p != pm)
		{
			*p = INV_PREMUL(*p);
			p++;
		}
	}
	else
	{
		#ifdef DEBUG_CONVERT
		fprintf(stderr, "convert_image: normal -> premultiplied\n");
		#endif
		// convert normal to premultiplied
		while (p != pm)
		{
			*p = PREMUL(*p);
			p++;
		}
	}
}

int IMAGE_size(GB_IMG *img)
{
	return img->width * img->height * (GB_IMAGE_FMT_IS_24_BITS(img->format) ? 3 : 4);
}

void IMAGE_create(GB_IMG *img, int width, int height, int format)
{
	if (width <= 0 || height <= 0)
	{
		GB_BASE save = img->ob;
		CLEAR(img);
		img->ob = save;
		img->owner = &_image_owner;
		return;
	}
	
	img->width = width;
	img->height = height;
	img->format = format;
	GB.Alloc(POINTER(&img->data), IMAGE_size(img));
	img->owner = &_image_owner;
	img->owner_handle = img->data;
}

void IMAGE_create_with_data(GB_IMG *img, int width, int height, int format, unsigned char *data)
{
	IMAGE_create(img, width, height, format);
	memcpy(img->data, data, IMAGE_size(img));
}

void IMAGE_convert(GB_IMG *img, int dst_format)
{
	uchar *data;
	int src_format = img->format;
	
	if (src_format == dst_format)
		return;
	
	//IMAGE_create(&tmp, img->width, img->height, format);
	img->format = dst_format;
	GB.Alloc(POINTER(&data), IMAGE_size(img));
	convert_image(data, dst_format, img->data, src_format, img->width, img->height);
	//GB.Free(POINTER(&img->data));
	IMAGE_take(img, &_image_owner, data, img->width, img->height, data);
}

// Check if a temporary handle is needed, and create it if needed by calling the owner "temp" function

void *IMAGE_check(GB_IMG *img, GB_IMG_OWNER *temp_owner)
{
	if (!img)
		return NULL;
		
	// If we already have the temporary handle, then do nothing
	if (img->temp_owner == temp_owner)
		return img->temp_handle;
	
	// If somebody else has a temporary handle
	if (img->temp_owner)
	{
		// release it only if it is not the owner
		if (img->temp_owner != img->owner && img->temp_owner->release)
			(*img->temp_owner->release)(img, img->temp_handle);
		img->temp_handle = 0;
	}
	
	// Get the temporary handle
	if (temp_owner)
	{
		// If we are the owner, we must use our owner handle as temporary handle
		if (img->owner == temp_owner)
			img->temp_handle = img->owner_handle;
		// If we are not the owner, then we will have to create the temporary handle ourself
		else
		{
			// Conversion can make gb.image the new owner and temporary owner
			IMAGE_convert(img, temp_owner->format);
			img->temp_handle = (*temp_owner->temp)(img);
		}
	}
	
	// Become the temporary owner
	img->temp_owner = temp_owner;
	
	return img->temp_handle;
}

// Take ownership of the image

void IMAGE_take(GB_IMG *img, GB_IMG_OWNER *owner, void *owner_handle, int width, int height, unsigned char *data)
{
	if (!img)
		return;
		
	// If we are already the owner with the same handle, then do nothing
	if (img->owner == owner && img->owner_handle == owner_handle)
		return;
	
	// Release the old owner
	//fprintf(stderr, "releasing image %p owned by %s\n", img, img->owner->name);
	(*img->owner->free)(img, img->owner_handle);
	
	// If we have the temporary handle too, then clean it as it is necessarily the same
	if (img->temp_owner == img->owner)
	{
		img->temp_owner = NULL;
		img->temp_handle = 0;
	}
	
	// Become the owner
	img->owner = owner;
	img->owner_handle = owner_handle;
	
	// As we are now the owner, then we must have the temporary handle too
	IMAGE_check(img, NULL);
	img->temp_owner = owner;
	img->temp_handle = owner_handle;
	
	// Initialize the data
	img->width = width;
	img->height = height;
	img->data = data;
	if (owner && owner->format)
		img->format = owner->format;
}

void IMAGE_delete(GB_IMG *img)
{
	IMAGE_take(img, NULL, NULL, 0, 0, NULL);
	img->format = 0;
}

#define GET_POINTER(_img, _p, _pm) \
	uint *_p = (uint *)(_img)->data; \
	uint *_pm = (uint *)GET_END_POINTER(_img);

void IMAGE_fill(GB_IMG *img, GB_COLOR col)
{
	GET_POINTER(img, p, pm);
	
	col = from_GB_COLOR(col, img->format);
	
	while (p != pm)
		*p++ = col;	
}

// GB_IMAGE_RGB[APX] only
void IMAGE_make_gray(GB_IMG *img)
{
	GET_POINTER(img, p, pm);
	uint col;
	uchar g;

	while (p != pm) 
	{
		col = BGRA_from_format(*p, img->format);
		g = (((RED(col) + BLUE(col)) >> 1) + GREEN(col)) >> 1;
		
		*p++ = BGRA_to_format(RGBA(g, g, g, ALPHA(col)), img->format);
	}
}

GB_COLOR IMAGE_get_pixel(GB_IMG *img, int x, int y)
{
	uint col;
	
  if (!is_valid(img, x, y))
  	return (-1);
  
  col = ((uint *)img->data)[y * img->width + x];
  return to_GB_COLOR(col, img->format);
}

void IMAGE_set_pixel(GB_IMG *img, int x, int y, GB_COLOR col)
{
  if (!is_valid(img, x, y))
  	return;
  
  ((uint *)img->data)[y * img->width + x] = from_GB_COLOR(col, img->format);
}

void IMAGE_replace(GB_IMG *img, GB_COLOR src, GB_COLOR dst, bool noteq)
{
	GET_POINTER(img, p, pm);

  src = from_GB_COLOR(src, img->format);
  dst = from_GB_COLOR(dst, img->format);

	if (noteq)
	{
		while (p != pm)
		{
			if (*p != src)
				*p = dst;
			p++;
		}
	}
	else
	{
		while (p != pm)
		{
			if (*p == src)
				*p = dst;
			p++;
		}
	}
}

// Comes from the GIMP

typedef
	struct {
		float r;
		float b;
		float g;
		float a;
		}
	FLOAT_RGB;

static void color_to_alpha(FLOAT_RGB *src, const FLOAT_RGB *color)
{
  FLOAT_RGB alpha;

  alpha.a = src->a;

  if (color->r < 0.0001)
    alpha.r = src->r;
  else if (src->r > color->r)
    alpha.r = (src->r - color->r) / (1.0 - color->r);
  else if (src->r < color->r)
    alpha.r = (color->r - src->r) / color->r;
  else alpha.r = 0.0;

  if (color->g < 0.0001)
    alpha.g = src->g;
  else if (src->g > color->g)
    alpha.g = (src->g - color->g) / (1.0 - color->g);
  else if (src->g < color->g)
    alpha.g = (color->g - src->g) / (color->g);
  else alpha.g = 0.0;

  if (color->b < 0.0001)
    alpha.b = src->b;
  else if (src->b > color->b)
    alpha.b = (src->b - color->b) / (1.0 - color->b);
  else if (src->b < color->b)
    alpha.b = (color->b - src->b) / (color->b);
  else alpha.b = 0.0;

  if (alpha.r > alpha.g)
    {
      if (alpha.r > alpha.b)
        {
          src->a = alpha.r;
        }
      else
        {
          src->a = alpha.b;
        }
    }
  else if (alpha.g > alpha.b)
    {
      src->a = alpha.g;
    }
  else
    {
      src->a = alpha.b;
    }

  if (src->a < 0.0001)
    return;

  src->r = (src->r - color->r) / src->a + color->r;
  src->g = (src->g - color->g) / src->a + color->g;
  src->b = (src->b - color->b) / src->a + color->b;

  src->a *= alpha.a;
}

void IMAGE_make_transparent(GB_IMG *img, GB_COLOR col)
{
	uint *p = (uint *)img->data;
	uint *pm = (uint *)(img->data + IMAGE_size(img));
	uint color;
	FLOAT_RGB rgb_color;
	FLOAT_RGB rgb_src;

	color = from_GB_COLOR(col, img->format);
	rgb_color.b = BLUE(color) / 255.0;
	rgb_color.g = GREEN(color) / 255.0;
	rgb_color.r = RED(color) / 255.0;
	rgb_color.a = 1.0;

	while (p != pm) 
	{
		color = BGRA_from_format(*p, img->format);
		rgb_src.b = BLUE(color) / 255.0;
		rgb_src.g = GREEN(color) / 255.0;
		rgb_src.r = RED(color) / 255.0;
		rgb_src.a = ALPHA(color) / 255.0;
		
		color_to_alpha(&rgb_src, &rgb_color);
	
		color = RGBA(
			(unsigned char)(255.0 * rgb_src.r + 0.5),
			(unsigned char)(255.0 * rgb_src.g + 0.5),
			(unsigned char)(255.0 * rgb_src.b + 0.5),
			(unsigned char)(255.0 * rgb_src.a + 0.5)
			);
	
		*p = BGRA_to_format(color, img->format);
		p++;
	}
}


void IMAGE_set_default_format(int format)
{
	_default_format = GB_IMAGE_FMT_CLEAR_PREMULTIPLIED(format);
}

int IMAGE_get_default_format()
{
	return _default_format;
}
