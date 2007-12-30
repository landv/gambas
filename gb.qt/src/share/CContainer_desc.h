GB_DESC CChildrenDesc[] =
{
  GB_DECLARE(".ContainerChildren", sizeof(CCONTAINER)), GB_VIRTUAL_CLASS(),

  GB_METHOD("_next", "Control", CCONTAINER_children_next, NULL),
  GB_PROPERTY_READ("Count", "i", CCONTAINER_children_count),

  GB_END_DECLARE
};


GB_DESC CContainerDesc[] =
{
  GB_DECLARE("Container", sizeof(CCONTAINER)), GB_INHERITS("Control"),
  GB_NOT_CREATABLE(),

  GB_PROPERTY_SELF("Children", ".ContainerChildren"),

  GB_PROPERTY_READ("ClientX", "i", CCONTAINER_x),
  GB_PROPERTY_READ("ClientY", "i", CCONTAINER_y),
  GB_PROPERTY_READ("ClientW", "i", CCONTAINER_width),
  GB_PROPERTY_READ("ClientWidth", "i", CCONTAINER_width),
  GB_PROPERTY_READ("ClientH", "i", CCONTAINER_height),
  GB_PROPERTY_READ("ClientHeight", "i", CCONTAINER_height),
  
  GB_METHOD("Find", "Control", CCONTAINER_find, "(X)i(Y)i"),

  GB_EVENT("Arrange", NULL, NULL, &EVENT_Arrange),
  GB_EVENT("Insert", NULL, "(Control)Control", &EVENT_Insert),

  GB_CONSTANT("_Properties", "s", "*"),
  GB_CONSTANT("_DefaultSize", "s", "24,24"),

  GB_END_DECLARE
};


GB_DESC CUserControlDesc[] =
{
  GB_DECLARE("UserControl", sizeof(CCONTAINER)), GB_INHERITS("Container"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),

  GB_PROPERTY("_Container", "Container", CUSERCONTROL_container),
  GB_PROPERTY("_AutoResize", "b", CCONTAINER_auto_resize),

  GB_CONSTANT("_Properties", "s", CUSERCONTROL_PROPERTIES),
  GB_CONSTANT("_DefaultSize", "s", "16,16"),

  GB_END_DECLARE
};


GB_DESC CUserContainerDesc[] =
{
  GB_DECLARE("UserContainer", sizeof(CUSERCONTAINER)), GB_INHERITS("Container"),
  GB_NOT_CREATABLE(),

  GB_METHOD("_new", NULL, CUSERCONTROL_new, "(Parent)Container;"),

  //GB_PROPERTY("Container", "Container", CUSERCONTAINER_container),
  GB_PROPERTY("_Container", "Container", CUSERCONTAINER_container),

  GB_PROPERTY("Arrangement", "i", CUSERCONTAINER_arrangement),
  GB_PROPERTY("AutoResize", "b", CUSERCONTAINER_auto_resize),
  GB_PROPERTY("Padding", "i", CUSERCONTAINER_padding),
  GB_PROPERTY("Spacing", "i", CUSERCONTAINER_spacing),
  
  GB_PROPERTY("Design", "b", CUSERCONTAINER_design),

  GB_CONSTANT("_Properties", "s", CUSERCONTAINER_PROPERTIES),
  GB_CONSTANT("_Arrangement", "i", ARRANGE_FILL),

  GB_END_DECLARE
};

