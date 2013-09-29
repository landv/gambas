/***************************************************************************

  gbx_class_desc.h

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

#ifndef __GBX_CLASS_DESC_H
#define __GBX_CLASS_DESC_H

#include "gb_class_desc_common.h"

#define CD_PROPERTY               'p'
#define CD_PROPERTY_READ          'r'
#define CD_METHOD                 'm'
#define CD_CONSTANT               'C'
#define CD_EVENT                  ':'
#define CD_STATIC_PROPERTY        'P'
#define CD_STATIC_PROPERTY_READ   'R'
#define CD_STATIC_METHOD          'M'
#define CD_VARIABLE               'v'
#define CD_STRUCT_FIELD           'f'
#define CD_STATIC_VARIABLE        'V'
#define CD_EXTERN                 'X'

#define CD_STATIC_LIST    				"PRMVX"
#define CD_CALL_SOMETHING_LIST		"prmPRM"


typedef
	struct {
		char *name;
		TYPE type;                  // property datatype
		void (*read)();             // read property function
		void (*write)();            // write property function
		unsigned native : 1;        // if the property is native
		unsigned _reserved : 7;
		char _reserved2[3];
		#ifdef OS_64BITS
		int _reserved3;
		#endif
		struct _CLASS *class;
		}
	CLASS_DESC_PROPERTY;

typedef
	struct {
		char *name;
		TYPE type;                  // variable datatype
		int offset;   	            // variable offset in object memory
		CTYPE ctype;                // variable compilation datatype
		intptr_t _reserved;
		#ifdef OS_64BITS
		intptr_t _reserved2;
		#endif
		struct _CLASS *class;
		}
	CLASS_DESC_VARIABLE;

typedef
	struct {
		char *name;
		TYPE type;                  // return value datatype
		void (*exec)();             // method
		TYPE *signature;            // signature
		char npmin;                 // minimum number of arguments
		char npmax;                 // maximum number of arguments
		char npvar;                 // if there is a variable number of arguments
		unsigned native : 1;        // if the method is native
		unsigned subr : 1;          // static method called like a subr
		unsigned _reserved : 6;
		#ifdef OS_64BITS
		int _reserved2;
		#endif
		struct _CLASS *class;
		}
	CLASS_DESC_METHOD;

typedef
	struct {
		char *name;
		TYPE type;                  // return value datatype - N/A
		intptr_t index;             // event index
		TYPE *signature;            // event signature
		char npmin;                 // minimum number of arguments
		char npmax;                 // maximum number of arguments
		char npvar;                 // if there is a variable number of arguments
		char _reserved;
		#ifdef OS_64BITS
		int _reserved2;
		#endif
		struct _CLASS *class;
		}
	CLASS_DESC_EVENT;

typedef
	struct {
		char *name;
		TYPE type;                  // return value datatype
		int exec;                   // extern function index
		TYPE *signature;            // signature
		char npmin;                 // minimum number of arguments
		char npmax;                 // maximum number of arguments
		char npvar;                 // if there is a variable number of arguments
		char _reserved;
		#ifdef OS_64BITS
		int _reserved2;
		#endif
		struct _CLASS *class;
		}
	CLASS_DESC_EXTERN;

typedef
	struct {
		char *name;
		TYPE type;                  // constant datatype
		union {
			int _integer;
			double _float;
			float _single;
			char *_string;
			int64_t _long;
			void *_pointer;
			}
			value;
		unsigned translate : 1;
		unsigned _reserved : 31;
		#ifdef OS_64BITS
		int _reserved2;
		#endif
		struct _CLASS *class;
		}
	CLASS_DESC_CONSTANT;

typedef
	struct {
		char *name;
		void (*func)();
		}
	CLASS_DESC_HOOK;

typedef
	struct {
		char *name;
		intptr_t type;
		intptr_t val1;
		intptr_t val2;
		union {
			double _double;
			intptr_t _int[2];
			}	val3;
		}
	CLASS_DESC_GAMBAS;


typedef
	union {
		CLASS_DESC_PROPERTY property;
		CLASS_DESC_VARIABLE variable;
		CLASS_DESC_METHOD method;
		CLASS_DESC_CONSTANT constant;
		CLASS_DESC_EVENT event;
		CLASS_DESC_HOOK hook;
		CLASS_DESC_GAMBAS gambas;
		CLASS_DESC_EXTERN ext;
		}
	CLASS_DESC;

typedef
	struct {
		char *name;
		int len;
		CLASS_DESC *desc;
		}
	PACKED
	CLASS_DESC_SYMBOL;


#define CLASS_DESC_get_type(d) (*(d)->gambas.name)
#define CLASS_DESC_is_static_method(d) (CLASS_DESC_get_type((CLASS_DESC *)d) == 'M')
#define CLASS_DESC_SELF (-1)

char *CLASS_DESC_get_signature(CLASS_DESC *cd);
const char *CLASS_DESC_get_type_name(const CLASS_DESC *desc);


#endif
