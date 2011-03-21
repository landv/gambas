/***************************************************************************

	cpaint_impl.cpp

	(c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CPAINT_IMPL_CPP

#ifdef OS_SOLARIS
/* Make math.h define M_PI and a few other things */
#define __EXTENSIONS__
/* Get definition for finite() */
#include <ieeefp.h>
#endif
#include <math.h>

#include <QPainter>
#include <QPen>
#include <QBrush>
#ifndef NO_X_WINDOW
#include <QX11Info>
#endif

#include "gambas.h"

#include "CConst.h"
#include "CFont.h"
#include "CWidget.h"
#include "CWindow.h"
#include "CPicture.h"
#include "CImage.h"
#include "CDrawingArea.h"
#include "CColor.h"
#include "CDraw.h"
#include "cprinter.h"
#include "csvgimage.h"
#include "cpaint_impl.h"

#define EXTRA(d) ((QT_PAINT_EXTRA *)d->extra)

#define COLOR_TO_INT(color) ((color).rgba() ^ 0xFF000000)
#define MASK_COLOR(col) ((col & 0xFF000000) ? Qt::color0 : Qt::color1)

#define PAINTER(d) EXTRA(d)->painter
#define PATH(d) EXTRA(d)->path
#define CLIP(d) EXTRA(d)->clip

static inline qreal to_deg(float angle)
{
	return (qreal)(angle * 180 / M_PI);
}

static bool init_painting(GB_PAINT *d, QPaintDevice *device)
{
	QPen pen;
	
	d->width = device->width();
	d->height = device->height();
	d->resolutionX = device->physicalDpiX();
	d->resolutionY = device->physicalDpiY();
	
	if (!PAINTER(d)) 
	{
		if (device->paintingActive())
		{
			GB.Error("Device already being painted");
			return TRUE;
		}
		
		EXTRA(d)->painter = new QPainter(device);
	}
	
	MyPaintEngine *engine = (MyPaintEngine *)device->paintEngine();
	engine->patchFeatures();
	
	EXTRA(d)->path = 0;
	EXTRA(d)->clip = 0;
	PAINTER(d)->setRenderHints(QPainter::Antialiasing, true);
	PAINTER(d)->setRenderHints(QPainter::TextAntialiasing, true);
	PAINTER(d)->setRenderHints(QPainter::SmoothPixmapTransform, true);
	
	pen = PAINTER(d)->pen();
	pen.setCapStyle(Qt::FlatCap);
	pen.setJoinStyle(Qt::MiterJoin);
	pen.setMiterLimit(10.0);
	pen.setWidthF(2.0);
	PAINTER(d)->setPen(pen);
	PAINTER(d)->setBrush(Qt::black);
	
	return FALSE;
}


static int Begin(GB_PAINT *d)
{
	void *device = d->device;
	QPaintDevice *target = NULL;
	
	if (GB.Is(device, CLASS_Picture))
	{
		QPixmap *pixmap = ((CPICTURE *)device)->pixmap;
		
		if (pixmap->isNull())
		{
			GB.Error("Bad picture");
			return TRUE;
		}
		
		target = pixmap;
	}
	else if (GB.Is(device, CLASS_Image))
	{
		QImage *image = CIMAGE_get((CIMAGE *)device);
		
		if (image->isNull())
		{
			GB.Error("Bad image");
			return TRUE;
		}
		
		target = image;
	}
	else if (GB.Is(device, CLASS_DrawingArea))
	{
		MyDrawingArea *wid;
		
		wid = (MyDrawingArea *)(((CWIDGET *)device)->widget);

		if (wid->isCached())
			target = wid->background();
		else if (wid->cache)
			target = wid->cache;
		else
		{
			if (!wid->inDrawEvent())
			{
				GB.Error("Cannot paint outside of Draw event handler");
				return TRUE;
			}
			
			target = wid;
		}
			
		wid->drawn++;
		
		if (init_painting(d, target))
			return TRUE;
		
		d->width = wid->width();
		d->height = wid->height();
		return FALSE;
	}
	else if (GB.Is(device, CLASS_Printer))
	{
		CPRINTER *printer = (CPRINTER *)device;
		
		if (!printer->printing)
		{
			GB.Error("Printer is not printing");
			return TRUE;
		}
		
		target = printer->printer;
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		CSVGIMAGE *svgimage = (CSVGIMAGE *)device;
		target = SVGIMAGE_begin(svgimage, &EXTRA(d)->painter);
		if (!target)
		{
			GB.Error("SvgImage size is not defined");
			return TRUE;
		}
	}
	
	return init_painting(d, target);
}

