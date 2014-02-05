/*
 * c_window.c - gb.ncurses Window class
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

#define __C_WINDOW_C

#include <stdlib.h>
#include <string.h>

#include <ncurses.h>
#include <panel.h>

#include "gambas.h"

#include "main.h"
#include "c_window.h"
#include "c_screen.h"
#include "c_color.h"

#define THIS				((CWINDOW *) _object)

#define MAKE_THING(snip, win, a, b)	\
do {					\
	if ((a) == -1)			\
		a = get##snip##x(win);	\
	if ((b) == -1)			\
		b = get##snip##y(win);	\
} while (0)

#define MAKE_CURSOR(win, x, y)	MAKE_THING(cur, win, x, y)
#define MAKE_COORDS(win, x, y)	MAKE_THING(beg, win, x, y)
#define MAKE_DIM(win, x, y)	MAKE_THING(max, win, w, h)

#define CHECK_RAISE_THING(snip, win, a, b)	\
if ((a) < 0 || (a) > get##snip##x(win)		\
    || (b) < 0 || (b) > get##snip##y(win)) {	 /* >= ? */ \
	GB.Error(GB_ERR_BOUND);			\
	return;					\
}

#define CHECK_RAISE_COORDS(win, x, y)	CHECK_RAISE_THING(max, win, x, y)
#define CHECK_RAISE_DIM(win, x, y)	CHECK_RAISE_THING(max, win, x, y)

static CWINDOW *_curwin = NULL;

DECLARE_EVENT(EVENT_Read);

/* This macro is mostly called by Gambas implementation functions to request
 * output on screen (read: to check if the output is buffered and if not
 * produce output by means of REAL_REFRESH()) */
#define REFRESH()			CWINDOW_refresh(THIS)

static void CWINDOW_refresh(CWINDOW *win)
{
	if (win->buffered)
		return;
	REAL_REFRESH();
}

void CWINDOW_raise_read(CWINDOW *win)
{
	if (!win) {
		if (!_curwin)
			return;
		win = _curwin;
	}

	if (GB.CanRaise(win, EVENT_Read))
		GB.Raise(win, EVENT_Read, 0);
}

static void CWINDOW_from_ncurses(CWINDOW *win, WINDOW *ncwin, bool border)
{
	win->main = ncwin;
	win->pan = new_panel(ncwin);
	keypad(ncwin, TRUE);
	win->has_border = border;
	win->border = 0;
	win->buffered = 0;
	win->wrap = 1;
	if (border) {
		WINDOW *new = derwin(ncwin, getmaxy(ncwin) - 2,
					    getmaxx(ncwin) - 2,
					    1, 1);
		syncok(new, TRUE);
		win->content = new;
	} else {
		win->content = win->main;
	}
	win->caption = NULL;
}

BEGIN_METHOD(Window_new, GB_BOOLEAN out_border; GB_INTEGER x; GB_INTEGER y;
			 GB_INTEGER w; GB_INTEGER h)

	WINDOW *new;
	int w, h;

	w = VARGOPT(w, COLS);
	h = VARGOPT(h, LINES);
	if (VARGOPT(out_border, 1)) {
		w = MIN(w + 2, COLS);
		h = MIN(h + 2, LINES);
	}
	new = newwin(h, w, VARGOPT(y, 0), VARGOPT(x, 0));
	CWINDOW_from_ncurses(THIS, new, VARGOPT(out_border, 1));
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_free)

	if (_curwin == THIS) {
		_curwin = NULL;
		INPUT_exit();
	}
	del_panel(THIS->pan);
	if (THIS->has_border)
		delwin(THIS->content);
	delwin(THIS->main);
	if (THIS->caption)
		GB.FreeString(&THIS->caption);
	REAL_REFRESH();

END_METHOD

BEGIN_METHOD(Window_get, GB_INTEGER y; GB_INTEGER x)

	THIS->pos.line = VARG(y);
	THIS->pos.col = VARG(x);
	RETURN_SELF();

END_METHOD

/**G
 * Wait until the user typed one of the characters listed in Opts or until
 * they typed wrong Tries times (infinity if not given).
 *
 * If there is an uppercase letter in the Opts string and the user hits
 * Return, the first uppercase letter found in Opts is taken as the default
 * value and the function returns. The case of letters does not matter in
 * any other regard (this function is case-insensitive).
 *
 * This function returns the typed character (always in lowercase!).
 **/
