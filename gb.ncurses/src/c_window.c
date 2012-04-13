/*
 * c_window.c - gb.ncurses Window class
 *
 * Copyright (C) 2012 Tobias Boege <tobias@gambas-buch.de>
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

#define __C_WINDOW_C

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <panel.h>

#include "gambas.h"
#include "gb_common.h"

#include "main.h"
#include "c_window.h"

/* The nc_window currently having the focus (raising Read events) */
static struct nc_window *focussed;

DECLARE_EVENT(EVENT_Read);
//DECLARE_EVENT(EVENT_Redraw);

/*
 * C NCurses interface
 * @_object: Reference to the struct nc_window representing the current object.
 *
 * The theory: Each struct nc_window has a main and a content window pointer. The main window
 * represents the window object outwards. It is linked to a panel structure which
 * is managed by the ncurses panel extension library to allow overlapping windows.
 * One aim of this design is to turn the ncurses-style inner-window border
 * into an outer-window border thus separating the graphics from the content so that the user
 * can access the window dimensions specified upon object instanciation whatever border-settings
 * the window may have and does not accidentally overwrite the border.
 * This is achieved by creating a dedicated content window accessed via the content member
 * of the structure. If there is no border (the border member is zero), the content window
 * is the same as the main window, both pointers are equal. If there is a border, a new
 * content window will be created which has the exact same size and content as the main window
 * has, the main window is enlarged to provide a border around the content.
 * Technically: the new content window is a subwindow of the enlarged main window so that they
 * share memory. All operations on the content window (wherever it points to) affects the memory
 * of the main window. The content windows are syncok()ed to their parent windows so that
 * any changes are immediately available to the latter and consequently to the physical screen upon
 * the next window refresh, that is the update_panel(); doupdate(); sequence of statements which
 * also takes care of window overlapping.
 */

#define E_COORDS	"Coordinates out of range"
#define E_DIMENSION	"Dimensions do not fit on screen"

void WINDOW_refresh()
{
	if (!NCURSES_running())
		return;
	update_panels();
	doupdate();
}

/**
 * Copies text with attributes from a window to a newly malloced array as if the window
 * was linear memory, too (line by line).
 * @src: source window
 * @arrp: pointer to store the newly malloced pointer containing the data in
 * @x: x position
 * @y: y position to start
 * @len: number of characters to read
 */
static int nc_window_get_mem(WINDOW *src, chtype **arrp, int sx, int sy, int len)
{
	int i;
	int ox, oy;
	chtype ch;

	getyx(src, oy, ox);
	MAKE_COORDS(src, sx, sy);
	if (BAD_COORDS(src, sx, sy))
	{
		GB.Error(E_COORDS);
		return -1;
	}
	len = MinMax(len, 0, (getmaxy(src) - sy) * getmaxx(src) - sx);

	GB.Alloc((void **) arrp, len * sizeof(chtype));
	for (i = 0; i < len; i++)
	{
		ch = mvwinch(src, sy, sx);
		(*arrp)[i] = ch;
		if (++sx >= getmaxx(src))
		{
			sy++;
			sx = 0;
		}
	}

	wmove(src, oy, ox);
	return 0;
}

/**
 * Copies text with attributes from memory to a window, temporarily turning off any additional
 * attributes the destination window may have.
 * @arr: data source
 * @dst: destination window
 * @sx: starting x
 * @sy: starting y coordinate
 * @len: length of data to copy from @arr (may not exceed its limits)
 */
static int nc_window_put_mem(chtype *arr, WINDOW *dst, int sx, int sy, unsigned int len)
{
	int i;
	int a;
	int attrs = getattrs(dst);

	MAKE_COORDS(dst, sx, sy);
	if (BAD_COORDS(dst, sx, sy))
	{
		GB.Error(E_COORDS);
		return -1;
	}
	XY2A(dst, sx, sy, a);
	len = MinMax(len, 0, getmaxy(dst) * getmaxx(dst) - a);

	wmove(dst, sy, sx);
	/* addch() ORs the current attributes but we want the chtypes drawn as they are in the buffer */
	wattrset(dst, A_NORMAL);
	/* advancing the cursor position is the good thing about addch() (at least, it is documented
	   behaviour...) */
	for (i = 0; i < len; i++) waddch(dst, arr[i]);
	wattrset(dst, attrs);
	return 0;
}

/**
 * Copy the specified rectangle of text
 * @src: source window
 * @dst: destination window
 * @sx: source starting x
 * @sy: source starting y
 * @nx: number of columns from @sx
 * @ny: number of lines from @sy
 * @dx: destination starting x
 * @dy: destinatino starting y
 */
