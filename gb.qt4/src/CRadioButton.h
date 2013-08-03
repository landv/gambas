/***************************************************************************

  CRadioButton.h

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

#ifndef __CRADIOBUTTON_H
#define __CRADIOBUTTON_H

#include "gambas.h"

#include <QRadioButton>

#include "CWidget.h"

#ifndef __CRADIOBUTTON_CPP
extern GB_DESC CRadioButtonDesc[];
#else

#define THIS    ((CRADIOBUTTON *)_object)
#define WIDGET  ((MyRadioButton *)((CWIDGET *)_object)->widget)

#endif

typedef
  struct {
    CWIDGET widget;
    }
  CRADIOBUTTON;

class MyRadioButton : public QRadioButton
{
	Q_OBJECT
	
public:

  MyRadioButton(QWidget *parent);
  void adjust(bool force = false);
  bool isAutoResize() const { return _autoResize; }
  void setAutoResize(bool a) { _autoResize = a; adjust(); }
	
protected:

  virtual void changeEvent(QEvent *);
  virtual void resizeEvent(QResizeEvent *);

private:

	unsigned _autoResize : 1;
};
	
class CRadioButton : public QObject
{
  Q_OBJECT

public:

  static CRadioButton manager;

public slots:

  void clicked(bool on);

};

#endif
