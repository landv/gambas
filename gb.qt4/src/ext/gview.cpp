/***************************************************************************

  gview.cpp

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

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
#include <QWindowsStyle>
#include <QAbstractScrollArea>

#include <ctype.h>

#include "main.h"
#include "gview.h"

#define ROUND_WIDTH(_w) ((int)((_w) + 0.4999))

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
	Qt::gray,
	Qt::green
};


/**---- GEditor -----------------------------------------------------------*/

QPixmap *GEditor::_cache = 0;
QPixmap *GEditor::_breakpoint = 0;
QPixmap *GEditor::_bookmark = 0;
int GEditor::count = 0;
QStyle *GEditor::_style = 0;

QImage *_blend_pattern = 0;

void GEditor::reset()
{
	x = y = xx = 0;
	x1m = x2m = 0;
	y1m = y2m = -1;
	lastx = -1;
	_cursor = false;
	lineNumberLength = 0;
	center = false;
	largestLine = 0;
	flashed = false;
	painting = false;
	_showRow = -1;
	_showCol = 0;
	_showLen = 0;
	_posOutside = false;
	_checkCache = true;
	_ensureCursorVisibleLater = false;
	
	foldClear();
}

GEditor::GEditor(QWidget *parent) 
	: Q3ScrollView(parent),
	fm(font())
{
	int i;
	//QStyle *oldStyle;

	if (count == 0)
	{
		_cache = new QPixmap();
		_style = new QWindowsStyle;
	}

	count++;

	setKeyCompression(true);
	setFocusPolicy(Qt::WheelFocus);
	setAttribute(Qt::WA_InputMethodEnabled, true);
	
	_border = true;
	
	setMouseTracking(true);
	viewport()->setMouseTracking(true);
	viewport()->setCursor(Qt::ibeamCursor);
	saveCursor();
  viewport()->setBackgroundRole(QPalette::Base);
	viewport()->setPaletteBackgroundColor(defaultColors[GLine::Background]);
	viewport()->setFocusProxy(this);
	
	//oldStyle = style();
	//Q3ScrollView::setStyle(_style);
	//viewport()->setStyle(_style);
	//Q3ScrollView::setStyle(oldStyle);
	ensurePolished();
	updateViewportAttributes();
	
	margin = 0;
	doc = 0;
	_showStringIgnoreCase = false;
	_insertMode = false;
	_cellw = _cellh = 0;
	_oddLine = false;
	_dblclick = false;
	_firstLineNumber = 0;

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

	blinkTimer = new QTimer(this);
	connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blinkTimerTimeout()));
	//startBlink();

	scrollTimer = new QTimer(this);
	connect(scrollTimer, SIGNAL(timeout()), this, SLOT(scrollTimerTimeout()));

	//connect(this, SIGNAL(contentsMoving(int, int)), this, SLOT(baptizeVisible(int, int)));
}

GEditor::~GEditor()
{
	doc->unsubscribe(this);
	count--;
	if (count == 0)
	{
		delete _cache;
		delete _breakpoint;
		delete _bookmark;
		_cache = 0;
		_breakpoint = 0;
		_bookmark = 0;
		delete _style;
	}
}

void GEditor::setBreakpointPixmap(QPixmap *p)
{
	if (!_breakpoint)
		_breakpoint = new QPixmap;
		
	*_breakpoint = *p;
}