BEGIN_METHOD(Window_Ask, GB_STRING opts; GB_INTEGER tries)

	int t, ch, i;
	char *o, c;

	t = VARGOPT(tries, -1);
	o = STRING(opts);

	while (t--) {
		ch = INPUT_get(TIMEOUT_NOTIMEOUT);
		/* XXX: not meant for ncurses special keys */
		if (ch > 127)
			continue;
		if (ch == '\n')
			for (i = 0; i < LENGTH(opts); i++)
				if (isupper(o[i]))
					goto found;
		for (i = 0; i < LENGTH(opts); i++)
			if (tolower(o[i]) == (char) ch)
				goto found;
	}
	GB.ReturnNull();
	return;

found:
	c = tolower(o[i]);
	GB.ReturnNewString(&c, 1);

END_METHOD

BEGIN_METHOD_VOID(Window_Lower)

	bottom_panel(THIS->pan);
	REFRESH();

END_METHOD

static void CWINDOW_clear(CWINDOW *win)
{
	werase(win->content);
}

BEGIN_METHOD_VOID(Window_Clear)

	CWINDOW_clear(THIS);
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Drain)

	INPUT_drain();

END_METHOD

BEGIN_METHOD(Window_DrawHLine, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len;
                               GB_STRING ch)

	mvwhline(THIS->content, VARG(y), VARG(x), *STRING(ch), VARG(len));
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_DrawVLine, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len;
                               GB_STRING ch)

	mvwvline(THIS->content, VARG(y), VARG(x), *STRING(ch), VARG(len));
	REFRESH();

END_METHOD

static void CWINDOW_get(CWINDOW *win, int x, int y, unsigned int len,
			char **ret)
{
	char *buf;

	MAKE_COORDS(win->content, x, y);
	CHECK_RAISE_COORDS(win->content, x, y);
	if (len == -1)
		len = getmaxx(win->content) - getcurx(win->content);
	len = MIN(len, (getmaxy(win->content) - getcury(win->content)) *
	               getmaxx(win->content) - getcurx(win->content) - 1);

	GB.Alloc((void **) &buf, len + 1);
	len = mvwinnstr(win->content, y, x, buf, len);
	if (len != ERR)
		buf[len] = 0;
	else
		GB.Free((void **) &buf);
	*ret = buf;
}

BEGIN_METHOD(Window_Get, GB_INTEGER x; GB_INTEGER y; GB_INTEGER len)

	int len;
	char *ret;

	len = VARGOPT(len, -1);
	CWINDOW_get(THIS, VARG(x), VARG(y), len, &ret);
	GB.ReturnNewZeroString(ret);
	GB.Free((void **) &ret);

END_METHOD

BEGIN_METHOD_VOID(Window_Hide)

	hide_panel(THIS->pan);
	REFRESH();

END_METHOD

static void CWINDOW_locate(CWINDOW *win, int x, int y)
{
	MAKE_CURSOR(win->content, x, y);
	CHECK_RAISE_COORDS(win->content, x, y);
	wmove(win->content, y, x);
}

BEGIN_METHOD(Window_Locate, GB_INTEGER x; GB_INTEGER y)

	CWINDOW_locate(THIS, VARG(x), VARG(y));
	REFRESH();

END_METHOD

static void CWINDOW_move(CWINDOW *win, int x, int y)
{
	MAKE_COORDS(win->main, x, y);
	CHECK_RAISE_COORDS(stdscr, x, y);

	move_panel(win->pan, y, x);
#if 0
	if (HAS_BORDER) {
		mvwin(THIS->content, y + 1, x + 1);
		WINDOW_draw_border(THIS, 1);
	}
#endif
}

BEGIN_METHOD(Window_Move, GB_INTEGER x; GB_INTEGER y)

	CWINDOW_move(THIS, VARGOPT(x, -1), VARGOPT(y, -1));
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Center)

	int x, y;

	x = (COLS - getmaxx(THIS->main)) / 2;
	y = (LINES - getmaxy(THIS->main)) / 2;
	CWINDOW_move(THIS, x, y);
	REFRESH();

END_METHOD

