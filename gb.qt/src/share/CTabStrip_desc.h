GB_DESC CTabChildrenDesc[] =
{
  GB_DECLARE(".TabChildren", 0), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CTAB_next, NULL),
  GB_PROPERTY_READ("Count", "i", CTAB_count),

  GB_END_DECLARE
};


GB_DESC CTabDesc[] =
{
  GB_DECLARE(".Tab", 0), GB_VIRTUAL_CLASS(),

  GB_PROPERTY("Text", "s", CTAB_text),
  GB_PROPERTY("Picture", "Picture", CTAB_picture),
  GB_PROPERTY("Caption", "s", CTAB_text),
  GB_PROPERTY("Enabled", "b", CTAB_enabled),
  GB_PROPERTY("Visible", "b", CTAB_visible),
  GB_PROPERTY_SELF("Children", ".TabChildren"),
  GB_METHOD("Delete", NULL, CTAB_delete, NULL),

  GB_END_DECLARE
};


GB_DESC CTabStripDesc[] =
{
  GB_DECLARE("TabStrip", sizeof(CTABSTRIP)), GB_INHERITS("Container"),

  GB_CONSTANT("Top", "i", 0),
  GB_CONSTANT("Bottom", "i", 1),

  GB_METHOD("_new", NULL, CTABSTRIP_new, "(Parent)Container;"),
  GB_METHOD("_free", NULL, CTABSTRIP_free, NULL),

  GB_PROPERTY("Count", "i", CTABSTRIP_count),
  GB_PROPERTY("Text", "s", CTABSTRIP_text),
  GB_PROPERTY("Picture", "Picture", CTABSTRIP_picture),
  GB_PROPERTY("Caption", "s", CTABSTRIP_text),
  GB_PROPERTY_READ("Current", ".Tab", CTABSTRIP_current),
  GB_PROPERTY("Index", "i", CTABSTRIP_index),
  GB_PROPERTY("Orientation", "i", CTABSTRIP_orientation),
  GB_PROPERTY("Enabled", "b", CTABSTRIP_enabled),

	#ifdef GB_QT_COMPONENT
  GB_PROPERTY_READ("ClientX", "i", CTABSTRIP_client_x),
  GB_PROPERTY_READ("ClientY", "i", CTABSTRIP_client_y),
  GB_PROPERTY_READ("ClientW", "i", CTABSTRIP_client_width),
  GB_PROPERTY_READ("ClientWidth", "i", CTABSTRIP_client_width),
  GB_PROPERTY_READ("ClientH", "i", CTABSTRIP_client_height),
  GB_PROPERTY_READ("ClientHeight", "i", CTABSTRIP_client_height),
  #endif

  GB_PROPERTY("Arrangement", "i", CCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CCONTAINER_spacing),

  GB_METHOD("_get", ".Tab", CTABSTRIP_get, "(Index)i"),

  GB_CONSTANT("_Properties", "s", CTABSTRIP_PROPERTIES),
  GB_CONSTANT("_DefaultEvent", "s", "Click"),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_EVENT("Click", NULL, NULL, &EVENT_Click),

  GB_END_DECLARE
};

