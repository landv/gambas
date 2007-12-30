/***************************************************************************

  gtrayicon.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

  Gtkmae "GTK+ made easy" classes

  Realizado para la Junta de Extremadura.
  Consejería de Educación Ciencia y Tecnología.
  Proyecto gnuLinEx

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/
#include "widgets.h"
#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define OPCODE "_NET_SYSTEM_TRAY_OPCODE"


/*****************************************************************************

Default picture

******************************************************************************/
/* XPM */
static char *_default_trayicon[] = {
/* columns rows colors chars-per-pixel */
"24 24 200 2",
"   c black",
".  c #0C0C0C",
"X  c #121015",
"o  c #100926",
"O  c #10033A",
"+  c #110C35",
"@  c #101334",
"#  c #11253E",
"$  c #10293F",
"%  c #282828",
"&  c #212C37",
"*  c gray23",
"=  c #100743",
"-  c #100C45",
";  c #101F46",
":  c #101E4B",
">  c #0F374D",
",  c #17264C",
"<  c #1D2A45",
"1  c #182F4E",
"2  c #102351",
"3  c #102957",
"4  c #14324C",
"5  c #12384E",
"6  c #173552",
"7  c #103A51",
"8  c #123761",
"9  c #123B60",
"0  c #2D384E",
"q  c #10465A",
"w  c #104A5E",
"e  c #1F4363",
"r  c #10526B",
"t  c #105470",
"y  c #195B79",
"u  c #106374",
"i  c #106B7C",
"p  c #34445C",
"a  c #264563",
"s  c #2D4966",
"d  c #275374",
"f  c #265678",
"g  c #225A7D",
"h  c #2B5374",
"j  c #324C67",
"k  c #305565",
"l  c #39536C",
"z  c #345C79",
"x  c #4B4B4B",
"c  c #404F56",
"v  c #4E5356",
"b  c #4F575B",
"n  c #525354",
"m  c #535658",
"M  c #565B5D",
"N  c #455469",
"B  c #4D5D64",
"V  c #505D61",
"C  c #406277",
"Z  c #5C6264",
"A  c #55657B",
"S  c #556970",
"D  c #5C6B71",
"F  c #57717B",
"G  c #626262",
"H  c #6E6C74",
"J  c #657075",
"K  c #647279",
"L  c #687378",
"P  c #777777",
"I  c #787C7E",
"U  c #107080",
"Y  c #187788",
"T  c #235D81",
"R  c #295F82",
"E  c #256387",
"W  c #296580",
"Q  c #296A83",
"!  c #297391",
"~  c #297599",
"^  c #2F7C9A",
"/  c #2A7DA3",
"(  c #4A7E99",
")  c #556380",
"_  c #547683",
"`  c #5E728A",
"'  c #597884",
"]  c #5A7B88",
"[  c #507FA6",
"{  c #657788",
"}  c #647E89",
"|  c #6F7C83",
" . c #2C84AA",
".. c #2E8EB6",
"X. c #2F92BA",
"o. c #3182A2",
"O. c #3086A8",
"+. c #308FB6",
"@. c #3696AE",
"#. c #3096BE",
"$. c #3098C0",
"%. c #3B9DC3",
"&. c #5D8494",
"*. c #5F8A9B",
"=. c #5386A0",
"-. c #5794B3",
";. c #51A2BD",
":. c #6D838D",
">. c #688E9D",
",. c #76858E",
"<. c #778891",
"1. c #738C9B",
"2. c #798A93",
"3. c #77909C",
"4. c #7A929F",
"5. c #6988A1",
"6. c #6392A5",
"7. c #6295AA",
"8. c #6B99AB",
"9. c #669FB6",
"0. c #7492A2",
"q. c #719AAC",
"w. c #7D98A4",
"e. c #7C9EAF",
"r. c #66A2B9",
"t. c #6BA1B6",
"y. c #6DA0B9",
"u. c #76A1B3",
"i. c #77AABF",
"p. c #579DC2",
"a. c #45A2C6",
"s. c #43A9D0",
"d. c #58ABCE",
"f. c #60AFCE",
"g. c #69AAC3",
"h. c #6BAEC8",
"j. c #6BB1CB",
"k. c #60B0D3",
"l. c #6CB6D2",
"z. c #6DB8D4",
"x. c #77B1C8",
"c. c #76B6D0",
"v. c #76BBD5",
"b. c #79BCD5",
"n. c #59C0E2",
"m. c #70C0DE",
"M. c #65C8E6",
"N. c #66CCEF",
"B. c #69C0E4",
"V. c #71C5E5",
"C. c #72C7E9",
"Z. c #72C9EA",
"A. c #74CEF0",
"S. c #76D3F6",
"D. c #76D5F9",
"F. c #77D8FC",
"G. c #79DCFE",
"H. c #7AE0FE",
"J. c #818282",
"K. c #858688",
"L. c #80939D",
"P. c #939393",
"I. c #9A9A9A",
"U. c #8499A5",
"Y. c #839EA9",
"T. c #889FAB",
"R. c #87A4B2",
"E. c #8DA6B2",
"W. c #92ACB9",
"Q. c #94B0BA",
"!. c #A9A6AE",
"~. c #A8A9A9",
"^. c #B4B5B5",
"/. c #93B2C1",
"(. c #9BB8C7",
"). c #9DBBCB",
"_. c #8CC5DC",
"`. c #A6C7D8",
"'. c #A8CCDD",
"]. c #AED5DF",
"[. c #AACDE1",
"{. c #B0D6E9",
"}. c #B2D8EC",
"|. c #BDE0E3",
" X c #BAE2F7",
".X c #BFE9FA",
"XX c gray76",
"oX c #CCCDCC",
"OX c #C1EDFD",
"+X c #C6F2FF",
"@X c #C8F5FC",
"#X c #CCFAFE",
"$X c #DAECF4",
"%X c #D4FEFE",
"&X c #D9FFFF",
"*X c #E5E5E5",
"=X c #EEEEEE",
"-X c #F5FAFC",
";X c gray100",
":X c None",
/* pixels */
":X:X:X:X:X:X:X}.}.}.'.`.).W.T.2.| :X:X:X:X:X:X:X",
":X:X:X:X:X.XOXOXOX.X X}.'.).Q.U.,.L M :X:X:X:X:X",
":X:X:X:X+X@X#X#X#X+XOX X{.`.(.E.L.| Z m :X:X:X:X",
":X:X:XOX#X%X&X%X#X#X.X/.4.).).W.U.<.J m x :X:X:X",
":X:X+X#X%X&X&X|.Y.1.D I.^.V ,.K D 3.:.D v v :X:X",
":X.X@X%X&X&XU.z ~ W K.*X=XoXn *X^.B 0.} S B b :X",
":XOX#X&X%X{ E #.$.Q ~.% * *Xn . P G u.>.] F S :X",
"}.OX#X%XW.f #.#.$.^ I J.P.XX& X P.Z i.6.>.&.' F ",
"}.OX#X@Xl ..$.$.$.#.k J.I.c O.s H _ x.9.8.6.>.] ",
"}..X+X].d $.$.$.d.%.$.o.! #.$.+.a 9.c.g.9.9.8.=.",
"[. X.X[.R $.#.a.$X$.$.$.$.$.$.$. .s h.h.h.g.9.7.",
"`.}. X[.h $.#.v.;X%.$.$.$.$.$.$.$./ l l.z.l.j.9.",
"(.'.}.}.p O.#._.;Xb.#.#.$.#.$.$.#.#.E j l.m.z.g.",
"W.).`.'.A 1 ..f.-X$Xa.#.$.$.$.$.#.#.$.e -.V.V.j.",
"T.W.(.).) O , T 5.!.` g / X.$.$.$.$.$...k.A.V.z.",
"<.U.E.W.0.+ O O ; r u r 7 4 T X.$.$.$.s.F.S.V.z.",
"| ,.4.U.E.N o + O - 3 t i i w 6 ~ $.%.N.G.D.Z.l.",
":XL | <.w.e.0 q 7 @ : - 2 r U u 4 T n.H.G.D.Z.:X",
":XM Z J :.0.q.p > w ; 7 3 3 t U Y ;.G.H.G.A.B.:X",
":X:Xm m D } >.6.C < $ # 7 8 y @.M.G.G.G.S.A.:X:X",
":X:X:Xx v S ] >.6.7.=.( =.p.B.F.G.G.G.S.Z.:X:X:X",
":X:X:X:Xv B S &.6.r.g.l.m.Z.Z.S.F.S.A.A.:X:X:X:X",
":X:X:X:X:Xb S ' >.7.r.h.z.m.V.Z.Z.Z.V.:X:X:X:X:X",
":X:X:X:X:X:X:XF ] =.7.9.h.h.z.z.z.:X:X:X:X:X:X:X"
};

