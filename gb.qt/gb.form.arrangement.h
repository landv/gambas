/***************************************************************************

  gb.qt.arrangement.h

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

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

#define __GB_QT_ARRANGEMENT_H

/***************************************************************************

#define WIDGET_TYPE
Datatype of variables that points at a widget

#define CONTAINER_TYPE
Datatype of variables that points at a container widget

#define ARRANGEMENT_TYPE
Datatype of variables that points at the arrangement structure.
This structure contains all the arrangement flags, and must have (at least)
the following fields:
- mode : the arrangement style.
- spacing : number of pixels that separates children.
- padding : number of pixels around the arrangement area.
- autoresize : if the container must try to fit its contents.
- locked : if the container is being arranged.
- user : if the container is a UserControl or a UserContainer.

#define IS_RIGHT_TO_LEFT()
If the current language is right to left written

#define GET_WIDGET(_object)
Returns the widget associated with the gambas control

#define GET_CONTAINER(_object)
Returns the container widget inside the gambas control that contains the
child widgets. It can be the same for a Panel, for example, or a sub widget
for something more complex, like a TabStrip.

#define GET_ARRANGEMENT(_object)
Returns a pointer to the arrangement flags of a Gambas control.

#define IS_EXPAND(_object)
Returns if the Expand property of a control is set or not.

#define IS_DESIGN(_object)
Returns if the control is in design mode.

#define IS_WIDGET_VISIBLE(_widget)
Returns if a widget is visible to the screen.

#define GET_WIDGET_CONTENTS(_widget, _x, _y, _w, _h)
Sets the _x, _y, _w & _h variables with the dimensions of the area
inside the container that must be arranged, in container coordinates.
If the container has a border, for example, then _x > 0 & _y > 0.

#define GET_WIDGET_X(_widget)
Returns the x position of a widget inside its parent.

#define GET_WIDGET_Y(_widget)
Returns the y position of a widget inside its parent.

#define GET_WIDGET_W(_widget)
Returns the width of a widget.

#define GET_WIDGET_H(_widget)
Returns the height of a widget.

#define MOVE_WIDGET(_widget, _x, _y)
Moves a widget inside its parent.

#define RESIZE_WIDGET(_widget, _w, _h)
Resizes a widget.

#define MOVE_RESIZE_WIDGET(_widget, _x, _y, _w, _h)
Move & resize a widget simultaneously.

#define INIT_CHECK_CHILDREN_LIST(_widget)
Initializes the list of children of _widget. Must return if there is no
children.

#define RESET_CHILDREN_LIST()
Resets the children list so that its next enumeration starts from the first child.

#define GET_NEXT_CHILD_WIDGET()
Returns the next child to arrange, or NULL if there is no child anymore.
Make a function, as this macro is called several times.

#define GET_OBJECT_FROM_WIDGET(_widget)
Returns the Gambas object associated with the specified widget, or NULL if there is
no Gambas object.

#define RAISE_ARRANGE_EVENT(_object)
Code to raise the Arrange event of containers.

#define FUNCTION_NAME
This is the name of the arrangement function


***************************************************************************/

#define ARRANGE_ROW ARRANGE_LEFT_RIGHT
#define ARRANGE_COLUMN ARRANGE_TOP_BOTTOM

