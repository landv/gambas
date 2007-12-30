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

#define CARRANGEMENT_PROPERTY "Arrangement{Arrange.*}"
#define CPADDING_PROPERTIES "Spacing{Range:0;64},Padding{Range:0;64}"
#define CARRANGEMENT_PROPERTIES CARRANGEMENT_PROPERTY ",AutoResize," CPADDING_PROPERTIES

#define CUSERCONTROL_PROPERTIES "*"
#define CUSERCONTAINER_PROPERTIES "*," CARRANGEMENT_PROPERTIES

#define CLABEL_PROPERTIES "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=Normal,Border{Border.*}"

#define CTEXTLABEL_PROPERTIES "*,Padding{Range:0;64},AutoResize,Text,Alignment{Align.*}=TopNormal,Border{Border.*}"

#define CPICTUREBOX_PROPERTIES "*,Picture,Stretch,Alignment{Align.*}=TopLeft,Border{Border.*}"

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

#define CCOLUMNVIEW_PROPERTIES "*,Mode{Select.*}=Single,Sorted,Editable,Resizable=False,Header=True,Border=True,ScrollBar{Scroll.*}=Both"

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

#define CTABSTRIP_PROPERTIES "*," CARRANGEMENT_PROPERTIES ",Count{Range:1;256}=1,Index,Text,Picture,Orientation{TabStrip.Top;Bottom}"

#define CWINDOW_PROPERTIES "*,Action,Text,Icon,Picture,Mask,Persistent,Border{Window.None;Fixed;Resizable}=Resizable,ToolBox,Stacking{Window.Normal;Above;Below}=Normal,Minimized,Maximized,FullScreen,Sticky,SkipTaskbar," CARRANGEMENT_PROPERTY "," CPADDING_PROPERTIES

#endif