/*****************************************************************************

Low level stuff

******************************************************************************/

void XTray_RequestDock(Display *xdisplay,Window icon)
{
	Window xmanager=None;
	XClientMessageEvent ev;
	Atom OpCodeAtom;
	Screen *xscreen;
	char buf[256];
	Atom selection_atom;

	buf[0]=0;

	xscreen=DefaultScreenOfDisplay(xdisplay);
	sprintf(buf,"_NET_SYSTEM_TRAY_S%d",XScreenNumberOfScreen (xscreen));
	selection_atom = XInternAtom (xdisplay,buf,0);

	XGrabServer (xdisplay);

	xmanager = XGetSelectionOwner (xdisplay,selection_atom);
	if ( xmanager != None)
		XSelectInput (xdisplay,xmanager,StructureNotifyMask);

	XUngrabServer (xdisplay);
	XFlush (xdisplay);


	/***********************************************
	 Dock Tray Icon
	************************************************/
	OpCodeAtom=XInternAtom(xdisplay,OPCODE,0);

	ev.type = ClientMessage;
	ev.window = xmanager;
	ev.message_type = OpCodeAtom;
	ev.format = 32;
	ev.data.l[0] = 0;
	ev.data.l[1] = SYSTEM_TRAY_REQUEST_DOCK;
	ev.data.l[2] = icon;
	ev.data.l[3] = 0;
	ev.data.l[4] = 0;

	XSendEvent (xdisplay,xmanager, 0, NoEventMask, (XEvent *)&ev);
	XSync (xdisplay, 0);

}

