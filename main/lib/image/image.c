/***************************************************************************

  image.c

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __IMAGE_C

#include "c_color.h"
#include "image.h"

bool IMAGE_debug = FALSE;

typedef
	struct { unsigned char d[3]; } PACKED uint24;

typedef
	struct {
		int format;
		const char *name;
	}
	FORMAT;
	
//#define DEBUG_CONVERT
//#define DEBUG_ME 1

static int _default_format = GB_IMAGE_RGBA;

static FORMAT _formats[] = 
{
	{ GB_IMAGE_BGRX, "BGRX" },
	{ GB_IMAGE_XRGB, "XRGB" },
	{ GB_IMAGE_RGBX, "RGBX" },
	{ GB_IMAGE_XBGR, "XBGR" },
	{ GB_IMAGE_BGR , "BGR" },
	{ GB_IMAGE_RGB , "RGB" },

	{ GB_IMAGE_BGRA, "BGRA" },
	{ GB_IMAGE_ARGB, "ARGB" },
	{ GB_IMAGE_RGBA, "RGBA" },
	{ GB_IMAGE_ABGR, "ABGR" },

	{ GB_IMAGE_BGRP, "BGRP" },
	{ GB_IMAGE_PRGB, "PRGB" },
	{ GB_IMAGE_RGBP, "RGBP" },
	{ GB_IMAGE_PBGR, "PBGR" },
	{ 0, NULL }
};

/*static inline unsigned char *GET_END_POINTER(GB_IMG *image)
{
	return &image->data[IMAGE_size(image)];
}*/

#define GET_END_POINTER(_img) (&(_img)->data[IMAGE_size(_img)])

#define PREMUL(_x)  \
({ \
	uint x = (_x); \
	uint a = x >> 24; \
	\
	if (a == 0) \
		x = 0; \
	else if (a != 0xFF) \
	{ \
		uint t = (x & 0xFF00FF) * a; \
		t = (t + ((t >> 8) & 0xFF00FF) + 0x800080) >> 8; \
		t &= 0xff00ff; \
		\
		x = ((x >> 8) & 0xff) * a; \
		x = (x + ((x >> 8) & 0xFF) + 0x80); \
		x &= 0xFF00; \
		x |= t | (a << 24); \
	} \
	x; \
})

#define INV_PREMUL(__p) \
({ \
	uint _p = (__p); \
	if (ALPHA(_p) == 0) \
		_p = 0; \
	else if (ALPHA(_p) != 0xFF) \
		_p = ((ALPHA(_p) << 24) \
				| (((255*RED(_p))/ ALPHA(_p)) << 16) \
				| (((255*GREEN(_p)) / ALPHA(_p)) << 8) \
				| ((255*BLUE(_p)) / ALPHA(_p))); \
	_p; \
})

#define SWAP(__p) \
({ \
	uint _p = (__p); \
	RGBA(ALPHA(_p), BLUE(_p), GREEN(_p), RED(_p)); \
})

#define SWAP_RED_BLUE(__p) \
({ \
	uint _p = (__p); \
	RGBA(BLUE(_p), GREEN(_p), RED(_p), ALPHA(_p)); \
})

// Convert from GB_COLOR to a specific format

static uint GB_COLOR_to_format(GB_COLOR col, int format)
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

// Convert from a specific format to GB_COLOR

static GB_COLOR GB_COLOR_from_format(uint col, int format)
{
	if (GB_IMAGE_FMT_IS_RGBA(format))
		col = SWAP_RED_BLUE(col);
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = INV_PREMUL(col);
	return col ^ 0xFF000000;
}

// Convert from BGRA to a specific format

static inline uint BGRA_to_format(uint col, int format)
{
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = PREMUL(col);
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	if (GB_IMAGE_FMT_IS_RGBA(format))
		col = SWAP_RED_BLUE(col);
	return col;
}

// Convert from a specific format to BGRA

static inline uint BGRA_from_format(uint col, int format)
{
	if (GB_IMAGE_FMT_IS_RGBA(format))
		col = SWAP_RED_BLUE(col);
	if (GB_IMAGE_FMT_IS_SWAPPED(format))
		col = SWAP(col);
	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(format))
		col = INV_PREMUL(col);
	return col;
}

// Convert from GB_COLOR to BGRA

static inline uint GB_COLOR_to_BGRA(GB_COLOR col)
{
	return col ^ 0xFF000000;
}

// Compose two BGRA colors

static inline uint BGRA_compose(uint dst, uint src)
{
	unsigned char a = ALPHA(src);
	if (a == 255)
		return src;
	else if (a == 0)
		return dst;
	else 
	{
		unsigned char r = ((RED(src) - RED(dst)) * a) / 256 + RED(dst);
		unsigned char g = ((GREEN(src) - GREEN(dst)) * a) / 256 + GREEN(dst);
		unsigned char b = ((BLUE(src) - BLUE(dst)) * a) / 256 + BLUE(dst);
		if (ALPHA(dst) > a)
			a = ALPHA(dst);
		return RGBA(r, g, b, a);
	}
}

static inline bool is_valid(GB_IMG *img, int x, int y)
{
	return !(x >= img->width || y >= img->height || x < 0 || y < 0);
}

static void free_image(GB_IMG *img, void *image)
{
	//fprintf(stderr, "free_image: %p %p : %d\n", img, img->data, IMAGE_size(img));
	GB.Free(POINTER(&img->data));
}

static GB_IMG_OWNER _image_owner = {
	"gb.image",
	0,
	free_image,
	free_image,
	NULL,
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

#define SYNCHRONIZE(_img) ({ if ((_img)->sync && (_img)->temp_owner) (*(_img)->temp_owner->sync)(_img); })
#define MODIFY(_img) ((_img)->modified = TRUE)

int IMAGE_size(GB_IMG *img)
{
	return img->width * img->height * (GB_IMAGE_FMT_IS_24_BITS(img->format) ? 3 : 4);
}

void IMAGE_create(GB_IMG *img, int width, int height, int format)
{
	GB_BASE save = img->ob;
	CLEAR(img);	
	img->ob = save;
	img->owner = &_image_owner;

	if (width <= 0 || height <= 0)
	{
		img->is_void = TRUE;
		return;
	}
	
	img->width = width;
	img->height = height;
	img->format = format;
	GB.Alloc(POINTER(&img->data), IMAGE_size(img));
	img->owner_handle = img->data;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "IMAGE_create: %p\n", img);
	#endif
}


