/***************************************************************************

  CTextBox.cpp

  The TextBox class

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

#define __CTEXTBOX_CPP

#include <qapplication.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qcursor.h>
#include <qsizepolicy.h>

#include "gambas.h"

#include "CConst.h"
#include "CTextBox.h"

DECLARE_EVENT(EVENT_Change);
DECLARE_EVENT(EVENT_Activate);
DECLARE_EVENT(EVENT_Click);

#define MAX_LEN 32767


/***************************************************************************

  TextBox

***************************************************************************/

static bool get(void *_object, QLineEdit **wid, bool error = true)
{
  QComboBox *combo;

  if (TEXTBOX->isA("QLineEdit"))
  {
    *wid = TEXTBOX;
    return false;
  }

  combo = COMBOBOX;
  if (!combo->editable())
  {
    if (error)
      GB.Error("ComboBox is read-only");
    return true;
  }

  *wid = combo->lineEdit();
  return false;
}

#define GET_TEXT_BOX() \
  QLineEdit *textbox; \
  if (get(_object, &textbox)) \
    return;



BEGIN_METHOD(CTEXTBOX_new, GB_OBJECT parent)

  QLineEdit *wid = new QLineEdit(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(textChanged(const QString &)), &CTextBox::manager, SLOT(event_change()));
  QObject::connect(wid, SIGNAL(returnPressed()), &CTextBox::manager, SLOT(event_activate()));

  wid->setAlignment(Qt::AlignLeft);

  CWIDGET_new(wid, (void *)_object);

END_METHOD


BEGIN_METHOD_VOID(CTEXTBOX_clear)

  TEXTBOX->clear();

END_METHOD

BEGIN_METHOD(CTEXTBOX_insert, GB_STRING text)

  GET_TEXT_BOX();

  //textbox->insert(QString(GB.ToZeroString(ARG(text))));
  textbox->insert(QSTRING_ARG(text));

END_METHOD


BEGIN_PROPERTY(CTEXTBOX_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(TEXTBOX->text()));
  else
    TEXTBOX->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_length)

  GB.ReturnInteger(TEXTBOX->text().length());

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_alignment)

  if (READ_PROPERTY)
    GB.ReturnInteger(CCONST_alignment(TEXTBOX->alignment() + Qt::AlignVCenter, ALIGN_NORMAL, false));
  else
    TEXTBOX->setAlignment(CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true) & Qt::AlignHorizontal_Mask);

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_pos)

  GET_TEXT_BOX();

  if (READ_PROPERTY)
    GB.ReturnInteger(textbox->cursorPosition());
  else
    textbox->setCursorPosition(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_read_only)

  if (READ_PROPERTY)
    GB.ReturnBoolean(TEXTBOX->isReadOnly());
  else
    TEXTBOX->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_border)

  GET_TEXT_BOX();

  if (READ_PROPERTY)
    GB.ReturnBoolean(textbox->frame());
  else
    textbox->setFrame(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_password)

  GET_TEXT_BOX();

  if (READ_PROPERTY)
    GB.ReturnBoolean(textbox->echoMode() != QLineEdit::Normal);
  else
    textbox->setEchoMode(VPROP(GB_BOOLEAN) ? QLineEdit::Password : QLineEdit::Normal);

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_max_length)

  int max;

  GET_TEXT_BOX();

  if (READ_PROPERTY)
  {
    max = textbox->maxLength();
    GB.ReturnInteger(max >= MAX_LEN ? 0 : max);
  }
  else
  {
    max = VPROP(GB_INTEGER);
    if (max < 1 || max > MAX_LEN)
      max = MAX_LEN;

    textbox->setMaxLength(max);
  }

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_selection)

  RETURN_SELF();

END_PROPERTY


/***************************************************************************

  .TextBox.Selection

***************************************************************************/

BEGIN_PROPERTY(CTEXTBOX_sel_text)

  GET_TEXT_BOX();

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(textbox->markedText()));
  else
    textbox->insert(QSTRING_PROP());

END_PROPERTY