void XTray_getSize(Display *xdisplay,Window icon,unsigned int *w,unsigned int *h)
{
	XWindowAttributes Attr;

	XGetWindowAttributes(xdisplay,icon,&Attr);

	if (w) *w=Attr.width;
	if (h) *h=Attr.height;

}

void XTray_getPosition(Display *xdisplay,Window icon,unsigned int *x,unsigned int *y)
{
	Window rwin;
	Window pwin,pwin2;
	Window *cwin;
	unsigned int count;
	XWindowAttributes Attr;
	Window w=icon;

	if (x) *x=0;
	if (y) *y=0;

	do
	{
		XQueryTree(xdisplay,w,&rwin,&pwin,&cwin,&count);
		if (cwin) XFree(cwin);
		if (pwin)
		{
			XGetWindowAttributes(xdisplay, pwin,&Attr);
			if (x) (*x)+=Attr.x;
			if (y) (*y)+=Attr.y;
			w=pwin;
		}
	} while (pwin);
}




/*************************************************************************

gTrayIcon

**************************************************************************/
bool  tray_enterleave(GtkWidget *widget, GdkEventCrossing *e,gTrayIcon *data)
{
	if (e->type==GDK_ENTER_NOTIFY)
	{
		if (data->onEnter) data->onEnter(data);
	}
	else
	{
		if (data->onLeave) data->onLeave(data);
	}
	return false;
}

void tray_end (GtkWidget *object,gTrayIcon *data)
{
	if (data->onHide) { data->onHide=false; return; }
	if (data->onDestroy) data->onDestroy(data);
	delete data;
}

