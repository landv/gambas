/***************************************************************************

  gbx_api.h

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

#ifndef __GBX_API_H
#define __GBX_API_H

#include "gambas.h"
#include "gbx_type.h"
#include "gbx_stream.h"
#include "gb_hash.h"

bool GB_GetInterface(const char *library, int version, void *iface);
void *GB_Hook(int type, void *hook);

bool GB_LoadComponent(const char *name);
const char *GB_CurrentComponent(void);

void GB_Wait(int);
void GB_Push(int nval, ...);
bool GB_CanRaise(void *object, int event_id);
bool GB_Raise(void *object, int event_id, int nparam, ...);
void GB_RaiseBegin(GB_RAISE_HANDLER *handler);
void GB_RaiseEnd(GB_RAISE_HANDLER *handler);
int GB_GetEvent(void *class, char *name);
char *GB_GetLastEventName();
bool GB_Stopped(void);

int GB_NParam(void);
const char *GB_GetUnknown(void);

bool GB_GetProperty(void *object, const char *property);
bool GB_SetProperty(void *object, const char *name, GB_VALUE *value);

bool GB_Serialize(const char *path, GB_VALUE *value);
bool GB_UnSerialize(const char *path, GB_VALUE *value);

void GB_Error(const char *msg, ...);
void GB_Deprecated(const char *msg, const char *func, const char *repl);
void GB_OnErrorBegin(GB_ERROR_HANDLER *handler);
void GB_OnErrorEnd(GB_ERROR_HANDLER *handler);

void GB_Ref(void *object);
void GB_Unref(void **object);
void GB_UnrefKeep(void **object, int);

void GB_StopEnum(void);
void *GB_GetEnum(void);
void *GB_BeginEnum(void *);
void GB_EndEnum(void *);
bool GB_NextEnum(void);
void GB_StopAllEnum(void *);

void GB_Return(GB_TYPE type, ...);
void GB_ReturnInteger(int val);
#define GB_ReturnInt GB_ReturnInteger
void GB_ReturnLong(int64_t val);
void GB_ReturnPointer(void *val);
void GB_ReturnBoolean(int val);
void GB_ReturnObject(void *val);
void GB_ReturnNull(void);
void GB_ReturnSingle(float val);
void GB_ReturnFloat(double val);
void GB_ReturnPtr(GB_TYPE type, void *value);
void GB_ReturnDate(GB_DATE *date);
void GB_ReturnSelf(void *object);
void GB_ReturnVariant(GB_VARIANT_VALUE *value);

void GB_ReturnConvVariant(void);
void GB_ReturnBorrow(void);
void GB_ReturnRelease(void);

void GB_ReturnString(char *str);
void GB_ReturnVoidString(void);
void GB_ReturnConstString(const char *str, int len);
void GB_ReturnConstZeroString(const char *str);
void GB_ReturnNewString(const char *src, int len);
void GB_ReturnNewZeroString(const char *src);

void *GB_GetClass(void *object);
char *GB_GetClassName(void *object);
void *GB_FindClass(const char *name);
bool GB_ExistClass(const char *name);
bool GB_ExistClassLocal(const char *name);

char *GB_ToZeroString(GB_STRING *src);

bool GB_LoadFile(const char *path, int lenp, char **addr, int *len);
bool GB_ExistFile(const char *path);
//void GB_ReleaseFile(char **addr, int len);
#define GB_ReleaseFile STREAM_unmap
char *GB_RealFileName(const char *path, int len);
char *GB_TempDir(void);
char *GB_TempFile(const char *pattern);
bool GB_CopyFile(const char *src, const char *dst);
//int GB_FindFile(const char *dir, int recursive, int follow, void (*found)(const char *));
bool GB_StatFile(const char *path, GB_FILE_STAT *info, bool follow);
void GB_BrowseProject(GB_BROWSE_PROJECT_CALLBACK func);
void GB_BrowseDirectory(const char *dir, GB_BROWSE_CALLBACK before, GB_BROWSE_CALLBACK after);

int GB_IsMissing(int param);

void GB_Attach(void *object, void *parent, const char *name);
void GB_Detach(void *object);

void GB_Store(GB_TYPE type, GB_VALUE *src, void *dst);
void GB_StoreString(GB_STRING *src, char **dst);
void GB_StoreObject(GB_OBJECT *object, void **dst);
void GB_StoreVariant(GB_VARIANT *src, void *dst);
void GB_BorrowValue(GB_VALUE *value);
void GB_ReleaseValue(GB_VALUE *value);

void GB_Watch(int fd, int flag, void *callback, intptr_t param);

void *GB_Create(void *class_name, const char *name, void *parent);
void *GB_New(void *class_name, const char *name, void *parent);
bool GB_CheckObject(void *object);
bool GB_Is(void *object, void *class);

bool GB_GetFunction(GB_FUNCTION *func, void *object, const char *name, const char *sign, const char *type);
GB_VALUE *GB_Call(GB_FUNCTION *func, int nparam, int release);
void *GB_GetClassInterface(void *class, const char *name);

const char *GB_AppName(void);
const char *GB_AppTitle(void);
const char *GB_AppVersion(void);
const char *GB_AppPath(void);
void *GB_AppStartupClass(void);

char *GB_SystemCharset(void);
char *GB_SystemDomainName(void);
bool GB_IsRightToLeft(void);
char *GB_SystemPath(void);
bool GB_SystemDebug(void);

void *GB_Eval(void *, void *);

void GB_ArrayNew(GB_ARRAY *array, TYPE type, int size);
int GB_ArrayCount(GB_ARRAY array);
void *GB_ArrayAdd(GB_ARRAY array);
void *GB_ArrayGet(GB_ARRAY array, int index);
TYPE GB_ArrayType(GB_ARRAY array);

void GB_CollectionNew(GB_COLLECTION *col, int mode);
int GB_CollectionCount(GB_COLLECTION col);
bool GB_CollectionSet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value);
bool GB_CollectionGet(GB_COLLECTION col, const char *key, int len, GB_VARIANT *value);
bool GB_CollectionEnum(GB_COLLECTION col, GB_COLLECTION_ITER *iter, GB_VARIANT *value, char **key, int *len);

void GB_Alloc(void **addr, int len);
void GB_AllocZero(void **addr, int len);
void GB_Free(void **addr);
void GB_Realloc(void **addr, int len);

char *GB_NewZeroString(char *src);
char *GB_TempString(char *src, int len);
void GB_FreeString(char **str);
int GB_StringLength(const char *str);
bool GB_ConvString(char **result, const char *str, int len, const char *src, const char *dst);

bool GB_Conv(GB_VALUE *, GB_TYPE);

bool GB_NumberToString(int flag, double value, const char *format, char **str, int *len);

void GB_HashTableNew(GB_HASHTABLE *hash, int mode);
void GB_HashTableAdd(GB_HASHTABLE hash, const char *key, int len, void *data);
void GB_HashTableRemove(GB_HASHTABLE hash, const char *key, int len);
bool GB_HashTableGet(GB_HASHTABLE hash, const char *key, int len, void **data);
void GB_HashTableEnum(GB_HASHTABLE hash, GB_HASHTABLE_ENUM_FUNC func);

void GB_NewArray(void *pdata, int size, int count);
int GB_CountArray(void *data);
void *GB_Add(void *pdata);

GB_STREAM *GB_StreamGet(void *);
void GB_StreamSetBytesRead(GB_STREAM *, int);
void GB_StreamSetSwapping(GB_STREAM *, int);
void GB_StreamSetAvailableNow(GB_STREAM *, int);
bool GB_StreamBlock(GB_STREAM *, int);
int GB_StreamRead(GB_STREAM *stream, void *addr, int len);
int GB_StreamWrite(GB_STREAM *stream, void *addr, int len);

int GB_tolower(int c);
int GB_toupper(int c);

void *GB_DebugGetClass(const char *name);
void *GB_DebugGetExec(void);
void GB_DebugBreakOnError(bool);

#define GB_PrintString PRINT_string

#ifndef __GBX_API_C
EXTERN void *GAMBAS_Api[];
EXTERN void *GAMBAS_DebugApi[];
EXTERN void *GAMBAS_JitApi[];
EXTERN unsigned int GAMBAS_MissingParam;
EXTERN bool GAMBAS_DoNotRaiseEvent;
EXTERN bool GAMBAS_StopEvent;
#endif

#endif

