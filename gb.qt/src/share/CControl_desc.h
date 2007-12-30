GB_DESC CControlDesc[] =
{
  GB_DECLARE("Control", sizeof(CCONTROL)), GB_NOT_CREATABLE(),

  GB_HOOK_CHECK(CCONTROL_check),

  GB_METHOD("_free", NULL, CCONTROL_delete, NULL),

  GB_METHOD("Move", NULL, CCONTROL_move, "(X)i(Y)i[(Width)i(Height)i]"),
  GB_METHOD("Resize", NULL, CCONTROL_resize, "(Width)i(Height)i"),

  GB_METHOD("MoveScaled", NULL, CCONTROL_move_scaled, "(X)f(Y)f[(Width)f(Height)f]"),
  GB_METHOD("ResizeScaled", NULL, CCONTROL_resize_scaled, "(Width)f(Height)f"),

  GB_METHOD("Delete", NULL, CCONTROL_delete, NULL),
  GB_METHOD("Show", NULL, CCONTROL_show, NULL),
  GB_METHOD("Hide", NULL, CCONTROL_hide, NULL),

  GB_METHOD("Raise", NULL, CCONTROL_raise, NULL),
  GB_METHOD("Lower", NULL, CCONTROL_lower, NULL),

  GB_PROPERTY("Next", "Control", CCONTROL_next),
  GB_PROPERTY("Previous", "Control", CCONTROL_previous),

  GB_METHOD("SetFocus", NULL, CCONTROL_set_focus, NULL),
  GB_METHOD("Refresh", NULL, CCONTROL_refresh, "[(X)i(Y)i(Width)i(Height)i]"),
  GB_METHOD("Grab", "Picture", CCONTROL_grab, NULL),
  GB_METHOD("Drag", NULL, CCONTROL_drag, "(Data)v[(Format)s]"),

  GB_METHOD("Reparent", NULL, CCONTROL_reparent, "(Container)Container;[(X)i(Y)i]"),

  GB_PROPERTY("X", "i", CCONTROL_x),
  GB_PROPERTY("Y", "i", CCONTROL_y),
  GB_PROPERTY_READ("ScreenX", "i", CCONTROL_screen_x),
  GB_PROPERTY_READ("ScreenY", "i", CCONTROL_screen_y),
  GB_PROPERTY("W", "i", CCONTROL_w),
  GB_PROPERTY("H", "i", CCONTROL_h),
  GB_PROPERTY("Left", "i", CCONTROL_x),
  GB_PROPERTY("Top", "i", CCONTROL_y),
  GB_PROPERTY("Width", "i", CCONTROL_w),
  GB_PROPERTY("Height", "i", CCONTROL_h),

  GB_PROPERTY("Visible", "b", CCONTROL_visible),
  GB_PROPERTY("Enabled", "b", CCONTROL_enabled),
  GB_PROPERTY("Expand", "b", CCONTROL_expand),

  GB_PROPERTY("Font", "Font", CCONTROL_font),
  GB_PROPERTY("Background", "i", CCONTROL_background),
  GB_PROPERTY("BackColor", "i", CCONTROL_background),
  GB_PROPERTY("Foreground", "i", CCONTROL_foreground),
  GB_PROPERTY("ForeColor", "i", CCONTROL_foreground),

  GB_PROPERTY("Design", "b", CCONTROL_design),
  GB_PROPERTY("Tag", "v", CCONTROL_tag),
  GB_PROPERTY("Mouse", "i<" MOUSE_CONSTANTS ">", CCONTROL_mouse),
  GB_PROPERTY("Cursor", "Cursor", CCONTROL_cursor),
  GB_PROPERTY("ToolTip", "s", CCONTROL_tooltip),
  GB_PROPERTY("Drop", "b", CCONTROL_drop),

  GB_PROPERTY_READ("Parent", "Container", CCONTROL_parent),
  GB_PROPERTY_READ("Window", "Window", CCONTROL_window),
  GB_PROPERTY_READ("Id", "i", CCONTROL_id),
  GB_PROPERTY_READ("Handle", "i", CCONTROL_id),

  GB_EVENT("Enter", NULL, NULL, &EVENT_Enter),
  GB_EVENT("GotFocus", NULL, NULL, &EVENT_GotFocus),
  GB_EVENT("LostFocus", NULL, NULL, &EVENT_LostFocus),
  GB_EVENT("KeyPress", NULL, NULL, &EVENT_KeyPress),
  GB_EVENT("KeyRelease", NULL, NULL, &EVENT_KeyRelease),
  GB_EVENT("Leave", NULL, NULL, &EVENT_Leave),
  GB_EVENT("MouseDown", NULL, NULL, &EVENT_MouseDown),
  GB_EVENT("MouseMove", NULL, NULL, &EVENT_MouseMove),
  GB_EVENT("MouseDrag", NULL, NULL, &EVENT_MouseDrag),
  GB_EVENT("MouseUp", NULL, NULL, &EVENT_MouseUp),
  GB_EVENT("MouseWheel", NULL, NULL, &EVENT_MouseWheel),
  GB_EVENT("DblClick", NULL, NULL, &EVENT_DblClick),
  GB_EVENT("Menu", NULL, NULL, &EVENT_Menu),
  GB_EVENT("Drag", NULL, NULL, &EVENT_Drag),
  GB_EVENT("DragMove", NULL, NULL, &EVENT_DragMove),
  GB_EVENT("Drop", NULL, NULL, &EVENT_Drop),

  GB_CONSTANT("_DefaultEvent", "s", "MouseDown"),
  GB_CONSTANT("_Properties", "s",
    "X{Position},Y{Position},Width{Dimension},Height{Dimension},Visible=True,Enabled=True,Font{Font},"
    "Background{Color}=-1,Foreground{Color}=-1,"
    "Tag,Mouse{Mouse.Default;Blank;Arrow;Cross;Wait;Text;SizeAll;SizeH;SizeV;SizeN;SizeS;SizeW;"
    "SizeE;SizeNWSE;SizeNESW;SplitH;SplitV;Pointing}=Default,ToolTip,Drop,Expand"
    ),
  GB_CONSTANT("_DefaultSize", "s", "16,16"),

  GB_END_DECLARE
};

