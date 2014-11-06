/***************************************************************************

  CTextBox.cpp

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

#define __CTEXTBOX_CPP

#include <qapplication.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qsizepolicy.h>
#include <QLineEdit>
#include <QListView>

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

	if (qobject_cast<QLineEdit *>(TEXTBOX))
	{
		*wid = TEXTBOX;
		return false;
	}

	combo = COMBOBOX;
	if (!combo->isEditable())
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



BEGIN_METHOD(TextBox_new, GB_OBJECT parent)

	QLineEdit *wid = new QLineEdit(QCONTAINER(VARG(parent)));

	QObject::connect(wid, SIGNAL(textChanged(const QString &)), &CTextBox::manager, SLOT(onChange()));
	QObject::connect(wid, SIGNAL(returnPressed()), &CTextBox::manager, SLOT(onActivate()));
	QObject::connect(wid, SIGNAL(selectionChanged()), &CTextBox::manager, SLOT(onSelectionChanged()));

	wid->setAlignment(Qt::AlignLeft);

	CWIDGET_new(wid, (void *)_object);
	
END_METHOD


BEGIN_METHOD_VOID(TextBox_Clear)

	TEXTBOX->clear();

END_METHOD

BEGIN_METHOD(TextBox_Insert, GB_STRING text)

	GET_TEXT_BOX();

	//textbox->insert(QString(GB.ToZeroString(ARG(text))));
	textbox->insert(QSTRING_ARG(text));

END_METHOD


BEGIN_PROPERTY(TextBox_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(TEXTBOX->text()));
	else
		TEXTBOX->setText(QSTRING_PROP());

END_PROPERTY


BEGIN_PROPERTY(TextBox_Length)

	GB.ReturnInteger(TEXTBOX->text().length());

END_PROPERTY


BEGIN_PROPERTY(TextBox_Alignment)

	if (READ_PROPERTY)
		GB.ReturnInteger(CCONST_alignment(TEXTBOX->alignment() + Qt::AlignVCenter, ALIGN_NORMAL, false));
	else
		TEXTBOX->setAlignment((Qt::Alignment)CCONST_alignment(VPROP(GB_INTEGER), ALIGN_NORMAL, true) & Qt::AlignHorizontal_Mask);

END_PROPERTY


BEGIN_PROPERTY(TextBox_Pos)

	GET_TEXT_BOX();

	if (READ_PROPERTY)
		GB.ReturnInteger(textbox->cursorPosition());
	else
		textbox->setCursorPosition(VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(TextBox_ReadOnly)

	if (READ_PROPERTY)
		GB.ReturnBoolean(TEXTBOX->isReadOnly());
	else
		TEXTBOX->setReadOnly(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Border)

	GET_TEXT_BOX();

	if (READ_PROPERTY)
		GB.ReturnBoolean(textbox->hasFrame());
	else
		textbox->setFrame(VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_PROPERTY(TextBox_Password)

	GET_TEXT_BOX();

	if (READ_PROPERTY)
		GB.ReturnBoolean(textbox->echoMode() != QLineEdit::Normal);
	else
		textbox->setEchoMode(VPROP(GB_BOOLEAN) ? QLineEdit::Password : QLineEdit::Normal);

END_PROPERTY


BEGIN_PROPERTY(TextBox_MaxLength)

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


/***************************************************************************

	.TextBox.Selection

***************************************************************************/

BEGIN_PROPERTY(TextBox_Selection_Text)

	GET_TEXT_BOX();

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(textbox->selectedText()));
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
	*start = textbox->selectionStart();
	if (*start < 0)
		*start = textbox->cursorPosition();
	if (!textbox->hasSelectedText())
		*length = 0;
	else
		*length = textbox->selectedText().length();
}


BEGIN_PROPERTY(TextBox_Selection_Length)

	int start, length;

	GET_TEXT_BOX();

	get_selection(textbox, &start, &length);

	GB.ReturnInteger(length);

