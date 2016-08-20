/*
 * c_graph.c - (Un)Directed, (un)weighted Graph interface
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

#define __C_GRAPH_C

#include <assert.h>

#include "gambas.h"
#include "c_graph.h"

static GB_HASHTABLE interfaces;

BEGIN_METHOD_VOID(Graph_init)

	GB.HashTable.New(&interfaces, GB_COMP_BINARY);

END_METHOD

static void enum_func(void *x)
{
	GB.Free(&x);
}

BEGIN_METHOD_VOID(Graph_exit)

	GB.HashTable.Enum(interfaces, (GB_HASHTABLE_ENUM_FUNC) enum_func);
	GB.HashTable.Free(&interfaces);

END_METHOD

BEGIN_METHOD_VOID(Graph_NoMethod)

	GB.Error("This method is not implemented.");

END_METHOD

BEGIN_PROPERTY(Graph_NoProperty)

	GB.Error("This property is not implemented.");

END_PROPERTY

BEGIN_METHOD(Graph_call, GB_BOOLEAN d; GB_BOOLEAN w)

	GB_CLASS GraphMatrixClass;

	GB.Push(2, GB_T_BOOLEAN, VARGOPT(d, 0),
		   GB_T_BOOLEAN, VARGOPT(w, 0));
	GraphMatrixClass = GB.FindClass("GraphMatrix");
	GB.ReturnObject(GB.New(GraphMatrixClass, NULL, (void *) (intptr_t) 2));

END_METHOD

#define get_func(func, arg, ret, need) 					\
do {									\
	GB_FUNCTION f;							\
									\
	err = #func;							\
	if (GB.GetFunction(&f, _object, #func, arg, ret)) {		\
		if (need)						\
			goto error;					\
		else /* Clear error */					\
			GB.Error(NULL);					\
		desc->func = NULL;					\
	} else {							\
		desc->func = f.desc;					\
	}								\
} while(0)

GRAPH_DESC *get_desc(void *_object)
{
	GRAPH_DESC *desc;
	const char *err;

	GB.Alloc((void **) &desc, sizeof(*desc));
	get_func(_getVertex,      "s",  NULL,       0);
	get_func(_getEdge,        "ss", NULL,       0);
	get_func(_nextVertex,     NULL, "s",        0);
	get_func(_nextEdge,       NULL, "String[]", 0);
	get_func(_countVertices,  NULL, "i",        0);
	get_func(_countEdges,     NULL, "i",        0);
	get_func(_nextInEdge,     NULL, "String[]", 0);
	get_func(_nextOutEdge,    NULL, "String[]", 0);
	get_func(_nextAdjacent,   NULL, "s",        0);
	get_func(_vertexProperty, ".",  "b",        0);
	get_func(_edgeProperty,   ".",  "b",        0);
	get_func(_vertexUnknown,  ".",  "v",        0);
	get_func(_edgeUnknown,    ".",  "v",        0);
	return desc;

error:
	GB.Error("Method \"&1\" not found", err);
	GB.Free((void **) &desc);
	return NULL;
}

#define THIS	((CGRAPH *) _object)

BEGIN_METHOD_VOID(Graph_new)

	GRAPH_DESC *desc;
	char *name = GB.GetClassName(THIS);
	size_t len = GB.StringLength(name);

	if (GB.HashTable.Get(interfaces, name, len, (void **) &desc)) {
		desc = get_desc(THIS);
		GB.HashTable.Add(interfaces, name, len, desc);
	}
	THIS->desc = desc;
	THIS->vertex = NULL;
	GB.Array.New(&THIS->edge, GB_T_STRING, 2);
	THIS->tag.type = GB_T_NULL;

END_METHOD

BEGIN_METHOD_VOID(Graph_free)

	GB.StoreVariant(NULL, &THIS->tag);
	GB.Unref(&THIS->edge);
	GB.FreeString(&THIS->vertex);

END_METHOD

BEGIN_PROPERTY(Graph_Tag)

	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->tag);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->tag);

END_PROPERTY

#define SET_VERTEX()						\
do {								\
	GB.FreeString(&THIS->vertex);				\
	THIS->vertex = GB.NewString(STRING(vert), LENGTH(vert));\
} while (0)

#define SET_VERTEX_P()						\
do {								\
	GB.FreeString(&THIS->vertex);				\
	THIS->vertex = GB.NewString(PSTRING(), PLENGTH());	\
} while (0)

#define SET_EDGE()						\
do {								\
	GB.StoreString(ARG(src), GB.Array.Get(THIS->edge, 0));	\
	GB.StoreString(ARG(dst), GB.Array.Get(THIS->edge, 1));	\
} while (0)

#define SET_EDGE_P()						\
do {								\
	GB.StoreObject(VPROP(GB_OBJECT), &THIS->edge);		\
} while (0)