static void set_selection(QLineEdit *textbox, int start, int length)
{
  int len = (int)textbox->text().length();

  if (start < 0 || start >= len)
  {
    start = textbox->cursorPosition();
    length = 0;
  }

  textbox->setCursorPosition(start);

  if (length <= 0)
    textbox->deselect();
  else
  {
    if ((start + length) >= len)
      length = len - start;
    textbox->setSelection(start, length);
  }
}

static void get_selection(QLineEdit *textbox, int *start, int *length)
{
  *start = textbox->cursorPosition();
  if (!textbox->hasMarkedText())
    *length = 0;
  else
    *length = textbox->markedText().length();
}


BEGIN_PROPERTY(CTEXTBOX_sel_length)

  int start, length;

  GET_TEXT_BOX();

  get_selection(textbox, &start, &length);

  GB.ReturnInteger(length);

END_PROPERTY


BEGIN_PROPERTY(CTEXTBOX_sel_start)

  int start, length;

  GET_TEXT_BOX();

  get_selection(textbox, &start, &length);
  GB.ReturnInteger(start);

END_PROPERTY


BEGIN_METHOD_VOID(CTEXTBOX_sel_clear)

  GET_TEXT_BOX();

  textbox->deselect();

END_METHOD

BEGIN_METHOD_VOID(CTEXTBOX_selected)

  GET_TEXT_BOX();

  GB.ReturnBoolean(textbox->hasSelectedText());

END_METHOD


BEGIN_METHOD(CTEXTBOX_sel_select, GB_INTEGER start; GB_INTEGER length)

  GET_TEXT_BOX();

	if (MISSING(start) && MISSING(length))
		textbox->selectAll();
	else if (!MISSING(start) && !MISSING(length))
  	set_selection(textbox, VARG(start), VARG(length));

END_METHOD

BEGIN_METHOD_VOID(CTEXTBOX_sel_all)

  GET_TEXT_BOX();

  textbox->selectAll();

END_METHOD


/***************************************************************************

  ComboBox

***************************************************************************/

#undef THIS
#define THIS OBJECT(CCOMBOBOX)

static void combo_raise_click(void *_object)
{
	if (THIS->click)
		return;
	THIS->click = true;
	GB.Raise(THIS, EVENT_Click, 0);
	THIS->click = false;
}

static int combo_get_current_item(void *_object)
{
	return COMBOBOX->count() == 0 ? -1 : COMBOBOX->currentItem();
}

static void combo_set_current_item(void *_object, int item)
{
	if (item != combo_get_current_item(THIS))
	{
		if (item < COMBOBOX->count())
			COMBOBOX->setCurrentItem(item);
	}
	
  if (item >= 0 && !COMBOBOX->signalsBlocked())
		combo_raise_click(THIS);
}


static void combo_set_editable(void *_object, bool ed)
{
  QLineEdit *textbox;

  if (ed)
  {
    if (!COMBOBOX->editable())
    {
      //CWidget::removeFilter(COMBOBOX);
      COMBOBOX->setEditable(true);
      //CWidget::installFilter(COMBOBOX);
      QObject::connect(COMBOBOX->lineEdit(), SIGNAL(returnPressed()), &CTextBox::manager, SLOT(event_activate()));

      if (CWIDGET_test_flag(THIS, WF_DESIGN))
      {
        get(_object, &textbox);
        //textbox->removeEventFilter(COMBOBOX);
        COMBOBOX->setFocusProxy(0);
      }
    }
  }
  else
  {
    COMBOBOX->setEditable(false);
  }

  if (CWIDGET_test_flag(THIS, WF_DESIGN))
    COMBOBOX->setFocusPolicy(QWidget::NoFocus);

  COMBOBOX->calcMinimumHeight();
}

static int combo_find_item(void *_object, const QString& s)
{
  for (int i = 0; i < (int)COMBOBOX->count(); i++)
  {
    if (COMBOBOX->text(i) == s)
      return i;
  }

  return (-1);
}