static void End(GB_PAINT *d)
{
	void *device = d->device;

	if (GB.Is(device, CLASS_DrawingArea))
	{
		MyDrawingArea *wid;
		
		wid = (MyDrawingArea *)(((CWIDGET *)device)->widget);

		if (wid)
		{
			if (wid->isCached())
				wid->refreshBackground();
	
			wid->drawn--;
		}
	}
	else if (GB.Is(device, CLASS_SvgImage))
	{
		PAINTER(d)->end(); // ??
	}

	delete EXTRA(d)->path;
	delete EXTRA(d)->clip;
	delete EXTRA(d)->painter;
}

static void Save(GB_PAINT *d)
{
	PAINTER(d)->save();
}

static void Restore(GB_PAINT *d)
{
	PAINTER(d)->restore();
}
		
static void apply_font(QFont &font, void *object = 0)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();

	PAINTER(d)->setFont(font);
}

static void Font(GB_PAINT *d, int set, GB_FONT *font)
{
	if (set)
		PAINTER(d)->setFont(*((CFONT *)(*font))->font);
	else
		*font = CFONT_create(PAINTER(d)->font(), apply_font);
}

static void init_path(GB_PAINT *d)
{
	switch (EXTRA(d)->fillRule)
	{
		case GB_PAINT_FILL_RULE_WINDING:
			PATH(d)->setFillRule(Qt::WindingFill); 
			break;
		case GB_PAINT_FILL_RULE_EVEN_ODD: 
		default:
			PATH(d)->setFillRule(Qt::OddEvenFill);
	}
}

#define CHECK_PATH(_d) \
	if (!PATH(_d)) \
		return; \
	else \
		init_path(_d);

#define PRESERVE_PATH(_d, _p) \
	if (!(_p)) \
	{ \
		delete PATH(_d); \
		EXTRA(_d)->path = 0; \
	}

#define CREATE_PATH(_d) \
	if (!PATH(_d)) \
		EXTRA(_d)->path = new QPainterPath();

static void Clip(GB_PAINT *d, int preserve)
{
	CHECK_PATH(d);
	
	QPainterPath path = PAINTER(d)->worldTransform().map(*PATH(d));
	
	if (CLIP(d))
		path = CLIP(d)->intersected(path);
	
	delete EXTRA(d)->clip;
	EXTRA(d)->clip = new QPainterPath(path);
	
	PRESERVE_PATH(d, preserve);
}

static void ResetClip(GB_PAINT *d)
{
	delete CLIP(d);
	EXTRA(d)->clip = 0;
}

static void get_path_extents(QPainterPath *path, GB_EXTENTS *ext, const QTransform &transform)
{
	if (!path)
	{
		ext->x1 = ext->x2 = ext->y1 = ext->y2 = 0.0;
		return;
	}
	
	QRectF rect = transform.inverted().mapRect(path->boundingRect());
	
	ext->x1 = (float)rect.left();
	ext->y1 = (float)rect.top();
	ext->x2 = (float)rect.right();
	ext->y2 = (float)rect.bottom();
}

static void ClipExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	get_path_extents(CLIP(d), ext, PAINTER(d)->transform());
	/*if (EXTRA(d)->w > 0 && EXTRA(d)->h > 0)
	{
		int x1 = EXTRA(d)->x;
		int x2 = x1 + EXTRA(d)->w;
		int y1 = EXTRA(d)->y;
		int y2 = y1 + EXTRA(d)->h;
			
		if (ext->x2 > ext->x1 && ext->y2 > ext->y1)
		{
			if (x1 < ext->x1) ext->x1 = x1;
			if (x2 > ext->x2) ext->x2 = x2;
			if (y1 < ext->y1) ext->y1 = y1;
			if (y2 > ext->y2) ext->y2 = y2;
		}
		else
		{
			ext->x1 = x1;
			ext->x2 = x2;
			ext->y1 = y1;
			ext->y2 = y2;
		}
	}*/
}
	
