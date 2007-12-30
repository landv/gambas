GB_DESC CDragDesc[] =
{
  GB_DECLARE("Drag", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_CONSTANT("Copy", "i", 0),
  GB_CONSTANT("Link", "i", 1),
  GB_CONSTANT("Move", "i", 2),

  GB_STATIC_PROPERTY("Icon", "Picture", CDRAG_picture),

  GB_STATIC_PROPERTY_READ("Data", "v", CDRAG_data),
  GB_STATIC_PROPERTY_READ("Format", "s", CDRAG_format),
  GB_STATIC_PROPERTY_READ("Type", "i", CDRAG_type),
  GB_STATIC_PROPERTY_READ("Action", "i", CDRAG_action),
  GB_STATIC_PROPERTY_READ("Source", "Control", CDRAG_source),
  GB_STATIC_PROPERTY_READ("X", "i", CDRAG_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAG_y),
  GB_STATIC_PROPERTY_READ("Pending", "b", CDRAG_pending),

  GB_STATIC_METHOD("_call", NULL, CDRAG_call, "(Source)Control;(Data)v[(Format)s]"),
  GB_STATIC_METHOD("Show", NULL, CDRAG_show, "(Control)Control;[(X)i(Y)i(Width)i(Height)i]"),
  GB_STATIC_METHOD("Hide", NULL, CDRAG_hide, NULL),
  GB_STATIC_METHOD("_exit", NULL, CDRAG_exit, NULL),

  GB_END_DECLARE
};

