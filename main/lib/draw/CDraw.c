/***************************************************************************

  CDraw.c

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

#define __CDRAW_C

#include "matrix.h"
#include "main.h"
#include "CDraw.h"

static GB_DRAW *_current = NULL;
#define THIS _current
#define DRAW _current->desc
#define THIS_MATRIX ((MATRIX *)&(THIS->matrix))

static bool check_device()
{
	if (!_current)
	{
		GB.Error("No current device");
		return TRUE;
	}
	else
		return FALSE;
}

PUBLIC GB_DRAW *DRAW_get_current()
{
	check_device();
	return _current;
}

#define CHECK_DEVICE() if (check_device()) return

bool DRAW_begin(void *device)
{
	GB_DRAW_DESC *desc;
	GB_DRAW *draw;
	GB_CLASS klass;
	
	klass = GB.GetClass(device);
	if (klass == GB.FindClass("Class"))
		klass = device;
 
	desc = (GB_DRAW_DESC *)GB.GetClassInterface(klass, "Draw");
	if (!desc)
	{
		GB.Error("Not a drawable object");
		return TRUE;
	}

	GB.Alloc(POINTER(&draw), sizeof(GB_DRAW) + desc->size);
	draw->desc = desc;
	draw->previous = _current;
	GB.Ref(device);
	draw->device = device;
	_current = draw;
	
	MATRIX_init(THIS_MATRIX);
	draw->save = NULL;
	draw->xform = FALSE;
	
	return draw->desc->Begin(draw);
}

BEGIN_METHOD(CDRAW_begin, GB_OBJECT device)

	void *device = VARG(device);

  if (GB.CheckObject(device))
    return;

	DRAW_begin(device);

END_METHOD


void DRAW_end()
{
	GB_DRAW *draw;

	if (!_current)
		return;
		
	draw = _current;
	_current = _current->previous;
	
	draw->desc->End(draw);
	
	GB.Unref(POINTER(&draw->device));
	GB.Free(POINTER(&draw));
}


BEGIN_METHOD_VOID(CDRAW_end)

	DRAW_end();
	
END_METHOD


BEGIN_METHOD_VOID(CDRAW_exit)

	while (_current)
		DRAW_end();

END_METHOD


BEGIN_PROPERTY(CDRAW_device)
	
  CHECK_DEVICE();
	GB.ReturnObject(THIS->device);

END_PROPERTY

BEGIN_PROPERTY(CDRAW_width)

  CHECK_DEVICE();
	GB.ReturnInteger(THIS->width);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_height)

  CHECK_DEVICE();
	GB.ReturnInteger(THIS->height);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_resolution)

  CHECK_DEVICE();
	GB.ReturnInteger(THIS->resolution);

END_PROPERTY

static void handle_int_property(void *_param, int (*get)(GB_DRAW *), void (*set)(GB_DRAW *, int))
{
  CHECK_DEVICE();

  if (READ_PROPERTY)
    GB.ReturnInteger((*get)(THIS));
  else
  	(*set)(THIS, VPROP(GB_INTEGER));
}

BEGIN_PROPERTY(CDRAW_background)

	handle_int_property(_param, DRAW->GetBackground, DRAW->SetBackground);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_foreground)

	handle_int_property(_param, DRAW->GetForeground, DRAW->SetForeground);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_x)

	int val;
  
  CHECK_DEVICE();
  DRAW->Clip.Get(THIS, &val, NULL, NULL, NULL);
  GB.ReturnInteger(val);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_y)

	int val;
  
  CHECK_DEVICE();
  DRAW->Clip.Get(THIS, NULL, &val, NULL, NULL);
  GB.ReturnInteger(val);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_w)

	int val;
  
  CHECK_DEVICE();
  DRAW->Clip.Get(THIS, NULL, NULL, &val, NULL);
  GB.ReturnInteger(val);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_h)

	int val;
  
  CHECK_DEVICE();
  DRAW->Clip.Get(THIS, NULL, NULL, NULL, &val);
  GB.ReturnInteger(val);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_clip_enabled)

  CHECK_DEVICE();

  if (READ_PROPERTY)
    GB.ReturnBoolean(DRAW->Clip.IsEnabled(THIS));
  else
  	DRAW->Clip.SetEnabled(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CDRAW_clip, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

  CHECK_DEVICE();
  DRAW->Clip.Set(THIS, VARG(x), VARG(y), VARG(w), VARG(h));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_invert)

  CHECK_DEVICE();

  if (READ_PROPERTY)
    GB.ReturnBoolean(DRAW->IsInverted(THIS));
  else
    DRAW->SetInverted(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_transparent)

  CHECK_DEVICE();

  if (READ_PROPERTY)
    GB.ReturnBoolean(DRAW->IsTransparent(THIS));
  else
    DRAW->SetTransparent(THIS, VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_font)

  CHECK_DEVICE();

  if (READ_PROPERTY)
    GB.ReturnObject(DRAW->GetFont(THIS));
  else
  	DRAW->SetFont(THIS, VPROP(GB_OBJECT));

END_PROPERTY


BEGIN_PROPERTY(CDRAW_line_width)

	handle_int_property(_param, DRAW->Line.GetWidth, DRAW->Line.SetWidth);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_line_style)

	handle_int_property(_param, DRAW->Line.GetStyle, DRAW->Line.SetStyle);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_color)

	handle_int_property(_param, DRAW->Fill.GetColor, DRAW->Fill.SetColor);
  
END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_style)

	handle_int_property(_param, DRAW->Fill.GetStyle, DRAW->Fill.SetStyle);
  
END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_x)

	int x, y;
	
  CHECK_DEVICE();

  DRAW->Fill.GetOrigin(THIS, &x, &y);
  
  if (READ_PROPERTY)
    GB.ReturnInteger(x);
  else
  	DRAW->Fill.SetOrigin(THIS, VPROP(GB_INTEGER), y);

END_PROPERTY


BEGIN_PROPERTY(CDRAW_fill_y)

	int x, y;
	
  CHECK_DEVICE();

  DRAW->Fill.GetOrigin(THIS, &x, &y);
  
  if (READ_PROPERTY)
    GB.ReturnInteger(y);
  else
  	DRAW->Fill.SetOrigin(THIS, x, VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_METHOD(CDRAW_point, GB_INTEGER x; GB_INTEGER y)

	int x, y;
	
  CHECK_DEVICE();
  
  x = VARG(x);
  y = VARG(y);
  
  if (THIS->xform)
  	MATRIX_map_point(THIS_MATRIX, &x, &y);
	
	DRAW->Draw.Point(THIS, x, y);

END_METHOD


BEGIN_METHOD(CDRAW_line, GB_INTEGER x1; GB_INTEGER y1; GB_INTEGER x2; GB_INTEGER y2)

	int x1, y1, x2, y2;
	
  CHECK_DEVICE();
  
  x1 = VARG(x1);
  y1 = VARG(y1);
  x2 = VARG(x2);
  y2 = VARG(y2);
  
  if (THIS->xform)
  {
  	MATRIX_map_point(THIS_MATRIX, &x1, &y1);
  	MATRIX_map_point(THIS_MATRIX, &x2, &y2);
  }
  
  DRAW->Draw.Line(THIS, x1, y1, x2, y2);

END_METHOD


BEGIN_METHOD(CDRAW_rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	int x, y, w, h;

  CHECK_DEVICE();
  
  x = VARG(x);
  y = VARG(y);
  w = VARG(w);
  h = VARG(h);
  
  if (w < 0)
  	x += w, w = (-w);
  if (h < 0)
  	y += h, h = (-h);
  
  if (THIS->xform)
  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
  
  if (w < 1 || h < 1)
  	return;
  
  DRAW->Draw.Rect(THIS, x, y, w, h);

END_METHOD


// BEGIN_METHOD(CDRAW_round_rect, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_FLOAT round)
// 
//   CHECK_DEVICE();
//   DRAW->Draw.RoundRect(THIS, VARG(x), VARG(y), VARG(w), VARG(h), VARGOPT(round, 0.25));
// 
// END_METHOD


BEGIN_METHOD(CDRAW_ellipse, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_FLOAT start; GB_FLOAT end)

	int x, y, w, h;

  CHECK_DEVICE();
  
  x = VARG(x);
  y = VARG(y);
  w = VARG(w);
  h = VARG(h);
  
  if (THIS->xform)
  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
  
  if (w < 1 || h < 1)
  	return;
  
  DRAW->Draw.Ellipse(THIS, x, y, w, h, VARGOPT(start, 0.0), VARGOPT(end, 0.0));

END_METHOD


BEGIN_METHOD(CDRAW_circle, GB_INTEGER x; GB_INTEGER y; GB_INTEGER radius; GB_FLOAT start; GB_FLOAT end)

	int x, y, w, h;
	int radius;
	
  CHECK_DEVICE();

	radius = VARG(radius);
	if (radius <= 0)
		return;
		
	x = VARG(x) - radius;
	y = VARG(y) - radius;
	w = radius * 2 - 1;
	h = radius * 2 - 1;
		
  if (THIS->xform)
  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
  
  DRAW->Draw.Ellipse(THIS, x, y, w, h, VARGOPT(start, 0.0), VARGOPT(end, 0.0));

END_METHOD


BEGIN_METHOD(CDRAW_text, GB_STRING text; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER align)

	int x, y, w, h;

  CHECK_DEVICE();
  
  x = VARG(x);
  y = VARG(y);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);
  
  if (THIS->xform)
  {
  	if (w < 0 || h < 0)
  		MATRIX_map_point(THIS_MATRIX, &x, &y);
  	else
	  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
  }
  
	DRAW->Draw.Text(THIS, STRING(text), LENGTH(text), x, y, w, h, VARGOPT(align, GB_DRAW_ALIGN_DEFAULT));

END_METHOD


BEGIN_METHOD(CDRAW_text_width, GB_STRING text)

	int w;
  CHECK_DEVICE();
  DRAW->Draw.TextSize(THIS, STRING(text), LENGTH(text), &w, NULL);
  GB.ReturnInteger(w);

END_METHOD


BEGIN_METHOD(CDRAW_text_height, GB_STRING text)

	int h;
  CHECK_DEVICE();
  DRAW->Draw.TextSize(THIS, STRING(text), LENGTH(text), NULL, &h);
  GB.ReturnInteger(h);

END_METHOD


BEGIN_METHOD(CDRAW_rich_text, GB_STRING text; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER align)

	int x, y, w, h;

  CHECK_DEVICE();
  
  x = VARG(x);
  y = VARG(y);
  w = VARGOPT(w, -1);
  h = VARGOPT(h, -1);
  
  if (THIS->xform)
  {
  	if (w < 0 || h < 0)
  		MATRIX_map_point(THIS_MATRIX, &x, &y);
  	else
	  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
  }
  
	DRAW->Draw.RichText(THIS, STRING(text), LENGTH(text), x, y, w, h, VARGOPT(align, GB_DRAW_ALIGN_DEFAULT));

END_METHOD


BEGIN_METHOD(CDRAW_rich_text_width, GB_STRING text)

	int w;
  CHECK_DEVICE();
  DRAW->Draw.RichTextSize(THIS, STRING(text), LENGTH(text), -1, &w, NULL);
  GB.ReturnInteger(w);

END_METHOD


BEGIN_METHOD(CDRAW_rich_text_height, GB_STRING text; GB_INTEGER width)

	int w = VARGOPT(width, -1);
	int h;
  CHECK_DEVICE();
  DRAW->Draw.RichTextSize(THIS, STRING(text), LENGTH(text), w, NULL, &h);
  GB.ReturnInteger(h);

END_METHOD


BEGIN_METHOD(CDRAW_polyline, GB_OBJECT points)

	uint n;
	GB_ARRAY points = VARG(points);
	int *coord;
	
  CHECK_DEVICE();

  n = GB.Array.Count(points) / 2;
  if (n == 0)
    return;
  
  coord = (int *)GB.Array.Get(points, 0);
  
  if (THIS->xform)
  	coord = MATRIX_map_array(THIS_MATRIX, coord, n);	
	
	DRAW->Draw.Polyline(THIS, n, coord);
	
	if (THIS->xform)
		MATRIX_free_array(&coord);

END_METHOD


BEGIN_METHOD(CDRAW_polygon, GB_OBJECT points)

	uint n;
	GB_ARRAY points = VARG(points);
	int *coord;
	
  CHECK_DEVICE();

  n = GB.Array.Count(points) / 2;
  if (n == 0)
    return;
  
  coord = (int *)GB.Array.Get(points, 0);
  
  if (THIS->xform)
  	coord = MATRIX_map_array(THIS_MATRIX, coord, n);	
	
	DRAW->Draw.Polygon(THIS, n, coord);
	
	if (THIS->xform)
		MATRIX_free_array(&coord);

END_METHOD


BEGIN_METHOD(CDRAW_picture, GB_OBJECT picture; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	GB_PICTURE picture = VARG(picture);
	GB_PICTURE_INFO info;
	int x, y, w, h;

  CHECK_DEVICE();

  if (GB.CheckObject(picture))
    return;

	GB.Picture.Info(picture, &info);

	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);
	w = VARGOPT(w, info.width);
	h = VARGOPT(h, info.height);

  if (THIS->xform)
  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
	
	DRAW->Draw.Picture(THIS, picture, 
		x, y, w, h,
		VARGOPT(sx, 0), VARGOPT(sy, 0), VARGOPT(sw, -1), VARGOPT(sh, -1));

END_METHOD


BEGIN_METHOD(CDRAW_image, GB_OBJECT image; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	GB_IMAGE image = VARG(image);
	GB_PICTURE_INFO info;
	int x, y, w, h;

  CHECK_DEVICE();

  if (GB.CheckObject(image))
    return;

	GB.Image.Info(image, &info);

	x = VARGOPT(x, 0);
	y = VARGOPT(y, 0);
	w = VARGOPT(w, info.width);
	h = VARGOPT(h, info.height);

  if (THIS->xform)
  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
	
	DRAW->Draw.Image(THIS, image, 
		x, y, w, h,
		VARGOPT(sx, 0), VARGOPT(sy, 0), VARGOPT(sw, -1), VARGOPT(sh, -1));

END_METHOD

static uint blend_color(uint col, uint gray, uint alpha)
{
	gray *= (0xFF - alpha);
	
	return
		((col & 0xFF) * alpha + gray) >> 8
		| ((((col >> 8) & 0xFF) * alpha + gray) >> 8) << 8
		| ((((col >> 16) & 0xFF) * alpha + gray) >> 8) << 16;
}

BEGIN_METHOD(CDRAW_zoom, GB_OBJECT image; GB_INTEGER zoom; GB_INTEGER x; GB_INTEGER y; GB_INTEGER sx; GB_INTEGER sy; GB_INTEGER sw; GB_INTEGER sh)

	GB_IMAGE image = VARG(image);
	GB_IMAGE_INFO info;
	int zoom, size, size2;
	int x, y, sx, sy, sw, sh;
	int i, j, xr, yr;
	uint *data, *conv = NULL;
	uint col, col1, col2, last_col;
	bool border;
	int fill_style, fill_color;
	uint a;
	bool opaque;

  CHECK_DEVICE();

  if (GB.CheckObject(image))
    return;

  zoom = VARG(zoom);
  if (zoom < 1)
  {
    GB.Error("Bad zoom factor");
    return;
  }
  
  GB.Image.Info(image, &info);
  
  x = VARGOPT(x, 0);
  y = VARGOPT(y, 0);

  sx = VARGOPT(sx, 0);
  sy = VARGOPT(sy, 0);
  sw = VARGOPT(sw, info.width);
  sh = VARGOPT(sh, info.height);

  DRAW_NORMALIZE(x, y, sw, sh, sx, sy, sw, sh, info.width, info.height);

	//DRAW->Fill.GetOrigin(THIS, &ox, &oy);
	
  border = DRAW->Line.GetStyle(THIS);

  if (zoom == 1 && !border)
  {
  	DRAW->Fill.SetStyle(THIS, 1);
  	DRAW->Fill.SetColor(THIS, 0x979797);
  	DRAW->Draw.Rect(THIS, x, y, sw, sh);
  	DRAW->Draw.Image(THIS, image, x, y, -1, -1, sx, sy, sw, sh);
  }
  else
  {
  	// May have to convert the image data
  	if (info.format != GB_IMAGE_BGRA && info.format != GB_IMAGE_BGRX)
			GB.Alloc(POINTER(&conv), sw * sizeof(int));
  	
    size = zoom;
    size2 = size / 2;
    //if (border) size++;

		fill_style = DRAW->Fill.GetStyle(THIS);
		fill_color = DRAW->Fill.GetColor(THIS);

		//last_style = -1;
		//style = border;
		last_col = 0xFF000000;
		col1 = 0;
		col2 = 0;
		a = 0xFF;
		opaque = TRUE;
		
		DRAW->Fill.SetStyle(THIS, 1); // FILL_SOLID
		DRAW->Fill.SetColor(THIS, 0);
		
    for (j = sy, yr = y; j < (sy + sh); j++, yr += zoom)
    {
			data = (uint *)info.data + j * info.width + sx;
			
			if (conv)
			{
				GB.Image.Convert(conv, GB_IMAGE_BGRA, data, info.format, sw, 1);
				data = conv;
			}
			
      for (i = sx, xr = x; i < (sx + sw); i++, xr += zoom)
      {
      	col = *data++;
      	
        if (col != last_col)
        {
        	last_col = col;
	      	
	      	a = col >> 24;
	      	col &= 0xFFFFFF;
      		
      		if (a < 0xFF)
      		{
      			if (a == 0)
      			{
      				col1 = 0x808080;
      				col2 = 0xC0C0C0;
      			}
      			else
      			{
      				col1 = blend_color(col, 0x80, a);
      				col2 = blend_color(col, 0xC0, a);
      			}
      			opaque = FALSE;
      		}
      		else
      		{
      			opaque = TRUE;
	        	DRAW->Fill.SetColor(THIS, col);
					}					
        }
        
       	if (opaque)
       		DRAW->Draw.Rect(THIS, xr, yr, size + 1, size + 1);
       	else
       	{
        	DRAW->Fill.SetColor(THIS, col1);
       		DRAW->Line.SetStyle(THIS, 0);
       		DRAW->Draw.Rect(THIS, xr, yr, size, size);
        	DRAW->Fill.SetColor(THIS, col2);
       		DRAW->Draw.Rect(THIS, xr + size2, yr, size - size2, size2);
       		DRAW->Draw.Rect(THIS, xr, yr + size2, size2, size - size2);
	       	DRAW->Line.SetStyle(THIS, border);
       		if (border && a >= 16)
       		{
       			DRAW->Fill.SetStyle(THIS, 0);
       			DRAW->Draw.Rect(THIS, xr, yr, size + 1, size + 1);
       			DRAW->Fill.SetStyle(THIS, 1);
       		}
       	}
       	
      }
		}
		
		if (conv)
			GB.Free(POINTER(&conv));
		
		DRAW->Fill.SetStyle(THIS, fill_style);
		DRAW->Fill.SetColor(THIS, fill_color);
	}

END_METHOD


BEGIN_METHOD(CDRAW_tile, GB_OBJECT picture; GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	int x, y, w, h;

	GB_PICTURE picture = VARG(picture);

  CHECK_DEVICE();

  if (GB.CheckObject(picture))
    return;

  x = VARG(x);
  y = VARG(y);
  w = VARG(w);
  h = VARG(h);
  
  if (THIS->xform)
  	MATRIX_map_rect(THIS_MATRIX, &x, &y, &w, &h);
  
	DRAW->Draw.TiledPicture(THIS, picture, x, y, w, h);
  
END_METHOD


BEGIN_METHOD_VOID(CDRAW_reset)

  CHECK_DEVICE();
	MATRIX_reset(THIS_MATRIX);
	THIS->xform = FALSE;
	
END_METHOD


BEGIN_METHOD_VOID(CDRAW_push)

	MATRIX *save;
	
	CHECK_DEVICE();
	
	GB.Alloc(POINTER(&save), sizeof(MATRIX));
	*save = *THIS_MATRIX;
	save->next = THIS->save;
	THIS->save = save;
	
END_METHOD


BEGIN_METHOD_VOID(CDRAW_pop)

	MATRIX *save;
	
	CHECK_DEVICE();
	
	if (!THIS->save)
	{
		MATRIX_reset(THIS_MATRIX);
		return;
	}
	
	save = THIS->save;
	THIS->save = save->next;
	*THIS_MATRIX = *save;
	
	GB.Free(POINTER(&save));

END_METHOD


BEGIN_METHOD(CDRAW_translate, GB_FLOAT dx; GB_FLOAT dy)

	double dx = VARG(dx);
	double dy = VARG(dy);
	
	CHECK_DEVICE();
	MATRIX_translate(THIS_MATRIX, dx, dy);
	THIS->xform = !THIS->matrix.identity;

END_METHOD


BEGIN_METHOD(CDRAW_scale, GB_FLOAT dx; GB_FLOAT dy)

	double dx = VARG(dx);
	double dy = VARG(dy);

	CHECK_DEVICE();
	MATRIX_scale(THIS_MATRIX, dx, dy);
	THIS->xform = !THIS->matrix.identity;

END_METHOD


// BEGIN_METHOD(CDRAW_rotate, GB_FLOAT a)
// 
// 	double a = VARG(a);
// 
// 	DP->rotate(a);
// 	if (DPM)
// 		DPM->rotate(a);
// 
// END_METHOD



GB_DESC CDrawClipDesc[] =
{
  GB_DECLARE(".DrawClip", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("X", "i", CDRAW_clip_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAW_clip_y),
  GB_STATIC_PROPERTY_READ("W", "i", CDRAW_clip_w),
  GB_STATIC_PROPERTY_READ("H", "i", CDRAW_clip_h),
  GB_STATIC_PROPERTY_READ("Width", "i", CDRAW_clip_w),
  GB_STATIC_PROPERTY_READ("Height", "i", CDRAW_clip_h),

  GB_STATIC_PROPERTY("Enabled", "b", CDRAW_clip_enabled),
  GB_STATIC_METHOD("_call", NULL, CDRAW_clip, "(X)i(Y)i(Width)i(Height)i"),
	
  GB_END_DECLARE
};

GB_DESC CDrawDesc[] =
{
  GB_DECLARE("Draw", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_exit", NULL, CDRAW_exit, NULL),

  GB_STATIC_METHOD("Begin", NULL, CDRAW_begin, "(Device)o"),
  GB_STATIC_METHOD("End", NULL, CDRAW_end, NULL),
  
  GB_STATIC_PROPERTY_READ("Device", "o", CDRAW_device),

  GB_STATIC_PROPERTY_READ("W", "i", CDRAW_width),
  GB_STATIC_PROPERTY_READ("H", "i", CDRAW_height),
  GB_STATIC_PROPERTY_READ("Width", "i", CDRAW_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CDRAW_height),
  GB_STATIC_PROPERTY_READ("Resolution", "i", CDRAW_resolution),
  
  GB_STATIC_PROPERTY_SELF("Clip", ".DrawClip"),
  
  GB_STATIC_PROPERTY("Background", "i", CDRAW_background),
  GB_STATIC_PROPERTY("Foreground", "i", CDRAW_foreground),
  GB_STATIC_PROPERTY("BackColor", "i", CDRAW_background),
  GB_STATIC_PROPERTY("ForeColor", "i", CDRAW_foreground),
  
  GB_STATIC_PROPERTY("Invert", "b", CDRAW_invert),
  GB_STATIC_PROPERTY("Transparent", "b", CDRAW_transparent),
  
  GB_STATIC_PROPERTY("Font", "Font", CDRAW_font),

  GB_STATIC_PROPERTY("LineWidth", "i", CDRAW_line_width),
  GB_STATIC_PROPERTY("LineStyle", "i", CDRAW_line_style),

  GB_STATIC_PROPERTY("FillColor", "i", CDRAW_fill_color),
  GB_STATIC_PROPERTY("FillStyle", "i", CDRAW_fill_style),
  GB_STATIC_PROPERTY("FillX", "i", CDRAW_fill_x),
  GB_STATIC_PROPERTY("FillY", "i", CDRAW_fill_y),
  
  GB_STATIC_METHOD("Point", NULL, CDRAW_point, "(X)i(Y)i"),
  GB_STATIC_METHOD("Line", NULL, CDRAW_line, "(X1)i(Y1)i(X2)i(Y2)i"),
  GB_STATIC_METHOD("Rect", NULL, CDRAW_rect, "(X)i(Y)i(Width)i(Height)i"),
  //GB_STATIC_METHOD("RoundRect", NULL, CDRAW_round_rect, "(X)i(Y)i(Width)i(Height)i[(Round)f]"),
  GB_STATIC_METHOD("Circle", NULL, CDRAW_circle, "(X)i(Y)i(Radius)i[(Start)f(End)f]"),
  GB_STATIC_METHOD("Ellipse", NULL, CDRAW_ellipse, "(X)i(Y)i(Width)i(Height)i[(Start)f(End)f]"),
  GB_STATIC_METHOD("Text", NULL, CDRAW_text, "(Text)s(X)i(Y)i[(Width)i(Height)i(Alignment)i)]"),
  GB_STATIC_METHOD("TextWidth", "i", CDRAW_text_width, "(Text)s"),
  GB_STATIC_METHOD("TextHeight", "i", CDRAW_text_height, "(Text)s"),
  GB_STATIC_METHOD("RichText", NULL, CDRAW_rich_text, "(Text)s(X)i(Y)i[(Width)i(Height)i(Alignment)i)]"),
  GB_STATIC_METHOD("RichTextWidth", "i", CDRAW_rich_text_width, "(Text)s"),
  GB_STATIC_METHOD("RichTextHeight", "i", CDRAW_rich_text_height, "(Text)s[(Width)i]"),
  GB_STATIC_METHOD("Polyline", NULL, CDRAW_polyline, "(Points)Integer[]"),
  GB_STATIC_METHOD("Polygon", NULL, CDRAW_polygon, "(Points)Integer[]"),
  
  GB_STATIC_METHOD("Picture", NULL, CDRAW_picture, "(Picture)Picture;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Tile", NULL, CDRAW_tile, "(Picture)Picture;(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("Image", NULL, CDRAW_image, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Zoom", NULL, CDRAW_zoom, "(Image)Image;(Zoom)i(X)i(Y)i[(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  
	GB_STATIC_METHOD("Reset", NULL, CDRAW_reset, NULL),
	GB_STATIC_METHOD("Push", NULL, CDRAW_push, NULL),
	GB_STATIC_METHOD("Pop", NULL, CDRAW_pop, NULL),
	GB_STATIC_METHOD("Translate", NULL, CDRAW_translate, "(DX)f(DY)f"),
	GB_STATIC_METHOD("Scale", NULL, CDRAW_scale, "(SX)f(SY)f"),
  
  #if 0
  GB_STATIC_METHOD("Drawing", NULL, CDRAW_drawing, "(Drawing)Drawing;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

	GB_STATIC_METHOD("Rotate", NULL, CDRAW_rotate, "(Angle)f"),
	#endif
  
  GB_END_DECLARE
};