void GEditor::setBookmarkPixmap(QPixmap *p)
{
	if (!_bookmark)
		_bookmark = new QPixmap;
		
	*_bookmark = *p;
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

int GEditor::getStringWidth(const QString &s, int len, bool unicode) const
{
	int i;
	ushort c;
	double w = 0;
	int us = -1, ul = 0;
	
	//if (len < 0)
	//	len = s.length();
	
	if (len == 0)
		return 0;
	
	if (_sameWidth && !unicode)
		return ROUND_WIDTH(len * _sameWidth);
	
	for (i = 0; i < len; i++)
	{
		c = s.at(i).unicode();
		if (c != 9 && !GString::isStandardChar(c))
		{
			if (us < 0)
			{
				us = i;
				ul = 0;
			}
			ul++;
		}
		else
		{
			if (us >= 0)
			{
				w += fm.width(s.mid(us), ul);
				us = -1;
			}
			if (c == 9)
				w = (ROUND_WIDTH(w) + _tabWidth) / _tabWidth * _tabWidth;
			else
				w += _charWidth[c];
		}
	}
	
	if (us >= 0)
		w += fm.width(s.mid(us), ul);
	
	//qDebug("getStringWidth: %g / %d (%g %d)", w, fm.width(s, len), _sameWidth, unicode);
	
	return w;
}

void GEditor::updateCache()
{
	//int nw = QMAX(cache->width(), QMIN(visibleWidth(), _cellw));
	//int nh = QMAX(cache->height(), QMIN(visibleHeight(), _cellh));
	int nw = QMAX(_cache->width(), ROUND_WIDTH(visibleWidth() + _charWidth['m'] * 2));
	int nh = QMAX(_cache->height(), visibleHeight() + _cellh);
	
	if (nw > 0 && nh > 0 && (nw != _cache->width() || nh != _cache->height()))
		_cache->resize(nw, nh);
	_checkCache = false;
}

int GEditor::lineWidth(int y) const
{
	// Add 2 or a character width so that we see the cursor at end of line
	GLine *l = doc->lines.at(y);
	return margin + getStringWidth(l->s.getString(), l->s.length(), l->unicode) + (_insertMode ? _charWidth['W'] : 2);
}


int GEditor::lineWidth(int y, int len)
{
	int ns;
	
	if (len <= 0)
		return margin;
	else
	{
		GLine *l = doc->lines.at(y);
		QString s = l->s.getString();
		ns = QMAX(0, len - s.length());
		len = QMIN(len, s.length());
		
		/*if (y != lineWidthCacheY)
		{
			lineWidthCacheY = y;
			lineWidthCache.clear();
			//qDebug("y = %d", y);
		}
		
		int lw = lineWidthCache[len];
		if (!lw)
		{
			//qDebug("lineWidthCache: %d", len);
			lw = getStringWidth(s, len); //fm.width(s, len);
			lineWidthCache.insert(len, lw);
		}*/
		
		int	lw = getStringWidth(s, len, l->unicode);
		lw += margin;
		if (ns) lw += ns * _charWidth[' '];
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
	}
}
	
void GEditor::updateHeight()
{
	_cellh = fm.ascent() + fm.descent() + 3;
	//updateCache();
	_checkCache = true;
		
	updateViewport();
}

void GEditor::redrawContents()
{
	updateContents();
	//if (contentsHeight() < visibleHeight())
	//	repaintContents(contentsX(), contentsHeight(), visibleWidth(), visibleHeight() - contentsHeight() + contentsX(), true);
}

static double *get_char_width_table(QFontMetrics &fm, QFont &font)
{
	static QHash<QString, double *> cache;
	
	int i;
	QString c, fd;
	double *cw;
	
	fd = font.toString();
	
	if (cache.contains(fd))
		return cache[fd];
		
	//qDebug("get_char_width_table: %s", (const char *)fd.toUtf8());

	cw = new double[256];
	
	for (i = 0; i < 256; i++)
	{
		c = QChar(i);
		c = c.repeated(64);
		cw[i] = (double)fm.width(c) / 64;
		//qDebug("_charWidth[%d] = %g", i, _charWidth[i]);
	}
	
	cache.insert(fd, cw);
	return cw;
}

void GEditor::updateFont()
{
	QFont f;
	int i;
	QString c;
	
	normalFont = QFont(font());
	normalFont.setKerning(false);
	italicFont = QFont(font());
	italicFont.setKerning(false);
	italicFont.setItalic(true);
	
	fm = QFontMetrics(normalFont);
	_ytext = fm.ascent() + 1;
	
	_charWidth = get_char_width_table(fm, normalFont);
	
	_sameWidth = _charWidth[' '];
	_tabWidth = _charWidth['m'] * 8;
	//_charWidth[9] = _sameWidth * 8;
	
	for (i = 33; i < 127; i++)
	{
		if (_charWidth[i] != _sameWidth)
		{
			//qDebug("[%d] = %g!", i, _charWidth[i]);
			_sameWidth = 0;
			break;
		}
	}
	
	if (_sameWidth)
	{
		for (i = 160; i < 256; i++)
		{
			if (i == 173)
				continue;
			if (_charWidth[i] != _sameWidth)
			{
				//qDebug("[%d] = %g!", i, _charWidth[i]);
				_sameWidth = 0;
				break;
			}
		}
	}
	
	//qDebug("%p: sameWidth = %g tabWidth = %d", this, _sameWidth, _tabWidth);
	
	if (_sameWidth)
	{
		QString t = "AbCdEfGh01#@WwmM";
		t = t.repeated(4);
		
		_sameWidth = (double)fm.width(t) / t.length();
		//qDebug("_sameWidth -> %g", _sameWidth);
	}
	
	updateMargin();
	updateWidth();
	updateHeight();
	updateContents();
}

void GEditor::changeEvent(QEvent *e)
{
	Q3ScrollView::changeEvent(e);
	if (e->type() == QEvent::FontChange)
		updateFont();
}

static int find_last_non_space(const QString &s)
{
	int i;
	ushort c;
	
	for (i = s.length() - 1; i >= 0; i--)
	{
		c = s[i].unicode();
		if (c > 32 || c == 9)
			return i;
	}
	return -1;
}

void GEditor::paintDottedSpaces(QPainter &p, int row, int ps, int ls)
{
	QPoint pa[ls];
	int i, y;
	double x, w;

	w = _charWidth[' '];
	x = lineWidth(row, ps) + w / 2;
	y = _cellh / 2;
	for (i = 0; i < ls; i++)
	{
		pa[i].setX(ROUND_WIDTH(x));
		pa[i].setY(y);
		x += w;
	}

	p.setOpacity(0.5);
	p.drawPoints(pa, i);
	p.setOpacity(1);
}

static QColor merge_color(const QColor &ca, const QColor &cb)
{
	int r, g, b;
	
	if (cb.value() >= 128)
	{
		r = ca.red() * cb.red() / 255;
		g = ca.green() * cb.green() / 255;
		b = ca.blue() * cb.blue() / 255;
	}
	else
	{
		r = ca.red() * (255-cb.red()) / 255;
		g = ca.green() * (255-cb.green()) / 255;
		b = ca.blue() * (255-cb.blue()) / 255;
	}

	return QColor(r, g, b);
}

/*static QColor blend_color(const QColor &ca, const QColor &cb)
{
	return QColor((ca.red() + cb.red()) / 2, (ca.green() + cb.green()) / 2, (ca.blue() + cb.blue()) / 2);
}*/

static QColor calc_color(QColor ca, QColor cb, QColor cd)
{
	if (!ca.isValid() && !cb.isValid())
		return cd;
	
	if (!ca.isValid())
		return cb;
	
	if (!cb.isValid())
		return ca;
	
	return QColor((ca.red() + cb.red()) / 2, (ca.green() + cb.green()) / 2, (ca.blue() + cb.blue()) / 2);
}

void GEditor::drawTextWithTab(QPainter &p, int sx, int x, int y, const QString &s)
{
	int tp, tnp, yt = y;
	
	tp = 0;
	
	for(;;) //while (pos < s.length())
	{
		tnp = s.indexOf('\t', tp);
		if (tnp < 0)
		{
			p.drawText(x, y, s.mid(tp));
			break;
		}
		
		if (tnp > tp)
		{
			p.drawText(x, y, s.mid(tp, tnp - tp));
			x += fm.width(s.mid(tp, tnp - tp));
		}
		p.setOpacity(0.5);
		p.drawLine(x, yt - 2, x, yt);
		p.drawLine(x + 1, yt, x + 2, yt);
		p.setOpacity(1);
		x = sx + ((x - sx) + _tabWidth) / _tabWidth * _tabWidth;
		tp = tnp + 1;
	}
}

static void highlight_text(QPainter &p, int x, int x2, int h, QColor color, QColor)
{
	//int i, j;
	
	//p.setPen(color);
	
	/*for (i = -1; i <= 1; i++)
		for (j = -1; j <= 1; j++)
			p.drawText(x + i, y + j, s);*/
	//color.setAlpha(192);
	//p.setCompositionMode(QPainter::CompositionMode_Overlay);
	p.fillRect(x, 0, x2 - x, h, color);
	//p.setCompositionMode(QPainter::CompositionMode_SourceOver);
	//border.setAlpha(128);
	//p.setPen(QColor(border));
	//p.drawRect(x, 0, x2 - x, h);
}

void GEditor::paintText(QPainter &p, GLine *l, int x, int y, int xmin, int lmax, int h, int xs1, int xs2, int row, QColor &fbg)
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
	int xs = x;

	if (l->s.length() == 0)
	{
		if (l->alternate)
		{
			nx = _cellw;
			
			int x1 = x;
			int x2 = nx;
			int xd1 = xs1;
			int xd2 = xs2;
			
			if (xd1 < x1)
				xd1 = x1;
			if (xd2 > x2)
				xd2 = x2;

			p.fillRect(x, 0, nx - x, h, merge_color(_altBackground, fbg));
			
			if (xd2 > xd1)
				p.fillRect(xd1, 0, xd2 - xd1, h, styles[GLine::Selection].color);
		}
		
		return;
	}
	
	p.setFont(normalFont);

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
				bg = _altBackground;
				if (i == (GB.Count(l->highlight) - 1) && l->alternate)
					nx = _cellw;
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

				p.fillRect(x, 0, nx - x, h, merge_color(bg, fbg));
				
				if (xd2 > xd1)
					p.fillRect(xd1, 0, xd2 - xd1, h, styles[GLine::Selection].color);
			}
			
			// Highlight braces
			if (getFlag(HighlightBraces))
			{
				if (row == y1m && x1m >= 0 && x1m >= pos && x1m < (pos + len))
					highlight_text(p, lineWidth(y1m, x1m), lineWidth(y1m, x1m + 1), _cellh - 1, styles[GLine::Highlight].color, styles[GLine::Normal].color);
				if (row == y2m && x2m >= 0 && x2m >= pos && x2m < (pos + len))
					highlight_text(p, lineWidth(y2m, x2m), lineWidth(y2m, x2m + 1), _cellh - 1, styles[GLine::Highlight].color, styles[GLine::Normal].color);
			}

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
					p.setFont(italic ? italicFont : normalFont);
				}

				if (st->bold)
				{
					QColor c = st->color;
					c.setAlpha(128);
					p.setPen(c);
					drawTextWithTab(p, xs, x + 1, y, sd);
				}
				
				p.setPen(st->color);
				drawTextWithTab(p, xs, x, y, sd);
			}
			
			if (st->underline)
			{
				p.setOpacity(0.5);
				p.drawLine(x, y + 2, nx - 1, y + 2);
				p.setOpacity(1);
			}
		}

		pos += len;
		x = nx;
	}

	if (pos < (int)l->s.length() && pos < (xmin + lmax))
	{
		p.setPen(styles[GLine::Normal].color);
		//p.drawText(x, y, l->s.mid(pos).getString());
		drawTextWithTab(p, xs, x, y, l->s.mid(pos).getString());
		if (ps >= 0 && pos >= ps && show_dots)
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
			pos = l->s.indexOf(_showString, pos, !_showStringIgnoreCase);
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

static void make_blend(QImage *pix, QColor start) //, bool loop = false)
{
	double r, g, b, a, da;
	int n = pix->height();
	int i;
	QPainter p;

	pix->fill(0);

	r = start.red();
	g = start.green();
	b = start.blue();

	if (n == 0)
		n = 1;

	a = 0;
	da = (128 / ((n + 1) / 2) - 1);

	p.begin(pix);

	for (i = 0; i < ((n + 1) / 2); i++)
	{
		QBrush brush(QColor((int)r, (int)g, (int)b, qMin(255,(int)a)));
		p.fillRect(0, i, pix->width(), 1, brush);
		p.fillRect(0, n - i - 1, pix->width(), 1, brush);
		a += da;
	}

	p.end();
}

/*static QColor blend_color(QColor a, QColor b, QColor r)
{
	#define f(c) ((a.c()*r.c() + (255 - a.c())*(255 - r.c())) * b.c() / 255 / 255)
	return QColor(f(red), f(green), f(blue));
}*/

void GEditor::paintCell(QPainter &p, int row, int)
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
	bool odd;
	bool drawSep;
	int ysep, wsep;

	ur = QRect(0, row * _cellh, _cellw, _cellh);
	contentsToViewport(ur.x(), ur.y(), x1, y1);
	ur.setRect(-x1, -y1, ur.width(), ur.height());

	if (row < 0)
		realRow = row;
	else
		realRow = viewToReal(row);
	
	if (realRow < 0 || realRow >= numLines())
	{
		QColor color = viewport()->paletteBackgroundColor();
		if (flashed)
			color = QColor(QRgb(color.rgb() ^ 0x00FFFFFF));
		p.fillRect(0, 0, _cellw, _cellh, color);
		return;
	}
	
	// Colorize as soon as possible
	highlight = (doc->getHighlightMode() != GDocument::None) && (getFlag(HighlightImmediately) || !doc->isLineEditedSomewhere(realRow)); //(l->modified && realRow == y && !getFlag(HighlightCurrent)));
	if (highlight)
	{
		painting = true;
		doc->colorize(realRow);
		painting = false;
	}
	
	l = doc->lines.at(realRow);
	
	folded = l->proc && isFolded(realRow);
		
	drawSep = false;
	ysep = 0;
	if (realRow > 0)
	{
		int nextRow = viewToReal(row + 1);
		
		if (l->isEmpty() && nextRow < numLines())
		{
			GLine *l2 = doc->lines.at(nextRow);
			if (l2->proc && !isFolded(nextRow))
			{
				drawSep = true;
				ysep = _cellh / 2;
			}
		}
		else if (l->proc && (folded || !doc->lines.at(viewToReal(row - 1))->isEmpty()))
			drawSep = true;
	}

	if (getFlag(ChangeBackgroundAtLimit)) //) && doc->getHighlightMode() != GDocument::None)
	{
		if (l->proc)
			_oddLine = !_oddLine;
		odd = _oddLine;
		//drawSep = false;
	}
	else
		odd = false;
	
	//xmin = (ur.left() - margin) / charWidth;
	xmin = posToColumn(realRow, 0) - 1;
	if (xmin < 0)
		xmin = 0;
	//lmax = 2 + visibleWidth() / charWidth;
	lmax = 2 + posToColumn(realRow, visibleWidth()) - xmin;
	//if (row == 0)
	//	qDebug("%d: %d %d %d %d (%d %d)", row, ur.left(), ur.top(), ur.width(), ur.height(), xmin, lmax);

	// Line background
	if (realRow == doc->currentLine()) //l->flag & (1 << GLine::CurrentFlag))
		a = styles[GLine::Current].color;

	if (getFlag(ShowCurrentLine) && realRow == y)
		b = styles[GLine::Line].color;

	//QPainter p(cache);
	
	if (isEnabled())
		color = calc_color(a, b, odd ? _oddBackground : styles[GLine::Background].color);
	else
		color = palette().color(QPalette::Disabled, QPalette::Base);
	
	p.fillRect(0, 0, _cellw, _cellh, color);
	
	if (drawSep && getFlag(ChangeBackgroundAtLimit) && isEnabled())
	{
		if (ysep)
		{
			//int xsep = qMax(0, margin - qMin(_cellh, 12) - contentsX());
			p.fillRect(0, ysep, visibleWidth(), _cellh - ysep, !odd ? _oddBackground : styles[GLine::Background].color);
		}
		
		drawSep = false;
	}
	
	p.setFont(normalFont);
	//p.setFont(painter->font());
	//p.translate(-ur.left(), 0);

	p.translate(-contentsX(), 0);
	
	// Show string
	if ((_showRow == realRow || _showString.length()))
		paintShowString(p, l, margin, _ytext, xmin, lmax, _cellh, realRow);
	
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
					x1 = margin;
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

	// Procedure separation
	
	if (drawSep)
	{
		int xsep = 0; //qMax(0, margin - qMin(_cellh, 12) - contentsX());
		
		p.translate(contentsX(), 0);
		
		if (getFlag(BlendedProcedureLimits))
		{
			if (!_blend_pattern)
			{
				_blend_pattern = new QImage(64, _cellh / 2, QImage::Format_ARGB32_Premultiplied);
				make_blend(_blend_pattern, styles[GLine::Selection].color);
			}
			
			wsep = visibleWidth();
			
			for (i = xsep; i < wsep; i += _blend_pattern->width())
				p.drawImage(i, ysep, *_blend_pattern, 0, 0, QMIN(wsep - i, _blend_pattern->width()), _cellh - ysep);
			
			if (xs1 && xs2)
				p.fillRect(x1, 0, x2 - x1, _cellh, styles[GLine::Selection].color);
			//p.drawTiledImage(0, ysep, visibleWidth(), _cellh - ysep, pattern);
		}
		else if (getFlag(ChangeBackgroundAtLimit))
		{
			if (ysep)
			{
				p.fillRect(0, ysep, visibleWidth(), _cellh - ysep, !odd ? _oddBackground : styles[GLine::Background].color);
				if (xs1 && xs2)
					p.fillRect(x1, 0, x2 - x1, _cellh, styles[GLine::Selection].color);
			}
		}
		else if (getFlag(ShowProcedureLimits))
		{
			//QBrush brush(styles[GLine::Selection].color, Qt::Dense4Pattern);
			//p.fillRect(0, 0, cache->width(), 1, brush);
			p.setPen(styles[GLine::Selection].color);
			p.setOpacity(0.5);
			p.drawLine(xsep, ysep, visibleWidth() - 1, ysep);
			p.setOpacity(1);
		}
		
		p.translate(-contentsX(), 0);
	}

	// Margin
	if (!getFlag(HideMargin) && (margin > ur.left()))
	{
		//if (!l->flag)
		//	p.fillRect(0, 0, margin, _cellh, color); //styles[GLine::Background].color);

		if (getFlag(ShowModifiedLines))
		{
			if (l->changed || l->saved)
			{
				int w = qMax(2, margin - qMin(_cellh, 12) - 1);
				p.setOpacity(0.5);
				p.fillRect(0, 0, w, _cellh, styles[l->changed ? GLine::Breakpoint : GLine::Highlight].color);
				p.setOpacity(1);
			}
			//if (l->modified)
			//	p.fillRect((margin - 2) / 2, _cellh / 2, 1, 1, styles[GLine::Breakpoint].color);
		}
		//else
		//	p.fillRect(0, 0, margin - 2, _cellh, odd ? _oddBackground : styles[GLine::Background].color);
		/*else if (getFlag(ShowCurrentLine))
			p.fillRect(0, 0, margin - 2, _cellh, styles[GLine::Line].color);*/

		//x1 = 0;

		if (getFlag(ShowLineNumbers) && (!drawSep || ysep == 0))
		{
			int n = realRow + 1 + _firstLineNumber;
			if ((n % 10) == 0 || (l->proc && folded))
				p.setPen(styles[GLine::Normal].color);
			else
				p.setPen(styles[GLine::Selection].color);
			p.drawText(2, _ytext, QString::number(n).rightJustify(lineNumberLength));
		}
	}
	
	// Line text
	if (!highlight)
	{
		// Highlight braces
		if (getFlag(HighlightBraces))
		{
			if (realRow == y1m && x1m >= 0)
				highlight_text(p, lineWidth(y1m, x1m), lineWidth(y1m, x1m + 1), _cellh - 1, styles[GLine::Highlight].color, styles[GLine::Normal].color);
			if (realRow == y2m && x2m >= 0)
				highlight_text(p, lineWidth(y2m, x2m), lineWidth(y2m, x2m + 1), _cellh - 1, styles[GLine::Highlight].color, styles[GLine::Normal].color);
		}

		if (l->s.length())
		{
			p.setPen(styles[GLine::Normal].color);
			//p.drawText(margin + xmin * charWidth, fm.ascent() + 1, l->s.getString().mid(xmin, lmax));
			//p.drawText(QRect(lineWidth(realRow, xmin), 0, visibleWidth(), _cellh), l->s.getString().mid(xmin, lmax), textOption);
			//p.drawText(lineWidth(realRow, xmin), fm.ascent(), l->s.getString().mid(xmin, lmax));
			drawTextWithTab(p, lineWidth(realRow, 0), lineWidth(realRow, xmin), _ytext, l->s.getString().mid(xmin, lmax));
			if (getFlag(ShowDots))
			{
				i = find_last_non_space(l->s.getString()) + 1;
				if (i >= 0 && i < (xmin + lmax))
					paintDottedSpaces(p, realRow, i, QMIN(xmin + lmax, (int)l->s.length()) - i);		
			}
		}
	}
	else
	{
		paintText(p, l, margin, _ytext, xmin, lmax, _cellh, xs1, xs2, realRow, color);
	}
	
	// Folding symbol
	if (!getFlag(NoFolding) && margin && l->proc)
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
	if ((l->flag & (1 << GLine::BreakpointFlag)) && _breakpoint && !_breakpoint->isNull())
	{
		//p.fillRect(margin - 10, 0, 8, _cellh, styles[GLine::Breakpoint].color);
		//updateBreakpoint(styles[GLine::Background].color.rgb(), styles[GLine::Breakpoint].color.rgb());
		p.drawPixmap(margin - _breakpoint->width() - 2, (_cellh - _breakpoint->height()) / 2, *_breakpoint);
	}

	// Bookmark symbol
	if ((l->flag & (1 << GLine::BookmarkFlag)) && _bookmark && !_bookmark->isNull())
	{
		//p.fillRect(margin - 10, 0, 8, _cellh, styles[GLine::Breakpoint].color);
		//updateBreakpoint(styles[GLine::Background].color.rgb(), styles[GLine::Breakpoint].color.rgb());
		p.drawPixmap(margin - _bookmark->width() - 2, (_cellh - _bookmark->height()) / 2, *_bookmark);
	}

	// Text cursor
	if (_cursor && realRow == y)
	{
		QColor color = styles[GLine::Normal].color;
		int xc = lineWidth(realRow, x);
		int wc = 2;
		//p.fillRect(QMIN((int)l->s.length(), x) * charWidth + margin, 0, 1, _cellh, styles[GLine::Normal].color);
		if (_insertMode)
			wc = lineWidth(realRow, x + 1) - xc;
		
		color.setAlpha(160);
		p.fillRect(xc, 0, wc, _cellh, color);
	}

	p.translate(contentsX(), 0);
	
	// Flash
	if (flashed)
	{
		//p.setCompositionMode(QPainter::CompositionMode_Xor);
		p.fillRect(0, 0, visibleWidth(), _cellh, Qt::black);
	}


	//if (cache->width() < visibleWidth())
	//	qDebug("cache->width() = %d < visibleWidth() = %d", cache->width(), visibleWidth());
	//painter->drawPixmap(ur.left(), 0, *cache, 0, 0, cache->width(), cache->height()); //, _cellw, _cellh);
}