gboolean tray_event(GtkWidget *widget, GdkEvent *event,gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;

	if (event->type==GDK_2BUTTON_PRESS)
	{
		if (data->onDoubleClick) data->onDoubleClick(data);
		return false;
	}


	return false;
}


gboolean tray_down (GtkWidget *widget,GdkEventButton *event,gTrayIcon *data)
{

	if (!gApplication::userEvents()) return false;

	if (data->onMousePress)
	{
		gMouse::setValid(true,(long)event->x,(long)event->y,event->button,event->state,0,0);
		data->onMousePress(data);
		gMouse::setValid(false,0,0,0,0,0,0);
	}

	if (event->button==3)
		if (data->onMenu)
			data->onMenu(data);

	return false;
}

gboolean tray_up (GtkWidget *widget,GdkEventButton *event,gTrayIcon *data)
{
	if (!gApplication::userEvents()) return false;

	if (data->onMouseRelease)
	{
		gMouse::setValid(true,(long)event->x,(long)event->y,event->button,event->state,0,0);
		data->onMouseRelease(data);
		gMouse::setValid(false,0,0,0,0,0,0);
	}

	return false;
}

gboolean tray_focus_In(GtkWidget *widget,GdkEventFocus *event,gTrayIcon *data)
{
	if (!gApplication::allEvents()) return false;

	if (data->onFocusEnter) data->onFocusEnter(data);
	return false;
}

gboolean tray_focus_Out(GtkWidget *widget,GdkEventFocus *event,gTrayIcon *data)
{
	if (!gApplication::allEvents()) return false;

	if (data->onFocusLeave) data->onFocusLeave(data);
	return false;
}

void gTrayIcon::setPicture(gPicture *pic)
{
	GtkWidget *icon;

	if (buficon) { g_object_unref(G_OBJECT(buficon)); buficon=NULL; }
	if (pic)
	{
		if (pic->pic)
		{
			buficon=pic->getPixbuf();
		}
	}

	if (plug)
	{
		icon=gtk_bin_get_child(GTK_BIN(plug));
		gtk_image_set_from_pixbuf(GTK_IMAGE(icon),buficon);
	}

}

gPicture* gTrayIcon::picture()
{
	gPicture *pic=NULL;

	pic=gPicture::fromPixbuf(buficon);
	return pic;

}

char* gTrayIcon::toolTip()
{
	return buftext;

}

void gTrayIcon::setToolTip(char* vl)
{
	GtkTooltips *tip;

	if (!vl) vl="";

	if (buftext) free(buftext);
	buftext=(char*)malloc(sizeof(char)*(strlen(vl) + 1));
	if (buftext) strcpy(buftext,vl);

	if (plug && buftext)
	{
		tip=gtk_tooltips_new();
		gtk_tooltips_set_tip(tip,plug,buftext,NULL);
		if (strlen(buftext)) gtk_tooltips_enable(tip);
		else gtk_tooltips_disable(tip);
	}

}

void gTrayIcon::show()
{
	GtkTooltips *tip;
	GtkWidget *icon;
	Window win;

	if (!plug)
	{
		plug=gtk_plug_new(0);
		icon=gtk_image_new_from_pixbuf(buficon);
		gtk_container_add(GTK_CONTAINER(plug),icon);

		if (buftext)
		{
			tip=gtk_tooltips_new();
			gtk_tooltips_set_tip(tip,plug,buftext,NULL);
			if (strlen(buftext)) gtk_tooltips_enable(tip);
			else gtk_tooltips_disable(tip);
		}

		gtk_widget_show_all(plug);

		//gtk_widget_add_events(plug,GDK_PROXIMITY_IN_MASK);
		gtk_widget_add_events(plug,GDK_BUTTON_PRESS_MASK);
		gtk_widget_add_events(plug,GDK_BUTTON_RELEASE_MASK);
		gtk_widget_add_events(plug,GDK_ENTER_NOTIFY_MASK);
		gtk_widget_add_events(plug,GDK_LEAVE_NOTIFY_MASK);
		g_signal_connect(G_OBJECT(plug),"destroy",G_CALLBACK(tray_end),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"event",G_CALLBACK(tray_event),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"button-release-event",G_CALLBACK(tray_up),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"button-press-event",G_CALLBACK(tray_down),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"focus-in-event",G_CALLBACK(tray_focus_In),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"focus-out-event",G_CALLBACK(tray_focus_Out),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"enter-notify-event",G_CALLBACK(tray_enterleave),(gpointer)this);
		g_signal_connect(G_OBJECT(plug),"leave-notify-event",G_CALLBACK(tray_enterleave),(gpointer)this);
		win=gtk_plug_get_id(GTK_PLUG(plug));
		XTray_RequestDock(gdk_display,win);
	}
}

