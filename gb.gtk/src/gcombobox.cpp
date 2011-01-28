/***************************************************************************

  gcombobox.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>
  
  Gtkmae "GTK+ made easy" classes
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include "widgets.h"
#include "widgets_private.h"
#include "gmainwindow.h"
#include "gcombobox.h"

/**************************************************************************
	
	gComboBox
	
**************************************************************************/

static void cb_click(GtkComboBox *widget,gComboBox *data)
{
	if (data->locked())
			return;
	
	if (!data->isReadOnly() && data->count())
	{
		data->lock();
		int index = data->index();
		const char *text = index >= 0 ? data->itemText(index) : NULL;
		if (!text) text = "";
		gtk_entry_set_text(GTK_ENTRY(data->entry), text);
		data->unlock();
		data->emit(SIGNAL(data->onChange));
	}
	
	if (data->index() >= 0)
		data->emit(SIGNAL(data->onClick));
}

/*static int pathToIndex(GtkTreePath *path)
{
	gint *indices;
	
	if (!path) 
		return -1;
	
	indices = gtk_tree_path_get_indices(path);
	return indices[0];
}*/

static GtkTreePath *indexToPath(int index)
{
	char buffer[16];
	sprintf(buffer, "%d", index);
	return gtk_tree_path_new_from_string(buffer);
}

static void combo_cell_text(GtkComboBox *combo, GtkCellRenderer *cell, GtkTreeModel *md, GtkTreeIter *iter, gTree *tr)
{
	gTreeRow *row = NULL;
	gTreeCell *data;
	const char *buf = "";
	char *key;

	key = tr->iterToKey(iter);
	if (key)
		row = (gTreeRow*)g_hash_table_lookup(tr->datakey, (gpointer)key);
	
	if (row)
	{
		data = row->get(0);
		if (data)
		{
			if (data->text()) 
				buf	=	data->text();
		}
	}

	g_object_set(G_OBJECT(cell),
		"text", buf,
		(void *)NULL);
}

static GtkWidget *_button;

static void cb_find_button(GtkWidget *widget, gpointer data)
{
	if (GTK_IS_TOGGLE_BUTTON(widget))
		_button = widget;
	else if (GTK_IS_CONTAINER(widget))
		gtk_container_forall(GTK_CONTAINER(widget), cb_find_button, NULL);
}

static GtkWidget *find_button(GtkWidget *combo)
{
	_button = NULL;
	gtk_container_forall(GTK_CONTAINER(combo), cb_find_button, NULL);
	return _button;
}

char *gComboBox::indexToKey(int index)
{
	char *key;
	GtkTreePath *path = indexToPath(index);
	key = find(path);
	gtk_tree_path_free(path);
	return key;	
}

gComboBox::gComboBox(gContainer *parent) : gTextBox(parent, true)
{
	/*if (!_style_init)
	{
		gtk_rc_parse_string(
			"style \"gambas-default-combo-box-style\" {\n"
			"  GtkComboBox::appears-as-list = 1\n"
			"}\n"
			"class \"GtkComboBox\" style : gtk \"gambas-default-combo-box-style\"\n"
			);
		_style_init = TRUE;
	}*/

	onChange = NULL;
	onClick = NULL;
	onActivate = NULL;
	
	//_no_click = false;
	_last_key = 0;
	_model_dirty = false;
	sort = false;
	entry = NULL;
	_button = NULL;
	
	g_typ = Type_gComboBox;
	
	tree = new gTree(NULL);
	tree->addColumn();
	tree->setHeaders(false);
	
	border = gtk_event_box_new();
	widget = gtk_combo_box_new_with_model(GTK_TREE_MODEL(tree->store));
	
	cell = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), cell, true);
	//gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), cell, "text", 0, (void *)NULL);
	g_object_set(cell, "ypad", 0, (void *)NULL);
	gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(widget), cell, (GtkCellLayoutDataFunc)combo_cell_text, (gpointer)tree, NULL);
	
	realize(false);
	
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(cb_click), (gpointer)this);

	//button = ((_GtkComboBoxPrivate *)GTK_COMBO_BOX(widget)->priv)->arrow->parent;
	updateFocusHandler();
	setReadOnly(false);
}