static void CWINDOW_print(CWINDOW *win, char *str, int x, int y,
			  attr_t attr, int pair)
{
	int width;
	char *p, *q;
	attr_t asave; short psave;

	wattr_get(win->content, &asave, &psave, NULL);
	if (attr == -1)
		attr = asave;
	if (pair == -1)
		pair = psave;
	wattr_set(win->content, attr, pair, NULL);

	p = str;
	do {
		CWINDOW_locate(win, x, y);
		width = strlen(p);
		if (!win->wrap)
			width = MIN(width, getmaxx(win->content) - x);

		/* waddnstr, being subsequent calls to waddch, rests at the
		 * end of the current line but we want to go to the next
		 * one. */
		width = MIN(width, getmaxx(win->content) *
			(getmaxy(win->content) - y) - x);
		if ((q = strchr(p, '\n')))
			width = MIN(width, q - p);
		waddnstr(win->content, p, width);
		p += width;
		x = getcurx(win->content);
		y = getcury(win->content);
		if (y == getmaxy(win->content) - 1)
			break;
		if (*p == '\n') {
			y++;
			p++;
		}
		if (*p)
			x = 0;
	} while (*p);
	CWINDOW_locate(win, x, y);
	wattr_set(win->content, asave, psave, NULL);
}

BEGIN_METHOD(Window_Print, GB_STRING text; GB_INTEGER x; GB_INTEGER y;
			   GB_INTEGER attr; GB_INTEGER pair)

	char text[LENGTH(text) + 1];

	strncpy(text, STRING(text), LENGTH(text));
	text[LENGTH(text)] = 0;
	CWINDOW_print(THIS, text, VARGOPT(x, -1), VARGOPT(y, -1),
		      VARGOPT(attr, -1), VARGOPT(pair, -1));
	REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_Raise)

	top_panel(THIS->pan);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_PrintCenter, GB_STRING text; GB_INTEGER attr;
				 GB_INTEGER pair)

	int lines = 1;
	int x, y;
	char text[LENGTH(text) + 1];
	char *p, *q;
	attr_t attr = VARGOPT(attr, -1);
	short pair = VARGOPT(pair, -1);

	memcpy(text, STRING(text), LENGTH(text));
	text[LENGTH(text)] = 0;
	p = text;
	while ((q = strchr(p, '\n'))) {
		lines++;
		p = q + 1;
	}

	p = text;
	y = (getmaxy(THIS->content) - lines) / 2;
	while (lines--) {
		if (!lines) {
			x = (getmaxx(THIS->content) - strlen(p)) / 2;
			CWINDOW_print(THIS, p, x, y, attr, pair);
		} else {
			q = strchr(p, '\n');
			if (q == p + 1) {
				y++;
				continue;
			}
			*q = 0;
			x = (getmaxx(THIS->content) - (q - p)) / 2;
			CWINDOW_print(THIS, p, x, y, attr, pair);
			y++;
			p = q + 1;
			*q = '\n';
		}
	}
	REFRESH();

END_METHOD

