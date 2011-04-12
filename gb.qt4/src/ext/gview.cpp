/***************************************************************************

  gview.cpp

  (c) 2000-2011 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
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

#include <QPainter>
#include <QScrollBar>
#include <QClipboard>
#include <QPixmap>
#include <QRegExp>
#include <QApplication>
#include <Q3DragObject>
#include <QTimer>
#include <Q3Dict>
#include <QCursor>
#include <Q3CString>
#include <QString>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QStyle>
#include <QStyleOption>

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
	Qt::red,
	Qt::black,
	Qt::gray,
	Qt::green
};

/**---- GEditor -----------------------------------------------------------*/

QPixmap *GEditor::cache = 0;
QPixmap *GEditor::breakpoint = 0;
int GEditor::count = 0;

void GEditor::reset()
{
	x = y = xx = 0;
	x1m = x2m = 0;
	ym = -1;
	lastx = -1;
	cursor = false;
	lineNumberLength = 0;
	center = false;
	largestLine = 0;
	flashed = false;
	painting = false;
	_showRow = -1;
	_showCol = 0;
	_showLen = 0;
	_posOutside = false;
	lineWidthCacheY = -1;
	
	foldClear();
}

GEditor::GEditor(QWidget *parent) 
	: Q3ScrollView(parent),
	fm(font())
{
	int i;

	if (count == 0)
		cache = new QPixmap();

	count++;

	setKeyCompression(true);
	setFocusPolicy(Qt::WheelFocus);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	//setAttribute(Qt::WA_StaticContents, true);
	
	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	viewport()->setCursor(Qt::ibeamCursor);
  viewport()->setBackgroundRole(QPalette::Base);
  viewport()->setAutoFillBackground(true);
	viewport()->setPaletteBackgroundColor(defaultColors[GLine::Background]);
	viewport()->setAttribute(Qt::WA_NoSystemBackground, true);
	//viewport()->setAttribute(Qt::WA_StaticContents, true);
  viewport()->setFocusProxy(this);
	
	margin = 0;
	doc = 0;
	_showStringIgnoreCase = false;
	_insertMode = false;
	_charWidth = fm.width(" ");
	_cellw = _cellh = 0;

	for (i = 0; i < GLine::NUM_STATE; i++)
	{
		styles[i].color = defaultColors[i];
		styles[i].bold = i == GLine::Keyword || i == GLine::Help;
		styles[i].italic = i == GLine::Comment;
		styles[i].underline = i == GLine::Error;
		if (i == GLine::Comment || i == GLine::Help)
		{
			styles[i].background = true;
			styles[i].backgroundColor = QColor(0xE8, 0xE8, 0xE8);
		}
		else
			styles[i].background = false;
	}

	flags = 1 << ShowDots;
	
	reset();
	
	setDocument(NULL);

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
	int nw = QMAX(cache->width(), QMIN(visibleWidth(), _cellw));
	int nh = QMAX(cache->height(), QMIN(visibleHeight(), _cellh));
	if (nw > 0 && nh > 0 && (nw != cache->width() || nh != cache->height()))
	{
		//qDebug("updateCache: %d %d\n", nw, nh);
		cache->resize(nw, nh);
	}
}

int GEditor::lineWidth(int y) const
{
	// Add 2 or a space width so that we see the cursor at end of line
	return margin + fm.width(doc->lines.at(y)->s.getString()) + (_insertMode ? _charWidth : 2);
}


int GEditor::lineWidth(int y, int len)
{
	int ns;
	
	if (len <= 0)
		return margin;
	else
	{
		QString s = doc->lines.at(y)->s.getString();
		ns = QMAX(0, len - s.length());
		len = QMIN(len, s.length());
		
		if (y != lineWidthCacheY)
		{
			lineWidthCacheY = y;
			lineWidthCache.clear();
			//qDebug("y = %d", y);
		}
		
		int lw = lineWidthCache[len];
		if (!lw)
		{
			//qDebug("lineWidthCache: %d", len);
			lw = fm.width(s, len);
			lineWidthCache.insert(len, lw);
		}
		
		lw += margin;
		if (ns) lw += ns * _charWidth;
		return lw;
	}
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
	
		//qDebug("contentsWidth() = %d  _cellw = %d", contentsWidth(), _cellw);
		//setCellWidth(QMAX(visibleWidth(), margin + charWidth * doc->getMaxLineLength() + 2));
		
		if (w > _cellw)
		{
			largestLine = y;
			goto UPDATE_WIDTH;
		}
		else if (w < _cellw && y == largestLine)
		{
			w = findLargestLine();
			goto UPDATE_WIDTH;
		}
	}
	
	return;
	
UPDATE_WIDTH:
	
	w = QMAX(visibleWidth(), w);
	if (w != _cellw)
	{
		_cellw = w;
		updateViewport();
		//qDebug("setCellWidth: %d (largestLine = %d)", w, largestLine);
	}
}
	
void GEditor::updateHeight()
{
	_cellh = fm.ascent() + fm.descent() + 2;
	updateCache();
		
	if (pattern.height() < _cellh)
		pattern.resize(16, _cellh);

	updateViewport();
}

void GEditor::redrawContents()
{
	updateContents();
	//if (contentsHeight() < visibleHeight())
	//	repaintContents(contentsX(), contentsHeight(), visibleWidth(), visibleHeight() - contentsHeight() + contentsX(), true);
}

void GEditor::changeEvent(QEvent *e)
{
	Q3ScrollView::changeEvent(e);
	if (e->type() == QEvent::FontChange)
	{
		fm = fontMetrics();
		_charWidth = fm.width(" ");
		italicFont = font();
		italicFont.setItalic(true);
		clearLineWidthCache();
		updateMargin();
		updateWidth();
		updateHeight();
		updateContents();
	}
}

static int find_last_non_space(const QString &s)
{
	int i;
	
	for (i = s.length() - 1; i >= 0; i--)
	{
		if (s[i].unicode() > 32)
			return i;
	}
	return -1;
}

