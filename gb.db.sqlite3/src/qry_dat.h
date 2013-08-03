/***************************************************************************

  qry_dat.h

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

/**********************************************************************
 * Copyright (c) 2002, Leo Seib, Hannover
 *
 * Project:Dataset C++ Dynamic Library
 * Module: FieldValue class and result sets classes header file
 * Author: Leo Seib      E-Mail: lev@almaty.pointstrike.net
 * Begin: 5/04/2002
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **********************************************************************/

using namespace std;

#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <map>
#include <vector>
#include <iostream>
#include "sqlite3.h"

enum fType
	{
		ft_String,
		ft_Boolean,
		ft_Char,
		ft_WChar,
		ft_WideString,
		ft_Short,
		ft_UShort,
		ft_Long,
		ft_ULong,
		ft_Float,
		ft_Double,
		ft_LongDouble,
		ft_Date,
		ft_Object,
		ft_Blob											// BM
	};

/* Size of Strings for fType */
#define ft_Boolean_Length  5		//TRUE or FALSE
#define ft_Short_Length  10
#define ft_LongDouble_Length  32
#define ft_Long_Length  12
#define ft_Float_Length  16
#define ft_Date_Length  19
#define ft_Double_Length  32

class field_value
	{
	private:
		fType field_type;
		string str_value;
		void *blob_value;
		bool is_null;
		int len;

	public:
		 field_value();
		~field_value();

		int get_len() const { return len; }

		fType get_fType() const
		{
			return field_type;
		}
		
		fType get_field_type() const
		{
			return field_type;
		}
		
		string gft();
		
		bool get_isNull() const
		{
			return is_null;
		}

		string get_asString() const;
		char *get_asBlob() const;
		bool get_asBool() const;
		int get_asInteger() const;
		#if 0
		char get_asChar() const;
		short get_asShort() const;
		unsigned short get_asUShort() const;
		long get_asLong() const;
		unsigned long get_asULong() const;
		double get_asDouble() const;
		#endif

		#if 0
		 field_value & operator=(const char *s)
		{
			set_asString(s);
			return *this;
		}
		field_value & operator=(const string & s)
		{
			set_asString(s);
			return *this;
		}
		#endif
		
		#if 0
		field_value & operator=(const bool b)
		{
			set_asBool(b);
			return *this;
		}
		field_value & operator=(const short s)
		{
			set_asShort(s);
			return *this;
		}
		field_value & operator=(const unsigned short us)
		{
			set_asUShort(us);
			return *this;
		}
		field_value & operator=(const long l)
		{
			set_asLong(l);
			return *this;
		}
		field_value & operator=(const unsigned long l)
		{
			set_asULong(l);
			return *this;
		}
		field_value & operator=(const int i)
		{
			set_asLong(i);
			return *this;
		}
		field_value & operator=(const double d)
		{
			set_asDouble(d);
			return *this;
		}
		#endif
		
		field_value & operator=(const field_value & fv);

		//class ostream;
		#if 0
		friend ostream & operator<<(ostream & os, const field_value & fv)
		{
			switch (fv.get_fType())
			{
				case ft_String:
				case ft_Blob:
				case ft_WideString:
				case ft_WChar:
				case ft_Object:
					{
						return os << fv.get_asString();
						break;
					}
				case ft_Date:
					{
						return os << fv.get_asString();
						break;
					}
				case ft_Boolean:
					{
						return os << fv.get_asBool();
						break;
					}
				case ft_Char:
					{
						return os << fv.get_asChar();
						break;
					}
				case ft_Short:
					{
						return os << fv.get_asShort();
						break;
					}
				case ft_UShort:
					{
						return os << fv.get_asUShort();
						break;
					}
				case ft_Long:
					{
						return os << fv.get_asLong();
						break;
					}
				case ft_ULong:
					{
						return os << fv.get_asULong();
						break;
					}
				case ft_Float:
				case ft_LongDouble:
				case ft_Double:
					{
						return os << fv.get_asDouble();
						break;
					}
			}
		}
		#endif

		void set_isNull(fType f);
		void set_asString(const char *s, fType type);
		void set_asString(const string & s, fType type);
		void set_asBlob(const char *s, int l);	//BM
		/*void set_asBool(const bool b);
		void set_asChar(const char c);
		void set_asShort(const short s);
		void set_asUShort(const unsigned short us);
		void set_asInteger(const int i);
		void set_asLong(const long l);
		void set_asULong(const unsigned long l);
		void set_asDouble(const double d);
		void set_asDate(const char *s);	//NG
		void set_asDate(const string & s);	//NG*/
	};

struct field_prop
	{
		string name, display_name;
		fType type;
		string field_table;					//?
		bool read_only;
		unsigned int field_len;
		unsigned int field_flags;
		unsigned int notnull;
		int idx;
	};

struct field
	{
		field_prop props;
		field_value val;
	};


typedef map < int, field > Fields;
typedef map < int, field_value > sql_record;
typedef map < int, field_prop > record_prop;
typedef map < int, sql_record > query_data;
typedef field_value variant;

//typedef Fields::iterator fld_itor;
typedef sql_record::iterator rec_itor;
typedef record_prop::iterator recprop_itor;
typedef query_data::iterator qry_itor;

struct result_set
	{
		sqlite3 *conn;							//NG
		record_prop record_header;
		query_data records;
	};
