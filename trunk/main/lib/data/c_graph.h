/*
 * c_graph.h
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

#ifndef __C_GRAPH_H
#define __C_GRAPH_H

#include "gambas.h"

extern GB_INTERFACE GB;

typedef struct {
	void *_getVertex;
	void *_getEdge;

	void *_nextVertex;
	void *_nextEdge;

	void *_countVertices;
	void *_countEdges;

	void *_nextInEdge;
	void *_nextOutEdge;
	void *_nextAdjacent;

	void *_vertexProperty;
	void *_edgeProperty;

	void *_vertexUnknown;
	void *_edgeUnknown;
} GRAPH_DESC;

typedef struct {
	GB_BASE ob;
	GRAPH_DESC *desc;
	char *vertex;
	GB_ARRAY edge;
	GB_VARIANT_VALUE tag;
} CGRAPH;

extern GRAPH_DESC *get_desc(void *_object);

#ifndef __C_GRAPH_C
extern GB_DESC CGraph[], CGraphVertices[], CGraphEdges[], CGraphInEdges[],
	       CGraphOutEdges[], CGraphAdjacent[], CGraphVertex[],
	       CGraphEdge[];
#endif

#endif /* __C_GRAPH_H */
