/***************************************************************************

  gb.form.picture.h

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

#define __GB_FORM_PICTURE_H

#define MAX_KEY 255

#define STOCK_PREFIX "icon:/"
#define STOCK_PREFIX_LEN 6

/***************************************************************************

#define LOAD_IMAGE_FUNC
Name of the global function that loads an image.

#define IMAGE_TYPE
Datatype of the internal image type.

#define CREATE_IMAGE_FROM_MEMORY(_image, _addr, _len, _ok)
Creates an image from data stored in memory.

#define DELETE_IMAGE(_image)
Deletes an image.

#define CREATE_PICTURE_FROM_IMAGE(_cpicture, _image)
How to create a CPICTURE object from the internal image type.

#define GET_FROM_STOCK(_name, _len)
Returns an image stored in the internal library stock.

#define GET_FROM_CACHE(_key)
Returns the CPICTURE object associated with the specified key in the cache.

#define INSERT_INTO_CACHE(_key, _cpicture)
Insert the CPICTURE object into the cache, under the specified key.

#define APPLICATION_THEME
The application theme string.

***************************************************************************/

static GB_FUNCTION _stock_get_func;

static bool init_stock()
{
	static bool init = false;
	static bool error = false;
	
	if (init)
	 return error;
	
	if (!GB.ExistClass("Stock"))
	{
    error = true;
	}
	else
	{
    error = GB.GetFunction(&_stock_get_func, (void *)GB.FindClass("Stock"), "_get", "ss", "Picture");
    init = true;
  }

  return error;
}

bool LOAD_IMAGE_FUNC(IMAGE_TYPE **p, const char *path, int lenp)
{
  char *addr;
  int len;
  bool ok;
  char *path_theme;
  int pos;
  
  *p = 0;
  
  if (APPLICATION_THEME && lenp > 0 && path[0] != '/')
  {
  	pos = lenp - 1;
  	while (pos >= 0)
  	{
  		if (path[pos] == '.')
  			break;
			pos--;
  	}
  	path_theme = GB.NewString(path, pos >= 0 ? pos : lenp);
  	path_theme = GB.AddChar(path_theme, '_');
  	path_theme = GB.AddString(path_theme, APPLICATION_THEME, GB.StringLength(APPLICATION_THEME));
  	if (pos >= 0)
  		path_theme = GB.AddString(path_theme, &path[pos], lenp - pos);
		ok = !GB.LoadFile(path_theme, GB.StringLength(path_theme), &addr, &len);
		GB.Error(NULL);
		GB.FreeString(&path_theme);
		if (ok)
			goto __LOAD;
  }

	GB.Error(NULL);
	if (GB.LoadFile(path, lenp, &addr, &len))
	{
		GB.Error(NULL);
		return FALSE;
	}
			
__LOAD:

	CREATE_IMAGE_FROM_MEMORY(*p, addr, len, ok)
	
	GB.ReleaseFile(addr, len);
  return ok;
}


static CPICTURE *get_picture(const char *path, int len)
{
  CPICTURE *pict = NULL;
  char key[MAX_KEY + 1];
  GB_VALUE *value;
  IMAGE_TYPE *img;

	if (len <= 0)
		return NULL;
	
  snprintf(key, sizeof(key), "%s\n%.*s", GB.Component.Current(), len, path);
	//fprintf(stderr, "get_picture: %s\n", key);

	pict = GET_FROM_CACHE(key);
  if (!pict)
  {
		#ifdef GET_FROM_STOCK
		img = GET_FROM_STOCK(path, len);
		if (img)
		{
			CREATE_PICTURE_FROM_IMAGE(pict, img);
		}
		else
		#endif 
		if (len >= STOCK_PREFIX_LEN && strncmp(path, STOCK_PREFIX, STOCK_PREFIX_LEN) == 0)
		{
			if (len == STOCK_PREFIX_LEN)
				goto __RETURN;

			if (init_stock())
				goto __RETURN;

			GB.Push(1, GB_T_STRING, &path[STOCK_PREFIX_LEN], len - STOCK_PREFIX_LEN);
			value = GB.Call(&_stock_get_func, 1, false);
			if (value->type >= GB_T_OBJECT)
				pict = (CPICTURE *)((GB_OBJECT *)value)->value;
			if (!pict)
				goto __RETURN;
		}
		else
		{
			LOAD_IMAGE_FUNC(&img, path, len);
			if (!img)
				goto __RETURN;
			
			CREATE_PICTURE_FROM_IMAGE(pict, img);
			DELETE_IMAGE(img);
			
			//if (img)
			//	fprintf(stderr, "CREATE_PICTURE_FROM_IMAGE: %p (%d %d)\n", img, img->width(), img->height());
			//if (pict)
			//	fprintf(stderr, "CREATE_PICTURE_FROM_IMAGE: -> %p (%d %d)\n", pict, pict->pixmap->width(), pict->pixmap->height());
		}

    INSERT_INTO_CACHE(key, pict);
  }

__RETURN:

	//fprintf(stderr, "get_picture: -> %p\n\n", pict);
  return pict;
}

static void set_picture(const char *path, int len, CPICTURE *newpict)
{
  char key[MAX_KEY + 1];

  snprintf(key, sizeof(key), "%s\n%.*s", GB.Component.Current(), len, path);
  INSERT_INTO_CACHE(key, newpict);
}
