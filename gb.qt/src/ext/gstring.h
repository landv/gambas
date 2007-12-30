/***************************************************************************

  gstring.h

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

#ifndef __GSTRING_H
#define __GSTRING_H

#include "gb_common.h"
#include "gambas.h"
#include "../gb.qt.h"

#include <qstring.h>

extern "C" GB_INTERFACE GB;
extern "C" QT_INTERFACE QT;

class GString
{
private:
	QString s;

public:
	GString() { s = ""; }
	~GString() {}
  GString(const QString &str);
  GString(const GString &str);
	const char *utf8() const { return TO_UTF8(s); }
	QString getString() const { return s; }
	uint length() const { return s.length(); }
	GString &operator=(const GString &str);
	GString &operator=(const char *utf8);
	GString &operator+=(const GString &str);
  GString &operator+=(const char *utf8);
  GString &operator+=(const char c);
  GString left(uint len) const;
	GString right(uint len) const;
	GString mid(uint index, uint len = 0xffffffff) const;
  GString &remove(uint index, uint len);
  GString &prepend(const GString &str);
  GString &insert(uint index, const GString &str);
  bool isNewLine(uint pos);
  bool isSpace(uint pos);
  bool isWordChar(uint pos);
	int find(char c, int index = 0) const;
	char at(uint pos);
};

inline GString::GString(const GString &str)
{
	s = str.getString();
}

inline GString::GString(const QString &str)
{
	s = str;
}

inline GString &GString::operator=(const GString &str)
{
	s = str.getString();
	return *this;
}

inline GString &GString::operator=(const char *utf8)
{
	s = TO_QSTRING(utf8);
	return *this;
}

inline GString &GString::operator+=(const GString &str)
{
	s += str.getString();
	return *this;
}

inline GString &GString::operator+=(const char *utf8)
{
	s += TO_QSTRING(utf8);
	return *this;
}

inline GString &GString::operator+=(const char c)
{
	s += c;
	return *this;
}

inline GString GString::left(uint len) const
{
	return GString(s.left(len));
}

inline GString GString::right(uint len) const
{
	return GString(s.right(len));
}

inline GString GString::mid(uint index, uint len) const
{
	return GString(s.mid(index, len));
}

inline GString &GString::remove(uint index, uint len)
{
	s.remove(index, len);
	return *this;
}

inline GString &GString::insert(uint index, const GString &str)
{
	s.insert(index, str.getString());
	return *this;
}

inline GString &GString::prepend(const GString &str)
{
	s.prepend(str.getString());
	return *this;
}

inline int GString::find(char c, int index) const
{
	return s.find(c, index);
}

inline bool GString::isSpace(uint pos)
{
  return s[pos].isSpace();
}

inline bool GString::isNewLine(uint pos)
{
  return s[pos] == '\n';
}

inline bool GString::isWordChar(uint pos)
{
	QChar c = s[pos];
  return c.isLetterOrNumber() || c == '_' || c == '$';
}

inline bool operator==(const GString &s1, const GString &s2)
{
	return s1.getString() == s2.getString();
}

inline bool operator!=(const GString &s1, const GString &s2)
{
	return s1.getString() != s2.getString();
}

inline const GString operator+(const GString &s1, const GString &s2)
{
	GString tmp(s1);
	tmp += s2;
  return tmp;
}

inline char GString::at(uint pos)
{
	return s[pos].latin1();
}

inline const GString operator+(const GString &s1, const char c)
{
	GString tmp(s1);
	tmp += c;
  return tmp;
}

inline const GString operator+(const char c, const GString &str)
{
	GString tmp;
	tmp += c;
	tmp += str;
  return tmp;
}

#endif