void GEditor::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
	int rowfirst = rowAt(cy);
	int rowlast = rowAt(cy + ch - 1);

	if (getFlag(ChangeBackgroundAtLimit) && rowfirst > 0)
		_oddLine = doc->getLimitIndex(viewToReal(rowfirst - 1)) & 1;
	else
		_oddLine = true;
	
	if (_checkCache)
		updateCache();
	
	QPainter pc(_cache);
	
	// Go through the rows
	for (int r = rowfirst; r <= rowlast; ++r) 
	{
		paintCell(pc, r, 0);
		pc.translate(0, _cellh);
	}
	
	pc.end();
	
	//ur = QRect(0, row * _cellh, _cellw, _cellh);
	//qDebug("drawContents: %d %d %d %d : %d %d . %d : %d %d", cx, cy, cw, ch, contentsX(), contentsY(), rowfirst * _cellh, viewport()->x(), viewport()->y());
	//p->setClipRect(cx, cy, cw, ch);
	p->drawPixmap(contentsX(), rowfirst * _cellh, *_cache, 0, 0, _cellw, _cellh * (rowlast - rowfirst + 1)); //, _cellw, _cellh);
	
	//qDebug("%d %d", contentsX(), contentsY());
	//p->setPen(styles[GLine::Normal].color);
	//p->drawText(contentsX(), contentsY() + 16, QString::number(y) + " : " + QString::number(x));
	
	if (_blend_pattern)
	{
		delete _blend_pattern;
		_blend_pattern = 0;
	}
}