END_PROPERTY


BEGIN_PROPERTY(TextBox_Selection_Start)

	int start, length;

	GET_TEXT_BOX();

	get_selection(textbox, &start, &length);
	GB.ReturnInteger(start);

END_PROPERTY


BEGIN_METHOD_VOID(TextBox_Unselect)

	GET_TEXT_BOX();

	textbox->deselect();

END_METHOD

BEGIN_METHOD_VOID(ComboBox_Selected)

	GET_TEXT_BOX();

	GB.ReturnBoolean(textbox->hasSelectedText());

END_METHOD


BEGIN_METHOD(TextBox_Select, GB_INTEGER start; GB_INTEGER length)

	GET_TEXT_BOX();

	if (MISSING(start) && MISSING(length))
		textbox->selectAll();
	else if (!MISSING(start) && !MISSING(length))
		set_selection(textbox, VARG(start), VARG(length));

END_METHOD

BEGIN_METHOD_VOID(TextBox_SelectAll)

	GET_TEXT_BOX();

	textbox->selectAll();

END_METHOD


/***************************************************************************

	ComboBox

***************************************************************************/

#undef THIS
#define THIS OBJECT(CCOMBOBOX)

static void raise_click_event(void *_object)
{
	if (THIS->click)
		return;
	THIS->click = true;
	GB.Raise(THIS, EVENT_Click, 0);
	THIS->click = false;
	
}

static int combo_get_current_item(void *_object)
{
	COMBOBOX->sort();
	return COMBOBOX->count() == 0 ? -1 : COMBOBOX->currentItem();
}

static void combo_set_current_item(void *_object, int item)
{
	COMBOBOX->sort();
	
	if (item != combo_get_current_item(THIS))
	{
		if (item < COMBOBOX->count())
			COMBOBOX->setCurrentItem(item);
	}
	
	if (item >= 0 && !COMBOBOX->signalsBlocked())
		raise_click_event(THIS);
}

static int combo_find_item(void *_object, const QString& s)
{
	COMBOBOX->sort();
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

	pos = combo_find_item(THIS, text);
	if (!COMBOBOX->isEditable() || pos >= 0)
		combo_set_current_item(_object, pos);
	if (COMBOBOX->isEditable())
		COMBOBOX->lineEdit()->setText(text);
}

static void combo_set_editable(void *_object, bool ed)
{
	QLineEdit *textbox;
	QString text;
	bool hasFocus = COMBOBOX->hasFocus();
	
	if (ed == COMBOBOX->isEditable())
		return;

	COMBOBOX->blockSignals(true);
	text = COMBOBOX->currentText();

	if (ed)
	{
		//CWidget::removeFilter(COMBOBOX);
		COMBOBOX->setEditable(true);
		COMBOBOX->setCompleter(0);
		//CWidget::installFilter(COMBOBOX);
		QObject::connect(COMBOBOX->lineEdit(), SIGNAL(returnPressed()), &CTextBox::manager, SLOT(onActivate()));
		QObject::connect(COMBOBOX->lineEdit(), SIGNAL(selectionChanged()), &CTextBox::manager, SLOT(onSelectionChanged()));

		if (CWIDGET_test_flag(THIS, WF_DESIGN))
		{
			get(_object, &textbox);
			//textbox->removeEventFilter(COMBOBOX);
			COMBOBOX->setFocusProxy(0);
		}
	}
	else
	{
		get(THIS, &textbox);
		textbox->setFocusProxy(0);
		COMBOBOX->setEditable(false);
		COMBOBOX->update();
	}
	
	combo_set_text(THIS, text);
	CWIDGET_reset_color((CWIDGET *)THIS);

	if (hasFocus)
		COMBOBOX->setFocus();
	
	if (CWIDGET_test_flag(THIS, WF_DESIGN))
		COMBOBOX->setFocusPolicy(Qt::NoFocus);

	COMBOBOX->blockSignals(false);
}