static void Fill(GB_PAINT *d, int preserve)
{
	CHECK_PATH(d);
	
	if (!CLIP(d))
		PAINTER(d)->fillPath(*PATH(d), PAINTER(d)->brush());
	else
	{
		QPainterPath path = PAINTER(d)->worldTransform().inverted().map(*CLIP(d));
		path = path.intersected(*PATH(d));
		PAINTER(d)->fillPath(path, PAINTER(d)->brush());
	}
	
	PRESERVE_PATH(d, preserve);
}

static void Stroke(GB_PAINT *d, int preserve)
{
	CHECK_PATH(d);
	
	if (PAINTER(d)->pen().widthF() > 0.0)
	{
		if (!CLIP(d))
			PAINTER(d)->strokePath(*PATH(d), PAINTER(d)->pen());
		else
		{
			QPainterPathStroker stroker;
			QPen pen = PAINTER(d)->pen();
			
			stroker.setCapStyle(pen.capStyle());
			stroker.setDashOffset(pen.dashOffset());
			stroker.setDashPattern(pen.dashPattern());
			stroker.setJoinStyle(pen.joinStyle());
			stroker.setMiterLimit(pen.miterLimit());
			stroker.setWidth(pen.widthF());
			
			QPainterPath path = PAINTER(d)->worldTransform().inverted().map(*CLIP(d));
			path = path.intersected(stroker.createStroke(*PATH(d)));
			PAINTER(d)->fillPath(path, PAINTER(d)->brush());
		}
	}
	
	PRESERVE_PATH(d, preserve);
}
		
static void PathExtents(GB_PAINT *d, GB_EXTENTS *ext)
{
	get_path_extents(PATH(d), ext, PAINTER(d)->transform());
}

static int PathContains(GB_PAINT *d, float x, float y)
{
	if (!PATH(d))
		return FALSE;

	QPointF point((qreal)x, (qreal)y);
	return PATH(d)->contains(point);
}

		
static void Dash(GB_PAINT *d, int set, float **dashes, int *count)
{
	QPen pen = PAINTER(d)->pen();

	if (set)
	{
		if (!*count)
			pen.setStyle(Qt::SolidLine);
		else
		{
			QVector<qreal> dv;
			for (int i = 0; i < *count; i++)
				dv << (qreal)(*dashes)[i];
			pen.setStyle(Qt::CustomDashLine);
			pen.setDashPattern(dv);
		}
		PAINTER(d)->setPen(pen);
	}
	else
	{
		if (pen.style() == Qt::CustomDashLine)
		{
			QVector<qreal> dv = pen.dashPattern();
			*count = dv.count();
			GB.Alloc(POINTER(dashes), sizeof(float) * *count);
			for (int i = 0; i < *count; i++)
				(*dashes)[i] = (float)dv[i];
		}
		else
		{
			*count = 0;
			*dashes = NULL;
		}
	}
}

static void DashOffset(GB_PAINT *d, int set, float *offset)
{
	QPen pen = PAINTER(d)->pen();
	
	if (set)
	{
		pen.setDashOffset((qreal)*offset);
		PAINTER(d)->setPen(pen);
	}
	else
	{
		*offset = (float)pen.dashOffset();
	}
}

		
static void FillRule(GB_PAINT *d, int set, int *value)
{
	if (set)
	{
		EXTRA(d)->fillRule = *value;
	}
	else
		*value = EXTRA(d)->fillRule;
}

static void LineCap(GB_PAINT *d, int set, int *value)
{
	QPen pen = PAINTER(d)->pen();
	
	if (set)
	{
		switch (*value)
		{
			case GB_PAINT_LINE_CAP_ROUND:
				pen.setCapStyle(Qt::RoundCap); break;
			case GB_PAINT_LINE_CAP_SQUARE:
				pen.setCapStyle(Qt::SquareCap); break;
			case GB_PAINT_LINE_CAP_BUTT:
			default:
				pen.setCapStyle(Qt::FlatCap);
		}
		PAINTER(d)->setPen(pen);
	}
	else
	{
		switch (pen.capStyle())
		{
			case Qt::RoundCap: *value = GB_PAINT_LINE_CAP_ROUND; break;
			case Qt::SquareCap: *value = GB_PAINT_LINE_CAP_SQUARE; break;
			case Qt::FlatCap: default: *value = GB_PAINT_LINE_CAP_BUTT;
		}
	}
}

