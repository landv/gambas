/***************************************************************************

  gb.form.arrangement.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#define __GB_FORM_ARRANGEMENT_H

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
- spacing : if children must be separate by Desktop.Scale
- padding : number of pixels around the arrangement area.
- margin : if the arrangement area must have a margin of Desktop.Scale
- indent: if the arrangement area must have an indentation of Desktop.Scale
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

#define MOVE_WIDGET(_object, _widget, _x, _y)
Moves a widget inside its parent.

#define RESIZE_WIDGET(_object, _widget, _w, _h)
Resizes a widget.

#define MOVE_RESIZE_WIDGET(_object, _widget, _x, _y, _w, _h)
Move & resize a widget simultaneously.

#define RESIZE_CONTAINER(_object, _w, _h)
Resizes the container object by resizing the widget itself.

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

#define DESKTOP_SCALE
Returns the value of Desktop.Scale

#define RAISE_ARRANGE_EVENT(_object)
Code to raise the Arrange event of containers.

#define RAISE_BEFORE_ARRANGE_EVENT(_object)
Code to raise the BeforeArrange event of containers.

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
	#ifndef GET_MAX_SIZE
	int wf, hf;
	#endif
	int dmax = 0, d;
	int sexp, nexp;
	bool swap, autoresize, invert;
	#ifndef GET_MAX_SIZE
	bool has_expand_children = false;
	#endif
	bool redo;
	bool first;
	void *ob;
	bool rtl;
	int rtlm;
	int padding;
	int spacing;
	int indent;

	if (!CAN_ARRANGE(_object))
		return;
		
	cont = (CONTAINER_TYPE)GET_CONTAINER(_object);
	arr = GET_ARRANGEMENT(_object);

	//if (!IS_WIDGET_VISIBLE(cont) && !IS_WIDGET_VISIBLE(GET_WIDGET(_object)))
	//  return;
	
	//fprintf(stderr, "CCONTAINER_arrange: %s: locked %d: mode %d: autoresize: %d\n", GET_OBJECT_NAME(_object), arr->locked, arr->mode, arr->autoresize);

	if (arr->locked)
		return;
		
// 	if (GET_WIDGET_W(cont) <= 1 || GET_WIDGET_H(cont) <= 1)
// 		return;

	//if (qstrcmp(GB.GetClassName(THIS), "FOutput") == 0)
	//	qDebug("CCONTAINER_arrange: do it!");
	
	arr->locked = true;

	RAISE_BEFORE_ARRANGE_EVENT(_object);

	if (arr->mode != ARRANGE_NONE)
	{
		// g_debug("arrange: %s (%d %d) (%d %d)", ((gControl *)_object)->name(), ((gContainer *)_object)->width(), ((gContainer *)_object)->height(), ((gContainer *)_object)->clientWidth(), ((gContainer *)_object)->clientHeight());
		//usleep(50000);

		if (arr->user)
			cont = (CONTAINER_TYPE)GET_WIDGET(_object);

		// INIT_CHECK_CHILDREN_LIST() can return
		arr->locked = false;
		if (!cont)
			goto __RETURN;
		
		INIT_CHECK_CHILDREN_LIST(cont);
		
		//fprintf(stderr, "CCONTAINER_arrange: %s: children: %d\n", GET_OBJECT_NAME(_object), list.count());

		if (!HAS_CHILDREN())
			goto __RETURN;
		
		arr->locked = true;

		//fprintf(stderr, "CCONTAINER_arrange: %s: mode %d: autoresize: %d\n", THIS->widget.name, arr->mode, arr->autoresize);

		invert = arr->invert;
		rtl = IS_RIGHT_TO_LEFT() ^ invert;
		rtlm = rtl ? -1 : 1;
		swap = (arr->mode & 1) == 0; // means "vertical"

		autoresize = arr->autoresize; // && !IS_EXPAND(_object);
		
		if (arr->margin)
			padding = arr->padding ? arr->padding : DESKTOP_SCALE;
		else if (!arr->spacing)
			padding = arr->padding;
		else
			padding = 0;
		
		if (arr->spacing)
			spacing = arr->padding ? arr->padding : DESKTOP_SCALE;
		else
			spacing = 0;
		
		indent = arr->indent * DESKTOP_SCALE;
		
		for(i = 0; i < 3; i++)
		{
			redo = false;

			GET_WIDGET_CONTENTS(cont, xc, yc, wc, hc);
			#ifndef GET_MAX_SIZE
			wf = GET_WIDGET_W(cont) - wc;
			hf = GET_WIDGET_H(cont) - hc;
			#endif
			
			//fprintf(stderr, "cont: %s: %d %d %d %d (%d %d)\n", ((gControl *)_object)->name(), xc, yc, wc, hc, wf, hf);
			
			//if (hc > GET_WIDGET_H(cont))
			//	qDebug("hc = %d H = %d ?", hc, GET_WIDGET_H(cont));
			
			xc += padding;
			yc += padding;
			wc -= padding * 2;
			hc -= padding * 2;

			//if (!strcmp(GET_OBJECT_NAME(_object), "TabStrip1"))
			//	fprintf(stderr, "#0: %d %d %d %d\n", xc, yc, wc, hc);
				
			if (indent)
			{
				if (swap)
				{
					if (!invert)
						yc += indent;
					hc -= indent;
				}
				else
				{
					if (!rtl)
						xc += indent;
					wc -= indent;
				}
			}

			// Beware! This is not the same test!
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

			//if (!strcmp(GET_OBJECT_NAME(_object), "HBox1"))
			//	fprintf(stderr, "#1\n");
				
			x = xc;
			y = yc;
			w = h = 0;

			if (swap)
			{
				if (invert)
					y += hc;
			}
			else
			{
				if (rtl)
					x += wc;
			}
			
			wid = 0;
			RESET_CHILDREN_LIST();

			switch (arr->mode)
			{
				case ARRANGE_HORIZONTAL:
				case ARRANGE_VERTICAL:

					sexp = 0;
					nexp = 0;
					dmax = 0;
					
					for(;;)
					{
						wid = GET_NEXT_CHILD_WIDGET();
	
						if (!wid)
							break;
						
						ob = GET_OBJECT_FROM_WIDGET(wid);
						
						if (!ob || IS_IGNORE(ob))
							continue;
						
						if (IS_EXPAND(ob))
							nexp++;
						else
							sexp += (swap ? GET_WIDGET_H(wid) : GET_WIDGET_W(wid));

						if (autoresize)
						{
							d = swap ? GET_WIDGET_W(wid) : GET_WIDGET_H(wid);
							if (d > dmax)
							{
								dmax = d;
							}
							//fprintf(stderr, "%s: %s: d = %d dmax = %d\n", ((gControl *)_object)->name(), wid->name(), d, dmax);
						}

						sexp += spacing;
					}
					
					#ifndef GET_MAX_SIZE
					has_expand_children = nexp > 0;
					#endif

					if (autoresize)
					{
					// TODO: use rtl
						if (swap)
							wc = dmax + indent;
						else
							hc = dmax + indent;
						//fprintf(stderr, "%s: dmax = %d\n", ((CWIDGET *)_object)->name, dmax);
					}

					sexp -= spacing;
					sexp = (swap ? hc : wc) - sexp;
					if (sexp < 0)
						sexp = 0;

					RESET_CHILDREN_LIST();
					wid = 0;

					for(;;)
					{
						first = wid == 0;

						wid = GET_NEXT_CHILD_WIDGET();
						if (!wid)
							break;

						ob = GET_OBJECT_FROM_WIDGET(wid);
						if (!ob || IS_IGNORE(ob))
							continue;

						if (!first)
						{
							if (swap)
							{
								if (invert)
									y -= spacing;
								else
									y += spacing;
							}
							else
								x += spacing * rtlm;
						}

						if (swap)
						{
							if (IS_EXPAND(ob)) // && !autoresize)
							{
								h = sexp / nexp;
								sexp -= h;
								nexp--;
								if (h <= 0)
									MOVE_WIDGET(ob, wid, -GET_WIDGET_W(wid), GET_WIDGET_Y(wid));
							}
							else
								h = GET_WIDGET_H(wid);
								
							//w = autoresize ? GET_WIDGET_W(wid) : wc;
							w = wc;

							if (h > 0 && w > 0)
							{
								if (w != GET_WIDGET_W(wid) || h != GET_WIDGET_H(wid))
									redo = true;

								if (invert)
								{
									y -= h;
									MOVE_RESIZE_WIDGET(ob, wid, x, y, w, h);
								}
								else
								{
									MOVE_RESIZE_WIDGET(ob, wid, x, y, w, h);
									y += h;
								}
							}
						}
						else
						{
							if (IS_EXPAND(ob)) // && !autoresize)
							{
								w = sexp / nexp;
								sexp -= w;
								nexp--;
								if (w <= 0)
									MOVE_WIDGET(ob, wid, GET_WIDGET_X(wid), -GET_WIDGET_H(wid));
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
									MOVE_RESIZE_WIDGET(ob, wid, x - w, y, w, h);
								else
									MOVE_RESIZE_WIDGET(ob, wid, x, y, w, h);
								x += w * rtlm;
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
						if (!ob || IS_IGNORE(ob))
							continue;

						if (swap)
						{
							if (rtl || invert)
							{
								if ((y > yc) && ((y + (IS_EXPAND(ob) ? DESKTOP_SCALE : GET_WIDGET_H(wid))) > hc))
								{
									y = yc;
									x -= w + spacing;
									w = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(ob, wid, x - GET_WIDGET_W(wid), y, GET_WIDGET_W(wid), hc - y);
									y = hc + spacing;
								}
								else
								{
									MOVE_WIDGET(ob, wid, x - GET_WIDGET_W(wid), y);
									y += GET_WIDGET_H(wid) + spacing;
								}

								if (GET_WIDGET_W(wid) > w)
									w = GET_WIDGET_W(wid);
							}
							else
							{
								if ((y > yc) && ((y + (IS_EXPAND(ob) ? DESKTOP_SCALE : GET_WIDGET_H(wid))) > hc))
								{
									y = yc;
									x += w + spacing;
									w = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(ob, wid, x, y, GET_WIDGET_W(wid), hc - y);
									y = hc + spacing;
								}
								else
								{
									MOVE_WIDGET(ob, wid, x, y);
									y += GET_WIDGET_H(wid) + spacing;
								}

								if (GET_WIDGET_W(wid) > w)
									w = GET_WIDGET_W(wid);
							}
						}
						else
						{
							if (rtl)
							{
								if ((x < (xc + wc)) && ((x - (IS_EXPAND(ob) ? DESKTOP_SCALE : GET_WIDGET_W(wid))) < xc))
								{
									x = xc + wc;
									y += h + spacing;
									h = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(ob, wid, xc, y, x - xc, GET_WIDGET_H(wid));
									x = xc - spacing;
								}
								else
								{
									MOVE_WIDGET(ob, wid, x - GET_WIDGET_W(wid), y);
									x -= GET_WIDGET_W(wid) + spacing;
								}

								if (GET_WIDGET_H(wid) > h)
									h = GET_WIDGET_H(wid);
							}
							else
							{
								if ((x > xc) && ((x + (IS_EXPAND(ob) ? DESKTOP_SCALE : GET_WIDGET_W(wid))) > wc))
								{
									x = xc;
									y += h + spacing;
									h = 0;
								}

								if (IS_EXPAND(ob))
								{
									MOVE_RESIZE_WIDGET(ob, wid, x, y, wc - x, GET_WIDGET_H(wid));
									x = wc + spacing;
								}
								else
								{
									MOVE_WIDGET(ob, wid, x, y);
									x += GET_WIDGET_W(wid) + spacing;
								}

								if (GET_WIDGET_H(wid) > h)
									h = GET_WIDGET_H(wid);
							}
						}
					}
					break;

				case ARRANGE_FILL:
					
					w = h = 0;
					
					for(;;)
					{
						wid = GET_NEXT_CHILD_WIDGET();
						if (!wid)
							break;

						ob = GET_OBJECT_FROM_WIDGET(wid);
						if (!ob || IS_IGNORE(ob))
							continue;
						
						MOVE_RESIZE_WIDGET(ob, wid, xc, yc, wc, hc);
						
						if (GET_WIDGET_H(wid) > h)
							h = GET_WIDGET_H(wid);
						
						if (GET_WIDGET_W(wid) > w)
							w = GET_WIDGET_W(wid);
					}

					break;
			}

			if (autoresize)
			{
				switch(arr->mode)
				{
					case ARRANGE_HORIZONTAL:
// 						#ifndef QNAMESPACE_H
// 						fprintf(stderr, "%s: HORIZONTAL: x = %d autoresize (%d, %d) pad %d spc %d [%d] rtl %d\n", 
// 						((gControl *)_object)->name(), x, x + arr->padding + wf, GET_WIDGET_H(cont), arr->padding, spacing, i, rtl);
// 						#else
// 						fprintf(stderr, "%s: HORIZONTAL: x = %d autoresize (%d, %d) pad %d spc %d [%d] rtl %d hec %d\n", 
// 							((CWIDGET *)_object)->name, x, has_expand_children ? GET_WIDGET_W(cont) : x + arr->padding + wf, 
// 							dmax + hf, arr->padding, spacing, i, rtl, has_expand_children);						
// 						#endif
						//cont->resize(x + cont->width() - wc - xc, cont->height());
						if (dmax > 0)
						{
							if (rtl)
								//RESIZE_WIDGET(cont, xc - x + arr->padding + wf, GET_WIDGET_H(cont));
								RESIZE_CONTAINER(_object, cont, has_expand_children ? GET_WIDGET_W(cont) : xc - x + padding + wf, dmax + hf + padding * 2);
							else
								//RESIZE_WIDGET(cont, x + arr->padding + wf, GET_WIDGET_H(cont));
								RESIZE_CONTAINER(_object, cont, has_expand_children ? GET_WIDGET_W(cont) : x + padding + wf, dmax + hf + padding * 2);
						}
						
						break;

					case ARRANGE_VERTICAL:
// 						#ifndef QNAMESPACE_H
// 						fprintf(stderr, "%s: VERTICAL: x = %d autoresize (%d, %d) pad %d spc %d [%d] rtl %d\n", 
// 						((gControl *)_object)->name(), x, x + arr->padding + wf, GET_WIDGET_H(cont), arr->padding, spacing, i, rtl);
// 						#else
// 						fprintf(stderr, "%s: VERTICAL: y = %d autoresize (%d, %d) pad %d spc %d [%d] rtl %d hec %d\n", 
// 							((CWIDGET *)_object)->name, y, dmax + wf, has_expand_children ? GET_WIDGET_H(cont) : y + arr->padding + hf, 
// 							arr->padding, spacing, i, rtl, has_expand_children);						
// 						#endif
						//RESIZE_WIDGET(cont, GET_WIDGET_W(cont), y + arr->padding + hf);
						if (dmax > 0)
							RESIZE_CONTAINER(_object, cont, dmax + wf + padding * 2, has_expand_children ? GET_WIDGET_H(cont) : y + padding + hf);
						break;

					case ARRANGE_COLUMN:
						if (rtl)
							RESIZE_CONTAINER(_object, cont, (wc - x) + w + padding + wf, GET_WIDGET_H(cont));
						else
							RESIZE_CONTAINER(_object, cont, x + w + padding + wf, GET_WIDGET_H(cont));
						break;

					case ARRANGE_ROW:
						//if ((y + h + arr->padding + GET_WIDGET_H(cont) - hc - yc) < 16)
						//	qDebug("y = %d h = %d arr->padding = %d H = %d hc = %d yc = %d -> %d", y, h, arr->padding, GET_WIDGET_H(cont), hc, yc, (y + h + arr->padding + GET_WIDGET_H(cont) - hc - yc));
						RESIZE_CONTAINER(_object, cont, GET_WIDGET_W(cont), y + h + padding + hf);
						break;
					
					case ARRANGE_FILL:
// 						#ifndef QNAMESPACE_H
// 						if (strncmp(((gControl *)_object)->name(), "DataControl", 11) == 0)
// 							fprintf(stderr, "%s: RESIZE_CONTAINER(%p, %p, %d, %d)\n", ((gControl *)_object)->name(), GET_WIDGET(_object), cont, w, h);
// 						#endif
						RESIZE_CONTAINER(_object, cont, w + padding * 2, h + padding * 2);
						break;
				}
			}

			if (!redo)
				break;
		}

	}

__RETURN:

	RAISE_ARRANGE_EVENT(_object);

	arr->locked = false;

	//qDebug("%p: dirty = FALSE", THIS);
	//arr->dirty = false;

	//qDebug("CCONTAINER_arrange: END %p", THIS);

	//qDebug("CCONTAINER_arrange: %p: END", cont);
}