void GEditor::paintDottedSpaces(QPainter &p, int row, int ps, int ls)
{
	QPoint pa[ls];
	int i, x, y, w;

	x = lineWidth(row, ps) + 1;
	y = fm.ascent();
	w = _charWidth;
	for (i = 0; i < ls; i++)
	{
		pa[i].setX(x);
		pa[i].setY(y);
		x += w;
	}

	p.drawPoints(pa, i);
}

void GEditor::paintText(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int h, int xs1, int xs2, int row)
{
	int i;
	int len, style, pos;
	bool alternate;
	QString sd;
	GHighlightStyle *st;
	bool italic = false;
	int nx;
	int ps;
	QColor bg;
	bool draw_bg;
	bool show_dots = getFlag(ShowDots);

	p.setFont(font());

	pos = 0;
	ps = find_last_non_space(l->s.getString()) + 1;
	
	for (i = 0; i < GB.Count(l->highlight); i++)
	{
		if (pos > (xmin + lmax))
			break;

		len = l->highlight[i].len;
		nx = lineWidth(row, pos + len);
		//w = len * charWidth;

		if ((pos + len) >= xmin)
		{
			style = l->highlight[i].state;
			alternate = l->highlight[i].alternate;
			st = &styles[style];
			draw_bg = false;
			if (st->background)
			{
				draw_bg = true;
				bg = st->backgroundColor;
			}
			else if (alternate)
			{
				draw_bg = true;
				bg = styles[GLine::Alternate].color;
			}
			
			if (style == GLine::Keyword && doc->isKeywordsUseUpperCase())
				sd = l->s.mid(pos, len).upper().getString();
			else
				sd = l->s.mid(pos, len).getString();

			if (draw_bg)
			{
				int x1 = x;
				int x2 = nx;
				int xd1 = xs1;
				int xd2 = xs2;
				
				if (xd1 < x1)
					xd1 = x1;
				if (xd2 > x2)
					xd2 = x2;

				if (alternate && (i == (GB.Count(l->highlight) - 1)))
					nx = _cellw;
					
				p.fillRect(x, 0, nx - x, h, bg);
				
				if (xd2 > xd1)
					p.fillRect(xd1, 0, xd2 - xd1, h, styles[GLine::Selection].color);
			}
			
			p.setPen(st->color);
			
			if (ps >= 0 && pos >= ps)
			{
				if (show_dots)
					paintDottedSpaces(p, row, pos, QMIN(xmin + lmax - pos, sd.length()));
			}
			else
			{
				if (st->italic != italic)
				{
					italic = st->italic;
					p.setFont(italic ? italicFont : font());
				}

				p.drawText(x, y, sd);
				if (st->bold)
					p.drawText(x + 1, y, sd);
			}
			
			if (st->underline)
				p.drawLine(x, y + 2, nx - 1, y + 2);
		}

		pos += len;
		x = nx;
	}

	if (pos < (int)l->s.length() && pos < (xmin + lmax))
	{
		p.setPen(styles[GLine::Normal].color);
		p.drawText(x, y, l->s.mid(pos).getString());
		if (show_dots)
			paintDottedSpaces(p, row, pos, QMIN(xmin + lmax - pos, (int)l->s.length() - pos));
	}
}