void gTrayIcon::hide()
{
	if (plug)
	{
		onHide=true;
		gtk_widget_destroy(plug);
		plug=NULL;
	}
}

bool gTrayIcon::visible()
{
	return (bool)plug;
}


void gTrayIcon::setVisible(bool vl)
{
	if (vl) show();
	else    hide();
}

long gTrayIcon::screenX()
{
	unsigned int ret;

	if (!plug) return 0;
	XTray_getPosition(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),&ret,NULL);

	return ret;
}

long gTrayIcon::screenY()
{
	unsigned int ret;

	if (!plug) return 0;
	XTray_getPosition(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),NULL,&ret);

	return ret;
}

long gTrayIcon::width()
{
	unsigned int ret;

	XTray_getSize(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),&ret,NULL);
	return ret;
}

long gTrayIcon::height()
{
	unsigned int ret;

	XTray_getSize(gdk_display,gtk_plug_get_id(GTK_PLUG(plug)),NULL,&ret);
	return ret;
}

gTrayIcon::gTrayIcon()
{
	buficon=gdk_pixbuf_new_from_xpm_data((const char**)_default_trayicon);
	plug=NULL;
	buftext=NULL;

	onHide=false;
	onMousePress=NULL;
	onMouseRelease=NULL;
	onMenu=NULL;
	onFocusEnter=NULL;
	onFocusLeave=NULL;
	onDoubleClick=NULL;
	onEnter=NULL;
	onLeave=NULL;
}

void gTrayIcon::destroy()
{
	if (buficon) g_object_unref(G_OBJECT(buficon));
	if (plug) gtk_widget_destroy(plug);
	if (buftext) free(buftext);
}

#else

void gTrayIcon::setPicture(gPicture *pic)
{

	stub("no-X11/gTrayIcon class");
}

gPicture* gTrayIcon::picture()
{
	stub("no-X11/gTrayIcon class");
	return NULL;

}

char* gTrayIcon::toolTip()
{
	stub("no-X11/gTrayIcon class");
	return NULL;

}

void gTrayIcon::setToolTip(char* vl)
{
	stub("no-X11/gTrayIcon class");

}

void gTrayIcon::show()
{
	stub("no-X11/gTrayIcon class");
}

void gTrayIcon::hide()
{
	stub("no-X11/gTrayIcon class");
}

bool gTrayIcon::visible()
{
	stub("no-X11/gTrayIcon class");
	return false;
}


void gTrayIcon::setVisible(bool vl)
{
	stub("no-X11/gTrayIcon class");
}

long gTrayIcon::screenX()
{
	stub("no-X11/gTrayIcon class");
	return 0;
}

long gTrayIcon::screenY()
{
	stub("no-X11/gTrayIcon class");
	return 0;
}

long gTrayIcon::width()
{
	stub("no-X11/gTrayIcon class");
	return 0;
}

long gTrayIcon::height()
{
	stub("no-X11/gTrayIcon class");
}

gTrayIcon::gTrayIcon()
{
	stub("no-X11/gTrayIcon class");
}

void gTrayIcon::destroy()
{
	stub("no-X11/gTrayIcon class");
}

#endif
