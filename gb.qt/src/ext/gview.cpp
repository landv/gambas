/***************************************************************************

  gview.cpp

  (c) 2000-2007 Benoit Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __G_VIEW_CPP

#include <qpainter.h>
#include <qscrollbar.h>
#include <qclipboard.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qdict.h>
#include <qcursor.h>
#include <qstyle.h>
#include <qcstring.h>
#include <qstring.h>

#include <ctype.h>

#include "main.h"
#include "gview.h"

#if 0
static const char *breakpoint_xpm[] = 
{
"8 8 3 1",
"  c None",
". c #FF7F7F",
"+ c #FF0000",
" .++++. ",
".++++++.",
"++++++++",
"++++++++",
"++++++++",
"++++++++",
".++++++.",
" .++++. "
};
#endif

static QColor defaultColors[GLine::NUM_STATE] =
{
	Qt::white,
	Qt::black,
	QColor(0x00, 0x80, 0xFF),
	Qt::blue,
	Qt::blue,
	Qt::black,
	Qt::red,
	Qt::magenta,
	Qt::gray,
	QColor(0xD0, 0x00, 0x00),
	QColor(0x00, 0x80, 0xFF),
	QColor(0x00, 0x80, 0xFF),
	QColor(0xC0, 0xC0, 0xFF),
	QColor(0xFF, 0xFF, 0x00),
	QColor(0xE8, 0xE8, 0xF8),
	Qt::red
};

/**---- GEditor -----------------------------------------------------------*/

QPixmap *GEditor::cache = 0;
QPixmap *GEditor::breakpoint = 0;
int GEditor::count = 0;

GEditor::GEditor(QWidget *parent) : QGridView(parent, 0, WNoAutoErase), fm(font())
{
	int i;

	if (count == 0)
		cache = new QPixmap();

	count++;

	setNumCols(1);
	setKeyCompression(true);
	setFocusPolicy(WheelFocus);
	setPaletteBackgroundColor(defaultColors[GLine::Background]);
	setInputMethodEnabled(true);
	
	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	viewport()->setCursor(ibeamCursor);
	//viewport()->setWFlags(WRepaintNoErase);

	x = y = xx = 0;
	x1m = x2m = 0;
	ym = -1;
	lastx = -1;
	cursor = false;
	margin = 0;
	lineNumberLength = 0;
	doc = 0;
	center = false;
	setDocument(NULL);
	largestLine = 0;
	flashed = false;
	painting = false;

	for (i = 0; i < GLine::NUM_STATE; i++)
	{
		styles[i].color = defaultColors[i];
		styles[i].bold = i == GLine::Keyword;
		styles[i].italic = i == GLine::Comment;
		styles[i].underline = i == GLine::Error;
	}

	flags = 0;
	//setFlag(ShowCurrentLine, true);
	setFont(QFont("monospace", QApplication::font().pointSize()));
	updateHeight();

	blinkTimer = new QTimer(this);
	connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blinkTimerTimeout()));
	//startBlink();

	scrollTimer = new QTimer(this);
	connect(scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimerTimeout()));

	connect(this, SIGNAL(contentsMoving(int, int)), this, SLOT(baptizeVisible(int, int)));
}

GEditor::~GEditor()
{
	doc->unsubscribe(this);
	count--;
	if (count == 0)
	{
		delete cache;
		delete breakpoint;
		cache = 0;
		breakpoint = 0;
	}
}

void GEditor::setBreakpointPixmap(QPixmap *p)
{
	if (!breakpoint)
		breakpoint = new QPixmap;
		
	*breakpoint = *p;
}

void GEditor::setDocument(GDocument *d)
{
	if (doc)
	{
		doc->unsubscribe(this);
		//disconnect(doc, 0, this, 0);
	}

	doc = d;
	if (!doc)
		doc = new GDocument;

	doc->subscribe(this);
	findLargestLine();
	//connect(doc, SIGNAL(textChanged()), this, SLOT(docTextChanged()));
}

int GEditor::getCharWidth() const
{
	return fm.width('m');
}

void GEditor::updateCache()
{
	if (cache->width() < visibleWidth() || cache->height() < cellHeight())
	{
		int nw = QMAX(cache->width(), visibleWidth());
		int nh = cellHeight(); //QMAX(cache->height(), cellHeight());
		//qDebug("cache->resize(%d, %d)", nw, nh);
		cache->resize(nw, nh);
	}
}

int GEditor::lineWidth(int y) const
{
	// Add 2 so that we see the cursor at end of line
	return margin + fm.width(doc->lines.at(y)->s.getString()) + 2;
}

int GEditor::lineWidth(int y, int len) const
{
	if (len <= 0)
		return margin;
	else
		return margin + fm.width(doc->lines.at(y)->s.getString(), len);
}

