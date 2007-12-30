GB_DESC CDialogDesc[] =
{
  GB_DECLARE("Dialog", 0), GB_VIRTUAL_CLASS(),

#ifdef GB_QT_COMPONENT
  GB_STATIC_METHOD("_exit", NULL, CDIALOG_exit, NULL),
#endif

  GB_STATIC_METHOD("OpenFile", "b", CDIALOG_open_file, "[(Multi)b]"),
  GB_STATIC_METHOD("SaveFile", "b", CDIALOG_save_file, NULL),
  GB_STATIC_METHOD("SelectDirectory", "b", CDIALOG_get_directory, NULL),
  GB_STATIC_METHOD("SelectColor", "b", CDIALOG_get_color, NULL),
  GB_STATIC_METHOD("SelectFont", "b", CDIALOG_select_font, NULL),

  GB_STATIC_PROPERTY("Title", "s", CDIALOG_title),
  GB_STATIC_PROPERTY("Path", "s", CDIALOG_path),
  GB_STATIC_PROPERTY_READ("Paths", "String[]", CDIALOG_paths),
  GB_STATIC_PROPERTY("Filter", "String[]", CDIALOG_filter),
  GB_STATIC_PROPERTY("Color", "i", CDIALOG_color),
  GB_STATIC_PROPERTY("Font", "Font", CDIALOG_font),

  GB_END_DECLARE
};
