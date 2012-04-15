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
static CWINDOW *focused = NULL;

#define MAIN (THIS ? THIS->main : stdscr)
#define CONTENT (THIS ? THIS->content : stdscr)

DECLARE_EVENT(EVENT_Read);

/*
 * C Window interface
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

/*
 * Redraw the screen no matter what particular buffering settings the windows have.
 * Note that this function may not be used to return into ncurses mode once left.
 */
void WINDOW_real_refresh()
{
	if (!NCURSES_running())
		return;
	update_panels();
	doupdate();
}
/**
 * Refresh the screen. This respects THIS' buffering wishes
 */
void WINDOW_refresh(void *_object)
{
	if (!IS_BUFFERED)
		WINDOW_real_refresh();
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
static int WINDOW_get_mem(WINDOW *src, chtype **arrp, int sx, int sy, int len)
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
static int WINDOW_put_mem(chtype *arr, WINDOW *dst, int sx, int sy, unsigned int len)
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
	for (i = 0; i < len; i++)
		waddch(dst, arr[i]);
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
static int WINDOW_copy_window(WINDOW *src, WINDOW *dst, int sx, int sy, int nx, int ny, int dx, int dy)
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
		for (j = 0; j < nx; j++)
			chbuf[i * nx + j] = mvwinch(src, sy + i, sx + j);

	wattrset(dst, A_NORMAL);
	for (i = 0; i < ny; i++)
	{
		if (dy + i >= getmaxy(dst)) break;
		wmove(dst, dy + i, dx);
		for (j = 0; j < nx; j++)
			waddch(dst, chbuf[i * nx + j]);
	}
	wattrset(dst, dattrs);
	GB.Free((void **) &chbuf);
	return 0;
}

/**
 * Unlink and deallocate the main panel together with the window and, if present, the
 * content window, refresh the screen to immediately remove leftovers of the window.
 */
static int WINDOW_remove(void *_object)
{
	wclear(CONTENT);
	if (HAS_BORDER)
	{
		delwin(CONTENT);
		wclear(MAIN);
	}
	REFRESH();
	del_panel(THIS->pan);
	delwin(MAIN);
	return 0;
}

/**
 * Create a content window
 * The contents of the main window are copied to the newly created content window.
 */
