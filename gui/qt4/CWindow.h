/***************************************************************************

  CWindow.h

  (c) 2000-2017 Benoît Minisini <gambas@users.sourceforge.net>

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
#include <QPoint>
#include <QPointer>

#include "gambas.h"
#include "CContainer.h"
#include "CMenu.h"
#include "CPicture.h"

//#define DEBUG_WINDOW 1

typedef
	struct CWINDOW {
		CWIDGET widget;
		MyContainer *container;
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
		int last_resize_w;
		int last_resize_h;
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
		unsigned stateChange : 1;
		unsigned closing : 1;
		unsigned hideMenuBar : 1;
		unsigned showMenuBar : 1;
		unsigned sticky : 1;
		unsigned noTakeFocus : 1;
		unsigned moved : 1;
		unsigned popup : 1;
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
//extern GB_DESC CWindowTypeDesc[];
extern GB_DESC CWindowsDesc[];
extern GB_DESC CFormDesc[];

extern CWINDOW *CWINDOW_Main;
extern int CWINDOW_MainDesktop;
extern CWINDOW *CWINDOW_Current;
extern CWINDOW *CWINDOW_Active;
extern CWINDOW *CWINDOW_LastActive;
#ifndef QT5
extern int CWINDOW_Embedder;
extern bool CWINDOW_Embedded;
#endif

#else

#define THIS ((CWINDOW *)_object)
#define WIDGET ((QWidget *)(((CWIDGET *)_object)->widget))
#define WINDOW ((MyMainWindow *)WIDGET)

#ifndef QT5
#define XEMBED ((QX11EmbedWidget *)(WIDGET->parent()))
#endif

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
	static CMENU *findMenu(CWINDOW *_object, const char *name);

protected:

	bool eventFilter(QObject *, QEvent *);

public slots:

	void error(void);
	void embedded(void);
	void closed(void);
	void destroy(void);
};

class MyMainWindow;

typedef
	struct {
		QPointer<MyMainWindow> that;
		QEventLoop *old;
		CWINDOW *save;
	}
	MODAL_INFO;

enum { PROP_ALL = -1, PROP_STACKING = 1, PROP_SKIP_TASKBAR = 2, PROP_BORDER = 4, PROP_STICKY = 8 };
	
class MyMainWindow : public QWidget
{
	Q_OBJECT

private:

	QSizeGrip *sg;
	QMenuBar *mb;
	bool _activate;
	bool _border;
	bool _resizable;
	bool _deleted;
	bool _enterLoop;
	bool _utility;
	int _type;
	Qt::WindowStates _state;
	int _screen;

protected:

	virtual void showEvent(QShowEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual void moveEvent(QMoveEvent *);
	virtual void keyPressEvent(QKeyEvent *);
	virtual void closeEvent(QCloseEvent *);
	virtual void changeEvent(QEvent *);

	//bool eventFilter(QObject *, QEvent *);

public slots:
	
	void activateLater();
	
public:

	enum { BorderNone = 0, BorderFixed = 1, BorderResizable = 2 };
	QHash<QString, CWIDGET *> names;
	void *_object;

	MyMainWindow(QWidget *parent, const char *name, bool embedded = false);
	~MyMainWindow();
	
	virtual void setVisible(bool visible);

	void initProperties(int which);
	void present(QWidget *parent = 0);
	void showActivate(QWidget *parent = 0);
	//void activateLater() { _activate = true; }
	void showModal();
	void showPopup(QPoint &pos);
	void setEventLoop();
	//bool isModal() { return testWFlags(WShowModal); }
	void doReparent(QWidget *w, const QPoint &p);

	bool hasBorder(void) const { return _border; }
	void setBorder(bool);
	
	bool isResizable(void) const { return _resizable; }
	void setResizable(bool);
	
	#ifdef NO_X_WINDOW
	#else
	int getType(void);
	void setType(int type);
	#endif
	
	//bool getTool(void) { return testWFlags(WStyle_Tool); }
	//void setTool(bool);
	bool isUtility(void) const { return _utility; }
	void setUtility(bool b);
	
	void setSizeGrip(bool);
	void moveSizeGrip();

	bool isPersistent(void);
	void setPersistent(bool);

	void center();
	void configure();
	
	void setName(const char *, CWIDGET *);
	
	void setState(Qt::WindowStates state);
	Qt::WindowStates getState() const;
	
	void setBetterMask(QPixmap &bg);
	
	int currentScreen() const;
	
	virtual void resize(int w, int h);
	virtual void setGeometry(int x, int y, int w, int h);
	
	friend void on_error_show_modal(MODAL_INFO *info);
};


//void CWINDOW_set_top_only(QWidget *w, bool top);
void CWINDOW_activate(CWIDGET *ob);
void CWINDOW_set_default_button(CWINDOW *win, QPushButton *button, bool on);
void CWINDOW_set_cancel_button(CWINDOW *win, QPushButton *button, bool on);
void CWINDOW_define_mask(CWINDOW *_object);
void CWINDOW_ensure_active_window();
bool CWINDOW_must_quit();
bool CWINDOW_close_all(bool main);
void CWINDOW_delete_all(bool main);
//void CWINDOW_fix_menubar(CWINDOW *window);

#endif