void IMAGE_create_with_data(GB_IMG *img, int width, int height, int format, unsigned char *data)
{
	IMAGE_create(img, width, height, format);
	if (data && !IMAGE_is_void(img))
		memcpy(img->data, data, IMAGE_size(img));
}

const char *IMAGE_format_to_string(int fmt)
{
	FORMAT *pf;
	
	for (pf = _formats; pf->name; pf++)
	{
		if (fmt == pf->format)
			return pf->name;
	}
	
	return NULL;
}

void IMAGE_convert(GB_IMG *img, int dst_format)
{
	uchar *data;
	int src_format = img->format;
	
	if (src_format == dst_format)
		return;
	
	//IMAGE_create(&tmp, img->width, img->height, format);
	img->format = dst_format;
	if (IMAGE_is_void(img))
		return;
	
	if (IMAGE_debug)
		fprintf(stderr, "gb.image: convert: %s -> %s\n", IMAGE_format_to_string(src_format), IMAGE_format_to_string(dst_format));
	
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
	
	#ifdef DEBUG_ME
	fprintf(stderr, "IMAGE_check: %p: %s (%p) / %s (%p) -> %s\n", 
					img, 
				 img->owner->name, img->owner_handle, 
				 img->temp_owner ? img->temp_owner->name : "NULL", img->temp_handle, 
				 temp_owner ? temp_owner->name : "NULL");
	#endif
	
	// If somebody else has a temporary handle
	if (img->temp_owner)
	{
		// release it only if it is not the owner
		if (img->temp_owner != img->owner && img->temp_owner->release)
			(*img->temp_owner->release)(img, img->temp_handle);
		img->temp_handle = 0;
		img->temp_owner = NULL;
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
			// Synchronize the image if needed
			SYNCHRONIZE(img);
			// Conversion can make gb.image the new owner and temporary owner
			IMAGE_convert(img, temp_owner->format);
			img->temp_handle = (*temp_owner->temp)(img);
		}
	}
	
	// Become the temporary owner
	img->temp_owner = temp_owner;
	
	#ifdef DEBUG_ME
	fprintf(stderr, "==========>: %p: %s (%p) / %s (%p)\n", 
					img, 
				 img->owner->name, img->owner_handle, 
				 img->temp_owner ? img->temp_owner->name : "NULL", img->temp_handle);
	#endif
	
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
	img->is_void = width <= 0 || height <= 0;
}

void IMAGE_delete(GB_IMG *img)
{
	#ifdef DEBUG_ME
	fprintf(stderr, "IMAGE_delete: %p\n", img);
	#endif
	//IMAGE_take(img, &_image_owner, NULL, 0, 0, NULL);
	
	// Release the temporary handle before the owner, because the temporary owner may write to the data before freeing!
	
	if (img->temp_owner && img->temp_owner != img->owner && img->temp_handle)
		(*img->temp_owner->release)(img, img->temp_handle);
	
	if (!IMAGE_is_void(img))
		(*img->owner->free)(img, img->owner_handle);
	
	img->width = img->height = 0;
	img->format = 0;
	img->temp_owner = NULL;
	img->temp_handle = NULL;
	img->owner = &_image_owner;
	img->owner_handle = NULL;
	img->is_void = TRUE;
}

void IMAGE_synchronize(GB_IMG *img)
{
	SYNCHRONIZE(img);
}

#define GET_POINTER(_img, _p, _pm) \
	uint *_p = (uint *)(_img)->data; \
	uint *_pm = (uint *)GET_END_POINTER(_img); \
	if ((_img)->is_void) return;

void IMAGE_fill(GB_IMG *img, GB_COLOR col)
{
	GET_POINTER(img, p, pm);
	
	//SYNCHRONIZE(img); unneeded, as the entire image will be replaced
	
	col = GB_COLOR_to_format(col, img->format);
	//fprintf(stderr, "fill with %08X\n", col);
	while (p != pm)
		*p++ = col;	
	
	MODIFY(img);
}

// GB_IMAGE_RGB[APX] only
void IMAGE_make_gray(GB_IMG *img)
{
	GET_POINTER(img, p, pm);
	uint col;
	uchar g;
	int format = img->format;

	SYNCHRONIZE(img);
	
	while (p != pm) 
	{
		col = BGRA_from_format(*p, format);
		g = (((RED(col) + BLUE(col)) >> 1) + GREEN(col)) >> 1;
		
		*p++ = BGRA_to_format(RGBA(g, g, g, ALPHA(col)), format);
	}

	MODIFY(img);
}

GB_COLOR IMAGE_get_pixel(GB_IMG *img, int x, int y)
{
	uint col;
	
	if (!is_valid(img, x, y))
		return (-1);
	
	SYNCHRONIZE(img);
	col = ((uint *)img->data)[y * img->width + x];
	return GB_COLOR_from_format(col, img->format);
}

void IMAGE_get_pixels(GB_IMG *img, int *data)
{
	SYNCHRONIZE(img);
	
	memcpy(data, img->data, img->width * img->height * sizeof(int));
}

void IMAGE_set_pixel(GB_IMG *img, int x, int y, GB_COLOR col)
{
	if (!is_valid(img, x, y))
		return;
	
	SYNCHRONIZE(img);
	((uint *)img->data)[y * img->width + x] = GB_COLOR_to_format(col, img->format);
	MODIFY(img);
}

void IMAGE_set_pixels(GB_IMG *img, int *data)
{
	SYNCHRONIZE(img);
	memcpy(img->data, data, img->width * img->height * sizeof(int));
	MODIFY(img);
}

void IMAGE_fill_rect(GB_IMG *img, int x, int y, int w, int h, GB_COLOR col, bool opaque)
{
	uint *p;
	int i;
	uint c;
	int format = img->format;
	
	if (x >= img->width || y >= img->height) return;
	
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if ((x + w) > img->width) { w = img->width - x; }
	if ((y + h) > img->height) { h = img->height - y; }
	
	if (w <= 0 || h <= 0) return;
	
	SYNCHRONIZE(img);

	p = &((uint *)img->data)[y * img->width + x];
	
	c = GB_COLOR_to_BGRA(col);
	
	if (opaque || ALPHA(c) == 255)
	{
		c = BGRA_to_format(c, format);
		while (h)
		{
			for(i = w; i; i--)
				*p++ = c;
			h--;
			p += img->width - w;
		}
	}
	else
	{
		while (h)
		{
			for(i = w; i; i--, p++)
				*p = BGRA_to_format(BGRA_compose(BGRA_from_format(*p, format), c), format);
			h--;
			p += img->width - w;
		}
	}
	
	MODIFY(img);
}

