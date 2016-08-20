/***************************************************************************

  cpoint_temp.h

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

#define IMPLEMENT_POINT_CLASS(__struct, __name, __gtype, __ctype, __sign, __return, __this, __rstruct, __rname)               \
                                                                                                                              \
__struct * __struct##_create(__ctype x, __ctype y)                                                                            \
{                                                                                                                             \
  __struct *p = GB.New(GB.FindClass(#__name), NULL, NULL);                                                                    \
  p->x = x;                                                                                                                   \
  p->y = y;                                                                                                                   \
  return p;                                                                                                                   \
}                                                                                                                             \
                                                                                                                              \
static inline __struct *__struct##_make(__struct *a, const __ctype x, const __ctype y)                                        \
{                                                                                                                             \
  if (a->ob.ref <= 1)                                                                                                         \
  {                                                                                                                           \
    a->x = x;                                                                                                                 \
    a->y = y;                                                                                                                 \
    return a;                                                                                                                 \
  }                                                                                                                           \
  else                                                                                                                        \
    return __struct##_create(x, y);                                                                                           \
}                                                                                                                             \
                                                                                                                              \
static __struct *_add_##__name(__struct *a, __struct *b, bool invert)                                                         \
{                                                                                                                             \
  return __struct##_make(a, a->x + b->x, a->y + b->y);                                                                        \
}                                                                                                                             \
                                                                                                                              \
static __struct *_sub_##__name(__struct *a, __struct *b, bool invert)                                                         \
{                                                                                                                             \
  return __struct##_make(a, a->x - b->x, a->y - b->y);                                                                        \
}                                                                                                                             \
                                                                                                                              \
static __struct *_mulf_##__name(__struct *a, double f, bool invert)                                                           \
{                                                                                                                             \
  return __struct##_make(a, a->x * f, a->y * f);                                                                              \
}                                                                                                                             \
                                                                                                                              \
static __struct *_mulo_##__name(__struct *a, void *b, bool invert)                                                            \
{                                                                                                                             \
  return NULL;                                                                                                                \
}                                                                                                                             \
                                                                                                                              \
static __struct *_divf_##__name(__struct *a, double f, bool invert)                                                           \
{                                                                                                                             \
  if (invert)                                                                                                                 \
    return NULL;                                                                                                              \
  if (f == 0.0)                                                                                                               \
    return NULL;                                                                                                              \
                                                                                                                              \
  return __struct##_make(a, a->x / f, a->y / f);                                                                              \
}                                                                                                                             \
                                                                                                                              \
static __struct *_divo_##__name(__struct *a, void *b, bool invert)                                                            \
{                                                                                                                             \
  return NULL;                                                                                                                \
}                                                                                                                             \
                                                                                                                              \
static int _equal_##__name(__struct *a, __struct *b, bool invert)                                                             \
{                                                                                                                             \
  return a->x == b->x && a->y == b->y;                                                                                        \
}                                                                                                                             \
                                                                                                                              \
static double _fabs_##__name(__struct *a)                                                                                     \
{                                                                                                                             \
  return hypot(a->x, a->y);                                                                                                   \
}                                                                                                                             \
                                                                                                                              \
static __struct *_neg_##__name(__struct *a)                                                                                   \
{                                                                                                                             \
  return __struct##_make(a, -a->x, -a->y);                                                                                    \
}                                                                                                                             \
                                                                                                                              \
static GB_OPERATOR_DESC _operator_##__name =                                                                                  \
{                                                                                                                             \
  .equal   = (void *)_equal_##__name,                                                                                         \
  .add     = (void *)_add_##__name,                                                                                           \
  .sub     = (void *)_sub_##__name,                                                                                           \
  .mulf    = (void *)_mulf_##__name,                                                                                          \
  .mulo    = (void *)_mulo_##__name,                                                                                          \
  .divf    = (void *)_divf_##__name,                                                                                          \
  .divo    = (void *)_divo_##__name,                                                                                          \
  .fabs     = (void *)_fabs_##__name,                                                                                         \
  .neg     = (void *)_neg_##__name,                                                                                           \
};                                                                                                                            \
                                                                                                                              \
char *__struct##_to_string(void *a, bool local)                                                                               \
{                                                                                                                             \
  char *result = NULL;                                                                                                        \
  char *str;                                                                                                                  \
  int len;                                                                                                                    \
                                                                                                                              \
  __ctype x = ((__struct *)a)->x;                                                                                             \
  __ctype y = ((__struct *)a)->y;                                                                                             \
                                                                                                                              \
  result = GB.AddChar(result, '[');                                                                                           \
                                                                                                                              \
  GB.NumberToString(local, x, NULL, &str, &len);                                                                              \
  result = GB.AddString(result, str, len);                                                                                    \
                                                                                                                              \
  result = GB.AddChar(result, local ? ' ' : ',');                                                                             \
                                                                                                                              \
  GB.NumberToString(local, y, NULL, &str, &len);                                                                              \
  result = GB.AddString(result, str, len);                                                                                    \
                                                                                                                              \
  result = GB.AddChar(result, ']');                                                                                           \
                                                                                                                              \
  return result;                                                                                                              \
}                                                                                                                             \
                                                                                                                              \
static bool _convert_##__name(void *a, GB_TYPE type, GB_VALUE *conv)                                                          \
{                                                                                                                             \
  if (a)                                                                                                                      \
  {                                                                                                                           \
    double norm = _fabs_##__name(a);                                                                                           \
                                                                                                                              \
    switch (type)                                                                                                             \
    {                                                                                                                         \
      case GB_T_FLOAT:                                                                                                        \
        conv->_float.value = norm;                                                                                            \
        return FALSE;                                                                                                         \
                                                                                                                              \
      case GB_T_SINGLE:                                                                                                       \
        conv->_single.value = norm;                                                                                           \
        return FALSE;                                                                                                         \
                                                                                                                              \
      case GB_T_INTEGER:                                                                                                      \
      case GB_T_SHORT:                                                                                                        \
      case GB_T_BYTE:                                                                                                         \
        conv->_integer.value = norm;                                                                                          \
        return FALSE;                                                                                                         \
                                                                                                                              \
      case GB_T_LONG:                                                                                                         \
        conv->_long.value = norm;                                                                                             \
        return FALSE;                                                                                                         \
                                                                                                                              \
      case GB_T_STRING:                                                                                                       \
      case GB_T_CSTRING:                                                                                                      \
        conv->_string.value.addr = __struct##_to_string(a, type == GB_T_CSTRING);                                             \
        conv->_string.value.start = 0;                                                                                        \
        conv->_string.value.len = GB.StringLength(conv->_string.value.addr);                                                  \
        return FALSE;                                                                                                         \
                                                                                                                              \
      default:                                                                                                                \
        if (type == GB.FindClass("Point"))                                                                                    \
        {                                                                                                                     \
          conv->_object.value = CPOINT_create(((__struct *)a)->x, ((__struct *)a)->y);                                        \
          return FALSE;                                                                                                       \
        }                                                                                                                     \
        if (type == GB.FindClass("PointF"))                                                                                   \
        {                                                                                                                     \
          conv->_object.value = CPOINTF_create(((__struct *)a)->x, ((__struct *)a)->y);                                       \
          return FALSE;                                                                                                       \
        }                                                                                                                     \
        else                                                                                                                  \
          return TRUE;                                                                                                        \
    }                                                                                                                         \
  }                                                                                                                           \
  else                                                                                                                        \
    return TRUE;                                                                                                              \
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
  GB.ReturnObject(__struct##_create(VARGOPT(x, 0), VARGOPT(y, 0)));                                                           \
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
  GB.ReturnObject(__struct##_create(__this->x, __this->y));                                                                   \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
BEGIN_METHOD(__name##_InRect, GB_OBJECT rect)                                                                                 \
                                                                                                                              \
  __rstruct *rect = VARG(rect);                                                                                               \
                                                                                                                              \
  if (GB.CheckObject(rect))                                                                                                   \
    return;                                                                                                                   \
                                                                                                                              \
  GB.ReturnBoolean(                                                                                                           \
    (__this->x >= rect->x) && (__this->x < (rect->x + rect->w))                                                               \
    && (__this->y >= rect->y) && (__this->y < (rect->y + rect->h))                                                            \
  );                                                                                                                          \
                                                                                                                              \
END_METHOD                                                                                                                    \
                                                                                                                              \
GB_DESC __name##Desc[] =                                                                                                      \
{                                                                                                                             \
  GB_DECLARE(#__name, sizeof(__struct)),                                                                                      \
                                                                                                                              \
  GB_METHOD("_new", NULL, __name##_new, "[(X)" __sign "(Y)" __sign),                                                          \
  GB_STATIC_METHOD("_call", #__name, __name##_call, "[(X)" __sign "(Y)" __sign),                                              \
                                                                                                                              \
  GB_PROPERTY("X", __sign, __name##_X),                                                                                       \
  GB_PROPERTY("Y", __sign, __name##_Y),                                                                                       \
                                                                                                                              \
  GB_METHOD("Copy", #__name, __name##_Copy, NULL),                                                                            \
  GB_METHOD("InRect", "b", __name##_InRect, "(Rectangle)" #__rname ";"),                                                      \
                                                                                                                              \
  GB_INTERFACE("_operator", &_operator_##__name),                                                                             \
  GB_INTERFACE("_convert", &_convert_##__name),                                                                               \
                                                                                                                              \
  GB_END_DECLARE                                                                                                              \
};                                                                                                                            