static void LineJoin(GB_PAINT *d, int set, int *value)
{
	QPen pen = PAINTER(d)->pen();
	
	if (set)
	{
		switch (*value)
		{
			case GB_PAINT_LINE_JOIN_ROUND:
				pen.setJoinStyle(Qt::RoundJoin); break;
			case GB_PAINT_LINE_JOIN_BEVEL:
				pen.setJoinStyle(Qt::BevelJoin); break;
			case GB_PAINT_LINE_JOIN_MITER:
			default:
				pen.setJoinStyle(Qt::MiterJoin);
		}
		PAINTER(d)->setPen(pen);
	}
	else
	{
		switch (pen.joinStyle())
		{
			case Qt::RoundJoin: *value = GB_PAINT_LINE_JOIN_ROUND; break;
			case Qt::BevelJoin: *value = GB_PAINT_LINE_JOIN_BEVEL; break;
			case Qt::MiterJoin: default: *value = GB_PAINT_LINE_JOIN_MITER;
		}
	}
}

static void LineWidth(GB_PAINT *d, int set, float *value)
{
	QPen pen = PAINTER(d)->pen();
	if (set)
	{
		pen.setWidthF((qreal)*value);
		PAINTER(d)->setPen(pen);
	}
	else
		*value = (float)pen.widthF();
}

static void MiterLimit(GB_PAINT *d, int set, float *value)
{
	QPen pen = PAINTER(d)->pen();
	if (set)
	{
		pen.setMiterLimit((qreal)*value);
		PAINTER(d)->setPen(pen);
	}
	else
		*value = (float)pen.miterLimit();
}


static void Operator(GB_PAINT *d, int set, int *value)
{
	QPainter::CompositionMode mode;
	
	if (set)
	{
		switch (*value)
		{
			case GB_PAINT_OPERATOR_CLEAR: mode = QPainter::CompositionMode_Clear; break;
			case GB_PAINT_OPERATOR_SOURCE: mode = QPainter::CompositionMode_Source; break;
			case GB_PAINT_OPERATOR_IN: mode = QPainter::CompositionMode_SourceIn; break;
			case GB_PAINT_OPERATOR_OUT: mode = QPainter::CompositionMode_SourceOut; break;
			case GB_PAINT_OPERATOR_ATOP: mode = QPainter::CompositionMode_SourceAtop; break;
			case GB_PAINT_OPERATOR_DEST: mode = QPainter::CompositionMode_Destination; break;
			case GB_PAINT_OPERATOR_DEST_OVER: mode = QPainter::CompositionMode_DestinationOver; break;
			case GB_PAINT_OPERATOR_DEST_IN: mode = QPainter::CompositionMode_DestinationIn; break;
			case GB_PAINT_OPERATOR_DEST_OUT: mode = QPainter::CompositionMode_DestinationOut; break;
			case GB_PAINT_OPERATOR_DEST_ATOP: mode = QPainter::CompositionMode_DestinationAtop; break;
			case GB_PAINT_OPERATOR_XOR: mode = QPainter::CompositionMode_Xor; break;
			case GB_PAINT_OPERATOR_ADD: mode = QPainter::CompositionMode_Plus; break;
			case GB_PAINT_OPERATOR_SATURATE: mode = QPainter::CompositionMode_Multiply; break;
			case GB_PAINT_OPERATOR_OVER: default: mode = QPainter::CompositionMode_SourceOver;
		}
		PAINTER(d)->setCompositionMode(mode);
	}
	else
	{
		switch (PAINTER(d)->compositionMode())
		{
			case QPainter::CompositionMode_Clear: *value = GB_PAINT_OPERATOR_CLEAR; break;
			case QPainter::CompositionMode_Source: *value = GB_PAINT_OPERATOR_SOURCE; break;
			case QPainter::CompositionMode_SourceIn: *value = GB_PAINT_OPERATOR_IN; break;
			case QPainter::CompositionMode_SourceOut: *value = GB_PAINT_OPERATOR_OUT; break;
			case QPainter::CompositionMode_SourceAtop: *value = GB_PAINT_OPERATOR_ATOP; break;
			case QPainter::CompositionMode_Destination: *value = GB_PAINT_OPERATOR_DEST; break;
			case QPainter::CompositionMode_DestinationOver: *value = GB_PAINT_OPERATOR_DEST_OVER; break;
			case QPainter::CompositionMode_DestinationIn: *value = GB_PAINT_OPERATOR_DEST_IN; break;
			case QPainter::CompositionMode_DestinationOut: *value = GB_PAINT_OPERATOR_DEST_OUT; break;
			case QPainter::CompositionMode_DestinationAtop: *value = GB_PAINT_OPERATOR_DEST_ATOP; break;
			case QPainter::CompositionMode_Xor: *value = GB_PAINT_OPERATOR_XOR; break;
			case QPainter::CompositionMode_Plus: *value = GB_PAINT_OPERATOR_ADD; break;
			case QPainter::CompositionMode_Multiply: *value = GB_PAINT_OPERATOR_SATURATE; break;
			case QPainter::CompositionMode_SourceOver: default: *value = GB_PAINT_OPERATOR_OVER;
		}
	}
}