static void CWINDOW_draw_border(CWINDOW *win)
{
	switch (win->border) {
	case BORDER_NONE:
		wborder(win->main, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		break;
	case BORDER_ASCII:
		wborder(win->main, '|', '|', '-', '-', '+', '+', '+', '+');
		break;
	case BORDER_ACS:
		box(win->main, 0, 0);
		break;
	}
	if (win->border != BORDER_NONE && win->caption) {
		int width = getmaxx(win->main) - 2;

		width = MIN(width, strlen(win->caption));
		mvwaddnstr(win->main, 0, 1, win->caption, width);
	}
}

static void CWINDOW_resize(CWINDOW *win, int w, int h)
{
	int x, y;

	MAKE_DIM(win->main, w, h);

	getbegyx(win->main, y, x);
	if (win->has_border) {
		w += 2;
		h += 2;
	}
	/* XXX: Not rather raise an error? */
	w = MIN(w, COLS - x);
	h = MIN(h, LINES - y);

	/* TODO: With the auto-created Window it does not work properly in
	 *       Screen_Resize() from within xterm (my testcase) */
	if (win->border)
		wborder(win->main, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wresize(win->main, h, w);
	if (win->has_border)
		wresize(win->content, h - 2, w - 2);
	replace_panel(win->pan, win->main);
	CWINDOW_draw_border(win);
}

BEGIN_METHOD(Window_Resize, GB_INTEGER w; GB_INTEGER h)

	CWINDOW_resize(THIS, VARGOPT(w, -1), VARGOPT(h, -1));
	REAL_REFRESH();

END_METHOD

BEGIN_METHOD_VOID(Window_SetFullscreen)

	CWINDOW_move(THIS, 0, 0);
	CWINDOW_resize(THIS, COLS, LINES);
	REFRESH();

END_METHOD

BEGIN_METHOD(Window_Read, GB_INTEGER timeout)

	int t;
	int ret;

	t = VARGOPT(timeout, -1);

	ret = INPUT_get(t);
	GB.ReturnInteger(ret);

END_METHOD

BEGIN_METHOD_VOID(Window_ReadLine)

	char line[256]; /* XXX: Must be enough! */
	int len;

	len = getnstr(line, sizeof(line) - 1);
	if (len == ERR) {
		GB.ReturnNull();
		return;
	}
	line[len] = 0;
	GB.ReturnNewZeroString(line);

END_METHOD

BEGIN_METHOD_VOID(Window_Show)

	show_panel(THIS->pan);
	REFRESH();

END_METHOD

static void CWINDOW_setfocus(CWINDOW *win)
{
	if (!_curwin)
		INPUT_init();
	_curwin = win;
}

BEGIN_METHOD_VOID(Window_SetFocus)

	CWINDOW_setfocus(THIS);

END_METHOD

BEGIN_PROPERTY(Window_Attributes)

	if (READ_PROPERTY) {
		attr_t attr;
		short pair;

		wattr_get(THIS->content, &attr, &pair, NULL);
		GB.ReturnInteger(attr);
		return;
	}
	wattrset(THIS->content, VPROP(GB_INTEGER));

END_PROPERTY

/* This is *not* a shortcut to setting Window.{Fore,Back}ground. The latter
 * two set the colour of everything in the window. This function only sets
 * the colour pair for all subsequent writes! */
BEGIN_PROPERTY(Window_Pair)

	chtype bkgd;

	bkgd = getbkgd(THIS->content);

	if (READ_PROPERTY) {
		GB.ReturnInteger(PAIR_NUMBER(bkgd & A_COLOR));
		return;
	}
	bkgd = COLOR_PAIR(VPROP(GB_INTEGER));
	wbkgdset(THIS->content, bkgd);

END_PROPERTY

#define COLOR_METHOD(r, f, b)				\
short pair, fg, bg;					\
attr_t attr;						\
							\
wattr_get(THIS->content, &attr, &pair, NULL);		\
pair_content(pair, &fg, &bg);				\
if (READ_PROPERTY) {					\
	GB.ReturnInteger(r);				\
	return;						\
}							\
pair = CPAIR_get(f, b);					\
if (pair == -1) {					\
	GB.Error(GB_ERR_BOUND);				\
	return;						\
}							\
wbkgd(THIS->content, COLOR_PAIR(pair) | attr | ' ');	\
REFRESH();

/*
 * One should be careful when using these two properties. wbkgd() changes
 * fore- *and* background of *all* characters in the window. So setting
 * the background may result in resetting even the foreground of differently
 * coloured characters.
 *
 * Best is to only set Background/Foreground once at the beginning of the
 * program when using colours extensively during the program.
 */

BEGIN_PROPERTY(Window_Background)

	COLOR_METHOD(bg, fg, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(Window_Foreground)

	COLOR_METHOD(fg, VPROP(GB_INTEGER), bg);

END_PROPERTY

BEGIN_PROPERTY(Window_Border)

	if (READ_PROPERTY) {
		GB.ReturnInteger(THIS->border);
		return;
	}

	THIS->border = VPROP(GB_INTEGER);
	CWINDOW_draw_border(THIS);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Buffered)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(THIS->buffered);
		return;
	}
	THIS->buffered = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Window_Caption)

	if (READ_PROPERTY) {
		GB.ReturnString(THIS->caption);
		return;
	}

	if (THIS->caption)
		GB.FreeString(&THIS->caption);
	THIS->caption = GB.NewZeroString(PSTRING());
	CWINDOW_draw_border(THIS);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_CursorX)

	if (READ_PROPERTY) {
		GB.ReturnInteger(getcurx(THIS->content));
		return;
	}
	CWINDOW_locate(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_CursorY)

	if (READ_PROPERTY) {
		GB.ReturnInteger(getcury(THIS->content));
		return;
	}
	CWINDOW_locate(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Height)

	if (READ_PROPERTY) {
		GB.ReturnInteger(getmaxy(THIS->content));
		return;
	}
	CWINDOW_resize(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Wrap)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(THIS->wrap);
		return;
	}
	THIS->wrap = VPROP(GB_BOOLEAN);

END_PROPERTY

BEGIN_PROPERTY(Window_Width)

	if (READ_PROPERTY) {
		GB.ReturnInteger(getmaxx(THIS->content));
		return;
	}
	CWINDOW_resize(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_X)

	if (READ_PROPERTY) {
		GB.ReturnInteger(getbegx(THIS->main));
		return;
	}
	CWINDOW_move(THIS, VPROP(GB_INTEGER), -1);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(Window_Y)

	if (READ_PROPERTY) {
		GB.ReturnInteger(getbegy(THIS->main));
		return;
	}
	CWINDOW_move(THIS, -1, VPROP(GB_INTEGER));
	REFRESH();