static void combo_set_text(CCOMBOBOX *_object, QString &text)
{
	int pos;
	
	if (COMBOBOX->editable())
		COMBOBOX->lineEdit()->setText(text);

	pos = combo_find_item(THIS, text);
	//COMBOBOX->blockSignals(true);
	combo_set_current_item(_object, pos);
}

static void combo_get_list(void *_object, GB_ARRAY array)
{
	int i;
	char *str;
	
	for (i = 0; i < COMBOBOX->count(); i++)
	{
		GB.NewString(&str, TO_UTF8(COMBOBOX->text(i)), 0);
		*((char **)GB.Array.Get(array, i)) = str;
	}
}

static void combo_set_list(void *_object, GB_ARRAY array)
{
	int i;
	QString text = COMBOBOX->currentText();
	
  COMBOBOX->blockSignals(true);
  COMBOBOX->clear();

	if (array)
	{
		for (i = 0; i < GB.Array.Count(array); i++)
		{
			COMBOBOX->insertItem(TO_QSTRING(*((char **)GB.Array.Get(array, i))));
		}
	}

	if (THIS->sorted)
		COMBOBOX->listBox()->sort();

  COMBOBOX->blockSignals(false);
	
	combo_set_text(THIS, text);
	
	//if (COMBOBOX->count())
	//	GB.Raise(THIS, EVENT_Click, 0);
}


BEGIN_METHOD(CCOMBOBOX_new, GB_OBJECT parent)

  MyComboBox *wid = new MyComboBox(QCONTAINER(VARG(parent)));

  QObject::connect(wid, SIGNAL(textChanged(const QString &)), &CTextBox::manager, SLOT(event_change()));
  QObject::connect(wid, SIGNAL(activated(int)), &CTextBox::manager, SLOT(event_click()));

  //QObject::connect(wid, SIGNAL(highlighted(int)), &CTextBox::manager, SLOT(event_click()));

  wid->setInsertionPolicy(QComboBox::NoInsertion);

  CWIDGET_new(wid, (void *)_object);

  combo_set_editable(_object, true);

END_METHOD


BEGIN_METHOD_VOID(CCOMBOBOX_clear)

  COMBOBOX->clear();
  COMBOBOX->clearEdit();

END_METHOD


BEGIN_METHOD_VOID(CCOMBOBOX_popup)

  COMBOBOX->popup();

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(COMBOBOX->currentText()));
  else
  {
    QString text = QSTRING_PROP();
		combo_set_text(THIS, text);
  }

END_PROPERTY


BEGIN_PROPERTY(CCOMBOBOX_length)

  GB.ReturnInteger(COMBOBOX->currentText().length());

END_PROPERTY



BEGIN_PROPERTY(CCOMBOBOX_read_only)

  if (READ_PROPERTY)
    GB.ReturnBoolean(!COMBOBOX->editable());
  else
    combo_set_editable(_object, !VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(CCOMBOBOX_get, GB_INTEGER index)

  int index = VARG(index);

  if (index < 0 || index >= (int)COMBOBOX->count())
  {
    GB.Error("Bad index");
    return;
  }

  THIS->index = index;

  RETURN_SELF();

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_item_text)

  if (READ_PROPERTY)
    GB.ReturnNewZeroString(TO_UTF8(COMBOBOX->text(THIS->index)));
  else
  {
    COMBOBOX->blockSignals(true);
    COMBOBOX->changeItem(QSTRING_PROP(), THIS->index);
    COMBOBOX->blockSignals(false);
  }

END_PROPERTY


BEGIN_METHOD(CCOMBOBOX_add, GB_STRING item; GB_INTEGER pos)

	int index;
	int pos;

  COMBOBOX->blockSignals(true);

	index = combo_get_current_item(THIS);
	
  pos = VARGOPT(pos, -1);
  if (pos < 0 || pos >= COMBOBOX->count())
  	pos = -1;
  COMBOBOX->insertItem(QSTRING_ARG(item), pos);
  if (THIS->sorted)
    COMBOBOX->listBox()->sort();
	
	if (index >= 0)
		combo_set_current_item(THIS, index);
	else
		combo_set_current_item(THIS, 0);
	
  COMBOBOX->blockSignals(false);
	
