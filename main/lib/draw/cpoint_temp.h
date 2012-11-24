/***************************************************************************

  cpoint_temp.h

  (c) 2000-2012 Benoît Minisini <gambas@users.sourceforge.net>

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

#define IMPLEMENT_POINT_CLASS(__struct, __name, __gtype, __ctype, __sign, __return, __this)                                   \
                                                                                                                              \
__struct * __struct##_create(void)                                                                                            \
{                                                                                                                             \
  return GB.New(GB.FindClass(#__name), NULL, NULL);                                                                           \
}                                                                                                                             \
                                                                                                                              \
                                                                                                                              \
BEGIN_METHOD(__name##_new, __gtype x; __gtype y; __gtype w; __gtype h)                                                        \
                                                                                                                              \
  if (!MISSING(x) && !MISSING(y))                                                                                             \
  {                                                                                                                           \
    __this->x = VARG(x);                                                                                                      \
    __this->y = VARG(y);                                                                                                      \
  }                                                                                                                           \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_call, __gtype x; __gtype y)                                                                             \
                                                                                                                              \
  __struct *rect = __struct##_create();                                                                                       \
                                                                                                                              \
  if (!MISSING(x) && !MISSING(y))                                                                                             \
  {                                                                                                                           \
    rect->x = VARG(x);                                                                                                        \
    rect->y = VARG(y);                                                                                                        \
  }                                                                                                                           \
                                                                                                                              \
  GB.ReturnObject(rect);                                                                                                      \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_PROPERTY(__name##_X)                                                                                                    \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->x);                                                                                                      \
  else                                                                                                                        \
    __this->x = VPROP(__gtype);                                                                                               \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_PROPERTY(__name##_Y)                                                                                                    \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->y);                                                                                                      \
  else                                                                                                                        \
    __this->y = VPROP(__gtype);                                                                                               \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_METHOD_VOID(__name##_Copy)                                                                                              \
                                                                                                                              \
  __struct *copy = __struct##_create();                                                                                       \
                                                                                                                              \
  copy->x = __this->x;                                                                                                        \
  copy->y = __this->y;                                                                                                        \
                                                                                                                              \
  GB.ReturnObject(copy);                                                                                                      \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
GB_DESC __name##Desc[] =                                                                                                      \
{                                                                                                                             \
  GB_DECLARE(#__name, sizeof(__struct)),                                                                                      \
                                                                                                                              \
  GB_METHOD("_new", NULL, __name##_new, "[(X)" __sign "(Y)" __sign),                                                          \
  GB_METHOD("_call", #__name, __name##_call, "[(X)" __sign "(Y)" __sign),                                                     \
                                                                                                                              \
  GB_PROPERTY("X", __sign, __name##_X),                                                                                       \
  GB_PROPERTY("Y", __sign, __name##_Y),                                                                                       \
                                                                                                                              \
  GB_METHOD("Copy", #__name, __name##_Copy, NULL),                                                                            \
                                                                                                                              \
  GB_END_DECLARE                                                                                                              \
};                                                                                                                            

