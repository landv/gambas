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

#include "gambas.h"

#include "c_graph.h"

typedef struct {
	GB_BASE ob;
	GB_VARIANT_VALUE tag;
} CGRAPH;

BEGIN_METHOD_VOID(Graph_NoMethod)

	GB.Error("This method is not implemented.");

END_METHOD

BEGIN_PROPERTY(Graph_NoProperty)

	GB.Error("This property is not implemented.");

END_PROPERTY

BEGIN_METHOD(Graph_call, GB_INTEGER count; GB_BOOLEAN d; GB_BOOLEAN w)

	//GB.GetClass("GraphMatrix");

END_METHOD

#define THIS	((CGRAPH *) _object)

BEGIN_METHOD_VOID(Graph_free)

	GB.StoreVariant(NULL, &THIS->tag);

END_METHOD

BEGIN_PROPERTY(Graph_Tag)

	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->tag);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->tag);

END_PROPERTY

GB_DESC CGraph[] = {
	GB_DECLARE("Graph", sizeof(CGRAPH)),
	GB_NOT_CREATABLE(),

	GB_STATIC_METHOD("_call", "Graph", Graph_call, "[(Count)i(Directed)b(Weighted)b]"),
	GB_METHOD("_free", NULL, Graph_free, NULL),

	GB_METHOD("_getVertex", "_Graph_Vertex", Graph_NoMethod, "(Vertex)s"),
	GB_METHOD("_getEdge", "Edge", Graph_NoMethod, "(Src)s(Dst)s"),

	GB_METHOD("_nextVertex", "s", Graph_NoMethod, NULL),
	GB_METHOD("_nextEdge", "Edge", Graph_NoMethod, NULL),

	GB_PROPERTY_SELF("Vertices", ".Graph.Vertices"),
	GB_PROPERTY_SELF("Edges", ".Graph.Edges"),
	GB_PROPERTY("Tag", "v", Graph_Tag),

	/* Synonyms... */
#if 0
	GB_METHOD("_get", ".Graph.Vertex", , "(Vertex)i"),
	GB_METHOD("_put", NULL, , "(Value)v(Vertex)i"),
	GB_METHOD("_next", "i", , NULL),
	GB_METHOD("Add", "i", , "(Value)v"),
	GB_METHOD("Remove", NULL, , "(Vertex)i"),
	GB_METHOD("Connect", "i", , "(Src)i(Dest)i[(Weight)f]"),
	GB_METHOD("Disconnect", NULL, , "(Edge)i"),
#endif

	GB_END_DECLARE
};

/*
 * Call another method/property of this class and fail if not found.
 */