static int nc_window_copy_window(WINDOW *src, WINDOW *dst, int sx, int sy, int nx, int ny, int dx, int dy)
{
	int i, j;
	chtype *chbuf; /* We ought to have a separate buffer in case @src and @dst overlap */
	int dattrs = getattrs(dst);

	MAKE_COORDS(src, sx, sy);
	MAKE_COORDS(dst, dx, dy);
	if (BAD_COORDS(src, sx, sy) || BAD_COORDS(dst, dx, dy))
	{
		GB.Error(E_COORDS);
		return -1;
	}
	if (BAD_COORDS(src, sx + nx - 1, sy + ny - 1) ||
	    BAD_COORDS(dst, dx + nx - 1, dy + ny - 1))
	{
		GB.Error(E_COORDS);
		return -1;
	}

	GB.Alloc((void **) &chbuf, nx * ny * sizeof(chtype));
	for (i = 0; i < ny; i++)
	{
		for (j = 0; j < nx; j++)
		{
			chbuf[i * nx + j] = mvwinch(src, sy + i, sx + j);
		}
	}

	wattrset(dst, A_NORMAL);
	for (i = 0; i < ny; i++)
	{
		if (dy + i >= getmaxy(dst)) break;
		wmove(dst, dy + i, dx);
		for (j = 0; j < nx; j++)
		{
			waddch(dst, chbuf[i * nx + j]);
		}
	}
	wattrset(dst, dattrs);
	GB.Free((void **) &chbuf);
	return 0;
}

/**
 * Unlink and deallocate the main panel together with the window and, if present, the
 * content window, refresh the screen to immediately remove leftovers of the window.
 */
static int nc_window_remove(void *_object)
{
	wclear(THIS->content);
	if (HAS_BORDER)
	{
		delwin(THIS->content);
		wclear(THIS->main);
	}
	REFRESH();
	del_panel(THIS->pan);
	delwin(THIS->main);
	return 0;
}

/**
 * Create a content window
 * The contents of the main window are copied to the newly created content window.
 */
static int nc_window_add_content(void *_object)
{
	WINDOW *new;

	if (HAS_BORDER) return 0;
	new = derwin(THIS->main, getmaxy(THIS->main) - 2, getmaxx(THIS->main) - 2, 1, 1);
	keypad(new, TRUE);
	syncok(new, TRUE);
	THIS->content = new;
	/* Copy main window contents, the library only provides overwrite() and friends for
	   this purpose but the windows do not overlap entirely as required by these functions */
	nc_window_main_to_content();
	/* The main window may not have any additional attributes but the content window */
	wattrset(new, getattrs(THIS->main));
	wattrset(THIS->main, A_NORMAL);
	return 0;
}

/**
 * Remove a content window
 * Free the memory taken up by the content window, substitute its pointer with the
 * main window pointer. Textual content in the content window and its attributes are copied
 * to the main window.
 */
static int nc_window_remove_content(void *_object)
{
	if (!HAS_BORDER) return 0;
	nc_window_content_to_main();
	wattrset(THIS->main, getattrs(THIS->content));
	delwin(THIS->content);
	THIS->content = THIS->main;
	return 0;
}

/**
 * Draws an inner-window border to the main window which appears as outer-window for the content
 * @b:  0: erase the border by overwriting it with all spaces
 *     !0: print a border
 */