int GEditor::posToColumn(int y, int px) const
{
	int i, lw, lw2;
	int len = doc->lineLength(y);
	
	if (len == 0)
		return 0;
	
	lw = 0;
	for (i = 0; i < len; i++)
	{
		lw2 = lineWidth(y, i + 1);
		if (px < ((lw + lw2) / 2))
			return i;
		lw = lw2;
	}
	
	return len;
}

int GEditor::findLargestLine()
{
	int w, mw = 0;
	
	//qDebug("search largest line...");
	
	for (int y = 0; y < doc->numLines(); y++)
	{
		w = lineWidth(y);
		if (w > mw)
		{
			mw = w;
			largestLine = y;
		}
	}
	
	return mw;
}

void GEditor::updateWidth(int y)
{
	int w;
	
	if (largestLine < 0 || largestLine >= numLines())
	{
		findLargestLine();
		y = -1;
	}
	
	if (y < 0)
	{
		w = lineWidth(largestLine);
		goto UPDATE_WIDTH;
	}
	else
	{
		w = lineWidth(y);
	
		//qDebug("contentsWidth() = %d  cellWidth() = %d", contentsWidth(), cellWidth());
		//setCellWidth(QMAX(visibleWidth(), margin + charWidth * doc->getMaxLineLength() + 2));
		
		if (w > cellWidth())
		{
			largestLine = y;
			goto UPDATE_WIDTH;
		}
		else if (w < cellWidth() && y == largestLine)
		{
			w = findLargestLine();
			goto UPDATE_WIDTH;
		}
	}
	
	return;
	
UPDATE_WIDTH:
	
	w = QMAX(visibleWidth(), w);
	if (w != cellWidth())
	{
		setCellWidth(w);
		//qDebug("setCellWidth: %d (largestLine = %d)", w, largestLine);
	}
	updateCache();
}

void GEditor::updateHeight()
{
	setCellHeight(fm.lineSpacing());
	updateCache();

	if (pattern.height() < cellHeight())
		pattern.resize(16, cellHeight());
}

void GEditor::redrawContents()
{
	updateContents();
	if (contentsHeight() < visibleHeight())
		repaintContents(contentsX(), contentsHeight(), visibleWidth(), visibleHeight() - contentsHeight() + contentsX(), true);
}

void GEditor::fontChange(const QFont &oldFont)
{
	QGridView::fontChange(oldFont);
	
	fm = fontMetrics();
	
	italicFont = font();
	italicFont.setItalic(true);
	
	updateMargin();
	updateHeight();
	updateContents();
}

void GEditor::paintText(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int row)
{
	int i;
	int len, style, pos;
	QString sd;
	GHighlightStyle *st;
	bool italic = false;
	int nx;

	pos = 0;
	p.setFont(font());

	for (i = 0; i < GB.Count(l->highlight); i++)
	{
		if (pos > (xmin + lmax))
			break;

		style = l->highlight[i].state;
		len = l->highlight[i].len;
		st = &styles[style];
		
		nx = lineWidth(row, pos + len);

		if ((pos + len) >= xmin)
		{
			sd = l->s.mid(pos, len).getString();

			/*if (style == GLine::Keyword)
			{
				if (getFlag(DrawWithRelief))
				{
					p.setPen(styles[GLine::Normal].color);
					p.drawText(x + 1, y + 1, sd);
					p.setPen(styles[style].color);
					p.drawText(x, y, sd);
				}
				else
				{
					p.setPen(styles[style].color);
					p.drawText(x, y, sd);
					p.drawText(x + 1, y, sd);
				}
			}
			else if (style == GLine::Error)
			{
				p.setPen(styles[GLine::Current].color);
				p.drawLine(x, y + 2, x + len * charWidth - 1, y + 2);
				p.setPen(styles[GLine::Normal].color);
				p.drawText(x, y, sd);
			}
			else
			{
				p.setPen(styles[style].color);
				p.drawText(x, y, sd);
			}*/

			if (st->italic != italic)
			{
				italic = st->italic;
				p.setFont(italic ? italicFont : font());
			}

			p.setPen(styles[style].color);
			p.drawText(x, y, sd);

			if (st->bold)
				p.drawText(x + 1, y, sd);

			if (st->underline)
				p.drawLine(x, y + 2, nx - 1, y + 2);
		}

		pos += len;
		x  = nx; //+= len * charWidth;
	}

	if (pos < (int)l->s.length() && pos < (xmin + lmax))
	{
		p.setPen(styles[GLine::Normal].color);
		p.drawText(x, y, l->s.mid(pos).getString());
	}
}

static void make_blend(QPixmap &pix, QColor start, QColor end, int height) //, bool loop = false)
{
	double r, g, b, dr, dg, db;
	int n = height * 3 / 4;
	int i;
	QPainter p;

	pix.fill(end);

	r = start.red();
	g = start.green();
	b = start.blue();

	if (n == 0)
		n = 1;

	dr = (end.red() - r) / n;
	dg = (end.green() - g) / n;
	db = (end.blue() - b) / n;

	p.begin(&pix);

	//if (loop)
	//	n >>= 1;

	for (i = 0; i < n; i++)
	{
		QBrush brush(QColor((int)r, (int)g, (int)b));
		p.fillRect(0, i, pix.width(), 1, brush);
		//if (loop)
		//  p.fillRect(0, n - i, pix.width(), 1, brush);
		r += dr; g += dg; b += db;
	}

	p.end();
}

