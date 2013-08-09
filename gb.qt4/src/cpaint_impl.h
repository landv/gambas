/***************************************************************************

  cpaint_impl.h

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

#ifndef __CPAINT_IMPL_H
#define __CPAINT_IMPL_H

#include "gambas.h"
#include "gb.paint.h"

#include <QPaintEngine>
#include <QPaintDevice>
#include <QImage>
#include <QPainter>
#include <QString>

#ifndef __CPAINT_IMPL_C

extern GB_PAINT_DESC PAINT_Interface;
extern GB_PAINT_MATRIX_DESC PAINT_MATRIX_Interface;

#endif

void PAINT_begin(void *device);
void PAINT_end();
QPainter *PAINT_get_current();
void PAINT_get_current_point(float *x, float *y);
void PAINT_clip(int x, int y, int w, int h);

class MyPaintEngine: public QPaintEngine
{
public:
	MyPaintEngine();
  virtual ~MyPaintEngine();

	virtual bool begin(QPaintDevice *pdev);
	virtual bool end();

	virtual void updateState(const QPaintEngineState &state);

	virtual void drawRects(const QRectF *rects, int rectCount);
	virtual void drawLines(const QLineF *lines, int lineCount);
	virtual void drawEllipse(const QRectF &r);
	virtual void drawPath(const QPainterPath &path);
	virtual void drawPoints(const QPointF *points, int pointCount);
	virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
	virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
	virtual void drawTextItem(const QPointF &p, const QTextItem &textItem);
	virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);
	virtual void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr, Qt::ImageConversionFlags flags = Qt::AutoColor);

	//virtual QPoint coordinateOffset() const;

	virtual Type type() const;
	
	void patchFeatures();
};

class MyPaintDevice: public QPaintDevice
{
public:
	MyPaintDevice();
	virtual QPaintEngine *paintEngine() const;
	
protected:
	virtual int metric(PaintDeviceMetric m) const;

private:
	static MyPaintEngine engine;
};


#endif
