/***************************************************************************

  CApplication.cpp

  (c) 2000-2009 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __CAPPLICATION_CPP


#include <qcstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qvariant.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdatastream.h>
#include <kurl.h>
#include <kprocess.h>

#include "gambas.h"
#include "main.h"

#include "../share/CPicture.h"
#include "../share/CImage.h"
#include "../share/CMouse.h"
#include "../share/CFont.h"

#include "CApplication.h"


//#define DEBUG_ME


typedef
  struct {
    const char *dcop;
    GB_TYPE type;
    }
  TYPE_CONV;

enum {
  QT_T_VOID,
  QT_T_ASYNC,
  QT_T_BOOL,
  QT_T_INT,
  QT_T_UNSIGNED,
  QT_T_UINT,
  QT_T_LONG,
  QT_T_ULONG,
  QT_T_QCOLOR,
  QT_T_FLOAT,
  QT_T_DOUBLE,
  QT_T_QCSTRING,
  QT_T_QSTRING,
  QT_T_KURL,
  QT_T_KURL_LIST,
  QT_T_QVARIANT,
  QT_T_QCSTRINGLIST,
  QT_T_QSTRINGLIST,
  QT_T_QMAP_QCSTRING_QSTRING,
  QT_T_QMAP_QSTRING_QSTRING,
  QT_T_DCOPREF,
  QT_T_QVALUELIST_DCOPREF,
  QT_T_QMAP_QCSTRING_DCOPREF,
  QT_T_QMAP_QSTRING_DCOPREF,
  QT_T_QPIXMAP,
  QT_T_QIMAGE,
  QT_T_QCURSOR,
  QT_T_QPOINT,
  QT_T_QSIZE,
  QT_T_QRECT,
  QT_T_QFONT,
};

static TYPE_CONV type_conv[] =
{
  { "void",                     GB_T_VOID               },
  { "ASYNC",                    GB_T_VOID               },

  { "bool",                     GB_T_BOOLEAN            },

  { "int",                      GB_T_INTEGER            },
  { "unsigned",                 GB_T_INTEGER            },
  { "uint",                     GB_T_INTEGER            },
  { "long",                     GB_T_INTEGER            },
  { "ulong",                    GB_T_INTEGER            },
  { "QColor",                   GB_T_INTEGER            },

  { "float",                    GB_T_FLOAT              },
  { "double",                   GB_T_FLOAT              },

  { "QCString",                 GB_T_STRING             },
  { "QString",                  GB_T_STRING             },
  { "KURL",                     GB_T_STRING             },
  { "KURL::List",               (GB_TYPE)"String[]"     },

  { "QVariant",                 GB_T_VARIANT            },

  { "QCStringList",             (GB_TYPE)"String[]"     },
  { "QStringList",              (GB_TYPE)"String[]"     },
  { "QMap<QCString,QString>",    (GB_TYPE)"Collection"   },
  { "QMap<QString,QString>",    (GB_TYPE)"Collection"   },

  { "DCOPRef",                  (GB_TYPE)"DCOPRef"      },
  { "QValueList<DCOPRef>",      (GB_TYPE)"Object[]"     },
  { "QMap<QCString,DCOPRef>",    (GB_TYPE)"Collection"   },
  { "QMap<QString,DCOPRef>",    (GB_TYPE)"Collection"   },

  { "QPixmap",                  (GB_TYPE)"Picture"      },
  { "QImage",                   (GB_TYPE)"Image"        },
  { "QCursor",                  (GB_TYPE)"Cursor"       },
  { "QPoint",                   (GB_TYPE)"Integer[]"    },
  { "QSize",                    (GB_TYPE)"Integer[]"    },
  { "QRect",                    (GB_TYPE)"Integer[]"    },
  { "QFont",                    (GB_TYPE)"Font"         },

  { NULL }
};


static QAsciiDict< CAPPLICATION > appCache;


static int get_type(const char *type)
{
  TYPE_CONV *p;
  int i;

  #ifdef DEBUG_ME
  qDebug("get_type(%s)", type);
  #endif

  if (type == 0 || *type == 0)
    return 0;

  for (p = type_conv, i = 0; p->dcop; p++, i++)
    if (strcasecmp(p->dcop, type) == 0)
    {
      #ifdef DEBUG_ME
      qDebug("-> %d", i);
      #endif
      return i;
    }

  return (-1);
}


static void flush_application()
{
  QAsciiDictIterator<CAPPLICATION> it(appCache);
  CAPPLICATION *app;

  //qDebug("flush_application");

  while (it.current())
  {
    //delete it.current()->pixmap;
    app = it.current();
    GB.Unref(POINTER(&app));
    ++it;
  }
}

static CAPPLICATION *get_application(const char *name, bool exec = true)
{
  QCString app;
  CAPPLICATION *_object = 0;
  bool reg;
  DCOPClient *dcop = kapp->dcopClient();

  app = name;
  reg = dcop->isApplicationRegistered(app);

  if (!reg)
  {
    QCStringList apps = dcop->registeredApplications();
    QCString appId = dcop->appId();

    for (QCStringList::ConstIterator it = apps.begin(); it != apps.end(); ++it)
    {
      if ((*it) != appId && (*it).left(9) != "anonymous")
      {
        if ((*it).find(app) == 0)
        {
          app = *it;
          reg = true;
          break;
        }
      }
    }
  }

  #ifdef DEBUG_ME
  if (reg)
    qDebug("Application is already registered");
  #endif

  if (!reg)
  {
    QString sName(name);
    QString url;
    QString error;
    int i;

    i = sName.find(' ');
    if (i >= 0)
    {
      url = sName.mid(i + 1).stripWhiteSpace();
      sName = sName.left(i).stripWhiteSpace();
    }

    if (exec)
    {
      #ifdef DEBUG_ME
      qDebug("starting %s...", sName.latin1());
      #endif
      kapp->startServiceByDesktopName(sName, url, &error);
    }

    if (error.length())
    {
      GB.Error("Cannot start KDE application: &1", error.latin1());
      return NULL;
    }
    else
      return get_application(name, false);
  }

  if (!appCache.isEmpty())
    _object = appCache[app];

  if (!_object)
  {
    GB.New(POINTER(&_object), GB.FindClass("KDEApplication"), NULL, NULL);
    GB.Ref(THIS);

    GB.NewString(&THIS->name, app, app.length());

    THIS->object = NULL;
    THIS->cache = new QAsciiDict< QDict<CFunction> >(17, false);
    THIS->cache->setAutoDelete(true);

    appCache.insert(app, THIS);
  }

  return THIS;
}

static CDCOPREF *make_dcopref(DCOPRef& dcopref)
{
  CDCOPREF *_object;

  if (dcopref.isNull())
    return NULL;

  GB.New(POINTER(&_object), GB.FindClass("DCOPRef"), NULL, NULL);
  _object->ref = new DCOPRef(dcopref);

  //qDebug("Create DCOPREF %p", _object);

  return _object;
}


BEGIN_METHOD_VOID(CAPPLICATION_exit)

  flush_application();

END_METHOD


BEGIN_METHOD_VOID(CAPPLICATION_free)

  GB.FreeString(&THIS->name);
  GB.FreeString(&THIS->object);
  delete THIS->cache;
  THIS->cache = 0;

END_METHOD


BEGIN_PROPERTY(CAPPLICATION_name)

  GB.ReturnString(THIS->name);

END_PROPERTY


BEGIN_METHOD(CAPPLICATION_get, GB_STRING name)

  GB.ReturnObject(get_application(GB.ToZeroString(ARG(name))));

END_METHOD


static void get_object(CAPPLICATION *_object, const char *name)
{
  GB.FreeString(&THIS->object);
  GB.NewString(&THIS->object, name, 0);

  GB.ReturnObject(THIS);
}


BEGIN_METHOD(CAPPLICATION_get_object, GB_STRING object)

  get_object(THIS, GB.ToZeroString(ARG(object)));

END_METHOD

static CFunction *get_function(CAPPLICATION *_object, const char *iname, const char *name, int nparam)
{
  QDict<CFunction> *iface = 0;
  CFunction *func = NULL;

  if (!iname)
    iname = "default";

  if (!THIS->cache->isEmpty())
    iface = (*THIS->cache)[iname];

  if (iface == 0)
  {
    bool ok;
    QCStringList funcs;
    QString dcsign, dctype, dcname;
    QStringList dcargs;
    int l, r, i;

    iface = new QDict<CFunction>(17, false);
    iface->setAutoDelete(true);

    #ifdef DEBUG_ME
    qDebug("**** %s.%s", THIS->name, iname);
    #endif

    funcs = kapp->dcopClient()->remoteFunctions(THIS->name, iname, &ok);
    if (!ok)
      return NULL;

    for (QCStringList::ConstIterator it = funcs.begin(); it != funcs.end(); ++it)
    {
      //dcsign = kapp->dcopClient()->normalizeFunctionSignature(*it);
      dcsign = *it;

      /* Type DCOP de retour */

      i = dcsign.find(" ");
      if (i >= 0)
        dctype = dcsign.left(i);
      dcsign = dcsign.mid(i + 1);

      l = dcsign.find('(');
      r = dcsign.findRev(')');

      if (l < 0 || r < 0) continue;

      /* Nom DCOP de la m�hode */

      dcname = dcsign.left(l).stripWhiteSpace();
      dcsign = dcsign.mid(l + 1, r - l - 1);

      /* Types des arguments de la m�hode */

      dcargs = QStringList::split(',', dcsign);

      for (QStringList::Iterator it = dcargs.begin(); it != dcargs.end(); ++it)
      {
        (*it) = (*it).stripWhiteSpace();

        i = (*it).find(' ');
        if (i >= 0)
          (*it) = (*it).left(i);
      }

      func = new CFunction();

      func->dcopName = dcname + "(" + dcargs.join(",") + ")";
      #ifdef DEBUG_ME
      qDebug("dcopName = %s", (const char *)func->dcopName);
      #endif

      /* Nom Gambas de la m�hode */

      dcname += QString::number(dcargs.count());

      QString dcname2 = dcname;

      for(i = 1; i < 256; i++)
      {
        dcname2 = dcname;
        if (i > 1) dcname2 += QString::number(i);

        if (iface->isEmpty() || iface->find(dcname2) == 0)
        {
          func->name = dcname2;
          break;
        }
      }

      #ifdef DEBUG_ME
      qDebug("name = %s", (const char *)func->name);
      #endif

      func->type = get_type(dctype.latin1());

      #ifdef DEBUG_ME
      qDebug("type = %s", (const char *)dctype);
      #endif

      func->args = new int[dcargs.count()];

      #ifdef DEBUG_ME
      for (QStringList::Iterator it = dcargs.begin(); it != dcargs.end(); ++it)
        qDebug("arg = %s", (*it).latin1());
      qDebug(" ");
      #endif

      i = 0;
      for (QStringList::Iterator it = dcargs.begin(); it != dcargs.end(); ++it, ++i)
        func->args[i] = get_type((*it).latin1());

      iface->insert((const char *)func->name, func);
    }

    THIS->cache->insert(iname, iface);
  }

  if (!iface->isEmpty())
    func = (*iface)[QString(name) + QString::number(nparam)];

  return func;
}