static void NewPath(GB_PAINT *d)
{
	PRESERVE_PATH(d, FALSE);
}

static void ClosePath(GB_PAINT *d)
{
	CHECK_PATH(d);
	PATH(d)->closeSubpath();
}

		
static void Arc(GB_PAINT *d, float xc, float yc, float radius, float angle, float length)
{
	CREATE_PATH(d);

	QRectF rect;
	rect.setCoords((qreal)(xc - radius), (qreal)(yc - radius), (qreal)(xc + radius), (qreal)(yc + radius));
	
	angle = - angle;
	length = - length;
	
	PATH(d)->arcMoveTo(rect, to_deg(angle));
	PATH(d)->arcTo(rect, to_deg(angle), to_deg(length));
}

static void Rectangle(GB_PAINT *d, float x, float y, float width, float height)
{
	CREATE_PATH(d);
	PATH(d)->addRect((qreal)x, (qreal)y, (qreal)width, (qreal)height);
}

static void GetCurrentPoint(GB_PAINT *d, float *x, float *y)
{
	if (!PATH(d))
	{
		*x = 0;
		*y = 0;
		return;
	}
	
	QPointF pt = PATH(d)->currentPosition();
	*x = (float)pt.x();
	*y = (float)pt.y();
}

static void MoveTo(GB_PAINT *d, float x, float y)
{
	CREATE_PATH(d);
	PATH(d)->moveTo((qreal)x, (qreal)y);
}

static void LineTo(GB_PAINT *d, float x, float y)
{
	CREATE_PATH(d);
	PATH(d)->lineTo((qreal)x, (qreal)y);
}

static void CurveTo(GB_PAINT *d, float x1, float y1, float x2, float y2, float x3, float y3)
{
	CREATE_PATH(d);
	PATH(d)->cubicTo(QPointF((qreal)x1, (qreal)y1), QPointF((qreal)x2, (qreal)y2), QPointF((qreal)x3, (qreal)y3));
}

static QPainterPath *_draw_path;
static float _draw_x, _draw_y;

