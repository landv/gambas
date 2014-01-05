/***************************************************************************

  gcombobox.cpp

  (c) 2004-2006 - Daniel Campos Fernández <dcamposf@gmail.com>

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

#include "widgets.h"
#include "gmainwindow.h"
#include "gdesktop.h"
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
		int index = data->index();
		if (index >= 0)
		{
			const char *text = index >= 0 ? data->itemText(index) : NULL;
			if (!text) text = "";
			data->lock();
			gtk_entry_set_text(GTK_ENTRY(data->entry), text);
			data->setIndex(index);
			data->unlock();
			data->emit(SIGNAL(data->onChange));
		}
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

static GtkWidget *_find_button;

static void cb_find_button(GtkWidget *widget, gpointer data)
{
	if (GTK_IS_TOGGLE_BUTTON(widget))
		_find_button = widget;
	else if (GTK_IS_CONTAINER(widget))
		gtk_container_forall(GTK_CONTAINER(widget), cb_find_button, NULL);
}

static GtkWidget *find_button(GtkWidget *combo)
{
	_find_button = NULL;
	gtk_container_forall(GTK_CONTAINER(combo), cb_find_button, NULL);
	return _find_button;
}

char *gComboBox::indexToKey(int index)
{
	char *key;
	GtkTreePath *path = indexToPath(index);
	key = find(path);
	gtk_tree_path_free(path);
	return key;	
}

void gComboBox::create(bool readOnly)
{
	bool first = !border;
	char *save = NULL;
	GB_COLOR bg, fg;
	
	//fprintf(stderr, "create: %d hasFocus = %d\n", readOnly, focus );
	
	lock();
	
	if (first)
		border = gtk_alignment_new(0, 0, 1, 1); //gtk_event_box_new();
	else
	{
		save = g_strdup(text());
		bg = background();
		fg = foreground();
	}
	
	if (widget)
	{
		if (cell) g_object_unref(cell);
		cell = NULL;
		gtk_widget_destroy(widget);
		_button = NULL;
	}
	
	if (readOnly)
	{
		widget = gtk_combo_box_new_with_model(GTK_TREE_MODEL(tree->store));
		entry = NULL;

		cell = gtk_cell_renderer_text_new ();
		g_object_ref_sink(cell);
		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), cell, true);
		//gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), cell, "text", 0, (void *)NULL);
		//g_object_set(cell, "ypad", 0, (void *)NULL);
		gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(widget), cell, (GtkCellLayoutDataFunc)combo_cell_text, (gpointer)tree, NULL);
	}
	else
	{
		GList *cells;
		
#if GTK_CHECK_VERSION(2, 24, 0)
		widget = gtk_combo_box_new_with_model_and_entry(GTK_TREE_MODEL(tree->store));
#else
		widget = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(tree->store), 0);
#endif
		entry = gtk_bin_get_child(GTK_BIN(widget));
#ifdef GTK3
		gtk_widget_set_hexpand(entry, FALSE);
#endif

		g_signal_handler_disconnect(widget, g_signal_handler_find(widget, G_SIGNAL_MATCH_ID, g_signal_lookup("changed", G_OBJECT_TYPE(widget)), 0, 0, 0, 0));
		
		cells = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(widget));
		cell = (GtkCellRenderer *)cells->data;
		g_list_free(cells);
		g_object_ref(cell);
		//gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), cell, "text", 0, (void *)NULL);
		//g_object_set(cell, "ypad", 0, (void *)NULL);
		gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(widget), cell, (GtkCellLayoutDataFunc)combo_cell_text, (gpointer)tree, NULL);
	}

	if (first)
	{
		realize(false);
	}
	else
	{
		gtk_container_add(GTK_CONTAINER(border), widget);
		gtk_widget_show(widget);
		widgetSignals();
	}
	
	g_signal_connect(G_OBJECT(widget), "changed", G_CALLBACK(cb_click), (gpointer)this);

	if (entry)
	{
		initEntry();
		setColorBase();
		//g_signal_connect(G_OBJECT(entry), "key-press-event", G_CALLBACK(gcb_keypress), (gpointer)this);
		//g_signal_connect(G_OBJECT(entry), "key-release-event", G_CALLBACK(gcb_keyrelease), (gpointer)this);
		g_signal_connect(G_OBJECT(entry), "focus-in-event", G_CALLBACK(gcb_focus_in), (gpointer)this);
		g_signal_connect(G_OBJECT(entry), "focus-out-event", G_CALLBACK(gcb_focus_out), (gpointer)this);
	}
	else
		setColorButton();
	
	updateFocusHandler();

	if (!first)
	{
		setBackground(bg);
		setForeground(fg);
		updateFont();
	}

	setText(save);
	g_free(save);
	
	unlock();
}

gComboBox::gComboBox(gContainer *parent) : gTextBox(parent, true)
{
	onChange = NULL;
	onClick = NULL;
	onActivate = NULL;
	
	_no_background = TRUE;
	_last_key = 0;
	_model_dirty = false;
	_model_dirty_timeout = 0;
	sort = false;
	border = widget = NULL;
	entry = NULL;
	_button = NULL;
	cell = NULL;
	
	g_typ = Type_gComboBox;
	
	tree = new gTree();
	tree->addColumn();
	//tree->addColumn();
	//tree->setHeaders(false);
	
	create(false);
}

gComboBox::~gComboBox()
{
	if (_model_dirty_timeout)
		g_source_remove(_model_dirty_timeout);
	
	 gtk_combo_box_popdown(GTK_COMBO_BOX(widget));
	
	if (cell) g_object_unref(cell);
	
	delete tree;
}

void gComboBox::popup()
{
	gtk_combo_box_popup(GTK_COMBO_BOX(widget));
}

#ifdef GTK3
void gComboBox::updateColor()
{
	gTextBox::updateColor();
	if (entry)
	{
		gt_widget_set_color(entry, FALSE, background(), _bg_name, &_bg_default);
		gt_widget_set_color(entry, TRUE, foreground(), _fg_name, &_fg_default);
	}
}
#else
void gComboBox::setRealBackground(gColor color)
{
	gControl::setRealBackground(color);
	if (entry) 
		set_gdk_base_color(entry, color);
}
#endif

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

char* gComboBox::itemText(int ind)
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
	//fprintf(stderr, "setIndex: %d\n", vl);
	
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

	if (entry)
		gTextBox::setText(itemText(vl));
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
	
	cell->setText(text);
	
	updateSort();
}

void gComboBox::setReadOnly(bool vl)
{
	if (isReadOnly() == vl)
		return;
	create(!isReadOnly());
}

void gComboBox::setSorted(bool vl)
{
	tree->setSorted(vl);
}

void gComboBox::setText(const char *vl)
{
	int index = find(vl);
	
	if (!entry || index >= 0)
		setIndex(index);
	else if (entry)
		gTextBox::setText(vl);
}

static gboolean combo_set_model_and_sort(gComboBox *combo)
{
	//fprintf(stderr, "combo_set_model_and_sort\n");
	gtk_combo_box_set_model(GTK_COMBO_BOX(combo->widget), GTK_TREE_MODEL(combo->tree->store));

	if (combo->isSorted())
		combo->tree->sort();
	
	combo->_model_dirty = false;
	combo->_model_dirty_timeout = 0;
	
	if (combo->isReadOnly())
		combo->checkIndex();
	
	return FALSE;
}

void gComboBox::updateModel()
{
	if (_model_dirty)
	{
		g_source_remove(_model_dirty_timeout);
		combo_set_model_and_sort(this);
	}
}

void gComboBox::updateSort()
{
	if (_model_dirty)
		return;
		
	_model_dirty = true;
	_model_dirty_timeout = g_timeout_add(0, (GSourceFunc)combo_set_model_and_sort, this);
	
	gtk_combo_box_set_model(GTK_COMBO_BOX(widget), NULL);
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

	//g_signal_lookup("rowGTK_TYPE_COMBO_BOX);

	row = tree->addRow(key, after, true);
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

	/*if (entry)
		gtk_widget_set_size_request(entry, 4, 4);*/
}

void gComboBox::updateFont()
{
	gControl::updateFont();
	if (cell)
		g_object_set(G_OBJECT(cell), "font-desc", font()->desc(), (void *)NULL);
	if (entry)
		gtk_widget_modify_font(entry, font()->desc());
}

void gComboBox::setFocus()
{
	gControl::setFocus();
	if (entry && window()->isVisible())
		gtk_widget_grab_focus(entry);
}

int gComboBox::minimumHeight()
{
	int h;
#ifdef GTK3
	gtk_widget_get_preferred_height(widget, &h, NULL);
#else
	GtkRequisition req;
	
	gtk_widget_size_request(widget, &req);
	h = req.height;
#endif

	if (entry)
		return h - 4;
	else
		return h;
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

bool gComboBox::hasBorder() const
{
	gboolean v;
	
	g_object_get(G_OBJECT(widget), "has-frame", &v, 	(void *)NULL);
	return v;
}

void gComboBox::setBorder(bool v)
{
	g_object_set(G_OBJECT(widget), "has-frame", v, 	(void *)NULL);
}