static void combo_get_list(void *_object, GB_ARRAY array)
{
	int i;
	
	COMBOBOX->sort();
	for (i = 0; i < COMBOBOX->count(); i++)
	{
		*((char **)GB.Array.Get(array, i)) = GB.NewZeroString(TO_UTF8(COMBOBOX->text(i)));
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

	COMBOBOX->setDirty();
	combo_set_text(THIS, text);
	
	if (!COMBOBOX->isEditable())
	{
		if (combo_get_current_item(THIS) < 0)
			combo_set_current_item(THIS, 0);
	}
	
	COMBOBOX->blockSignals(false);
}



BEGIN_METHOD(ComboBox_new, GB_OBJECT parent)

	MyComboBox *wid = new MyComboBox(QCONTAINER(VARG(parent)));

	QObject::connect(wid, SIGNAL(editTextChanged(const QString &)), &CTextBox::manager, SLOT(onChange()));
	QObject::connect(wid, SIGNAL(activated(int)), &CTextBox::manager, SLOT(onClick()));

	CWIDGET_new(wid, (void *)_object);

	combo_set_editable(_object, true);

END_METHOD


BEGIN_METHOD_VOID(ComboBox_Clear)

	COMBOBOX->clear();
	COMBOBOX->clearEditText();

END_METHOD


BEGIN_METHOD_VOID(ComboBox_Popup)

	COMBOBOX->showPopup();

END_METHOD


BEGIN_PROPERTY(ComboBox_Text)

	if (READ_PROPERTY)
		GB.ReturnNewZeroString(TO_UTF8(COMBOBOX->currentText()));
	else
	{
		QString text = QSTRING_PROP();
		combo_set_text(THIS, text);
	}

END_PROPERTY


BEGIN_PROPERTY(ComboBox_Length)

	GB.ReturnInteger(COMBOBOX->currentText().length());

END_PROPERTY



BEGIN_PROPERTY(ComboBox_ReadOnly)

	if (READ_PROPERTY)
		GB.ReturnBoolean(!COMBOBOX->isEditable());
	else
		combo_set_editable(_object, !VPROP(GB_BOOLEAN));

END_PROPERTY


BEGIN_METHOD(ComboBox_get, GB_INTEGER index)

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
		GB.ReturnNewZeroString(TO_UTF8(COMBOBOX->itemText(THIS->index)));
	else
	{
		COMBOBOX->blockSignals(true);
		COMBOBOX->setItemText(THIS->index, QSTRING_PROP());
		COMBOBOX->setDirty();
		COMBOBOX->blockSignals(false);
	}

END_PROPERTY


BEGIN_METHOD(ComboBox_Add, GB_STRING item; GB_INTEGER pos)

	int index;
	int pos = VARGOPT(pos, -1);

	COMBOBOX->blockSignals(true);
	index = combo_get_current_item(THIS);
	
	if (pos < 0 || pos >= COMBOBOX->count())
		pos = -1;
	
	if (pos < 0)
		COMBOBOX->addItem(QSTRING_ARG(item));
	else
		COMBOBOX->insertItem(pos, QSTRING_ARG(item));

	COMBOBOX->setDirty();

	if (index >= 0)
		combo_set_current_item(THIS, index);
	else
		combo_set_current_item(THIS, 0);
	
	COMBOBOX->blockSignals(false);
	
END_METHOD


BEGIN_METHOD(ComboBox_Remove, GB_INTEGER pos)

	COMBOBOX->blockSignals(true);
	COMBOBOX->removeItem(VARG(pos));
	COMBOBOX->setDirty();
	COMBOBOX->blockSignals(false);

END_METHOD


BEGIN_PROPERTY(ComboBox_Sorted)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COMBOBOX->isSortingEnabled());
	else
		COMBOBOX->setSortingEnabled(VPROP(GB_BOOLEAN));

END_METHOD


