/***************************************************************************

  CWindow.h

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

#ifndef __CWINDOW_H
#define __CWINDOW_H

#include <QMoveEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMainWindow>
#include <QHash>
#include <QList>
#include <QMenuBar>
#include <QEvent>
#include <QPushButton>
#include <QSizeGrip>

#include "gambas.h"
#include "CContainer.h"
#include "CMenu.h"
#include "CPicture.h"

//#define DEBUG_WINDOW 1

//class MyCentral;

typedef
	struct CWINDOW {
		CWIDGET widget;
		QWidget *container;
		CARRANGEMENT arrangement;
		QMenuBar *menuBar;
		int ret;
		CPICTURE *icon;
		CPICTURE *picture;
		CWIDGET *focus;
		QPushButton *defaultButton;
		QPushButton *cancelButton;
		int loopLevel;
		int x;
		int y;
		int w;
		int h;
		int minw;
		int minh;
		unsigned toplevel : 1;
		unsigned embedded : 1;
		unsigned xembed : 1;
		unsigned stacking : 2;
		unsigned skipTaskbar : 1;
		unsigned masked : 1;
		unsigned reallyMasked : 1;
		unsigned opened : 1;
		unsigned hidden : 1;
		unsigned toolbar : 1;
		unsigned scale : 1;
		unsigned minsize : 1;
		unsigned title : 1;
		unsigned enterLoop : 1;
		unsigned stateChange : 1;
		unsigned opening : 1;
		unsigned closing : 1;
		unsigned hideMenuBar : 1;
		unsigned showMenuBar : 1;
		unsigned sticky : 1;
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
extern GB_DESC CWindowTypeDesc[];
extern GB_DESC CWindowsDesc[];
extern GB_DESC CFormDesc[];

extern CWINDOW *CWINDOW_Main;
extern CWINDOW *CWINDOW_Current;
extern CWINDOW *CWINDOW_Active;
extern CWINDOW *CWINDOW_LastActive;
extern int CWINDOW_Embedder;
extern bool CWINDOW_Embedded;

#else

#define THIS ((CWINDOW *)_object)
#define WIDGET ((QWidget *)(((CWIDGET *)_object)->widget))
#define WINDOW ((MyMainWindow *)WIDGET)
#define XEMBED ((QX11EmbedWidget *)(WIDGET->parent()))

#endif

class CWindow : public QObject
{
	Q_OBJECT

public:

	static QList<CWINDOW *> list;

	static CWindow manager;
	static int count;

	static void insertTopLevel(CWINDOW *_object);
	static void removeTopLevel(CWINDOW *_object);

protected:

	bool eventFilter(QObject *, QEvent *);

public slots:

	void error(void);
	void embedded(void);
	void closed(void);
	void destroy(void);
};


class MyMainWindow : public QWidget
{
	Q_OBJECT

private:

	QSizeGrip *sg;
	QMenuBar *mb;
	//bool shown;
	//int border;
	//int state;
	bool mustCenter;
	bool _activate;
	bool _border;
	bool _resizable;
	bool _deleted;
	int _type;

	void doReparent(QWidget *, Qt::WFlags, const QPoint &);

protected:

	//bool event(QEvent *);
	virtual void showEvent(QShowEvent *);
	//void hideEvent(QHideEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void moveEvent(QMoveEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void closeEvent(QCloseEvent *);
	virtual void changeEvent(QEvent *);

	//bool eventFilter(QObject *, QEvent *);

public:

	enum { BorderNone = 0, BorderFixed = 1, BorderResizable = 2 };
	QHash<QString, CWIDGET *> names;
	void *_object;

	MyMainWindow(QWidget *parent, const char *name, bool embedded = false);
	~MyMainWindow();

	void initProperties();
	void showActivate(QWidget *parent = 0);
	//void activateLater() { _activate = true; }
	void showModal();
	void showPopup();
	void afterShow();
	//bool isModal() { return testWFlags(WShowModal); }
	void doReparent(QWidget *w, const QPoint &p) { doReparent(w, windowFlags(), p); }

	bool hasBorder(void) { return _border; }
	void setBorder(bool, bool = false);
	
	bool isResizable(void) { return _resizable; }
	void setResizable(bool, bool = false);
	
	#ifdef NO_X_WINDOW
	#else
	int getType(void);
	void setType(int type);
	#endif
	
	//bool getTool(void) { return testWFlags(WStyle_Tool); }
	//void setTool(bool);
	bool isToolbar(void);
	
	void setSizeGrip(bool);
	void moveSizeGrip();

	void paintUnclip(bool);
	bool isPersistent(void);
	void setPersistent(bool);

	void center(bool);
	void configure(void);
	
	void setName(const char *, CWIDGET *);
	
	virtual void resize(int w, int h);
	virtual void setGeometry(int x, int y, int w, int h);
};


//void CWINDOW_set_top_only(QWidget *w, bool top);
void CWINDOW_activate(CWIDGET *ob);
void CWINDOW_set_default_button(CWINDOW *win, QPushButton *button, bool on);
void CWINDOW_set_cancel_button(CWINDOW *win, QPushButton *button, bool on);
void CWINDOW_define_mask(CWINDOW *_object);
void CWINDOW_fix_menubar(CWINDOW *window);

#endif