void GEditor::paintShowString(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int h, int row)
{
	int pos;
	QString sd;
	int nx;
	int ps;
	QColor bg;

	bg = styles[GLine::Highlight].color;
	
	if (_showString.length())
	{
		pos = 0;
		for(;;)
		{
			if (pos >= (int)l->s.length())
				break;
			pos = l->s.find(_showString, pos, !_showStringIgnoreCase);
			if (pos < 0)
				break;
			ps = lineWidth(row, pos);
			//if (ps > (xmin + lmax))
			//	break;
			nx = lineWidth(row, pos + _showString.length());
			p.fillRect(ps, 0, nx - ps, h, bg);
			pos += _showString.length();
		}
	}

	if (row == _showRow && _showLen > 0 && _showCol >= 0 && _showCol < (int)l->s.length())
	{
		ps = lineWidth(row, _showCol);
		nx = lineWidth(row, QMIN((int)l->s.length(), _showCol + _showLen));
		p.fillRect(ps, 0, nx - ps, h, bg);
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

static void highlight_text(QPainter &p, int x, int y, int x2, int yy, QString s, QColor color)
{
	//int i, j;
	
	//p.setPen(color);
	
	/*for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++)
			p.drawText(x + i, y + j, s);*/
		
	p.fillRect(x, 0, x2 - x, yy - 1, color);
}

void GEditor::paintCell(QPainter *painter, int row, int)
{
	QRect ur;
	GLine *l;
	int x1, y1, x2, y2, xs1, xs2;
	QColor color, a, b, c;
	int xmin, lmax;
	int i;
	int realRow;
	bool folded;
	bool highlight;

	ur = QRect(0, row * _cellh, _cellw, _cellh);
	contentsToViewport(ur.x(), ur.y(), x1, y1);
	ur.setRect(-x1, y1, ur.width(), ur.height());

	if (row < 0)
		realRow = row;
	else
		realRow = viewToReal(row);
	
	if (realRow < 0 || realRow >= numLines())
	{
		QColor color = viewport()->paletteBackgroundColor();
		if (flashed)
			color = QColor(QRgb(color.rgb() ^ 0x00FFFFFF));
		painter->fillRect(0, 0, ur.width(), ur.height(), color);
		return;
	}
	
	l = doc->lines.at(realRow);
	
	// Colorize as soon as possible
	highlight = (doc->getHighlightMode() != GDocument::None) && !doc->isLineEditedSomewhere(realRow); //(l->modified && realRow == y && !getFlag(HighlightCurrent)));
	if (highlight)
	{
		painting = true;
		doc->colorize(realRow);
		painting = false;
	}
	
	folded = l->proc && isFolded(realRow);
		
	//xmin = (ur.left() - margin) / charWidth;
	xmin = posToColumn(realRow, 0) - 1;
	if (xmin < 0)
		xmin = 0;
	//lmax = 2 + visibleWidth() / charWidth;
	lmax = 2 + posToColumn(realRow, visibleWidth()) - xmin;
	//if (row == 0)
	//	qDebug("%d: %d %d %d %d (%d %d)", row, ur.left(), ur.top(), ur.width(), ur.height(), xmin, lmax);

	// Line background
	if (l->flag & (1 << GLine::CurrentFlag))
		a = styles[GLine::Current].color;

	if (getFlag(ShowCurrentLine) && realRow == y)
		b = styles[GLine::Line].color;

	QPainter p(cache);
	
	color = calc_color(a, b, styles[GLine::Background].color);
	
	p.fillRect(0, 0, _cellw, _cellh, color);
	
	p.setFont(font());
	//p.setFont(painter->font());
	//p.translate(-ur.left(), 0);

	// Procedure separation
	if (getFlag(ShowProcedureLimits) && l->proc)
	{
		if (getFlag(BlendedProcedureLimits))
		{
			make_blend(pattern, styles[GLine::Line].color, color, _cellh);
			p.drawTiledPixmap(0, 0, cache->width(), cache->height(), pattern);
		}
		else
		{
			QBrush brush(styles[GLine::Selection].color, Qt::Dense4Pattern);
			p.fillRect(0, 0, cache->width(), 1, brush);
		}
	}

	p.translate(-ur.left(), 0);

	// Show string
	if ((_showRow == realRow || _showString.length()))
		paintShowString(p, l, margin, fm.ascent() + 1, xmin, lmax, _cellh, realRow);
	
	// Selection background
	xs1 = 0;
	xs2 = 0;
	if (doc->hasSelection())
	{
		doc->getSelection(&y1, &x1, &y2, &x2, _insertMode);

		if (realRow >= y1 && realRow <= y2 && !(realRow == y2 && x2 == 0))
		{
			if (_insertMode)
			{
				x1 = lineWidth(y1, x1);
				x2 = lineWidth(y2, x2);
			}
			else
			{
				if (realRow > y1 || x1 == 0)
					x1 = 0;
				else
					x1 = lineWidth(y1, x1);

				if (realRow < y2)
					x2 = _cellw + 1;
				else
					x2 = lineWidth(y2, x2);
			}

			p.fillRect(x1, 0, x2 - x1, _cellh, styles[GLine::Selection].color);
			xs1 = x1;
			xs2 = x2;
		}
	}

	// Margin
	if (margin && (margin > ur.left()))
	{
		//if (!l->flag)
		//	p.fillRect(0, 0, margin, _cellh, color); //styles[GLine::Background].color);

		if (getFlag(ShowModifiedLines) && l->changed)
			p.fillRect(0, 0, margin - 2, _cellh, styles[GLine::Highlight].color);
		else if (getFlag(ShowCurrentLine))
			p.fillRect(0, 0, margin - 2, _cellh, styles[GLine::Line].color);

		//x1 = 0;

		if (getFlag(ShowLineNumbers))
		{
			int n = realRow + 1;
			if ((n % 10) == 0 || (l->proc && folded))
				p.setPen(styles[GLine::Normal].color);
			else
				p.setPen(styles[GLine::Selection].color);
			p.drawText(2, fm.ascent() + 1, QString::number(n).rightJustify(lineNumberLength));
		}
	}
	
	// Highlight braces
	if (getFlag(HighlightBraces) && realRow == ym && x1m >= 0)
	{
		//highlight_text(p, x1m * charWidth + margin, fm.ascent(), l->s.getString().mid(x1m, 1), styles[GLine::Highlight].color);
		//highlight_text(p, x2m * charWidth + margin, fm.ascent(), l->s.getString().mid(x2m, 1), styles[GLine::Highlight].color);
		highlight_text(p, lineWidth(ym, x1m), fm.ascent(), lineWidth(ym, x1m + 1), _cellh, l->s.getString().mid(x1m, 1), styles[GLine::Highlight].color);
		//highlight_text(p, lineWidth(ym, x2m), fm.ascent(), lineWidth(ym, x2m + 1), _cellh, l->s.getString().mid(x2m, 1), styles[GLine::Highlight].color);
		/*p.fillRect(x1m * charWidth + margin, 0, charWidth, _cellh, styles[GLine::Highlight].color);
		p.fillRect(x2m * charWidth + margin, 0, charWidth, _cellh, styles[GLine::Highlight].color);*/
	}

	// Line text
	if (l->s.length())
	{
		if (!highlight)
		{
			p.setPen(styles[GLine::Normal].color);
			//p.drawText(margin + xmin * charWidth, fm.ascent() + 1, l->s.getString().mid(xmin, lmax));
			p.drawText(lineWidth(realRow, xmin), fm.ascent() + 1, l->s.getString().mid(xmin, lmax));
			if (getFlag(ShowDots))
			{
				i = find_last_non_space(l->s.getString()) + 1;
				if (i >= 0 && i < (xmin + lmax))
					paintDottedSpaces(p, realRow, i, QMIN(xmin + lmax, (int)l->s.length()) - i);		
			}
		}
		/*else if (l->flag)
		{
			p.setPen(styles[GLine::Background].color);
			p.drawText(margin, fm.ascent(), l->s.getString().mid(xmin, lmax));
		}*/
		else
		{
			paintText(p, l, margin, fm.ascent() + 1, xmin, lmax, _cellh, xs1, xs2, realRow);
		}
	}
	
	// Folding symbol
	if (margin && l->proc)
	{
		QPalette pal;
		QStyleOption opt;
		int w;
		
		pal.setColor(QColorGroup::Foreground, styles[GLine::Normal].color);
		w = qMin(_cellh, 12);
		//opt.rect = QRect(margin - 12, 0, 12, 12);
		opt.rect = QRect(margin - w - 2, 0, w, _cellh);
		opt.palette = pal;
		opt.state |= QStyle::State_Enabled;
		
		style()->drawPrimitive(folded ? QStyle::PE_IndicatorArrowRight : QStyle::PE_IndicatorArrowDown, &opt, &p);
	}
	
	// Breakpoint symbol
	if ((l->flag & (1 << GLine::BreakpointFlag)) && breakpoint && !breakpoint->isNull())
	{
		//p.fillRect(margin - 10, 0, 8, _cellh, styles[GLine::Breakpoint].color);
		//updateBreakpoint(styles[GLine::Background].color.rgb(), styles[GLine::Breakpoint].color.rgb());
		p.drawPixmap(margin - breakpoint->width() - 2, (_cellh - breakpoint->height()) / 2, *breakpoint);
	}

	// Text cursor
	if (cursor && realRow == y)
	{
		int xc = lineWidth(realRow, x);
		int wc = 2;
		//p.fillRect(QMIN((int)l->s.length(), x) * charWidth + margin, 0, 1, _cellh, styles[GLine::Normal].color);
		if (_insertMode)
			wc = lineWidth(realRow, x + 1) - xc;
		
		p.fillRect(xc, 0, wc, _cellh, styles[GLine::Normal].color);
	}

	// Flash
	if (flashed)
	{
		//p.setCompositionMode(QPainter::CompositionMode_Xor);
		p.fillRect(0, 0, visibleWidth(), _cellh, Qt::black);
	}

	p.end();
	
	//if (cache->width() < visibleWidth())
	//	qDebug("cache->width() = %d < visibleWidth() = %d", cache->width(), visibleWidth());
	painter->drawPixmap(ur.left(), 0, *cache, 0, 0, cache->width(), cache->height()); //, _cellw, _cellh);
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

	if (len == 0 || x > len)
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
				s[i] = (i < len) ? str.at(i) : 0;
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

	if (x > 0 && x <= len)
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

void GEditor::leaveCurrentLine()
{
	if (y < 0 || y >= numLines())
		return;
	
	doc->colorize(y);
	if (!_insertMode && x > lineLength(y))
		x = lineLength(y);
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
	
	if (ny == y)
	{
		if (nx < 0 && ny > 0)
		{
			ny = viewToReal(realToView(y) - 1);
			nx = lineLength(ny);
		}
		else if (!_insertMode && nx > lineLength(ny) && ny < (numLines() - 1))
		{
			ny = viewToReal(realToView(y) + 1);
			nx = 0;
		}
	}
	
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
	
	ny = checkFolded(ny);
	
	if (nx < 0)
		nx = 0;
	else 
	{
		int xmax = _insertMode ? QMAX((_cellw / _charWidth) + 1, lineLength(largestLine)) : lineLength(ny);
		
		if (nx > xmax)
			nx = xmax;
	}
	
	if (ny != y)
		leaveCurrentLine();

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
		
		updateLine(oy);
		
		if (hasFocus())
			startBlink();
		else
			updateLine(y);
			
		ensureCursorVisible();
		//QTimer::singleShot(0, this, SLOT(ensureCursorVisible()));
		
		change = true;
		
		if (mark)
			doc->endSelection(y, x);
		
		updateInfo();
		emit cursorMoved();
	}
	else if (center)
		ensureCursorVisible();

	checkMatching();
	
	return change;
}

void GEditor::insert(QString text)
{
	doc->eraseSelection(_insertMode);
	unfoldLine(y);
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


void GEditor::cursorUp(bool shift, bool ctrl, bool alt)
{
	if (alt)
	{
		if (!ctrl)
		{
			QString str;
			int x1, y1, x2, y2;
			bool hasSelection;
			
			hasSelection = doc->hasSelection();
			
			if (hasSelection)
			{
				doc->getSelection(&y1, &x1, &y2, &x2, _insertMode);
				if (x2) y2++;
			}
			else
			{
				y1 = y;
				y2 = y + 1;
				x1 = x;
			}
			
			if (y1 > 0)
			{
				str = doc->lines.at(y1 - 1)->s.getString() + '\n';
				
				doc->begin();
				doc->remove(y1 - 1, 0, y1, 0);
				doc->insert(y2 - 1, 0, str);
				if (hasSelection)
				{
					cursorGoto(y1 - 1, 0, false);
					doc->startSelection(this, y1 - 1, 0);
					doc->endSelection(y2 - 1, 0);
				}
				doc->end();
			}
		}
	}
	else if (ctrl)
		cursorGoto(doc->getPreviousLimit(y), xx, shift);
	else
		cursorGoto(viewToReal(realToView(y) - 1), xx, shift);
}


void GEditor::cursorDown(bool shift, bool ctrl, bool alt)
{
	if (alt)
	{
		if (!ctrl)
		{
			QString str;
			int x1, y1, x2, y2;
			bool hasSelection;
			
			hasSelection = doc->hasSelection();
			
			if (hasSelection)
			{
				doc->getSelection(&y1, &x1, &y2, &x2, _insertMode);
				if (x2) y2++;
			}
			else
			{
				y1 = y;
				y2 = y + 1;
				x1 = x;
			}
			
			if (y2 < (numLines() - 1))
			{
				str = doc->lines.at(y2)->s.getString() + '\n';
				
				doc->begin();
				doc->remove(y2, 0, y2 + 1, 0);
				doc->insert(y1, 0, str);
				if (hasSelection)
				{
					cursorGoto(y2 + 1, 0, false);
					doc->startSelection(this, y1 + 1, 0);
					doc->endSelection(y2 + 1, 0);
				}
				doc->end();
			}
		}
	}
	else if (!ctrl)
		cursorGoto(viewToReal(realToView(y) + 1), xx, shift);
	else
		cursorGoto(doc->getNextLimit(y), xx, shift);
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
	int page = visibleHeight() / _cellh;
	cursorGoto(viewToReal(realToView(y) - page), 0, mark);
}

void GEditor::cursorPageDown(bool mark)
{
	int page = visibleHeight() / _cellh;
	cursorGoto(viewToReal(realToView(y) + page), 0, mark);
}

void GEditor::newLine()
{
	doc->begin();
	doc->eraseSelection(_insertMode);
	doc->insert(y, x, '\n' + doc->lines.at(y)->s.left(QMIN(x, doc->getIndent(y))));
	cursorGoto(doc->yAfter, doc->xAfter, false);
	doc->end();
}

void GEditor::del(bool ctrl)
{
	if (doc->hasSelection())
	{
		doc->eraseSelection(_insertMode);
		return;
	}
	
	if (x >= lineLength(y))
	{
		if (!_insertMode && y < (numLines() - 1))
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
		doc->eraseSelection(_insertMode);
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

	doc->getSelection(&y1, &x1, &y2, &x2, _insertMode);
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

	QString text = doc->getSelectedText(_insertMode).getString();
	QApplication::clipboard()->setText(text, mouse ? QClipboard::Selection : QClipboard::Clipboard);
}

void GEditor::cut()
{
	copy(false);
	doc->eraseSelection(_insertMode);
}

void GEditor::paste(bool mouse)
{
	QString text;
	QString subType("plain");
	int i, i2, xs;
	QString tab;

	text = QApplication::clipboard()->text(subType, mouse ? QClipboard::Selection : QClipboard::Clipboard);

	if (text.length() == 0)
		return;
	
	tab.fill(' ', doc->getTabWidth());
	text.replace("\t", tab);

	for (i = 0; i < text.length(); i++)
	{
		if ((text[i] < ' ' || text[i].isSpace()) && text[i] != '\n')
			text[i] = ' ';
	}

	if (_insertMode)
	{
		doc->begin();
		i = 0;
		while (i < (int)text.length())
		{
			i2 = text.find('\n', i);
			if (i2 < 0) 
				i2 = text.length();
			xs = x;
			insert(text.mid(i, i2 - i));
			x = xs;
			y++;
			if (y >= doc->numLines())
				insert("\n");
			i = i2 + 1; 
		}
		doc->end();
	}
	else
		insert(text);
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
	bool shift = e->state() & Qt::ShiftButton;
	bool ctrl = e->state() & Qt::ControlButton;
	bool alt = e->state() & Qt::AltButton;

	e->accept();

	if (doc->isReadOnly())
	{
		switch (e->key())
		{
			case Qt::Key_Left:
				cursorLeft(shift, ctrl); return;
			case Qt::Key_Right:
				cursorRight(shift, ctrl); return;
			case Qt::Key_Up:
				cursorUp(shift, ctrl, false); return;
			case Qt::Key_Down:
				cursorDown(shift, ctrl, false); return;
			case Qt::Key_Home:
				cursorHome(shift, ctrl); return;
			case Qt::Key_End:
				cursorEnd(shift, ctrl); return;
			case Qt::Key_Prior:
				cursorPageUp(shift); return;
			case Qt::Key_Next:
				cursorPageDown(shift); return;
		}

		if (!ctrl)
			goto __IGNORE;

		switch(e->key())
		{
			case Qt::Key_C:
				copy(); return;
			case Qt::Key_A:
				selectAll(); return;
		}
	}
	else
	{
		QString t = e->text();
		if (t.length() && t.at(0).isPrint()
				&& e->key() != Qt::Key_Return
				&& e->key() != Qt::Key_Enter
				&& e->key() != Qt::Key_Delete
				&& e->key() != Qt::Key_Backspace)
		{
			if (_insertMode)
			{
				doc->begin();
				del(false);
				insert(t);
				doc->end();
			}
			else
				insert(t);
			return;
		}

		switch (e->key())
		{
			case Qt::Key_Left:
				cursorLeft(shift, ctrl); return;
			case Qt::Key_Right:
				cursorRight(shift, ctrl); return;
			case Qt::Key_Up:
				cursorUp(shift, ctrl, alt); return;
			case Qt::Key_Down:
				cursorDown(shift, ctrl, alt); return;
			case Qt::Key_Enter: 
			case Qt::Key_Return:
				newLine(); return;
			case Qt::Key_Home:
				cursorHome(shift, ctrl); return;
			case Qt::Key_End:
				cursorEnd(shift, ctrl); return;
			case Qt::Key_Backspace:
				backspace(ctrl); return;
			case Qt::Key_Delete:
				del(ctrl); return;
			case Qt::Key_Prior:
				cursorPageUp(shift); return;
			case Qt::Key_Next:
				cursorPageDown(shift); return;
			case Qt::Key_Tab:
				tab(false); return;
			case Qt::Key_BackTab:
				tab(true); return;
			case Qt::Key_Insert:
				setInsertMode(!_insertMode);
		}

		if (!ctrl)
			goto __IGNORE;

		switch(e->key())
		{
			case Qt::Key_C:
				copy(); return;
			case Qt::Key_X:
				cut(); return;
			case Qt::Key_V:
				paste(); return;
			case Qt::Key_Z:
				undo(); return;
			case Qt::Key_Y:
				redo(); return;
			case Qt::Key_A:
				selectAll(); return;
		}
	}

__IGNORE:

	e->ignore();
}


int GEditor::posToColumn(int y, int px)
{
	int i = -1, lw;
	int len = doc->lineLength(y);
	QString s = doc->lines.at(y)->s.getString();
	int d, f;
	
	if (px < margin || px >= visibleWidth())
		_posOutside = true;
	
	if (len == 0)
		return (px - margin) / _charWidth;
		
	px += contentsX();
	
	d = 0;
	f = len;
	while (f > d)
	{
		if (i < 0)
			i = px / fm.width("m");
		else
			i = (d + f) / 2;
		
		lw = lineWidth(y, i);
		if (px < lw)
		{
			f = i;
			continue;
		}
		
		lw += fm.width(s[i]);
		if (px >= lw)
		{
			d = i + 1;
			continue;
		}
		
		d = i;
		break;
	}
	
	_posOutside = d > len;
	return d;
}


int GEditor::posToLine(int py)
{
	int ny;

	ny = rowAt(contentsY() + py);
	_posOutside = true;
	if (ny < 0)
		ny = 0;
	else if (ny >= visibleLines())
		ny = visibleLines() - 1;
	else
		_posOutside = false;
	
	return viewToReal(ny); 
}

bool GEditor::posToCursor(int px, int py, int *y, int *x)
{
	int nx, ny;
	bool outside;

	ny = posToLine(py);
	outside = _posOutside;
	nx = posToColumn(ny, px);
	if (_insertMode)
		nx = QMAX(0, nx); //QMIN(nx, lineLength(ny)));
	else
		nx = QMAX(0, QMIN(nx, lineLength(ny)));

	*y = ny;
	*x = nx;
	
	return outside || _posOutside;
}

void GEditor::cursorToPos(int y, int x, int *px, int *py)
{
	int npx, npy;

	y = realToView(y);

	npy = y * _cellh - contentsY();
	//npx = x * charWidth - contentsX() + margin;
	npx = lineWidth(y, x) - contentsX();
	
	*py = npy;
	*px = npx;
}

bool GEditor::updateCursor()
{
	if (contentsX() + lastx >= margin)
	{
		viewport()->setCursor(Qt::IBeamCursor);
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
	bool shift = e->state() & Qt::ShiftButton;
	int nx, ny;
	//stopAutoScroll();
	//d->dnd_startpos = e->pos();

	if (e->button() != Qt::LeftButton && e->button() != Qt::MidButton)
		return;

	posToCursor(e->pos().x(), e->pos().y(), &ny, &nx);
	
	lastx = e->pos().x();
	left = updateCursor();
	
	if (left)
	{
		if (doc->lines.at(ny)->proc)
		{
			if (isFolded(ny))
				unfoldLine(ny);
			else
				foldLine(ny);
		}
	}
	else
		cursorGoto(ny, nx, shift);
}

void GEditor::mouseMoveEvent(QMouseEvent *e)
{
	if ((e->buttons() & Qt::MouseButtonMask) == Qt::LeftButton)
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
	int ny;
	
	if (left)
	{
		ny = posToLine(e->pos().y());
		if (doc->lines.at(ny)->proc)
		{
			if (isFolded(ny))
				foldAll();
			else
				unfoldAll();
		}
		else
			emit marginDoubleClicked(ny);
		return;
	}
	
	if (e->button() == Qt::LeftButton && !(e->state() & Qt::ShiftButton))
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

	if (e->button() == Qt::MidButton)
		paste(true);
}


void GEditor::resizeEvent(QResizeEvent *e)
{
	Q3ScrollView::resizeEvent(e);
	baptizeVisible();
	updateWidth();
	updateCache();
	//updateViewport();
}

bool GEditor::isCursorVisible()
{
	int px, py;
	
	cursorToPos(y, x, &px, &py);
	
	return !(px < margin || px >= (visibleWidth() - 2) || py < 0 || py >= (visibleHeight() - _cellh - 1));
}

void GEditor::ensureCursorVisible()
{
	int yy;
	
	if (!isUpdatesEnabled())
		return;
	
	if (!isCursorVisible())
	{
		yy = realToView(y);
		
		if (center)
			//ensureVisible(x * charWidth, y * _cellh + _cellh / 2, margin + 2, visibleHeight() / 2);
			ensureVisible(lineWidth(y, x) + getCharWidth() / 2, yy * _cellh + _cellh / 2, margin + 2, visibleHeight() / 2);
		else
			//ensureVisible(x * charWidth, y * _cellh + _cellh / 2, margin + 2, _cellh);
			ensureVisible(lineWidth(y, x) + getCharWidth() / 2, yy * _cellh + _cellh / 2, margin + 2, _cellh);
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
	if (!doc->isReadOnly())
	{
		cursor = !cursor;
		updateLine(y);
	}
}

void GEditor::focusInEvent(QFocusEvent *e)
{
	startBlink();
	//ensureCursorVisible();
	Q3ScrollView::focusInEvent(e);
}

void GEditor::focusOutEvent(QFocusEvent *e)
{
	stopBlink();
	leaveCurrentLine();
	Q3ScrollView::focusOutEvent(e);
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
		viewport()->setPaletteBackgroundColor(style->color);
		redrawContents();
	}
	else
		updateContents();
}

void GEditor::setNumRows(int n)
{
	_nrows = realToView(n - 1) + 1;
	//Q3GridView::setNumRows(n);
	updateViewport();
	updateContents();
	//Q3ScrollView::updateScrollBars();
	//if (contentsHeight() < visibleHeight())
	//	repaintContents(contentsX(), contentsHeight(), visibleWidth(), visibleHeight() - contentsHeight() + contentsX(), true);
	//if (contentsHeight() < visibleHeight())
		//repaintContents(true);
}

void GEditor::getStyle(int index, GHighlightStyle *style) const
{
	if (index < 0 || index >= GLine::NUM_STATE)
		index = GLine::Background;

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
			nm += breakpoint->width() + 2;
		else
			nm += 8;
	}
	else if (getFlag(ShowLineNumbers))
		nm += 4;
	
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
	clearLineWidthCache();
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

void GEditor::inputMethodEvent(QInputMethodEvent *e)
{
	//qDebug("inputMethodEvent: %s\n", (const char *)e->commitString().toUtf8());

	if (doc->isReadOnly())
	{
		e->ignore();
		return;
	}

	insert(e->commitString());
	
	/*if (QApplication::keypadNavigationEnabled() && 
					hasFocus() && !hasEditFocus()
					&& !e->preeditString().isEmpty()) 
	{
		setEditFocus(true);
		selectAll();        // so text is replaced rather than appended to
	}*/

	#if 0
    int priorState = d->undoState;
    d->removeSelectedText();

    int c = d->cursor; // cursor position after insertion of commit string
    if (e->replacementStart() <= 0)
        c += e->commitString().length() + qMin(-e->replacementStart(), e->replacementLength());

    d->cursor += e->replacementStart();

    // insert commit string
    if (e->replacementLength()) {
        d->selstart = d->cursor;
        d->selend = d->selstart + e->replacementLength();
        d->removeSelectedText();
    }
    if (!e->commitString().isEmpty())
        d->insert(e->commitString());

    d->cursor = c;

    d->textLayout.setPreeditArea(d->cursor, e->preeditString());
    d->preeditCursor = e->preeditString().length();
    d->hideCursor = false;
    QList<QTextLayout::FormatRange> formats;
    for (int i = 0; i < e->attributes().size(); ++i) {
        const QInputMethodEvent::Attribute &a = e->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            d->preeditCursor = a.start;
            d->hideCursor = !a.length;
        } else if (a.type == QInputMethodEvent::TextFormat) {
            QTextCharFormat f = qvariant_cast<QTextFormat>(a.value).toCharFormat();
            if (f.isValid()) {
                QTextLayout::FormatRange o;
                o.start = a.start + d->cursor;
                o.length = a.length;
                o.format = f;
                formats.append(o);
            }
        }
    }
    d->textLayout.setAdditionalFormats(formats);
    d->updateTextLayout();
    update();
    if (!e->commitString().isEmpty())
        d->emitCursorPositionChanged();
    d->finishChange(priorState);
#ifndef QT_NO_COMPLETER
    if (!e->commitString().isEmpty())
        d->complete(Qt::Key_unknown);
#endif
	#endif
}

QVariant GEditor::inputMethodQuery(Qt::InputMethodQuery property) const
{
	//qDebug("inputMethodQuery: %d\n", (int)property);
	GEditor *that = (GEditor *)this;
	
	switch(property) 
	{
		case Qt::ImMicroFocus:
		{
			int cx, cy, px, py;
			
			getCursor(&cy, &cx);
			that->cursorToPos(cy, cx, &px, &py);
			return QRect(px, py, 1, _cellh);
		}
		case Qt::ImFont:
				return font();
		case Qt::ImCursorPosition:
				return QVariant(x);
		case Qt::ImSurroundingText:
				return QVariant(doc->getLine(y).getString());
		case Qt::ImCurrentSelection:
				return QVariant(doc->getSelectedText(_insertMode).getString());
		default:
				return QVariant();
	}
}


bool GEditor::focusNextPrevChild(bool)
{
	return false;
}

static bool is_power_of_ten(int n)
{
	for(;;)
	{
		if (n % 10)
			return false;
		n /= 10;
		if (n == 1)
			return true;
	}
}

void GEditor::lineInserted(int y)
{
	if (largestLine >= y)
		largestLine++;
	
	if (getFlag(ShowLineNumbers) && is_power_of_ten(numLines()))
		updateMargin();
}

void GEditor::lineRemoved(int y)
{
	if (largestLine == y)
		updateWidth(y);
	else if (largestLine > y)
		largestLine--;
	
	if (getFlag(ShowLineNumbers) && is_power_of_ten(numLines() + 1))
		updateMargin();
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

#if 0
static void dump_fold(GArray<GFoldedProc> &fold)
{
	uint i;
	GFoldedProc *fp;
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		fprintf(stderr, "[%d %d] ", fp->start, fp->end);
	}
	
	fprintf(stderr, "\n");
}
#endif

int GEditor::checkCursor(int y)
{
	uint i;
	GFoldedProc *fp;
	int ny;
	
	ny = y;
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (y > fp->start && y <= fp->end)
		{
			ny = fp->start;
			break;
		}
	}
	
	return ny;
}

void GEditor::foldLine(int row, bool no_refresh)
{
	uint i;
	int pos;
	int start, end;
	GFoldedProc *fp;
	int ny;
	GLine *l;
	
	if (!doc->hasLimit(row))
		row = doc->getPreviousLimit(row);
	
	//fprintf(stderr, "foldLine %d\n", row);
	
	start = row;
	end = doc->getNextLimit(row);
	if (end < 0)
		end = numLines() - 1;
	else
	{
		for(;;)
		{
			end--;
			l = doc->lines.at(end);
			//qDebug("[%d] state = %d %d", end, l->highlight ? l->highlight[0].state : -1, l->highlight ? l->highlight[0].len : -1);
			if (!l->highlight || (l->highlight[0].state != GLine::Comment && l->highlight[0].state != GLine::Help))
				break;
		}
	}
	
	pos = -1;
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (end >= fp->start && start <= fp->end)
			return;
		if (pos < 0 && start < fp->start)
			pos = i;
	}
	
	fp = new GFoldedProc;
	fp->start = start;
	fp->end = end;
	
	if (pos < 0)
		fold.append(fp);
	else
		fold.insert(pos, fp);
	
	//dump_fold(fold);
	
	ny = checkCursor(y);
	if (ny != y)
		cursorGoto(ny, x, false);
	
	if (!no_refresh)
	{
		setNumRows(numLines());	
		//redrawContents();
	}
}

void GEditor::unfoldLine(int row)
{
	uint i;
	GFoldedProc *fp;

	//fprintf(stderr, "unfoldLine %d\n", row);
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (row >= fp->start && row <= fp->end)
		{
			fold.remove(i);
			//dump_fold(fold);
			setNumRows(numLines());	
			//redrawContents();
			return;
		}
	}
	
}

int GEditor::viewToReal(int row) const
{
	uint i;
	GFoldedProc *fp;
	
	//fprintf(stderr, "viewToReal: %d -> ", row);
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (row <= fp->start)
			break;
		row += fp->end - fp->start;
	}
	
	//fprintf(stderr, "%d\n", row);
	
	return row;
}

