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

static IntPoint to_point_xy(double x, double y)
{
	return IntPoint(x * SCALE + 0.5, y * SCALE + 0.5);
}

static IntPoint to_point(GEOM_POINTF *point)
{
	return to_point_xy(point->x, point->y);
}

static GEOM_POINTF *from_point(IntPoint p)
{
	return GEOM.CreatePointF((double)p.X / SCALE, (double)p.Y / SCALE);
}

static bool is_polygon_closed(Path &p)
{
	int n = p.size() - 1;

	if (n <= 1)
		return false;

	return p[0].X == p[n].X && p[0].Y == p[n].Y;
}

static void set_polygon_closed(Path &p, bool closed)
{
	if (is_polygon_closed(p) == closed)
		return;

	if (closed)
		p.push_back(p[0]);
	else
		p.erase(p.begin() + p.size() - 1);
}

static bool to_polygons(Paths &polygons, GB_ARRAY array)
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

static GB_ARRAY from_polygons(Paths &polygons, bool closed)
{
	GB_ARRAY a;
	CPOLYGON *p;
	uint i;

	GB.Array.New(&a, GB.FindClass("Polygon"), polygons.size());

	for (i = 0; i < polygons.size(); i++)
	{
		if (polygons[i].size() == 0)
			continue;
		
		set_polygon_closed(polygons[i], closed);

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

		GB.Array.New(&a, GB.FindClass("PointF"), POLY->size()); // + THIS->closed);
		data = (GEOM_POINTF **)GB.Array.Get(a, 0);
		for(i = 0; i < (int)POLY->size(); i++)
		{
			data[i] = from_point((*POLY)[i]);
			GB.Ref(data[i]);
		}

		/*if (closed)
		{
			data[i] = from_point((*POLY)[0]);
			GB.Ref(data[i]);
		}*/

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

	POLY = new Path;

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

	ReversePath(*POLY);

END_METHOD

BEGIN_METHOD(Polygon_Simplify, GB_INTEGER fill)

	Paths result;

	SimplifyPolygon(*POLY, result, (PolyFillType)VARGOPT(fill, pftNonZero));

	GB.ReturnObject(from_polygons(result, is_polygon_closed(*POLY)));

END_METHOD

BEGIN_METHOD(Polygon_Clean, GB_FLOAT distance)

	bool closed;
	CPOLYGON *result = (CPOLYGON *)GB.New(GB.FindClass("Polygon"), NULL, 0);

	result->poly->resize(POLY->size());

	closed = is_polygon_closed(*POLY);

	CleanPolygon(*POLY, *(result->poly), VARGOPT(distance, 1.415));

	set_polygon_closed(*(result->poly), closed);

	GB.ReturnObject(result);

END_METHOD

BEGIN_METHOD(Polygon_Add, GB_FLOAT x; GB_FLOAT y)

	POLY->push_back(to_point_xy(VARG(x), VARG(y)));

END_METHOD

BEGIN_METHOD(Polygon_AddPoint, GB_OBJECT point)

	GEOM_POINTF *point = (GEOM_POINTF *)VARG(point);

	if (GB.CheckObject(point))
		return;

	POLY->push_back(to_point(point));

END_METHOD

BEGIN_METHOD(Polygon_Remove, GB_INTEGER index; GB_INTEGER count)

	int index = VARG(index);
	int count = VARGOPT(count, 1);
	int index2;

	if (index < 0 || index >= (int)POLY->size())
	{
		GB.Error(GB_ERR_BOUND);
		return;
	}

	if (count < 0)
		count = (int)POLY->size() - index;

	index2 = index + count;

	if (index2 > (int)POLY->size())
		index2 = POLY->size();

	if (count == 1)
		POLY->erase(POLY->begin() + index);
	else
		POLY->erase(POLY->begin() + index, POLY->begin() + index2);

END_METHOD

BEGIN_PROPERTY(Polygon_Orientation)

	GB.ReturnBoolean(Orientation(*POLY));

END_PROPERTY

//---------------------------------------------------------------------------

BEGIN_METHOD(Clipper_Offset, GB_OBJECT polygons; GB_FLOAT delta; GB_INTEGER join; GB_FLOAT limit; GB_BOOLEAN do_not_fix)

	Paths polygons;
	Paths result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	SimplifyPolygons(polygons, result, pftNonZero);
	polygons = result;

	ClipperOffset co;
	co.AddPaths(polygons, (JoinType)VARGOPT(join, jtSquare), etClosedPolygon);
	co.MiterLimit = VARGOPT(limit, 0.0);
	co.Execute(result, VARG(delta) * SCALE);

	//OffsetPaths(polygons, result, VARG(delta) * SCALE, (JoinType)VARGOPT(join, jtSquare), VARGOPT(limit, 0.0), !VARGOPT(do_not_fix, false));

	GB.ReturnObject(from_polygons(result, true));

END_METHOD


BEGIN_METHOD(Clipper_Simplify, GB_OBJECT polygons; GB_INTEGER fill)

	Paths polygons;
	Paths result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	SimplifyPolygons(polygons, result, (PolyFillType)VARGOPT(fill, pftNonZero));

	GB.ReturnObject(from_polygons(result, true));

END_METHOD

BEGIN_METHOD(Clipper_Clean, GB_OBJECT polygons; GB_FLOAT distance)

	Paths polygons;
	Paths result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	result.resize(polygons.size());

	CleanPolygons(polygons, result, VARGOPT(distance, 1.415));

	GB.ReturnObject(from_polygons(result, true));

END_METHOD

static void execute(ClipType action, PolyFillType fill, void *subject, void *clip)
{
	Clipper c;
	Paths psubject, pclip, result;
	PolyTree tree;

	if (to_polygons(psubject, subject))
		return;

	if (clip && to_polygons(pclip, clip))
		return;

	c.AddPaths(psubject, ptSubject, true);
	if (clip)
		c.AddPaths(pclip, ptClip, true);

	c.StrictlySimple(true);
	c.Execute(action, tree, fill, fill);
	ClosedPathsFromPolyTree(tree, result);

	GB.ReturnObject(from_polygons(result, true));
}

BEGIN_METHOD(Clipper_Union, GB_OBJECT subject; GB_OBJECT clip; GB_INTEGER fill)

	execute(ctUnion, (PolyFillType)VARGOPT(fill, pftNonZero), VARG(subject), VARGOPT(clip, NULL));

END_METHOD

BEGIN_METHOD(Clipper_Intersection, GB_OBJECT subject; GB_OBJECT clip; GB_INTEGER fill)

	execute(ctIntersection, (PolyFillType)VARGOPT(fill, pftNonZero), VARG(subject), VARGOPT(clip, NULL));

END_METHOD

BEGIN_METHOD(Clipper_Difference, GB_OBJECT subject; GB_OBJECT clip; GB_INTEGER fill)

	execute(ctDifference, (PolyFillType)VARGOPT(fill, pftNonZero), VARG(subject), VARGOPT(clip, NULL));

END_METHOD

BEGIN_METHOD(Clipper_ExclusiveOr, GB_OBJECT subject; GB_OBJECT clip; GB_INTEGER fill)

	execute(ctXor, (PolyFillType)VARGOPT(fill, pftNonZero), VARG(subject), VARGOPT(clip, NULL));

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
	GB_PROPERTY_READ("Orientation", "b", Polygon_Orientation),

	GB_METHOD("Reverse", NULL, Polygon_Reverse, NULL),
	GB_METHOD("Simplify", "Polygon[]", Polygon_Simplify, "[(Fill)i]"),
	GB_METHOD("Clean", "Polygon", Polygon_Clean, "[(Distance)f]"),

	GB_METHOD("Add", NULL, Polygon_Add, "(X)f(Y)f"),
	GB_METHOD("AddPoint", NULL, Polygon_AddPoint, "(Point)Point;"),
	GB_METHOD("Remove", NULL, Polygon_Remove, "(Index)i[(Count)i]"),

	GB_INTERFACE("_convert", &_convert_polygon),

	GB_END_DECLARE
};

GB_DESC ClipperDesc[] =
{
	GB_DECLARE_VIRTUAL("Clipper"),

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

	GB_STATIC_METHOD("Union", "Polygon[]", Clipper_Union, "(Polygons)Polygon[];[(Clip)Polygon[];(Fill)i]"),
	GB_STATIC_METHOD("Intersection", "Polygon[]", Clipper_Intersection, "(Polygons)Polygon[];[(Clip)Polygon[];(Fill)i]"),
	GB_STATIC_METHOD("Difference", "Polygon[]", Clipper_Difference, "(Polygons)Polygon[];[(Clip)Polygon[];(Fill)i]"),
	GB_STATIC_METHOD("ExclusiveOr", "Polygon[]", Clipper_ExclusiveOr, "(Polygons)Polygon[];[(Clip)Polygon[];(Fill)i]"),

	GB_END_DECLARE
};