static int WINDOW_add_content(void *_object)
{
	WINDOW *new;

	if (HAS_BORDER)
		return 0;
	new = derwin(THIS->main, getmaxy(THIS->main) - 2, getmaxx(THIS->main) - 2, 1, 1);
	keypad(new, TRUE);
	syncok(new, TRUE);
	THIS->content = new;
	/* Copy main window contents, the library only provides overwrite() and friends for
	   this purpose but the windows do not overlap entirely as required by these functions */
	WINDOW_main_to_content();
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
static int WINDOW_remove_content(void *_object)
{
	if (!HAS_BORDER)
		return 0;
	WINDOW_content_to_main();
	wattrset(THIS->main, getattrs(THIS->content));
	delwin(THIS->content);
	THIS->content = MAIN;
	return 0;
}

/**
 * Draws an inner-window border to the main window which appears as outer-window for the content
 * @b:  0: erase the border by overwriting it with all spaces
 *     !0: print a border
 */
static int WINDOW_draw_border(void *_object, bool b)
{
	/* TODO: how to check for the possibility of displaying ACS chars?
	   terminfo exports the 'acsc' variable but i couldn't find
	   the format of that string */
	//WINDOW_print(THIS, tigetstr("acsc"), -1, -1);
	if (b)
		box(MAIN, 0, 0);
	else
		wborder(MAIN, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	return 0;
}

/**
 * Move the cursor associated with the content window.
 * @x: x position
 * @y: y position relative to the upper left corner of the content window
 * Either of the parameters may be -1 which indicates to not change the current
 * value
 */
static int WINDOW_cursor_move(void *_object, int x, int y)
{
	MAKE_COORDS(CONTENT, x, y);
	if (BAD_COORDS(CONTENT, x, y))
	{
		GB.Error(E_COORDS);
		return -1;
	}

	if (wmove(CONTENT, y, x) == ERR)
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
static int WINDOW_move(void *_object, int x, int y)
{
	int ox, oy;

	getbegyx(MAIN, oy, ox);
	if (x == -1)
		x = ox;
	if (y == -1)
		y = oy;
	if (BAD_COORDS(stdscr, x, y))
	{
		GB.Error(E_COORDS);
		return -1;
	}

	move_panel(THIS->pan, y, x);
	if (HAS_BORDER)
	{
		mvwin(CONTENT, y + 1, x + 1);
		WINDOW_draw_border(THIS, 1);
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
static int WINDOW_resize(void *_object, int w, int h)
{
	int ow, oh;

	getmaxyx(CONTENT, ow, oh);
	if (w == -1)
		w = ow;
	if (h == -1)
		h = oh;

	if (HAS_BORDER)
	{
		if ((w + 2) <= 0 || (w + 2) > COLS ||
		    (h + 2) <= 0 || (h + 2) > LINES)
		{
			GB.Error(E_DIMENSION);
			return -1;
		}

		WINDOW_draw_border(THIS, 0);
		wresize(MAIN, h + 2, w + 2);
	}

	if (w <= 0 || w > COLS || h <= 0 || h > LINES)
	{
		GB.Error(E_DIMENSION);
		return -1;
	}
	wresize(CONTENT, h, w) == ERR;
	replace_panel(THIS->pan, MAIN);

	if (HAS_BORDER)
		WINDOW_draw_border(THIS, 1);
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
static int WINDOW_print(void *_object, char *str, int x, int y)
{
	int width, len;
	char *p;

	p = str;
	do
	{
		if (WINDOW_cursor_move(THIS, x, y) == -1)
			return -1;
		width = getmaxx(CONTENT) - x;
		if ((len = strlen(p)) < width)
			width = len;
		waddnstr(CONTENT, p, width);
		p += width;
		if (THIS && !THIS->wrap)
			break;
		x = 0;
		y++;
		if (y >= getmaxy(CONTENT))
			break;
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
int WINDOW_insert(void *_object, char *str, int x, int y)
{
	static int ex = 0, ey = 0; /* hold the cursor positions where the inserted text will end */
	int len, res, a;
	static char *shifted = NULL; /* we have one buffer that is overwritten by each call */
	static char *temp = NULL; /* temporary storage to be stored in @shifted */
	static int rec = 0; /* keeps track of recursion depth to do optimisation */

	/* FIXME: this algorithm really messes up the view when wrapped and e.g. A_REVERSE is set on the window:
	   it does not preserve the attributes of the formerly present text */

	rec++;
	if (WINDOW_cursor_move(THIS, x, y) == -1)
	{
		res = -1;
		goto _return;
	}

	/* we need the real vals, WINDOW_cursor_move() interpreted the -1 values for us */
	x = getcurx(CONTENT);
	y = getcury(CONTENT);
	if (rec == 1) /* first call, initialise all static control data */
	{
		XY2A(CONTENT, getmaxx(CONTENT) - 1, getmaxy(CONTENT) - 1, a);
		a += strlen(str);
		A2XY(CONTENT, a, ex, ey);
		/* the text may fill the entire box, so we better catch that and locate the cursor
		   at the end in that case */
		if (ey >= getmaxy(CONTENT))
		{
			ex = getmaxx(CONTENT) - 1;
			ey = getmaxy(CONTENT) - 1;
		}
	}

	/* if we're on the last line, nothing will be wrapped around at all, so we take this branch.
	   in this case, the call works exactly like the non-wrapping insert and begins to unroll the
	   recursion. */
	if (!THIS->wrap || y == getmaxy(CONTENT) - 1)
	{
		mvwinsstr(CONTENT, y, x, str);
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
		if (x + len > getmaxx(CONTENT))
		{
			len = getmaxx(CONTENT) - x;
			strcpy(temp, str + len);
			winstr(CONTENT, temp + strlen(temp));
		}
		else
		{
			/* remainder on the right (giving a negative @n to winnstr() family seems broken
			   in my library and may be elsewhere (manpages say it's an extension to XSI curses) */
			mvwinstr(CONTENT, y, getmaxx(CONTENT) - len, temp);
		}
		/* the winnstr() family of functions used to extract shifted characters moves the cursor, we have
		   to reset it here (would have been nice to leave that fact in the manpages...) */
		mvwinsstr(CONTENT, y, x, str);
		strcpy(shifted, temp);
		WINDOW_insert(THIS, shifted, 0, y + 1);
	}

	res = 0;
	_return:
	if (!--rec)
	{
		WINDOW_cursor_move(THIS, ex, ey);
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
static int WINDOW_insert(void *_object, char *str, int x, int y)
{
	int a, i, len, slen;
	chtype *ins, *shifted;

	if (WINDOW_cursor_move(THIS, x, y) == -1)
		return -1;
	if (!THIS->wrap)
	{
		winsstr(CONTENT, str);
		return 0;
	}

	x = getcurx(CONTENT);
	y = getcury(CONTENT);
	XY2A(CONTENT, x, y, a);
	len = strlen(str);
	slen = getmaxy(CONTENT) * getmaxx(CONTENT) - a - len + 1;
	GB.Alloc((void **) &ins, len * sizeof(chtype));
	for (i = 0; i < len; i++)
		ins[i] = (chtype) str[i] | getattrs(CONTENT);
	WINDOW_get_mem(CONTENT, &shifted, x, y, getmaxy(CONTENT) * getmaxx(CONTENT) - a);

	WINDOW_put_mem(ins, CONTENT, x, y, len);
	getyx(CONTENT, y, x);
	WINDOW_put_mem(shifted, CONTENT, x, y, slen);

	/* place the cursor after the last inserted char */
	WINDOW_cursor_move(THIS, x, y);

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
static int WINDOW_get_str(void *_object, int x, int y, unsigned int len, char **ret)
{
	int i;
	chtype *chbuf;
	char *buf;

	if (len == -1)
		len = getmaxx(CONTENT) - getcurx(CONTENT);

	GB.Alloc((void **) &buf, len + 1);
	WINDOW_get_mem(CONTENT, &chbuf, x, y, len);
	for (i = 0; i < len; i++)
		buf[i] = chbuf[i] & A_CHARTEXT;
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
static int WINDOW_key_timeout(void *_object, int timeout, int *ret)
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
		if (timeout >= 0) 
			*ret = 0;
	}

	timeout(-1);
	return 0;
}

/**
 * Flush the input queue
 */
static void WINDOW_flush()
{
	flushinp();
}

/**
 * Move the main panel to the bottom of the panel stack thus arrange it
 * below all other windows that may overlap it
 */
static int WINDOW_bottom(void *_object)
{
	bottom_panel(THIS->pan);
	return 0;
}

/**
 * Move the main panel to the top of the panel stack
 */
static int WINDOW_top(void *_object)
{
	top_panel(THIS->pan);
	return 0;
}

/**
 * Remove the main panel from the panel stack thus hiding it from the user
 */
static int WINDOW_hide(void *_object)
{
	hide_panel(THIS->pan);
	return 0;
}

/**
 * Insert the main panel into the panel stack. note that the panel is moved to the top
 * of the stack thus making it "the most" visible
 */
static int WINDOW_show(void *_object)
{
	show_panel(THIS->pan);
	return 0;
}

/**
 * Callback for data on stdin
 * @fd: stdin means 0
 * @type: having watched stdin for read means GB_WATCH_READ
 * @param: NULL since we don't need it
 * This function raises the Read event. Since only one callback appears to be able to
 * be registered for an fd, there can only be one window to raise this event, i.e.
 * the one being set to "have the focus" via Window.SetFocus()
 */
static void WINDOW_read_callback(int fd, int type, void *param)
{
	GB.Raise(focused, EVENT_Read, 0);
}

/**
 * Bring the given nc_window to focus. That means that this window now raises the
 * Read event if data arrives on stdin. The global variable @focussed is used to
 * keep track of that very window
 */
static int WINDOW_setfocus(void *_object)
{
	if (!focused)
		GB.Watch(0, GB_WATCH_READ, WINDOW_read_callback, 0);
	else
		GB.Unref((void **) &focused);
	focused = THIS;
	GB.Ref((void **) &focused);
	return 0;
}

/**
 * Import a window and fill the @_object with default values (actual object instanciation)
 * @imp: WINDOW to import
 */
static int WINDOW_import(void *_object, WINDOW *imp)
{
	/* Clean up former members */
	if (HAS_BORDER)
		WINDOW_remove_content(THIS);
	if (MAIN)
		WINDOW_remove(THIS);

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
static int WINDOW_attrs_driver(void *_object, int attr, int req)
{
	switch (req)
	{
		case ATTR_DRV_RET:
			return getattrs(CONTENT);
		case ATTR_DRV_ON:
			wattron(CONTENT, attr);
			break;
		case ATTR_DRV_OFF:
			wattroff(CONTENT, attr);
	}
	return 0;
}

/**
 * Return the state of, set or unset a given attribute on a single character
 * @attr: attribute
 * @x: x coordinate
 * @y: y coordinate
 * @req: request to perform
 * As this is the character-based counterpart to WINDOW_attrs_driver, this
 * also handles colors
 */
static int WINDOW_char_attrs_driver(void *_object, int attr, int x, int y, int req)
{
	int ox, oy;
	int res;
	chtype ch;
	unsigned int mask, shift; /* unsigned in case A_COLOR has its MSB set */

	/* I wonder if everybody searched and hacked around to find that color stuff... */
	/* Find the number of bits to right-shift to get the color number from chtype
	   attributes - _portably_ (don't know if it is required, if anyone could point
	   to the magical manpage where all that is written?) */
	mask = ~(int)A_COLOR;
	for (shift = 0; mask & 1; shift++, mask >>= 1);

	getyx(CONTENT, oy, ox);
	WINDOW_cursor_move(THIS, x, y);
	ch = winch(CONTENT);
	switch (req)
	{
		case ATTR_DRV_RET:
			/* A_ATTRIBUTES inherits A_COLOR (at least on my setup... I really
			   hate undocument facts as they may be no facts) */
			res = ch & A_ATTRIBUTES;
			goto _cleanup;
		case ATTR_DRV_ON:
			wchgat(CONTENT, 1, (ch & A_ATTRIBUTES) | attr, (ch & A_COLOR) >> shift, NULL);
			break;
		case ATTR_DRV_OFF:
			wchgat(CONTENT, 1, (ch & A_ATTRIBUTES) & ~attr, (ch & A_COLOR) >> shift, NULL);
	}

	res = 0;
	_cleanup:
	WINDOW_cursor_move(THIS, ox, oy);
	return res;
}

/*
 * Window class
 */

BEGIN_PROPERTY(Window_Attrs)

	RETURN_SELF();

END_PROPERTY

BEGIN_PROPERTY(Window_Background)

	;

END_PROPERTY

BEGIN_PROPERTY(Window_Border)

	bool b;
	int x, y;

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->border);
		return;
	}

	b = VPROP(GB_BOOLEAN);

	if (b == THIS->border)
		return;

	/* FIXME: try Full() and then Border = True */
	if (b)
	{
		WINDOW_resize(THIS, getmaxx(CONTENT) + 2, getmaxy(CONTENT) + 2);
		WINDOW_move(THIS, (x = getbegx(MAIN) - 1) == -1 ? 0 : x,
		                     (y = getbegy(MAIN) - 1) == -1 ? 0 : y);
		WINDOW_add_content(THIS);
		WINDOW_draw_border(THIS, 1);
	}
	else
	{
		/* FIXME: routine logic */
		/* FIXME: try Border = False if the window is Full() */
		WINDOW_draw_border(THIS, 0);
		WINDOW_remove_content(THIS);
		THIS->border = b; /* FIXME: this is important here due to the automatic border-redraw in WINDOW_resize */
		WINDOW_resize(THIS, getmaxx(CONTENT) - 2, getmaxy(CONTENT) - 2);
		WINDOW_move(THIS, (x = getbegx(MAIN) + 1) >= getmaxx(MAIN) ? getmaxx(MAIN) - 1 : x,
		               (y = getbegy(MAIN) + 1) >= getmaxy(MAIN) ? getmaxy(MAIN) - 1 : y);
	}
	THIS->border = b;
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Buffered)

	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->buffered);
		return;
	}
	THIS->buffered = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Window_ClientHeight)

	GB.ReturnInteger(getmaxy(MAIN));

END_PROPERTY

BEGIN_PROPERTY(Window_ClientWidth)

	GB.ReturnInteger(getmaxx(MAIN));

END_PROPERTY

BEGIN_PROPERTY(Window_CursorX)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getcurx(CONTENT));
		return;
	}
	WINDOW_cursor_move(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_CursorY)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getcury(CONTENT));
		return;
	}
	WINDOW_cursor_move(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Foreground)

	;

END_PROPERTY

BEGIN_PROPERTY(Window_Height)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getmaxy(CONTENT));
		return;
	}
	WINDOW_resize(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Wrap)

	/* This property only affects subsequent prints */
	if (READ_PROPERTY)
	{
		GB.ReturnBoolean(THIS->wrap);
		return;
	}
	THIS->wrap = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Window_Width)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getmaxx(CONTENT));
		return;
	}
	WINDOW_resize(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_X)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getbegx(MAIN));
		return;
	}
	WINDOW_move(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Y)

	if (READ_PROPERTY)
	{
		GB.ReturnInteger(getbegy(MAIN));
		return;
	}
	WINDOW_move(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_METHOD(Window_new, GB_INTEGER x; GB_INTEGER y; GB_INTEGER w; GB_INTEGER h)

	WINDOW *new;

	if (!NCURSES_RUNNING)
	{
		GB.Error("Not in NCurses mode");
		return;
	}
	new = newwin(VARG(h), VARG(w), VARG(y), VARG(x));
	WINDOW_import(THIS, new);

	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_free)

	WINDOW_remove(THIS);
	if (focused == THIS)
		GB.Watch(0, GB_WATCH_NONE, NULL, 0);
	/* the REFRESH() makes use of THIS so it would be unwise to call it
	   since we just invalidated it */
	REAL_REFRESH();

