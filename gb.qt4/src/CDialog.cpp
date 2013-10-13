/***************************************************************************

  CDialog.cpp

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
static bool dialog_show_hidden = false;

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
      s += " (" + filter.replace(";", " ") + ")";
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


BEGIN_PROPERTY(Dialog_Title)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(dialog_title));
  else
    dialog_title = QSTRING_PROP();

END_PROPERTY


BEGIN_PROPERTY(Dialog_Filter)

  if (READ_PROPERTY)
    GB.ReturnObject(dialog_filter);
  else
    GB.StoreObject(PROP(GB_OBJECT), POINTER(&dialog_filter));

END_PROPERTY


BEGIN_PROPERTY(Dialog_ShowHidden)

	if (READ_PROPERTY)
		GB.ReturnBoolean(dialog_show_hidden);
	else
		dialog_show_hidden = VPROP(GB_BOOLEAN);

END_PROPERTY


BEGIN_PROPERTY(Dialog_Path)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(dialog_path));
  else
    dialog_path = QSTRING_PROP();

END_PROPERTY


BEGIN_PROPERTY(Dialog_Paths)

  GB.ReturnObject(dialog_paths);

END_PROPERTY


BEGIN_PROPERTY(Dialog_Font)

  if (READ_PROPERTY)
		GB.ReturnObject(dialog_font);
  else
  {
		CFONT *font = (CFONT *)VPROP(GB_OBJECT);
		
		GB.StoreObject(NULL, POINTER(&dialog_font));
		if (font)
		{
			dialog_font = CFONT_create(*font->font);
			GB.Ref(dialog_font);
		}
  }

END_PROPERTY


BEGIN_PROPERTY(Dialog_Color)

  if (READ_PROPERTY)
    GB.ReturnInteger(dialog_color);
  else
    dialog_color = VPROP(GB_INTEGER);

END_PROPERTY


static QString my_getOpenFileName()
{
	QFileDialog dialog(qApp->activeWindow(), dialog_title, dialog_path, get_filter());
	
	dialog.setMode(QFileDialog::ExistingFile);
	dialog.setOption(QFileDialog::DontUseNativeDialog);
	dialog.setFilter(dialog_show_hidden ? (dialog.filter() | QDir::Hidden | QDir::System) : (dialog.filter() & ~(QDir::Hidden | QDir::System)));
	
	if (dialog.exec() == QDialog::Accepted)
		return dialog.selectedFiles().value(0);
	else
		return QString();
}

static QStringList my_getOpenFileNames()
{
	QFileDialog dialog(qApp->activeWindow(), dialog_title, dialog_path, get_filter());
	
	dialog.setMode(QFileDialog::ExistingFiles);
	dialog.setOption(QFileDialog::DontUseNativeDialog);
	dialog.setFilter(dialog_show_hidden ? (dialog.filter() | QDir::Hidden | QDir::System) : (dialog.filter() & ~(QDir::Hidden | QDir::System)));
	
	if (dialog.exec() == QDialog::Accepted)
		return dialog.selectedFiles();
	else
		return QStringList();
}

static QString my_getSaveFileName()
{
	QString dir, file;

	dir = dialog_path;
	if (!dialog_path.endsWith('/'))
	{
		int pos = dialog_path.lastIndexOf('/');
		if (pos >= 0)
		{
			dir = dialog_path.left(pos);
			file = dialog_path.mid(pos + 1);
		}
	}

	QFileDialog dialog(qApp->activeWindow(), dialog_title, dir, get_filter());
	dialog.selectFile(file);
	
  dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setMode(QFileDialog::AnyFile);
	dialog.setOption(QFileDialog::DontUseNativeDialog);
	dialog.setFilter(dialog_show_hidden ? (dialog.filter() | QDir::Hidden | QDir::System) : (dialog.filter() & ~(QDir::Hidden | QDir::System)));

	if (dialog.exec() == QDialog::Accepted) 
		return dialog.selectedFiles().value(0);
	else
		return QString();
}

static QString my_getExistingDirectory()
{
	QFileDialog dialog(qApp->activeWindow(), dialog_title, dialog_path);
	
	dialog.setMode(QFileDialog::Directory);
	dialog.setOption(QFileDialog::DontUseNativeDialog);

	if (dialog.exec() == QDialog::Accepted) 
		return dialog.selectedFiles().value(0);
	else
		return QString();
}


BEGIN_METHOD(Dialog_OpenFile, GB_BOOLEAN multi)

  if (!VARGOPT(multi, false))
  {
    QString file;
  
    file = my_getOpenFileName();

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

    files = my_getOpenFileNames();

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
        *(char **)GB.Array.Get(list, i) = GB.NewZeroString(TO_UTF8(files[i]));
      
      GB.ReturnBoolean(false);
    }
  }
  
  dialog_title = QString::null;

END_METHOD


BEGIN_METHOD_VOID(Dialog_SaveFile)

  QString file;

  file = my_getSaveFileName();

  if (file.isNull())
    GB.ReturnBoolean(true);
  else
  {
    dialog_path = file;
    GB.ReturnBoolean(false);
  }
  
  dialog_title = QString::null;

END_METHOD


BEGIN_METHOD_VOID(Dialog_SelectDirectory)

  QString file;

  file = my_getExistingDirectory();

  if (file.isNull())
    GB.ReturnBoolean(true);
  else
  {
    dialog_path = file;
    GB.ReturnBoolean(false);
  }

  dialog_title = QString::null;

END_METHOD


BEGIN_METHOD_VOID(Dialog_SelectColor)

  QColor color;

  color = QColorDialog::getColor(dialog_color, qApp->activeWindow(), dialog_title); //, qApp->activeWindow());

  if (!color.isValid())
    GB.ReturnBoolean(true);
  else
  {
    dialog_color = color.rgb() & 0xFFFFFF;
    GB.ReturnBoolean(false);
  }

END_METHOD


BEGIN_METHOD_VOID(Dialog_SelectFont)

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
  
  qfont = QFontDialog::getFont(&ok, qfont, qApp->activeWindow(), dialog_title);

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
		GB.Ref(dialog_font);
    GB.ReturnBoolean(false);
  }

END_METHOD

GB_DESC CDialogDesc[] =
{
  GB_DECLARE("Dialog", 0), GB_VIRTUAL_CLASS(),

  //GB_STATIC_METHOD("_init", NULL, CDIALOG_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CDIALOG_exit, NULL),

  GB_STATIC_METHOD("OpenFile", "b", Dialog_OpenFile, "[(Multi)b]"),
  GB_STATIC_METHOD("SaveFile", "b", Dialog_SaveFile, NULL),
  GB_STATIC_METHOD("SelectDirectory", "b", Dialog_SelectDirectory, NULL),
  GB_STATIC_METHOD("SelectColor", "b", Dialog_SelectColor, NULL),
  GB_STATIC_METHOD("SelectFont", "b", Dialog_SelectFont, NULL),

  GB_STATIC_PROPERTY("Title", "s", Dialog_Title),
  GB_STATIC_PROPERTY("Path", "s", Dialog_Path),
  GB_STATIC_PROPERTY_READ("Paths", "String[]", Dialog_Paths),
  GB_STATIC_PROPERTY("Filter", "String[]", Dialog_Filter),
  GB_STATIC_PROPERTY("Color", "i", Dialog_Color),
  GB_STATIC_PROPERTY("Font", "Font", Dialog_Font),
  GB_STATIC_PROPERTY("ShowHidden", "b", Dialog_ShowHidden),

  GB_END_DECLARE
};