#define GET_STRING(_arg) (TO_QSTRING(GB.ToZeroString((GB_STRING *)_arg)))
#define GET_OBJECT(_arg, _type) ((_type *)(((GB_OBJECT *)_arg)->value))

static bool call_method(CAPPLICATION *_object, const char *object, const char *name, GB_VALUE *args, int nparam, bool error)
{
  int i, n, ret, j;
  GB_VALUE *arg;
  bool ok;
  int type;
  GB_TYPE gtype;
  QByteArray data, replyData;
  CFunction *func;
  void *ob = NULL;

  n = nparam;

  func = get_function(THIS, object, name, n);

  if (!func)
  {
    if (error)
    {
      if (object)
        GB.Error("Unknown method: &1.&2.&3", THIS->name, object, name);
      else
        GB.Error("Unknown method: &1.&2", THIS->name, name);
    }
    return true;
  }

  #ifdef DEBUG_ME
  qDebug("call_method: %s.%s.%s", THIS->name, object, name);
  #endif

  /* Conversion & Marshalling */

  QDataStream d(data, IO_WriteOnly);

  for (i = 0; i < n; i++)
  {
    arg = &args[i];
    type = func->args[i];

    if (type < 0)
    {
      GB.Error("Cannot handle datatype");
      return true;
    }

    gtype = type_conv[type].type;

    if (gtype < GB_T_OBJECT)
      ret = GB.Conv(arg, gtype);
    else
    {
      ret = GB.Conv(arg, (GB_TYPE)GB.FindClass((const char *)gtype));
      if (!ret)
      {
        ob = GET_OBJECT(arg, void);
        ret = GB.CheckObject(ob);
      }
    }

    if (ret)
      return true;

    if (type == QT_T_INT)
      d << (int)((GB_INTEGER *)arg)->value;
    else if (type == QT_T_UNSIGNED || type == QT_T_UINT)
      d << (unsigned int)((GB_INTEGER *)arg)->value;
    else if (type == QT_T_LONG)
      d << (long)((GB_INTEGER *)arg)->value;
    else if (type == QT_T_ULONG)
      d << (unsigned long)((GB_INTEGER *)arg)->value;
    else if (type == QT_T_FLOAT)
      d << (float)((GB_FLOAT *)arg)->value;
    else if (type == QT_T_DOUBLE)
      d << (double)((GB_FLOAT *)arg)->value;
    else if (type == QT_T_BOOL)
      d << (bool)((GB_BOOLEAN *)arg)->value;
    else if (type == QT_T_QCSTRING)
      d << QCString(GET_STRING(arg));
    else if (type == QT_T_QSTRING)
      d << GET_STRING(arg);
    else if (type == QT_T_QCSTRINGLIST)
    {
      GB_ARRAY a = (GB_ARRAY)ob;
      QCStringList list;
      char *val;

      for (j = 0; j < GB.Array.Count(a); j++)
      {
        val = *((char **)GB.Array.Get(a, j));
        list.append(QCString(TO_QSTRING(val)));
      }

      d << list;
    }
    else if (type == QT_T_QSTRINGLIST)
    {
      GB_ARRAY a = (GB_ARRAY)ob;
      QStringList list;
      char *val;

      for (j = 0; j < GB.Array.Count(a); j++)
      {
        val = *((char **)GB.Array.Get(a, j));
        list.append(TO_QSTRING(val));
      }

      d << list;
    }
    else if (type == QT_T_KURL)
      d << KURL(GET_STRING(arg));
    else if (type == QT_T_KURL_LIST)
    {
      GB_ARRAY a = (GB_ARRAY)ob;
      KURL::List list;
      char *url;

      for (j = 0; j < GB.Array.Count(a); j++)
      {
        url = *((char **)GB.Array.Get(a, j));
        list.append(KURL(TO_QSTRING(url)));
      }

      d << list;
    }
    else if (type == QT_T_QPIXMAP)
      d << *(GET_OBJECT(arg, CPICTURE)->pixmap);
    else if (type == QT_T_QIMAGE)
      d << *(QT.GetImage(GET_OBJECT(arg, CIMAGE)));
    else if (type == QT_T_QCURSOR)
      d << *(GET_OBJECT(arg, CCURSOR)->cursor);
    else if (type == QT_T_QPOINT)
    {
      GB_ARRAY a = (GB_ARRAY)ob;

      if (GB.Array.Count(a) != 2)
      {
        GB.Error("Cannot convert datatype");
        return true;
      }

      QPoint p(*((int *)GB.Array.Get(a, 0)), *((int *)GB.Array.Get(a, 1)));
      d << p;
    }
    else if (type == QT_T_QSIZE)
    {
      GB_ARRAY a = (GB_ARRAY)ob;

      if (GB.Array.Count(a) != 2)
      {
        GB.Error("Cannot convert datatype");
        return true;
      }

      QSize s(*((int *)GB.Array.Get(a, 0)), *((int *)GB.Array.Get(a, 1)));
      d << s;
    }
    else if (type == QT_T_QRECT)
    {
      GB_ARRAY a = (GB_ARRAY)ob;

      if (GB.Array.Count(a) != 4)
      {
        GB.Error("Cannot convert datatype");
        return true;
      }

      QRect r(*((int *)GB.Array.Get(a, 0)), *((int *)GB.Array.Get(a, 1)), *((int *)GB.Array.Get(a, 2)), *((int *)GB.Array.Get(a, 3)));
      d << r;
    }
    else if (type == QT_T_QFONT)
    {
      d << *(GET_OBJECT(arg, CFONT)->font);
    }
    else
    {
      GB.Error("Cannot handle datatype");
      return true;
    }
  }

  QCString cname(THIS->name);
  QCString cobject(object);
  QCString ctype;

  ok = kapp->dcopClient()->call
    (
     cname,
     cobject,
     func->dcopName,
     data,
     ctype,
     replyData
    );

  if (!ok)
  {
    if (kapp->dcopClient()->isApplicationRegistered(THIS->name))
      GB.Error("DCOP call failed");
    else
      GB.Error("Application is not registered anymore");

    return true;
  }

  /* Demarshalling */

  type = get_type(ctype);

  if (type == QT_T_VOID || type == QT_T_ASYNC)
    return false;

  QDataStream reply(replyData, IO_ReadOnly);

  if (type == QT_T_QVARIANT)
  {
    Q_INT32 vt;
    QString vtn;
    reply >> vt;
    vtn = QVariant::typeToName((QVariant::Type)vt);
    type = get_type(vtn.latin1());
  }

  if (type == QT_T_INT)
  {
    int i;
    reply >> i;
    GB.ReturnInteger(i);
  }
  else if (type == QT_T_UINT)
  {
    uint i;
    reply >> i;
    GB.ReturnInteger(i);
  }
  else if (type == QT_T_LONG)
  {
    long l;
    reply >> l;
    GB.ReturnInteger(l);
  }
  else if (type == QT_T_ULONG)
  {
    ulong l;
    reply >> l;
    GB.ReturnInteger((long)l);
  }
  else if (type == QT_T_FLOAT)
  {
    float f;
    reply >> f;
    GB.ReturnFloat(f);
  }
  else if (type == QT_T_DOUBLE)
  {
    double d;
    reply >> d;
    GB.ReturnFloat(d);
  }
  else if (type == QT_T_BOOL)
  {
    bool b;
    reply >> b;
    GB.ReturnBoolean(b ? 1 : 0);
  }
  else if (type == QT_T_QSTRING)
  {
    QString s;
    reply >> s;
    GB.ReturnNewZeroString(s);
  }
  else if (type == QT_T_QCSTRING)
  {
    QCString r;
    reply >> r;
    GB.ReturnNewZeroString(r);
  }
  else if (type == QT_T_KURL)
  {
    KURL r;
    reply >> r;
    GB.ReturnNewZeroString(r.prettyURL());
  }
  else if (type == QT_T_QCOLOR)
  {
    QColor r;
    reply >> r;
    GB.ReturnInteger(r.rgb() & 0xFFFFFF);
  }
  else if (type == QT_T_QSTRINGLIST)
  {
    GB_ARRAY array;
    const char *data;
    QStringList r;
    reply >> r;

    n = r.count();

    GB.Array.New(&array, GB_T_STRING, n);
    for (i = 0; i < n; i++)
    {
      data = TO_UTF8(r[i]);
      GB.NewString((char **)GB.Array.Get(array, i), data, 0);
    }

    GB.ReturnObject(array);
  }
  else if (type == QT_T_QCSTRINGLIST)
  {
    GB_ARRAY array;
    QCStringList r;
    reply >> r;

    n = r.count();

    GB.Array.New(&array, GB_T_STRING, n);

    for (i = 0; i < n; i++)
      GB.NewString((char **)GB.Array.Get(array, i), (const char *)r[i], 0);

    GB.ReturnObject(array);
  }
  else if (type == QT_T_QMAP_QCSTRING_QSTRING)
  {
    GB_COLLECTION col;
    GB_VARIANT value;
    const char *key;
    char *data;

    QMap<QCString,QString> r;
    reply >> r;

    GB.Collection.New(&col, GB_COMP_BINARY);

    QMap<QCString,QString>::Iterator it;

    for (it = r.begin(); it != r.end(); it++)
    {
      GB.NewString(&data, TO_UTF8(it.data()), 0);
      key = (const char *)it.key();

      value.type = GB_T_VARIANT;
      value.value.type = GB_T_STRING;
      value.value._string.value = (char *)data;

      GB.Collection.Set(col, key, 0, &value);

      GB.FreeString(&data);
    }

    GB.ReturnObject(col);
  }
  else if (type == QT_T_QMAP_QSTRING_QSTRING)
  {
    GB_COLLECTION col;
    GB_VARIANT value;
    QCString ckey;
    const char *key;
    char *data;

    QMap<QString,QString> r;
    reply >> r;

    GB.Collection.New(&col, GB_COMP_BINARY);

    QMap<QString,QString>::Iterator it;

    for (it = r.begin(); it != r.end(); it++)
    {
      //ckey = it.data().utf8();
      //key = TO_UTF8(it.data());
      //GB.NewString(&data, (const char *)ckey, 0);
      //ckey = it.key().utf8();

      key = TO_UTF8(it.key());
      GB.NewString(&data, TO_UTF8(it.data()), 0);

      value.type = GB_T_VARIANT;
      value.value.type = GB_T_STRING;
      value.value._string.value = data;

      GB.Collection.Set(col, key, 0, &value);

      GB.FreeString(&data);
    }

    GB.ReturnObject(col);
  }
  else if (type == QT_T_DCOPREF)
  {
    DCOPRef r;
    reply >> r;
    GB.ReturnObject(make_dcopref(r));
  }
  else if (type == QT_T_QVALUELIST_DCOPREF)
  {
    GB_ARRAY array;
    QValueList<DCOPRef> r;
    reply >> r;
    CDCOPREF *ref;

    n = r.count();

    GB.Array.New(&array, GB_T_OBJECT, n);

    for (i = 0; i < n; i++)
    {
      ref = make_dcopref(r[i]);
      GB.Ref(ref);
      *((CDCOPREF **)GB.Array.Get(array, i)) = ref;
    }

    GB.ReturnObject(array);
  }
  else if (type == QT_T_QMAP_QSTRING_DCOPREF)
  {
    GB_COLLECTION col;
    GB_VARIANT value;
    const char *key;
    CDCOPREF *ref;

    QMap<QString,DCOPRef> r;
    reply >> r;

    GB.Collection.New(&col, GB_COMP_BINARY);

    QMap<QString,DCOPRef>::Iterator it;

    for (it = r.begin(); it != r.end(); it++)
    {
      key = TO_UTF8(it.key());

      value.type = GB_T_VARIANT;
      value.value.type = GB_T_OBJECT;
      ref = make_dcopref(it.data());
      //GB.Ref(ref);
      value.value._object.value = ref;

      GB.Collection.Set(col, key, 0, &value);
    }

    GB.ReturnObject(col);
  }
  else if (type == QT_T_QMAP_QCSTRING_DCOPREF)
  {
    GB_COLLECTION col;
    GB_VARIANT value;
    const char *key;
    CDCOPREF *ref;

    QMap<QCString,DCOPRef> r;
    reply >> r;

    GB.Collection.New(&col, GB_COMP_BINARY);

    QMap<QCString,DCOPRef>::Iterator it;

    for (it = r.begin(); it != r.end(); it++)
    {
      key = (const char *)it.key();

      value.type = GB_T_VARIANT;
      value.value.type = GB_T_OBJECT;
      ref = make_dcopref(it.data());
      //GB.Ref(ref);
      value.value._object.value = ref;

      //qDebug("key: %s  ref: %p", key, ref);
      GB.Collection.Set(col, key, 0, &value);
    }

    GB.ReturnObject(col);
  }
  else if (type == QT_T_QPOINT)
  {
    GB_ARRAY array;
    QPoint r;
    reply >> r;

    GB.Array.New(&array, GB_T_INTEGER, 2);

    *((int *)GB.Array.Get(array, 0)) = r.x();
    *((int *)GB.Array.Get(array, 1)) = r.y();

    GB.ReturnObject(array);
  }
  else if (type == QT_T_QSIZE)
  {
    GB_ARRAY array;
    QSize r;
    reply >> r;

    GB.Array.New(&array, GB_T_INTEGER, 2);

    *((int *)GB.Array.Get(array, 0)) = r.width();
    *((int *)GB.Array.Get(array, 1)) = r.height();

    GB.ReturnObject(array);
  }
  else if (type == QT_T_QRECT)
  {
    GB_ARRAY array;
    QRect r;
    reply >> r;

    GB.Array.New(&array, GB_T_INTEGER, 4);

    *((int *)GB.Array.Get(array, 0)) = r.x();
    *((int *)GB.Array.Get(array, 1)) = r.y();
    *((int *)GB.Array.Get(array, 2)) = r.width();
    *((int *)GB.Array.Get(array, 3)) = r.height();

    GB.ReturnObject(array);
  }
  else
  {
    qWarning("Cannot unmarshall %s", (const char *)ctype);
    GB.ReturnNull();
  }

  return false;
}


