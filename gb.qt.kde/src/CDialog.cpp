/***************************************************************************

  CDialog.cpp

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

#define __CDIALOG_CPP

#include <kcolordialog.h>
#include <kfontdialog.h>
#include <kfiledialog.h>

#include "gambas.h"
#include "main.h"

#include "CDialog.h"


static QString dialog_title;
static unsigned long dialog_color = 0;
static QFont dialog_font;
static GB_ARRAY dialog_filter = NULL;
static QString dialog_path;
static GB_ARRAY dialog_paths = NULL;

static QString get_filter(void)
{
  QString s;
  QString f, ext, extp;
  int i;

  if (dialog_filter)
  {
    for (i = 0; i < (GB.Array.Count(dialog_filter) / 2); i++)
    {
		  extp = ext = TO_QSTRING(*((char **)GB.Array.Get(dialog_filter, i * 2)));
		  if (ext == "*")
		  	continue;
      if (s.length())
        s += "\n";
		  extp.replace(";", " "); 
      s += extp;
      s += "|";
      s += TO_QSTRING(*((char **)GB.Array.Get(dialog_filter, i * 2 + 1)));
      s += " (" + ext + ")";
    }
    
    s += "\n*|";
    s += TO_QSTRING(GB.Translate("All files"));
    s += " (*)";
  }

  //qDebug("%s", s.latin1());
  
  return s;
}


BEGIN_METHOD_VOID(CDIALOG_exit)

  GB.StoreObject(NULL, (void **)&dialog_filter);
  GB.StoreObject(NULL, (void **)&dialog_paths);

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
    GB.StoreObject(PROP(GB_OBJECT), (void **)&dialog_filter);

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
    GB.ReturnObject(QT.CreateFont(dialog_font, NULL, NULL));
  else
  {
    if (GB.CheckObject(VPROP(GB_OBJECT)))
      return;
  
    dialog_font = *(((QT_FONT *)VPROP(GB_OBJECT))->font);
  }

END_PROPERTY


BEGIN_PROPERTY(CDIALOG_color)

  if (READ_PROPERTY)
    GB.ReturnInteger((long)dialog_color);
  else
    dialog_color = VPROP(GB_INTEGER);

END_PROPERTY


BEGIN_METHOD(CDIALOG_open_file, GB_BOOLEAN multi)

  if (!VARGOPT(multi, false))
  {
    QString file;
  
    file = KFileDialog::getOpenFileName(dialog_path, get_filter(), qApp->activeWindow(), dialog_title);
    //file = QFileDialog::getOpenFileName(dialog_path, get_filter(), qApp->activeWindow(), 0, dialog_title);

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
    unsigned int i;

    files = KFileDialog::getOpenFileNames(dialog_path, get_filter(), qApp->activeWindow(), dialog_title);
    //files = QFileDialog::getOpenFileNames(get_filter(), dialog_path, qApp->activeWindow(), 0, dialog_title);

    if (files.isEmpty())
    {
      GB.StoreObject(NULL, (void **)&dialog_paths);
      GB.ReturnBoolean(true);
    }
    else
    {
      GB.Array.New(&list, GB_T_STRING, files.count());
      ob.value = list;
      GB.StoreObject(&ob, (void **)&dialog_paths);
      
      for (i = 0; i < files.count(); i++)
        GB.NewString((char **)GB.Array.Get(list, i), TO_UTF8(files[i]), 0);
      
      GB.ReturnBoolean(false);
    }
  }
  
  dialog_title = QString::null;

END_METHOD





BEGIN_METHOD_VOID(CDIALOG_save_file)

  QString file;

  file = KFileDialog::getSaveFileName(dialog_path, get_filter(), qApp->activeWindow(), dialog_title);

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

  file = KFileDialog::getExistingDirectory(dialog_path, qApp->activeWindow(), dialog_title);

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

  QColor color(dialog_color);
  int result;

  result = KColorDialog::getColor(color, qApp->activeWindow());

  if (result != KColorDialog::Accepted)
    GB.ReturnBoolean(true);
  else
  {
    dialog_color = color.rgb() & 0xFFFFFF;
    GB.ReturnBoolean(false);
  }

END_METHOD


BEGIN_METHOD_VOID(CDIALOG_select_font)

  int result = KFontDialog::getFont(dialog_font, qApp->activeWindow());

  if (result != KFontDialog::Accepted)
    GB.ReturnBoolean(true);
  else
    GB.ReturnBoolean(false);

END_METHOD


GB_DESC CDialogDesc[] =
{
  GB_DECLARE("Dialog", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_exit", NULL, CDIALOG_exit, NULL),

  GB_STATIC_METHOD("OpenFile", "b", CDIALOG_open_file, "[(Multi)b]"),
  GB_STATIC_METHOD("SaveFile", "b", CDIALOG_save_file, NULL),
  GB_STATIC_METHOD("SelectDirectory", "b", CDIALOG_get_directory, NULL),

  GB_STATIC_METHOD("SelectColor", "b", CDIALOG_get_color, NULL),
  //GB_STATIC_METHOD("SelectFont", "b", CDIALOG_select_font, NULL),

  GB_STATIC_PROPERTY("Title", "s", CDIALOG_title),
  GB_STATIC_PROPERTY("Path", "s", CDIALOG_path),
  GB_STATIC_PROPERTY_READ("Paths", "String[]", CDIALOG_paths),
  GB_STATIC_PROPERTY("Filter", "String[]", CDIALOG_filter),
  GB_STATIC_PROPERTY("Color", "i", CDIALOG_color),
  //GB_STATIC_PROPERTY("Font", "Font", CDIALOG_font),

  GB_END_DECLARE
};