static void draw_text(GB_PAINT *d, bool rich, const char *text, int len, float w, float h, int align, bool draw)
{
	QPointF pos;
	
	GetCurrentPoint(d, &_draw_x, &_draw_y);
	
	if (draw)
	{
		if (CLIP(d))
		{
			QTransform save = PAINTER(d)->worldTransform();
			PAINTER(d)->resetTransform();
			PAINTER(d)->setClipping(true);
			PAINTER(d)->setClipPath(*CLIP(d));
			PAINTER(d)->setWorldTransform(save);
		}
		
		if (rich)
			DRAW_rich_text(PAINTER(d), QString::fromUtf8(text, len), _draw_x, _draw_y, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		else
			DRAW_text(PAINTER(d), QString::fromUtf8(text, len), _draw_x, _draw_y, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		
		if (CLIP(d))
			PAINTER(d)->setClipping(false);
	}
	else
	{
		CREATE_PATH(d);
	
		_draw_path = PATH(d);

		if (w <= 0 || h <= 0)
			_draw_y -= PAINTER(d)->fontMetrics().ascent();
		
		MyPaintDevice device;
		QPainter p(&device);
		
		p.setFont(PAINTER(d)->font());
		
		if (rich)
			DRAW_rich_text(&p, QString::fromUtf8(text, len), 0, 0, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		else
			DRAW_text(&p, QString::fromUtf8(text, len), 0, 0, w, h, CCONST_alignment(align, ALIGN_TOP_NORMAL, true));	
		
		p.end();
		_draw_path = NULL;
	}
}

static void Text(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw)
{
	draw_text(d, false, text, len, w, h, align, draw);
}

static void RichText(GB_PAINT *d, const char *text, int len, float w, float h, int align, bool draw)
{
	draw_text(d, true, text, len, w, h, align, draw);
}

static void get_text_extents(GB_PAINT *d, bool rich, const char *text, int len, GB_EXTENTS *ext, float width)
{
	QPainterPath path;
	MyPaintDevice device;
	QPainter p(&device);
	
	p.setFont(PAINTER(d)->font());
	_draw_path = &path;
	GetCurrentPoint(d, &_draw_x, &_draw_y);
	_draw_y -= PAINTER(d)->fontMetrics().ascent();
	
	if (rich)
		DRAW_rich_text(&p, QString::fromUtf8(text, len), 0, 0, width, -1, CCONST_alignment(ALIGN_TOP_NORMAL, ALIGN_TOP_NORMAL, true));	
	else
		DRAW_text(&p, QString::fromUtf8(text, len), 0, 0, -1, -1, CCONST_alignment(ALIGN_TOP_NORMAL, ALIGN_TOP_NORMAL, true));	
	
	p.end();
	
	get_path_extents(&path, ext, QTransform());
	_draw_path = NULL;
}

static void TextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext)
{
	get_text_extents(d, false, text, len, ext, -1);
}

static void RichTextExtents(GB_PAINT *d, const char *text, int len, GB_EXTENTS *ext, float width)
{
	get_text_extents(d, true, text, len, ext, width);
}

static void Matrix(GB_PAINT *d, int set, GB_TRANSFORM matrix)
{
	QTransform *t = (QTransform *)matrix;
	
	if (set)
	{
		if (t)
			PAINTER(d)->setWorldTransform(*t);
		else
			PAINTER(d)->resetTransform();
	}
	else
		*t = PAINTER(d)->worldTransform();
}

		
static void SetBrush(GB_PAINT *d, GB_BRUSH brush)
{
	QBrush *b = (QBrush *)brush;
	PAINTER(d)->setBrush(*b);

	QPen p = PAINTER(d)->pen();
	p.setBrush(*b);
	PAINTER(d)->setPen(p);
	//PAINTER(d)->setBrushOrigin(QPointF((qreal)x, (qreal)y));
}

static void DrawImage(GB_PAINT *d, GB_IMAGE image, float x, float y, float w, float h)
{
	QImage *img = CIMAGE_get((CIMAGE *)image);
	QRectF rect(x, y, w, h);
	
	PAINTER(d)->drawImage(rect, *img);
}
		
static void BrushFree(GB_BRUSH brush)
{
	delete (QBrush *)brush;
}

static void BrushColor(GB_BRUSH *brush, GB_COLOR color)
{
	QBrush *br = new QBrush(CCOLOR_make(color));
	*brush = (GB_BRUSH)br;
}

static void BrushImage(GB_BRUSH *brush, GB_IMAGE image)
{
	QImage *img = CIMAGE_get((CIMAGE *)image);
	
	QBrush *br = new QBrush(*img);
	*brush = (GB_BRUSH)br;
}

static void BrushLinearGradient(GB_BRUSH *brush, float x0, float y0, float x1, float y1, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	QLinearGradient gradient;
	int i;
	
	gradient.setStart((qreal)x0, (qreal)y0);
	gradient.setFinalStop((qreal)x1, (qreal)y1);
	
	for (i = 0; i < nstop; i++)
		gradient.setColorAt((qreal)positions[i], CCOLOR_make(colors[i]));

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT:
			gradient.setSpread(QGradient::RepeatSpread); break;
		case GB_PAINT_EXTEND_REFLECT:
			gradient.setSpread(QGradient::ReflectSpread); break;
		case GB_PAINT_EXTEND_PAD:
		default:
			gradient.setSpread(QGradient::PadSpread);
	}

	QBrush *br = new QBrush(gradient);
	*brush = br;
}

static void BrushRadialGradient(GB_BRUSH *brush, float cx, float cy, float r, float fx, float fy, int nstop, double *positions, GB_COLOR *colors, int extend)
{
	QRadialGradient gradient;
	int i;
	
	gradient.setCenter((qreal)cx, (qreal)cy);
	gradient.setRadius((qreal)r);
	gradient.setFocalPoint((qreal)fx, (qreal)fy);
	
	for (i = 0; i < nstop; i++)
		gradient.setColorAt((qreal)positions[i], CCOLOR_make(colors[i]));

	switch (extend)
	{
		case GB_PAINT_EXTEND_REPEAT:
			gradient.setSpread(QGradient::RepeatSpread); break;
		case GB_PAINT_EXTEND_REFLECT:
			gradient.setSpread(QGradient::ReflectSpread); break;
		case GB_PAINT_EXTEND_PAD:
		default:
			gradient.setSpread(QGradient::PadSpread);
	}

	QBrush *br = new QBrush(gradient);
	*brush = br;
}

static void BrushMatrix(GB_BRUSH brush, int set, GB_TRANSFORM matrix)
{
	QBrush *b = (QBrush *)brush;
	QTransform *t = (QTransform *)matrix;
	
	if (set)
	{
		if (t)
			b->setTransform(*t);
		else
			b->setTransform(QTransform());
	}
	else
		*t = b->transform();
}

static void TransformCreate(GB_TRANSFORM *matrix)
{
	*matrix = (GB_TRANSFORM)(new QTransform());
}

static void TransformDelete(GB_TRANSFORM *matrix)
{
	delete (QTransform *)*matrix;
	*matrix = 0;
}

static void TransformInit(GB_TRANSFORM matrix, float xx, float yx, float xy, float yy, float x0, float y0)
{
	QTransform *t = (QTransform *)matrix;
	QMatrix m((qreal)xx, (qreal)yx, (qreal)xy, (qreal)yy, (qreal)x0, (qreal)y0);
	QTransform t2(m);
	*t = t2;
}

static void TransformTranslate(GB_TRANSFORM matrix, float tx, float ty)
{
	QTransform *t = (QTransform *)matrix;
	t->translate((qreal)tx, (qreal)ty);
}

static void TransformScale(GB_TRANSFORM matrix, float sx, float sy)
{
	QTransform *t = (QTransform *)matrix;
	t->scale((qreal)sx, (qreal)sy);
}

static void TransformRotate(GB_TRANSFORM matrix, float angle)
{
	QTransform *t = (QTransform *)matrix;
	t->rotate(to_deg(-angle));
}

static int TransformInvert(GB_TRANSFORM matrix)
{
	QTransform *t  = (QTransform *)matrix;
	bool inv;
	QTransform it = t->inverted(&inv);
	if (inv)
	{
		*t = it;
		return FALSE;
	}
	else
		return TRUE;
}

static void TransformMultiply(GB_TRANSFORM matrix, GB_TRANSFORM matrix2)
{
	QTransform *t  = (QTransform *)matrix;
	QTransform *t2  = (QTransform *)matrix2;
	
	*t = *t * *t2;
}


GB_PAINT_DESC PAINT_Interface = {
	// Size of the GB_PAINT structure extra data
	sizeof(QT_PAINT_EXTRA),
	Begin,
	End,
	Save,
	Restore,
	Font,
	Clip,
	ResetClip,
	ClipExtents,
	Fill,
	Stroke,
	PathExtents,
	PathContains,
	Dash,
	DashOffset,
	FillRule,
	LineCap,
	LineJoin,
	LineWidth,
	MiterLimit,
	Operator,
	NewPath,
	ClosePath,
	Arc,
	Rectangle,
	GetCurrentPoint,
	MoveTo,
	LineTo,
	CurveTo,
	Text,
	TextExtents,
	RichText,
	RichTextExtents,
	Matrix,
	SetBrush,
	DrawImage,
	{
		BrushFree,
		BrushColor,
		BrushImage,
		BrushLinearGradient,
		BrushRadialGradient,
		BrushMatrix,
	},
	{
		TransformCreate,
		TransformDelete,
		TransformInit,
		TransformTranslate,
		TransformScale,
		TransformRotate,
		TransformInvert,
		TransformMultiply
	}
};

void PAINT_begin(void *device)
{
	DRAW.Paint.Begin(device);
}

void PAINT_end()
{
	DRAW.Paint.End();
}

QPainter *PAINT_get_current()
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	return d ? PAINTER(d) : NULL;
}