BEGIN_METHOD(CAPPLICATION_unknown, GB_VALUE param[0])

  if (GB.IsProperty())
  {
    if (call_method(THIS, NULL, GB.GetUnknown(), NULL, 0, false))
      get_object(THIS, GB.GetUnknown());
  }
  else
  {
    call_method(THIS, THIS->object, GB.GetUnknown(), ARG(param[0]), GB.NParam(), true);
    GB.FreeString(&THIS->object);
  }

END_METHOD


#undef THIS
#define THIS ((CDCOPREF *)_object)

BEGIN_METHOD_VOID(CDCOPREF_free)

  delete THIS->ref;
  THIS->ref = 0;

  //qDebug("Delete DCOPREF %p", _object);

END_METHOD


BEGIN_METHOD(CDCOPREF_unknown, GB_VALUE param[0])

  CAPPLICATION *app;
  DCOPRef *ref = THIS->ref;

  //qDebug("Calling DCOPREF %p", _object);

  app = get_application(ref->app());

  call_method(app, ref->object(), GB.GetUnknown(), ARG(param[0]), GB.NParam(), true);

END_METHOD


BEGIN_PROPERTY(CDCOPREF_name)

  GB.ReturnNewZeroString(THIS->ref->object());

END_PROPERTY


BEGIN_PROPERTY(CDCOPREF_application)

  GB.ReturnObject(get_application(THIS->ref->app()));