END_METHOD

BEGIN_METHOD(Window_get, GB_INTEGER y; GB_INTEGER x)

	THIS->pos.line = VARG(y);
	THIS->pos.col = VARG(x);
	RETURN_SELF();

END_METHOD

BEGIN_METHOD(Window_Ask, GB_STRING opts; GB_INTEGER tries)

	bool miss;
	int t, ch;
	char *o, c[2];

	miss = MISSING(tries);
	if (!miss)
		t = VARG(tries);
	else
		t = -1; /* just to silence the compiler */

	o = STRING(opts);

	while (miss || t--)
	{
		ch = getch();
		/* Per convention, only dealing with byte chars */
		if (ch > 255)
			continue;
		*c = (char) ch;
		if (strchr(o, *c))
		{
			c[1] = 0;
			GB.ReturnNewZeroString(c);
			return;
		}
	}
	GB.ReturnNull();

END_METHOD

BEGIN_METHOD_VOID(Window_Bottom)

	WINDOW_bottom(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Cls)

	/* ncurses sets the cursor to 0,0 after wclear() which may or may not be
	   surprising. */
	wclear(CONTENT);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Flush)

	WINDOW_flush();

END_METHOD

BEGIN_METHOD_VOID(Window_Full)

	WINDOW_move(THIS, 0, 0);
	if (HAS_BORDER)
		WINDOW_resize(THIS, COLS - 2, LINES - 2);
	else
		WINDOW_resize(THIS, COLS, LINES);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_Get, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len)

	int l;
	char *ret;

	/* if no @len is given, we return until the end of the line */
	if (MISSING(len))
		l = -1;
	else
		l = VARG(len);
	WINDOW_get_str(THIS, VARG(x), VARG(y), l, &ret);
	GB.ReturnNewZeroString(ret);
	GB.Free((void **) &ret);