static int nc_window_draw_border(void *_object, bool b)
{
	/* TODO: how to check for the possibility of displaying ACS chars?
	   terminfo exports the 'acsc' variable but i couldn't find
	   the format of that string */
	//nc_window_print(THIS, tigetstr("acsc"), -1, -1);
	if (b) box(THIS->main, 0, 0);
	else wborder(THIS->main, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	return 0;
}

/**
 * Move the cursor associated with the content window.
 * @x: x position
 * @y: y position relative to the upper left corner of the content window
 * Either of the parameters may be -1 which indicates to not change the current
 * value
 */
static int nc_window_cursor_move(void *_object, int x, int y)
{
	MAKE_COORDS(THIS->content, x, y);
	if (BAD_COORDS(THIS->content, x, y))
	{
		GB.Error(E_COORDS);
		return -1;
	}

	if (wmove(THIS->content, y, x) == ERR)
	{
		GB.Error("Could not move cursor");
		return -1;
	}
	return 0;
}

/**
 * Move the entire nc_window on screen.
 * @x: new x coordinate
 * @y: new y coordinate of the upper left corner of the main window relative to the
 * screen. Again, -1 denotes to leave the value as is
 */
static int nc_window_move(void *_object, int x, int y)
{
	int ox, oy;

	getbegyx(THIS->main, oy, ox);
	if (x == -1) x = ox;
	if (y == -1) y = oy;
	if (BAD_COORDS(stdscr, x, y))
	{
		GB.Error(E_COORDS);
		return -1;
	}

	move_panel(THIS->pan, y, x);
	if (HAS_BORDER)
	{
		mvwin(THIS->content, y + 1, x + 1);
		nc_window_draw_border(THIS, 1);
	}
	return 0;
}

/**
 * Resize the whole window. An enlarged content window is filled with spaces,
 * whereas a shortened content window gets truncated with respect to its 0,0
 * upper left corner.
 * @w: new width
 * @h: new height of the content window.
 * The main window is resized accordingly. Values of -1 prevent the present
 * dimensions from being altered
 */
static int nc_window_resize(void *_object, int w, int h)
{
	int ow, oh;

	getmaxyx(THIS->content, ow, oh);
	if (w == -1) w = ow;
	if (h == -1) h = oh;

	if (HAS_BORDER)
	{
		if ((w + 2) <= 0 || (w + 2) > COLS ||
		    (h + 2) <= 0 || (h + 2) > LINES)
		{
			GB.Error(E_DIMENSION);
			return -1;
		}

		nc_window_draw_border(THIS, 0);
		wresize(THIS->main, h + 2, w + 2);
	}

	if (w <= 0 || w > COLS || h <= 0 || h > LINES)
	{
		GB.Error(E_DIMENSION);
		return -1;
	}
	wresize(THIS->content, h, w) == ERR;
	replace_panel(THIS->pan, THIS->main);

	if (HAS_BORDER) nc_window_draw_border(THIS, 1);
	return 0;
}

/**
 * Print a given string in the content window starting at x,y position overwriting what is
 * already there.
 * @str: the string to print
 * @x: x coordinate
 * @y: y coordinate relative to the upper left corner of the content window
 * The principle of -1 arguments for x,y as described previously applies also to this function.
 * A set wrapping option causes @str to wrap around to the next line if it doesn't fit. Otherwise
 * it is truncated at line's end
 */
static int nc_window_print(void *_object, char *str, int x, int y)
{
	int width, len;
	char *p;

	p = str;
	do
	{
		if (nc_window_cursor_move(THIS, x, y) == -1) return -1;
		width = getmaxx(THIS->content) - x;
		if ((len = strlen(p)) < width) width = len;
		waddnstr(THIS->content, p, width);
		p += width;
		if (!THIS->wrap) break;
		x = 0;
		y++;
		if (y >= getmaxy(THIS->content)) break;
	}
	while (*p);
	return 0;
}

#if 0 /* Works but a simpler, although more memory intense, solution is ready */
/**
 * Insert a given string before the cursor position denoted by x,y coordinates shifting what follows
 * this position.
 * @str: string to insert
 * @x: x coordinate
 * @y: y coordinate
 * -1 as coordinate value means to leave the current value set.
 * If wrapping is set for the window, the entire content, which means all lines, are shifted. If wrapping
 * was turned off, only the current line is shifted but not wrapped around if the end is reached.
 * Note that this function calls itself recursively. Regarding the memory usage and recursion end conditions,
 * it is obvious that if this function leads to stack overflow something was seriously screwed up in the earlier
 * call contexts (or the terminal has about 2^penguin gazillion lines).
 */
int nc_window_insert(void *_object, char *str, int x, int y)
{
	static int ex = 0, ey = 0; /* hold the cursor positions where the inserted text will end */
	int len, res, a;
	static char *shifted = NULL; /* we have one buffer that is overwritten by each call */
	static char *temp = NULL; /* temporary storage to be stored in @shifted */
	static int rec = 0; /* keeps track of recursion depth to do optimisation */

	/* FIXME: this algorithm really messes up the view when wrapped and e.g. A_REVERSE is set on the window:
	   it does not preserve the attributes of the formerly present text */

	rec++;
	if (nc_window_cursor_move(THIS, x, y) == -1)
	{
		res = -1;
		goto _return;
	}

	/* we need the real vals, nc_window_cursor_move() interpreted the -1 values for us */
	x = getcurx(THIS->content);
	y = getcury(THIS->content);
	if (rec == 1) /* first call, initialise all static control data */
	{
		XY2A(THIS->content, getmaxx(THIS->content) - 1, getmaxy(THIS->content) - 1, a);
		a += strlen(str);
		A2XY(THIS->content, a, ex, ey);
		/* the text may fill the entire box, so we better catch that and locate the cursor
		   at the end in that case */
		if (ey >= getmaxy(THIS->content))
		{
			ex = getmaxx(THIS->content) - 1;
			ey = getmaxy(THIS->content) - 1;
		}
	}

	/* if we're on the last line, nothing will be wrapped around at all, so we take this branch.
	   in this case, the call works exactly like the non-wrapping insert and begins to unroll the
	   recursion. */
	if (!THIS->wrap || y == getmaxy(THIS->content) - 1)
	{
		mvwinsstr(THIS->content, y, x, str);
	}
	else
	{
		len = strlen(str);
		if (rec == 1)
		{
			if (!(shifted = malloc(len + 1)) || !(temp = malloc(len + 1)))
			{
				GB.Error("Could not allocate memory");
				res = -1;
				goto _return;
			}
		}
		/* if the @str itself overflows the line, there will be a remainder of
		   it that has to preceed the shifted string to be wrapped to the next line */
		if (x + len > getmaxx(THIS->content))
		{
			len = getmaxx(THIS->content) - x;
			strcpy(temp, str + len);
			winstr(THIS->content, temp + strlen(temp));
		}
		else
		{
			/* remainder on the right (giving a negative @n to winnstr() family seems broken
			   in my library and may be elsewhere (manpages say it's an extension to XSI curses) */
			mvwinstr(THIS->content, y, getmaxx(THIS->content) - len, temp);
		}
		/* the winnstr() family of functions used to extract shifted characters moves the cursor, we have
		   to reset it here (would have been nice to leave that fact in the manpages...) */
		mvwinsstr(THIS->content, y, x, str);
		strcpy(shifted, temp);
		nc_window_insert(THIS, shifted, 0, y + 1);
	}

	res = 0;
	_return:
	if (!--rec)
	{
		nc_window_cursor_move(THIS, ex, ey);
		REFRESH();
		if (shifted)
		{
			free(shifted);
			shifted = NULL;
		}
		if (temp)
		{
			free(temp);
			temp = NULL;
		}
	}
	return res;
}
#endif

/**
 * Insert a given string before the cursor position denoted by x,y and shift what follows it
 * @str: string to insert
 * @x: x coordinate
 * @y: y coordinate
 * -1 as coordinate value means to leave the current value set.
 * If wrapping is set for the window, the entire content, which means all lines, are shifted. If wrapping
 * was turned off, only the current line is shifted but not wrapped around if the end is reached.
 */
static int nc_window_insert(void *_object, char *str, int x, int y)
{
	int a, i, len, slen;
	chtype *ins, *shifted;

	if (nc_window_cursor_move(THIS, x, y) == -1) return -1;
	if (!THIS->wrap)
	{
		winsstr(THIS->content, str);
		return 0;
	}

	x = getcurx(THIS->content);
	y = getcury(THIS->content);
	XY2A(THIS->content, x, y, a);
	len = strlen(str);
	slen = getmaxy(THIS->content) * getmaxx(THIS->content) - a - len + 1;
	GB.Alloc((void **) &ins, len * sizeof(chtype));
	for (i = 0; i < len; i++) ins[i] = (chtype) str[i] | getattrs(THIS->content);
	nc_window_get_mem(THIS->content, &shifted, x, y, getmaxy(THIS->content) * getmaxx(THIS->content) - a);

	nc_window_put_mem(ins, THIS->content, x, y, len);
	getyx(THIS->content, y, x);
	nc_window_put_mem(shifted, THIS->content, x, y, slen);

	/* place the cursor after the last inserted char */
	nc_window_cursor_move(THIS, x, y);

	GB.Free((void **) &shifted);
	GB.Free((void **) &ins);
	return 0;
}

/**
 * Get a string from the content window starting at x,y
 * @x: x coordinate
 * @y: y coordinate to start reading from
 * @len: length of string to read
 * @ret: to store a newly malloced pointer containing the string from @x,@y and of length @len
 *       (or what it implied) in
 * @x and @y may be -1. If @len is -1 we shall read until the end of line.
 * The cursor position is reset after the operation.
 */
static int nc_window_get_str(void *_object, int x, int y, unsigned int len, char **ret)
{
	int i;
	chtype *chbuf;
	char *buf;

	if (len == -1) len = getmaxx(THIS->content) - getcurx(THIS->content);

	GB.Alloc((void **) &buf, len + 1);
	nc_window_get_mem(THIS->content, &chbuf, x, y, len);
	for (i = 0; i < len; i++) buf[i] = chbuf[i] & A_CHARTEXT;
	buf[len] = 0;
	GB.Free((void **) &chbuf);

	*ret = buf;
	return 0;
}

/**
 * Wait for a keypress for the given amount of time
 * @timeout: gives the number of milliseconds for a timeout. If the timeout expires,
 *           0 will be delivered back. A negative value denotes to not use any timeout.
 * @ret: the key value (or 0 in case of timeout expiration) will be stored at this address.
 */
static int nc_window_key_timeout(void *_object, int timeout, int *ret)
{
	/* wtimeout() and wgetch() cause ncurses, for whatever reason, to mess up the panel
	   layout: the particular window gets risen to the top of everything. Consequently
	   I use the stdscr here which appears to not suffer from this. */
	timeout(timeout);
	*ret = getch();
	if (*ret == ERR)
	{
		/* Had a timeout, the manual doesn't define any errors to happen for wgetch()
		   besides NULL pointer arguments. The only source of ERR is timeout expired. */
		if (timeout >= 0) *ret = 0;
	}

	timeout(-1);
	return 0;
}

/**
 * Move the main panel to the bottom of the panel stack thus arrange it
 * below all other windows that may overlap it
 */
static int nc_window_bottom(void *_object)
{
	bottom_panel(THIS->pan);
	return 0;
}

/**
 * Move the main panel to the top of the panel stack
 */
static int nc_window_top(void *_object)
{
	top_panel(THIS->pan);
	return 0;
}

/**
 * Remove the main panel from the panel stack thus hiding it from the user
 */
static int nc_window_hide(void *_object)
{
	hide_panel(THIS->pan);
	return 0;
}

/**
 * Insert the main panel into the panel stack. note that the panel is moved to the top
 * of the stack thus making it "the most" visible
 */
static int nc_window_show(void *_object)
{
	show_panel(THIS->pan);
	return 0;
}

/**
 * Callback for data on stdin
 * @fd: stdin means 0
 * @type: having watched stdin for read means GB_WATCH_READ
 * This function raises the Read event. Since only one callback appears to be able to
 * be registered for an fd, there can only be one window to raise this event, i.e.
 * the one being set to "have the focus" via Window.SetFocus()
 */
static void nc_window_read_callback(int fd, int type, void *_object)
{
	GB.Raise(THIS, EVENT_Read, 0);
}

/**
 * Bring the given nc_window to focus. That means that this window now raises the
 * Read event if data arrives on stdin. The global variable @focussed is used to
 * keep track of that very window
 */
static int nc_window_setfocus(void *_object)
{
	focussed = THIS;
	GB.Watch(0, GB_WATCH_READ, nc_window_read_callback, (intptr_t) THIS);
	return 0;
}

/**
 * Import a window and fill the @_object with default values (actual object instanciation)
 * @imp: WINDOW to import
 */
static int nc_window_import(void *_object, WINDOW *imp)
{
	/* Clean up former members */
	if (HAS_BORDER) nc_window_remove_content(THIS);
	if (THIS->main) nc_window_remove(THIS);

	THIS->main = imp;
	THIS->pan = new_panel(imp);
	keypad(imp, TRUE);
	THIS->border = FALSE;
	THIS->wrap = TRUE;
	THIS->content = THIS->main;
	return 0;
}

/**
 * Return the state of, set or unset a given attribute
 * @attr: attribute subject
 * @req: the operation to perform. One of ATTR_DRV_*, watch the source on what they do
 *       in particular
 * This routine works with "normal" attributes and colors indifferently (as the latter are
 * attributes)
 */
static int nc_window_attrs_driver(void *_object, int attr, int req)
{
	switch (req)
	{
		case ATTR_DRV_RET:
			return getattrs(THIS->content);
		case ATTR_DRV_ON:
			wattron(THIS->content, attr);
			break;
		case ATTR_DRV_OFF:
			wattroff(THIS->content, attr);
	}
	return 0;
}

/**
 * Return the state of, set or unset a given attribute on a single character
 * @attr: attribute
 * @x: x coordinate
 * @y: y coordinate
 * @req: request to perform
 * As this is the character-based counterpart to nc_window_attrs_driver, this
 * also handles colors
 */
static int nc_window_char_attrs_driver(void *_object, int attr, int x, int y, int req)
{
	int ox, oy;
	int res;
	chtype ch;
	unsigned int mask, shift; /* unsigned in case A_COLOR has its MSB set */

	/* I wonder if everybody searched and hacked around to find that color stuff... */
	/* Find the number of bits to right-shift to get the color number from chtype
	   attributes - _portably_ (don't know if it is required, if anyone could point
	   to the magical manpage where all that is written?) */
	mask = ~A_COLOR;
	for (shift = 0; mask & 1; shift++, mask >>= 1);

	getyx(THIS->content, oy, ox);
	nc_window_cursor_move(THIS, x, y);
	ch = winch(THIS->content);
	switch (req)
	{
		case ATTR_DRV_RET:
			/* A_ATTRIBUTES inherits A_COLOR (at least on my setup... I really
			   hate undocument facts as they may be no facts) */
			res = ch & A_ATTRIBUTES;
			goto _cleanup;
		case ATTR_DRV_ON:
			wchgat(THIS->content, 1, (ch & A_ATTRIBUTES) | attr, (ch & A_COLOR) >> shift, NULL);
			break;
		case ATTR_DRV_OFF:
			wchgat(THIS->content, 1, (ch & A_ATTRIBUTES) & ~attr, (ch & A_COLOR) >> shift, NULL);
	}

	res = 0;
	_cleanup:
	nc_window_cursor_move(THIS, ox, oy);
	return res;
}

/*
 * Window class
 */

BEGIN_PROPERTY(CWindow_windowattrs)

	THIS->pos.line = -1;
	THIS->pos.col = -1;
	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(CWindow_background)

	;

END_PROPERTY

BEGIN_PROPERTY(CWindow_border)

	bool b;
	int x, y;

	if (READ_PROPERTY) GB.ReturnBoolean(THIS->border);
	b = VPROP(GB_BOOLEAN);

	if (b == THIS->border) return;
	if (b)
	{
		nc_window_resize(THIS, getmaxx(THIS->content) + 2, getmaxy(THIS->content) + 2);
		nc_window_move(THIS, (x = getbegx(THIS->main) - 1) == -1 ? 0 : x,
		                     (y = getbegy(THIS->main) - 1) == -1 ? 0 : y);
		nc_window_add_content(THIS);
		nc_window_draw_border(THIS, 1);
	}
	else
	{
		/* FIXME: routine logic */
		/* FIXME: try Border = False if the window is Full() */
		nc_window_draw_border(THIS, 0);
		nc_window_remove_content(THIS);
		THIS->border = b; /* FIXME: this is important here due to the automatic border-redraw in nc_window_resize */
		nc_window_resize(THIS, getmaxx(THIS->content) - 2, getmaxy(THIS->content) - 2);
		nc_window_move(THIS, (x = getbegx(THIS->main) + 1) >= getmaxx(THIS->main) ? getmaxx(THIS->main) - 1 : x,
		               (y = getbegy(THIS->main) + 1) >= getmaxy(THIS->main) ? getmaxy(THIS->main) - 1 : y);
	}
	THIS->border = b;
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CWindow_container_height)

	GB.ReturnInteger(getmaxy(THIS->main));

