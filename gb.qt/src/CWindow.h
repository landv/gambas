/***************************************************************************

  CWindow.h

  The Window and Form classes

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

#ifndef __CWINDOW_H
#define __CWINDOW_H

#include <qmainwindow.h>
#include <qasciidict.h>
#include <qmenubar.h>
#include <qptrdict.h>
#include <qevent.h>
#include <qpushbutton.h>
#include <qsizegrip.h>

#include "gambas.h"
#include "CContainer.h"
#include "CMenu.h"
#include "CPicture.h"

//class MyCentral;

typedef
  struct CWINDOW {
    CWIDGET widget;
    QWidget *container;
    CARRANGEMENT arrangement;
    //QAsciiDict<void> *dict;
    QMenuBar *menuBar;
    CMenuList *menu;
    long ret;
    CPICTURE *icon;
    CPICTURE *picture;
    CWIDGET *focus;
	  QPushButton *defaultButton;
  	QPushButton *cancelButton;
    QObjectList *controls;
    long x;
    long y;
    long w;
    long h;
    unsigned toplevel : 1;
    unsigned embedded : 1;
    unsigned xembed : 1;
    unsigned stacking : 2;
    unsigned skipTaskbar : 1;
    unsigned masked : 1;
    unsigned shown : 1;
    unsigned hidden : 1;
    unsigned toolbar : 1;
    unsigned scale : 1;
    unsigned minsize : 1;
    }
  CWINDOW;

typedef
  struct {
    CWINDOW window;
    }
  CFORM;


#ifndef __CWINDOW_CPP

extern GB_DESC CWindowDesc[];
extern GB_DESC CWindowMenusDesc[];
extern GB_DESC CWindowControlsDesc[];
//extern GB_DESC CWindowToolBarsDesc[];
extern GB_DESC CWindowsDesc[];
extern GB_DESC CFormDesc[];

extern CWINDOW *CWINDOW_Main;
extern CWINDOW *CWINDOW_Current;
extern CWINDOW *CWINDOW_Active;
extern long CWINDOW_Embedder;
extern bool CWINDOW_Embedded;

#else

#define THIS ((CWINDOW *)_object)
#define WIDGET ((QWidget *)(((CWIDGET *)_object)->widget))
#define WINDOW ((MyMainWindow *)WIDGET)
#define XEMBED ((QtXEmbedClient *)WIDGET)

/*typedef
	struct {
		int type;
		long index;
		}
	MENU_ENUM;

typedef
	struct {
		int type;
		void *control;
		}
	CONTROL_ENUM;

#define ENUM_MENU 1
#define ENUM_CONTROL 2
*/

#endif

class CWindow : public QObject
{
  Q_OBJECT

public:

  static QPtrDict<CWINDOW> dict;

  static CWindow manager;
  static int count;

protected:

  bool eventFilter(QObject *, QEvent *);

public slots:

  void error(void);
  void embedded(void);
  void closed(void);
  void destroy(void);
};


class MyMainWindow : public QMainWindow
{
  Q_OBJECT

private:

  QSizeGrip *sg;
  //bool shown;
  int border;
  //int state;
  bool mustCenter;
  int loopLevel;
  bool _activate;

  void doReparent(QWidget *, WFlags, const QPoint &, bool showIt = false);

protected:

	//bool event(QEvent *);
  void showEvent(QShowEvent *);
  //void hideEvent(QHideEvent *);
  void resizeEvent(QResizeEvent *);
  void moveEvent(QMoveEvent *);
  void keyPressEvent(QKeyEvent *);
  void closeEvent(QCloseEvent *);

  //bool eventFilter(QObject *, QEvent *);

public:

  enum { BorderNone = 0, BorderFixed = 1, BorderResizable = 2 };
  QAsciiDict<CWIDGET> names;

  MyMainWindow(QWidget *parent, const char *name, bool embedded = false);
  ~MyMainWindow();

	void hide();
  void initProperties();
  void showActivate();
  void showModal();
  void showPopup();
  bool isModal() { return testWFlags(WShowModal); }
  int level() { return loopLevel; }
  void doReparent(QWidget *w, const QPoint &p, bool showIt = false) { doReparent(w, getWFlags(), p, showIt); }

  int getBorder(void) { return border; }
  void setBorder(int, bool = false);
  bool getTool(void) { return testWFlags(WStyle_Tool); }
  void setTool(bool);
  void setSizeGrip(bool);
  void moveSizeGrip();

  void paintUnclip(bool);
  bool isPersistent(void);
  void setPersistent(bool);

  void center(bool);
  void configure(void);
  
  void setName(const char *, CWIDGET *);
};


//void CWINDOW_set_top_only(QWidget *w, bool top);
void CWINDOW_activate(CWIDGET *ob);
void CWINDOW_set_default_button(CWINDOW *win, QPushButton *button, bool on);
void CWINDOW_set_cancel_button(CWINDOW *win, QPushButton *button, bool on);


#endif
