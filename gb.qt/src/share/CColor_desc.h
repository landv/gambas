GB_DESC CColorInfoDesc[] =
{
  GB_DECLARE(".ColorInfo", 0), GB_VIRTUAL_CLASS(),

  GB_STATIC_PROPERTY("Alpha", "i", CCOLOR_info_alpha),
  GB_STATIC_PROPERTY("Red", "i", CCOLOR_info_red),
  GB_STATIC_PROPERTY("Green", "i", CCOLOR_info_green),
  GB_STATIC_PROPERTY("Blue", "i", CCOLOR_info_blue),
  GB_STATIC_PROPERTY("Hue", "i", CCOLOR_info_hue),
  GB_STATIC_PROPERTY("Saturation", "i", CCOLOR_info_saturation),
  GB_STATIC_PROPERTY("Value", "i", CCOLOR_info_value),

  GB_END_DECLARE
};


GB_DESC CColorDesc[] =
{
  GB_DECLARE("Color", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Default", "i", COLOR_DEFAULT),

  GB_CONSTANT("Black", "i", 0),

  GB_CONSTANT("White", "i", 0xFFFFFF),
  GB_CONSTANT("LightGray", "i", 0xC0C0C0),
  GB_CONSTANT("Gray", "i", 0x808080),
  GB_CONSTANT("DarkGray", "i", 0x404040),

  GB_CONSTANT("Blue", "i", 0xFF),
  GB_CONSTANT("DarkBlue", "i", 0x80),

  GB_CONSTANT("Green", "i", 0xFF00L),
  GB_CONSTANT("DarkGreen", "i", 0x8000),

  GB_CONSTANT("Red", "i", 0xFF0000),
  GB_CONSTANT("DarkRed", "i", 0x800000),

  GB_CONSTANT("Cyan", "i", 0xFFFFL),
  GB_CONSTANT("DarkCyan", "i", 0x8080L),

  GB_CONSTANT("Magenta", "i", 0xFF00FF),
  GB_CONSTANT("DarkMagenta", "i", 0x800080),

  GB_CONSTANT("Yellow", "i", 0xFFFF00),
  GB_CONSTANT("DarkYellow", "i", 0x808000),

  GB_CONSTANT("Orange", "i", 0xFF8000),
  GB_CONSTANT("Violet", "i", 0x8000FF),
  GB_CONSTANT("Pink", "i", 0xFF80FF),

  GB_CONSTANT("Transparent", "i", -1),

  GB_STATIC_PROPERTY("Background", "i", CCOLOR_background),
  GB_STATIC_PROPERTY("SelectedBackground", "i", CCOLOR_selected_background),
  GB_STATIC_PROPERTY("LightBackground", "i", CCOLOR_light_background),
  GB_STATIC_PROPERTY("TextBackground", "i", CCOLOR_text_background),
  GB_STATIC_PROPERTY("ButtonBackground", "i", CCOLOR_button_background),

  GB_STATIC_PROPERTY("Foreground", "i", CCOLOR_foreground),
  GB_STATIC_PROPERTY("SelectedForeground", "i", CCOLOR_selected_foreground),
  GB_STATIC_PROPERTY("TextForeground", "i", CCOLOR_text_foreground),
  GB_STATIC_PROPERTY("ButtonForeground", "i", CCOLOR_button_foreground),

  GB_STATIC_METHOD("RGB", "i", CCOLOR_rgb, "(Red)i(Green)i(Blue)i[(Alpha)i]"),
  GB_STATIC_METHOD("HSV", "i", CCOLOR_hsv, "(Hue)i(Saturation)i(Value)i"),
  
  GB_STATIC_METHOD("Lighter", "i", CCOLOR_lighter, "(Color)i"),
  GB_STATIC_METHOD("Darker", "i", CCOLOR_darker, "(Color)i"),
  GB_STATIC_METHOD("Medium", "i", CCOLOR_medium, "(Color1)i(Color2)i"),

  GB_STATIC_METHOD("_get", ".ColorInfo", CCOLOR_get, "(Color)i"),

  GB_END_DECLARE
};