END_PROPERTY

BEGIN_PROPERTY(CWindow_container_width)

	GB.ReturnInteger(getmaxx(THIS->main));

END_PROPERTY

BEGIN_PROPERTY(CWindow_cursorx)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getcurx(THIS->content));
		return;
	}
	nc_window_cursor_move(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CWindow_cursory)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getcury(THIS->content));
		return;
	}
	nc_window_cursor_move(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CWindow_foreground)

	;

END_PROPERTY

BEGIN_PROPERTY(CWindow_height)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getmaxy(THIS->content));
		return;
	}
	nc_window_resize(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CWindow_wrap)

	/* This property only affects subsequent prints */
	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->wrap);
		return;
	}
	THIS->wrap = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(CWindow_width)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getmaxx(THIS->content));
		return;
	}
	nc_window_resize(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CWindow_x)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getbegx(THIS->main));
		return;
	}
	nc_window_move(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CWindow_y)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getbegy(THIS->main));
		return;
	}
	nc_window_move(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

//GB_TIMER *timer;

//void nc_timer_callback(intptr_t param)
//{
//	addch('A' | A_REVERSE);refresh();
//	GB.Unref(&timer);
//}

BEGIN_METHOD(CWindow_new, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	WINDOW *new;

	if (!NCURSES_RUNNING)
	{
		GB.Error("Not in NCurses mode");
		return;
	}
	new = newwin(VARG(h), VARG(w), VARG(y), VARG(x));
	nc_window_import(THIS, new);

//	timer = GB.Every(1000, nc_timer_callback, NULL);
//	GB.Ref(&timer);

	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_free)

	nc_window_remove(THIS);
	if (focussed == THIS) GB.Watch(0, GB_WATCH_NONE, NULL, (intptr_t) NULL);
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_get, GB_INTEGER y; GB_INTEGER x; GB_INTEGER len)

	int gx, gy, glen;
	char *ret;

	gy = VARG(y);
	/* if x parameter is missing, we start at the beginning of the line */
	if (MISSING(x)) gx = 0;
	else gx = VARG(x);

	/* if no length is given, we return until the end of the line */
	if (MISSING(len)) glen = -1;
	else glen = VARG(len);

	nc_window_get_str(THIS, gx, gy, glen, &ret);
	GB.ReturnNewZeroString(ret);
	GB.Free((void **) &ret);

