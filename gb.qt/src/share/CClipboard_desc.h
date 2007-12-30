GB_DESC CClipboardDesc[] =
{
  GB_DECLARE("Clipboard", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Text", "i", 1),
  GB_CONSTANT("Image", "i", 2),

  GB_STATIC_METHOD("_init", NULL, CCLIPBOARD_init, NULL),

  GB_STATIC_METHOD("Clear", NULL, CCLIPBOARD_clear, NULL),

  GB_STATIC_PROPERTY_READ("Format", "s", CCLIPBOARD_format),
  GB_STATIC_PROPERTY_READ("Formats", "String[]", CCLIPBOARD_formats),
  GB_STATIC_PROPERTY_READ("Type", "i", CCLIPBOARD_type),

  GB_STATIC_METHOD("Copy", NULL, CCLIPBOARD_copy, "(Data)v[(Format)s]"),
  GB_STATIC_METHOD("Paste", "v", CCLIPBOARD_paste, "[(Format)s]"),

  GB_END_DECLARE
};
