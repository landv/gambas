/***************************************************************************

  csvgimage.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CSVGIMAGE_CPP

#include "main.h"
#include "gambas.h"
#include "cpaint_impl.h"
#include "csvgimage.h"

#define MM_TO_PT(_mm) ((_mm) * 72 / 25.4)
#define PT_TO_MM(_pt) ((_pt) / 72 * 25.4)

static void release(CSVGIMAGE *_object)
{
	if (RENDERER)
	{
		delete RENDERER;
		RENDERER = NULL;
	}
	
	if (GENERATOR)
	{
		delete GENERATOR;
		THIS->generator = NULL;
		unlink(THIS->file);
		GB.FreeString(&THIS->file);
	}

	THIS->width = THIS->height = 0;
}

QSvgGenerator *SVGIMAGE_begin(CSVGIMAGE *_object, QPainter **painter)
{
	if (!GENERATOR)
	{
		if (THIS->width <= 0 || THIS->height <= 0)
		{
			GB.Error("SvgImage size is not defined");
			return NULL;
		}
		
		GB.NewString(&THIS->file, GB.TempFile(NULL), 0);
		THIS->generator = new QSvgGenerator();
		GENERATOR->setSize(QSize(THIS->width, THIS->height));
		//GENERATOR->setViewBox(QRectF(0, 0, THIS->width, THIS->height));
		GENERATOR->setFileName(THIS->file);
		
		if (RENDERER)
		{
			*painter = new QPainter(GENERATOR);
			RENDERER->render(*painter, QRectF(0, 0, THIS->width, THIS->height));
		}
		else
			*painter = NULL;
	}
	
	return GENERATOR;
}

BEGIN_METHOD(SvgImage_new, GB_FLOAT width; GB_FLOAT height)

	THIS->width = VARGOPT(width, 0);
	THIS->height = VARGOPT(height, 0);

END_METHOD

BEGIN_METHOD_VOID(SvgImage_free)

	release(THIS);

END_METHOD

BEGIN_PROPERTY(SvgImage_Width)

	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->width);
	else
		THIS->width = VPROP(GB_FLOAT);
	
END_PROPERTY

BEGIN_PROPERTY(SvgImage_Height)
	
	if (READ_PROPERTY)
		GB.ReturnFloat(THIS->height);
	else
		THIS->height = VPROP(GB_FLOAT);
	
END_PROPERTY

static bool load_file(CSVGIMAGE *_object, const char *path, int len_path)
{
	QSvgRenderer *renderer;
	QByteArray data;
	char *addr;
	int len;
	bool ret = true;

	if (GB.LoadFile(path, len_path, &addr, &len))
	{
		GB.Error("Unable to load SVG file");
		return true;
	}

	data = QByteArray::fromRawData(addr, len);

	renderer = new QSvgRenderer(data);
	if (!renderer->isValid())
	{
		GB.Error("Unable to load SVG file: unable to create renderer"); 
		goto __RETURN;
	}
	
	release(THIS);
	THIS->renderer = renderer;
	THIS->width = renderer->defaultSize().width();
	THIS->height = renderer->defaultSize().height();

	renderer = NULL;
	ret = false;

__RETURN:

	if (renderer)
		delete renderer;
	
	GB.ReleaseFile(addr, len);
	return ret;
}

BEGIN_METHOD(SvgImage_Load, GB_STRING path)

	CSVGIMAGE *svgimage;

	GB.New(POINTER(&svgimage), CLASS_SvgImage, NULL, NULL);

	if (load_file(svgimage, STRING(path), LENGTH(path)))
	{
		GB.Unref(POINTER(&svgimage));
		return;
	}
	
	GB.ReturnObject(svgimage);

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Paint)

	QPainter *painter = PAINT_get_current();
	float x, y;
	
	if (!painter)
		return;
	
	if (THIS->file)
		load_file(THIS, THIS->file, GB.StringLength(THIS->file));
	
	if (!RENDERER)
		return;

	PAINT_get_current_point(&x, &y);
	RENDERER->render(painter, QRectF(x, y, THIS->width, THIS->height));

END_METHOD

BEGIN_METHOD(SvgImage_Save, GB_STRING file)
	
	if (!THIS->file)
	{
		QPainter *painter;
		if (!SVGIMAGE_begin(THIS, &painter))
		{
			GB.Error("Void image");
			return;
		}
		if (painter)
			delete painter;
	}
	
	if (GB.CopyFile(THIS->file, GB.FileName(STRING(file), LENGTH(file))))
		return;
	
	load_file(THIS, THIS->file, GB.StringLength(THIS->file));

END_METHOD

BEGIN_METHOD_VOID(SvgImage_Clear)

	release(THIS);

END_METHOD

GB_DESC SvgImageDesc[] =
{
  GB_DECLARE("SvgImage", sizeof(CSVGIMAGE)),

  GB_METHOD("_new", NULL, SvgImage_new, "[(Width)f(Height)f]"),
  GB_METHOD("_free", NULL, SvgImage_free, NULL),

  GB_PROPERTY("Width", "f", SvgImage_Width),
  GB_PROPERTY("Height", "f", SvgImage_Height),

  GB_STATIC_METHOD("Load", "SvgImage", SvgImage_Load, "(Path)s"),
  GB_METHOD("Save", NULL, SvgImage_Save, "(Path)s"),
	GB_METHOD("Paint", NULL, SvgImage_Paint, NULL),

  GB_METHOD("Clear", NULL, SvgImage_Clear, NULL),

  GB_INTERFACE("Paint", &PAINT_Interface),

  GB_END_DECLARE
};