END_METHOD

BEGIN_METHOD(CWindow_put, GB_STRING text; GB_INTEGER y; GB_INTEGER x)

	int sx, sy;

	sy = VARG(y);

	if (MISSING(x)) sx = 0;
	else sx = VARG(x);

	nc_window_print(THIS, STRING(text), sx, sy);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_bottom)

	nc_window_bottom(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_cls)

	/* ncurses sets the cursor to 0,0 after wclear() which may or may not be
	   surprising. */
	wclear(THIS->content);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_full)

	nc_window_move(THIS, 0, 0);
	if (HAS_BORDER) nc_window_resize(THIS, COLS - 2, LINES - 2);
	else nc_window_resize(THIS, COLS, LINES);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_hide)

	nc_window_hide(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_hline, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len; GB_STRING ch; GB_INTEGER thickness)

	int ox, oy, gx, gy;
	char c;
	int length, t;
	int i;

	getyx(THIS->content, oy, ox);
	c = *(STRING(ch));
	length = VARG(len);
	if (MISSING(thickness)) t = 1;
	else t = VARG(thickness);

	gx = VARG(x);
	gy = VARG(y);
	for (i = 0; i < t; i++)
	{
		nc_window_cursor_move(THIS, gx, gy);
		whline(THIS->content, c, length);
		gy++;
	}
	nc_window_cursor_move(THIS, ox, oy);
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_insert, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	nc_window_insert(THIS, STRING(text), MISSING(x) ? -1 : VARG(x), MISSING(y) ? -1 : VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_locate, GB_INTEGER x; GB_INTEGER y)

	nc_window_cursor_move(THIS, VARG(x), VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_move, GB_INTEGER x; GB_INTEGER y)

	nc_window_move(THIS, MISSING(x) ? -1 : VARG(x), MISSING(y) ? -1 : VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_print, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	nc_window_print(THIS, STRING(text), MISSING(x) ? -1 : VARG(x), MISSING(y) ? -1 : VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_resize, GB_INTEGER w; GB_INTEGER h)

	nc_window_resize(THIS, MISSING(w) ? -1 : VARG(w), MISSING(h) ? -1 : VARG(h));
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_show)

	nc_window_show(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(CWindow_setfocus)

	nc_window_setfocus(THIS);

END_METHOD

BEGIN_METHOD_VOID(CWindow_top)

	nc_window_top(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_vline, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len; GB_STRING ch; GB_INTEGER thickness)

	int ox, oy, gx, gy;
	char c;
	int length, t;
	int i;

	getyx(THIS->content, oy, ox);
	c = *(STRING(ch));
	length = VARG(len);
	if (MISSING(thickness)) t = 1;
	else t = VARG(thickness);

	gx = VARG(x);
	gy = VARG(y);
	for (i = 0; i < t; i++)
	{
		nc_window_cursor_move(THIS, gx, gy);
		wvline(THIS->content, c, length);
		gx++;
	}
	nc_window_cursor_move(THIS, ox, oy);
	REFRESH();

END_METHOD

BEGIN_METHOD(CWindow_waitkey, GB_INTEGER timeout)

	int t;
	int ret;

	if (MISSING(timeout)) t = -1;
	else t = VARG(timeout);

	nc_window_key_timeout(THIS, t, &ret);
	GB.ReturnInteger(ret);

END_METHOD

/*
 * .Window.Attrs virtual class
 */

BEGIN_PROPERTY(CWindowAttrs_normal)

	/* normal is special because it turns off all other attributes and can't itself been turned off */
	if (READ_PROPERTY) GB.ReturnBoolean(getattrs(THIS->content) == A_NORMAL);
	if (VPROP(GB_BOOLEAN)) wattrset(THIS->content, A_NORMAL);

END_PROPERTY

BEGIN_PROPERTY(CWindowAttrs_underline)

	WIN_ATTR_METHOD_BOOL(A_UNDERLINE);

END_PROPERTY

BEGIN_PROPERTY(CWindowAttrs_reverse)

	WIN_ATTR_METHOD_BOOL(A_REVERSE);

END_PROPERTY

BEGIN_PROPERTY(CWindowAttrs_blink)

	WIN_ATTR_METHOD_BOOL(A_BLINK);

END_PROPERTY

BEGIN_PROPERTY(CWindowAttrs_bold)

	WIN_ATTR_METHOD_BOOL(A_BOLD);

END_PROPERTY

BEGIN_PROPERTY(CWindowAttrs_color)

	//inch() attributes have the pair number, but we give the color pair to attron()?
	//WIN_COLOR_METHOD();

END_PROPERTY

BEGIN_METHOD(CWindowAttrs_get, GB_INTEGER y; GB_INTEGER x)

	THIS->pos.line = VARG(y);
	THIS->pos.col = VARG(x);
	RETURN_SELF();

END_METHOD

/*
 * .Char.Attrs virtual class
 */

BEGIN_PROPERTY(CCharAttrs_normal)

END_PROPERTY

BEGIN_PROPERTY(CCharAttrs_underline)

	CHAR_ATTR_METHOD_BOOL(A_UNDERLINE);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CCharAttrs_reverse)

	CHAR_ATTR_METHOD_BOOL(A_REVERSE);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CCharAttrs_blink)

	CHAR_ATTR_METHOD_BOOL(A_BLINK);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CCharAttrs_bold)

	CHAR_ATTR_METHOD_BOOL(A_BOLD);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CCharAttrs_color)

