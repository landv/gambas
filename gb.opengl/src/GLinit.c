/***************************************************************************

  GLinit.c

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#include "gambas.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glew.h>
#include <GL/glx.h>
#include <stdio.h>
#include <string.h>

static Window win;
static GLXContext ctx;
static Display *dpy;

void init_glew()
{
	XSetWindowAttributes attr;
	unsigned long mask;
	Window root;
	XVisualInfo *visinfo;

	int attribSingle[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		None };
	int attribDouble[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		None };

	int width = 100, height = 100;
	dpy = XOpenDisplay(NULL);
	int scrnum = XDefaultScreen(dpy);

	root = RootWindow(dpy, scrnum);

	visinfo = glXChooseVisual(dpy, scrnum, attribSingle);

	if (!visinfo)
	{
		visinfo = glXChooseVisual(dpy, scrnum, attribDouble);
		if (!visinfo)
		{
			fprintf(stderr, "Error: couldn't find RGB GLX visual\n");
			return;
		}
	}

	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = StructureNotifyMask | ExposureMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root, 0, 0, width, height,
		0, visinfo->depth, InputOutput,
		visinfo->visual, mask, &attr);

	ctx = glXCreateContext( dpy, visinfo, NULL, True );

	if (!ctx)
	{
		fprintf(stderr, "Error: glXCreateContext failed\n");
		XDestroyWindow(dpy, win);
		return;
	}

	glXDestroyContext(dpy, ctx);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}