END_METHOD


BEGIN_METHOD(CCOMBOBOX_remove, GB_INTEGER pos)

  COMBOBOX->blockSignals(true);
  COMBOBOX->removeItem(VARG(pos));
  COMBOBOX->blockSignals(false);

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_sorted)

  if (READ_PROPERTY)
    GB.ReturnBoolean(THIS->sorted);
  else
  {
    THIS->sorted = VPROP(GB_BOOLEAN);
    if (THIS->sorted)
      COMBOBOX->listBox()->sort();
  }

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_count)

  GB.ReturnInteger(COMBOBOX->count());

END_PROPERTY


BEGIN_PROPERTY(CCOMBOBOX_index)

  if (READ_PROPERTY)
    GB.ReturnInteger(combo_get_current_item(THIS));
  else
    combo_set_current_item(THIS, VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(CCOMBOBOX_current)

  THIS->index = combo_get_current_item(THIS);

  if (THIS->index < 0)
    GB.ReturnNull();
  else
    RETURN_SELF();

END_PROPERTY


/*
BEGIN_PROPERTY(CCOMBOBOX_mouse)

  if (READ_PROPERTY)
    GB.ReturnInteger(COMBOBOX->cursor().shape());
  else
  {
    if (COMBOBOX->editable())
      COMBOBOX->lineEdit()->setCursor(PROPERTY(int));

    COMBOBOX->setCursor(PROPERTY(int));
  }

END_METHOD
*/

BEGIN_METHOD(CCOMBOBOX_find, GB_STRING item)

  GB.ReturnInteger(combo_find_item(THIS, QSTRING_ARG(item)));

END_METHOD


BEGIN_PROPERTY(CCOMBOBOX_list)

 	GB_ARRAY array;
 	
  if (READ_PROPERTY)
  {
  	GB.Array.New(&array, GB_T_STRING, COMBOBOX->count());
    combo_get_list(THIS, array);
    GB.ReturnObject(array);
	}
  else
  {
    combo_set_list(THIS, (GB_ARRAY)VPROP(GB_OBJECT));
  }

END_PROPERTY



/***************************************************************************

  class MyComboBox

***************************************************************************/

MyComboBox::MyComboBox(QWidget *parent) :
  QComboBox(false, parent)
{
  calcMinimumHeight();
}


void MyComboBox::fontChange(const QFont &font)
{
  QWidget::fontChange(font);
  calcMinimumHeight();
}

void MyComboBox::calcMinimumHeight()
{
  QFontMetrics fm = fontMetrics();

  if (editable())
    setMinimumHeight(fm.lineSpacing() + height() - lineEdit()->height());
  else
    setMinimumHeight(fm.lineSpacing() + 2);
}


/***************************************************************************

  class CTextBox

***************************************************************************/

CTextBox CTextBox::manager;

void CTextBox::event_change(void)
{
  RAISE_EVENT(EVENT_Change);
}


void CTextBox::event_activate(void)
{
  RAISE_EVENT(EVENT_Activate);
}


void CTextBox::event_click()
{
	GET_SENDER(_object);
	combo_raise_click(THIS);
}

/***************************************************************************

  Descriptions

***************************************************************************/

GB_DESC CTextBoxSelectionDesc[] =
{
  GB_DECLARE(".TextBoxSelection", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTEXTBOX_sel_text),
  GB_PROPERTY_READ("Length", "i", CTEXTBOX_sel_length),
  GB_PROPERTY_READ("Start", "i", CTEXTBOX_sel_start),
  GB_PROPERTY_READ("Pos", "i", CTEXTBOX_sel_start),

  GB_METHOD("Hide", NULL, CTEXTBOX_sel_clear, NULL),

  GB_END_DECLARE
};


GB_DESC CTextBoxDesc[] =
{
  GB_DECLARE("TextBox", sizeof(CTEXTBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTEXTBOX_new, "(Parent)Container;"),

  GB_PROPERTY("Text", "s", CTEXTBOX_text),
  GB_PROPERTY("Alignment", "i", CTEXTBOX_alignment),
  GB_PROPERTY_READ("Length", "i", CTEXTBOX_length),
  GB_PROPERTY("Pos", "i", CTEXTBOX_pos),
  GB_PROPERTY("ReadOnly", "b", CTEXTBOX_read_only),
  GB_PROPERTY("Border", "b", CTEXTBOX_border),
  GB_PROPERTY("Password", "b", CTEXTBOX_password),
  GB_PROPERTY("MaxLength", "i", CTEXTBOX_max_length),

  GB_PROPERTY_SELF("Selection", ".TextBoxSelection"),
  GB_METHOD("Select", NULL, CTEXTBOX_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", NULL, CTEXTBOX_sel_all, NULL),
  GB_METHOD("Unselect", NULL, CTEXTBOX_sel_clear, NULL),
  GB_PROPERTY_READ("Selected", "b", CTEXTBOX_selected),

  GB_METHOD("Clear", NULL, CTEXTBOX_clear, NULL),
  GB_METHOD("Insert", NULL, CTEXTBOX_insert, "(Text)s"),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),

	TEXTBOX_DESCRIPTION,

  GB_END_DECLARE
};


GB_DESC CComboBoxItemDesc[] =
{
  GB_DECLARE(".ComboBoxItem", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CCOMBOBOX_item_text),

  GB_END_DECLARE
};


GB_DESC CComboBoxDesc[] =
{
  GB_DECLARE("ComboBox", sizeof(CCOMBOBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CCOMBOBOX_new, "(Parent)Container;"),
  GB_METHOD("_get", ".ComboBoxItem", CCOMBOBOX_get, "(Index)i"),

  GB_PROPERTY("Text", "s", CCOMBOBOX_text),
  GB_PROPERTY_READ("Length", "i", CCOMBOBOX_length),
  GB_PROPERTY("Pos", "i", CTEXTBOX_pos),
  GB_PROPERTY("ReadOnly", "b", CCOMBOBOX_read_only),
  GB_PROPERTY("Password", "b", CTEXTBOX_password),
  GB_PROPERTY("MaxLength", "i", CTEXTBOX_max_length),

  GB_PROPERTY_SELF("Selection", ".TextBoxSelection"),
  GB_METHOD("Select", NULL, CTEXTBOX_sel_select, "[(Start)i(Length)i]"),
  GB_METHOD("SelectAll", NULL, CTEXTBOX_sel_all, NULL),
  GB_METHOD("Unselect", NULL, CTEXTBOX_sel_clear, NULL),
  GB_PROPERTY_READ("Selected", "b", CTEXTBOX_selected),

  GB_METHOD("Popup", NULL, CCOMBOBOX_popup, NULL),
  GB_METHOD("Clear", NULL, CCOMBOBOX_clear, NULL),
  GB_METHOD("Insert", NULL, CTEXTBOX_insert, "(Text)s"),

  GB_METHOD("Add", NULL, CCOMBOBOX_add, "(Item)s[(Index)i]"),
  GB_METHOD("Remove", NULL, CCOMBOBOX_remove, "(Index)i"),

  GB_METHOD("Find", "i", CCOMBOBOX_find, "(Item)s"),

  GB_PROPERTY("Sorted", "b", CCOMBOBOX_sorted),

  GB_PROPERTY("List", "String[]", CCOMBOBOX_list),
  //GB_PROPERTY("Contents", "s", CCOMBOBOX_list),

  GB_PROPERTY_READ("Count", "i", CCOMBOBOX_count),
  GB_PROPERTY_READ("Current", ".ComboBoxItem", CCOMBOBOX_current),
  GB_PROPERTY("Index", "i", CCOMBOBOX_index),

  GB_EVENT("Change", NULL, NULL, &EVENT_Change),
  GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

	COMBOBOX_DESCRIPTION,

  GB_END_DECLARE
};