END_PROPERTY

#define TIMEOUT_NOTIMEOUT	-1

GB_DESC CWindowDesc[] =
{
	GB_DECLARE("Window", sizeof(struct nc_window)),

	GB_EVENT("Read", NULL, NULL, &EVENT_Read),

	/* Constants */
	GB_CONSTANT("NoTimeout", "i", TIMEOUT_NOTIMEOUT),

	/* Properties */
	GB_PROPERTY_READ("Attributes", ".Window.Attrs", CWindow_windowattrs),

	GB_PROPERTY("Background", "i", CWindow_background),
	GB_PROPERTY("Paper", "i", CWindow_background),

	GB_PROPERTY("Border", "b", CWindow_border),

	//GB_PROPERTY("Columns",

	GB_PROPERTY_READ("ContainerHeight", "i", CWindow_container_height),
	GB_PROPERTY_READ("ContainerH", "i", CWindow_container_height),
	GB_PROPERTY_READ("ContainerWidth", "i", CWindow_container_width),
	GB_PROPERTY_READ("ComtainerW", "i", CWindow_container_width),

	GB_PROPERTY("CursorX", "i", CWindow_cursorx),
	GB_PROPERTY("CursorY", "i", CWindow_cursory),

	GB_PROPERTY("Foreground", "i", CWindow_foreground),
	GB_PROPERTY("Pen", "i", CWindow_foreground),

	GB_PROPERTY("Height", "i", CWindow_height),
	GB_PROPERTY("H", "i", CWindow_height),

	GB_PROPERTY("Wrap", "b", CWindow_wrap),

	GB_PROPERTY("Width", "i", CWindow_width),
	GB_PROPERTY("W", "i", CWindow_width),

	GB_PROPERTY("X", "i", CWindow_x),
	GB_PROPERTY("Y", "i", CWindow_y),

	/* Methods */
	GB_METHOD("_new", NULL, CWindow_new, "(x)i(y)i(w)i(h)i"),
	GB_METHOD("_free", NULL, CWindow_free, NULL),
	GB_METHOD("_get", "s", CWindow_get, "(y)i[(x)i(len)i]"),
	GB_METHOD("_put", NULL, CWindow_put, "(text)s(y)i[(x)i]"),

	GB_METHOD("Bottom", NULL, CWindow_bottom, NULL),
	GB_METHOD("Top", NULL, CWindow_top, NULL),

	GB_METHOD("Cls", NULL, CWindow_cls, NULL),

	GB_METHOD("Full", NULL, CWindow_full, NULL),

	GB_METHOD("Hide", NULL, CWindow_hide, NULL),
	GB_METHOD("Show", NULL, CWindow_show, NULL),

	GB_METHOD("HLine", NULL, CWindow_hline, "(x)i(y)i(len)i(ch)s[(thickness)i]"),
	GB_METHOD("VLine", NULL, CWindow_vline, "(x)i(y)i(len)i(ch)s[(thickness)i]"),

	GB_METHOD("Insert", NULL, CWindow_insert, "(text)s[(x)i(y)i]"),
	GB_METHOD("Print", NULL, CWindow_print, "(text)s[(x)i(y)i]"),

	GB_METHOD("Locate", NULL, CWindow_locate, "(x)i(y)i"),
	GB_METHOD("Move", NULL, CWindow_move, "[(x)i(y)i]"),
	GB_METHOD("Resize", NULL, CWindow_resize, "[(w)i(h)i]"),

	GB_METHOD("SetFocus", NULL, CWindow_setfocus, NULL),

	GB_METHOD("WaitKey", "i", CWindow_waitkey, "[(timeout)i]"),

	GB_END_DECLARE
};

GB_DESC CWindowAttrsDesc[] =
{
	GB_DECLARE(".Window.Attrs", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Normal", "b", CWindowAttrs_normal),
	GB_PROPERTY("Underline", "b", CWindowAttrs_underline),
	GB_PROPERTY("Reverse", "b", CWindowAttrs_reverse),
	GB_PROPERTY("Blink", "b", CWindowAttrs_blink),
	GB_PROPERTY("Bold", "b", CWindowAttrs_bold),

	GB_METHOD("_get", ".Char.Attrs", CWindowAttrs_get, "(y)i(x)i"),

	GB_END_DECLARE
};

GB_DESC CCharAttrsDesc[] =
{
	GB_DECLARE(".Char.Attrs", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Normal", "b", CCharAttrs_normal),
	GB_PROPERTY("Underline", "b", CCharAttrs_underline),
	GB_PROPERTY("Reverse", "b", CCharAttrs_reverse),
	GB_PROPERTY("Blink", "b", CCharAttrs_blink),
	GB_PROPERTY("Bold", "b", CCharAttrs_bold),

	GB_PROPERTY("Color", "i", CCharAttrs_color),

	GB_END_DECLARE
};