int GEditor::realToView(int row) const
{
	uint i;
	GFoldedProc *fp;
	int y;
	
	//fprintf(stderr, "realToView: %d -> ", row);
	
	y = row;
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (row < fp->start)
			continue;
		if (row <= fp->end)
			y -= row - fp->start;
		else
			y -= fp->end - fp->start;
	}
	
	//fprintf(stderr, "%d\n", y);
	
	return y;
}

int GEditor::visibleLines() const
{
	int n;
	uint i;
	GFoldedProc *fp;
	
	n = numLines();
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		n -= fp->end - fp->start;
	}
	
	return n;
}

bool GEditor::isFolded(int row)
{
	uint i;
	GFoldedProc *fp;
	int d, f;
	
	d = 0;
	f = fold.count();
	
	while (f > d)
	{
		i = (f + d) / 2;
		fp = fold.at(i);
		if (fp->start == row)
			return true;
		else if (fp->start < row)
			d = i + 1;
		else
			f = i;
	}
	
	/*for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (fp->start == row)
			return true;
		else if (fp->start > row)
			break;
	}*/
	
	return false;
}

/*bool GEditor::insideFolded(int row)
{
	uint i;
	GFoldedProc *fp;
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (row > fp->start && row <= fp->end)
			return true;
	}
	
	return false;
}*/

