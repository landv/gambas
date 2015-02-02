/***************************************************************************

  gb.form.properties.h

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

#ifndef __GB_FORM_PROPERTIES_H
#define __GB_FORM_PROPERTIES_H

#define CCONTROL_PROPERTIES \
	"X{Position},Y{Position},Width{Dimension},Height{Dimension},Visible=True,Enabled=True,Font{Font}," \
	"Background{Color}=-1,Foreground{Color}=-1," \
	"Tag,Tracking,NoTabFocus,Mouse{Mouse.Default;Blank;Arrow;Cross;Wait;Text;SizeAll;SizeH;SizeV;SizeN;SizeS;SizeW;" \
	"SizeE;SizeNWSE;SizeNESW;SplitH;SplitV;Pointing}=Default,ToolTip,Drop,Expand,Ignore,PopupMenu{Menu}"
#define CARRANGEMENT_PROPERTY "Arrangement{Arrange.None;Horizontal;Vertical;Row;Column;Fill}"
#define CPADDING_PROPERTIES "Spacing,Margin,Padding{Range:0;63},Indent"
#define CWINDOW_PADDING_PROPERTIES "Spacing,Margin,Padding{Range:0;63}"
#define CARRANGEMENT_PROPERTIES CARRANGEMENT_PROPERTY ",AutoResize," CPADDING_PROPERTIES ",Invert"
#define CWINDOW_ARRANGEMENT_PROPERTIES CARRANGEMENT_PROPERTY ",AutoResize," CWINDOW_PADDING_PROPERTIES ",Invert"
#define CUSERCONTROL_PROPERTIES "*"
#define CUSERCONTAINER_PROPERTIES "*," CARRANGEMENT_PROPERTIES

#define CBUTTON_PROPERTIES "*,Action,AutoResize,Text,Picture,Border=True,Default,Cancel"
#define CCHECKBOX_PROPERTIES "*,Action,AutoResize,Text,Tristate,Value{CheckBox.False;True;None}"
#define CCOMBOBOX_PROPERTIES "*,Action,ReadOnly,List,Text,Password,MaxLength,Sorted,Border=True"
#define CDIAL_PROPERTIES "*,MinValue=0,MaxValue=100,Step=1,PageStep=10,Wrap,Mark=True"
#define CDRAWINGAREA_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Border{Border.None;Plain;Sunken;Raised;Etched},Cached,Focus,NoBackground,Tablet"
#define CEDITOR_PROPERTIES "*,Font,Border=True,ScrollBar{Scroll.*}=Both,Highlight{Highlight.None;Custom;Gambas;HTML;CSS;WebPage;Diff;JavaScript;SQL}=None,ReadOnly=False,TabSize{Range:1;16}=2"
#define CFRAME_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Text"
#define CHBOX_PROPERTIES "*,AutoResize," CPADDING_PROPERTIES ",Invert"
#define CLABEL_PROPERTIES "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=Normal,Border{Border.None;Plain;Sunken;Raised;Etched},Transparent"
#define CLCDNUMBER_PROPERTIES "*,Value,Digits{Range:1;64}=1,SmallDecimalPoint,Style{LCDNumber.Outline;Filled;Flat}=Outline,Mode{LCDNumber.Decimal;Hexadecimal;Binary}=Decimal,Border{Border.None;Plain;Sunken;Raised;Etched}"
#define CLISTBOX_PROPERTIES "*,List,Border=True,Mode{Select.*}=Single,Sorted"
#define CMENU_PROPERTIES "Action,Text,Picture,Enabled=True,Radio,Toggle,Checked,Visible=True,Tag,Shortcut"
#define CMOVIEBOX_PROPERTIES "*,Path,Playing,Alignment{Align.*}=TopLeft,Border{Border.None;Plain;Sunken;Raised;Etched}"
#define CPANEL_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Border{Border.None;Plain;Sunken;Raised;Etched}"
#define CPICTUREBOX_PROPERTIES "*,Padding{Range:0;64},Picture,Stretch,AutoResize,Alignment{Align.*}=TopLeft,Border{Border.None;Plain;Sunken;Raised;Etched}"
#define CPRINTER_PROPERTIES "Orientation{Printer.Portrait;Landscape}=Portrait,Paper{Printer.A3;A4;A5;B5;Letter;Executive}=A4,CollateCopies,ReverseOrder,Duplex{Printer.Simplex;Horizontal;Vertical}=Simplex,GrayScale,FullPage"
#define CRADIOBUTTON_PROPERTIES "*,AutoResize,Text,Value"
#define CSCROLLBAR_PROPERTIES "*,MinValue=0,MaxValue=100,Step=1,PageStep=10,Tracking=True"
#define CSCROLLVIEW_PROPERTIES "*," CARRANGEMENT_PROPERTY "," CPADDING_PROPERTIES ",Border=True,ScrollBar{Scroll.*}=Both"
#define CSEPARATOR_PROPERTIES "*"
#define CSLIDER_PROPERTIES "*,Action,MinValue=0,MaxValue=100,Step=1,PageStep=10,Tracking=True,Mark,Value"
#define CSPINBOX_PROPERTIES "*,Action,MinValue=0,MaxValue=100,Step=1,Wrap,Value,Border=True"
#define CTABSTRIP_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Count{Range:1;256}=1,Index,Text,TextFont,Picture,Orientation{Align.Top;Bottom;Left;Right}=Top,Closable"
#define CTEXTAREA_PROPERTIES "*,Text,Alignment{Align.Normal;Left;Center;Right}=Normal,ReadOnly,Wrap,Border=True,ScrollBar{Scroll.*}=Both"
#define CTEXTBOX_PROPERTIES "*,Action,Text,Alignment{Align.Normal;Left;Center;Right}=Normal,ReadOnly,Password,MaxLength,Border=True"
#define CTEXTEDIT_PROPERTIES "*,ReadOnly,Wrap,Border=True,ScrollBar{Scroll.*}=Both"
#define CTEXTLABEL_PROPERTIES "*,Padding{Range:0;63},AutoResize,Text,Alignment{Align.*}=TopNormal,Wrap=True,Border{Border.None;Plain;Sunken;Raised;Etched},Transparent"
#define CTOGGLEBUTTON_PROPERTIES "*,Action,AutoResize,Text,Picture,Border=True,Radio,Value"
#define CTOOLBUTTON_PROPERTIES "*,Action,AutoResize,Text,Picture,Border,Radio,Toggle,Value"
#define CTRAYICON_PROPERTIES "Visible=False,Tag,Tooltip,Picture"
#define CVBOX_PROPERTIES "*,AutoResize," CPADDING_PROPERTIES ",Invert"
#define CWINDOW_PROPERTIES "*,Action,Text,Icon,Picture,Mask,Persistent,Resizable=True,Border=True,Utility,Stacking{Window.Normal;Above;Below}=Normal,Minimized,Maximized,FullScreen,Sticky,SkipTaskbar,Opacity{Range:0;100}=100,Transparent," CWINDOW_ARRANGEMENT_PROPERTIES

#define DESCRIBE_CONTROL(_prop, _event, _size) \
	GB_CONSTANT("_Properties", "s", _prop), \
	GB_CONSTANT("_DefaultEvent", "s", _event), \
	GB_CONSTANT("_DefaultSize", "s", _size)

#define DESCRIBE_VIEW_CONTROL(_prop, _event, _size) \
	GB_CONSTANT("_Properties", "s", _prop), \
	GB_CONSTANT("_DefaultEvent", "s", _event), \
	GB_CONSTANT("_DefaultSize", "s", _size), \
	GB_CONSTANT("_Group", "s", "View")

#define DESCRIBE_SPECIAL_CONTROL(_prop, _event, _size) \
	DESCRIBE_CONTROL(_prop, _event, _size), \
	GB_CONSTANT("_IsControl", "b", TRUE), \
	GB_CONSTANT("_IsVirtual", "b", TRUE), \
	GB_CONSTANT("_Family", "s", "Form"), \
	GB_CONSTANT("_Group", "s", "Special")

#define DESCRIBE_CONTAINER(_prop, _event) \
	GB_CONSTANT("_Properties", "s", _prop), \
	GB_CONSTANT("_DefaultEvent", "s", _event), \
	GB_CONSTANT("_DefaultSize", "s", "24,24")

#define DESCRIBE_CONTAINER_ARR(_prop, _event, _arr) \
	DESCRIBE_CONTAINER(_prop, _event), \
	GB_CONSTANT("_DefaultArrangement", "s", _arr)

#define DESCRIBE_MULTI_CONTAINER(_prop, _event) \
	GB_CONSTANT("_IsMultiContainer", "b", TRUE), \
	DESCRIBE_CONTAINER(_prop, _event, _arr)

#define DESCRIBE_MULTI_CONTAINER_ARR(_prop, _event, _arr) \
	GB_CONSTANT("_IsMultiContainer", "b", TRUE), \
	DESCRIBE_CONTAINER_ARR(_prop, _event, _arr)

#define CONTROL_DESCRIPTION \
	DESCRIBE_CONTROL(CCONTROL_PROPERTIES, "MouseDown", "16,16"), \
	GB_CONSTANT("_IsControl", "b", TRUE), \
	GB_CONSTANT("_Family", "s", "Form") \
	
#define CONTAINER_DESCRIPTION \
	GB_CONSTANT("_IsContainer", "b", TRUE), \
	GB_CONSTANT("_Group", "s", "Container") \
	
#define SIMILAR(_similar) GB_CONSTANT("_Similar", "s", _similar)
	
#define BUTTON_DESCRIPTION DESCRIBE_CONTROL(CBUTTON_PROPERTIES, "Click", "16,4")
#define CHECKBOX_DESCRIPTION DESCRIBE_CONTROL(CCHECKBOX_PROPERTIES, "Click", "24,4"), SIMILAR("Button")
#define COMBOBOX_DESCRIPTION DESCRIBE_CONTROL(CCOMBOBOX_PROPERTIES, "Click", "24,4"), SIMILAR("TextBox")
#define DIAL_DESCRIPTION DESCRIBE_CONTROL(CDIAL_PROPERTIES, "Change", "6,6"), SIMILAR("Slider")
#define DRAWINGAREA_DESCRIPTION DESCRIBE_CONTAINER(CDRAWINGAREA_PROPERTIES, "Draw")
#define EDITOR_DESCRIPTION DESCRIBE_CONTROL(CEDITOR_PROPERTIES, "KeyPress", "16,16"), SIMILAR("TextArea")
#define EMBEDDER_DESCRIPTION DESCRIBE_CONTROL("*", "Embed", "24,24"), GB_CONSTANT("_Group", "s", "Special")
#define FRAME_DESCRIPTION DESCRIBE_CONTAINER(CFRAME_PROPERTIES, "MouseDown"), SIMILAR("Panel")
#define HBOX_DESCRIPTION DESCRIBE_CONTAINER_ARR(CHBOX_PROPERTIES, "MouseDown", "H"), SIMILAR("Panel")
#define HPANEL_DESCRIPTION DESCRIBE_CONTAINER_ARR(CHBOX_PROPERTIES, "MouseDown", "R"), SIMILAR("Panel")
#define LABEL_DESCRIPTION DESCRIBE_CONTROL(CLABEL_PROPERTIES, "MouseDown", "24,4")
#define LCDNUMBER_DESCRIPTION DESCRIBE_CONTROL(CLCDNUMBER_PROPERTIES, "MouseDown", "24,6"), SIMILAR("Label"), GB_CONSTANT("_Group", "s", "Deprecated")
#define LISTBOX_DESCRIPTION DESCRIBE_CONTROL(CLISTBOX_PROPERTIES, "Click", "16,16")
#define MOVIEBOX_DESCRIPTION DESCRIBE_CONTROL(CMOVIEBOX_PROPERTIES, "MouseDown", "16,16")
#define PANEL_DESCRIPTION DESCRIBE_CONTAINER_ARR(CPANEL_PROPERTIES, "MouseDown", "F")
#define PICTUREBOX_DESCRIPTION DESCRIBE_CONTROL(CPICTUREBOX_PROPERTIES, "MouseDown", "16,16")
#define RADIOBUTTON_DESCRIPTION DESCRIBE_CONTROL(CRADIOBUTTON_PROPERTIES, "Click", "24,4"), SIMILAR("Button")
#define SCROLLBAR_DESCRIPTION DESCRIBE_CONTROL(CSCROLLBAR_PROPERTIES, "Change", "36,4"), SIMILAR("Slider")
#define SCROLLVIEW_DESCRIPTION DESCRIBE_CONTAINER_ARR(CSCROLLVIEW_PROPERTIES, "MouseDown", "F"), SIMILAR("Panel")
#define SEPARATOR_DESCRIPTION DESCRIBE_CONTROL(CSEPARATOR_PROPERTIES, "MouseDown", "1,4")
#define SLIDER_DESCRIPTION DESCRIBE_CONTROL(CSLIDER_PROPERTIES, "Change", "24,4")
#define SPINBOX_DESCRIPTION DESCRIBE_CONTROL(CSPINBOX_PROPERTIES, "Change", "9,4"), SIMILAR("TextBox,Slider")
#define TABSTRIP_DESCRIPTION DESCRIBE_MULTI_CONTAINER_ARR(CTABSTRIP_PROPERTIES, "Click", "F")
#define TEXTAREA_DESCRIPTION DESCRIBE_CONTROL(CTEXTAREA_PROPERTIES, "KeyPress", "16,16"), SIMILAR("TextBox")
#define TEXTBOX_DESCRIPTION DESCRIBE_CONTROL(CTEXTBOX_PROPERTIES, "KeyPress", "24,4")
#define TEXTEDIT_DESCRIPTION DESCRIBE_CONTROL(CTEXTEDIT_PROPERTIES, "Change", "16,16"), SIMILAR("TextArea")
#define TEXTLABEL_DESCRIPTION DESCRIBE_CONTROL(CTEXTLABEL_PROPERTIES, "MouseDown", "24,4"), SIMILAR("Label")
#define TOGGLEBUTTON_DESCRIPTION DESCRIBE_CONTROL(CTOGGLEBUTTON_PROPERTIES, "Click", "16,4"), SIMILAR("Button")
#define TOOLBUTTON_DESCRIPTION DESCRIBE_CONTROL(CTOOLBUTTON_PROPERTIES, "Click", "4,4"), SIMILAR("Button")
#define TRAYICON_DESCRIPTION DESCRIBE_SPECIAL_CONTROL(CTRAYICON_PROPERTIES, "Menu", "4,4")
#define USERCONTAINER_DESCRIPTION DESCRIBE_CONTAINER_ARR(CUSERCONTAINER_PROPERTIES, "MouseDown", "F")
#define VBOX_DESCRIPTION DESCRIBE_CONTAINER_ARR(CVBOX_PROPERTIES, "MouseDown", "V"), SIMILAR("Panel")
#define VPANEL_DESCRIPTION DESCRIBE_CONTAINER_ARR(CVBOX_PROPERTIES, "MouseDown", "C"), SIMILAR("Panel")
#define WINDOW_DESCRIPTION DESCRIBE_CONTAINER_ARR(CWINDOW_PROPERTIES, "Open", "F")

#define MENU_DESCRIPTION \
	GB_CONSTANT("_IsControl", "b", TRUE), \
	GB_CONSTANT("_IsContainer", "b", TRUE), \
	GB_CONSTANT("_Family", "s", "Form"), \
	GB_CONSTANT("_Properties", "s", CMENU_PROPERTIES), \
	GB_CONSTANT("_DefaultEvent", "s", "Click")

#define FORM_DESCRIPTION \
	GB_CONSTANT("_IsForm", "b", TRUE), \
	GB_CONSTANT("_HiddenControls", "s", "Form,Control,Menu,Container,UserControl,UserContainer,Window")

#define PRINTER_DESCRIPTION \
	GB_CONSTANT("_IsControl", "b", TRUE), \
	GB_CONSTANT("_Family", "s", "*"), \
	GB_CONSTANT("_IsVirtual", "b", TRUE), \
	GB_CONSTANT("_Group", "s", "Special"), \
	GB_CONSTANT("_Properties", "s", CPRINTER_PROPERTIES), \
	GB_CONSTANT("_DefaultEvent", "s", "Draw")

#define USERCONTROL_DESCRIPTION \
	DESCRIBE_CONTROL(CUSERCONTROL_PROPERTIES, "MouseDown", "16,16"), \
	GB_CONSTANT("_Group", "s", ""), \
	GB_CONSTANT("_IsContainer", "b", FALSE)


#endif