void IMAGE_replace(GB_IMG *img, GB_COLOR src, GB_COLOR dst, bool noteq)
{
	GET_POINTER(img, p, pm);

	src = GB_COLOR_to_format(src, img->format);
	dst = GB_COLOR_to_format(dst, img->format);

	SYNCHRONIZE(img);

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
	
	MODIFY(img);
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
			src->a = alpha.r;
		else
			src->a = alpha.b;
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
	uint color;
	FLOAT_RGB rgb_color;
	FLOAT_RGB rgb_src;
	int format = img->format;
	GET_POINTER(img, p, pm);
	//uint *p = (uint *)img->data;
	//uint *pm = (uint *)(img->data + IMAGE_size(img));

	//fprintf(stderr, "IMAGE_make_transparent: %d x %d / %d\n", img->width, img->height, img->format);

	SYNCHRONIZE(img);

	color = GB_COLOR_to_BGRA(col);
	rgb_color.b = BLUE(color) / 255.0;
	rgb_color.g = GREEN(color) / 255.0;
	rgb_color.r = RED(color) / 255.0;
	rgb_color.a = 1.0;

	while (p != pm) 
	{
		color = BGRA_from_format(*p, format);
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
	
		*p = BGRA_to_format(color, format);
		//fprintf(stderr, "[%d] %08X\n", p - (uint *)img->data, *p);
		p++;
	}
	
	MODIFY(img);
	//fprintf(stderr, "IMAGE_make_transparent: ** DONE **\n");
}


void IMAGE_set_default_format(int format)
{
	_default_format = format; //GB_IMAGE_FMT_CLEAR_PREMULTIPLIED(format);
}

int IMAGE_get_default_format()
{
	return _default_format;
}

// Parameter correction
#define CHECK_PARAMETERS(dst, dx, dy, dw, dh, src, sx, sy, sw, sh) \
	if ( sw < 0 ) sw = src->width; \
	if ( sh < 0 ) sh = src->height; \
	if (dw < 0) dw = sw; \
	if (dh < 0) dh = sh; \
	if (dw != sw || dh != sh) \
	{ \
		GB.Error("Stretching images is not implemented in gb.image"); \
		return; \
	} \
	if ( sx < 0 ) { dx -= sx; dw += sx; sw += sx; sx = 0; } \
	if ( sy < 0 ) { dy -= sy; dh += sy; sh += sy; sy = 0; } \
	if ( dx < 0 ) { sx -= dx; sw += dx; dx = 0; } \
	if ( dy < 0 ) { sy -= dy; sh += dy; dy = 0; } \
	if ( (sx + sw) > src->width ) sw = src->width - sx; \
	if ( (sy + sh) > src->height ) sh = src->height - sy; \
	if ( (dx + sw) > dst->width ) sw = dst->width - dx; \
	if ( (dy + sh) > dst->height ) sh = dst->height - dy; \
	if (sw <= 0 || sh <= 0) \
		return;

void IMAGE_bitblt(GB_IMG *dst, int dx, int dy, int dw, int dh, GB_IMG *src, int sx, int sy, int sw, int sh)
{
	int sfmt = src->format;
	int dfmt = dst->format;
	
	CHECK_PARAMETERS(dst, dx, dy, dw, dh, src, sx, sy, sw, sh);
	
	SYNCHRONIZE(src);
	SYNCHRONIZE(dst);
		
	uint *d = (uint *)dst->data + dy * dst->width + dx;
	uint *s = (uint *)src->data + sy * src->width + sx;

	if (GB_IMAGE_FMT_IS_32_BITS(sfmt) && GB_IMAGE_FMT_IS_32_BITS(dfmt))
	{
		if (sfmt != dfmt)
		{
			const int dd = dst->width - sw;
			const int ds = src->width - sw;
			int t;
			while (sh--) 
			{
				for (t = sw; t--;)
				{
					*d = BGRA_to_format(BGRA_from_format(*s, sfmt), dfmt);
					d++;
					s++;
				}
				
				d += dd;
				s += ds;
			}
		}
		else if (sw < 64)
		{
			const int dd = dst->width - sw;
			const int ds = src->width - sw;
			int t;
			while (sh--) 
			{
				for (t = sw; t--;)
					*d++ = *s++;
				d += dd;
				s += ds;
			}
		} 
		else 
		{
			// Trust libc
			const int dd = dst->width;
			const int ds = src->width;
			const int b = sw * sizeof(uint);
			while (sh--) 
			{
				memcpy(d, s, b);
				d += dd;
				s += ds;
			}
		}
	}
	else if (GB_IMAGE_FMT_IS_24_BITS(sfmt) && GB_IMAGE_FMT_IS_24_BITS(sfmt))
	{
		char *d = (char *)dst->data + (dy * dst->width + dx) * 3;
		char *s = (char *)src->data + (sy * src->width + sx) * 3;
		const int dd = dst->width * 3;
		const int ds = src->width * 3;
		const int b = sw * 3;

		while (sh--) 
		{
			memcpy(d, s, b);
			d += dd;
			s += ds;
		}
	}
	else
	{
		GB.Error("The pixel size of both images must be the same");
	}
	
	MODIFY(dst);
}

void IMAGE_set_opacity(GB_IMG *dst, uchar opacity)
{
	if (!GB_IMAGE_FMT_IS_32_BITS(dst->format))
	{
		GB.Error("The image must have an alpha channel");
		return;
	}
	
	if (opacity == 255)
		return;

	SYNCHRONIZE(dst);
	
	GET_POINTER(dst, p, pm);

	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(dst->format))
	{
		uint *pp = p;
		while (pp != pm)
		{
			*pp = INV_PREMUL(*pp);
			pp++;
		}
	}
	
	uchar *d = (uchar *)p;
	uchar *dm = (uchar *)pm;
	
	if (!GB_IMAGE_FMT_IS_SWAPPED(dst->format))
	{
		d += 3;
		dm += 3;
	}
	
	if (opacity == 0)
	{
		while (d != dm)
		{
			*d = 0;
			d += 4;
		}
	}
	else
	{
		uchar da[256];
		int i;
		
		for (i = 0; i < 256; i++)
			da[i] = i * opacity >> 8;
	
		while (d != dm)
		{
			*d = da[*d];
			d += 4;
		}
	}

	if (GB_IMAGE_FMT_IS_PREMULTIPLIED(dst->format))
	{
		uint *pp = p;
		while (pp != pm)
		{
			*pp = PREMUL(*pp);
			pp++;
		}
	}
	
	MODIFY(dst);
}