int GEditor::checkFolded(int row)
{
	uint i;
	GFoldedProc *fp;
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (row <= fp->end)
		{
			if (row > fp->start)
				return fp->start;
			else
				break;
		}
	}
	
	return row;
}

void GEditor::foldAll()
{
	int row;
	
	row = 0;
	for(;;)
	{
		foldLine(row, true);
		row = doc->getNextLimit(row);
		if (row < 0)
			break;
	}
	
	setNumRows(numLines());
	//updateContents();
}

void GEditor::unfoldAll()
{
	foldClear();
	setNumRows(numLines());	
	ensureCursorVisible();
	//redrawContents();
}


void GEditor::foldRemove(int y1, int y2)
{
	uint i;
	GFoldedProc *fp;
	int n;
	
	if (y2 < 0)
	{
		unfoldLine(y1);
		return;
	}
	
	n = y2 - y1 + 1;
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (y2 < fp->start)
		{
			fp->start -= n;
			fp->end -= n;
		}
		else if (y1 > fp->end)
			continue;
		else
		{
			fold.remove(i);
			i--;
		}
	}
}


void GEditor::foldInsert(int y, int n)
{
	uint i;
	GFoldedProc *fp;
	
	if (n == 0)
	{
		unfoldLine(y);
		return;
	}
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (fp->start > y)
		{
			fp->start += n;
			fp->end += n;
		}
		else if (fp->end >= y)
		{
			fp->end += n;
			fold.remove(i); //unfoldLine(fp->start);
			i--;
		}
	}
}