gComboBox::~gComboBox()
{
	gt_object_unref_sink(cell);
	g_object_unref(G_OBJECT(tree->store));
}

void gComboBox::popup()
{
	gtk_combo_box_popup(GTK_COMBO_BOX(widget));
}

void gComboBox::setRealBackground(gColor color)
{
	gControl::setRealBackground(color);
	if (entry) 
		set_gdk_base_color(entry, color);
}


void gComboBox::setRealForeground(gColor color)
{
	gControl::setRealForeground(color);
	if (entry) 
		set_gdk_text_color(entry, color);
}

int gComboBox::count()
{
	return tree->rowCount();
}

int gComboBox::index()
{
	updateModel();
	return gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
}

char *gComboBox::itemText(int ind)
{
	gTreeRow *row;
	gTreeCell *cell;
	
	if (ind < 0)
		return NULL;
	
	updateModel();
	
	char *key = indexToKey(ind);
	
	if (!key) return NULL;
	row = tree->getRow(key);
	if (!row) return NULL;
	cell = row->get(0);
	if (!cell) return NULL;
	return cell->text();
}

int gComboBox::length()
{
	gchar *buf;
	
	if (!entry)
	{
		buf = itemText(index());
		if (!buf) 
			return 0;
		else
			return g_utf8_strlen(buf, -1);
	}
	else
		return gTextBox::length();
}

bool gComboBox::isSorted()
{
	return tree->isSorted();
}

char* gComboBox::text()
{
	if (entry)
		return gTextBox::text();
	else
		return itemText(index());
}

void gComboBox::setIndex(int vl)
{
	if (vl < 0)
		vl = -1;
	else if (vl >= count()) 
		return;
	
	if (vl == index())
	{
		emit(SIGNAL(onClick));
		return;
	}
	
	updateModel();
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), vl);
}

void gComboBox::checkIndex()
{
	if (index() < 0)
	{
		lock();
		setIndex(0);
		unlock();
	}
}

void gComboBox::setItemText(int ind, const char *text)
{
	gTreeRow *row;
	gTreeCell *cell;
	
	char *key = indexToKey(ind);
	
	if (!key) return;
	row = tree->getRow(key);
	if (!row) return;
	cell = row->get(0);
	if (!cell) return;
	
	//fprintf(stderr, "setItemText: %d %s\n", ind, text);
	cell->setText(text);
	
	updateSort();
}

void gComboBox::setReadOnly(bool vl)
{
	if (isReadOnly() == vl)
		return;
	
	if ((!vl) && (!entry) )
	{		
		entry = gtk_entry_new ();
		/* this flag is a hack to tell the entry to fill its allocation. */
		GTK_ENTRY(entry)->is_cell_renderer = TRUE;
		gtk_container_add(GTK_CONTAINER(widget), entry);
		//gtk_widget_set_size_request(entry, width(), height());
		gtk_widget_show(entry);

		if (count())
			gTextBox::setText(itemText(index()));
		
		setBackground(background());
		setForeground(foreground());
		setFont(font());
		
		initEntry();
		//g_signal_connect(G_OBJECT(entry), "key-press-event", G_CALLBACK(gcb_keypress), (gpointer)this);
		//g_signal_connect(G_OBJECT(entry), "key-release-event", G_CALLBACK(gcb_keyrelease), (gpointer)this);
		g_signal_connect(G_OBJECT(entry), "focus-in-event", G_CALLBACK(gcb_focus_in), (gpointer)this);
		g_signal_connect(G_OBJECT(entry), "focus-out-event", G_CALLBACK(gcb_focus_out), (gpointer)this);
	}
	
	if (vl && entry) 
	{
		char *txt = g_strdup(text());
		
		setIndex(-1);
		gtk_widget_destroy(entry);
		entry = NULL;
		_button = NULL;
		
		_model_dirty = true;
		updateModel();
		
		setText(txt);
		g_free(txt);
	}
	
	updateFocusHandler();
}

void gComboBox::setSorted(bool vl)
{
	tree->setSorted(vl);
}

