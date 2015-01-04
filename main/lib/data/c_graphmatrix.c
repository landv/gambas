/*
 * c_graphmatrix.c - Graph as adjacency matrix
 *
 * Copyright (C) 2014 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define __C_GRAPHMATRIX_C

#include <assert.h>

#include "gambas.h"
#include "c_graph.h"
#include "c_graphmatrix.h"

#define E_NOVERTEX	"Vertex does not exist"
#define E_NOEDGE	"Edge does not exist"
#define E_NOPUT		"No suitable _put method in the Matrix class"

typedef struct {
	int set : 1;
	double weight;
} EDGE;

/*
 * The VERT structure is a row in the adjacency matrix.
 */
typedef struct {
	EDGE *edges;
	GB_VARIANT_VALUE val;
	char *name;
} VERT;

/* Virtual object selector or part of enumeration state */
union virt {
	unsigned int vertex;
	struct {
		unsigned int src, dst;
	};
};

typedef struct {
	CGRAPH base;
	int directed : 1;
	int weighted : 1;
	GB_HASHTABLE names;
	VERT *matrix;
	/*
	 * NOTE: This field is used by the "true" virtual object portions of
	 * this class: GraphMatrix.Vertices[x] and GraphMatrix.Edges[x, y].
	 *
	 * In the enumerators (InEdges, OutEdges, Adjacent), we must use the
	 * "current" vertex/edge which is stored within ->base.
	 */
	union virt v;
	void *gsl_matrix; /* Cache gb.gsl Matrix object of this */
} CMATRIX;

#define THIS	((CMATRIX *) _object)

BEGIN_METHOD(Matrix_new, GB_BOOLEAN d; GB_BOOLEAN w)

	THIS->directed = VARGOPT(d, 0);
	THIS->weighted = VARGOPT(w, 0);
	THIS->v.vertex = -1;
	THIS->v.src = THIS->v.dst = -1;
	GB.HashTable.New(&THIS->names, GB_COMP_NOCASE);
	GB.NewArray(&THIS->matrix, sizeof(*THIS->matrix), 0);
	THIS->gsl_matrix = NULL;

END_METHOD

BEGIN_METHOD_VOID(Matrix_free)

	unsigned int i, count = GB.Count(THIS->matrix);

	GB.HashTable.Free(&THIS->names);
	for (i = 0; i < count; i++) {
		VERT *cur = &THIS->matrix[i];

		GB.FreeString(&cur->name);
		GB.FreeArray(&cur->edges);
		GB.StoreVariant(NULL, &cur->val);
	}
	GB.FreeArray(&THIS->matrix);
	GB.Unref(&THIS->gsl_matrix);

END_METHOD

static unsigned int get_vertex(CMATRIX *mat, const char *str, size_t len)
{
	uintptr_t vert; /* Be wide enough to get a void *! */

	if (GB.HashTable.Get(mat->names, str, len, (void **) &vert))
		return -1;
	assert(vert >= 0 && vert < GB.Count(mat->matrix));
	return (unsigned int) vert;
}

static unsigned int get_cur_vertex(CMATRIX *mat)
{
	return get_vertex(mat, mat->base.vertex,
		GB.StringLength(mat->base.vertex));
}

BEGIN_METHOD(Matrix_getVertex, GB_STRING vert)

	unsigned int vert = get_vertex(THIS, STRING(vert), LENGTH(vert));

	if (vert == -1) {
		GB.Error(E_NOVERTEX);
		return;
	}
	THIS->v.vertex = vert;
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_METHOD(Matrix_getEdge, GB_STRING src; GB_STRING dst)

	unsigned int src = get_vertex(THIS, STRING(src), LENGTH(src)),
		     dst = get_vertex(THIS, STRING(dst), LENGTH(dst));

	if (src == -1 || dst == -1) {
		GB.Error(E_NOVERTEX);
		return;
	}
	if (!THIS->matrix[src].edges[dst].set) {
		GB.Error(E_NOEDGE);
		return;
	}
	THIS->v.src = src; THIS->v.dst = dst;
	GB.ReturnSelf(THIS);

END_METHOD

struct enum_state {
	union virt v;
	GB_ARRAY e;
};

BEGIN_METHOD_VOID(Matrix_nextVertex)

	struct enum_state *state = GB.GetEnum();

	if (state->v.vertex == GB.Count(THIS->matrix)) {
		GB.StopEnum();
		return;
	}
	GB.ReturnString(THIS->matrix[state->v.vertex++].name);

END_METHOD