/*static QColor blend_color(QColor a, QColor b, QColor r)
{
	#define f(c) ((a.c()*r.c() + (255 - a.c())*(255 - r.c())) * b.c() / 255 / 255)
	return QColor(f(red), f(green), f(blue));
}*/

static QColor calc_color(QColor ca, QColor cb, QColor cd)
{
	int r = 1, g = 1, b = 1, n = 0;

	#define test(x) if (x.isValid()) { r *= x.red(); g *= x.green(); b *= x.blue(); n++; }

	test(ca);
	test(cb);
	//test(cc);

	if (n == 0)
		return cd;
	else
	{
		n = (n == 2) ? 255 : (n == 3) ? 255*255 : 1;
		return QColor(r / n, g / n, b / n);
	}
}

static void highlight_text(QPainter &p, int x, int y, QString s, QColor color)
{
	int i, j;
	
	p.setPen(color);
	
	for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++)
			p.drawText(x + i, y + j, s);
}

void GEditor::paintCell(QPainter * painter, int row, int)
{
	QRect ur;
	GLine *l;
	int x1, y1, x2, y2;
	QColor color, a, b, c;
	int xmin, lmax;

	ur = cellGeometry(row, 0);
	contentsToViewport(ur.x(), ur.y(), x1, y1);
	ur.setRect(-x1, y1, ur.width(), ur.height());

	if (row >= numLines())
	{
		/*QColor color = styles[GLine::Background].color;
		if (flashed)
			color = QColor(QRgb(color.rgb() ^ 0x00FFFFFF));
		painter->fillRect(0, 0, ur.width(), ur.height(), styles[GLine::Background].color);*/
		return;
	}

	l = doc->lines.at(row);

	//xmin = (ur.left() - margin) / charWidth;
	//if (xmin < 0)
	//	xmin = 0;
	xmin = posToColumn(row, ur.left());
	//lmax = 2 + visibleWidth() / charWidth;
	lmax = 2 + posToColumn(row, ur.left() + visibleWidth()) - xmin;
	
	//if (row == 0)
	//	qDebug("%d: %d %d %d %d (%d %d)", row, ur.left(), ur.top(), ur.width(), ur.height(), xmin, lmax);

	// Line background

	if (l->flag & (1 << GLine::CurrentFlag))
		a = styles[GLine::Current].color;

	if (getFlag(ShowCurrentLine) && row == y)
		b = styles[GLine::Line].color;

	QPainter p(cache);
	
	color = calc_color(a, b, styles[GLine::Background].color);
	p.fillRect(0, 0, cellWidth(), cellHeight(), color);
	
	p.setFont(painter->font());
	//p.translate(-ur.left(), 0);

	// Procedure separation
	if (getFlag(ShowProcedureLimits) && l->proc)
	{
		if (getFlag(BlendedProcedureLimits))
		{
			make_blend(pattern, styles[GLine::Line].color, color, cellHeight());
			p.drawTiledPixmap(0, 0, cache->width(), cache->height(), pattern);
		}
		else
		{
			QBrush brush(styles[GLine::Selection].color, Qt::Dense4Pattern);
			p.fillRect(0, 0, cache->width(), 1, brush);
		}
	}

	p.translate(-ur.left(), 0);

	// Selection background
	if (doc->hasSelection())
	{
		doc->getSelection(&y1, &x1, &y2, &x2);

		if (row >= y1 && row <= y2 && !(row == y2 && x2 == 0))
		{
			if (row > y1 || x1 == 0)
				x1 = 0;
			else
				//x1 *= charWidth;
				x1 = lineWidth(y1, x1);

			if (row < y2)
				x2 = cellWidth() + 1;
			else
				//x2 *= charWidth;
				x2 = lineWidth(y2, x2);

			p.fillRect(x1, 0, x2 - x1, cellHeight(), styles[GLine::Selection].color);
		}
	}

	// Margin
	if (margin && (margin > ur.left()))
	{
		//if (!l->flag)
		//	p.fillRect(0, 0, margin, cellHeight(), color); //styles[GLine::Background].color);

		if (getFlag(ShowModifiedLines) && l->changed)
			p.fillRect(0, 0, margin - 2, cellHeight(), styles[GLine::Highlight].color);
		else //if (getFlag(ShowLineNumbers))
			p.fillRect(0, 0, margin - 2, cellHeight(), styles[GLine::Line].color);

		x1 = 0;

		if (getFlag(ShowLineNumbers))
		{
			int n = row + 1;
			if ((n % 10) == 0)
				p.setPen(styles[GLine::Normal].color);
			else
				p.setPen(styles[GLine::Selection].color);
			p.drawText(x1 + 2, fm.ascent(), QString::number(n).rightJustify(lineNumberLength));
		}
	}
	
	// Breakpoint symbol
	if ((l->flag & (1 << GLine::BreakpointFlag)) && breakpoint && !breakpoint->isNull())
	{
		//p.fillRect(margin - 10, 0, 8, cellHeight(), styles[GLine::Breakpoint].color);
		//updateBreakpoint(styles[GLine::Background].color.rgb(), styles[GLine::Breakpoint].color.rgb());
		p.drawPixmap(margin - (cellHeight() + breakpoint->width()) / 2, (cellHeight() - breakpoint->height()) / 2, *breakpoint);
	}

	// Highlight braces
	if (getFlag(HighlightBraces) && row == ym && x1m >= 0)
	{
		//highlight_text(p, x1m * charWidth + margin, fm.ascent(), l->s.getString().mid(x1m, 1), styles[GLine::Highlight].color);
		//highlight_text(p, x2m * charWidth + margin, fm.ascent(), l->s.getString().mid(x2m, 1), styles[GLine::Highlight].color);
		highlight_text(p, lineWidth(ym, x1m), fm.ascent(), l->s.getString().mid(x1m, 1), styles[GLine::Highlight].color);
		highlight_text(p, lineWidth(ym, x2m), fm.ascent(), l->s.getString().mid(x2m, 1), styles[GLine::Highlight].color);
		/*p.fillRect(x1m * charWidth + margin, 0, charWidth, cellHeight(), styles[GLine::Highlight].color);
		p.fillRect(x2m * charWidth + margin, 0, charWidth, cellHeight(), styles[GLine::Highlight].color);*/
	}

	// Line text
	if (doc->getHighlightMode() == GDocument::None || (l->modified && row == y && !getFlag(HighlightCurrent)))
	{
		p.setPen(styles[GLine::Normal].color);
		//p.drawText(margin + xmin * charWidth, fm.ascent(), l->s.getString().mid(xmin, lmax));
		p.drawText(lineWidth(row, xmin), fm.ascent(), l->s.getString().mid(xmin, lmax));
	}
	/*else if (l->flag)
	{
		p.setPen(styles[GLine::Background].color);
		p.drawText(margin, fm.ascent(), l->s.getString().mid(xmin, lmax));
	}*/
	else
	{
		painting = true;
		doc->colorize(row);
		painting = false;
		paintText(p, l, margin, fm.ascent(), xmin, lmax, row);
	}

	// Text cursor
	if (cursor && row == y)
		//p.fillRect(QMIN((int)l->s.length(), x) * charWidth + margin, 0, 1, cellHeight(), styles[GLine::Normal].color);
		p.fillRect(lineWidth(row, QMIN((int)l->s.length(), x)), 0, 1, cellHeight(), styles[GLine::Normal].color);

	// Flash
	if (flashed)
	{
		p.setRasterOp(XorROP);
		p.fillRect(0, 0, visibleWidth(), cellHeight(), Qt::white);
	}

	p.end();

	//if (cache->width() < visibleWidth())
	//	qDebug("cache->width() = %d < visibleWidth() = %d", cache->width(), visibleWidth());
	painter->drawPixmap(ur.left(), 0, *cache, 0, 0, cellWidth(), cellHeight());
}