END_METHOD

BEGIN_METHOD_VOID(Window_Hide)

	WINDOW_hide(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_DrawHLine, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len; GB_STRING ch; GB_INTEGER thickness)

	int ox, oy, gx, gy;
	char c;
	int length, t;
	int i;

	getyx(CONTENT, oy, ox);
	c = *(STRING(ch));
	length = VARG(len);
	if (MISSING(thickness))
		t = 1;
	else
		t = VARG(thickness);

	gx = VARG(x);
	gy = VARG(y);
	for (i = 0; i < t; i++)
	{
		WINDOW_cursor_move(THIS, gx, gy);
		whline(CONTENT, c, length);
		gy++;
	}
	WINDOW_cursor_move(THIS, ox, oy);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_Insert, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	WINDOW_insert(THIS, STRING(text), MISSING(x) ? -1 : VARG(x), MISSING(y) ? -1 : VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_Locate, GB_INTEGER x; GB_INTEGER y)

	WINDOW_cursor_move(THIS, VARG(x), VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_Move, GB_INTEGER x; GB_INTEGER y)

	WINDOW_move(THIS, MISSING(x) ? -1 : VARG(x), MISSING(y) ? -1 : VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_Print, GB_STRING text; GB_INTEGER x; GB_INTEGER y)

	WINDOW_print(THIS, STRING(text), MISSING(x) ? -1 : VARG(x), MISSING(y) ? -1 : VARG(y));
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_PrintCenter, GB_STRING text)

	int lines = 1;
	int x, y;
	char *p, *q;

	p = STRING(text);
	while ((q = strchr(p, '\n')))
	{
		lines++;
		p = q + 1;
	}

	p = STRING(text);
	y = (getmaxy(CONTENT) - lines) / 2;
	while (lines--)
	{
		if (!lines)
		{
			x = (getmaxx(CONTENT) - strlen(p)) / 2;
			WINDOW_print(THIS, p, x, y);
		}
		else
		{
			q = strchr(p, '\n');
			*q = 0;
			x = (getmaxx(CONTENT) - (q - p)) / 2;
			WINDOW_print(THIS, p, x, y);
			y++;
			p = q + 1;
			*q = '\n';
		}
	}
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Refresh)

	REAL_REFRESH();