/* Return non-zero if no next edge was found */
static int next_edge(CMATRIX *mat, unsigned int *srcp, unsigned int *dstp)
{
	unsigned int src = *srcp, dst = *dstp;
	unsigned int count = GB.Count(mat->matrix);

	do {
		dst = (dst + 1) % count;
		if (!dst)
			src++;
		if (src >= count)
			return -1;
	} while (!mat->matrix[src].edges[dst].set);
	*srcp = src;
	*dstp = dst;
	return 0;
}

/**G
 * The same String[] object is used during the enumeration. If you want to
 * save a snapshot of it, use String[].Copy().
 *
 * Also, you should maybe not change the contents of this array. It may work
 * now but I can't guarantee it will in the future.
 */
BEGIN_METHOD_VOID(Matrix_nextEdge)

	struct enum_state *state = GB.GetEnum();
	unsigned int src = state->v.src, dst = state->v.dst;

	if (!state->e) {
		GB.Array.New(&state->e, GB_T_STRING, 2);
		GB.Ref(state->e);
		/* Try edge 0,0 which is a special case according to the
		 * logic below. */
		if (THIS->matrix[src].edges[dst].set)
			goto found;
	}
	/* End of enumeration? */
	if (next_edge(THIS, &src, &dst)) {
		GB.StopEnum();
		GB.Unref(&state->e);
		return;
	}
	state->v.src = src; state->v.dst = dst;
found:;
	GB_STRING str;

	str.type = GB_T_STRING;

	str.value.addr = THIS->matrix[src].name;
	str.value.start = 0;
	str.value.len = GB.StringLength(str.value.addr);
	GB.StoreString(&str, GB.Array.Get(state->e, 0));

	str.value.addr = THIS->matrix[dst].name;
	str.value.len = GB.StringLength(str.value.addr);
	GB.StoreString(&str, GB.Array.Get(state->e, 1));
	GB.ReturnObject(state->e);

END_METHOD

BEGIN_METHOD_VOID(Matrix_countVertices)

	GB.ReturnInteger(GB.Count(THIS->matrix));

END_METHOD

BEGIN_METHOD_VOID(Matrix_countEdges)

	unsigned int i, j, count = GB.Count(THIS->matrix), edges = 0;

	for (i = 0; i < count; i++)
		for (j = 0; j < count; j++)
			if (THIS->matrix[i].edges[j].set)
				edges++;
	GB.ReturnInteger(edges);

END_METHOD

static int next_edge_vertical(CMATRIX *mat, unsigned int *srcp,
					    unsigned int *dstp)
{
	unsigned int src = *srcp, dst = *dstp;
	unsigned int count = GB.Count(mat->matrix);

	do {
		src = (src + 1) % count;
		if (!src)
			dst++;
		if (dst >= count)
			return -1;
	} while (!mat->matrix[src].edges[dst].set);
	*srcp = src;
	*dstp = dst;
	return 0;
}

BEGIN_METHOD_VOID(Matrix_nextInEdge)

	struct enum_state *state = GB.GetEnum();
	unsigned int src = state->v.src, dst = state->v.dst;

	if (!state->e) {
		dst = state->v.dst = get_cur_vertex(THIS);
		GB.Array.New(&state->e, GB_T_STRING, 2);
		GB.Ref(state->e);
		if (THIS->matrix[src].edges[dst].set)
			goto found;
	}
	if (next_edge_vertical(THIS, &src, &dst) || dst != state->v.dst) {
		GB.StopEnum();
		GB.Unref(&state->e);
		return;
	}
	state->v.src = src;
found:;
	GB_STRING str;

	str.type = GB_T_STRING;

	str.value.addr = THIS->matrix[src].name;
	str.value.start = 0;
	str.value.len = GB.StringLength(str.value.addr);
	GB.StoreString(&str, GB.Array.Get(state->e, 0));

	str.value.addr = THIS->matrix[dst].name;
	str.value.len = GB.StringLength(str.value.addr);
	GB.StoreString(&str, GB.Array.Get(state->e, 1));
	GB.ReturnObject(state->e);

END_METHOD

BEGIN_METHOD_VOID(Matrix_nextOutEdge)

	struct enum_state *state = GB.GetEnum();
	unsigned int src = state->v.src, dst = state->v.dst;

	if (!state->e) {
		src = state->v.src = get_cur_vertex(THIS);
		GB.Array.New(&state->e, GB_T_STRING, 2);
		GB.Ref(state->e);
		if (THIS->matrix[src].edges[dst].set)
			goto found;
	}
	if (next_edge(THIS, &src, &dst) || src != state->v.src) {
		GB.StopEnum();
		GB.Unref(&state->e);
		return;
	}
	state->v.dst = dst;