void GEditor::checkMatching()
{
	static GString look("()[]{}");
	
	GString str;
	char c, match, search;
	int pos, len;
	bool backward;
	int old_y1m, old_y2m, old_x1m, old_x2m;
	int level;
	bool ignore;
	
	old_y1m = y1m;
	old_y2m = y2m;
	old_x1m = x1m;
	old_x2m = x2m;
	
	if (y < 0 || y >= (int)numLines())
		goto __NOT_FOUND;
	
	str = doc->lines.at(y)->s;
	len = str.length();
	
	pos = -1;
	
	if (x > 0)
	{
		c = str.at(x - 1);
		pos = look.indexOf(c);
		x1m = x - 1;
	}
	
	if (pos < 0 && x < len)
	{
		c = str.at(x);
		pos = look.indexOf(c);
		x1m = x;
	}
	
	if (pos < 0)
		goto __NOT_FOUND;
	
	y1m = y;
	
	backward = pos & 1;
	match = c;
	search = look.at(pos + (backward ? -1 : 1));
	
	//fprintf(stderr, "find '%c' at pos %d. Searching %s for '%c'\n", match, x1m, backward ? "backward" : "forward", search);
	
	y2m = y1m;
	x2m = x1m;
	level = 0;
	ignore = false;
	
	for(;;)
	{
		if (backward)
		{
			x2m--;
			if (x2m < 0)
			{
				y2m--;
				if (y2m < 0)
					goto __NOT_FOUND;
				str = doc->lines.at(y2m)->s;
				len = str.length();
				x2m = len;
				continue;
			}
		}
		else
		{
			x2m++;
			if (x2m >= len)
			{
				y2m++;
				if (y2m >= (int)numLines())
					goto __NOT_FOUND;
				str = doc->lines.at(y2m)->s;
				len = str.length();
				x2m = -1;
				continue;
			}
		}
		
		c = str.at(x2m);

		if (ignore)
		{
			if (c == '"' && (x2m == 0 || str.at(x2m - 1) != '\\'))
				ignore = false;
			continue;
		}
		
		if (c == search)
		{
			if (level <= 0)
				goto __FOUND;
			level--;
		}
		else if (c == match)
		{
			level++;
		}
		else if (c == '"')
		{
			ignore = true;
		}
	}
	
__NOT_FOUND:

	y1m = y2m = -1;
	x1m = x2m = -1;
	
__FOUND:

	if (y1m != old_y1m || y2m != old_y2m || x1m != old_x1m || x2m != old_x2m)
	{
		if (old_y1m >= 0) updateLine(old_y1m);
		if (old_y2m >= 0) updateLine(old_y2m);
		if (y1m >= 0) updateLine(y1m);
		if (y2m >= 0) updateLine(y2m);
		//fprintf(stderr, "y1m = %d x1m = %d y2m = %d x2m = %d\n", y1m, x1m, y2m, x2m);
	}
}