void PAINT_get_current_point(float *x, float *y)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (!d)
		return;
	GetCurrentPoint(d, x, y);
}

void PAINT_clip(int x, int y, int w, int h)
{
	GB_PAINT *d = (GB_PAINT *)DRAW.Paint.GetCurrent();
	if (d)
	{
		/*PAINTER(d)->setClipRect(x, y, w, h);
		EXTRA(d)->x = x;
		EXTRA(d)->y = y;
		EXTRA(d)->w = w;
		EXTRA(d)->h = h;*/
		Rectangle(d, x, y, w, h);
		Clip(d, FALSE);
	}
}

/*************************************************************************/

MyPaintEngine::MyPaintEngine() : QPaintEngine(0) {}
MyPaintEngine::~MyPaintEngine() {}

void MyPaintEngine::patchFeatures()
{
	if (type() == PostScript || type() == Pdf)
	{
		QPaintEngine::PaintEngineFeatures f = QPaintEngine::AllFeatures;
    f &= (QPaintEngine::PorterDuff | QPaintEngine::PerspectiveTransform
           | QPaintEngine::ObjectBoundingModeGradients
           | QPaintEngine::LinearGradientFill
           | QPaintEngine::RadialGradientFill
           | QPaintEngine::ConicalGradientFill);
		qWarning("warning: patching current paint engine");
		gccaps = f; //PerspectiveTransform;
	}
}

