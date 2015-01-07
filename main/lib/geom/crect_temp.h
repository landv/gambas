/***************************************************************************

  crect_temp.h

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

#define IMPLEMENT_RECT_CLASS(__struct, __name, __gtype, __ctype, __sign, __return, __this, __pstruct, __pname)                \
                                                                                                                              \
static void __struct##_normalize(__struct *_object)                                                                           \
{                                                                                                                             \
  if (__this->w < 0)                                                                                                          \
  {                                                                                                                           \
    __this->w = (- __this->w);                                                                                                \
    __this->x -= __this->w;                                                                                                   \
  }                                                                                                                           \
                                                                                                                              \
  if (__this->h < 0)                                                                                                          \
  {                                                                                                                           \
    __this->h = (- __this->h);                                                                                                \
    __this->y -= __this->h;                                                                                                   \
  }                                                                                                                           \
}                                                                                                                             \
                                                                                                                              \
__struct * __struct##_create(void)                                                                                            \
{                                                                                                                             \
  return GB.New(GB.FindClass(#__name), NULL, NULL);                                                                           \
}                                                                                                                             \
                                                                                                                              \
                                                                                                                              \
BEGIN_METHOD(__name##_new, __gtype x; __gtype y; __gtype w; __gtype h)                                                        \
                                                                                                                              \
  if (!MISSING(x) && !MISSING(y) && !MISSING(w) && !MISSING(h))                                                               \
  {                                                                                                                           \
    __this->x = VARG(x);                                                                                                      \
    __this->y = VARG(y);                                                                                                      \
    __this->w = VARG(w);                                                                                                      \
    __this->h = VARG(h);                                                                                                      \
    __struct##_normalize(__this);                                                                                             \
  }                                                                                                                           \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_call, __gtype x; __gtype y; __gtype w; __gtype h)                                                       \
                                                                                                                              \
  __struct *rect = __struct##_create();                                                                                       \
                                                                                                                              \
  rect->x = VARG(x);                                                                                                          \
  rect->y = VARG(y);                                                                                                          \
  rect->w = VARG(w);                                                                                                          \
  rect->h = VARG(h);                                                                                                          \
  __struct##_normalize(rect);                                                                                                 \
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
BEGIN_PROPERTY(__name##_Width)                                                                                                \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->w);                                                                                                      \
  else                                                                                                                        \
  {                                                                                                                           \
    __this->w = VPROP(__gtype);                                                                                               \
    __struct##_normalize(__this);                                                                                             \
  }                                                                                                                           \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_PROPERTY(__name##_Height)                                                                                               \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->h);                                                                                                      \
  else                                                                                                                        \
  {                                                                                                                           \
    __this->h = VPROP(__gtype);                                                                                               \
    __struct##_normalize(__this);                                                                                             \
  }                                                                                                                           \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_PROPERTY(__name##_Left)                                                                                                 \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->x);                                                                                                      \
  else                                                                                                                        \
  {                                                                                                                           \
    __ctype dx = VPROP(__gtype) - __this->x;                                                                                      \
    if (dx > __this->w)                                                                                                       \
      dx = __this->w;                                                                                                         \
                                                                                                                              \
    __this->x += dx;                                                                                                          \
    __this->w -= dx;                                                                                                          \
  }                                                                                                                           \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_PROPERTY(__name##_Top)                                                                                                  \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->y);                                                                                                      \
  else                                                                                                                        \
  {                                                                                                                           \
    __ctype dy = VPROP(__gtype) - __this->y;                                                                                      \
    if (dy > __this->h)                                                                                                       \
      dy = __this->h;                                                                                                         \
                                                                                                                              \
    __this->y += dy;                                                                                                          \
    __this->h -= dy;                                                                                                          \
  }                                                                                                                           \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_PROPERTY(__name##_Right)                                                                                                \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->x + __this->w);                                                                                          \
  else                                                                                                                        \
  {                                                                                                                           \
    __ctype x2 = VPROP(__gtype);                                                                                                  \
    if (x2 < __this->x)                                                                                                       \
      x2 = __this->x;                                                                                                         \
                                                                                                                              \
    __this->w = x2 - __this->x;                                                                                               \
  }                                                                                                                           \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_PROPERTY(__name##_Bottom)                                                                                               \
                                                                                                                              \
  if (READ_PROPERTY)                                                                                                          \
    __return(__this->y + __this->h);                                                                                          \
  else                                                                                                                        \
  {                                                                                                                           \
    __ctype y2 = VPROP(__gtype);                                                                                                  \
    if (y2 < __this->y)                                                                                                       \
      y2 = __this->y;                                                                                                         \
                                                                                                                              \
    __this->h = y2 - __this->y;                                                                                               \
  }                                                                                                                           \
                                                                                                                              \
END_PROPERTY                                                                                                                  \
                                                                                                                              \
BEGIN_METHOD_VOID(__name##_Clear)                                                                                             \
                                                                                                                              \
  __this->x = __this->y = __this->w = __this->h = 0;                                                                          \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD_VOID(__name##_IsVoid)                                                                                            \
                                                                                                                              \
  GB.ReturnBoolean(__this->w <= 0 || __this->h <= 0);                                                                         \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD_VOID(__name##_Copy)                                                                                              \
                                                                                                                              \
  __struct *copy = __struct##_create();                                                                                       \
                                                                                                                              \
  copy->x = __this->x;                                                                                                        \
  copy->y = __this->y;                                                                                                        \
  copy->w = __this->w;                                                                                                        \
  copy->h = __this->h;                                                                                                        \
                                                                                                                              \
  GB.ReturnObject(copy);                                                                                                      \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Move, __gtype x; __gtype y; __gtype w; __gtype h)                                                       \
                                                                                                                              \
  __this->x = VARG(x);                                                                                                        \
  __this->y = VARG(y);                                                                                                        \
  if (!MISSING(w) && !MISSING(h))                                                                                             \
  {                                                                                                                           \
    __this->w = VARG(w);                                                                                                      \
    __this->h = VARG(h);                                                                                                      \
    __struct##_normalize(__this);                                                                                             \
  }                                                                                                                           \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Resize, __gtype w; __gtype h)                                                                           \
                                                                                                                              \
  __this->w = VARG(w);                                                                                                        \
  __this->h = VARG(h);                                                                                                        \
  __struct##_normalize(__this);                                                                                               \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Translate, __gtype dx; __gtype dy)                                                                      \
                                                                                                                              \
  __this->x += VARG(dx);                                                                                                      \
  __this->y += VARG(dy);                                                                                                      \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Union, GB_OBJECT rect)                                                                                  \
                                                                                                                              \
  __struct *dest;                                                                                                             \
  __ctype x, y, w, h;                                                                                                         \
  __struct *rect = (__struct *)VARG(rect);                                                                                    \
                                                                                                                              \
  if (GB.CheckObject(rect))                                                                                                   \
    return;                                                                                                                   \
                                                                                                                              \
  dest = __struct##_create();                                                                                                 \
                                                                                                                              \
  x = Min(__this->x, rect->x);                                                                                                \
  y = Min(__this->y, rect->y);                                                                                                \
  w = Max(__this->x + __this->w, rect->x + rect->w) - x;                                                                      \
  h = Max(__this->y + __this->h, rect->y + rect->h) - y;                                                                      \
                                                                                                                              \
  dest->x = x;                                                                                                                \
  dest->y = y;                                                                                                                \
  dest->w = w;                                                                                                                \
  dest->h = h;                                                                                                                \
                                                                                                                              \
  GB.ReturnObject(dest);                                                                                                      \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Intersection, GB_OBJECT rect)                                                                           \
                                                                                                                              \
  __struct *dest;                                                                                                             \
  __ctype x, y, x2, y2;                                                                                                       \
  __struct *rect = (__struct *)VARG(rect);                                                                                    \
                                                                                                                              \
  if (GB.CheckObject(rect))                                                                                                   \
    return;                                                                                                                   \
                                                                                                                              \
  x = Max(__this->x, rect->x);                                                                                                \
  y = Max(__this->y, rect->y);                                                                                                \
  x2 = Min(__this->x + __this->w, rect->x + rect->w);                                                                         \
  y2 = Min(__this->y + __this->h, rect->y + rect->h);                                                                         \
                                                                                                                              \
  if (x2 > x && y2 > y)                                                                                                       \
  {                                                                                                                           \
    dest = __struct##_create();                                                                                               \
                                                                                                                              \
    dest->x = x;                                                                                                              \
    dest->y = y;                                                                                                              \
    dest->w = x2 - x;                                                                                                         \
    dest->h = y2 - y;                                                                                                         \
                                                                                                                              \
    GB.ReturnObject(dest);                                                                                                    \
  }                                                                                                                           \
  else                                                                                                                        \
  {                                                                                                                           \
    GB.ReturnNull();                                                                                                          \
  }                                                                                                                           \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Contains, __gtype x; __gtype y)                                                                         \
                                                                                                                              \
  __ctype x = VARG(x);                                                                                                            \
  __ctype y = VARG(y);                                                                                                            \
                                                                                                                              \
  GB.ReturnBoolean((x >= __this->x) && (x < (__this->x + __this->w)) && (y >= __this->y) && (y < (__this->y + __this->h)));   \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_Adjust, __gtype left; __gtype top; __gtype right; __gtype bottom)                                       \
                                                                                                                              \
  __ctype left = VARG(left);                                                                                                      \
  __ctype top = VARGOPT(top, left);                                                                                               \
  __ctype right = VARGOPT(right, left);                                                                                           \
  __ctype bottom = VARGOPT(bottom, top);                                                                                          \
                                                                                                                              \
  __this->x += left;                                                                                                          \
  __this->w -= (left + right);                                                                                                \
  __this->y += top;                                                                                                           \
  __this->h -= (top + bottom);                                                                                                \
                                                                                                                              \
  if (__this->w < 1 || __this->h < 1)                                                                                         \
    __this->w = __this->h = 0;                                                                                                \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD_VOID(__name##_Center)                                                                                            \
                                                                                                                              \
  __pstruct *point = GB.New(GB.FindClass(#__pname), NULL, NULL);                                                              \
	point->x = __this->x + __this->w / 2;                                                                                       \
	point->y = __this->y + __this->h / 2;                                                                                       \
	GB.ReturnObject(point);                                                                                                     \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
GB_DESC __name##Desc[] =                                                                                                      \
{                                                                                                                             \
  GB_DECLARE(#__name, sizeof(__struct)),                                                                                      \
                                                                                                                              \
  GB_METHOD("_new", NULL, __name##_new, "[(X)" __sign "(Y)" __sign "(Width)" __sign "(Height)" __sign "]"),                   \
  GB_STATIC_METHOD("_call", #__name, __name##_call, "(X)" __sign "(Y)" __sign "(Width)" __sign "(Height)" __sign),            \
                                                                                                                              \
  GB_PROPERTY("X", __sign, __name##_X),                                                                                       \
  GB_PROPERTY("Y", __sign, __name##_Y),                                                                                       \
  GB_PROPERTY("W", __sign, __name##_Width),                                                                                   \
  GB_PROPERTY("H", __sign, __name##_Height),                                                                                  \
  GB_PROPERTY("Width", __sign, __name##_Width),                                                                               \
  GB_PROPERTY("Height", __sign, __name##_Height),                                                                             \
  GB_PROPERTY("Left", __sign, __name##_Left),                                                                                 \
  GB_PROPERTY("Top", __sign, __name##_Top),                                                                                   \
  GB_PROPERTY("Right", __sign, __name##_Right),                                                                               \
  GB_PROPERTY("Bottom", __sign, __name##_Bottom),                                                                             \
                                                                                                                              \
  GB_METHOD("Clear", NULL, __name##_Clear, NULL),                                                                             \
  GB_METHOD("IsVoid", "b", __name##_IsVoid, NULL),                                                                            \
  GB_METHOD("Copy", #__name, __name##_Copy, NULL),                                                                            \
  GB_METHOD("Move", NULL, __name##_Move, "(X)" __sign "(Y)" __sign "[(Width)" __sign "(Height)" __sign "]"),                  \
  GB_METHOD("Resize", NULL, __name##_Resize, "(Width)" __sign "(Height)" __sign ""),                                          \
  GB_METHOD("Translate", NULL, __name##_Translate, "(DX)" __sign "(DY)" __sign ""),                                           \
  GB_METHOD("Union", #__name, __name##_Union, "(Rect)" #__name ";"),                                                          \
  GB_METHOD("Intersection", #__name, __name##_Intersection, "(Rect)" #__name ";"),                                            \
  GB_METHOD("Contains", "b", __name##_Contains, "(X)" __sign "(Y)" __sign ""),                                                \
  GB_METHOD("Adjust", NULL, __name##_Adjust, "(Left)" __sign "[(Top)" __sign "(Right)" __sign "(Bottom)" __sign "]"),         \
  GB_METHOD("Center", #__pname, __name##_Center, NULL),                                                                       \
                                                                                                                              \
  GB_END_DECLARE                                                                                                              \
};                                                                                                                            

