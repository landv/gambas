/***************************************************************************

  qtxembed.h


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
#ifndef QTXEMBED_H
#define QTXEMBED_H
#include <qguardedptr.h>
#include <qwidget.h>
#include <X11/Xlib.h>

class QtXEmbedClient : public QWidget
{
    Q_OBJECT
public:
    QtXEmbedClient(QWidget *parent = 0, const char *name = 0);
    ~QtXEmbedClient();

    void embedInto(WId id);
    WId containerWinId() const;

    enum Errors {
	Unknown = 0,
	Internal = 1,
	InvalidWindowID = 2
    };
    
signals:
    void embedded();
    void containerClosed();
    void error(int);
    
protected:
    bool x11Event(XEvent *);
    bool eventFilter(QObject *, QEvent *);
    
    void setEmbedded();
private:
    int focusOriginator;

    WId container;
    QGuardedPtr<QWidget> currentFocus;
};

class QtXEmbedContainer : public QWidget
{
    Q_OBJECT
public:
    QtXEmbedContainer(QWidget *parent = 0, const char *name = 0);
    ~QtXEmbedContainer();
        
    void embed(WId id, bool xEmbedClient = true);
    void discardClient();

    WId clientWinId() const;

    QSize minimumSizeHint() const;
    
    enum Errors {
	Unknown = 0,
	Internal = 1,
	InvalidWindowID = 2
    };

signals:
    void clientIsEmbedded();
    void clientClosed();
    void error(int);
    
protected:
    bool x11Event(XEvent *);
    bool eventFilter(QObject *, QEvent *);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    
    bool isEmbedded() const;
    void moveInputToProxy();

    void acceptClient(WId window);
    void rejectClient(WId window);

    void checkGrab();
    
    WId topLevelParentWinId() const;

private:    
    WId client;
    QWidget *focusProxy;
    bool clientIsXEmbed;
    bool xgrab;
    QRect clientOriginalRect;
    QSize wmMinimumSizeHint;
};

#endif
