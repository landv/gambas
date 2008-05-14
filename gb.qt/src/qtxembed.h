/***************************************************************************
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
*/
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