END_METHOD

BEGIN_METHOD(Window_Resize, GB_INTEGER w; GB_INTEGER h)

	WINDOW_resize(THIS, MISSING(w) ? -1 : VARG(w), MISSING(h) ? -1 : VARG(h));
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Show)

	WINDOW_show(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_SetFocus)

	WINDOW_setfocus(THIS);

END_METHOD

BEGIN_METHOD_VOID(Window_Top)

	WINDOW_top(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_DrawVLine, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len; GB_STRING ch; GB_INTEGER thickness)

	int ox, oy, gx, gy;
	char c;
	int length, t;
	int i;

	getyx(CONTENT, oy, ox);
	c = *(STRING(ch));
	length = VARG(len);
	if (MISSING(thickness))
		t = 1;
	else
		t = VARG(thickness);

	gx = VARG(x);
	gy = VARG(y);
	for (i = 0; i < t; i++)
	{
		WINDOW_cursor_move(THIS, gx, gy);
		wvline(CONTENT, c, length);
		gx++;
	}
	WINDOW_cursor_move(THIS, ox, oy);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_WaitKey, GB_INTEGER timeout)

	int t;
	int ret;

	if (MISSING(timeout))
		t = -1;
	else
		t = VARG(timeout);

	WINDOW_key_timeout(THIS, t, &ret);
	GB.ReturnInteger(ret);