void GEditor::checkMatching()
{
	static GString look("()[]{}");

	if (y < 0 || y >= (int)numLines())
		return;

	GString str = doc->lines.at(y)->s;
	int len = (int)str.length();

	char c, cr;
	int pos, d, l, bx, by, bx2;
	char s[len];
	bool ignore;
	int i;

	bx = -1;
	bx2 = -1;
	by = -1;
	ignore = false;

	if (len == 0)
		goto __OK;

	pos = -1;

	for (i = 0; i < len; i++)
	{
		c = str.at(i);

		if (ignore)
		{
			if (c == '\\')
			{
				s[i] = c;
				i++;
				s[i] = str.at(i);
				continue;
			}

			if (c == '"')
				ignore = false;
			else
				c = ' ';
		}
		else
		{
			if (c == '"')
				ignore = true;
		}

		s[i] = c;
	}

	c = 0;

	if (x > 0)
	{
		bx2 = x - 1;
		c = s[bx2];
		pos = look.find(c);
	}

	if (pos < 0 && x < len)
	{
		bx2 = x;
		c = s[bx2];
		pos = look.find(c);
	}

	if (c && pos >= 0)
	{
		d = (pos & 1) ? -1 : 1;
		cr = look.at(pos + d);

		pos = bx2;
		l = 0;
		for(;;)
		{
			pos += d;
			if (pos < 0 || pos >= len)
				break;

			if (s[pos] == c)
				l++;
			else if (s[pos] == cr)
			{
				l--;
				if (l < 0)
				{
					bx = pos;
					by = y;
					break;
				}
			}
		}
	}

__OK:

	if (by != ym || bx != x1m || bx2 != x2m)
	{
		if (ym >= 0)
			updateLine(ym);
		ym = by;
		x1m = bx;
		x2m = bx2;
		if (ym >= 0)
			updateLine(ym);
	}
}


