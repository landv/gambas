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

#include <stdio.h>

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

BEGIN_METHOD(Graph_call, GB_BOOLEAN d; GB_BOOLEAN w)

	GB_CLASS GraphMatrixClass;

	GB.Push(2, GB_T_BOOLEAN, VARGOPT(d, 0),
		   GB_T_BOOLEAN, VARGOPT(w, 0));
	GraphMatrixClass = GB.FindClass("GraphMatrix");
	GB.ReturnObject(GB.New(GraphMatrixClass, NULL, (void *) (intptr_t) 2));

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

	GB_STATIC_METHOD("_call", "Graph", Graph_call, "[(Directed)b(Weighted)b]"),
	GB_METHOD("_free", NULL, Graph_free, NULL),

	GB_METHOD("_getVertex", "_Graph_Vertex", Graph_NoMethod, "(Vertex)s"),
	GB_METHOD("_getEdge", "_Graph_Edge", Graph_NoMethod, "(Src)s(Dst)s"),

	GB_METHOD("_nextVertex", "s", Graph_NoMethod, NULL),
	GB_METHOD("_nextEdge", "String[]", Graph_NoMethod, NULL),

	GB_METHOD("_countVertices", "i", Graph_NoMethod, NULL),
	GB_METHOD("_countEdges", "i", Graph_NoMethod, NULL),

	GB_METHOD("Add", "_Graph_Vertex", Graph_NoMethod, "(Name)s"),
	GB_METHOD("Remove", NULL, Graph_NoMethod, "(Vertex)s"),
	GB_METHOD("Connect", "_Graph_Edge", Graph_NoMethod, "(Src)s(Dst)s"),
	GB_METHOD("Disconnect", NULL, Graph_NoMethod, "(Src)s(Dst)s"),

	GB_PROPERTY("Tag", "v", Graph_Tag),

	GB_PROPERTY_SELF("Vertices", ".Graph.Vertices"),
	GB_PROPERTY_SELF("Edges", ".Graph.Edges"),

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
	CALL_GRAPH("_getVertex", 1, "s", NULL);

END_METHOD

BEGIN_PROPERTY(GraphVertices_Count)

	TRY_CALL_GRAPH("_countVertices", 0, NULL, "i");

	/* Default: enumerate the vertices, counting */

END_PROPERTY

GB_DESC CGraphVertices[] = {
	GB_DECLARE(".Graph.Vertices", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "s", GraphVertices_next, NULL),
	GB_METHOD("_get", "_Graph_Vertex", GraphVertices_get, "(Vertex)s"),

	GB_PROPERTY_READ("Count", "i", GraphVertices_Count),

	GB_END_DECLARE
};

BEGIN_PROPERTY(GraphVertex_InDegree)

	/* Default: enumerate */

END_PROPERTY

BEGIN_PROPERTY(GraphVertex_OutDegree)

	/* Default: enumerate */

END_PROPERTY

GB_DESC CGraphVertex[] = {
	GB_DECLARE("_Graph_Vertex", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_nextInEdge", "String[]", Graph_NoMethod, NULL),
	GB_METHOD("_nextOutEdge", "String[]", Graph_NoMethod, NULL),
	GB_METHOD("_nextAdjacent", "s", Graph_NoMethod, NULL),

	GB_PROPERTY_READ("InDegree", "i", GraphVertex_InDegree),
	GB_PROPERTY_READ("OutDegree", "i", GraphVertex_OutDegree),
	GB_PROPERTY("Name", "s", Graph_NoProperty),
	GB_PROPERTY("Value", "v", Graph_NoProperty),

	GB_PROPERTY_SELF("InEdges", ".Vertex.InEdges"),
	GB_PROPERTY_SELF("OutEdges", ".Vertex.OutEdges"),
	GB_PROPERTY_SELF("Adjacent", ".Vertex.Adjacent"),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(VertexInEdges_next)

	CALL_GRAPH("_nextInEdge", 0, NULL, "String[]");

END_METHOD

GB_DESC CVertexInEdges[] = {
	GB_DECLARE(".Vertex.InEdges", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "String[]", VertexInEdges_next, NULL),

	GB_END_DECLARE
};

BEGIN_METHOD_VOID(VertexOutEdges_next)

	CALL_GRAPH("_nextOutEdge", 0, NULL, "String[]");

END_METHOD

GB_DESC CVertexOutEdges[] = {
	GB_DECLARE(".Vertex.OutEdges", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "String[]", VertexOutEdges_next, NULL),

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

	CALL_GRAPH("_nextEdge", 0, NULL, "String[]");

END_METHOD

BEGIN_METHOD(GraphEdges_get, GB_STRING src; GB_STRING dst)

	GB.Push(2, GB_T_STRING, STRING(src), LENGTH(src),
	           GB_T_STRING, STRING(dst), LENGTH(dst));
	CALL_GRAPH("_getEdge", 2, "ss", NULL);

END_METHOD

BEGIN_PROPERTY(GraphEdges_Count)

	TRY_CALL_GRAPH("_countEdges", 0, NULL, "i");

	/* Default: enumerate the edges, counting */

END_PROPERTY

GB_DESC CGraphEdges[] = {
	GB_DECLARE(".Graph.Edges", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_next", "String[]", GraphEdges_next, NULL),
	GB_METHOD("_get", "_Graph_Edge", GraphEdges_get, "(Src)s(Dst)s"),
	GB_METHOD("Exist", "b", Graph_NoMethod, "(Src)s(Dst)s"),

	GB_PROPERTY_READ("Count", "i", GraphEdges_Count),

	GB_END_DECLARE
};

GB_DESC CGraphEdge[] = {
	GB_DECLARE("_Graph_Edge", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY_READ("Src", "s", Graph_NoProperty),
	GB_PROPERTY_READ("Dst", "s", Graph_NoProperty),
	GB_PROPERTY("Weight", "f", Graph_NoProperty),

	GB_PROPERTY_READ("Source", "s", Graph_NoProperty),
	GB_PROPERTY_READ("Destination", "s", Graph_NoProperty),

	GB_END_DECLARE
};