#define CALL_GRAPH(func, narg, arg, ret)			\
do {								\
	GB_FUNCTION f;						\
								\
	if (GB.GetFunction(&f, _object, func, arg, ret)) {	\
		GB.Error("Method " func " not found");		\
		return;						\
	}							\
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
#define TRY_CALL_GRAPH(func, narg, arg, ret)			\
do {								\
	GB_FUNCTION f;						\
								\
	if (!GB.GetFunction(&f, _object, func, arg, ret)) {	\
		GB.Call(&f, narg, 0);				\
		return;						\
	}							\
} while (0)

BEGIN_METHOD_VOID(GraphVertices_next)

	CALL_GRAPH("_nextVertex", 0, NULL, "s");

END_METHOD

BEGIN_METHOD(GraphVertices_get, GB_STRING vert)

	GB.Push(1, GB_T_STRING, STRING(vert), LENGTH(vert));
	CALL_GRAPH("_getVertex", 1, "s", "_Graph_Vertex");

END_METHOD

BEGIN_PROPERTY(GraphVertices_Count)

	TRY_CALL_GRAPH("_countVertices", 0, NULL, "i");

	/* Default: enumerate the vertices, counting */
	/* BeginEnum? */

END_PROPERTY

GB_DESC CGraphVertices[] = {
	GB_DECLARE(".Graph.Vertices", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "s", GraphVertices_next, NULL),
	GB_METHOD("_get", "_Graph_Vertex", GraphVertices_get, "(Vertex)s"),

	GB_PROPERTY_READ("Count", "i", GraphVertices_Count),

	GB_END_DECLARE
};

GB_DESC CGraphVertex[] = {
	GB_DECLARE("_Graph_Vertex", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_nextInEdge", "Edge", Graph_NoMethod, NULL),
	GB_METHOD("_nextOutEdge", "Edge", Graph_NoMethod, NULL),
	GB_METHOD("_nextAdjacent", "s", Graph_NoMethod, NULL),

	GB_PROPERTY_SELF("InEdges", ".Vertex.InEdges"),
	GB_PROPERTY_SELF("OutEdges", ".Vertex.OutEdges"),
	GB_PROPERTY_SELF("Adjacent", ".Vertex.Adjacent"),
	//GB_PROPERTY_READ("InDegree", "i", GraphVertex_InDegree),
	//GB_PROPERTY_READ("OutDegree", "i", GraphVertex_OutDegree),
	GB_PROPERTY("Value", "v", Graph_NoProperty),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(VertexInEdges_next)

	CALL_GRAPH("_nextInEdge", 0, NULL, "Edge");

END_METHOD

GB_DESC CVertexInEdges[] = {
	GB_DECLARE(".Vertex.InEdges", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Edge", VertexInEdges_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(VertexOutEdges_next)

	CALL_GRAPH("_nextOutEdge", 0, NULL, "Edge");

END_METHOD

GB_DESC CVertexOutEdges[] = {
	GB_DECLARE(".Vertex.OutEdges", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Edge", VertexOutEdges_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(VertexAdjacent_next)

	CALL_GRAPH("_nextAdjacent", 0, NULL, "s");

END_METHOD

GB_DESC CVertexAdjacent[] = {
	GB_DECLARE(".Vertex.Adjacent", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "s", VertexAdjacent_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(GraphEdges_next)

	CALL_GRAPH("_nextEdge", 0, NULL, "Edge");

END_METHOD

BEGIN_METHOD(GraphEdges_get, GB_STRING src; GB_STRING dst)

	GB.Push(2, GB_T_STRING, STRING(src), LENGTH(src),
	           GB_T_STRING, STRING(dst), LENGTH(dst));
	CALL_GRAPH("_getEdge", 2, "ss", "Edge");

END_METHOD

BEGIN_METHOD(GraphEdges_Exist, GB_STRING src; GB_STRING dst)

	GB_VALUE *ret;

	GB.Push(2, GB_T_STRING, STRING(src), LENGTH(src),
	           GB_T_STRING, STRING(dst), LENGTH(dst));
	CALL_GRAPH_VAL("_getEdge", 2, "ss", "Edge", ret);
	GB.ReturnBoolean(ret->_object.value != NULL);

END_METHOD

BEGIN_PROPERTY(GraphEdges_Count)

	TRY_CALL_GRAPH("_countEdges", 0, NULL, "i");

	/* Default: enumerate the edges, counting */

END_PROPERTY

GB_DESC CGraphEdges[] = {
	GB_DECLARE(".Graph.Edges", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "Edge", GraphEdges_next, NULL),
	GB_METHOD("_get", "Edge", GraphEdges_get, "(Src)s(Dst)s"),
	GB_METHOD("Exist", "b", GraphEdges_Exist, "(Src)s(Dst)s"),

	GB_PROPERTY_READ("Count", "i", GraphEdges_Count),

	GB_END_DECLARE
};

typedef struct {
	GB_BASE ob;
	char *src, *dst;
	float weight;
} CEDGE;

#define EDGE	((CEDGE *) _object)

BEGIN_METHOD(Edge_new, GB_STRING src; GB_STRING dst; GB_FLOAT weight)

	EDGE->src = MISSING(src) ? NULL : GB.NewString(STRING(src), LENGTH(src));
	EDGE->dst = MISSING(dst) ? NULL : GB.NewString(STRING(dst), LENGTH(dst));
	EDGE->weight = MISSING(weight) ? 0 : VARG(weight);

END_METHOD

BEGIN_METHOD_VOID(Edge_free)

	GB.FreeString(&EDGE->src);
	GB.FreeString(&EDGE->dst);

END_METHOD

BEGIN_PROPERTY(Edge_Src)

	if (READ_PROPERTY) {
		GB.ReturnString(EDGE->src);
		return;
	}
	EDGE->src = GB.NewString(PSTRING(), PLENGTH());

END_PROPERTY

BEGIN_PROPERTY(Edge_Dst)

	if (READ_PROPERTY) {
		GB.ReturnString(EDGE->dst);
		return;
	}
	EDGE->dst = GB.NewString(PSTRING(), PLENGTH());

END_PROPERTY

BEGIN_PROPERTY(Edge_Weight)

	if (READ_PROPERTY) {
		GB.ReturnFloat(EDGE->weight);
		return;
	}
	EDGE->weight = VPROP(GB_FLOAT);

END_PROPERTY

GB_DESC CEdge[] = {
	GB_DECLARE("Edge", sizeof(CEDGE)),

	GB_METHOD("_new", NULL, Edge_new, "[(Src)s(Dst)s(Weight)f]"),
	GB_METHOD("_free", NULL, Edge_free, NULL),

	GB_PROPERTY("Src", "s", Edge_Src),
	GB_PROPERTY("Dst", "s", Edge_Dst),
	GB_PROPERTY("Weight", "f", Edge_Weight),

	GB_PROPERTY("Source", "s", Edge_Src),
	GB_PROPERTY("Destination", "s", Edge_Dst),

	GB_END_DECLARE
};
