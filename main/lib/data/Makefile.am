COMPONENT = gb.data
include $(top_srcdir)/component.am

gblib_LTLIBRARIES = gb.data.la

gb_data_la_LDFLAGS = -module @LD_FLAGS@
gb_data_la_CPPFLAGS  = -I$(top_srcdir)/share -I$(top_srcdir)/gbx
gb_data_la_CFLAGS = $(AM_CFLAGS)

gb_data_la_SOURCES = \
 main.h main.c list.h lookup3.h string_compare.h \
 c_list.h c_list.c \
 c_deque.h c_deque.c \
 c_circular.h c_circular.c \
 c_avltree.h c_avltree.c \
 c_graph.h c_graph.c \
 c_graphmatrix.h c_graphmatrix.c \
 c_heap.h c_heap.c \
 trie.h trie.c \
 c_trie.h c_trie.c

