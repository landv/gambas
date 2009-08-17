/***************************************************************************

  CMouse.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __CMOUSE_CPP

#include <qapplication.h>
#include <qpixmap.h>
#include <qcursor.h>
#include <qnamespace.h>

#include "gambas.h"
#include "main.h"

#include "CPicture.h"
#include "CMouse.h"

CMOUSE_INFO CMOUSE_info = { 0 };

void CMOUSE_clear(int valid)
{
  if (valid)
    CMOUSE_info.valid++;
  else
    CMOUSE_info.valid--;

  /*if (CMOUSE_info.valid == 0)
    CLEAR(&CMOUSE_info);*/
}


//int CMOUSE_last_state = 0;

//static CCURSOR PredefinedCursor[LastCursor + 1] = { { { 0, 0 }, NULL, NULL } };
//static int MouseClassID;

#if 0
static int translate_state(int s)
{
  int bst = 0;

  if (s & Button1Mask)
    bst |= Qt::LeftButton;
  if ( s & Button2Mask)
    bst |= Qt::MidButton;
  if ( s & Button3Mask)
    bst |= Qt::RightButton;
  if ( s & ShiftMask)
    bst |= Qt::ShiftModifier;
  if ( s & ControlMask)
    bst |= Qt::ControlModifier;
  if ( s & qt_alt_mask)
    bst |= Qt::AltModifier;
  if ( s & qt_meta_mask)
    bst |= Qt::MetaModifier;

  return bst;
}

static int get_state()
{
  Window root;
  Window child;
  int root_x, root_y, win_x, win_y;
  uint state;
  Display* dpy = QPaintDevice::x11AppDisplay();

  for (int i = 0; i < ScreenCount(dpy); i++)
  {
    if (XQueryPointer(dpy, RootWindow(dpy, i), &root, &child,
          &root_x, &root_y, &win_x, &win_y, &state))
      return translate_state(state);
  }

  return 0;
}
#endif

BEGIN_METHOD(CCURSOR_new, GB_OBJECT picture; GB_INTEGER x; GB_INTEGER y)

  CPICTURE *pict = (CPICTURE *)VARG(picture);

  THIS->x = VARGOPT(x, -1);
  THIS->y = VARGOPT(y, -1);

  //GB.StoreObject(ARG(picture), POINTER(&THIS->picture));
  if (GB.CheckObject(pict))
    return;
  
  if (THIS->x < 0 || THIS->x >= pict->pixmap->width())
    THIS->x = -1;
    
  if (THIS->y < 0 || THIS->y >= pict->pixmap->height())
    THIS->y = -1;
    
  THIS->cursor = new QCursor(*(pict->pixmap), THIS->x, THIS->y);

END_METHOD


BEGIN_METHOD_VOID(CCURSOR_delete)

  //GB.Unref(POINTER(&THIS->picture));
  delete THIS->cursor;

END_METHOD


/*BEGIN_PROPERTY(CCURSOR_picture)

  GB.ReturnObject(THIS->picture);

END_PROPERTY*/


BEGIN_PROPERTY(CCURSOR_x)

  GB.ReturnInteger(THIS->x);

END_PROPERTY


BEGIN_PROPERTY(CCURSOR_y)

  GB.ReturnInteger(THIS->y);

END_PROPERTY


// BEGIN_METHOD(CCURSOR_get, int shape)
//
//   int shape = PARAM(shape);
//   CCURSOR *p;
//
//   if (shape < 0 || shape > LastCursor)
//     GB.ReturnObject(NULL);
//
//   p = &PredefinedCursor[shape];
//   if (p->ob.klass == 0)
//   {
//     p->ob.klass = MouseClassID;
//     p->cursor = new QCursor(shape);
//     GB.Ref(p);
//   }
//
//   GB.ReturnObject(p);
//
// END_METHOD


BEGIN_PROPERTY(CMOUSE_screen_x)

  GB.ReturnInteger(QCursor::pos().x());

END_PROPERTY


BEGIN_PROPERTY(CMOUSE_screen_y)

  GB.ReturnInteger(QCursor::pos().y());

END_PROPERTY


BEGIN_METHOD(CMOUSE_move, GB_INTEGER x; GB_INTEGER y)

  QCursor::setPos(VARG(x), VARG(y));

END_PROPERTY

#define CHECK_VALID() \
  if (CMOUSE_info.valid == 0) \
  { \
    GB.Error("No mouse event data"); \
    return; \
  }

BEGIN_PROPERTY(CMOUSE_x)

  CHECK_VALID();
  GB.ReturnInteger(CMOUSE_info.x);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_y)

  CHECK_VALID();
  GB.ReturnInteger(CMOUSE_info.y);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_start_x)

  CHECK_VALID();
  GB.ReturnInteger(CMOUSE_info.sx);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_start_y)

  CHECK_VALID();
  GB.ReturnInteger(CMOUSE_info.sy);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_button)

  CHECK_VALID();
  GB.ReturnInteger((int)CMOUSE_info.button);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_left)

  CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.button & Qt::LeftButton);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_right)

  CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.button & Qt::RightButton);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_middle)

  CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.button & Qt::MidButton);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_shift)

  //CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.modifier & Qt::ShiftModifier);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_control)

  //CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.modifier & Qt::ControlModifier);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_alt)

  //CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.modifier & Qt::AltModifier);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_meta)

  //CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.modifier & Qt::MetaModifier);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_normal)

  //CHECK_VALID();
  GB.ReturnBoolean(CMOUSE_info.modifier == Qt::NoModifier);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_orientation)

  CHECK_VALID();
  GB.ReturnInteger(CMOUSE_info.orientation);