void IMAGE_draw_alpha(GB_IMG *dst, int dx, int dy, GB_IMG *src, int sx, int sy, int sw, int sh)
{
	if (!GB_IMAGE_FMT_IS_32_BITS(src->format) || !GB_IMAGE_FMT_IS_32_BITS(dst->format))
	{
		GB.Error("The images must have an alpha channel");
		return;
	}

	CHECK_PARAMETERS(dst, dx, dy, sw, sh, src, sx, sy, sw, sh);
	
	SYNCHRONIZE(src);
	SYNCHRONIZE(dst);
		
	uchar *d = (uchar *)((uint *)dst->data + dy * dst->width + dx);
	uchar *s = (uchar *)((uint *)src->data + sy * src->width + sx);

	const int dd = (dst->width - sw) * 4;
	const int ds = (src->width - sw) * 4;
	//uint cs, cd;
	int sformat = src->format;
	int dformat = dst->format;
	int t;
	
	if (!GB_IMAGE_FMT_IS_SWAPPED(sformat))
		s += 3;
	if (!GB_IMAGE_FMT_IS_SWAPPED(dformat))
		d += 3;
	
	while (sh--)
	{
		for (t = sw; t--; d += 4,s += 4)
		{
			if (*s < *d)
				*d = *s;
		}
		
		d += dd;
		s += ds;
	}

	MODIFY(dst);
}

void IMAGE_compose(GB_IMG *dst, int dx, int dy, int dw, int dh, GB_IMG *src, int sx, int sy, int sw, int sh)
{
	if (dst->format != src->format)
	{
		GB.Error("The images must have the same format");
		return;
	}

	CHECK_PARAMETERS(dst, dx, dy, dw, dh, src, sx, sy, sw, sh);
	
	SYNCHRONIZE(src);
	SYNCHRONIZE(dst);

	switch(src->format)
	{
		case GB_IMAGE_RGBA: case GB_IMAGE_BGRA:
		{
			#if 0
			uint *d = (uint *)dst->data + dy * dst->width + dx;
			uint *s = (uint *)src->data + sy * src->width + sx;
	
			const int dd = dst->width - sw;
			const int ds = src->width - sw;
			int t;
			while (sh--) 
			{
				for (t = sw; t--;)
				{
					unsigned char a = ALPHA(*s);
					if (a == 255)
						*d = *s;
					else if (a)
					{
						unsigned char r = ((RED(*s)-RED(*d)) * a) / 256 + RED(*d);
						unsigned char g = ((GREEN(*s)-GREEN(*d)) * a) / 256 + GREEN(*d);
						unsigned char b = ((BLUE(*s)-BLUE(*d)) * a) / 256 + BLUE(*d);
						if (ALPHA(*d) > a)
							a = ALPHA(*d);
						*d = RGBA(r,g,b,a);
					}
					d++;
					s++;
				}
				d += dd;
				s += ds;
			}
			#else
			uchar *d = (uchar *)((uint *)dst->data + dy * dst->width + dx);
			uchar *s = (uchar *)((uint *)src->data + sy * src->width + sx);
	
			const int dd = (dst->width - sw) * 4;
			const int ds = (src->width - sw) * 4;
			int t;
			while (sh--) 
			{
				for (t = sw; t--;)
				{
					unsigned char a = s[3];
					if (a == 255)
						*(uint *)d = *(uint *)s;
					else if (a)
					{
						d[0] = ((s[0] - d[0]) * a) / 256 + d[0];
						d[1] = ((s[1] - d[1]) * a) / 256 + d[1];
						d[2] = ((s[2] - d[2]) * a) / 256 + d[2];
						if (d[3] > a)
							d[3] = a;
					}
					d += 4;
					s += 4;
				}
				d += dd;
				s += ds;
			}
			#endif

			break;
		}
		
		default:
			GB.Error("Unsupported image format");
			return;
	}
	
	MODIFY(dst);
}

void IMAGE_colorize(GB_IMG *img, GB_COLOR color)
{
	GET_POINTER(img, p, pm);
	uint col;
	int h, s, v;
	int r, g, b;
	int hcol, scol, vcol;
	int format = img->format;
	uchar vcolmul[256];
	int i;
	
	SYNCHRONIZE(img);

	col = GB_COLOR_to_BGRA(color);
	COLOR_rgb_to_hsv(RED(col), GREEN(col), BLUE(col), &hcol, &scol, &vcol);
	
	for (i = 0; i < 256; i++)
		vcolmul[i] = vcol * i / 255;

	while (p != pm) 
	{
		col = BGRA_from_format(*p, format);
		COLOR_rgb_to_hsv(RED(col), GREEN(col), BLUE(col), &h, &s, &v);
		COLOR_hsv_to_rgb(hcol, scol, vcolmul[v], &r, &g, &b);
		*p++ = BGRA_to_format(RGBA(r, g, b, ALPHA(col)), img->format);
	}
	
	MODIFY(img);
}

void IMAGE_mask(GB_IMG *img, GB_COLOR color)
{
	GET_POINTER(img, p, pm);
	uint col;
	unsigned char red[256], blue[256], green[256], alpha[256];
	int i, r, g, b, a;
	int format = img->format;
	
	SYNCHRONIZE(img);

	col = GB_COLOR_to_format(color, img->format);
	r = RED(col);
	g = GREEN(col);
	b = BLUE(col);
	a = ALPHA(col);
	
	for (i = 0; i < 256; i++)
	{
		red[i] = i * r / 255;
		green[i] = i * g / 255;
		blue[i] = i * b / 255;
		alpha[i] = i * a / 255;
	}

	while (p != pm) 
	{
		col = BGRA_from_format(*p, format);
		*p++ = BGRA_to_format(RGBA(red[RED(col)], green[GREEN(col)], blue[BLUE(col)], alpha[ALPHA(col)]), format);
	}
	
	MODIFY(img);
}

void IMAGE_mirror(GB_IMG *src, GB_IMG *dst, bool horizontal, bool vertical)
{
	if (dst->width != src->width || dst->height != src->height || dst->format != src->format || IMAGE_is_void(src))
		return;

	int w = src->width;
	int h = src->height;

	int dxi = horizontal ? -1 : 1;
	int dxs = horizontal ? (w - 1) : 0;
	int dyi = vertical ? -1 : 1;
	int dy = vertical ? (h - 1) : 0;
	int sx, sy;

	SYNCHRONIZE(src);

	if (GB_IMAGE_FMT_IS_24_BITS(src->format))
	{
		for (sy = 0; sy < h; sy++, dy += dyi) 
		{
			uint24 *ssl = (uint24 *)(src->data + sy * src->width * 3);
			uint24 *dsl = (uint24 *)(dst->data + dy * dst->width * 3);
			int dx = dxs;
			for (sx = 0; sx < w; sx++, dx += dxi)
					dsl[dx] = ssl[sx];
		}
	}
	else 
	{
		for (sy = 0; sy < h; sy++, dy += dyi) 
		{
			uint *ssl = (uint *)(src->data + sy * src->width * 4);
			uint *dsl = (uint *)(dst->data + dy * dst->width * 4);
			int dx = dxs;
			for (sx = 0; sx < w; sx++, dx += dxi)
					dsl[dx] = ssl[sx];
		}
	}
	
	MODIFY(dst);
}