void FUNCTION_NAME(void *_object) //(QFrame *cont)
{
  CONTAINER_TYPE cont;
  ARRANGEMENT_TYPE arr;

  WIDGET_TYPE wid;
  int x, y, w, h, i;
  int xc, yc, wc, hc;
  int wm, hm;
  int wf, hf;
  int sexp, nexp;
  bool swap, autoresize;
  bool redo;
  bool first;
  void *ob;
  bool rtl = IS_RIGHT_TO_LEFT();
  int rtlm = rtl ? -1 : 1;

  cont = (CONTAINER_TYPE)GET_CONTAINER(_object);
  arr = GET_ARRANGEMENT(_object);

  //qDebug("CCONTAINER_arrange: %p %s %d %d: BEGIN", THIS, GB.GetClassName(THIS), IS_WIDGET_VISIBLE(cont), arr->mode);

  if (!IS_WIDGET_VISIBLE(GET_WIDGET(_object)))  //|| IS_DESIGN(_object))
    return;

  if (arr->locked)
    return;

	if (arr->mode != ARRANGE_NONE)
	{
		if (arr->user)
			cont = (CONTAINER_TYPE)GET_WIDGET(_object);

		INIT_CHECK_CHILDREN_LIST(cont);

		arr->locked = true;

		swap = (arr->mode & 1) == 0;
		autoresize = arr->autoresize; // && !IS_EXPAND(_object);

		for(i = 0; i < 3; i++)
		{
			redo = false;

			GET_WIDGET_CONTENTS(cont, xc, yc, wc, hc);
			wf = GET_WIDGET_W(cont) - wc;
			hf = GET_WIDGET_H(cont) - hc;
			
			//if (hc > GET_WIDGET_H(cont))
			//	qDebug("hc = %d H = %d ?", hc, GET_WIDGET_H(cont));
			
			xc += arr->padding;
			yc += arr->padding;
			wc -= arr->padding * 2;
			hc -= arr->padding * 2;

			//qDebug("CCONTAINER_arrange: %p: %s (%d, %d, %d, %d) pad %d spc %d [%d]", THIS, GB.GetClassName(THIS), xc, yc, wc, hc, arr->padding, arr->spacing, i);

			if (autoresize)
			{
				if (wc <= 0 && hc <= 0)
					break;
			}
			else
			{
				if (wc <= 0 || hc <= 0)
					break;
			}

			x = xc;
			y = yc;
			w = h = 0;
			wm = hm = 0;

			if (rtl && !swap)
				x += wc;

			wid = 0;
			RESET_CHILDREN_LIST();

			switch (arr->mode)
			{
				case ARRANGE_HORIZONTAL:
				case ARRANGE_VERTICAL:

					sexp = 0;
					nexp = 0;

					for(;;)
					{
						wid = GET_NEXT_CHILD_WIDGET();
						if (!wid)
							break;

						ob = GET_OBJECT_FROM_WIDGET(wid);

						if (ob && IS_EXPAND(ob))
							nexp++;
						else
							sexp += (swap ? GET_WIDGET_H(wid) : GET_WIDGET_W(wid));

						sexp += arr->spacing;
					}

					sexp -= arr->spacing;
					sexp = (swap ? hc : wc) - sexp;

					RESET_CHILDREN_LIST();
					wid = 0;

					for(;;)
					{
						first = wid == 0;

						wid = GET_NEXT_CHILD_WIDGET();
						if (!wid)
							break;

						if (!first)
						{
							if (swap)
								y += arr->spacing;
							else
								x += arr->spacing * rtlm;
						}

						ob = GET_OBJECT_FROM_WIDGET(wid);

						if (swap)
						{
							if (IS_EXPAND(ob) && !autoresize)
							{
								h = sexp / nexp;
								sexp -= h;
								nexp--;
							}
							else
								h = GET_WIDGET_H(wid);
								
							//w = autoresize ? GET_WIDGET_W(wid) : wc;
							w = wc;

							if (h > 0 && w > 0)
							{
								if (w != GET_WIDGET_W(wid) || h != GET_WIDGET_H(wid))
									redo = true;

								MOVE_RESIZE_WIDGET(wid, x, y, w, h);
								y += h;
							
								if (w > wm) wm = w;
							}
						}
						else
						{
							if (IS_EXPAND(ob) && !autoresize)
							{
								w = sexp / nexp;
								sexp -= w;
								nexp--;
							}
							else
								w = GET_WIDGET_W(wid);
								
							//h = autoresize ? GET_WIDGET_H(wid) : hc;
							h = hc;

							if (w > 0 && h > 0)
							{
								if (w != GET_WIDGET_W(wid) || h != GET_WIDGET_H(wid))
									redo = true;

								if (rtl)
									MOVE_RESIZE_WIDGET(wid, x - w, y, w, h);
								else
									MOVE_RESIZE_WIDGET(wid, x, y, w, h);
								x += w * rtlm;
								
								if (h > hm) hm = h;
							}
						}
					}
					break;

				case ARRANGE_ROW:
				case ARRANGE_COLUMN:

					wc += xc;
					hc += yc;

					for(;;)
					{
						wid = GET_NEXT_CHILD_WIDGET();
						if (!wid)
							break;

						ob = GET_OBJECT_FROM_WIDGET(wid);

						if (swap)
						{
							if (rtl)
							{
								if ((y > yc) && ((y + (IS_EXPAND(ob) ? 8 : GET_WIDGET_H(wid))) > hc))
								{
									y = yc;
									x -= w + arr->spacing;
									w = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(wid, x - GET_WIDGET_W(wid), y, GET_WIDGET_W(wid), hc - y);
									y = hc + arr->spacing;
								}
								else
								{
	   							MOVE_WIDGET(wid, x - GET_WIDGET_W(wid), y);
									y += GET_WIDGET_H(wid) + arr->spacing;
								}

								if (GET_WIDGET_W(wid) > w)
									w = GET_WIDGET_W(wid);
							}
							else
							{
								if ((y > yc) && ((y + (IS_EXPAND(ob) ? 8 : GET_WIDGET_H(wid))) > hc))
								{
									y = yc;
									x += w + arr->spacing;
									w = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(wid, x, y, GET_WIDGET_W(wid), hc - y);
									y = hc + arr->spacing;
								}
								else
								{
									MOVE_WIDGET(wid, x, y);
									y += GET_WIDGET_H(wid) + arr->spacing;
								}

								if (GET_WIDGET_W(wid) > w)
									w = GET_WIDGET_W(wid);
							}
						}
						else
						{
							if (rtl)
							{
								if ((x < (xc + wc)) && ((x - (IS_EXPAND(ob) ? 8 : GET_WIDGET_W(wid))) < xc))
								{
									x = xc + wc;
									y += h + arr->spacing;
									h = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(wid, xc, y, x - xc, GET_WIDGET_H(wid));
									x = xc - arr->spacing;
								}
								else
								{
									MOVE_WIDGET(wid, x - GET_WIDGET_W(wid), y);
									x -= GET_WIDGET_W(wid) + arr->spacing;
								}

								if (GET_WIDGET_H(wid) > h)
									h = GET_WIDGET_H(wid);
							}
							else
							{
								if ((x > xc) && ((x + (IS_EXPAND(ob) ? 8 : GET_WIDGET_W(wid))) > wc))
								{
									x = xc;
									y += h + arr->spacing;
									h = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(wid, x, y, wc - x, GET_WIDGET_H(wid));
									x = wc + arr->spacing;
								}
								else
								{
									MOVE_WIDGET(wid, x, y);
									x += GET_WIDGET_W(wid) + arr->spacing;
								}

								if (GET_WIDGET_H(wid) > h)
									h = GET_WIDGET_H(wid);
							}
						}
					}
					break;

				case ARRANGE_FILL:

					for(;;)
					{
						wid = GET_NEXT_CHILD_WIDGET();
						if (!wid)
							break;

						MOVE_RESIZE_WIDGET(wid, xc, yc, wc, hc);
					}

					break;
			}

			if (autoresize)
			{
				switch(arr->mode)
				{
					case ARRANGE_HORIZONTAL:
						//qDebug("CCONTAINER_arrange: %p: %s x = %d autoresize (%d, %d) pad %d spc %d [%d]", THIS, GB.GetClassName(THIS), x, x + cont->width() - wc - xc, cont->height(), arr->padding, arr->spacing, i);
						//cont->resize(x + cont->width() - wc - xc, cont->height());
						if (rtl)
							//RESIZE_WIDGET(cont, xc - x + arr->padding + wf, hm + arr->padding * 2 + hf);
							RESIZE_WIDGET(cont, xc - x + arr->padding + wf, GET_WIDGET_H(cont));
						else
							//RESIZE_WIDGET(cont, x + arr->padding + wf, hm + arr->padding * 2 + hf);
							RESIZE_WIDGET(cont, x + arr->padding + wf, GET_WIDGET_H(cont));
						break;

					case ARRANGE_COLUMN:
						if (rtl)
							RESIZE_WIDGET(cont, (wc - x) + w + arr->padding + wf, GET_WIDGET_H(cont));
						else
							RESIZE_WIDGET(cont, x + w + arr->padding + wf, GET_WIDGET_H(cont));
						break;

					case ARRANGE_VERTICAL:
						//RESIZE_WIDGET(cont, wm + arr->padding * 2 + wf, y + arr->padding + hf);
						RESIZE_WIDGET(cont, GET_WIDGET_W(cont), y + arr->padding + hf);
						break;

					case ARRANGE_ROW:
						//if ((y + h + arr->padding + GET_WIDGET_H(cont) - hc - yc) < 16)
						//	qDebug("y = %d h = %d arr->padding = %d H = %d hc = %d yc = %d -> %d", y, h, arr->padding, GET_WIDGET_H(cont), hc, yc, (y + h + arr->padding + GET_WIDGET_H(cont) - hc - yc));
						RESIZE_WIDGET(cont, GET_WIDGET_W(cont), y + h + arr->padding + hf);
						break;
				}
			}

			if (!redo)
				break;
		}

		arr->locked = false;
	}

	#ifdef RAISE_ARRANGE_EVENT
	RAISE_ARRANGE_EVENT(_object);
	#endif

  //qDebug("%p: dirty = FALSE", THIS);
  //arr->dirty = false;

  //qDebug("CCONTAINER_arrange: END %p", THIS);

  //qDebug("CCONTAINER_arrange: %p: END", cont);
}