BEGIN_PROPERTY(ComboBox_Count)

	GB.ReturnInteger(COMBOBOX->count());

END_PROPERTY


BEGIN_PROPERTY(ComboBox_Index)

	if (READ_PROPERTY)
		GB.ReturnInteger(combo_get_current_item(THIS));
	else
		combo_set_current_item(THIS, VPROP(GB_INTEGER));

END_PROPERTY


BEGIN_PROPERTY(ComboBox_Current)

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

BEGIN_METHOD(ComboBox_Find, GB_STRING item)

	GB.ReturnInteger(combo_find_item(THIS, QSTRING_ARG(item)));

END_METHOD


BEGIN_PROPERTY(ComboBox_List)

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


BEGIN_PROPERTY(ComboBox_Border)

	if (READ_PROPERTY)
		GB.ReturnBoolean(COMBOBOX->hasFrame());
	else
		COMBOBOX->setFrame(VPROP(GB_BOOLEAN));

END_PROPERTY




/***************************************************************************

	class MyComboBox

***************************************************************************/

MyComboBox::MyComboBox(QWidget *parent) :
	QComboBox(parent)
{
	_sorted = _dirty = false;
	setCompleter(0);
	setInsertPolicy(NoInsert);
	calcMinimumHeight();
}


void MyComboBox::changeEvent(QEvent *e)
{
	QComboBox::changeEvent(e);
	if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange)
		calcMinimumHeight();
}

void MyComboBox::calcMinimumHeight()
{
	QFontMetrics fm = fontMetrics();

	if (isEditable())
		setMinimumHeight(fm.lineSpacing() + height() - lineEdit()->height());
	else
		setMinimumHeight(fm.lineSpacing() + 2);
}

void MyComboBox::sort()
{
	if (_sorted && _dirty)
	{
		model()->sort(0);
		_dirty = false;
	}
}

void MyComboBox::showPopup()
{
	sort();
	QComboBox::showPopup();
}

/***************************************************************************

	class CTextBox

***************************************************************************/

CTextBox CTextBox::manager;

void CTextBox::onChange(void)
{
	RAISE_EVENT(EVENT_Change);
}


void CTextBox::onActivate(void)
{
	RAISE_EVENT(EVENT_Activate);
}


void CTextBox::onClick()
{
	GET_SENDER();
	raise_click_event(THIS);
}


void CTextBox::onSelectionChanged(void)
{
	GET_SENDER();
	GET_TEXT_BOX();

	if (THIS->locked)
		return;

	if (CCONTROL_last_event_type == QEvent::MetaCall)
	{
		THIS->locked = true;
		set_selection(textbox, THIS->start, THIS->length);
		THIS->locked = false;
	}
	else
	{
		get_selection(textbox, &THIS->start, &THIS->length);
	}
}




/***************************************************************************

	Descriptions

***************************************************************************/