END_METHOD

/*
 * .Window.Attrs virtual class
 */

BEGIN_PROPERTY(WindowAttrs_Normal)

	/* normal is special because it turns off all other attributes and can't itself been turned off */
	if (READ_PROPERTY)
		GB.ReturnBoolean(getattrs(CONTENT) == A_NORMAL);
	if (VPROP(GB_BOOLEAN))
		wattrset(CONTENT, A_NORMAL);

END_PROPERTY

BEGIN_PROPERTY(WindowAttrs_Underline)

	WIN_ATTR_METHOD_BOOL(A_UNDERLINE);

END_PROPERTY

BEGIN_PROPERTY(WindowAttrs_Reverse)

	WIN_ATTR_METHOD_BOOL(A_REVERSE);

END_PROPERTY

BEGIN_PROPERTY(WindowAttrs_Blink)

	WIN_ATTR_METHOD_BOOL(A_BLINK);

END_PROPERTY

BEGIN_PROPERTY(WindowAttrs_Bold)

	WIN_ATTR_METHOD_BOOL(A_BOLD);

END_PROPERTY

BEGIN_PROPERTY(WindowAttrs_Color)

	//inch() attributes have the pair number, but we give the color pair to attron()?
	//WIN_COLOR_METHOD();

END_PROPERTY

/*
 * .Char.Attrs virtual class
 */

BEGIN_PROPERTY(CharAttrs_Normal)

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Underline)

	CHAR_ATTR_METHOD_BOOL(A_UNDERLINE);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Reverse)

	CHAR_ATTR_METHOD_BOOL(A_REVERSE);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Blink)

	CHAR_ATTR_METHOD_BOOL(A_BLINK);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Bold)

	CHAR_ATTR_METHOD_BOOL(A_BOLD);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Color)

END_PROPERTY

#define TIMEOUT_NOTIMEOUT	-1

