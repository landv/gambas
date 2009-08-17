/***************************************************************************

  CDialog.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

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

#define __CDIALOG_CPP

#include <qcolor.h>
#include <qapplication.h>
#include <qcolordialog.h>
#include <qfontdialog.h>
#include <QFileDialog>

#include "gambas.h"
#include "CColor.h"
#include "CFont.h"

#include "CDialog.h"

static QString dialog_title;
static GB_ARRAY dialog_filter = NULL;
static QString dialog_path;
static GB_ARRAY dialog_paths = NULL;
static CFONT *dialog_font = NULL;

static unsigned int dialog_color = 0;


static QString get_filter(void)
{
  QString s;
  int i;
  QString filter;

  if (dialog_filter)
  {
    for (i = 0; i < (GB.Array.Count(dialog_filter) / 2); i++)
    {
    	filter = TO_QSTRING(*((char **)GB.Array.Get(dialog_filter, i * 2)));
    	if (filter == "*")
    		continue;
    
      if (s.length())
        s += ";;";
      s += TO_QSTRING(*((char **)GB.Array.Get(dialog_filter, i * 2 + 1)));
      s += " (" + filter + ")";
    }
    
    s += ";;";
    s += TO_QSTRING(GB.Translate("All files"));
    s += " (*)";
  }

  return s;
}


BEGIN_METHOD_VOID(CDIALOG_exit)

	GB.StoreObject(NULL, POINTER(&dialog_filter));
	GB.StoreObject(NULL, POINTER(&dialog_paths));
	GB.StoreObject(NULL, POINTER(&dialog_font));

END_METHOD


BEGIN_PROPERTY(CDIALOG_title)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(dialog_title));
  else
    dialog_title = QSTRING_PROP();

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_filter)

  if (READ_PROPERTY)
    GB.ReturnObject(dialog_filter);
  else
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&dialog_filter));

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_path)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(dialog_path));
  else
    dialog_path = QSTRING_PROP();

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_paths)

  GB.ReturnObject(dialog_paths);

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_font)

  if (READ_PROPERTY)
		GB.ReturnObject(dialog_font);
  else
  {
		CFONT *font = (CFONT *)VPROP(GB_OBJECT);
		
		GB.StoreObject(NULL, POINTER(&dialog_font));
		if (font)
			dialog_font = CFONT_create(*font->font);
  }

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_color)

  if (READ_PROPERTY)
    GB.ReturnInteger(dialog_color);
  else
    dialog_color = VPROP(GB_INTEGER);

END_PROPERTY


BEGIN_METHOD(CDIALOG_open_file, GB_BOOLEAN multi)

  if (!VARGOPT(multi, false))
  {
    QString file;
  
    file = QFileDialog::getOpenFileName(qApp->activeWindow(), dialog_title, dialog_path, get_filter());

    if (file.isNull())
      GB.ReturnBoolean(true);
    else
    {
      dialog_path = file;
      GB.ReturnBoolean(false);
    }
  }
  else
  {
    QStringList files;
    GB_ARRAY list;
    GB_OBJECT ob;
    int i;

    files = QFileDialog::getOpenFileNames(qApp->activeWindow(), dialog_title, dialog_path, get_filter());

    if (files.isEmpty())
    {
      GB.StoreObject(NULL, POINTER(&dialog_paths));
      GB.ReturnBoolean(true);
    }
    else
    {
      GB.Array.New(&list, GB_T_STRING, files.count());
      ob.value = list;
      GB.StoreObject(&ob, POINTER(&dialog_paths));
      
      for (i = 0; i < files.count(); i++)
        GB.NewString((char **)GB.Array.Get(list, i), TO_UTF8(files[i]), 0);
      
      GB.ReturnBoolean(false);
    }
  }
  
  dialog_title = QString::null;

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_save_file)

  QString file;

  file = QFileDialog::getSaveFileName(qApp->activeWindow(), dialog_title, dialog_path, get_filter());

  if (file.isNull())
    GB.ReturnBoolean(true);
  else
  {
    dialog_path = file;
    GB.ReturnBoolean(false);
  }
  
  dialog_title = QString::null;

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_get_directory)

  QString file;

  file = QFileDialog::getExistingDirectory(qApp->activeWindow(), dialog_title, dialog_path);

  if (file.isNull())
    GB.ReturnBoolean(true);
  else
  {
    dialog_path = file;
    GB.ReturnBoolean(false);
  }

  dialog_title = QString::null;

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_get_color)

  QColor color;

  color = QColorDialog::getColor(dialog_color, qApp->activeWindow());

  if (!color.isValid())
    GB.ReturnBoolean(true);
  else
  {
    dialog_color = color.rgb() & 0xFFFFFF;
    GB.ReturnBoolean(false);
  }

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_select_font)

  QFont qfont;
  bool ok;
  #ifdef USE_DPI
  int dpiX, dpiY;
  #endif

	if (dialog_font)
		qfont = *dialog_font->font;
	else
		qfont = QApplication::font();
  //qDebug("AVANT: %g --> %g", qfont.pointSizeFloat(), SIZE_REAL_TO_VIRTUAL(qfont.pointSizeFloat()));
  qfont.setPointSizeF(SIZE_REAL_TO_VIRTUAL(qfont.pointSizeF()));
  
  #ifdef USE_DPI
  dpiX = QPaintDevice::x11AppDpiX();
  dpiY = QPaintDevice::x11AppDpiY();
  QPaintDevice::x11SetAppDpiX(CFONT_dpi);
  QPaintDevice::x11SetAppDpiY(CFONT_dpi);
  #endif
  
  qfont = QFontDialog::getFont(&ok, qfont, qApp->activeWindow());

  #ifdef USE_DPI
  QPaintDevice::x11SetAppDpiX(dpiX);
  QPaintDevice::x11SetAppDpiY(dpiY);
  #endif
  
  //qDebug("APRES: %g --> %g", qfont.pointSizeFloat(), SIZE_VIRTUAL_TO_REAL(qfont.pointSizeFloat()));
  qfont.setPointSizeF(SIZE_VIRTUAL_TO_REAL(qfont.pointSizeF()));
  
  if (!ok)
    GB.ReturnBoolean(true);
  else
  {
		GB.StoreObject(NULL, POINTER(&dialog_font));
		dialog_font = CFONT_create(qfont);
    GB.ReturnBoolean(false);
  }

END_METHOD

GB_DESC CDialogDesc[] =
{
  GB_DECLARE("Dialog", 0), GB_VIRTUAL_CLASS(),

  //GB_STATIC_METHOD("_init", NULL, CDIALOG_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CDIALOG_exit, NULL),

  GB_STATIC_METHOD("OpenFile", "b", CDIALOG_open_file, "[(Multi)b]"),
  GB_STATIC_METHOD("SaveFile", "b", CDIALOG_save_file, NULL),
  GB_STATIC_METHOD("SelectDirectory", "b", CDIALOG_get_directory, NULL),
  GB_STATIC_METHOD("SelectColor", "b", CDIALOG_get_color, NULL),
  GB_STATIC_METHOD("SelectFont", "b", CDIALOG_select_font, NULL),

  GB_STATIC_PROPERTY("Title", "s", CDIALOG_title),
  GB_STATIC_PROPERTY("Path", "s", CDIALOG_path),
  GB_STATIC_PROPERTY_READ("Paths", "String[]", CDIALOG_paths),
  GB_STATIC_PROPERTY("Filter", "String[]", CDIALOG_filter),
  GB_STATIC_PROPERTY("Color", "i", CDIALOG_color),
  GB_STATIC_PROPERTY("Font", "Font", CDIALOG_font),

  GB_END_DECLARE
};


