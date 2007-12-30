GB_DESC CAlignDesc[] =
{
  GB_DECLARE("Align", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("Normal", "i", ALIGN_NORMAL),
  GB_CONSTANT("Left", "i", ALIGN_LEFT),
  GB_CONSTANT("Right", "i", ALIGN_RIGHT),
  GB_CONSTANT("Center", "i", ALIGN_CENTER),

  GB_CONSTANT("TopNormal", "i", ALIGN_TOP_NORMAL),
  GB_CONSTANT("TopLeft", "i", ALIGN_TOP_LEFT),
  GB_CONSTANT("TopRight", "i", ALIGN_TOP_RIGHT),
  GB_CONSTANT("Top", "i", ALIGN_TOP),

  GB_CONSTANT("BottomNormal", "i", ALIGN_BOTTOM_NORMAL),
  GB_CONSTANT("BottomLeft", "i", ALIGN_BOTTOM_LEFT),
  GB_CONSTANT("BottomRight", "i", ALIGN_BOTTOM_RIGHT),
  GB_CONSTANT("Bottom", "i", ALIGN_BOTTOM),

  GB_END_DECLARE
};


GB_DESC CArrangeDesc[] =
{
  GB_DECLARE("Arrange", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", ARRANGE_NONE),
  GB_CONSTANT("Horizontal", "i", ARRANGE_HORIZONTAL),
  GB_CONSTANT("Vertical", "i", ARRANGE_VERTICAL),
  GB_CONSTANT("LeftRight", "i", ARRANGE_ROW),
  GB_CONSTANT("TopBottom", "i", ARRANGE_COLUMN),
  GB_CONSTANT("Row", "i", ARRANGE_ROW),
  GB_CONSTANT("Column", "i", ARRANGE_COLUMN),
  GB_CONSTANT("Fill", "i", ARRANGE_FILL),

  GB_END_DECLARE
};


GB_DESC CBorderDesc[] =
{
  GB_DECLARE("Border", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", BORDER_NONE),
  GB_CONSTANT("Plain", "i", BORDER_PLAIN),
  GB_CONSTANT("Sunken", "i", BORDER_SUNKEN),
  GB_CONSTANT("Raised", "i", BORDER_RAISED),
  GB_CONSTANT("Etched", "i", BORDER_ETCHED),

  GB_END_DECLARE
};


GB_DESC CScrollDesc[] =
{
  GB_DECLARE("Scroll", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", 0),
  GB_CONSTANT("Horizontal", "i", 1),
  GB_CONSTANT("Vertical", "i", 2),
  GB_CONSTANT("Both", "i", 3),

  GB_END_DECLARE
};


GB_DESC CLineDesc[] =
{
  GB_DECLARE("Line", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", LINE_NONE),
  GB_CONSTANT("Solid", "i", LINE_SOLID),
  GB_CONSTANT("Dash", "i", LINE_DASH),
  GB_CONSTANT("Dot", "i", LINE_DOT),
  GB_CONSTANT("DashDot", "i", LINE_DOT_LINE),
  GB_CONSTANT("DashDotDot", "i", LINE_DASH_DOT_DOT),

  GB_END_DECLARE
};


GB_DESC CFillDesc[] =
{
  GB_DECLARE("Fill", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", FILL_NONE),
  GB_CONSTANT("Solid", "i", FILL_SOLID),
  GB_CONSTANT("Dense94", "i", FILL_DENSE_94),
  GB_CONSTANT("Dense88", "i", FILL_DENSE_88),
  GB_CONSTANT("Dense63", "i", FILL_DENSE_63),
  GB_CONSTANT("Dense50", "i", FILL_DENSE_50),
  GB_CONSTANT("Dense37", "i", FILL_DENSE_37),
  GB_CONSTANT("Dense12", "i", FILL_DENSE_12),
  GB_CONSTANT("Dense6", "i", FILL_DENSE_06),
  GB_CONSTANT("Horizontal", "i", FILL_HORIZONTAL),
  GB_CONSTANT("Vertical", "i", FILL_VERTICAL),
  GB_CONSTANT("Cross", "i", FILL_CROSS),
  GB_CONSTANT("Diagonal", "i", FILL_DIAGONAL),
  GB_CONSTANT("BackDiagonal", "i", FILL_BACK_DIAGONAL),
  GB_CONSTANT("CrossDiagonal", "i", FILL_CROSS_DIAGONAL),

  GB_END_DECLARE
};


GB_DESC CSelectDesc[] =
{
  GB_DECLARE("Select", 0), GB_VIRTUAL_CLASS(),

  GB_CONSTANT("None", "i", SELECT_NONE),
  GB_CONSTANT("Single", "i", SELECT_SINGLE),
  GB_CONSTANT("Multiple", "i", SELECT_MULTIPLE),

  GB_END_DECLARE
};

