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

int GB_GetInterface(const char *library, int version, void *iface);
void *GB_Hook(int type, void *hook);

int GB_LoadComponent(const char *name);
const char *GB_CurrentComponent(void);

void GB_Push(int nval, ...);
int GB_CanRaise(void *object, int event_id);
int GB_Raise(void *object, int event_id, int nparam, ...);
int GB_GetEvent(void *class, char *name);
char *GB_GetLastEventName();
int GB_Stopped(void);

int GB_NParam(void);
const char *GB_GetUnknown(void);
int GB_IsProperty(void);

void GB_Error(const char *msg, ...);

void GB_Ref(void *object);
void GB_Unref(void **object);
void GB_UnrefKeep(void **object, int);

void GB_StopEnum(void);
void *GB_GetEnum(void);
void GB_ListEnum(void *);
int GB_NextEnum(void);
void GB_StopAllEnum(void *);

void GB_Return(unsigned int type, ...);
void GB_ReturnInteger(int val);
#define GB_ReturnInt GB_ReturnInteger
void GB_ReturnLong(int64_t val);
void GB_ReturnPointer(void *val);
void GB_ReturnBoolean(int val);
void GB_ReturnObject(void *val);
void GB_ReturnNull(void);
void GB_ReturnFloat(double val);
void GB_ReturnPtr(unsigned int type, void *value);
void GB_ReturnDate(GB_DATE *date);

void GB_ReturnString(char *str);
void GB_ReturnConstString(const char *str, int len);
void GB_ReturnConstZeroString(const char *str);
void GB_ReturnNewString(const char *src, int len);
void GB_ReturnNewZeroString(const char *src);

void *GB_GetClass(void *object);
char *GB_GetClassName(void *object);
void *GB_FindClass(const char *name);
int GB_ExistClass(const char *name);
int GB_ExistClassLocal(const char *name);

char *GB_ToZeroString(GB_STRING *src);

int GB_LoadFile(const char *path, int lenp, char **addr, int *len);
void GB_ReleaseFile(char **addr, int len);
char *GB_GetTempDir(void);
char *GB_RealFileName(const char *path, int len);

int GB_IsMissing(int param);

void GB_Attach(void *object, void *parent, const char *name);
void GB_Detach(void *object);

void GB_Store(GB_TYPE type, GB_VALUE *src, void *dst);
void GB_StoreString(GB_STRING *src, char **dst);
void GB_StoreObject(GB_OBJECT *object, void **dst);
void GB_StoreVariant(GB_VARIANT *src, void *dst);

void GB_Watch(int fd, int flag, void *callback, int param);

int GB_New(void **object, void *class_name, const char *name, void *parent);
int GB_CheckObject(void *object);
int GB_Is(void *object, void *class);

int GB_GetFunction(GB_FUNCTION *func, void *object, const char *name, const char *sign, const char *type);
GB_VALUE *GB_Call(GB_FUNCTION *func, int nparam, int release);
void *GB_GetClassInterface(void *class, const char *name);

const char *GB_AppName(void);
const char *GB_AppTitle(void);
const char *GB_AppVersion(void);
const char *GB_AppPath(void);
const char *GB_AppStartup(void);

char *GB_SystemCharset(void);

void *GB_Eval(void *, void *);

void GB_ArrayNew(GB_ARRAY *array, unsigned int type, int size);
int GB_ArrayCount(GB_ARRAY array);
void *GB_ArrayAdd(GB_ARRAY array);
void *GB_ArrayGet(GB_ARRAY array, int index);

void GB_CollectionNew(GB_COLLECTION *col, int mode);
int GB_CollectionCount(GB_COLLECTION col);
void GB_CollectionSet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value);
int GB_CollectionGet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value);
int GB_CollectionEnum(GB_COLLECTION col, GB_VARIANT *value, char **key, int *len);

void GB_Alloc(void **addr, int len);
void GB_Free(void **addr);
void GB_Realloc(void **addr, int len);

void GB_FreeString(char **str);
int GB_StringLength(const char *str);
bool GB_ConvString(char **result, const char *str, int len, const char *src, const char *dst);

int GB_Conv(GB_VALUE *, GB_TYPE);

int GB_NumberToString(int flag, double value, const char *format, char **str, int *len);

void GB_HashTableNew(GB_HASHTABLE *hash, int mode);
void GB_HashTableAdd(GB_HASHTABLE hash, const char *key, int len, void *data);
void GB_HashTableRemove(GB_HASHTABLE hash, const char *key, int len);
int GB_HashTableGet(GB_HASHTABLE hash, const char *key, int len, void **data);
void GB_HashTableEnum(GB_HASHTABLE hash, GB_HASHTABLE_ENUM_FUNC func);

void GB_NewArray(void *pdata, int size, int count);
int GB_CountArray(void *data);
void *GB_Add(void *pdata);

void GB_StreamInit(GB_STREAM *, int fd);

int GB_tolower(int c);
int GB_toupper(int c);

void GB_SubCollectionNew(GB_SUBCOLLECTION *subcollection, GB_SUBCOLLECTION_DESC *desc, void *container);
void GB_SubCollectionAdd(void *_object, const char *key, int len, void *value);
void GB_SubCollectionRemove(void *_object, const char *key, int len);
void *GB_SubCollectionContainer(void *_object);
void *GB_SubCollectionGet(void *_object, const char *key, int len);

void GB_PrintData(GB_TYPE type, void *addr);

int GB_ImageCreate(GB_IMAGE *image, void *data, int width, int height, int format);
void GB_ImageInfo(GB_IMAGE image, GB_IMAGE_INFO *info);
void GB_ImageConvert(void *dst, int dst_format, void *src, int src_format, int w, int h);
int GB_PictureCreate(GB_PICTURE *picture, void *data, int width, int height, int format);
void GB_PictureInfo(GB_PICTURE picture, GB_PICTURE_INFO *info);

void *GB_DebugGetClass(const char *name);
void *GB_DebugGetExec(void);

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

