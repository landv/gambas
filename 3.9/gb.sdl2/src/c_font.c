/***************************************************************************

  c_font.c

  (c) 2014 Benoît Minisini <gambas@users.sourceforge.net>

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

#define __C_FONT_C

#include "default_font.h"
#include "c_font.h"

typedef
	struct {
		LIST list;
		char *path;
		char *name;
	}
	FONT_LOAD;

static FONT_LOAD *_font_list = NULL;

#define THIS ((CFONT *)_object)

static void create_cache(CFONT *_object)
{
	if (!THIS->cache)
		GB.HashTable.New(&THIS->cache, GB_COMP_BINARY);
}

CFONT *FONT_create()
{
	CFONT *font = (CFONT *)GB.New(CLASS_Font, NULL, NULL);
	create_cache(font);
	return font;
}

static FONT_LOAD *find_font(const char *name)
{
	FONT_LOAD *p;

	LIST_for_each(p, _font_list)
	{
		if (strcasecmp(name, p->name) == 0)
			return p;
	}

	return NULL;
}

static bool load_font(char *path, char *name)
{
	FONT_LOAD *load;
	char *addr;
	int len;

	if (GB.LoadFile(path, strlen(path), &addr, &len))
		return TRUE;

	GB.ReleaseFile(addr, len);

	if (!name || !*name)
	{
		char *p = strrchr(path, '/');
		if (p)
			name = p + 1;
		else
			name = path;

		p = strrchr(name, '.');
		if (p)
			name = GB.TempString(name, p - name);
	}

	if (find_font(name))
	{
		GB.Error("Font already exists");
		return TRUE;
	}

	GB.AllocZero(POINTER(&load), sizeof(FONT_LOAD));
	load->path = GB.NewZeroString(path);
	load->name = GB.NewZeroString(name);
	LIST_insert(&_font_list, load, &load->list);

	return FALSE;
}

static void release_cache(CFONT *_object)
{
	GB.HashTable.Enum(THIS->cache, (GB_HASHTABLE_ENUM_FUNC)SDL_FreeImage);
	GB.HashTable.Free(&THIS->cache);
}

static bool init_font()
{
	if (!TTF_WasInit() && TTF_Init())
	{
		GB.Error("Unable to initialize TTF library: &1", TTF_GetError());
		return TRUE;
	}
	else
		return FALSE;
}

static bool check_font(CFONT *_object)
{
	FONT_LOAD *font;
	int style;
	char *addr;
	int len;

	if (!THIS->dirty)
		return FALSE;

	if (THIS->font)
	{
		if (init_font())
			return TRUE;
		TTF_CloseFont(THIS->font);
		THIS->font = NULL;
	}

	release_cache(THIS);
	create_cache(THIS);

	if (!THIS->name)
	{
		THIS->dirty = FALSE;
		return FALSE;
	}

	font = find_font(THIS->name);
	if (!font)
	{
		GB.Error("Unknown font: &1", THIS->name);
		return TRUE;
	}

	if (GB.LoadFile(font->path, strlen(font->path), &addr, &len))
		return TRUE;

	if (init_font())
		return TRUE;

	THIS->font = TTF_OpenFontRW(SDL_RWFromConstMem(addr, len), TRUE, THIS->size);
	if (!THIS->font)
	{
		GB.Error("Unable to load font: &1: &2", THIS->name, TTF_GetError());
		return TRUE;
	}

	style = TTF_STYLE_NORMAL;
	if (THIS->bold) style |= TTF_STYLE_BOLD;
	if (THIS->italic) style |= TTF_STYLE_ITALIC;
	TTF_SetFontStyle(THIS->font, style);

	THIS->dirty = FALSE;

	return FALSE;
}

static int get_font_ascent(void *_object)
{
	if (THIS->font)
		return TTF_FontAscent(THIS->font);
	else
		return DEFAULT_FONT_ASCENT * THIS->size / DEFAULT_FONT_HEIGHT;
}

static int get_font_descent(void *_object)
{
	if (THIS->font)
		return TTF_FontDescent(THIS->font);
	else
		return DEFAULT_FONT_DESCENT * THIS->size / DEFAULT_FONT_HEIGHT;
}

static void get_text_size(void *_object, const char *text, int *width, int *height)
{
	if (!text || !*text)
	{
		*width = 0;
		*height = get_font_ascent(THIS) + get_font_descent(THIS);
		return;
	}

	if (THIS->font)
	{
		TTF_SizeUTF8(THIS->font, text, width, height);
		return;
	}

	*width = UTF8_get_length(text, strlen(text)) * DEFAULT_FONT_WIDTH * THIS->size / DEFAULT_FONT_HEIGHT;
	*height = THIS->size;
}


SDL_Image *FONT_render_text(CFONT *_object, CWINDOW *window, char *text, int len, int *w, int *h)
{
	SDL_Surface *surface;
	SDL_Image *image;
	bool exist;

	if (check_font(THIS))
		return NULL;

	exist = !GB.HashTable.Get(THIS->cache, text, len, POINTER(&image));

	if (exist)
	{
		if (image->window == window)
		{
			SDL_QueryTexture(image->texture, NULL, NULL, w, h);
			if (!THIS->name)
			{
				*w = *w * THIS->size / DEFAULT_FONT_HEIGHT;
				*h = *h * THIS->size / DEFAULT_FONT_HEIGHT;
			}

			return image;
		}

		SDL_FreeImage(image);
		GB.HashTable.Remove(THIS->cache, text, len);
	}

	if (THIS->name)
	{
		SDL_Color color = { 0xFF, 0xFF, 0xFF, 0xFF };
		char c = text[len];
		text[len] = 0;
		surface = TTF_RenderUTF8_Blended(THIS->font, text, color);
		text[len] = c;
		*w = surface->w;
		*h = surface->h;
	}
	else
	{
		int size = UTF8_get_length(text, len);

		surface = SDL_CreateRGBSurface(0, size * DEFAULT_FONT_WIDTH, DEFAULT_FONT_HEIGHT, 32, RMASK, GMASK, BMASK, AMASK);

		if (SDL_MUSTLOCK(surface))
			SDL_LockSurface(surface);

		FONT_render_default((uint *)surface->pixels, size, text, len);

		if (SDL_MUSTLOCK(surface))
			SDL_UnlockSurface(surface);

		*w = surface->w * THIS->size / DEFAULT_FONT_HEIGHT;
		*h = surface->h * THIS->size / DEFAULT_FONT_HEIGHT;
	}

	image = SDL_CreateImage(surface);

	if (GB.HashTable.Count(THIS->cache) >= 128)
	{
		release_cache(THIS);
		create_cache(THIS);
	}

	GB.HashTable.Add(THIS->cache, text, len, image);
	return image;
}

//-------------------------------------------------------------------------

BEGIN_METHOD_VOID(Font_exit)

	FONT_LOAD *next;

	while (_font_list)
	{
		next = _font_list->list.next;
		GB.FreeString(&_font_list->path);
		GB.FreeString(&_font_list->name);
		GB.Free(POINTER(&_font_list));
		_font_list = next;
	}

END_METHOD

BEGIN_METHOD(Font_Load, GB_STRING path; GB_STRING name)

	char *path = GB.ToZeroString(ARG(path));
	char *name;

	if (MISSING(name))
		name = NULL;
	else
		name = GB.ToZeroString(ARG(name));

	load_font(path, name);

END_METHOD

BEGIN_METHOD(Font_get, GB_STRING font)

	CFONT *font;
	char *desc = GB.ToZeroString(ARG(font));
	char *elt;
	int val;
	bool bold = FALSE;
	bool italic = FALSE;
	int size = 0;
	char *name = NULL;

	for (elt = strtok(desc, ","); elt; elt = strtok(NULL, ","))
	{
		if (strcasecmp(elt, "bold") == 0)
		{
			bold = TRUE;
			continue;
		}

		if (strcasecmp(elt, "italic") == 0)
		{
			italic = TRUE;
			continue;
		}

		val = atoi(elt);
		if (val)
		{
			size = val;
			continue;
		}

		if (name)
		{
			GB.Error("Font name defined twice");
			goto ERROR;
		}

		name = GB.NewZeroString(elt);
	}

	if (size < 1 || size > 1024)
	{
		GB.Error("Incorrect font size");
		goto ERROR;
	}

	font = FONT_create();
	font->name = name;
	font->bold = bold;
	font->italic = italic;
	font->size = size;
	font->dirty = TRUE;

	GB.ReturnObject(font);
	return;

ERROR:

	GB.FreeString(&name);
	return;

END_METHOD

BEGIN_METHOD_VOID(Font_free)

	release_cache(THIS);

	if (THIS->font)
		TTF_CloseFont(THIS->font);

	GB.FreeString(&THIS->name);

END_METHOD

BEGIN_PROPERTY(Font_Size)

	if (READ_PROPERTY)
		GB.ReturnInteger(THIS->size);
	else
	{
		int size = VPROP(GB_INTEGER);
		if (size < 1 || size > 1024)
			GB.Error("Incorrect font size");
		else if (THIS->size != size)
		{
			THIS->size = size;
			if (THIS->name)
				THIS->dirty = TRUE;
		}
	}

END_PROPERTY

BEGIN_PROPERTY(Font_Bold)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->bold);
	else if (THIS->bold != VPROP(GB_BOOLEAN))
	{
		THIS->bold = VPROP(GB_BOOLEAN);
		if (THIS->font && !THIS->dirty)
			TTF_SetFontStyle(THIS->font, (TTF_GetFontStyle(THIS->font) ^ TTF_STYLE_BOLD));
	}

END_PROPERTY

BEGIN_PROPERTY(Font_Italic)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->italic);
	else if (THIS->italic != VPROP(GB_BOOLEAN))
	{
		THIS->italic = VPROP(GB_BOOLEAN);
		if (THIS->font && !THIS->dirty)
			TTF_SetFontStyle(THIS->font, (TTF_GetFontStyle(THIS->font) ^ TTF_STYLE_ITALIC));
	}

END_PROPERTY

BEGIN_PROPERTY(Font_Name)

	if (READ_PROPERTY)
		GB.ReturnString(THIS->name);
	else
	{
		GB.StoreString(PROP(GB_STRING), &THIS->name);
		THIS->dirty = TRUE;
	}

END_PROPERTY

BEGIN_PROPERTY(Font_Ascent)

	if (check_font(THIS))
		return;

	GB.ReturnInteger(TTF_FontAscent(THIS->font));

END_PROPERTY

BEGIN_PROPERTY(Font_Descent)

	if (check_font(THIS))
		return;

	GB.ReturnInteger(TTF_FontDescent(THIS->font));

END_PROPERTY

BEGIN_PROPERTY(Font_Fixed)

	if (check_font(THIS))
		return;

	GB.ReturnBoolean(TTF_FontFaceIsFixedWidth(THIS->font));

END_PROPERTY

BEGIN_METHOD(Font_TextWidth, GB_STRING text)

	int w, h;

	if (check_font(THIS))
		return;

	get_text_size(THIS, GB.ToZeroString(ARG(text)), &w, &h);
	GB.ReturnInteger(w);

END_METHOD

BEGIN_METHOD(Font_TextHeight, GB_STRING text)

	int w, h;

	if (check_font(THIS))
		return;

	get_text_size(THIS, GB.ToZeroString(ARG(text)), &w, &h);
	GB.ReturnInteger(h);

END_METHOD

//-------------------------------------------------------------------------

GB_DESC FontDesc[] =
{
	GB_DECLARE("Font", sizeof(CFONT)), GB_NOT_CREATABLE(),

	GB_CONSTANT("DefaultHeight", "i", DEFAULT_FONT_HEIGHT),

	GB_STATIC_METHOD("Load", NULL, Font_Load, "(Path)s[(Name)s]"),
	GB_STATIC_METHOD("_get", "Font", Font_get, "(Font)s"),
	GB_STATIC_METHOD("_exit", NULL, Font_exit, NULL),

	GB_METHOD("_free", NULL, Font_free, NULL),

	GB_PROPERTY("Size", "i", Font_Size),
	GB_PROPERTY("Bold", "b", Font_Bold),
	GB_PROPERTY("Italic", "b", Font_Italic),
	//GB_PROPERTY("Underline", "b", Font_Underline),

	GB_PROPERTY_READ("Name", "s", Font_Name),
	GB_PROPERTY_READ("Ascent", "i", Font_Ascent),
	GB_PROPERTY_READ("Descent", "i", Font_Descent),
	GB_PROPERTY_READ("Fixed", "b", Font_Fixed),

	GB_METHOD("TextWidth", "i", Font_TextWidth, "(Text)s"),
	GB_METHOD("TextHeight", "i", Font_TextHeight, "(Text)s"),
	/*GB_METHOD("GetImage", "Image", Font_GetImage, "(Text)s"),*/

	GB_END_DECLARE
};