bool MyPaintEngine::begin(QPaintDevice *pdev) 
{
	setActive(true);
	return true; 
}

bool MyPaintEngine::end() 
{
	setActive(false);
	return true;
}

void MyPaintEngine::updateState(const QPaintEngineState &state) 
{
	//qDebug("MyPaintEngine::updateState: %04X", (int)state.state());
}

void MyPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
	//qDebug("MyPaintEngine::drawRects");
}

void MyPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
	//qDebug("MyPaintEngine::drawLines");
}

void MyPaintEngine::drawEllipse(const QRectF &r)
{
	//qDebug("MyPaintEngine::drawEllipse");
}

void MyPaintEngine::drawPath(const QPainterPath &path)
{
	//qDebug("MyPaintEngine::drawPath");
}

void MyPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
	//qDebug("MyPaintEngine::drawPoints");
}

void MyPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
	//qDebug("MyPaintEngine::drawPolygon");
}


void MyPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
	//qDebug("MyPaintEngine::drawPixmap");
}

void MyPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
	//qDebug("MyPaintEngine::drawTiledPixmap");
}

void MyPaintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags)
{
	//qDebug("MyPaintEngine::drawImage");
}


//QPoint MyPaintEngine::coordinateOffset() const;

MyPaintEngine::Type MyPaintEngine::type() const
{
	return QPaintEngine::User;
}

void MyPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
	//qDebug("MyPaintEngine::drawTextItem: %g %g [%s] '%s'", p.x() + _draw_x, p.y() + _draw_y, (const char *)textItem.font().toString().toUtf8(), (const char *)textItem.text().toUtf8());
	//_draw_path->moveTo(p.x() + _draw_x, p.y() + _draw_y);
	_draw_path->addText(p.x() + _draw_x, p.y() + _draw_y, textItem.font(), textItem.text());
}

/*************************************************************************/

MyPaintEngine MyPaintDevice::engine;

MyPaintDevice::MyPaintDevice() : QPaintDevice() //QImage(1, 1, QImage::Format_ARGB32)
{
}

QPaintEngine *MyPaintDevice::paintEngine() const
{ 
	return &engine; 
}

int MyPaintDevice::metric(PaintDeviceMetric m) const
{
	QPaintDevice *d = PAINT_get_current()->device();

	switch(m)
	{
		case PdmWidth: return d->width();
		case PdmHeight: return d->height();
		case PdmWidthMM: return d->widthMM();
		case PdmHeightMM: return d->heightMM();
		#if QT_VERSION <= 0x040600
		case PdmNumColors: return d->numColors();
		#else
		case PdmNumColors: return d->colorCount();
		#endif
		case PdmDepth: return d->depth();
		case PdmDpiX: return d->logicalDpiX();
		case PdmDpiY: return d->logicalDpiY();
		case PdmPhysicalDpiX: return d->physicalDpiX();
		case PdmPhysicalDpiY: return d->physicalDpiY();
		default: return 0;
	}
}
