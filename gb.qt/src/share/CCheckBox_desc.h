GB_DESC CCheckBoxDesc[] =
{
  GB_DECLARE("CheckBox", sizeof(CCHECKBOX)), GB_INHERITS("Control"),

  GB_METHOD("_new", NULL, CCHECKBOX_new, "(Parent)Container;"),

	GB_CONSTANT("False", "i", 0),
	GB_CONSTANT("True", "i", -1),
	GB_CONSTANT("None", "i", 1),

  GB_PROPERTY("Text", "s", CCHECKBOX_text),
  GB_PROPERTY("Caption", "s", CCHECKBOX_text),

  GB_PROPERTY("Value", "i", CCHECKBOX_value),
  GB_PROPERTY("Tristate", "b", CCHECKBOX_tristate),
  GB_PROPERTY("Action", "s", CCONTROL_action),

  GB_CONSTANT("_Properties", "s", "*,Action,Text,Tristate,Value{CheckBox.False;True;None}"),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),
  GB_CONSTANT("_DefaultSize", "s", "24,3"),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};