void GEditor::showString(GString s, bool ignoreCase)
{
	_showString = s;
	_showStringIgnoreCase = ignoreCase;
	updateContents();
}

void GEditor::showWord(int y, int x, int len)
{
	_showRow = y;
	_showCol = x;
	_showLen = len;
	updateLine(y);
}

void GEditor::updateLine(int y)
{
	QRect r(0, realToView(y) * _cellh, _cellw, _cellh);
	updateContents(r);
}

void GEditor::paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch)
{
	if ((_nrows * _cellh) >= contentsHeight())
		return;
	
	contentsToViewport(cx, cy, cx, cy);
	QRegion reg(QRect(cx, cy, cw, ch));
	QSize size(_cellw, _nrows * _cellh);
	reg = reg.subtracted(QRect(contentsToViewport(QPoint(0, 0)), size));

	// And draw the rectangles (transformed as needed)
	QVector<QRect> r = reg.rects();
	for (int i = 0; i < (int)r.count(); ++i)
		p->fillRect(r[i], viewport()->paletteBackgroundColor());
}

void GEditor::getInfo(QRect *rect, QString *info) const
{
	int ix, iy, iw, ih;
	QString s;
	
	s = QString("%1:%2").arg(x + 1).arg(y + 1);
	
	iw = _charWidth * 10 + 4;
	ih = _cellh + 2;
	ix = visibleWidth() - iw - 2;
	iy = visibleHeight() - ih - 2;

	*rect = QRect(ix, iy, iw, ih);
	if (info) *info = s;
}