END_PROPERTY

GB_DESC CWindowDesc[] = {
	GB_DECLARE("Window", sizeof(CWINDOW)),
	GB_AUTO_CREATABLE(),

	GB_EVENT("Read", NULL, NULL, &EVENT_Read),

	/* Methods */
	GB_METHOD("_new", NULL, Window_new, "[(BorderFrame)b(X)i(Y)i(W)i(H)i]"),
	GB_METHOD("_free", NULL, Window_free, NULL),
	GB_METHOD("_get", ".Char.Attrs", Window_get, "(Y)i(X)i"),

	GB_METHOD("Ask", "s", Window_Ask, "(Opts)s[(Tries)i]"),
	GB_METHOD("Read", "i", Window_Read, "[(Timeout)i]"),
	GB_METHOD("ReadLine", "s", Window_ReadLine, NULL),
	GB_METHOD("Drain", NULL, Window_Drain, NULL),

	GB_METHOD("Get", "s", Window_Get, "[(X)i(Y)i(Len)i]"),

	GB_METHOD("Clear", NULL, Window_Clear, NULL),
	GB_METHOD("Cls", NULL, Window_Clear, NULL),

	GB_METHOD("Lower", NULL, Window_Lower, NULL),
	GB_METHOD("Raise", NULL, Window_Raise, NULL),
	GB_METHOD("Hide", NULL, Window_Hide, NULL),
	GB_METHOD("Show", NULL, Window_Show, NULL),

	GB_METHOD("DrawHLine", NULL, Window_DrawHLine,"(X)i(Y)i(Len)i(C)s"),
	GB_METHOD("DrawVLine", NULL, Window_DrawVLine,"(X)i(Y)i(Len)i(C)s"),
	GB_METHOD("Print", NULL, Window_Print, "(Text)s[(X)i(Y)i(Attr)i(Pair)i]"),
	GB_METHOD("PrintCenter", NULL, Window_PrintCenter, "(Text)s[(Attr)i(Pair)i]"),

	GB_METHOD("Locate", NULL, Window_Locate, "(X)i(Y)i"),

	GB_METHOD("Move", NULL, Window_Move, "[(X)i(Y)i]"),
	GB_METHOD("Center", NULL, Window_Center, NULL),
	GB_METHOD("Resize", NULL, Window_Resize, "[(W)i(H)i]"),
	GB_METHOD("SetFullscreen", NULL, Window_SetFullscreen, NULL),

	GB_METHOD("SetFocus", NULL, Window_SetFocus, NULL),

	GB_PROPERTY("Attributes", "i", Window_Attributes),

	GB_PROPERTY("Background", "i", Window_Background),
	GB_PROPERTY("Paper", "i", Window_Background),
	GB_PROPERTY("Foreground", "i", Window_Foreground),
	GB_PROPERTY("Pen", "i", Window_Foreground),
	GB_PROPERTY("Pair", "i", Window_Pair),

	GB_PROPERTY("Border", "i", Window_Border),
	GB_PROPERTY("Buffered", "b", Window_Buffered),
	GB_PROPERTY("Caption", "s", Window_Caption),

	GB_PROPERTY("CursorX", "i", Window_CursorX),
	GB_PROPERTY("CursorY", "i", Window_CursorY),

	/* GB_PROPERTY("ClientHeight", ...), etc */
	GB_PROPERTY("Height", "i", Window_Height),
	GB_PROPERTY("H", "i", Window_Height),
	GB_PROPERTY("Width", "i", Window_Width),
	GB_PROPERTY("W", "i", Window_Width),

	GB_PROPERTY("Wrap", "b", Window_Wrap),

	GB_PROPERTY("X", "i", Window_X),
	GB_PROPERTY("Y", "i", Window_Y),

	GB_END_DECLARE
};

/*
 * Attrs
 */

GB_DESC CWindowAttrsDesc[] = {
	GB_DECLARE("Attr", 0),
	GB_NOT_CREATABLE(),

	GB_CONSTANT("Normal", "i", A_NORMAL),
	GB_CONSTANT("Underline", "i", A_UNDERLINE),
	GB_CONSTANT("Reverse", "i", A_REVERSE),
	GB_CONSTANT("Blink", "i", A_BLINK),
	GB_CONSTANT("Bold", "i", A_BOLD),

	GB_END_DECLARE
};

