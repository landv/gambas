/*
 * main.c - gb.data glue
 *
 * Copyright (C) 2012/3 Tobias Boege <tobias@gambas-buch.de>
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

#define __MAIN_C

#include "c_list.h"
#include "c_deque.h"
#include "c_circular.h"
#include "c_avltree.h"
#include "c_trie.h"
#include "c_graph.h"
#include "c_graphmatrix.h"
#include "c_heap.h"
#include "main.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT = {
	CList,
	CListBackwards,
	CListItem,

	CDeque,
	CStack,
	CQueue,
	CPrioQueue,

	CCircular,

	CAvlTree,

	CTrie,
	CTriePrefix,

	CGraph,
	CGraphVertices,
	CGraphEdges,
	CGraphInEdges,
	CGraphOutEdges,
	CGraphAdjacent,
	CGraphVertex,
	CGraphEdge,

	CGraphMatrix,
	CMatrixVertices,
	CMatrixEdges,
	CMatrixVertex,
	CMatrixEdge,

	CHeap,

	NULL
};

int EXPORT GB_INIT()
{
	return 0;
}


void EXPORT GB_EXIT()
{
}