BEGIN_PROPERTY(Graph_Vertex)

	if (READ_PROPERTY) {
		GB.ReturnString(THIS->vertex);
		return;
	}
	SET_VERTEX_P();

END_PROPERTY

BEGIN_PROPERTY(Graph_Edge)

	if (READ_PROPERTY) {
		GB.ReturnObject(THIS->edge);
		return;
	}
	SET_EDGE_P();

END_PROPERTY

GB_DESC CGraph[] = {
	GB_DECLARE("Graph", sizeof(CGRAPH)),
	GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("_init", NULL, Graph_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, Graph_exit, NULL),

	GB_STATIC_METHOD("_call", "Graph", Graph_call, "[(Directed)b(Weighted)b]"),
	GB_METHOD("_new", NULL, Graph_new, NULL),
	GB_METHOD("_free", NULL, Graph_free, NULL),

	/* Graph */
	GB_METHOD("_getVertex", ".Graph.Vertex", Graph_NoMethod, "(Vertex)s"),
	GB_METHOD("_getEdge", ".Graph.Edge", Graph_NoMethod, "(Src)s(Dst)s"),

	GB_METHOD("_nextVertex", "s", Graph_NoMethod, NULL),
	GB_METHOD("_nextEdge", "String[]", Graph_NoMethod, NULL),

	GB_METHOD("_countVertices", "i", Graph_NoMethod, NULL),
	GB_METHOD("_countEdges", "i", Graph_NoMethod, NULL),

	/* Vertex */
	GB_METHOD("_nextInEdge", "String[]", Graph_NoMethod, NULL),
	GB_METHOD("_nextOutEdge", "String[]", Graph_NoMethod, NULL),

	/* Public */
	GB_METHOD("Add", ".Graph.Vertex", Graph_NoMethod, "(Name)s"),
	GB_METHOD("Remove", NULL, Graph_NoMethod, "(Vertex)s"),
	GB_METHOD("Connect", ".Graph.Edge", Graph_NoMethod, "(Src)s(Dst)s"),
	GB_METHOD("Disconnect", NULL, Graph_NoMethod, "(Src)s(Dst)s"),

	GB_PROPERTY("Tag", "v", Graph_Tag),

	GB_PROPERTY("_Vertex", "s", Graph_Vertex),
	GB_PROPERTY("_Edge", "String[]", Graph_Edge),

	GB_PROPERTY_SELF("Vertices", ".Graph.Vertices"),
	GB_PROPERTY_SELF("Edges", ".Graph.Edges"),

	GB_PROPERTY_SELF("InEdges", ".Graph.InEdges"),
	GB_PROPERTY_SELF("OutEdges", ".Graph.OutEdges"),
	GB_PROPERTY_SELF("Adjacent", ".Graph.Adjacent"),

	GB_END_DECLARE
};

/*
 * Call another method/property of this class and fail if not found.
 */
#define CALL_GRAPH(func, narg)					\
do {								\
	GB_FUNCTION f;						\
								\
	f.desc = THIS->desc->func;				\
	f.object = THIS;					\
	GB.Call(&f, narg, 0);					\
} while (0)

#define CALL_GRAPH_VAL(func, narg, arg, ret, retval)		\
do {								\
	GB_FUNCTION f;						\
								\
	if (GB.GetFunction(&f, _object, func, arg, ret)) {	\
		GB.Error("Method " func " not found");		\
		return;						\
	}							\
	retval = GB.Call(&f, narg, 0);				\
} while (0)

/*
 * Try to call another method/property of this class. If found, return
 * afterwards. If not, continue.
 */
#define TRY_CALL_GRAPH(func, narg)				\
do {								\
	if (THIS->desc->func) {					\
		CALL_GRAPH(func, narg);				\
		return;						\
	}							\
} while (0)

BEGIN_METHOD_VOID(GraphVertices_next)

	CALL_GRAPH(_nextVertex, 0);

END_METHOD

BEGIN_METHOD(GraphVertices_get, GB_STRING vert)

	SET_VERTEX();
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_PROPERTY(GraphVertices_Count)

	CALL_GRAPH(_countVertices, 0);
	/* Default: enumerate the vertices, counting. Then use
	 * TRY_CALL_GRAPH. */

END_PROPERTY