#if 0
void GEditor::oldCheckMatching()
{


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
#endif

void GEditor::leaveCurrentLine()
{
	if (y < 0 || y >= numLines())
		return;
	
	doc->colorize(y, true);
	
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
			if (ny >= numLines())
				ny = y;
			else
				nx = 0;
		}
	}
	
	if (ny < 0)
	{
		nx = QMAX(0, nx);
		ny = 0;
	}
	else if (ny >= numLines())
	{
		ny = numLines() - 1;
		nx = lineLength(ny);
	}
	else
		ny = checkFolded(ny);
	
	if (nx < 0)
		nx = 0;
	else 
	{
		int xmax = _insertMode ? QMAX((int)(_cellw / _charWidth[' ']) + 1, lineLength(largestLine)) : lineLength(ny);
		
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
		
		if (oy != y && !doc->insideUndo())
			_cutBuffer.clear();
		
		emit cursorMoved();
	}
	else if (center)
		ensureCursorVisible();

	checkMatching();
	
	return change;
}

void GEditor::insert(QString text)
{
	doc->begin();
	doc->eraseSelection(_insertMode);
	unfoldLine(y);
	doc->insert(y, x, text);
	doc->end();
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
		if (ctrl)
			movePreviousSameIndent(shift);
		else
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
	{
		int yl = doc->getPreviousLimit(y);
		if (yl >= 0)
			cursorGoto(yl, xx, shift);
	}
	else
		cursorGoto(viewToReal(realToView(y) - 1), xx, shift);
}


