/***************************************************************************

  cwebelement.cpp

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

#define __CWEBELEMENT_CPP

#include "cwebframe.h"
#include "cwebelement.h"

typedef
	struct {
		GB_BASE ob;
		int x, y, w, h;
	}
	CRECT;

CWEBELEMENT *CWEBELEMENT_create(const QWebElement &elt)
{
	void *_object;
	
	if (elt.isNull())
		return NULL;
	
	_object = GB.New(GB.FindClass("WebElement"), 0, 0);
	ELT = new QWebElement(elt);
	//qDebug("create WebElement %p / %p (%ld)", THIS, ELT, THIS->ob.ref);
	return THIS;
}

static int check_element(void *_object)
{
	return !ELT || ELT->isNull();
}

//---------------------------------------------------------------------------

/*BEGIN_METHOD_VOID(WebElement_new)

	ELT = new QWebElement;

END_METHOD*/

BEGIN_METHOD_VOID(WebElement_free)

	//qDebug("WebElement_free: %p / %p", THIS, ELT);
	delete ELT;

END_METHOD

BEGIN_PROPERTY(WebElement_HTML)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(ELT->toOuterXml()));
	else
		ELT->setOuterXml(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(WebElement_InnerHTML)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(ELT->toInnerXml()));
	else
		ELT->setInnerXml(QSTRING_PROP());

END_PROPERTY

BEGIN_PROPERTY(WebElement_Frame)

	GB.ReturnObject(CWEBFRAME_get(ELT->webFrame()));

END_PROPERTY

BEGIN_METHOD(WebElement_Eval, GB_STRING javascript)

	QVariant result = ELT->evaluateJavaScript(QSTRING_ARG(javascript));
	MAIN_return_qvariant(result);

END_METHOD