END_PROPERTY

BEGIN_PROPERTY(CMOUSE_delta)

  CHECK_VALID();
  GB.ReturnFloat((double)CMOUSE_info.delta / 120);

END_PROPERTY



GB_DESC CCursorDesc[] =
{
  GB_DECLARE("Cursor", sizeof(CCURSOR)),

  GB_METHOD("_new", NULL, CCURSOR_new, "(Picture)Picture;[(X)i(Y)i]"),
  GB_METHOD("_free", NULL, CCURSOR_delete, NULL),

  GB_PROPERTY_READ("X", "i", CCURSOR_x),
  GB_PROPERTY_READ("Y", "i", CCURSOR_y),
  //GB_PROPERTY_READ("Picture", "Picture", CCURSOR_picture),

  GB_END_DECLARE
};


GB_DESC CMouseDesc[] =
{
  GB_DECLARE("Mouse", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("ScreenX", "i", CMOUSE_screen_x),
  GB_STATIC_PROPERTY_READ("ScreenY", "i", CMOUSE_screen_y),
  //GB_STATIC_PROPERTY_READ("State", "i", CMOUSE_state),
  GB_STATIC_METHOD("Move", NULL, CMOUSE_move, "(X)i(Y)i"),

  GB_CONSTANT("Default", "i", CMOUSE_DEFAULT),
  GB_CONSTANT("Custom", "i", CMOUSE_CUSTOM),
  GB_CONSTANT("Blank", "i", Qt::BlankCursor),
  GB_CONSTANT("Arrow", "i", Qt::ArrowCursor),
  GB_CONSTANT("Cross", "i", Qt::CrossCursor),
  GB_CONSTANT("Wait", "i", Qt::WaitCursor),
  GB_CONSTANT("Text", "i", Qt::IBeamCursor),
  GB_CONSTANT("SizeAll", "i", Qt::SizeAllCursor),
  GB_CONSTANT("SizeH", "i", Qt::SizeHorCursor),
  GB_CONSTANT("SizeV", "i", Qt::SizeVerCursor),
  GB_CONSTANT("SizeN", "i", Qt::SizeVerCursor),
  GB_CONSTANT("SizeS", "i", Qt::SizeVerCursor),
  GB_CONSTANT("SizeW", "i", Qt::SizeHorCursor),
  GB_CONSTANT("SizeE", "i", Qt::SizeHorCursor),
  GB_CONSTANT("SizeNW", "i", Qt::SizeFDiagCursor),
  GB_CONSTANT("SizeSE", "i", Qt::SizeFDiagCursor),
  GB_CONSTANT("SizeNE", "i", Qt::SizeBDiagCursor),
  GB_CONSTANT("SizeSW", "i", Qt::SizeBDiagCursor),
  GB_CONSTANT("SizeNWSE", "i", Qt::SizeFDiagCursor),
  GB_CONSTANT("SizeNESW", "i", Qt::SizeBDiagCursor),
  GB_CONSTANT("SplitH", "i", Qt::SplitHCursor),
  GB_CONSTANT("SplitV", "i", Qt::SplitVCursor),
  GB_CONSTANT("Pointing", "i", Qt::PointingHandCursor),

  //GB_CONSTANT("Left", "i", Qt::LeftButton),
  //GB_CONSTANT("Right", "i", Qt::RightButton),
  //GB_CONSTANT("Middle", "i", Qt::MidButton),
  //GB_CONSTANT("Shift", "i", Qt::ShiftButton),
  //GB_CONSTANT("Control", "i", Qt::ControlButton),
  //GB_CONSTANT("Alt", "i", Qt::AltButton),
  GB_CONSTANT("Horizontal", "i", Qt::Horizontal),
  GB_CONSTANT("Vertical", "i", Qt::Vertical),

  GB_STATIC_PROPERTY_READ("X", "i", CMOUSE_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CMOUSE_y),
  GB_STATIC_PROPERTY_READ("StartX", "i", CMOUSE_start_x),
  GB_STATIC_PROPERTY_READ("StartY", "i", CMOUSE_start_y),
  GB_STATIC_PROPERTY_READ("Left", "b", CMOUSE_left),
  GB_STATIC_PROPERTY_READ("Right", "b", CMOUSE_right),
  GB_STATIC_PROPERTY_READ("Middle", "b", CMOUSE_middle),
  GB_STATIC_PROPERTY_READ("Button", "i", CMOUSE_button),
  GB_STATIC_PROPERTY_READ("Shift", "b", CMOUSE_shift),
  GB_STATIC_PROPERTY_READ("Control", "b", CMOUSE_control),
  GB_STATIC_PROPERTY_READ("Alt", "b", CMOUSE_alt),
  GB_STATIC_PROPERTY_READ("Meta", "b", CMOUSE_meta),
  GB_STATIC_PROPERTY_READ("Normal", "b", CMOUSE_normal),
  GB_STATIC_PROPERTY_READ("Orientation", "i", CMOUSE_orientation),
  GB_STATIC_PROPERTY_READ("Delta", "f", CMOUSE_delta),

  GB_END_DECLARE
};