found:;
	GB_STRING str;

	str.type = GB_T_STRING;

	str.value.addr = THIS->matrix[src].name;
	str.value.start = 0;
	str.value.len = GB.StringLength(str.value.addr);
	GB.StoreString(&str, GB.Array.Get(state->e, 0));

	str.value.addr = THIS->matrix[dst].name;
	str.value.len = GB.StringLength(str.value.addr);
	GB.StoreString(&str, GB.Array.Get(state->e, 1));
	GB.ReturnObject(state->e);

END_METHOD

BEGIN_METHOD_VOID(Matrix_nextAdjacent)

	struct enum_state *state = GB.GetEnum();
	unsigned int src = state->v.src, dst = state->v.dst;

	if (!state->e) {
		src = state->v.src = get_cur_vertex(THIS);
		state->e = (void *) 1;
		if (THIS->matrix[src].edges[dst].set)
			goto found;
	}
	if (next_edge(THIS, &src, &dst) || src != state->v.src) {
		GB.StopEnum();
		return;
	}
	state->v.dst = dst;
found:
	GB.ReturnString(THIS->matrix[dst].name);

END_METHOD

/* TODO: This is used in Add() and Remove() to completely delete the gb.gsl
 * matrix. We could also enlarge/shrink it accordingly... */
static void invalidate_gsl_matrix(CMATRIX *mat)
{
	GB.Unref(&mat->gsl_matrix);
	mat->gsl_matrix = NULL;
}

static void update_gsl_matrix(CMATRIX *mat, unsigned i, unsigned j)
{
	GB_FUNCTION put;

	if (!mat->gsl_matrix)
		return;
	if (GB.GetFunction(&put, mat->gsl_matrix, "_put", "vii", NULL)) {
		GB.Error(E_NOPUT);
		return;
	}
	GB.Push(3, GB_T_INTEGER, !!mat->matrix[i].edges[j].set,
		   GB_T_INTEGER, i,
		   GB_T_INTEGER, j);
	GB.Call(&put, 3, 0);
}

BEGIN_METHOD(Matrix_Add, GB_STRING name)

	unsigned int vert = get_vertex(THIS, STRING(name), LENGTH(name)), i;
	VERT *new;

	if (vert != -1)
		goto end;

	vert = GB.Count(THIS->matrix);
	new = GB.Add(&THIS->matrix);
	/* Add edge buffers to all other vertices */
	for (i = 0; i < vert; i++) {
		EDGE *e = GB.Add(&THIS->matrix[i].edges);

		e->set = 0;
		e->weight = 0;
	}
	GB.NewArray(&new->edges, sizeof(*new->edges), vert + 1);
	/* No outgoing edges */
	memset(new->edges, 0, (vert + 1) * sizeof(*new->edges));

	new->val.type = GB_T_NULL;
	GB.StoreVariant(NULL, &new->val);

	new->name = GB.NewString(STRING(name), LENGTH(name));

	GB.HashTable.Add(THIS->names, STRING(name), LENGTH(name),
			 (void *) (intptr_t) vert);

	invalidate_gsl_matrix(THIS);

end:
	THIS->v.vertex = vert;
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_METHOD(Matrix_Remove, GB_STRING vert)

	unsigned int vert = get_vertex(THIS, STRING(vert), LENGTH(vert));
	unsigned int count = GB.Count(THIS->matrix), i;

	if (vert == -1) {
		GB.Error(E_NOVERTEX);
		return;
	}
	for (i = 0; i < count; i++) {
		if (i == vert)
			continue;
		GB.Remove(&THIS->matrix[i].edges, i, 1);
	}
	GB.FreeArray(&THIS->matrix[vert].edges);
	GB.StoreVariant(NULL, &THIS->matrix[vert].val);
	GB.FreeString(&THIS->matrix[vert].name);
	GB.Remove(&THIS->matrix, vert, 1);

	GB.HashTable.Remove(THIS->names, STRING(vert), LENGTH(vert));

	invalidate_gsl_matrix(THIS);

END_METHOD

