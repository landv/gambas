GB_DESC CDrawClipDesc[] =
{
  GB_DECLARE(".DrawClip", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY_READ("X", "i", CDRAW_clip_x),
  GB_STATIC_PROPERTY_READ("Y", "i", CDRAW_clip_y),
  GB_STATIC_PROPERTY_READ("W", "i", CDRAW_clip_w),
  GB_STATIC_PROPERTY_READ("H", "i", CDRAW_clip_h),
  GB_STATIC_PROPERTY_READ("Width", "i", CDRAW_clip_w),
  GB_STATIC_PROPERTY_READ("Height", "i", CDRAW_clip_h),

  GB_STATIC_PROPERTY("Enabled", "b", CDRAW_clip_enabled),
  GB_STATIC_METHOD("_call", NULL, CDRAW_clip, "(X)i(Y)i(Width)i(Height)i"),

  GB_END_DECLARE
};

GB_DESC CDrawDesc[] =
{
  GB_DECLARE("Draw", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_METHOD("_init", NULL, CDRAW_init, NULL),
  GB_STATIC_METHOD("_exit", NULL, CDRAW_exit, NULL),

  GB_STATIC_METHOD("Begin", NULL, CDRAW_begin, "(Device)o"),
  GB_STATIC_METHOD("End", NULL, CDRAW_end, NULL),
  GB_STATIC_PROPERTY_READ("Device", "o", CDRAW_device),

  GB_STATIC_PROPERTY_READ("W", "i", CDRAW_width),
  GB_STATIC_PROPERTY_READ("H", "i", CDRAW_height),
  GB_STATIC_PROPERTY_READ("Width", "i", CDRAW_width),
  GB_STATIC_PROPERTY_READ("Height", "i", CDRAW_height),
  GB_STATIC_PROPERTY_READ("Resolution", "i", CDRAW_resolution),
  
  GB_STATIC_PROPERTY_SELF("Clip", ".DrawClip"),

  GB_STATIC_PROPERTY("BackColor", "i", CDRAW_background),
  GB_STATIC_PROPERTY("Background", "i", CDRAW_background),
  GB_STATIC_PROPERTY("Transparent", "b", CDRAW_transparent),
  GB_STATIC_PROPERTY("Invert", "b", CDRAW_invert),

  GB_STATIC_PROPERTY("ForeColor", "i", CDRAW_foreground),
  GB_STATIC_PROPERTY("Foreground", "i", CDRAW_foreground),

  GB_STATIC_PROPERTY("LineWidth", "i", CDRAW_line_width),
  GB_STATIC_PROPERTY("LineStyle", "i", CDRAW_line_style),

  GB_STATIC_PROPERTY("FillColor", "i", CDRAW_fill_color),
  GB_STATIC_PROPERTY("FillStyle", "i", CDRAW_fill_style),

  GB_STATIC_PROPERTY("Font", "Font", CDRAW_font),

  GB_STATIC_PROPERTY("FillX", "i", CDRAW_fill_x),
  GB_STATIC_PROPERTY("FillY", "i", CDRAW_fill_y),
  
  GB_STATIC_METHOD("Ellipse", NULL, CDRAW_ellipse, "(X)i(Y)i(Width)i(Height)i[(Start)f(Length)f]"),
  GB_STATIC_METHOD("Line", NULL, CDRAW_line, "(X1)i(Y1)i(X2)i(Y2)i"),

  GB_STATIC_METHOD("Picture", NULL, CDRAW_picture, "(Picture)Picture;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Image", NULL, CDRAW_image, "(Image)Image;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Drawing", NULL, CDRAW_drawing, "(Drawing)Drawing;(X)i(Y)i[(Width)i(Height)i(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),

  GB_STATIC_METHOD("Zoom", NULL, CDRAW_zoom, "(Image)Image;(Zoom)i(X)i(Y)i[(SrcX)i(SrcY)i(SrcWidth)i(SrcHeight)i]"),
  GB_STATIC_METHOD("Tile", NULL, CDRAW_tile, "(Picture)Picture;(X)i(Y)i(Width)i(Height)i"),

  GB_STATIC_METHOD("Point", NULL, CDRAW_point, "(X)i(Y)i"),
  GB_STATIC_METHOD("Rect", NULL, CDRAW_rect, "(X)i(Y)i(Width)i(Height)i"),
  GB_STATIC_METHOD("RoundRect", NULL, CDRAW_round_rect, "(X)i(Y)i(Width)i(Height)i[(Round)f]"),
  GB_STATIC_METHOD("Text", NULL, CDRAW_text, "(Text)s(X)i(Y)i[(Width)i(Height)i(Alignment)i)]"),

  GB_STATIC_METHOD("TextWidth", "i", CDRAW_text_width, "(Text)s"),
  GB_STATIC_METHOD("TextHeight", "i", CDRAW_text_height, "(Text)s"),

  GB_STATIC_METHOD("Polyline", NULL, CDRAW_polyline, "(Points)Integer[]"),
  GB_STATIC_METHOD("Polygon", NULL, CDRAW_polygon, "(Points)Integer[]"),

	GB_STATIC_METHOD("Reset", NULL, CDRAW_reset, NULL),
	GB_STATIC_METHOD("Push", NULL, CDRAW_push, NULL),
	GB_STATIC_METHOD("Pop", NULL, CDRAW_pop, NULL),
	GB_STATIC_METHOD("Translate", NULL, CDRAW_translate, "(DX)f(DY)f"),
	GB_STATIC_METHOD("Scale", NULL, CDRAW_scale, "(SX)f(SY)f"),
	GB_STATIC_METHOD("Rotate", NULL, CDRAW_rotate, "(Angle)f"),

  GB_END_DECLARE
};


