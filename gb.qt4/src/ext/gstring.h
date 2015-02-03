/***************************************************************************

  gstring.h

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
	GString &operator=(const QString &str);
	GString &operator=(const char *utf8);
	GString &operator+=(const GString &str);
  GString &operator+=(const char *utf8);
  GString &operator+=(const char c);
  GString left(uint len) const;
	GString right(uint len) const;
	GString mid(uint index, uint len = 0xffffffff) const;
  GString &remove(uint index, uint len);
  GString &append(const GString &str);
  GString &append(char c);
  GString &prepend(const GString &str);
  GString &prepend(char c);
  GString &insert(uint index, const GString &str);
  bool isNewLine(uint pos) const;
  bool isSpace(uint pos) const;
  bool isWordChar(uint pos) const;
	int indexOf(char c, int index = 0) const;
	int indexOf(const GString &str, int index = 0, bool cs = true) const;
	char at(uint pos) const;
	GString lower() const;
	GString upper() const;
	bool hasUnicode() const;
	bool hasTab() const { return indexOf('\t') >= 0; }
	void clear() { s.clear(); }
	int findNextLine(int index, int &len) const;
	
	static bool isStandardChar(ushort c);
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

inline GString &GString::operator=(const QString &str)
{
	s = str;
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

inline GString GString::lower() const
{
	QString sl = s;
	int i;
	
	for (i = 0; i < s.length(); i++)
		sl[i] = tolower(s[i].latin1());
		
	return GString(sl);
}

inline GString GString::upper() const
{
	QString sl = s;
	int i;
	
	for (i = 0; i < s.length(); i++)
		sl[i] = toupper(s[i].latin1());
		
	return GString(sl);
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

inline GString &GString::append(const GString &str)
{
	s.append(str.getString());
	return *this;
}

inline GString &GString::append(char c)
{
	s.append(c);
	return *this;
}

inline GString &GString::prepend(const GString &str)
{
	s.prepend(str.getString());
	return *this;
}

inline GString &GString::prepend(char c)
{
	s.prepend(c);
	return *this;
}

inline int GString::indexOf(char c, int index) const
{
	return s.indexOf(c, index);
}

inline int GString::indexOf(const GString &str, int index, bool cs) const
{
	return s.indexOf(str.getString(), index, cs ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

inline bool GString::isSpace(uint pos) const
{
	if (pos >= (uint)s.length())
		return false;
  return s[pos].isSpace();
}

inline bool GString::isNewLine(uint pos) const
{
	if (pos >= (uint)s.length())
		return false;
  return s[pos] == '\n';
}

inline bool GString::isWordChar(uint pos) const
{
	if (pos >= (uint)s.length())
		return false;
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

inline char GString::at(uint pos) const
{
	if (pos >= (uint)s.length())
		return 0;
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

inline bool GString::isStandardChar(ushort c)
{
	return !(c < 32 || (c >= 127 && c < 160) || c == 173 || c > 255);
}

#endif