bool GEditor::cursorGoto(int ny, int nx, bool mark)
{
	bool setxx;
	bool change = false;

	if (!mark)
	{
		if (doc->hasSelection())
			doc->hideSelection();
	}

	setxx = xx != nx;

	if (ny < 0)
	{
		nx = QMAX(0, nx);
		ny = 0;
	}
	else if (ny > (numLines() - 1))
	{
		ny = numLines() - 1;
		nx = QMIN(nx, lineLength(ny));
	}
	else if (ny == y)
	{
		if (nx < 0 && ny > 0)
		{
			ny--;
			nx = lineLength(ny);
		}
		else if (nx > lineLength(ny) && ny < (numLines() - 1))
		{
			ny++;
			nx = 0;
		}
	}

	if (nx < 0)
		nx = 0;
	else if (nx > lineLength(ny))
		nx = lineLength(ny);

	if (ny != y && getFlag(HighlightCurrent))
		doc->colorize(y);

	if (y != ny || x != nx)
	{
		int oy = y;
		
		if (mark)
		{
			if (!doc->hasSelection(this))
				doc->startSelection(this, y, x);
		}
		
		y = ny;
		x = nx;
		if (setxx)
			xx = x;
		cursor = hasFocus();
		change = true;

		repaintLine(oy);
		repaintLine(y);
		
		ensureCursorVisible();
		
		if (mark)
			doc->endSelection(y, x);

		emit cursorMoved();
	}

	checkMatching();

		//QTimer::singleShot(30, this, SLOT(ensureCursorVisible()));

	return change;
}

void GEditor::insert(QString text)
{
	doc->eraseSelection();
	doc->insert(y, x, text);
	cursorGoto(doc->yAfter, doc->xAfter, false);
}

void GEditor::cursorLeft(bool shift, bool ctrl)
{
	if (ctrl && x > 0)
	{
		int nx = doc->wordLeft(y, x);
		cursorGoto(y, nx, shift);
	}
	else
		cursorGoto(y, x - 1, shift);
}


void GEditor::cursorRight(bool shift, bool ctrl)
{
	if (ctrl && x < lineLength(y))
	{
		int nx = doc->wordRight(y, x);
		cursorGoto(y, nx, shift);
	}
	else
		cursorGoto(y, x + 1, shift);
}

void GEditor::cursorUp(bool shift, bool ctrl)
{
	if (!ctrl)
	{
		cursorGoto(y - 1, xx, shift);
		return;
	}

	int yy = y;
	for(;;)
	{
		yy--;
		if (yy <= 0)
			break;
		if (doc->lines.at(yy)->proc)
			break;
	}

	cursorGoto(yy, xx, shift);
}


void GEditor::cursorDown(bool shift, bool ctrl)
{
	if (!ctrl)
	{
		cursorGoto(y + 1, xx, shift);
		return;
	}

	int yy = y;
	for(;;)
	{
		yy++;
		if (yy >= numLines())
			break;
		if (doc->lines.at(yy)->proc)
			break;
	}

	cursorGoto(yy, xx, shift);
}

void GEditor::cursorHome(bool shift, bool ctrl)
{
	if (ctrl)
		cursorGoto(0, 0, shift);
	else
	{
		int indent = doc->getIndent(y);
		if (x == indent)
			cursorGoto(y, 0, shift);
		else
			cursorGoto(y, indent, shift);
	}
}


void GEditor::cursorEnd(bool shift, bool ctrl)
{
	if (ctrl)
		cursorGoto(numLines() - 1, lineLength(numLines() - 1), shift);
	else
		cursorGoto(y, lineLength(y), shift);
}

void GEditor::cursorPageUp(bool mark)
{
	int page = visibleHeight() / cellHeight();
	cursorGoto(y - page, 0, mark);
}

void GEditor::cursorPageDown(bool mark)
{
	int page = visibleHeight() / cellHeight();
	cursorGoto(y + page, 0, mark);
}

void GEditor::newLine()
{
	doc->eraseSelection();
	doc->insert(y, x, '\n' + doc->lines.at(y)->s.left(QMIN(x, doc->getIndent(y))));
	cursorGoto(doc->yAfter, doc->xAfter, false);
}

void GEditor::del(bool ctrl)
{
	if (doc->hasSelection())
	{
		doc->eraseSelection();
		return;
	}

	if (x == lineLength(y))
	{
		if (y < (numLines() - 1))
			doc->remove(y, x, y + 1, 0);
	}
	else
	{
		if (ctrl && x < lineLength(y))
		{
			int nx = doc->wordRight(y, x);
			doc->remove(y, x, y, nx);
		}
		else
			doc->remove(y, x, y, x + 1);
	}
}