GB_DESC CGraphVertices[] = {
	GB_DECLARE_VIRTUAL(".Graph.Vertices"),

	GB_METHOD("_next", "s", GraphVertices_next, NULL),
	GB_METHOD("_get", ".Graph.Vertex", GraphVertices_get, "(Vertex)s"),

	GB_PROPERTY_READ("Count", "i", GraphVertices_Count),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(GraphEdges_next)

	CALL_GRAPH(_nextEdge, 0);

END_METHOD

BEGIN_METHOD(GraphEdges_get, GB_STRING src; GB_STRING dst)

	SET_EDGE();
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_PROPERTY(GraphEdges_Count)

	CALL_GRAPH(_countEdges, 0);
	/* Default: enumerate the edges, counting. Then use
	 * TRY_CALL_GRAPH. */

END_PROPERTY

GB_DESC CGraphEdges[] = {
	GB_DECLARE_VIRTUAL(".Graph.Edges"),

	GB_METHOD("_next", "String[]", GraphEdges_next, NULL),
	GB_METHOD("_get", ".Graph.Edge", GraphEdges_get, "(Src)s(Dst)s"),
	GB_METHOD("Exist", "b", Graph_NoMethod, "(Src)s(Dst)s"),

	GB_PROPERTY_READ("Count", "i", GraphEdges_Count),

	GB_END_DECLARE
};

BEGIN_METHOD(GraphInEdges_get, GB_STRING vert)

	SET_VERTEX();
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_METHOD_VOID(GraphInEdges_next)

	CALL_GRAPH(_nextInEdge, 0);

END_METHOD

GB_DESC CGraphInEdges[] = {
	GB_DECLARE_VIRTUAL(".Graph.InEdges"),

	GB_METHOD("_get", ".Graph.InEdges", GraphInEdges_get, "(Vertex)s"),
	GB_METHOD("_next", "String[]", GraphInEdges_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD(GraphOutEdges_get, GB_STRING vert)

	SET_VERTEX();
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_METHOD_VOID(GraphOutEdges_next)

	CALL_GRAPH(_nextOutEdge, 0);

END_METHOD

GB_DESC CGraphOutEdges[] = {
	GB_DECLARE_VIRTUAL(".Graph.OutEdges"),

	GB_METHOD("_get", ".Graph.OutEdges", GraphOutEdges_get, "(Vertex)s"),
	GB_METHOD("_next", "String[]", GraphOutEdges_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD(GraphAdjacent_get, GB_STRING vert)

	SET_VERTEX();
	GB.ReturnSelf(THIS);

END_METHOD

BEGIN_METHOD_VOID(GraphAdjacent_next)

	CALL_GRAPH(_nextAdjacent, 0);

END_METHOD

GB_DESC CGraphAdjacent[] = {
	GB_DECLARE_VIRTUAL(".Graph.Adjacent"),

	GB_METHOD("_get", ".Graph.Adjacent", GraphAdjacent_get, "(Vertex)s"),
	GB_METHOD("_next", "s", GraphAdjacent_next, NULL),

	GB_END_DECLARE
};

BEGIN_PROPERTY(GraphVertex_InDegree)

	/* Call ??? */
	/* Default: enumerate */
	assert(0);

END_PROPERTY

BEGIN_PROPERTY(GraphVertex_OutDegree)

	/* Call ??? */
	/* Default: enumerate */
	assert(0);

END_PROPERTY

BEGIN_METHOD_VOID(GraphVertex_property)

	CALL_GRAPH(_vertexProperty, GB.NParam());

END_METHOD

BEGIN_METHOD_VOID(GraphVertex_unknown)

	CALL_GRAPH(_vertexUnknown, GB.NParam());

END_METHOD

GB_DESC CGraphVertex[] = {
	GB_DECLARE_VIRTUAL(".Graph.Vertex"),

	GB_PROPERTY_READ("InDegree", "i", GraphVertex_InDegree),
	GB_PROPERTY_READ("OutDegree", "i", GraphVertex_OutDegree),

	GB_METHOD("_property", "b", GraphVertex_property, "."),
	GB_METHOD("_unknown", "v", GraphVertex_unknown, "."),

	GB_END_DECLARE
};

BEGIN_PROPERTY(GraphEdge_Src)

	GB.ReturnString(GB.Array.Get(THIS->edge, 0));

END_PROPERTY

BEGIN_PROPERTY(GraphEdge_Dst)

	GB.ReturnString(GB.Array.Get(THIS->edge, 1));

END_PROPERTY

BEGIN_METHOD_VOID(GraphEdge_property)

	CALL_GRAPH(_edgeProperty, GB.NParam());

END_METHOD

BEGIN_METHOD_VOID(GraphEdge_unknown)

	CALL_GRAPH(_edgeUnknown, GB.NParam());

END_METHOD

GB_DESC CGraphEdge[] = {
	GB_DECLARE_VIRTUAL(".Graph.Edge"),

	GB_PROPERTY_READ("Src", "s", GraphEdge_Src),
	GB_PROPERTY_READ("Dst", "s", GraphEdge_Dst),

	GB_PROPERTY_READ("Source", "s", GraphEdge_Src),
	GB_PROPERTY_READ("Destination", "s", GraphEdge_Dst),
	GB_PROPERTY("Weight", "f", Graph_NoProperty),

	GB_METHOD("_property", "b", GraphEdge_property, "."),
	GB_METHOD("_unknown", "v", GraphEdge_unknown, "."),

	GB_END_DECLARE
};