BEGIN_METHOD(Matrix_Connect, GB_STRING src; GB_STRING dst; GB_FLOAT w)

	unsigned int src = get_vertex(THIS, STRING(src), LENGTH(src)),
		     dst = get_vertex(THIS, STRING(dst), LENGTH(dst));
	float w = VARGOPT(w, 1);

	if (src == -1 || dst == -1) {
		GB.Error(E_NOVERTEX);
		return;
	}
	THIS->matrix[src].edges[dst].set = 1;
	THIS->matrix[src].edges[dst].weight = w;
	THIS->v.src = src; THIS->v.dst = dst;
	update_gsl_matrix(THIS, src, dst);
	/* Duplicate if the graph is undirected */
	if (!THIS->directed && src != dst) {
		THIS->matrix[dst].edges[src].set = 1;
		THIS->matrix[dst].edges[src].weight = w;
		update_gsl_matrix(THIS, dst, src);
	}

	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_METHOD(Matrix_Disconnect, GB_STRING src; GB_STRING dst)

	unsigned int src = get_vertex(THIS, STRING(src), LENGTH(src)),
		     dst = get_vertex(THIS, STRING(dst), LENGTH(dst));

	if (src == -1 || dst == -1) {
		GB.Error(E_NOVERTEX);
		return;
	}
	THIS->matrix[src].edges[dst].set = 0;
	update_gsl_matrix(THIS, src, dst);
	if (!THIS->directed && src != dst) {
		THIS->matrix[dst].edges[src].set = 0;
		update_gsl_matrix(THIS, dst, src);
	}

END_METHOD

BEGIN_PROPERTY(Matrix_Matrix)

	unsigned int count = GB.Count(THIS->matrix), i, j;
	void *obj;
	GB_FUNCTION put;

	if (THIS->gsl_matrix) {
		GB.ReturnObject(THIS->gsl_matrix);
		return;
	}

	if (GB.Component.Load("gb.gsl")) {
		GB.Error("gb.gsl could not be found");
		return;
	}

	GB.Push(3, GB_T_INTEGER, count,
		   GB_T_INTEGER, count,
		   GB_T_BOOLEAN, 0);
	obj = GB.New(GB.FindClass("Matrix"), NULL, (void *) (intptr_t) 3);
	if (GB.GetFunction(&put, obj, "_put", "vii", NULL)) {
		GB.Error(E_NOPUT);
		return;
	}
	/* TODO: Direct access possible? */
	for (i = 0; i < count; i++) {
		for (j = 0; j < count; j++) {
			GB.Push(3, GB_T_INTEGER,
					!!THIS->matrix[i].edges[j].set,
				   GB_T_INTEGER, i,
				   GB_T_INTEGER, j);
			GB.Call(&put, 3, 0);
		}
	}
	THIS->gsl_matrix = obj;
	GB.Ref(obj);
	GB.ReturnObject(obj);

END_PROPERTY

GB_DESC CGraphMatrix[] = {
	GB_DECLARE("GraphMatrix", sizeof(CMATRIX)),
	GB_INHERITS("Graph"),

	GB_METHOD("_new", NULL, Matrix_new, "[(Directed)b(Weighted)b]"),
	GB_METHOD("_free", NULL, Matrix_free, NULL),

	GB_METHOD("_getVertex", ".Matrix.Vertex", Matrix_getVertex, "(Vertex)s"),
	GB_METHOD("_getEdge", ".Matrix.Edge", Matrix_getEdge, "(Src)s(Dst)s"),

	GB_METHOD("_nextVertex", "s", Matrix_nextVertex, NULL),
	GB_METHOD("_nextEdge", "String[]", Matrix_nextEdge, NULL),

	GB_METHOD("_countVertices", "i", Matrix_countVertices, NULL),
	GB_METHOD("_countEdges", "i", Matrix_countEdges, NULL),

	GB_METHOD("_nextInEdge", "String[]", Matrix_nextInEdge, NULL),
	GB_METHOD("_nextOutEdge", "String[]", Matrix_nextOutEdge, NULL),
	GB_METHOD("_nextAdjacent", "s", Matrix_nextAdjacent, NULL),

	GB_METHOD("Add", ".Matrix.Vertex", Matrix_Add, "(Name)s"),
	GB_METHOD("Remove", NULL, Matrix_Remove, "(Vertex)s"),
	GB_METHOD("Connect", ".Matrix.Edge", Matrix_Connect, "(Src)s(Dst)s[(Weight)f]"),
	GB_METHOD("Disconnect", NULL, Matrix_Disconnect, "(Src)s(Dst)s"),

	GB_PROPERTY_SELF("Vertices", ".Matrix.Vertices"),
	GB_PROPERTY_SELF("Edges", ".Matrix.Edges"),

	/*
	 * Require gb.gsl.
	 */
	//GB_STATIC_METHOD("FromMatrix", "GraphMatrix", Matrix_FromMatrix, "(Matrix)Matrix;"),
	GB_PROPERTY_READ("Matrix", "Matrix", Matrix_Matrix),

	GB_END_DECLARE
};

BEGIN_METHOD(MatrixVertices_get, GB_STRING vert)

	THIS->v.vertex = get_vertex(THIS, STRING(vert), LENGTH(vert));
	GB.ReturnSelf(THIS);