END_PROPERTY


GB_DESC CKDEObjectDesc[] =
{
  GB_DECLARE(".KDEObject", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_unknown", "v", CAPPLICATION_unknown, "."),

  GB_END_DECLARE
};


GB_DESC CKDEDCOPRefDesc[] =
{
  GB_DECLARE("DCOPRef", sizeof(CDCOPREF)), GB_NOT_CREATABLE(),

  GB_METHOD("_free", NULL, CDCOPREF_free, NULL),
  GB_METHOD("_unknown", "v", CDCOPREF_unknown, "."),

  GB_PROPERTY_READ("Name", "s", CDCOPREF_name),
  GB_PROPERTY_READ("Application", "KDEApplication", CDCOPREF_application),

  GB_END_DECLARE
};


GB_DESC CKDEApplicationDesc[] =
{
  GB_DECLARE("KDEApplication", sizeof(CAPPLICATION)),
  GB_NOT_CREATABLE(),

  GB_STATIC_METHOD("_exit", NULL, CAPPLICATION_exit, NULL),

  GB_METHOD("_free", NULL, CAPPLICATION_free, NULL),

  GB_PROPERTY_READ("Name", "s", CAPPLICATION_name),

  GB_METHOD("_get", ".KDEObject", CAPPLICATION_get_object, "(Object)s"),
  GB_METHOD("_unknown", "v", CAPPLICATION_unknown, "."),

  GB_END_DECLARE
};


GB_DESC CApplicationDesc[] =
{
  GB_DECLARE("Application", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_get", "KDEApplication", CAPPLICATION_get, "(Name)s"),
  //GB_STATIC_METHOD("_next", "KDEApplication", CAPPLICATION_next, NULL),

  //GB_STATIC_PROPERTY("Title", "s", CDIALOG_title),

  GB_END_DECLARE
};

