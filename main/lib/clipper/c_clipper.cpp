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
#include "clipper.hpp"

using namespace ClipperLib;

#define SCALE 1000000.0

static IntPoint to_point(GEOM_POINTF *point)
{
	return IntPoint(point->x * SCALE + 0.5, point->y * SCALE + 0.5);
}

static GEOM_POINTF *from_point(IntPoint p)
{
	return GEOM.CreatePointF((double)p.X / SCALE, (double)p.Y / SCALE);
}

/*static bool is_closed(GB_ARRAY p)
{
	if (!p)
		return false;

	int count = GB.Array.Count(p);
	if (count <= 1)
		return false;

	GEOM_POINTF **ap = (GEOM_POINTF **)GB.Array.Get(p, 0);
	return ap[0]->x == ap[count - 1]->x && ap[0]->y == ap[count - 1]->y;
}*/

static bool to_polygons(Polygons &polygons, GB_ARRAY array)
{
	int count;
	GB_ARRAY a;
	int i, j;
	GEOM_POINTF **ap;
	GEOM_POINTF *pp;

	if (GB.CheckObject(array))
		return true;

	count = GB.Array.Count(array);
	if (count == 0)
		return false;

	polygons.resize(count);

	for(i = 0; i < count; i++)
	{
		a = *(GB_ARRAY *)GB.Array.Get(array, i);
		if (!a)
			continue;
		ap = (GEOM_POINTF **)GB.Array.Get(a, 0);

		for (j = 0; j < GB.Array.Count(a); j++)
		{
			pp = ap[j];
			if (!pp)
				continue;
			//fprintf(stderr, "<< %d %d: %g %g\n", i, j, pp->x, pp->y);
			polygons[i].push_back(to_point(pp));
		}
	}

	return false;
}

static GB_ARRAY from_polygons(Polygons &polygons, GB_ARRAY array)
{
	GB_ARRAY a;
	GB_ARRAY p;
	uint i, j, n;
	GEOM_POINTF *pp;

	GB.Array.New(&a, GB.FindClass("PointF[]"), polygons.size());
	for (i = 0; i < polygons.size(); i++)
	{
		n = polygons[i].size();
		GB.Array.New(&p, GB.FindClass("PointF"), n);

		for (j = 0; j < n; j++)
		{
			pp = from_point(polygons[i][j]);
			//fprintf(stderr, ">> %d %d: %g %g\n", i, j, pp->x, pp->y);
			*(GEOM_POINTF **)GB.Array.Get(p, j) = pp;
			GB.Ref(pp);
		}

		// Close polygon
		pp = from_point(polygons[i][0]);
		//fprintf(stderr, ">> %d %d: %g %g\n", i, j, pp->x, pp->y);
		*(GEOM_POINTF **)GB.Array.Add(p) = pp;
		GB.Ref(pp);

		*(GB_ARRAY *)GB.Array.Get(a, i) = p;
		GB.Ref(p);
	}

	return a;
}

BEGIN_METHOD(Clipper_OffsetPolygons, GB_OBJECT polygons; GB_FLOAT delta; GB_INTEGER join; GB_FLOAT limit; GB_BOOLEAN do_not_fix)

	Polygons polygons;
	Polygons result;

	if (to_polygons(polygons, VARG(polygons)))
		return;

	SimplifyPolygons(polygons, result, pftNonZero);
	polygons = result;
	OffsetPolygons(polygons, result, VARG(delta) * SCALE, (JoinType)VARGOPT(join, jtSquare), VARGOPT(limit, 0.0), !VARGOPT(do_not_fix, false));

	GB.ReturnObject(from_polygons(result, VARG(polygons)));

END_METHOD

GB_DESC ClipperDesc[] =
{
	GB_DECLARE_VIRTUAL("Clipper"),

	//void OffsetPolygons(const Polygons &in_polys, Polygons &out_polys, double delta, JoinType jointype = jtSquare, double limit = 0.0, bool autoFix = true);

	GB_CONSTANT("JoinMiter", "i", jtMiter),
	GB_CONSTANT("JoinSquare", "i", jtSquare),
	GB_CONSTANT("JoinRound", "i", jtRound),

	GB_STATIC_METHOD("OffsetPolygons", "PointF[][]", Clipper_OffsetPolygons, "(Polygons)PointF[][];(Delta)f[(Join)i(Limit)f(DoNotFix)b]"),

	GB_END_DECLARE
};