void GEditor::backspace(bool ctrl)
{
	int indent;
	int yy;
	bool empty;

	if (doc->hasSelection())
	{
		doc->eraseSelection();
		return;
	}

	indent = doc->getIndent(y);

	if (x > 0 && x <= indent)
	{
		yy = y;
		indent = 0;
		while (yy > 0)
		{
			yy--;
			indent = doc->getIndent(yy, &empty);
			if (!empty && x > indent)
				break;
		}
		cursorGoto(y, indent, true);
		del(false);
	}
	else
	{
		if (ctrl && x > 0)
		{
			int nx = doc->wordLeft(y, x);
			doc->remove(y, nx, y, x);
		}
		else
		{
			if (cursorGoto(y, x - 1, false))
				del(false);
		}
	}
}

void GEditor::tab(bool back)
{
	QString ins;
	int y1, x1, y2, x2, yy;
	int indent, i;
	bool empty;
	int tw = doc->getTabWidth();

	if (!doc->hasSelection())
	{
		if (!back)
		{
			ins.fill(' ', tw - (x % tw));
			insert(ins);
		}
		return;
	}

	doc->getSelection(&y1, &x1, &y2, &x2);
	doc->startSelection(this, y1, 0);
	if (x2)
		y2++;
	doc->endSelection(y2, 0);

	indent = 65536;
	for (yy = y1; yy < y2; yy++)
	{
		i = doc->getIndent(yy, &empty);
		if (!empty)
			indent = QMIN(indent, i);
	}

	if (back && indent <= 0)
		return;

	doc->begin();

	if (!back)
	{
		ins.fill(' ', tw - (indent % tw));

		for (yy = y1; yy < y2; yy++)
		{
			doc->insert(yy, 0, ins);
			doc->colorize(yy);
		}
	}
	else
	{
		indent %= tw;
		if (indent == 0)
			indent = tw;

		ins.fill(' ', indent);

		for (yy = y1; yy < y2; yy++)
		{
			if (doc->lines.at(yy)->s.left(indent) == ins)
			{
				doc->remove(yy, 0, yy, indent);
				doc->colorize(yy);
			}
		}
	}

	doc->startSelection(this, y1, 0);
	doc->endSelection(y2, 0);

	doc->end();
}


void GEditor::copy(bool mouse)
{
	if (!doc->hasSelection())
		return;

	QString text = doc->getSelectedText().getString();
	QApplication::clipboard()->setText(text, mouse ? QClipboard::Selection : QClipboard::Clipboard);
}

void GEditor::cut()
{
	copy(false);
	doc->eraseSelection();
}

void GEditor::paste(bool mouse)
{
	QString text;
	QCString subType = "plain";
	uint i;
	QString tab;

	text = QApplication::clipboard()->text(subType, mouse ? QClipboard::Selection : QClipboard::Clipboard);

	tab.fill(' ', doc->getTabWidth());
	text.replace("\t", tab);

	for (i = 0; i < text.length(); i++)
	{
		if ((text[i] < ' ' || text[i].isSpace()) && text[i] != '\n')
			text[i] = ' ';
	}

	doc->begin();
	insert(text);
	doc->end();
}

void GEditor::undo()
{
	if (!doc->undo())
		cursorGoto(doc->yAfter, doc->xAfter, false);
}

void GEditor::redo()
{
	if (!doc->redo())
		cursorGoto(doc->yAfter, doc->xAfter, false);
}

void GEditor::selectAll()
{
	cursorGoto(0, 0, false);
	cursorGoto(numLines() - 1, lineLength(numLines() - 1), true);
}