void GEditor::cursorDown(bool shift, bool ctrl, bool alt)
{
	if (alt)
	{
		if (ctrl)
			moveNextSameIndent(shift);
		else
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
		cursorGoto(QMIN(numLines() - 1, viewToReal(realToView(y) + 1)), xx, shift);
	else
	{
		int yl = doc->getNextLimit(y);
		
		if (yl >= 0)
			cursorGoto(yl, xx, shift);
		else
			cursorGoto(numLines(), 0, shift);
	}
}

void GEditor::movePreviousSameIndent(bool shift)
{
	int indent = doc->getIndent(y);
	int i2, y2;
	
	for(y2 = y - 1; y2 >= 0; y2--)
	{
		i2 = doc->getIndent(y2);
		if (i2 == indent && i2 < doc->lineLength(y2))
		{
			cursorGoto(y2, x, shift);
			break;
		}
	}
}

void GEditor::cursorHome(bool shift, bool ctrl, bool alt)
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

void GEditor::moveNextSameIndent(bool shift)
{
	int indent = doc->getIndent(y);
	int i2, y2;
	
	for(y2 = y + 1; y2 < numLines(); y2++)
	{
		i2 = doc->getIndent(y2);
		if (i2 == indent && i2 < doc->lineLength(y2))
		{
			cursorGoto(y2, x, shift);
			break;
		}
	}
}

void GEditor::cursorEnd(bool shift, bool ctrl, bool alt)
{
	if (ctrl)
		cursorGoto(numLines(), 0, shift);
	else
		cursorGoto(y, lineLength(y), shift);
}

void GEditor::cursorPageUp(bool shift, bool alt)
{
	int page = visibleHeight() / _cellh;
	cursorGoto(viewToReal(realToView(y) - page), 0, shift);
}

void GEditor::cursorPageDown(bool shift, bool alt)
{
	int page = visibleHeight() / _cellh;
	cursorGoto(viewToReal(realToView(y) + page), 0, shift);
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
	
	doc->begin();
	
	if (x >= lineLength(y))
	{
		if (y < (numLines() - 1))
		{
			if (_insertMode)
				doc->insert(y, x, QString(), TRUE);
			doc->remove(y, x, y + 1, 0);
		}
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
	
	doc->end();
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

	doc->begin();
	
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
	
	doc->end();
}

void GEditor::selectCurrentLine()
{
	cursorGoto(y, 0, FALSE);
	cursorGoto(y + 1, 0, TRUE);
}

void GEditor::deleteCurrentLine()
{
	bool im;

	if (doc->hasSelection())
	{
		doc->eraseSelection(_insertMode);
		return;
	}

	im = _insertMode;
	_insertMode = FALSE;
	doc->begin();
	selectCurrentLine();
	del(FALSE);
	doc->end();
	_insertMode = im;
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
			return;
		}

		doc->startSelection(this, y, 0);
		doc->endSelection(y + 1, 0);
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
	if (!doc->hasSelection())
	{
		doc->begin();
		selectCurrentLine();
		_cutBuffer += doc->getSelectedText(_insertMode);
		QApplication::clipboard()->setText(_cutBuffer.getString(), QClipboard::Clipboard);
		doc->eraseSelection(_insertMode);
		doc->end();
		return;
	}

	copy(false);
	doc->eraseSelection(_insertMode);
}

void GEditor::paste(bool mouse)
{
	QString text;
	GString gtext;
	QString subType("plain");
	int i, i2, xs, len;
	QString tab;

	text = QApplication::clipboard()->text(subType, mouse ? QClipboard::Selection : QClipboard::Clipboard);

	if (text.length() == 0)
		return;
	
	tab.fill(' ', doc->getTabWidth());
	text.replace("\t", tab);

	for (i = 0; i < text.length(); i++)
	{
		if ((text[i] < ' ' || text[i].isSpace()) && text[i] != '\n' && text[i] != '\r')
			text[i] = ' ';
	}

	if (_insertMode)
	{
		gtext = text;
		doc->begin();
		i = 0;
		while (i < (int)text.length())
		{
			i2 = gtext.findNextLine(i, len);
			xs = x;
			insert(text.mid(i, len));
			x = xs;
			y++;
			if (y >= doc->numLines())
				insert("\n");
			i = i2;
		}
		doc->end();
	}
	else
		insert(text);
}

void GEditor::undo()
{
	doc->undo();
	//cursorGoto(doc->yAfter, doc->xAfter, false);
}

void GEditor::redo()
{
	doc->redo();
	//cursorGoto(doc->yAfter, doc->xAfter, false);
}

void GEditor::selectAll()
{
	cursorGoto(0, 0, false);
	cursorGoto(numLines(), 0, true);
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
				cursorHome(shift, ctrl, alt); return;
			case Qt::Key_End:
				cursorEnd(shift, ctrl, alt); return;
			case Qt::Key_Prior:
				cursorPageUp(shift, alt); return;
			case Qt::Key_Next:
				cursorPageDown(shift, alt); return;
			case Qt::Key_Return:
				if (ctrl)
					expand(shift);
				return;
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
		if (t.length() && (t.at(0).isPrint() || (t.at(0).unicode() == 9 && ctrl))
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
				newLine(); return;
			case Qt::Key_Return:
				if (ctrl)
					expand(shift);
				else
					newLine();
				return;
			case Qt::Key_Home:
				cursorHome(shift, ctrl, alt); return;
			case Qt::Key_End:
				cursorEnd(shift, ctrl, alt); return;
			case Qt::Key_Backspace:
				backspace(ctrl); return;
			case Qt::Key_Delete:
				del(ctrl); return;
			case Qt::Key_Prior:
				cursorPageUp(shift, alt); return;
			case Qt::Key_Next:
				cursorPageDown(shift, alt); return;
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
			case Qt::Key_D:
				deleteCurrentLine(); return;
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
		return (px - margin) / _charWidth[' '];
		
	px += contentsX();
	
	d = 0;
	f = len;
	while (f > d)
	{
		if (i < 0)
			i = px / _charWidth['m'];
		else
			i = (d + f) / 2;
		
		lw = lineWidth(y, i);
		if (px < lw)
		{
			f = i;
			continue;
		}
		
		/*if (_sameWidth)
			lw += _sameWidth;
		else*/
			lw = lineWidth(y, i + 1); //+= getStringWidth(s.mid(i, 1)); //fm.width(s[i]);
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
		viewport()->setCursor(_saveCursor);
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
	
	if (!left)
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
	
	_dblclick = true;
	
	if (left)
	{
		ny = posToLine(e->pos().y());
		if (!getFlag(NoFolding) && doc->lines.at(ny)->proc)
		{
			if (isFolded(ny))
				foldAll();
			else
				unfoldAll();
		}
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
	}
	else
	{
		if (left && !_dblclick)
		{
			int ny = posToLine(e->pos().y());
			if (!getFlag(NoFolding) && doc->lines.at(ny)->proc)
			{
				if (isFolded(ny))
					unfoldLine(ny);
				else
					foldLine(ny);
			}
			emit marginClicked(ny);
		}

		if (e->button() == Qt::MidButton)
			paste(true);
	}
	
	_dblclick = false;
}


void GEditor::resizeEvent(QResizeEvent *e)
{
	Q3ScrollView::resizeEvent(e);
	//baptizeVisible();
	updateWidth();
}

void GEditor::viewportResizeEvent(QResizeEvent *e)
{
	Q3ScrollView::viewportResizeEvent(e);
	updateWidth();
	if (!_ensureCursorVisibleLater)
	{
		_ensureCursorVisibleLater = true;
		QTimer::singleShot(0, this, SLOT(ensureCursorVisible()));
	}
	_checkCache = true;
}

bool GEditor::isCursorVisible()
{
	int px, py;
	
	cursorToPos(y, x, &px, &py);
	
	return !(px < margin || px > (visibleWidth() - QMAX(2, margin)) || py < 0 || py > (visibleHeight() - _cellh));
}

void GEditor::ensureCursorVisible()
{
	int xx, yy;
	
	if (!isUpdatesEnabled() || !isVisible())
		return;
	
	xx = lineWidth(y, x); // + _charWidth['m'] / 2
	yy = realToView(y) * _cellh + _cellh / 2;
	
	//fprintf(stderr, "%p: xx = %d yy = %d vw = %d vh = %d center = %d contentX() = %d", this, xx, yy, visibleWidth(), visibleHeight(), center, contentsX());
	
	if (xx >= visibleWidth() || contentsX() > 0)
		ensureVisible(xx, yy, margin + 2, center ? (visibleHeight() / 2) : _cellh);
	else
		ensureVisible(0, yy, margin + 2, center ? (visibleHeight() / 2) : _cellh);
	
	//fprintf(stderr, " -> %d\n", contentsX());
	
	center = false;
	_ensureCursorVisibleLater = false;
}

void GEditor::startBlink()
{
	blinkTimer->start(QApplication::cursorFlashTime() / 2, false);
	_cursor = true;
	updateLine(y);
}

void GEditor::stopBlink()
{
	blinkTimer->stop();
	_cursor = false;
	updateLine(y);
}

void GEditor::blinkTimerTimeout()
{
	if (!doc->isReadOnly())
	{
		_cursor = !_cursor;
		updateLine(y);
	}
}

void GEditor::focusInEvent(QFocusEvent *e)
{
	startBlink();
	//ensureCursorVisible();
	Q3ScrollView::focusInEvent(e);
	doc->setCurrentView(this);
}

void GEditor::focusOutEvent(QFocusEvent *e)
{
	stopBlink();
	//leaveCurrentLine();
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
	int sat;
	int gray;
	
	if (index < 0 || index >= GLine::NUM_STATE)
		return;

	styles[index] = *style;

	if (index == GLine::Background)
	{
		viewport()->setPaletteBackgroundColor(style->color);
		redrawContents();
	}
	else
		updateContents();
	
	if (index == GLine::Background)
	{
		_oddBackground = style->color;

		sat = _oddBackground.saturation();
		gray = 128 + (_oddBackground.value() - 128) * 0.8;

		_altBackground = QColor(gray, gray, gray);
		
		if (_oddBackground.value() > 127)
			_oddBackground.setHsv(_oddBackground.hue(), sat, _oddBackground.value() - 16);
		else
			_oddBackground.setHsv(_oddBackground.hue(), sat, _oddBackground.value() + 16);
	}
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

	if (getFlag(NoFolding))
		unfoldAll();
	
	updateMargin();
	updateContents();
}

void GEditor::updateMargin()
{
	int charWidth = _charWidth['m'];
	int nm = 1, lnl = 0;
	
	if (!getFlag(HideMargin))
	{
		nm = 2;
	
		//if (doc->getHighlightMode() != GDocument::None)
		{
			int bw = 8;
			
			if (_breakpoint && !_breakpoint->isNull())
				bw = qMax(bw, _breakpoint->width() + 2);
			
			if (_bookmark && !_bookmark->isNull())
				bw = qMax(bw, _bookmark->width() + 2);
			
			nm += bw;
		}
		//else if (getFlag(ShowLineNumbers))
		//	nm += 4;
		
		if (getFlag(ShowLineNumbers))
		{
			int cnt = numLines() + _firstLineNumber;

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
	}

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

void GEditor::inputMethodEvent(QInputMethodEvent *e)
{
	//qDebug("inputMethodEvent: %s\n", (const char *)e->commitString().toUtf8());

	if (doc->isReadOnly())
	{
		e->ignore();
		return;
	}

	insert(e->commitString());
	
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
			return QVariant(QString());
		case Qt::ImAnchorPosition:
			return QVariant(x);
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
	
	if (getFlag(NoFolding))
		return;
	
	if (!doc->hasLimit(row))
		row = doc->getPreviousLimit(row);
	
	if (row < 0 || row >= numLines())
		return;
	
	//fprintf(stderr, "foldLine %d\n", row);
	
	start = row;
	end = doc->getNextLimit(row);
	if (end < 0)
		end = numLines() - 1;
	else
		end--;
	/*else
	{
		while (end > start)
		{
			end--;
			l = doc->lines.at(end);
			//qDebug("[%d] state = %d %d", end, l->highlight ? l->highlight[0].state : -1, l->highlight ? l->highlight[0].len : -1);
			if (!l->highlight || (l->highlight[0].state != GLine::Comment && l->highlight[0].state != GLine::Help))
				break;
		}
	}*/
	
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
	
	for (i = 0; i < fold.count(); i++)
	{
		fp = fold.at(i);
		if (row <= fp->start)
			break;
		if (fp->end < (numLines() - 1))
			row += fp->end - fp->start;
		else
			row = numLines();
	}
	
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
	
	if (getFlag(NoFolding))
		return;
	
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
	
	if (getFlag(NoFolding))
		return;
	
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
	
	if (getFlag(NoFolding))
		return;
	
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

#if 0
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

void GEditor::viewportPaintEvent(QPaintEvent *e)
{
	QString info;
	QRect rect;
	QColor color;
	
	Q3ScrollView::viewportPaintEvent(e);
	
	if (getFlag(ShowCursorPosition))
	{
		getInfo(&rect, &info);
		if (rect.intersects(e->rect()))
		{
			QPainter p(viewport());
			color = styles[GLine::Current].color;
			color.setAlpha(128);
			p.fillRect(rect, color);
			//rect.translate(0, 2);
			p.setPen(styles[GLine::Normal].color);
			p.drawText(rect, Qt::AlignCenter, info);
			/*color = styles[GLine::Normal].color;
			color.setAlpha(128);
			p.setPen(color);
			//rect.translate(0, -2);
			p.drawRect(rect);*/
		}
	}
}
#endif

void GEditor::updateViewport()
{
	int vw, vh;
	
	/*vw = contentsRect().width();
	vh = contentsRect().height();
	
	if (doc)
	{
		vw = QMAX(_cellw, vw);
		vh = QMAX(_cellh * _nrows, vh);
		//qDebug("updateViewport: h = %d  vh = %d", _cellh * numLines(), contentsRect().height());
	}*/
	
	vw = QMAX(visibleWidth(), _cellw);
	vh = QMAX(visibleHeight(), _cellh * _nrows);
	
	if (vw != contentsWidth() || vh != contentsHeight())
		Q3ScrollView::resizeContents(vw, vh);

	//updateCache();
		_checkCache = true;
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

void GEditor::updateViewportAttributes()
{
	bool b;
	
	//if (!::strcmp(style()->metaObject()->className(), "Oxygen::Style"))
		b = !hasBorder();
	//else
	//	b = true;
	
	viewport()->setAttribute(Qt::WA_NoSystemBackground, b);
	viewport()->setAttribute(Qt::WA_PaintOnScreen, b);
}

void GEditor::setBorder(bool b)
{
	if (_border == b)
		return;
	
	style()->unpolish(this);
	setFrameStyle(b ? StyledPanel + Sunken : NoFrame);
	style()->polish(this);
	updateViewportAttributes();
}

void GEditor::setLineOffset(int l)
{
	_firstLineNumber = l;
	update();
}

void GEditor::expand(bool shift)
{
	bool e;

	e = isFolded(y);

	if (shift)
	{
		if (e)
			unfoldAll();
		else
			foldAll();
	}
	else
	{
		if (e)
			unfoldLine(y);
		else
			foldLine(y);
	}
}

void GEditor::saveCursor()
{
	_saveCursor = viewport()->cursor();
}