BEGIN_PROPERTY(WebElement_Child)

	GB.ReturnObject(CWEBELEMENT_create(ELT->firstChild()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_Last)

	GB.ReturnObject(CWEBELEMENT_create(ELT->lastChild()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_Next)

	GB.ReturnObject(CWEBELEMENT_create(ELT->nextSibling()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_Previous)

	GB.ReturnObject(CWEBELEMENT_create(ELT->previousSibling()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_Parent)

	GB.ReturnObject(CWEBELEMENT_create(ELT->parent()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_Document)

	GB.ReturnObject(CWEBELEMENT_create(ELT->document()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_HasFocus)

	GB.ReturnBoolean(ELT->hasFocus());

END_PROPERTY

BEGIN_METHOD_VOID(WebElement_SetFocus)

	ELT->setFocus();

END_METHOD

BEGIN_METHOD(WebElement_FindFirst, GB_STRING selector)

	GB.ReturnObject(CWEBELEMENT_create(ELT->findFirst(QSTRING_ARG(selector))));

END_METHOD

BEGIN_METHOD(WebElement_FindAll, GB_STRING selector)

	GB_ARRAY array;
	int i;
	QWebElementCollection result = ELT->findAll(QSTRING_ARG(selector));
	CWEBELEMENT *elt;
	
	GB.Array.New(&array, GB.FindClass("WebElement"), result.count());
	
	for (i = 0; i < result.count(); i++)
	{
		elt = CWEBELEMENT_create(result.at(i));
		GB.Ref(elt);
		*((CWEBELEMENT **)GB.Array.Get(array, i)) = elt;
	}
	
	GB.ReturnObject(array);

END_METHOD

BEGIN_PROPERTY(WebElement_Tag)

	GB.ReturnNewZeroString(TO_UTF8(ELT->tagName()));

END_PROPERTY

BEGIN_PROPERTY(WebElement_Classes)

	GB_ARRAY array;
	int i;
	QStringList classes = ELT->classes();
	
	GB.Array.New(&array, GB_T_STRING, classes.count());
	for (i = 0; i < classes.count(); i++)
		*((const char **)GB.Array.Get(array, i)) = TO_UTF8(classes.at(i));
	
	GB.ReturnObject(array);

END_PROPERTY

BEGIN_METHOD(WebElement_AddClass, GB_STRING klass)

	ELT->addClass(QSTRING_ARG(klass));

END_METHOD

BEGIN_METHOD(WebElement_RemoveClass, GB_STRING klass)

	ELT->removeClass(QSTRING_ARG(klass));

END_METHOD

BEGIN_METHOD(WebElement_get, GB_STRING attr)

	GB.ReturnNewZeroString(TO_UTF8(ELT->attribute(QSTRING_ARG(attr))));

END_METHOD

BEGIN_METHOD(WebElement_put, GB_STRING value; GB_STRING attr)

	ELT->setAttribute(QSTRING_ARG(attr), QSTRING_ARG(value));

END_METHOD

BEGIN_METHOD(WebElement_HasAttribute, GB_STRING attr)

	GB.ReturnBoolean(ELT->hasAttribute(QSTRING_ARG(attr)));

END_METHOD

BEGIN_METHOD(WebElement_RemoveAttribute, GB_STRING attr)

	ELT->removeAttribute(QSTRING_ARG(attr));

END_METHOD

BEGIN_PROPERTY(WebElement_Geometry)

	QRect r = ELT->geometry();
	
	GB.Push(4, GB_T_INTEGER, r.x(), GB_T_INTEGER, r.y(), GB_T_INTEGER, r.width(), GB_T_INTEGER, r.height());
	GB.ReturnObject(GB.New(GB.FindClass("Rect"), 0, (void *)4));

END_PROPERTY

BEGIN_METHOD_VOID(WebElement_Delete)

	ELT->removeFromDocument();

END_METHOD

BEGIN_METHOD(WebElement_Add, GB_STRING markup)

	ELT->appendInside(QSTRING_ARG(markup));
	
END_METHOD

BEGIN_METHOD(WebElement_AddFirst, GB_STRING markup)

	ELT->prependInside(QSTRING_ARG(markup));
	
END_METHOD

BEGIN_METHOD(WebElement_AddBefore, GB_STRING markup)

	ELT->prependOutside(QSTRING_ARG(markup));
	
END_METHOD

BEGIN_METHOD(WebElement_AddAfter, GB_STRING markup)

	ELT->appendOutside(QSTRING_ARG(markup));
	
END_METHOD

BEGIN_METHOD(WebElement_Enclose, GB_STRING markup)

	ELT->encloseContentsWith(QSTRING_ARG(markup));
	
END_METHOD

BEGIN_METHOD(WebElement_Replace, GB_STRING markup)

	ELT->replace(QSTRING_ARG(markup));
	
END_METHOD

BEGIN_METHOD_VOID(WebElement_Clear)

	ELT->removeAllChildren();

END_METHOD

BEGIN_METHOD(WebElement_Paint, GB_OBJECT clip)

	QPainter *painter = QT.GetCurrentPainter();
	
	if (!painter)
		return;
	
	#if QT_VERSION >= 0x040800
	
	if (MISSING(clip))
		ELT->render(painter);
	else
	{
		CRECT *rect = (CRECT *)VARG(clip);
		QRect clip(rect->x, rect->y, rect->w, rect->h);
		ELT->render(painter, clip);
	}
	
	#else
	
		ELT->render(painter);
		
	#endif

END_METHOD


//---------------------------------------------------------------------------

BEGIN_METHOD(WebElementStyle_get, GB_STRING property)

	GB.ReturnNewZeroString(TO_UTF8(ELT->styleProperty(QSTRING_ARG(property), QWebElement::InlineStyle)));

END_METHOD

BEGIN_METHOD(WebElementStyle_put, GB_STRING value; GB_STRING property)

	ELT->setStyleProperty(QSTRING_ARG(property), QSTRING_ARG(value));

END_METHOD

//---------------------------------------------------------------------------

GB_DESC WebElementStyleDesc[] =
{
  GB_DECLARE_VIRTUAL(".WebElement.Style"),
	GB_HOOK_CHECK(check_element),
	
	GB_METHOD("_get", "s", WebElementStyle_get, "(Property)s"),
	GB_METHOD("_put", "s", WebElementStyle_put, "(Value)s(Property)s"),
	
	GB_END_DECLARE
};

GB_DESC WebElementDesc[] =
{
  GB_DECLARE("WebElement", sizeof(CWEBELEMENT)),
	GB_HOOK_CHECK(check_element), GB_NOT_CREATABLE(),
	
	//GB_METHOD("_new", NULL, WebElement_new, NULL),
	GB_METHOD("_free", NULL, WebElement_free, NULL),
	
	GB_PROPERTY("HTML", "s", WebElement_HTML),
	GB_PROPERTY("InnerHTML", "s", WebElement_InnerHTML),
	GB_PROPERTY_READ("Frame", "WebFrame", WebElement_Frame),

	GB_PROPERTY_READ("Tag", "s", WebElement_Tag),
	
	GB_PROPERTY_READ("Classes", "String[]", WebElement_Classes),
	GB_METHOD("AddClass", NULL, WebElement_AddClass, "(Class)s"),
	GB_METHOD("RemoveClass", NULL, WebElement_RemoveClass, "(Class)s"),

	GB_PROPERTY_READ("Child", "WebElement", WebElement_Child),
	GB_PROPERTY_READ("First", "WebElement", WebElement_Child),
	GB_PROPERTY_READ("Last", "WebElement", WebElement_Last),
	GB_PROPERTY_READ("Next", "WebElement", WebElement_Next),
	GB_PROPERTY_READ("Previous", "WebElement", WebElement_Previous),
	GB_PROPERTY_READ("Parent", "WebElement", WebElement_Parent),
	GB_PROPERTY_READ("Document", "WebElement", WebElement_Document),

	GB_PROPERTY_READ("HasFocus", "b", WebElement_HasFocus),
	GB_METHOD("SetFocus", NULL, WebElement_SetFocus, NULL),
	
	GB_METHOD("Eval", "v", WebElement_Eval, "(JavaScript)s"),
	
	GB_METHOD("FindFirst", "WebElement", WebElement_FindFirst, "(Selector)s"),
	GB_METHOD("FindAll", "WebElement[]", WebElement_FindAll, "(Selector)s"),
	
	GB_METHOD("_get", "s", WebElement_get, "(Attribute)s"),
	GB_METHOD("_put", "s", WebElement_put, "(Value)s(Attribute)s"),
	GB_METHOD("HasAttribute", "b", WebElement_HasAttribute, "(Attribute)s"),
	GB_METHOD("RemoveAttribute", NULL, WebElement_RemoveAttribute, "(Attribute)s"),
	
	GB_PROPERTY_READ("Geometry", "Rect", WebElement_Geometry),
	
	GB_METHOD("Delete", NULL, WebElement_Delete, NULL),
	GB_METHOD("Clear", NULL, WebElement_Clear, NULL),

	GB_METHOD("Add", NULL, WebElement_Add, "(Markup)s"),
	GB_METHOD("AddFirst", NULL, WebElement_AddFirst, "(Markup)s"),
	GB_METHOD("AddBefore", NULL, WebElement_AddBefore, "(Markup)s"),
	GB_METHOD("AddAfter", NULL, WebElement_AddAfter, "(Markup)s"),
	GB_METHOD("Enclose", NULL, WebElement_Enclose, "(Markup)s"),
	GB_METHOD("Replace", NULL, WebElement_Replace, "(Markup)s"),
	
	GB_METHOD("Paint", NULL, WebElement_Paint, "[(Clip)Rect;]"),
	
	GB_PROPERTY_SELF("Style", ".WebElement.Style"),
	
	GB_END_DECLARE
};


