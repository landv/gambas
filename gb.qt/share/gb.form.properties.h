/***************************************************************************

  gb.form.properties.h

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

#ifndef __GB_FORM_PROPERTIES_H
#define __GB_FORM_PROPERTIES_H

#define CCONTROL_PROPERTIES \
  "X{Position},Y{Position},Width{Dimension},Height{Dimension},Visible=True,Enabled=True,Font{Font}," \
  "Background{Color}=-1,Foreground{Color}=-1," \
  "Tag,Mouse{Mouse.Default;Blank;Arrow;Cross;Wait;Text;SizeAll;SizeH;SizeV;SizeN;SizeS;SizeW;" \
  "SizeE;SizeNWSE;SizeNESW;SplitH;SplitV;Pointing}=Default,ToolTip,Drop,Expand,Ignore"
#define CARRANGEMENT_PROPERTY "Arrangement{Arrange.None;Horizontal;Vertical;Row;Column;Fill}"
#define CPADDING_PROPERTIES "Spacing{Range:0;64},Padding{Range:0;64}"
#define CARRANGEMENT_PROPERTIES CARRANGEMENT_PROPERTY ",AutoResize," CPADDING_PROPERTIES
#define CUSERCONTROL_PROPERTIES "*"
#define CUSERCONTAINER_PROPERTIES "*," CARRANGEMENT_PROPERTIES

#define CLABEL_PROPERTIES "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=Normal,Border{Border.*},Transparent"
#define CTEXTLABEL_PROPERTIES "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=TopNormal,Border{Border.*},Transparent"
#define CPICTUREBOX_PROPERTIES "*,Picture,Stretch,AutoResize,Alignment{Align.*}=TopLeft,Border{Border.*}"
#define CTEXTBOX_PROPERTIES "*,Text,Alignment{Align.Normal;Left;Center;Right}=Normal,ReadOnly,Password,MaxLength,Border=True"
#define CTEXTAREA_PROPERTIES "*,Text,ReadOnly,Wrap,Border=True,ScrollBar{Scroll.*}=Both"
#define CTEXTEDIT_PROPERTIES "*,ReadOnly,ScrollBar{Scroll.*}=Both,Border=True"
#define CLISTBOX_PROPERTIES "*,List,Mode{Select.*}=Single,Sorted"
#define CCOMBOBOX_PROPERTIES "*,Text,ReadOnly,Password,MaxLength,List,Sorted"
#define CBUTTON_PROPERTIES "*,Action,Text,Picture,Border=True,Default,Cancel"
#define CTOGGLEBUTTON_PROPERTIES "*,Action,Text,Picture,Border=True,Radio,Value"
#define CTOOLBUTTON_PROPERTIES "*,Action,Text,Picture,Border,Radio,Toggle,Value"
#define CCHECKBOX_PROPERTIES "*,Action,Text,Tristate,Value{CheckBox.False;True;None}"
#define CRADIOBUTTON_PROPERTIES "*,Text,Value"
#define CSPINBOX_PROPERTIES "*,MinValue=0,MaxValue=100,Step=1,Wrap,Value"
#define CDRAWINGAREA_PROPERTIES "*,Cached,Tracking,Focus,Merge,Border{Border.*}"
#define CTREEVIEW_PROPERTIES "*,Mode{Select.*}=Single,Sorted,Editable,Border=True,ScrollBar{Scroll.*}=Both"
#define CLISTVIEW_PROPERTIES "*,Mode{Select.*}=Single,Sorted,Editable,Border=True,ScrollBar{Scroll.*}=Both"
#define CCOLUMNVIEW_PROPERTIES "*,Mode{Select.*}=Single,Sorted,Editable,Header=True,Resizable=False,AutoResize=True,Border=True,ScrollBar{Scroll.*}=Both"
#define CICONVIEW_PROPERTIES "*,Mode{Select.*}=Single,Sorted,Editable,GridWidth{Range:0;64}=0,Border=True,ScrollBar{Scroll.*}=Both"
#define CSCROLLVIEW_PROPERTIES "*," CARRANGEMENT_PROPERTY "," CPADDING_PROPERTIES ",Border=True,ScrollBar{Scroll.*}=Both"
#define CGRIDVIEW_PROPERTIES "*,Mode{Select.None;Single;Multiple}=None,Grid=True,Header{GridView.None;Vertical;Horizontal;Both}=None" \
                             ",Scrollbar{Scroll.*}=Both,Border=True,Resizable=True"
#define CPANEL_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Border{Border.*}"
#define CHBOX_PROPERTIES "*,AutoResize," CPADDING_PROPERTIES
#define CVBOX_PROPERTIES CHBOX_PROPERTIES
#define CHSPLIT_PROPERTIES "*"
#define CVSPLIT_PROPERTIES "*"
#define CFRAME_PROPERTIES "*,Text"
#define CTABSTRIP_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Count{Range:1;256}=1,Index,Text,Picture,Orientation{Align.Top;Bottom}"
#define CWINDOW_PROPERTIES "*,Action,Text,Icon,Picture,Mask,Persistent,Resizable=True,Border=True,Type{Window.Normal;Toolbar;Splash;Popup;Combo;Panel}=Normal,Stacking{Window.Normal;Above;Below}=Normal,Minimized,Maximized,FullScreen,Sticky,SkipTaskbar," CARRANGEMENT_PROPERTIES
#define CMOVIEBOX_PROPERTIES "*,Path,Playing,Border{Border.*}"
#define CPROGRESSBAR_PROPERTIES "*,Label=True"
#define CSCROLLBAR_PROPERTIES "*,MinValue=0,MaxValue=100,Step=1,PageStep=10,Tracking=True"
#define CSLIDER_PROPERTIES "*,MinValue=0,MaxValue=100,Step=1,PageStep=10,Tracking=True,Mark,Value"
#define CTRAYICON_PROPERTIES "Visible=False,Tag,Tooltip,Picture"
#define CMENU_PROPERTIES "Action,Text,Picture,Enabled=True,Toggle,Checked,Visible=True,Tag,Shortcut"

#define DESCRIBE_CONTROL(_prop, _event, _size) \
  GB_CONSTANT("_Properties", "s", _prop), \
  GB_CONSTANT("_DefaultEvent", "s", _event), \
  GB_CONSTANT("_DefaultSize", "s", _size)

#define DESCRIBE_CONTROL_NO_SIZE(_prop, _event) \
  GB_CONSTANT("_Properties", "s", _prop), \
  GB_CONSTANT("_DefaultEvent", "s", _event)

#define DESCRIBE_CONTAINER(_prop, _event, _arr) \
  GB_CONSTANT("_Properties", "s", _prop), \
  GB_CONSTANT("_DefaultEvent", "s", _event), \
  GB_CONSTANT("_DefaultSize", "s", "24,24"), \
  GB_CONSTANT("_Arrangement", "i", _arr)

#define BUTTON_DESCRIPTION DESCRIBE_CONTROL(CBUTTON_PROPERTIES, "Click", "16,3")
#define CHECKBOX_DESCRIPTION DESCRIBE_CONTROL(CCHECKBOX_PROPERTIES, "Click", "24,3")
#define COLUMNVIEW_DESCRIPTION DESCRIBE_CONTROL(CCOLUMNVIEW_PROPERTIES, "Click", "16,16")
#define COMBOBOX_DESCRIPTION DESCRIBE_CONTROL(CCOMBOBOX_PROPERTIES, "Click", "24,3")
#define CONTROL_DESCRIPTION DESCRIBE_CONTROL(CCONTROL_PROPERTIES, "MouseDown", "16,16")
#define DRAWINGAREA_DESCRIPTION DESCRIBE_CONTAINER(CDRAWINGAREA_PROPERTIES, "Draw", ARRANGE_NONE)
#define EMBEDDER_DESCRIPTION DESCRIBE_CONTROL("*", "Embed", "24,24")
#define FRAME_DESCRIPTION DESCRIBE_CONTAINER(CFRAME_PROPERTIES, "MouseDown", ARRANGE_NONE)
#define GRIDVIEW_DESCRIPTION DESCRIBE_CONTROL(CGRIDVIEW_PROPERTIES, "Click", "16,16")
#define HBOX_DESCRIPTION DESCRIBE_CONTAINER(CHBOX_PROPERTIES, "MouseDown", ARRANGE_HORIZONTAL)
#define HPANEL_DESCRIPTION DESCRIBE_CONTAINER(CHBOX_PROPERTIES, "MouseDown", ARRANGE_ROW)
#define HSPLIT_DESCRIPTION DESCRIBE_CONTAINER(CHSPLIT_PROPERTIES, "Resize", ARRANGE_HORIZONTAL)
#define ICONVIEW_DESCRIPTION DESCRIBE_CONTROL(CICONVIEW_PROPERTIES, "Click", "16,16")
#define LABEL_DESCRIPTION DESCRIBE_CONTROL(CLABEL_PROPERTIES, "MouseDown", "24,3")
#define LISTBOX_DESCRIPTION DESCRIBE_CONTROL(CLISTBOX_PROPERTIES, "Click", "16,16")
#define LISTVIEW_DESCRIPTION DESCRIBE_CONTROL(CLISTVIEW_PROPERTIES, "Click", "16,16")
#define MOVIEBOX_DESCRIPTION DESCRIBE_CONTROL(CMOVIEBOX_PROPERTIES, "MouseDown", "16,16")
#define PANEL_DESCRIPTION DESCRIBE_CONTAINER(CPANEL_PROPERTIES, "MouseDown", ARRANGE_FILL)
#define PICTUREBOX_DESCRIPTION DESCRIBE_CONTROL(CPICTUREBOX_PROPERTIES, "MouseDown", "16,16")
#define PROGRESSBAR_DESCRIPTION DESCRIBE_CONTROL(CPROGRESSBAR_PROPERTIES, "MouseDown", "36,3")
#define RADIOBUTTON_DESCRIPTION DESCRIBE_CONTROL(CRADIOBUTTON_PROPERTIES, "Click", "24,3")
#define SCROLLBAR_DESCRIPTION DESCRIBE_CONTROL(CSCROLLBAR_PROPERTIES, "Change", "36,3")
#define SCROLLVIEW_DESCRIPTION DESCRIBE_CONTAINER(CSCROLLVIEW_PROPERTIES, "MouseDown", ARRANGE_FILL)
#define SEPARATOR_DESCRIPTION DESCRIBE_CONTROL("*", "MouseDown", "1,3")
#define SLIDER_DESCRIPTION DESCRIBE_CONTROL(CSLIDER_PROPERTIES, "Change", "36,3")
#define SPINBOX_DESCRIPTION DESCRIBE_CONTROL(CSPINBOX_PROPERTIES, "Change", "9,3")
#define TABSTRIP_DESCRIPTION DESCRIBE_CONTAINER(CTABSTRIP_PROPERTIES, "Click", ARRANGE_FILL)
#define TEXTAREA_DESCRIPTION DESCRIBE_CONTROL(CTEXTAREA_PROPERTIES, "KeyPress", "16,16")
#define TEXTBOX_DESCRIPTION DESCRIBE_CONTROL(CTEXTBOX_PROPERTIES, "KeyPress", "24,3")
#define TEXTEDIT_DESCRIPTION DESCRIBE_CONTROL(CTEXTEDIT_PROPERTIES, "Change", "16,16")
#define TEXTLABEL_DESCRIPTION DESCRIBE_CONTROL(CTEXTLABEL_PROPERTIES, "MouseDown", "24,3")
#define TOGGLEBUTTON_DESCRIPTION DESCRIBE_CONTROL(CTOGGLEBUTTON_PROPERTIES, "Click", "16,3")
#define TOOLBUTTON_DESCRIPTION DESCRIBE_CONTROL(CTOOLBUTTON_PROPERTIES, "Click", "3,3")
#define TRAYICON_DESCRIPTION DESCRIBE_CONTROL(CTRAYICON_PROPERTIES, "Menu", "4,4")
#define TREEVIEW_DESCRIPTION DESCRIBE_CONTROL(CTREEVIEW_PROPERTIES, "Click", "16,16")
#define USERCONTROL_DESCRIPTION DESCRIBE_CONTROL(CUSERCONTROL_PROPERTIES, "MouseDown", "16,16")
#define USERCONTAINER_DESCRIPTION DESCRIBE_CONTAINER(CUSERCONTAINER_PROPERTIES, "MouseDown", ARRANGE_FILL)
#define VBOX_DESCRIPTION DESCRIBE_CONTAINER(CVBOX_PROPERTIES, "MouseDown", ARRANGE_VERTICAL)
#define VPANEL_DESCRIPTION DESCRIBE_CONTAINER(CVBOX_PROPERTIES, "MouseDown", ARRANGE_COLUMN)
#define VSPLIT_DESCRIPTION DESCRIBE_CONTAINER(CVSPLIT_PROPERTIES, "Resize", ARRANGE_VERTICAL)
#define MENU_DESCRIPTION DESCRIBE_CONTROL_NO_SIZE(CMENU_PROPERTIES, "Click")

#endif