GB_DESC CWindowDesc[] =
{
	GB_DECLARE("Window", sizeof(CWINDOW)),

	GB_EVENT("Read", NULL, NULL, &EVENT_Read),

	/* Constants */
	GB_CONSTANT("NoTimeout", "i", TIMEOUT_NOTIMEOUT),

	/* Properties */
	GB_PROPERTY_READ("Attributes", ".Window.Attrs", Window_Attrs),

	GB_PROPERTY("Background", "i", Window_Background),
	GB_PROPERTY("Paper", "i", Window_Background),

	GB_PROPERTY("Border", "b", Window_Border),

	GB_PROPERTY("Buffered", "b", Window_Buffered),

	GB_PROPERTY_READ("ClientHeight", "i", Window_ClientHeight),
	GB_PROPERTY_READ("ClientH", "i", Window_ClientHeight),
	GB_PROPERTY_READ("ClientWidth", "i", Window_ClientWidth),
	GB_PROPERTY_READ("ClientW", "i", Window_ClientWidth),

	GB_PROPERTY("CursorX", "i", Window_CursorX),
	GB_PROPERTY("CursorY", "i", Window_CursorY),

	GB_PROPERTY("Foreground", "i", Window_Foreground),
	GB_PROPERTY("Pen", "i", Window_Foreground),

	GB_PROPERTY("Height", "i", Window_Height),
	GB_PROPERTY("H", "i", Window_Height),

	GB_PROPERTY("Wrap", "b", Window_Wrap),

	GB_PROPERTY("Width", "i", Window_Width),
	GB_PROPERTY("W", "i", Window_Width),

	GB_PROPERTY("X", "i", Window_X),
	GB_PROPERTY("Y", "i", Window_Y),

	/* Methods */
	GB_METHOD("_new", NULL, Window_new, "(X)i(Y)i(W)i(H)i"),
	GB_METHOD("_free", NULL, Window_free, NULL),
	GB_METHOD("_get", ".Char.Attrs", Window_get, "(Y)i(X)i"),

	GB_METHOD("Ask", "s", Window_Ask, "(Opts)s[(Tries)i]"),

	GB_METHOD("Bottom", NULL, Window_Bottom, NULL),
	GB_METHOD("Top", NULL, Window_Top, NULL),

	GB_METHOD("Cls", NULL, Window_Cls, NULL),

	GB_METHOD("Flush", NULL, Window_Flush, NULL),

	GB_METHOD("Full", NULL, Window_Full, NULL),

	GB_METHOD("Get", "s", Window_Get, "(X)i(Y)i[(Len)i]"),

	GB_METHOD("Hide", NULL, Window_Hide, NULL),
	GB_METHOD("Show", NULL, Window_Show, NULL),

	GB_METHOD("DrawHLine", NULL, Window_DrawHLine, "(X)i(Y)i(Len)i(Ch)s[(Thickness)i]"),
	GB_METHOD("DrawVLine", NULL, Window_DrawVLine, "(X)i(Y)i(Len)i(Ch)s[(Thickness)i]"),

	GB_METHOD("Insert", NULL, Window_Insert, "(Text)s[(X)i(Y)i]"),
	GB_METHOD("Print", NULL, Window_Print, "(Text)s[(X)i(Y)i]"),
	GB_METHOD("PrintCenter", NULL, Window_PrintCenter, "(Text)s"),

	GB_METHOD("Locate", NULL, Window_Locate, "(X)i(Y)i"),
	GB_METHOD("Move", NULL, Window_Move, "[(X)i(Y)i]"),
	GB_METHOD("Resize", NULL, Window_Resize, "[(W)i(H)i]"),

	GB_METHOD("Refresh", NULL, Window_Refresh, NULL),

	GB_METHOD("SetFocus", NULL, Window_SetFocus, NULL),

	GB_METHOD("WaitKey", "i", Window_WaitKey, "[(Timeout)i]"),

	GB_END_DECLARE
};

