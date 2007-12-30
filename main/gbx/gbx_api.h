/***************************************************************************

  gbx_api.h

  Gambas API for external libraries

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

#ifndef __GBX_API_H
#define __GBX_API_H

#include "gambas.h"

PUBLIC int GB_GetInterface(const char *library, int version, void *iface);
PUBLIC void *GB_Hook(int type, void *hook);

PUBLIC int GB_LoadComponent(const char *name);
PUBLIC const char *GB_CurrentComponent(void);

PUBLIC void GB_Push(int nval, ...);
PUBLIC int GB_CanRaise(void *object, int event_id);
PUBLIC int GB_Raise(void *object, int event_id, int nparam, ...);
PUBLIC int GB_GetEvent(void *class, char *name);
PUBLIC char *GB_GetLastEventName();
PUBLIC int GB_Stopped(void);

PUBLIC int GB_NParam(void);
PUBLIC const char *GB_GetUnknown(void);
PUBLIC int GB_IsProperty(void);

PUBLIC void GB_Error(const char *msg, ...);

PUBLIC void GB_Ref(void *object);
PUBLIC void GB_Unref(void **object);
PUBLIC void GB_UnrefKeep(void **object, int);

PUBLIC void GB_StopEnum(void);
PUBLIC void *GB_GetEnum(void);
PUBLIC void GB_ListEnum(void *);
PUBLIC int GB_NextEnum(void);
PUBLIC void GB_StopAllEnum(void *);

PUBLIC void GB_Return(unsigned int type, ...);
PUBLIC void GB_ReturnInteger(int val);
#define GB_ReturnInt GB_ReturnInteger
PUBLIC void GB_ReturnLong(long long val);
PUBLIC void GB_ReturnBoolean(int val);
PUBLIC void GB_ReturnObject(void *val);
PUBLIC void GB_ReturnNull(void);
PUBLIC void GB_ReturnFloat(double val);
PUBLIC void GB_ReturnPtr(unsigned int type, void *value);
PUBLIC void GB_ReturnDate(GB_DATE *date);

PUBLIC void GB_ReturnString(char *str);
PUBLIC void GB_ReturnConstString(const char *str, int len);
PUBLIC void GB_ReturnConstZeroString(const char *str);
PUBLIC void GB_ReturnNewString(const char *src, int len);
PUBLIC void GB_ReturnNewZeroString(const char *src);

PUBLIC void *GB_GetClass(void *object);
PUBLIC char *GB_GetClassName(void *object);
PUBLIC void *GB_FindClass(const char *name);
PUBLIC int GB_ExistClass(const char *name);
PUBLIC int GB_ExistClassLocal(const char *name);

PUBLIC char *GB_ToZeroString(GB_STRING *src);

PUBLIC int GB_LoadFile(const char *path, int lenp, char **addr, int *len);
PUBLIC void GB_ReleaseFile(char **addr, int len);
PUBLIC char *GB_GetTempDir(void);
PUBLIC char *GB_RealFileName(const char *path, int len);

PUBLIC int GB_IsMissing(int param);

PUBLIC void GB_Attach(void *object, void *parent, const char *name);
PUBLIC void GB_Detach(void *object);

PUBLIC void GB_Store(GB_TYPE type, GB_VALUE *src, void *dst);
PUBLIC void GB_StoreString(GB_STRING *src, char **dst);
PUBLIC void GB_StoreObject(GB_OBJECT *object, void **dst);
PUBLIC void GB_StoreVariant(GB_VARIANT *src, void *dst);

PUBLIC void GB_Watch(int fd, int flag, void *callback, int param);

PUBLIC int GB_New(void **object, void *class_name, const char *name, void *parent);
PUBLIC int GB_CheckObject(void *object);
PUBLIC int GB_Is(void *object, void *class);

PUBLIC int GB_GetFunction(GB_FUNCTION *func, void *object, const char *name, const char *sign, const char *type);
PUBLIC GB_VALUE *GB_Call(GB_FUNCTION *func, int nparam, int release);
PUBLIC void *GB_GetClassInterface(void *class, const char *name);

PUBLIC const char *GB_AppName(void);
PUBLIC const char *GB_AppTitle(void);
PUBLIC const char *GB_AppVersion(void);
PUBLIC const char *GB_AppPath(void);
PUBLIC const char *GB_AppStartup(void);

PUBLIC char *GB_SystemCharset(void);

PUBLIC void *GB_Eval(void *, void *);

PUBLIC void GB_ArrayNew(GB_ARRAY *array, unsigned int type, int size);
PUBLIC int GB_ArrayCount(GB_ARRAY array);
PUBLIC void *GB_ArrayAdd(GB_ARRAY array);
PUBLIC void *GB_ArrayGet(GB_ARRAY array, int index);

PUBLIC void GB_CollectionNew(GB_COLLECTION *col, int mode);
PUBLIC int GB_CollectionCount(GB_COLLECTION col);
PUBLIC void GB_CollectionSet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value);
PUBLIC int GB_CollectionGet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value);
PUBLIC int GB_CollectionEnum(GB_COLLECTION col, GB_VARIANT *value, char **key, int *len);

PUBLIC void GB_Alloc(void **addr, int len);
PUBLIC void GB_Free(void **addr);
PUBLIC void GB_Realloc(void **addr, int len);

PUBLIC void GB_FreeString(char **str);
PUBLIC int GB_StringLength(const char *str);
PUBLIC bool GB_ConvString(char **result, const char *str, int len, const char *src, const char *dst);

PUBLIC int GB_Conv(GB_VALUE *, GB_TYPE);

PUBLIC int GB_NumberToString(int flag, double value, const char *format, char **str, int *len);

PUBLIC void GB_HashTableNew(GB_HASHTABLE *hash, int mode);
PUBLIC void GB_HashTableAdd(GB_HASHTABLE hash, const char *key, int len, void *data);
PUBLIC void GB_HashTableRemove(GB_HASHTABLE hash, const char *key, int len);
PUBLIC int GB_HashTableGet(GB_HASHTABLE hash, const char *key, int len, void **data);
PUBLIC void GB_HashTableEnum(GB_HASHTABLE hash, GB_HASHTABLE_ENUM_FUNC func);

PUBLIC void GB_NewArray(void *pdata, int size, int count);
PUBLIC int GB_CountArray(void *data);
PUBLIC void *GB_Add(void *pdata);

PUBLIC void GB_StreamInit(GB_STREAM *, int fd);

PUBLIC int GB_tolower(int c);
PUBLIC int GB_toupper(int c);

PUBLIC void GB_SubCollectionNew(GB_SUBCOLLECTION *subcollection, GB_SUBCOLLECTION_DESC *desc, void *container);
PUBLIC void GB_SubCollectionAdd(void *_object, const char *key, int len, void *value);
PUBLIC void GB_SubCollectionRemove(void *_object, const char *key, int len);
PUBLIC void *GB_SubCollectionContainer(void *_object);
PUBLIC void *GB_SubCollectionGet(void *_object, const char *key, int len);

PUBLIC void GB_PrintData(GB_TYPE type, void *addr);

PUBLIC int GB_ImageCreate(GB_IMAGE *image, void *data, int width, int height, int format);
PUBLIC void GB_ImageInfo(GB_IMAGE image, GB_IMAGE_INFO *info);
PUBLIC void GB_ImageConvert(void *dst, int dst_format, void *src, int src_format, int w, int h);
PUBLIC int GB_PictureCreate(GB_PICTURE *picture, void *data, int width, int height, int format);
PUBLIC void GB_PictureInfo(GB_PICTURE picture, GB_PICTURE_INFO *info);

PUBLIC void *GB_DebugGetClass(const char *name);
PUBLIC void *GB_DebugGetExec(void);

#define GB_PrintString PRINT_string

#ifndef __GBX_API_C
EXTERN void *GAMBAS_Api[];
EXTERN void *GAMBAS_DebugApi[];
EXTERN unsigned int GAMBAS_ReturnType;
EXTERN unsigned int GAMBAS_MissingParam;
EXTERN bool GAMBAS_Error;
EXTERN bool GAMBAS_DoNotRaiseEvent;
EXTERN bool GAMBAS_StopEvent;
#endif

#endif