void GEditor::keyPressEvent(QKeyEvent *e)
{
	bool shift = e->state() & ShiftButton;
	bool ctrl = e->state() & ControlButton;

	e->accept();

	if (doc->isReadOnly())
	{
		switch (e->key())
		{
			case Key_Left:
				cursorLeft(shift, ctrl); return;
			case Key_Right:
				cursorRight(shift, ctrl); return;
			case Key_Up:
				cursorUp(shift, ctrl); return;
			case Key_Down:
				cursorDown(shift, ctrl); return;
			case Key_Home:
				cursorHome(shift, ctrl); return;
			case Key_End:
				cursorEnd(shift, ctrl); return;
			case Key_Prior:
				cursorPageUp(shift); return;
			case Key_Next:
				cursorPageDown(shift); return;
		}

		if (!ctrl)
			goto __IGNORE;

		switch(e->key())
		{
			case Key_C:
				copy(); return;
			case Key_A:
				selectAll(); return;
		}
	}
	else
	{
		if (e->text().length()
				&& e->key() != Key_Return
				&& e->key() != Key_Enter
				&& e->key() != Key_Delete
				&& e->key() != Key_Backspace
				&& (!e->ascii() || e->ascii() >= 32))
		{
			insert(e->text());
			return;
		}

		switch (e->key())
		{
			case Key_Left:
				cursorLeft(shift, ctrl); return;
			case Key_Right:
				cursorRight(shift, ctrl); return;
			case Key_Up:
				cursorUp(shift, ctrl); return;
			case Key_Down:
				cursorDown(shift, ctrl); return;
			case Key_Enter: case Key_Return:
				newLine(); return;
			case Key_Home:
				cursorHome(shift, ctrl); return;
			case Key_End:
				cursorEnd(shift, ctrl); return;
			case Key_Backspace:
				backspace(ctrl); return;
			case Key_Delete:
				del(ctrl); return;
			case Key_Prior:
				cursorPageUp(shift); return;
			case Key_Next:
				cursorPageDown(shift); return;
			case Key_Tab:
				tab(false); return;
			case Key_BackTab:
				tab(true); return;
		}

		if (!ctrl)
			goto __IGNORE;

		switch(e->key())
		{
			case Key_C:
				copy(); return;
			case Key_X:
				cut(); return;
			case Key_V:
				paste(); return;
			case Key_Z:
				undo(); return;
			case Key_Y:
				redo(); return;
			case Key_A:
				selectAll(); return;
		}
	}

__IGNORE:

	e->ignore();
}


int GEditor::posToLine(int py) const
{
	int ny;

	ny = rowAt(contentsY() + py);
	if (ny < 0)
		ny = 0;
	else if (ny >= numLines())
		ny = numLines() - 1;
		
	return ny;
}

void GEditor::posToCursor(int px, int py, int *y, int *x) const
{
	int nx, ny;

	ny = posToLine(py);
	
	//nx = (contentsX() + px - margin) / charWidth;
	//nx = QMAX(0, QMIN(nx, lineLength(ny)));
	nx = posToColumn(ny, px);

	*y = ny;
	*x = nx;
}

void GEditor::cursorToPos(int y, int x, int *px, int *py) const
{
	int npx, npy;

	npy = y * cellHeight() - contentsY();
	//npx = x * charWidth - contentsX() + margin;
	npx = lineWidth(y, x) - contentsX();

	*py = npy;
	*px = npx;
}

bool GEditor::updateCursor()
{
	if (contentsX() + lastx >= margin)
	{
		viewport()->setCursor(Qt::IbeamCursor);
		return false;
	}
	else
	{
		viewport()->setCursor(Qt::ArrowCursor);  
		return true;
	}
}

void GEditor::mousePressEvent(QMouseEvent *e)
{
	bool shift = e->state() & ShiftButton;
	int nx, ny;
	//stopAutoScroll();
	//d->dnd_startpos = e->pos();

	if (e->button() != LeftButton && e->button() != MidButton)
		return;

	posToCursor(e->pos().x(), e->pos().y(), &ny, &nx);
	
	lastx = e->pos().x();
	left = updateCursor();
	
	if (!left)
		cursorGoto(ny, nx, shift);
}

void GEditor::mouseMoveEvent(QMouseEvent *e)
{
	if ((e->state() & Qt::MouseButtonMask) == LeftButton)
	{
		if (left)
		{
			//ly = posToLine(e->pos().y());
			if (!scrollTimer->isActive())
				cursorGoto(posToLine(e->pos().y()), 0, false);
		}
		
		if (!scrollTimer->isActive())
		{
			stopBlink();
			scrollTimer->start(25, false);
		}
	}

	lastx = e->pos().x();
	left = updateCursor();
}

void GEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (left)
	{
		emit marginDoubleClicked(posToLine(e->pos().y()));
		return;
	}
	
	if (e->button() == LeftButton && !(e->state() & ShiftButton))
	{
		int xl, xr;

		xl = doc->wordLeft(y, x, true);
		xr = doc->wordRight(y, x, true);

		if (xr > xl)
		{
			doc->hideSelection();
			cursorGoto(y, xl, false);
			cursorGoto(y, xr, true);
			copy(true);
		}
	}
}

void GEditor::mouseReleaseEvent(QMouseEvent *e)
{
	if (scrollTimer->isActive())
	{
		scrollTimer->stop();
		startBlink();
		copy(true);
		return;
	}

	if (left)
		emit marginClicked(posToLine(e->pos().y()));

	if (e->button() == MidButton)
		paste(true);
}


void GEditor::resizeEvent(QResizeEvent *e)
{
	QGridView::resizeEvent(e);
	baptizeVisible();
	updateWidth(-1);
}

bool GEditor::isCursorVisible() const
{
	int px, py;
	
	cursorToPos(y, x, &px, &py);
	
	return !(px < margin || px >= (visibleWidth() - 2) || py < 0 || py >= (visibleHeight() - cellHeight() - 1));
}