GB_DESC CWindowAttrsDesc[] =
{
	GB_DECLARE(".Window.Attrs", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Normal", "b", WindowAttrs_Normal),
	GB_PROPERTY("Underline", "b", WindowAttrs_Underline),
	GB_PROPERTY("Reverse", "b", WindowAttrs_Reverse),
	GB_PROPERTY("Blink", "b", WindowAttrs_Blink),
	GB_PROPERTY("Bold", "b", WindowAttrs_Bold),

	GB_END_DECLARE
};

GB_DESC CCharAttrsDesc[] =
{
	GB_DECLARE(".Char.Attrs", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Normal", "b", CharAttrs_Normal),
	GB_PROPERTY("Underline", "b", CharAttrs_Underline),
	GB_PROPERTY("Reverse", "b", CharAttrs_Reverse),
	GB_PROPERTY("Blink", "b", CharAttrs_Blink),
	GB_PROPERTY("Bold", "b", CharAttrs_Bold),

	//GB_PROPERTY("Color", "i", CCharAttrs_color),

	GB_END_DECLARE
};

GB_DESC CScreenDesc[] = 
{
	GB_DECLARE("Screen", 0), GB_VIRTUAL_CLASS(),
	
	GB_EVENT("Read", NULL, NULL, &EVENT_Read),

	/* Constants */
	//GB_CONSTANT("NoTimeout", "i", TIMEOUT_NOTIMEOUT),

	/* Properties */
	//GB_PROPERTY_READ("Attributes", ".Window.Attrs", Window_Attrs),

	GB_STATIC_PROPERTY("Background", "i", Window_Background),
	GB_STATIC_PROPERTY("Paper", "i", Window_Background),

	//GB_STATIC_PROPERTY("Border", "b", Window_Border),

	//GB_STATIC_PROPERTY("Buffered", "b", Window_Buffered),

	GB_STATIC_PROPERTY_READ("ClientHeight", "i", Window_ClientHeight),
	GB_STATIC_PROPERTY_READ("ClientH", "i", Window_ClientHeight),
	GB_STATIC_PROPERTY_READ("ClientWidth", "i", Window_ClientWidth),
	GB_STATIC_PROPERTY_READ("ClientW", "i", Window_ClientWidth),

	GB_STATIC_PROPERTY("CursorX", "i", Window_CursorX),
	GB_STATIC_PROPERTY("CursorY", "i", Window_CursorY),

	GB_STATIC_PROPERTY("Foreground", "i", Window_Foreground),
	GB_STATIC_PROPERTY("Pen", "i", Window_Foreground),

	GB_STATIC_PROPERTY("Height", "i", Window_Height),
	GB_STATIC_PROPERTY("H", "i", Window_Height),

	//GB_STATIC_PROPERTY("Wrap", "b", Window_Wrap),

	GB_STATIC_PROPERTY("Width", "i", Window_Width),
	GB_STATIC_PROPERTY("W", "i", Window_Width),

	GB_STATIC_PROPERTY("X", "i", Window_X),
	GB_STATIC_PROPERTY("Y", "i", Window_Y),

	/* Methods */
	//GB_STATIC_METHOD("_get", ".Char.Attrs", Window_get, "(Y)i(X)i"),

	GB_STATIC_METHOD("Ask", "s", Window_Ask, "(Opts)s[(Tries)i]"),

	GB_STATIC_METHOD("Bottom", NULL, Window_Bottom, NULL),
	GB_STATIC_METHOD("Top", NULL, Window_Top, NULL),

	GB_STATIC_METHOD("Cls", NULL, Window_Cls, NULL),

	GB_STATIC_METHOD("Flush", NULL, Window_Flush, NULL),

	GB_STATIC_METHOD("Get", "s", Window_Get, "(X)i(Y)i[(Len)i]"),

	GB_STATIC_METHOD("DrawHLine", NULL, Window_DrawHLine, "(X)i(Y)i(Len)i(Ch)s[(Thickness)i]"),
	GB_STATIC_METHOD("DrawVLine", NULL, Window_DrawVLine, "(X)i(Y)i(Len)i(Ch)s[(Thickness)i]"),

	GB_STATIC_METHOD("Insert", NULL, Window_Insert, "(Text)s[(X)i(Y)i]"),
	GB_STATIC_METHOD("Print", NULL, Window_Print, "(Text)s[(X)i(Y)i]"),
	GB_STATIC_METHOD("PrintCenter", NULL, Window_PrintCenter, "(Text)s"),

	GB_STATIC_METHOD("Locate", NULL, Window_Locate, "(X)i(Y)i"),

	GB_STATIC_METHOD("Refresh", NULL, Window_Refresh, NULL),

	GB_STATIC_METHOD("SetFocus", NULL, Window_SetFocus, NULL),

	GB_STATIC_METHOD("WaitKey", "i", Window_WaitKey, "[(Timeout)i]"),
	
	GB_END_DECLARE
};