void gComboBox::setText(const char *vl)
{
	int index = find(vl);
	
	if (index >= 0)
		setIndex(index);
	else
	{
		setIndex(-1);
		if (entry)
			gTextBox::setText(vl);
	}
}

static gboolean combo_set_model_and_sort(gComboBox *combo)
{
	gtk_combo_box_set_model(GTK_COMBO_BOX(combo->widget), GTK_TREE_MODEL(combo->tree->store));
	if (combo->isSorted())
		combo->tree->sort();
	combo->_model_dirty = false;
	if (combo->isReadOnly())
		combo->checkIndex();
	return FALSE;
}

void gComboBox::updateModel()
{
	if (_model_dirty)
		combo_set_model_and_sort(this);
}

void gComboBox::updateSort()
{
	if (_model_dirty)
		return;
		
	_model_dirty = true;
	gtk_combo_box_set_model(GTK_COMBO_BOX(widget), NULL);
	g_timeout_add(0, (GSourceFunc)combo_set_model_and_sort, this);
}

void gComboBox::add(const char *text, int pos)
{
	//GtkTreeModel *model;
	gTreeRow *row;
	gTreeCell *cell;
	
	char key[16];
	char *after;
	
	_last_key++;
	sprintf(key, "%d", _last_key);
	
	if (pos < 0 || pos > count())
		after = NULL;
	else
		after = indexToKey(pos);

	row = tree->addRow(key, NULL, after, true);
	if (row) 
	{
		cell = row->get(0);
		if (cell)
		{
			cell->setText(text);
			updateSort();
		}
	}
	
	//gtk_combo_box_set_model(GTK_COMBO_BOX(widget), model);
}

void gComboBox::clear()
{
	lock();
	tree->clear();
	unlock();
}

int gComboBox::find(const char *text)
{
	int i;
	const char *it;
	
	if (!text)
		text = "";
	
	for (i = 0; i < count(); i++)
	{
		it = itemText(i);
		if (!it)
		 it = "";
		if (!strcmp(it, text))
			return i;
	}
	
	return -1;
}

void gComboBox::remove(int pos)
{
	updateModel();
	tree->removeRow(indexToKey(pos));
	updateSort();
}

void gComboBox::resize(int w, int h)
{
	gControl::resize(w,h);
	
	if (entry)
		gtk_widget_set_size_request(entry, width(), height());
}

void gComboBox::setFont(gFont *f)
{
	gControl::setFont(f);
	g_object_set(G_OBJECT(cell), "font-desc", font() ? font()->desc() : NULL, (void *)NULL);
	if (entry)
		gtk_widget_modify_font(entry, font() ? font()->desc() : NULL);
}

void gComboBox::setFocus()
{
	gControl::setFocus();
	if (entry && window()->isVisible())
		gtk_widget_grab_focus(entry);
}

int gComboBox::minimumHeight()
{
	GtkRequisition req;
	
	gtk_widget_size_request(widget, &req);
	if (entry)
		return req.height - 4;
	else
		return req.height;
}


bool gComboBox::isReadOnly()
{
	return entry == NULL;
}


static gboolean button_focus_in(GtkWidget *widget, GdkEventFocus *event, gComboBox *control)
{	
	if (control->isReadOnly())
		return gcb_focus_in(widget, event, control);

	control->setFocus();
	return false;
}

static gboolean button_focus_out(GtkWidget *widget, GdkEventFocus *event, gComboBox *control)
{	
	if (control->isReadOnly())
		return gcb_focus_out(widget, event, control);
	
	return false;
}

void gComboBox::updateFocusHandler()
{
	GtkWidget *button = find_button(widget);
	
	if (button == _button)
		return;
	
	_button = button;
	//g_signal_connect(G_OBJECT(button), "key-press-event", G_CALLBACK(gcb_keypress), (gpointer)this);
	//g_signal_connect(G_OBJECT(button), "key-release-event", G_CALLBACK(gcb_keyrelease), (gpointer)this);
	g_signal_connect(G_OBJECT(button), "focus-in-event", G_CALLBACK(button_focus_in), (gpointer)this);
	g_signal_connect(G_OBJECT(button), "focus-out-event", G_CALLBACK(button_focus_out), (gpointer)this);
}
