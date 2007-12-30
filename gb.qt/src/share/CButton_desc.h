GB_DESC CButtonDesc[] =
{
  GB_DECLARE("Button", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CBUTTON_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CBUTTON_free, NULL),

  GB_PROPERTY("Text", "s", CBUTTON_text),
  GB_PROPERTY("Caption", "s", CBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CBUTTON_picture),

  GB_PROPERTY("Border", "b", CBUTTON_border),
  GB_PROPERTY("Default", "b", CBUTTON_default),
  GB_PROPERTY("Cancel", "b", CBUTTON_cancel),
  GB_PROPERTY("Value", "b", CBUTTON_value),

	BUTTON_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};

GB_DESC CToggleButtonDesc[] =
{
  GB_DECLARE("ToggleButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTOGGLEBUTTON_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTOGGLEBUTTON_free, NULL),

  GB_PROPERTY("Text", "s", CTOGGLEBUTTON_text),
  GB_PROPERTY("Caption", "s", CTOGGLEBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CTOGGLEBUTTON_picture),
  GB_PROPERTY("Value", "b", CTOGGLEBUTTON_value),
  //GB_PROPERTY("Flat", "b", CBUTTON_flat),
  GB_PROPERTY("Border", "b", CTOGGLEBUTTON_border),
  GB_PROPERTY("Radio", "b", CTOGGLEBUTTON_radio),

	TOGGLEBUTTON_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_ClickToggle),

  GB_END_DECLARE
};


GB_DESC CToolButtonDesc[] =
{
  GB_DECLARE("ToolButton", sizeof(CBUTTON)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CTOOLBUTTON_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTOOLBUTTON_free, NULL),

  GB_PROPERTY("Text", "s", CTOOLBUTTON_text),
  GB_PROPERTY("Caption", "s", CTOOLBUTTON_text),
  GB_PROPERTY("Picture", "Picture", CTOOLBUTTON_picture),
  GB_PROPERTY("Value", "b", CTOOLBUTTON_value),
  GB_PROPERTY("Toggle", "b", CTOOLBUTTON_toggle),
  GB_PROPERTY("Border", "b", CTOOLBUTTON_border),
  GB_PROPERTY("Radio", "b", CTOOLBUTTON_radio),

	TOOLBUTTON_DESCRIPTION,

  GB_EVENT("Click", NULL, NULL, &EVENT_ClickTool),

  GB_END_DECLARE
};