void GEditor::ensureCursorVisible()
{
	if (!isUpdatesEnabled())
		return;
	
	if (!isCursorVisible())
	{
		if (center)
			//ensureVisible(x * charWidth, y * cellHeight() + cellHeight() / 2, margin + 2, visibleHeight() / 2);
			ensureVisible(lineWidth(y, x) + getCharWidth() / 2, y * cellHeight() + cellHeight() / 2, margin + 2, visibleHeight() / 2);
		else
			//ensureVisible(x * charWidth, y * cellHeight() + cellHeight() / 2, margin + 2, cellHeight());
			ensureVisible(lineWidth(y, x) + getCharWidth() / 2, y * cellHeight() + cellHeight() / 2, margin + 2, cellHeight());
	}
	center = false;
}

void GEditor::startBlink()
{
	blinkTimer->start(QApplication::cursorFlashTime() / 2, false);
	cursor = true;
	updateLine(y);
}

void GEditor::stopBlink()
{
	blinkTimer->stop();
	cursor = false;
	updateLine(y);
}

void GEditor::blinkTimerTimeout()
{
	cursor = !cursor;
	updateLine(y);
}

void GEditor::focusInEvent(QFocusEvent *e)
{
	startBlink();
	ensureCursorVisible();
	QGridView::focusInEvent(e);
}

void GEditor::focusOutEvent(QFocusEvent *e)
{
	stopBlink();
	QGridView::focusOutEvent(e);
}

void GEditor::scrollTimerTimeout()
{
	int ny, nx;
	QPoint pos = mapFromGlobal(QCursor::pos());

	posToCursor(pos.x(), pos.y(), &ny, &nx);
	cursorGoto(ny, nx, true);
}

void GEditor::setStyle(int index, GHighlightStyle *style)
{
	if (index < 0 || index >= GLine::NUM_STATE)
		return;

	styles[index] = *style;

	if (index == 0)
	{
		setPaletteBackgroundColor(style->color);
		redrawContents();
	}
	else
		updateContents();
}

void GEditor::setNumRows(int n)
{
	QGridView::setNumRows(n);
	QGridView::updateScrollBars();
	if (contentsHeight() < visibleHeight())
		repaintContents(contentsX(), contentsHeight(), visibleWidth(), visibleHeight() - contentsHeight() + contentsX(), true);
	//if (contentsHeight() < visibleHeight())
		//repaintContents(true);
}

void GEditor::getStyle(int index, GHighlightStyle *style)
{
	if (index < 0 || index >= GLine::NUM_STATE)
	{
		*style = styles[GLine::Background];
		return;
	}

	*style = styles[index];
}

void GEditor::setFlag(int f, bool v)
{
	if (v)
		flags |= (1 << f);
	else
		flags &= ~(1 << f);

	updateMargin();
	updateContents();
}

void GEditor::updateMargin()
{
	int charWidth = getCharWidth();
	int nm, lnl = 0;
	
	nm = 2;
	
	if (doc->getHighlightMode() != GDocument::None)
	{
		if (breakpoint && !breakpoint->isNull())
			nm += breakpoint->width();
		else
			nm += 8;
	}
	
	if (getFlag(ShowLineNumbers))
	{
		int cnt = numLines();

		while (cnt)
		{
			nm += charWidth;
			lnl++;
			cnt /= 10;
		}
		
		nm += 4;
	}
	
	if (getFlag(ShowModifiedLines) && nm < 6)
		nm = 6;

	if (nm != margin)
	{
		margin = nm;
		lineNumberLength = lnl;
		updateContents();
		updateCursor();
	}
}

void GEditor::docTextChangedLater()
{
	emit textChanged();
}

void GEditor::docTextChanged()
{
	if (painting)
		QTimer::singleShot(0, this, SLOT(docTextChangedLater()));
	else
		docTextChangedLater();
}


void GEditor::baptizeVisible()
{
	doc->baptizeUntil(lastVisibleRow());
}

void GEditor::baptizeVisible(int x, int y)
{
	doc->baptizeUntil(lastVisibleRow(y));
}


void GEditor::imStartEvent( QIMEvent *e )
{
}

void GEditor::imComposeEvent( QIMEvent *e )
{
}

void GEditor::imEndEvent( QIMEvent *e )
{
	insert(e->text());
}

bool GEditor::focusNextPrevChild(bool)
{
	return false;
}

void GEditor::lineInserted(int y)
{
	if (largestLine >= y)
		largestLine++;
}

void GEditor::lineRemoved(int y)
{
	if (largestLine == y)
		updateWidth(y);
	else if (largestLine > y)
		largestLine--;
}

void GEditor::flash()
{
	if (flashed)
		return;
		
	flashed = true;
	setPaletteBackgroundColor(QColor(QRgb(styles[GLine::Background].color.rgb() ^ 0xFFFFFF)));
	redrawContents();
	QTimer::singleShot(50, this, SLOT(unflash()));
}

void GEditor::unflash()
{
	flashed = false;
	setPaletteBackgroundColor(styles[GLine::Background].color);
	redrawContents();
}