void GEditor::updateInfo()
{
	QRect r;
	
	getInfo(&r, NULL);
	viewport()->update(r);
}

void GEditor::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
	int rowfirst = rowAt(cy);
	int rowlast = rowAt(cy + ch - 1);

	// Go through the rows
	for (int r = rowfirst; r <= rowlast; ++r) 
	{
		// get row position and height
		int rowp = r * _cellh;
		// Translate painter and draw the cell
		p->translate(0, rowp);
		paintCell(p, r, 0);
		p->translate(0, -rowp);
	}
}

void GEditor::viewportPaintEvent(QPaintEvent *e)
{
	QString info;
	QRect rect;
	QColor color;
	
	Q3ScrollView::viewportPaintEvent(e);
	
	if (getFlag(ShowCursorPosition))
	{
		QPainter p(viewport());
		getInfo(&rect, &info);
		color = styles[GLine::Current].color;
		color.setAlpha(128);
		p.fillRect(rect, color);
		rect.translate(0, 2);
		p.setPen(styles[GLine::Normal].color);
		p.drawText(rect, Qt::AlignCenter, info);
	}
}

void GEditor::updateViewport()
{
	int vw, vh;
	
	vw = contentsRect().width();
	vh = contentsRect().height();
	
	if (doc)
	{
		vw = QMAX(_cellw, vw);
		vh = QMAX(_cellh * _nrows, vh);
		//qDebug("updateViewport: h = %d  vh = %d", _cellh * numLines(), contentsRect().height());
	}
	
	if (vw != contentsWidth() || vh != contentsHeight())
		Q3ScrollView::resizeContents(vw, vh);

	updateCache();
}

void GEditor::resizeContents(int w, int h)
{
	updateViewport();
}

void GEditor::setInsertMode(bool mode)
{
	int x1, y1, x2, y2, i;
	
	if (mode !=_insertMode)
	{
		_insertMode = mode;
		if (!_insertMode)
			x = QMIN(x, lineLength(y));
		
		if (doc->hasSelection())
		{
			doc->getSelection(&y1, &x1, &y2, &x2, _insertMode);
			x = x2; y = y2;
			
			for (i = y1; i <= y2; i++)
				updateLine(i);
		}
		else
			updateLine(y);
	}
}