/*
 * .Char.Attrs
 */

/* Seemingly, chgat() doesn't mark the window dirty. We use wtouchln() and
 * wsyncup(). */
#define CHAR_ATTR_METHOD(a)					\
int ox, oy;							\
chtype ch;							\
								\
getyx(THIS->content, oy, ox);					\
ch = mvwinch(THIS->content, THIS->pos.line, THIS->pos.col);	\
if (READ_PROPERTY) {						\
	GB.ReturnBoolean(ch & a);				\
	return;							\
}								\
if (VPROP(GB_BOOLEAN))						\
	wchgat(THIS->content, 1, (ch & A_ATTRIBUTES) | a,	\
		PAIR_NUMBER(ch), NULL);				\
else								\
	wchgat(THIS->content, 1, (ch & A_ATTRIBUTES) & ~a,	\
		PAIR_NUMBER(ch), NULL);				\
wtouchln(THIS->content, THIS->pos.line, 1, 1);			\
wsyncup(THIS->content);						\
wmove(THIS->content, oy, ox);					\
REFRESH();

BEGIN_PROPERTY(CharAttrs_Normal)

	int ox, oy;
	chtype ch;

	getyx(THIS->content, oy, ox);
	ch = mvwinch(THIS->content, THIS->pos.line, THIS->pos.col);
	if (READ_PROPERTY) {
		GB.ReturnBoolean((ch & A_ATTRIBUTES) == A_NORMAL);
		return;
	}
	if (VPROP(GB_BOOLEAN))
		wchgat(THIS->content, 1, A_NORMAL, PAIR_NUMBER(ch), NULL);
	wtouchln(THIS->content, THIS->pos.line, 1, 1);
	wmove(THIS->content, oy, ox);
	REFRESH();

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Underline)

	CHAR_ATTR_METHOD(A_UNDERLINE);

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Reverse)

	CHAR_ATTR_METHOD(A_REVERSE);

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Blink)

	CHAR_ATTR_METHOD(A_BLINK);

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Bold)

	CHAR_ATTR_METHOD(A_BOLD);

END_PROPERTY

#define CHAR_COLOR_METHOD(r, f, b)			\
short pair;						\
int ox, oy;						\
chtype ch;						\
short fg, bg;						\
							\
getyx(THIS->content, oy, ox);				\
ch = mvwinch(THIS->content, THIS->pos.line, THIS->pos.col);\
pair = PAIR_NUMBER(ch & A_COLOR);			\
pair_content(pair, &fg, &bg);				\
if (READ_PROPERTY) {					\
	GB.ReturnInteger(r);				\
	return;						\
}							\
pair = CPAIR_get(f, b);					\
if (pair == -1) {					\
	GB.Error(GB_ERR_BOUND);				\
	return;						\
}							\
wchgat(THIS->content, 1, (ch & A_ATTRIBUTES), pair, NULL);\
wtouchln(THIS->content, THIS->pos.line, 1, 1);		\
wsyncup(THIS->content);					\
wmove(THIS->content, oy, ox);				\
REFRESH();

BEGIN_PROPERTY(CharAttrs_Background)

	CHAR_COLOR_METHOD(bg, fg, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(CharAttrs_Foreground)

	CHAR_COLOR_METHOD(fg, VPROP(GB_INTEGER), bg);

END_PROPERTY

GB_DESC CCharAttrsDesc[] = {
	GB_DECLARE(".Char.Attrs", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Normal", "b", CharAttrs_Normal),
	GB_PROPERTY("Underline", "b", CharAttrs_Underline),
	GB_PROPERTY("Reverse", "b", CharAttrs_Reverse),
	GB_PROPERTY("Blink", "b", CharAttrs_Blink),
	GB_PROPERTY("Bold", "b", CharAttrs_Bold),

	GB_PROPERTY("Background", "i", CharAttrs_Background),
	GB_PROPERTY("Foreground", "i", CharAttrs_Foreground),

	GB_END_DECLARE
};

/*
 * Border
 */

GB_DESC CBorderDesc[] = {
	GB_DECLARE("Border", 0),
	GB_NOT_CREATABLE(),

	GB_CONSTANT("None", "i", BORDER_NONE),
	GB_CONSTANT("Ascii", "i", BORDER_ASCII),
	GB_CONSTANT("ACS", "i", BORDER_ACS),

	GB_END_DECLARE
};
