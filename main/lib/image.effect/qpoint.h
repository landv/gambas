/***************************************************************************

  qpoint.h

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
/****************************************************************************
** $Id: qt/qpoint.h   3.3.4   edited May 27 2003 $
**
** Definition of QPoint class
**
** Created : 931028
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QPOINT_H
#define QPOINT_H

#include "qt.h"

class Q_EXPORT QPoint
{
public:
    QPoint();
    QPoint( int xpos, int ypos );

    bool   isNull()	const;

    int	   x()		const;
    int	   y()		const;
    void   setX( int x );
    void   setY( int y );

    int manhattanLength() const;

    QCOORD &rx();
    QCOORD &ry();

    QPoint &operator+=( const QPoint &p );
    QPoint &operator-=( const QPoint &p );
    QPoint &operator*=( int c );
    QPoint &operator*=( double c );
    QPoint &operator/=( int c );
    QPoint &operator/=( double c );

    friend inline bool	 operator==( const QPoint &, const QPoint & );
    friend inline bool	 operator!=( const QPoint &, const QPoint & );
    friend inline const QPoint operator+( const QPoint &, const QPoint & );
    friend inline const QPoint operator-( const QPoint &, const QPoint & );
    friend inline const QPoint operator*( const QPoint &, int );
    friend inline const QPoint operator*( int, const QPoint & );
    friend inline const QPoint operator*( const QPoint &, double );
    friend inline const QPoint operator*( double, const QPoint & );
    friend inline const QPoint operator-( const QPoint & );
    friend inline const QPoint operator/( const QPoint &, int );
    friend inline const QPoint operator/( const QPoint &, double );

private:
    static void warningDivByZero();

#if defined(Q_OS_MAC)
    QCOORD yp;
    QCOORD xp;
#else
    QCOORD xp;
    QCOORD yp;
#endif
};


/*****************************************************************************
  QPoint stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator<<( QDataStream &, const QPoint & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QPoint & );
#endif

/*****************************************************************************
  QPoint inline functions
 *****************************************************************************/

inline QPoint::QPoint()
{ xp=0; yp=0; }

inline QPoint::QPoint( int xpos, int ypos )
{ xp=(QCOORD)xpos; yp=(QCOORD)ypos; }

inline bool QPoint::isNull() const
{ return xp == 0 && yp == 0; }

inline int QPoint::x() const
{ return xp; }

inline int QPoint::y() const
{ return yp; }

inline void QPoint::setX( int x )
{ xp = (QCOORD)x; }

inline void QPoint::setY( int y )
{ yp = (QCOORD)y; }

inline QCOORD &QPoint::rx()
{ return xp; }

inline QCOORD &QPoint::ry()
{ return yp; }

inline QPoint &QPoint::operator+=( const QPoint &p )
{ xp+=p.xp; yp+=p.yp; return *this; }

inline QPoint &QPoint::operator-=( const QPoint &p )
{ xp-=p.xp; yp-=p.yp; return *this; }

inline QPoint &QPoint::operator*=( int c )
{ xp*=(QCOORD)c; yp*=(QCOORD)c; return *this; }

inline QPoint &QPoint::operator*=( double c )
{ xp=(QCOORD)(xp*c); yp=(QCOORD)(yp*c); return *this; }

inline bool operator==( const QPoint &p1, const QPoint &p2 )
{ return p1.xp == p2.xp && p1.yp == p2.yp; }

inline bool operator!=( const QPoint &p1, const QPoint &p2 )
{ return p1.xp != p2.xp || p1.yp != p2.yp; }

inline const QPoint operator+( const QPoint &p1, const QPoint &p2 )
{ return QPoint(p1.xp+p2.xp, p1.yp+p2.yp); }

inline const QPoint operator-( const QPoint &p1, const QPoint &p2 )
{ return QPoint(p1.xp-p2.xp, p1.yp-p2.yp); }

inline const QPoint operator*( const QPoint &p, int c )
{ return QPoint(p.xp*c, p.yp*c); }

inline const QPoint operator*( int c, const QPoint &p )
{ return QPoint(p.xp*c, p.yp*c); }

inline const QPoint operator*( const QPoint &p, double c )
{ return QPoint((QCOORD)(p.xp*c), (QCOORD)(p.yp*c)); }

inline const QPoint operator*( double c, const QPoint &p )
{ return QPoint((QCOORD)(p.xp*c), (QCOORD)(p.yp*c)); }

inline const QPoint operator-( const QPoint &p )
{ return QPoint(-p.xp, -p.yp); }

inline QPoint &QPoint::operator/=( int c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0 )
	warningDivByZero();
#endif
    xp/=(QCOORD)c;
    yp/=(QCOORD)c;
    return *this;
}

inline QPoint &QPoint::operator/=( double c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0.0 )
	warningDivByZero();
#endif
    xp=(QCOORD)(xp/c);
    yp=(QCOORD)(yp/c);
    return *this;
}

inline const QPoint operator/( const QPoint &p, int c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0 )
	QPoint::warningDivByZero();
#endif
    return QPoint(p.xp/c, p.yp/c);
}

inline const QPoint operator/( const QPoint &p, double c )
{
#if defined(QT_CHECK_MATH)
    if ( c == 0.0 )
	QPoint::warningDivByZero();
#endif
    return QPoint((QCOORD)(p.xp/c), (QCOORD)(p.yp/c));
}

#define Q_DEFINED_QPOINT
#endif // QPOINT_H
