GB_DESC CDrawingAreaDesc[] =
{
  GB_DECLARE("DrawingArea", sizeof(CDRAWINGAREA)), GB_INHERITS("Container"),

  GB_METHOD("_new", NULL, CDRAWINGAREA_new, "(Parent)Container;"),

  GB_PROPERTY("Cached", "b", CDRAWINGAREA_cached),
  GB_PROPERTY("Merge", "b", CDRAWINGAREA_merge),
#ifdef GB_QT_COMPONENT
  GB_PROPERTY("Border", "i", CWIDGET_border_full),
  GB_PROPERTY("Background", "i", CDRAWINGAREA_background),
  GB_PROPERTY("BackColor", "i", CDRAWINGAREA_background),
#endif
  GB_PROPERTY("Tracking", "b", CDRAWINGAREA_track_mouse),
  GB_PROPERTY("Focus", "b", CDRAWINGAREA_focus),
  GB_PROPERTY("Enabled", "b", CDRAWINGAREA_enabled),

  GB_METHOD("Clear", NULL, CDRAWINGAREA_clear, NULL),

  GB_CONSTANT("_Properties", "s", CDRAWINGAREA_PROPERTIES),

  GB_CONSTANT("_DefaultEvent", "s", "Draw"),

  GB_EVENT("Draw", NULL, NULL, &EVENT_draw),

  GB_END_DECLARE
};