GB_DESC CTextBoxSelectionDesc[] =
{
	GB_DECLARE(".TextBox.Selection", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Text", "s", TextBox_Selection_Text),
	GB_PROPERTY_READ("Length", "i", TextBox_Selection_Length),
	GB_PROPERTY_READ("Start", "i", TextBox_Selection_Start),
	GB_PROPERTY_READ("Pos", "i", TextBox_Selection_Start),

	GB_METHOD("Hide", NULL, TextBox_Unselect, NULL),

	GB_END_DECLARE
};


GB_DESC CTextBoxDesc[] =
{
	GB_DECLARE("TextBox", sizeof(CTEXTBOX)), GB_INHERITS("Control"),

	GB_METHOD("_new", NULL, TextBox_new, "(Parent)Container;"),

	GB_PROPERTY("Text", "s", TextBox_Text),
	GB_PROPERTY("Alignment", "i", TextBox_Alignment),
	GB_PROPERTY_READ("Length", "i", TextBox_Length),
	GB_PROPERTY("Pos", "i", TextBox_Pos),
	GB_PROPERTY("ReadOnly", "b", TextBox_ReadOnly),
	GB_PROPERTY("Border", "b", TextBox_Border),
	GB_PROPERTY("Password", "b", TextBox_Password),
	GB_PROPERTY("MaxLength", "i", TextBox_MaxLength),

	GB_PROPERTY_SELF("Selection", ".TextBox.Selection"),
	GB_METHOD("Select", NULL, TextBox_Select, "[(Start)i(Length)i]"),
	GB_METHOD("SelectAll", NULL, TextBox_SelectAll, NULL),
	GB_METHOD("Unselect", NULL, TextBox_Unselect, NULL),
	GB_PROPERTY_READ("Selected", "b", ComboBox_Selected),

	GB_METHOD("Clear", NULL, TextBox_Clear, NULL),
	GB_METHOD("Insert", NULL, TextBox_Insert, "(Text)s"),

	GB_EVENT("Change", NULL, NULL, &EVENT_Change),
	GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),

	TEXTBOX_DESCRIPTION,

	GB_END_DECLARE
};


GB_DESC CComboBoxItemDesc[] =
{
	GB_DECLARE(".ComboBox.Item", 0), GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Text", "s", CCOMBOBOX_item_text),

	GB_END_DECLARE
};


GB_DESC CComboBoxDesc[] =
{
	GB_DECLARE("ComboBox", sizeof(CCOMBOBOX)), GB_INHERITS("Control"),

	GB_METHOD("_new", NULL, ComboBox_new, "(Parent)Container;"),
	GB_METHOD("_get", ".ComboBox.Item", ComboBox_get, "(Index)i"),

	GB_PROPERTY("Text", "s", ComboBox_Text),
	GB_PROPERTY_READ("Length", "i", ComboBox_Length),
	GB_PROPERTY("Pos", "i", TextBox_Pos),
	GB_PROPERTY("ReadOnly", "b", ComboBox_ReadOnly),
	GB_PROPERTY("Password", "b", TextBox_Password),
	GB_PROPERTY("MaxLength", "i", TextBox_MaxLength),
	GB_PROPERTY("Border", "b", ComboBox_Border),

	GB_PROPERTY_SELF("Selection", ".TextBox.Selection"),
	GB_METHOD("Select", NULL, TextBox_Select, "[(Start)i(Length)i]"),
	GB_METHOD("SelectAll", NULL, TextBox_SelectAll, NULL),
	GB_METHOD("Unselect", NULL, TextBox_Unselect, NULL),
	GB_PROPERTY_READ("Selected", "b", ComboBox_Selected),

	GB_METHOD("Popup", NULL, ComboBox_Popup, NULL),
	GB_METHOD("Clear", NULL, ComboBox_Clear, NULL),
	GB_METHOD("Insert", NULL, TextBox_Insert, "(Text)s"),

	GB_METHOD("Add", NULL, ComboBox_Add, "(Item)s[(Index)i]"),
	GB_METHOD("Remove", NULL, ComboBox_Remove, "(Index)i"),

	GB_METHOD("Find", "i", ComboBox_Find, "(Item)s"),

	GB_PROPERTY("Sorted", "b", ComboBox_Sorted),

	GB_PROPERTY("List", "String[]", ComboBox_List),
	//GB_PROPERTY("Contents", "s", ComboBox_List),

	GB_PROPERTY_READ("Count", "i", ComboBox_Count),
	GB_PROPERTY_READ("Current", ".ComboBox.Item", ComboBox_Current),
	GB_PROPERTY("Index", "i", ComboBox_Index),

	GB_EVENT("Change", NULL, NULL, &EVENT_Change),
	GB_EVENT("Activate", NULL, NULL, &EVENT_Activate),
	GB_EVENT("Click", NULL, NULL, &EVENT_Click),

	COMBOBOX_DESCRIPTION,

	GB_END_DECLARE
};


