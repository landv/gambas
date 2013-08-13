/***************************************************************************

  c_clipper.cpp

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

#define __C_CLIPPER_CPP

#include "main.h"
#include "gb.geom.h"
#include "c_clipper.h"

#define SCALE 1000000.0

static IntPoint to_point(GEOM_POINTF *point)
{
	return IntPoint(point->x * SCALE + 0.5, point->y * SCALE + 0.5);
}

static GEOM_POINTF *from_point(IntPoint p)
{
	return GEOM.CreatePointF((double)p.X / SCALE, (double)p.Y / SCALE);
}

static bool to_polygons(Polygons &polygons, GB_ARRAY array)
{
	int count;
	CPOLYGON *p;
	int i;

	if (GB.CheckObject(array))
		return true;

	count = GB.Array.Count(array);
	if (count == 0)
		return false;

	polygons.clear();

	for(i = 0; i < count; i++)
	{
		p = *(CPOLYGON **)GB.Array.Get(array, i);
		if (!p)
			continue;

		polygons.push_back(*(p->poly));
	}

	return false;
}

static GB_ARRAY from_polygons(Polygons &polygons)
{
	GB_ARRAY a;
	CPOLYGON *p;
	uint i;

	GB.Array.New(&a, GB.FindClass("Polygon"), polygons.size());

	for (i = 0; i < polygons.size(); i++)
	{
		if (polygons[i].size() == 0)
			continue;
		
		p = (CPOLYGON *)GB.New(GB.FindClass("Polygon"), NULL, NULL);
		*(p->poly) = polygons[i];

		*(GB_ARRAY *)GB.Array.Get(a, i) = p;
		GB.Ref(p);
	}

	return a;
}

//---------------------------------------------------------------------------

#define THIS ((CPOLYGON *)_object)
#define POLY THIS->poly

static bool _convert_polygon(CPOLYGON *_object, GB_TYPE type, GB_VALUE *conv)
{
	if (type != GB.FindClass("PointF[]"))
		return true;

	if (THIS)
	{
		// Polygon --> PointF[]
		GB_ARRAY a;
		int i;
		GEOM_POINTF **data;

		GB.Array.New(&a, GB.FindClass("PointF"), POLY->size() + 1);
		data = (GEOM_POINTF **)GB.Array.Get(a, 0);
		for(i = 0; i < (int)POLY->size(); i++)
		{
			data[i] = from_point((*POLY)[i]);
			GB.Ref(data[i]);
		}

		data[i] = from_point((*POLY)[0]);
		GB.Ref(data[i]);

		conv->_object.value = a;
		return false;
	}
	else
	{
		// PointF[] --> Polygon
		CPOLYGON *p;
		GB_ARRAY a = (GB_ARRAY)conv->_object.value;
		int size = GB.Array.Count(a);
		int i;
		GEOM_POINTF **points;

		p = (CPOLYGON *)GB.New(GB.FindClass("Polygon"), NULL, NULL);

		points = (GEOM_POINTF **)GB.Array.Get(a, 0);
		for (i = 0; i < size; i++)
		{
			if (!points[i])
				continue;

			p->poly->push_back(to_point(points[i]));
		}

		conv->_object.value = p;
		return false;
	}
}

BEGIN_METHOD(Polygon_new, GB_INTEGER size)

	POLY = new Polygon;

	if (!MISSING(size))
		POLY->resize(VARG(size));

END_METHOD

BEGIN_METHOD_VOID(Polygon_free)

	delete POLY;

END_METHOD

BEGIN_METHOD(Polygon_get, GB_INTEGER index)

	int index = VARG(index);

	if (index < 0 || index >= (int)POLY->size())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	GB.ReturnObject(from_point((*POLY)[index]));

END_METHOD

BEGIN_METHOD(Polygon_put, GB_OBJECT point; GB_INTEGER index)

	int index = VARG(index);
	GEOM_POINTF *point = (GEOM_POINTF *)VARG(point);

	if (GB.CheckObject(point))
		return;

	if (index < 0 || index >= (int)POLY->size())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	(*POLY)[index] = to_point(point);

END_METHOD

BEGIN_PROPERTY(Polygon_Count)

	GB.ReturnInteger(POLY->size());

END_PROPERTY

BEGIN_PROPERTY(Polygon_Max)

	GB.ReturnInteger(POLY->size() - 1);

END_PROPERTY

BEGIN_PROPERTY(Polygon_Area)

	GB.ReturnFloat(Area(*POLY) / SCALE / SCALE);

END_PROPERTY

BEGIN_METHOD_VOID(Polygon_Reverse)

	ReversePolygon(*POLY);

END_METHOD

BEGIN_METHOD(Polygon_Simplify, GB_INTEGER fill)

	Polygons result;

	SimplifyPolygon(*POLY, result, (PolyFillType)VARGOPT(fill, pftNonZero));

	GB.ReturnObject(from_polygons(result));

END_METHOD

BEGIN_METHOD(Polygon_Clean, GB_FLOAT distance)

	CPOLYGON *result = (CPOLYGON *)GB.New(GB.FindClass("Polygon"), NULL, 0);

	result->poly->resize(POLY->size());

	CleanPolygon(*POLY, *(result->poly), VARGOPT(distance, 1.415));

	GB.ReturnObject(result);

END_METHOD

//---------------------------------------------------------------------------

BEGIN_METHOD(Clipper_Offset, GB_OBJECT polygons; GB_FLOAT delta; GB_INTEGER join; GB_FLOAT limit; GB_BOOLEAN do_not_fix)

	Polygons polygons;
	Polygons result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	SimplifyPolygons(polygons, result, pftNonZero);
	polygons = result;
	OffsetPolygons(polygons, result, VARG(delta) * SCALE, (JoinType)VARGOPT(join, jtSquare), VARGOPT(limit, 0.0), !VARGOPT(do_not_fix, false));

	GB.ReturnObject(from_polygons(result));

END_METHOD


BEGIN_METHOD(Clipper_Simplify, GB_OBJECT polygons; GB_INTEGER fill)

	Polygons polygons;
	Polygons result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	SimplifyPolygons(polygons, result, (PolyFillType)VARGOPT(fill, pftNonZero));

	GB.ReturnObject(from_polygons(result));

END_METHOD

BEGIN_METHOD(Clipper_Clean, GB_OBJECT polygons; GB_FLOAT distance)

	Polygons polygons;
	Polygons result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	result.resize(polygons.size());

	CleanPolygons(polygons, result, VARGOPT(distance, 1.415));

	GB.ReturnObject(from_polygons(result));

END_METHOD

BEGIN_METHOD(Clipper_Union, GB_OBJECT subject; GB_INTEGER fill)

	Clipper c;
	Polygons subject, result;

	if (to_polygons(subject, VARG(subject)))
		return;

	//if (VARG(clip) && to_polygons(clip, VARG(clip)))
	//	return;

	c.AddPolygons(subject, ptSubject);
	//c.AddPolygons(clip, ptClip);
	c.Execute(ctUnion, result, (PolyFillType)VARGOPT(fill, pftNonZero), (PolyFillType)VARGOPT(fill, pftNonZero));

	GB.ReturnObject(from_polygons(result));

END_METHOD

//---------------------------------------------------------------------------

GB_DESC PolygonDesc[] =
{
	GB_DECLARE("Polygon", sizeof(CPOLYGON)),

	GB_METHOD("_new", NULL, Polygon_new, "[(Size)i]"),
	GB_METHOD("_free", NULL, Polygon_free, NULL),

	GB_METHOD("_get", "PointF", Polygon_get, "(Index)i"),
	GB_METHOD("_put", NULL, Polygon_put, "(Index)i(Point)PointF;"),

	GB_PROPERTY_READ("Count", "i", Polygon_Count),
	GB_PROPERTY_READ("Max", "i", Polygon_Max),
	GB_PROPERTY_READ("Area", "f", Polygon_Area),

	GB_METHOD("Reverse", NULL, Polygon_Reverse, NULL),
	GB_METHOD("Simplify", "Polygon[]", Polygon_Simplify, "[(Fill)i]"),
	GB_METHOD("Clean", "Polygon", Polygon_Clean, "[(Distance)f]"),

	GB_INTERFACE("_convert", &_convert_polygon),

	GB_END_DECLARE
};

GB_DESC ClipperDesc[] =
{
	GB_DECLARE_VIRTUAL("Clipper"),

	//void OffsetPolygons(const Polygons &in_polys, Polygons &out_polys, double delta, JoinType jointype = jtSquare, double limit = 0.0, bool autoFix = true);
	//void SimplifyPolygons(const Polygons &in_polys, Polygons &out_polys, PolyFillType fillType = pftEvenOdd);
	//void CleanPolygons(Polygons &in_polys, Polygon &out_polys, double distance = 1.415);

	GB_CONSTANT("JoinMiter", "i", jtMiter),
	GB_CONSTANT("JoinSquare", "i", jtSquare),
	GB_CONSTANT("JoinRound", "i", jtRound),

	GB_CONSTANT("FillEvenOdd", "i", pftEvenOdd),
	GB_CONSTANT("FillWinding", "i", pftNonZero),
	GB_CONSTANT("FillNonZero", "i", pftNonZero),
	GB_CONSTANT("FillPositive", "i", pftPositive),
	GB_CONSTANT("FillNegative", "i", pftNegative),

	GB_STATIC_METHOD("Offset", "Polygon[]", Clipper_Offset, "(Polygons)Polygon[];(Delta)f[(Join)i(Limit)f(DoNotFix)b]"),
	GB_STATIC_METHOD("Simplify", "Polygon[]", Clipper_Simplify, "(Polygons)Polygon[];[(Fill)i]"),
	GB_STATIC_METHOD("Clean", "Polygon[]", Clipper_Clean, "(Polygons)Polygon[];[(Distance)f]"),

	GB_STATIC_METHOD("Union", "Polygon[]", Clipper_Union, "(Polygons)Polygon[];[(Fill)i]"),

	GB_END_DECLARE
};