void IMAGE_rotate(GB_IMG *src, GB_IMG *dst, bool left)
{
	if (dst->width != src->height || dst->width != src->height || dst->format != src->format || IMAGE_is_void(src))
		return;

	int x, y;
	int w = dst->width;
	int h = dst->height;

	SYNCHRONIZE(src);

	if (GB_IMAGE_FMT_IS_24_BITS(src->format))
	{
		uint24 *pd = (uint24 *)dst->data;
		
		if (left)
		{
			
			for (y = 0; y < h; y++)
			{
				uint24 *ps = (uint24 *)(src->data + (h - 1 - y) * 3);
				
				for (x = 0; x < w; x++)
				{
					*pd++ = *ps;
					ps += h;
				}
			}
		}
		else
		{
			for (y = 0; y < h; y++)
			{
				uint24 *ps = (uint24 *)(src->data + ((w - 1) * h + y) * 3);
				
				for (x = 0; x < w; x++)
				{
					*pd++ = *ps;
					ps -= h;
				}
			}
		}
	}
	else 
	{
		uint *pd = (uint *)dst->data;
		
		if (left)
		{
			for (y = 0; y < h; y++)
			{
				uint *ps = (uint *)(src->data + (h - 1 - y) * 4);
				
				for (x = 0; x < w; x++)
				{
					*pd++ = *ps;
					ps += h;
				}
			}
		}
		else
		{
			for (y = 0; y < h; y++)
			{
				uint *ps = (uint *)(src->data + ((w - 1) * h + y) * 4);
				
				for (x = 0; x < w; x++)
				{
					*pd++ = *ps;
					ps -= h;
				}
			}
		}
	}
	
	MODIFY(dst);
}

#if 0
#define GET_RGBA(_col, _r, _g, _b, _a) { _r = RED(_col); _g = GREEN(_col); _b = BLUE(_col); _a = ALPHA(_col); }

void IMAGE_transform(GB_IMG *dst, GB_IMG *src, double sx, double sy, double sdx, double sdy)
{
	int x, y;
	double ssx;
	double ssy;
	uint col, cdp;
	int ix, iy;
	uchar r, g, b, a;
	uchar rc, gc, bc, ac;
	int wx, wy;
	
	uint *dp = (uint *)dst->data;
	memset(dp, 0, IMAGE_size(dst));
	
	dp += dst->width;
	
	for (y = 1; y < (dst->height - 1); y++)
	{
		ssx = sx;
		ssy = sy;
		
		dp++;
		for (x = 1; x < (dst->width - 1); x++)
		{
			ix = sx;
			iy = sy;
			
			if (is_valid(src, ix, iy))
			{
				col = BGRA_from_format(*((uint *)src->data + iy * src->width + ix), src->format);
				
				//dp[1] = RGBA(RED(col) * wx / 256, GREEN(col) * wx / 256, BLUE(col) * wx / 256, ALPHA(col) * wx / 256);

				wx = 128;
				
				cdp = BGRA_from_format(*dp, dst->format);
				dp[0] = RGBA(RED(cdp) + RED(col) * wx / 256, GREEN(cdp) + GREEN(col) * wx / 256, BLUE(cdp) + BLUE(col) * wx / 256, ALPHA(col));

				wx = 32;
				
				cdp = BGRA_from_format(dp[-1], dst->format);
				dp[-1] = RGBA(RED(cdp) + RED(col) * wx / 256, GREEN(cdp) + GREEN(col) * wx / 256, BLUE(cdp) + BLUE(col) * wx / 256, ALPHA(col));
				
				cdp = BGRA_from_format(dp[-dst->width], dst->format);
				dp[-dst->width] = RGBA(RED(cdp) + RED(col) * wx / 256, GREEN(cdp) + GREEN(col) * wx / 256, BLUE(cdp) + BLUE(col) * wx / 256, ALPHA(col));
				
				dp[1] = RGBA(RED(col) * wx / 256, GREEN(col) * wx / 256, BLUE(col) * wx / 256, 0);
				
				dp[dst->width] = dp[1]; //RGBA(RED(col) * wx / 256, GREEN(col) * wx / 256, BLUE(col) * wx / 256, ALPHA(col) * wx / 256);
			}

			sx += sdx;
			sy += sdy;
			dp++;
		}
		dp++;
		sx = ssx - sdy;
		sy = ssy + sdx;
	}
}
#endif