END_METHOD

GB_DESC CMatrixVertices[] = {
	GB_DECLARE_VIRTUAL(".Matrix.Vertices"),
	GB_INHERITS(".Graph.Vertices"),

	GB_METHOD("_get", ".Matrix.Vertex", MatrixVertices_get, "(Vertex)s"),

	GB_END_DECLARE
};

BEGIN_METHOD(MatrixEdges_get, GB_STRING src; GB_STRING dst)

	THIS->v.src = get_vertex(THIS, STRING(src), LENGTH(src));
	THIS->v.dst = get_vertex(THIS, STRING(dst), LENGTH(dst));
	GB.ReturnSelf(THIS);

END_METHOD

GB_DESC CMatrixEdges[] = {
	GB_DECLARE_VIRTUAL(".Matrix.Edges"),
	GB_INHERITS(".Graph.Edges"),

	GB_METHOD("_get", ".Matrix.Edge", MatrixEdges_get, "(Src)s(Dst)s"),

	GB_END_DECLARE
};

BEGIN_PROPERTY(MatrixVertex_InDegree)

	unsigned int i, count = GB.Count(THIS->matrix), deg = 0;

	for (i = 0; i < count; i++)
		if (THIS->matrix[i].edges[THIS->v.vertex].set)
			deg++;
	GB.ReturnInteger(deg);

END_PROPERTY

BEGIN_PROPERTY(MatrixVertex_OutDegree)

	unsigned int j, count = GB.Count(THIS->matrix), deg = 0;

	for (j = 0; j < count; j++)
		if (THIS->matrix[THIS->v.vertex].edges[j].set)
			deg++;
	GB.ReturnInteger(deg);

END_PROPERTY

BEGIN_PROPERTY(MatrixVertex_Name)

	char *name = THIS->matrix[THIS->v.vertex].name;

	if (READ_PROPERTY) {
		GB.ReturnString(name);
		return;
	}
	GB.HashTable.Remove(THIS->names, name, GB.StringLength(name));
	GB.FreeString(&THIS->matrix[THIS->v.vertex].name);
	THIS->matrix[THIS->v.vertex].name =
		GB.NewString(PSTRING(), PLENGTH());
	GB.HashTable.Add(THIS->names, PSTRING(), PLENGTH(),
			 (void *) (intptr_t) THIS->v.vertex);

END_PROPERTY

BEGIN_PROPERTY(MatrixVertex_Value)

	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->matrix[THIS->v.vertex].val);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->matrix[THIS->v.vertex].val);

END_METHOD

GB_DESC CMatrixVertex[] = {
	GB_DECLARE_VIRTUAL(".Matrix.Vertex"),
	GB_INHERITS(".Graph.Vertex"),

	GB_PROPERTY_READ("InDegree", "i", MatrixVertex_InDegree),
	GB_PROPERTY_READ("OutDegree", "i", MatrixVertex_OutDegree),
	GB_PROPERTY("Name", "s", MatrixVertex_Name),
	GB_PROPERTY("Value", "v", MatrixVertex_Value),

	GB_END_DECLARE
};

BEGIN_PROPERTY(MatrixEdge_Src)

	int src = THIS->v.src;

	GB.ReturnString(THIS->matrix[src].name);

END_PROPERTY

BEGIN_PROPERTY(MatrixEdge_Dst)

	int dst = THIS->v.dst;

	GB.ReturnString(THIS->matrix[dst].name);

END_PROPERTY

BEGIN_PROPERTY(MatrixEdge_Weight)

	int src = THIS->v.src, dst = THIS->v.dst;

	if (READ_PROPERTY) {
		GB.ReturnFloat(THIS->matrix[src].edges[dst].weight);
		return;
	}
	THIS->matrix[src].edges[dst].weight = VPROP(GB_FLOAT);
	if (!THIS->directed && src != dst)
		THIS->matrix[dst].edges[src].weight = VPROP(GB_FLOAT);

END_PROPERTY

GB_DESC CMatrixEdge[] = {
	GB_DECLARE_VIRTUAL(".Matrix.Edge"),
	GB_INHERITS(".Graph.Edge"),

	GB_PROPERTY_READ("Src", "s", MatrixEdge_Src),
	GB_PROPERTY_READ("Dst", "s", MatrixEdge_Dst),
	GB_PROPERTY("Weight", "f", MatrixEdge_Weight),

	GB_PROPERTY_READ("Source", "s", MatrixEdge_Src),
	GB_PROPERTY_READ("Destination", "s", MatrixEdge_Dst),

	GB_END_DECLARE
};