/*
StackBlur - a fast almost Gaussian Blur For Canvas

Version: 	0.5
Author:		Mario Klingemann
Contact: 	mario@quasimondo.com
Website:	http://www.quasimondo.com/StackBlurForCanvas
Twitter:	@quasimondo

In case you find this class useful - especially in commercial projects -
I am not totally unhappy for a small donation to my PayPal account
mario@quasimondo.de

Or support me on flattr: 
https://flattr.com/thing/72791/StackBlur-a-fast-almost-Gaussian-Blur-Effect-for-CanvasJavascript

Copyright (c) 2010 Mario Klingemann

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

static const short mul_table[] = {
	512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
	454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
	482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
	437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
	497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
	320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
	446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
	329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
	505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
	399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
	324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
	268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
	451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
	385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
	332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
	289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};
        
   
static const char shg_table[] = {
	9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
	17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
	19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
	20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
	21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
	23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

typedef
	struct _BLUR_STACK {
		uchar r, g, b, a;
		struct _BLUR_STACK *next;
	}
	BLUR_STACK;

void IMAGE_blur(GB_IMG *img, int radius) //top_x, top_y, width, height, radius )
{
	if (radius < 1)
		return;
	
	if (radius >= 255)
		radius = 254;
	
	if (img->is_void)
		return;
	
	SYNCHRONIZE(img);
	
	uchar *pixels = (uchar *)img->data;
	int width = img->width;
	int height = img->height;
	
	uint x, y, i, p, yp, yi, yw, r_sum, g_sum, b_sum, a_sum, 
	r_out_sum, g_out_sum, b_out_sum, a_out_sum,
	r_in_sum, g_in_sum, b_in_sum, a_in_sum, rbs;
	
	uchar pr, pg, pb, pa;
			
	uint div = radius + radius + 1;
	uint widthMinus1  = width - 1;
	uint heightMinus1 = height - 1;
	uint radiusPlus1  = radius + 1;
	uint sumFactor = radiusPlus1 * ( radiusPlus1 + 1 ) / 2;
	
	BLUR_STACK *stackStart, *stackEnd, *stack;

	stackStart = alloca(sizeof(BLUR_STACK) * div);
	
	stack = stackStart;
	stackEnd = stackStart + radiusPlus1;
	
	for (i = 1; i < div; i++)
	{
		stack->next = stack + 1;
		stack++;
	}
	
	stack->next = stackStart;
	
	BLUR_STACK *stackIn = NULL;
	BLUR_STACK *stackOut = NULL;
	
	yw = yi = 0;
	
	uint mul_sum = mul_table[radius];
	uint shg_sum = shg_table[radius];
	
	//fprintf(stderr, "blur: format = %d / %02X%02X%02X%02X\n", img->format, pixels[0], pixels[1], pixels[2], pixels[3]);

	if (GB_IMAGE_FMT_IS_32_BITS(img->format))
	{
		for (y = 0; y < height; y++)
		{
			r_in_sum = g_in_sum = b_in_sum = a_in_sum = r_sum = g_sum = b_sum = a_sum = 0;
			
			pr = pixels[yi];
			pg = pixels[yi + 1];
			pb = pixels[yi + 2];
			pa = pixels[yi + 3];
			
			r_out_sum = radiusPlus1 * pr;
			g_out_sum = radiusPlus1 * pg;
			b_out_sum = radiusPlus1 * pb;
			a_out_sum = radiusPlus1 * pa;
			
			r_sum += sumFactor * pr;
			g_sum += sumFactor * pg;
			b_sum += sumFactor * pb;
			a_sum += sumFactor * pa;
			
			stack = stackStart;
			
			for (i = 0; i < radiusPlus1; i++)
			{
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack->a = pa;
				stack = stack->next;
			}
			
			for (i = 1; i < radiusPlus1; i++)
			{
				p = yi + (( widthMinus1 < i ? widthMinus1 : i ) << 2 );
				rbs = radiusPlus1 - i;
				
				pr = pixels[p];
				pg = pixels[p + 1];
				pb = pixels[p + 2];
				pa = pixels[p + 3];
				
				r_sum += pr * rbs;
				g_sum += pg * rbs;
				b_sum += pb * rbs;
				a_sum += pa * rbs;
				
				r_in_sum += pr;
				g_in_sum += pg;
				b_in_sum += pb;
				a_in_sum += pa;
				
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack->a = pa;
				
				stack = stack->next;
			}
			
			
			stackIn = stackStart;
			stackOut = stackEnd;
			for (x = 0; x < width; x++)
			{
				pixels[yi] = (r_sum * mul_sum) >> shg_sum; // * 255 / pa;
				pixels[yi + 1] = (g_sum * mul_sum) >> shg_sum; // * 255 / pa;
				pixels[yi + 2] = (b_sum * mul_sum) >> shg_sum; // * 255 / pa;
				pixels[yi + 3] = (a_sum * mul_sum) >> shg_sum;
				
				r_sum -= r_out_sum;
				g_sum -= g_out_sum;
				b_sum -= b_out_sum;
				a_sum -= a_out_sum;
				
				r_out_sum -= stackIn->r;
				g_out_sum -= stackIn->g;
				b_out_sum -= stackIn->b;
				a_out_sum -= stackIn->a;
				
				p = x + radius + 1;
				p = ( yw + ( p < widthMinus1 ? p : widthMinus1 ) ) << 2;
				
				r_in_sum += pixels[p];
				g_in_sum += pixels[p + 1];
				b_in_sum += pixels[p + 2];
				a_in_sum += pixels[p + 3];
				
				r_sum += r_in_sum;
				g_sum += g_in_sum;
				b_sum += b_in_sum;
				a_sum += a_in_sum;
				
				stackIn->r = pixels[p];
				stackIn->g = pixels[p + 1];
				stackIn->b = pixels[p + 2];
				stackIn->a = pixels[p + 3];
				stackIn = stackIn->next;
				
				r_out_sum += ( pr = stackOut->r );
				g_out_sum += ( pg = stackOut->g );
				b_out_sum += ( pb = stackOut->b );
				a_out_sum += ( pa = stackOut->a );
				
				r_in_sum -= pr;
				g_in_sum -= pg;
				b_in_sum -= pb;
				a_in_sum -= pa;
				
				stackOut = stackOut->next;

				yi += 4;
			}
			yw += width;
		}

		for (x = 0; x < width; x++)
		{
			g_in_sum = b_in_sum = a_in_sum = r_in_sum = g_sum = b_sum = a_sum = r_sum = 0;
			
			yi = x << 2;
			
			pr = pixels[yi];
			pg = pixels[yi + 1];
			pb = pixels[yi + 2];
			pa = pixels[yi + 3];

			r_out_sum = radiusPlus1 * pr;
			g_out_sum = radiusPlus1 * pg;
			b_out_sum = radiusPlus1 * pb;
			a_out_sum = radiusPlus1 * pa;
			
			r_sum += sumFactor * pr;
			g_sum += sumFactor * pg;
			b_sum += sumFactor * pb;
			a_sum += sumFactor * pa;
			
			stack = stackStart;
			
			for (i = 0; i < radiusPlus1; i++)
			{
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack->a = pa;
				stack = stack->next;
			}
			
			yp = width;
			
			for (i = 1; i <= radius; i++)
			{
				yi = ( yp + x ) << 2;
				
				rbs = radiusPlus1 - i;
				
				pr = pixels[yi];
				pg = pixels[yi + 1];
				pb = pixels[yi + 2];
				pa = pixels[yi + 3];

				r_sum += pr * rbs;
				g_sum += pg * rbs;
				b_sum += pb * rbs;
				a_sum += pa * rbs;
				
				r_in_sum += pr;
				g_in_sum += pg;
				b_in_sum += pb;
				a_in_sum += pa;
				
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack->a = pa;
				stack = stack->next;
			
				if (i < heightMinus1)
					yp += width;
			}
			
			yi = x;
			stackIn = stackStart;
			stackOut = stackEnd;
			for (y = 0; y < height; y++)
			{
				p = yi << 2;
				
				pixels[p]   = (r_sum * mul_sum) >> shg_sum;
				pixels[p + 1] = (g_sum * mul_sum) >> shg_sum;
				pixels[p + 2] = (b_sum * mul_sum) >> shg_sum;
				pixels[p + 3] = (a_sum * mul_sum) >> shg_sum;
				
				r_sum -= r_out_sum;
				g_sum -= g_out_sum;
				b_sum -= b_out_sum;
				a_sum -= a_out_sum;
				
				r_out_sum -= stackIn->r;
				g_out_sum -= stackIn->g;
				b_out_sum -= stackIn->b;
				a_out_sum -= stackIn->a;
				
				p = y + radiusPlus1;
				p = ( x + (( p < heightMinus1 ? p : heightMinus1 ) * width )) << 2;
				
				r_sum += ( r_in_sum += pixels[p]);
				g_sum += ( g_in_sum += pixels[p+1]);
				b_sum += ( b_in_sum += pixels[p+2]);
				a_sum += ( a_in_sum += pixels[p+3]);
				
				stackIn->r = pixels[p];
				stackIn->g = pixels[p+1];
				stackIn->b = pixels[p+2];
				stackIn->a = pixels[p+3];
				stackIn = stackIn->next;
				
				r_out_sum += ( pr = stackOut->r );
				g_out_sum += ( pg = stackOut->g );
				b_out_sum += ( pb = stackOut->b );
				a_out_sum += ( pa = stackOut->a );
				
				r_in_sum -= pr;
				g_in_sum -= pg;
				b_in_sum -= pb;
				a_in_sum -= pa;
				
				stackOut = stackOut->next;
				
				yi += width;
			}
		}
	}
	else // 24 bits =================================
	{
		for (y = 0; y < height; y++)
		{
			r_in_sum = g_in_sum = b_in_sum = r_sum = g_sum = b_sum = 0;
			
			pr = pixels[yi];
			pg = pixels[yi + 1];
			pb = pixels[yi + 2];
			
			r_out_sum = radiusPlus1 * pr;
			g_out_sum = radiusPlus1 * pg;
			b_out_sum = radiusPlus1 * pb;
			
			r_sum += sumFactor * pr;
			g_sum += sumFactor * pg;
			b_sum += sumFactor * pb;
			
			stack = stackStart;
			
			for (i = 0; i < radiusPlus1; i++)
			{
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack = stack->next;
			}
			
			for (i = 1; i < radiusPlus1; i++)
			{
				p = yi + (( widthMinus1 < i ? widthMinus1 : i ) * 3 );
				rbs = radiusPlus1 - i;
				
				pr = pixels[p];
				pg = pixels[p + 1];
				pb = pixels[p + 2];
				
				r_sum += pr * rbs;
				g_sum += pg * rbs;
				b_sum += pb * rbs;
				
				r_in_sum += pr;
				g_in_sum += pg;
				b_in_sum += pb;
				
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				
				stack = stack->next;
			}
			
			
			stackIn = stackStart;
			stackOut = stackEnd;
			for (x = 0; x < width; x++)
			{
				pixels[yi] = (r_sum * mul_sum) >> shg_sum; // * 255 / pa;
				pixels[yi + 1] = (g_sum * mul_sum) >> shg_sum; // * 255 / pa;
				pixels[yi + 2] = (b_sum * mul_sum) >> shg_sum; // * 255 / pa;
				
				r_sum -= r_out_sum;
				g_sum -= g_out_sum;
				b_sum -= b_out_sum;
				
				r_out_sum -= stackIn->r;
				g_out_sum -= stackIn->g;
				b_out_sum -= stackIn->b;
				
				p = x + radius + 1;
				p = ( yw + ( p < widthMinus1 ? p : widthMinus1 ) ) * 3;
				
				r_in_sum += pixels[p];
				g_in_sum += pixels[p + 1];
				b_in_sum += pixels[p + 2];
				
				r_sum += r_in_sum;
				g_sum += g_in_sum;
				b_sum += b_in_sum;
				
				stackIn->r = pixels[p];
				stackIn->g = pixels[p + 1];
				stackIn->b = pixels[p + 2];
				stackIn = stackIn->next;
				
				r_out_sum += ( pr = stackOut->r );
				g_out_sum += ( pg = stackOut->g );
				b_out_sum += ( pb = stackOut->b );
				
				r_in_sum -= pr;
				g_in_sum -= pg;
				b_in_sum -= pb;
				
				stackOut = stackOut->next;

				yi += 3;
			}
			yw += width;
		}

		for (x = 0; x < width; x++)
		{
			g_in_sum = b_in_sum = r_in_sum = g_sum = b_sum = r_sum = 0;
			
			yi = x << 2;
			
			pr = pixels[yi];
			pg = pixels[yi + 1];
			pb = pixels[yi + 2];

			r_out_sum = radiusPlus1 * pr;
			g_out_sum = radiusPlus1 * pg;
			b_out_sum = radiusPlus1 * pb;
			
			r_sum += sumFactor * pr;
			g_sum += sumFactor * pg;
			b_sum += sumFactor * pb;
			
			stack = stackStart;
			
			for (i = 0; i < radiusPlus1; i++)
			{
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack = stack->next;
			}
			
			yp = width;
			
			for (i = 1; i <= radius; i++)
			{
				yi = ( yp + x ) << 2;
				
				rbs = radiusPlus1 - i;
				
				pr = pixels[yi];
				pg = pixels[yi + 1];
				pb = pixels[yi + 2];

				r_sum += pr * rbs;
				g_sum += pg * rbs;
				b_sum += pb * rbs;
				
				r_in_sum += pr;
				g_in_sum += pg;
				b_in_sum += pb;
				
				stack->r = pr;
				stack->g = pg;
				stack->b = pb;
				stack = stack->next;
			
				if (i < heightMinus1)
					yp += width;
			}
			
			yi = x;
			stackIn = stackStart;
			stackOut = stackEnd;
			for (y = 0; y < height; y++)
			{
				p = yi << 2;
				
				pixels[p]   = (r_sum * mul_sum) >> shg_sum;
				pixels[p + 1] = (g_sum * mul_sum) >> shg_sum;
				pixels[p + 2] = (b_sum * mul_sum) >> shg_sum;
				
				r_sum -= r_out_sum;
				g_sum -= g_out_sum;
				b_sum -= b_out_sum;
				
				r_out_sum -= stackIn->r;
				g_out_sum -= stackIn->g;
				b_out_sum -= stackIn->b;
				
				p = y + radiusPlus1;
				p = ( x + (( p < heightMinus1 ? p : heightMinus1 ) * width )) * 3;
				
				r_sum += ( r_in_sum += pixels[p]);
				g_sum += ( g_in_sum += pixels[p+1]);
				b_sum += ( b_in_sum += pixels[p+2]);
				
				stackIn->r = pixels[p];
				stackIn->g = pixels[p + 1];
				stackIn->b = pixels[p + 2];
				stackIn = stackIn->next;
				
				r_out_sum += ( pr = stackOut->r );
				g_out_sum += ( pg = stackOut->g );
				b_out_sum += ( pb = stackOut->b );
				
				r_in_sum -= pr;
				g_in_sum -= pg;
				b_in_sum -= pb;
				
				stackOut = stackOut->next;
				
				yi += width;
			}
		}
	}
	
	MODIFY(img);
}


//---------------------------------------------------------------------------

static inline int between0And255 (int val)
{
	if (val < 0)
		return 0;
	else if (val > 255)
		return 255;
	else
		return val;
}

static inline int get_brightness(int base, int strength)
{
	if (strength == 0) return base;
	return between0And255(base + strength);
}

static inline int get_contrast(int base, int strength)
{
	if (strength == 0) return base;
	return between0And255 ((base - 127) * (strength + 255) / 255 + 127);
}

static inline int myRound(double d)
{
	return d >= 0.0 ? (int)(d + 0.5) : (int)( d - ((int)d-1) + 0.5 ) + ((int)d-1);
}

static inline int get_gamma(int base, int strength)
{
	if (strength == 0) return base;
	return between0And255(myRound(255.0 * pow(base / 255.0, 1.0 / pow(10, strength / 255.0))));
}


void IMAGE_balance(GB_IMG *img, int brightness, int contrast, int gamma, int hue, int saturation, int lightness)
{
	GET_POINTER(img, p, pm);
	uint col;
	//int h, s, v;
	//int r, g, b;
	//int hcol, scol, vcol;
	int format = img->format;
	int i;
	uchar *pp;

	SYNCHRONIZE(img);

	if (brightness || contrast || gamma)
	{
		uchar trgb[256];

		for (i = 0; i < 256; i++)
			trgb[i] = get_gamma(get_contrast(get_brightness(i, brightness), contrast), gamma);

		if (img->format == GB_IMAGE_BGRA || img->format == GB_IMAGE_RGBA)
		{
			while (p != pm)
			{
				pp = (uchar *)p;
				pp[0] = trgb[pp[0]];
				pp[1] = trgb[pp[1]];
				pp[2] = trgb[pp[2]];
				p++;
			}
		}
		else
		{
			while (p != pm)
			{
				col = BGRA_from_format(*p, format);
				*p++ = BGRA_to_format(RGBA(trgb[RED(col)], trgb[GREEN(col)], trgb[BLUE(col)], ALPHA(col)), format);
			}
		}
	}

	if (hue || saturation)
	{
		double sm;
		uchar r, g, b;

		if (saturation < 0)
			sm = 1 + saturation / 255.0;
		else
			sm = 1 + saturation / 255.0 * 2;

		double hue6 = (double)hue / 360 * 6;

		double l, h, s, v, m, f;

		p = (uint *)img->data;
		while (p != pm)
		{
			col = BGRA_from_format(*p, format);
			r = RED(col); g = GREEN(col); b = BLUE(col);

			uchar vs = r;
			if (g > vs) vs = g;
			if (b > vs) vs = b;

			uchar ms = r;
			if (g < ms) ms = g;
			if (b < ms) ms = b;

			uchar vm = (vs - ms);

			l = (double)(ms + vs) / 510;

			if (vs && vm)
			{
				if ((ms + vs) <= 255)
				{
					s = (double)vm / (vs + ms) * sm;
					if (s > 1) s = 1;
					v = l * (1 + s);
				}
				else
				{
					s = (double)vm / (510 - (vs + ms)) * sm;
					if (s > 1) s = 1;
					v = (l + s - l * s);
				}

				if (r == vs)
				{
					if (g == ms)
						h = 5 + (((double)vs-b)/vm) + hue6;
					else
						h = 1 - ((double)(vs-g)/vm) + hue6;
				}
				else if (g == vs)
				{
					if (b == ms)
						h = 1 + ((double)(vs-r)/vm) + hue6;
					else
						h = 3 - ((double)(vs-b)/vm) + hue6;
				}
				else
				{
					if (r == ms)
						h = 3 + ((double)(vs-g)/vm) + hue6;
					else
						h = 5 - ((double)(vs-r)/vm) + hue6;
				}

				if (h < 0) h += 6;
				if (h >= 6) h -= 6;

				m = l + l - v;
				f = h - (int)h;

				switch((int)h)
				{
					case 0:
						r = v*255; g = (m+((v-m)*f))*255; b = m*255; break;
					case 1:
						r = (v-((v-m)*f))*255; g = v*255; b = m*255; break;
					case 2:
						r = m*255; g = v*255; b = (m+((v-m)*f))*255; break;
					case 3:
						r = m*255; g = (v-((v-m)*f))*255; b = v*255; break;
					case 4:
						r = (m+((v-m)*f))*255; g = m*255; b = v*255; break;
					case 5:
						r = v*255; g = m*255; b = (v-((v-m)*f))*255; break;
				}
			}

			*p++ = BGRA_to_format(RGBA(between0And255(r), between0And255(g), between0And255(b), ALPHA(col)), format);
		}
	}

	if (lightness)
	{
		uchar trgb[256];
		double lp = 1 + lightness / 255.0;
		double lm = 1 - lightness / 255.0;

		for (i = 0; i < 256; i++)
		{
			if (lightness < 0)
				trgb[i] = between0And255(i * lp);
			else
				trgb[i] = between0And255(i * lm + lightness);
		}

		p = (uint *)img->data;

		if (img->format == GB_IMAGE_BGRA || img->format == GB_IMAGE_RGBA)
		{
			while (p != pm)
			{
				pp = (uchar *)p;
				pp[0] = trgb[pp[0]];
				pp[1] = trgb[pp[1]];
				pp[2] = trgb[pp[2]];
				p++;
			}
		}
		else
		{
			while (p != pm)
			{
				col = BGRA_from_format(*p, format);
				*p++ = BGRA_to_format(RGBA(trgb[RED(col)], trgb[GREEN(col)], trgb[BLUE(col)], ALPHA(col)), format);
			}
		}
	}

	MODIFY(img);
}


//---------------------------------------------------------------------------


