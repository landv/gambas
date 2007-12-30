/***************************************************************************

  html.cpp

  (c) 2004-2005 - Daniel Campos Fernández <danielcampos@netcourrier.com>
  
  Gtkmae "GTK+ made easy" classes
  
  Realizado para la Junta de Extremadura. 
  Consejería de Educación Ciencia y Tecnología. 
  Proyecto gnuLinEx
  
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

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct
{
	int type;
	char *key;
	char *data;
	
} html_token;

char *html_buffer=NULL;
GList *html_data=NULL;
GList *html_tokens=NULL;

/***************************************************************************

General

****************************************************************************/

html_token* html_create_token()
{
	html_token *tk;
	
	tk=(html_token*)g_malloc(sizeof(html_token));
	if (tk)
	{
		tk->key=NULL;
		tk->data=NULL;
	}
	return tk;
}

void html_free_tokens(void)
{
	GList *iter;
	html_token *tk;
	
	if (!html_tokens) return;
	
	iter=g_list_first(html_tokens);
	
	while (iter)
	{
		tk=(html_token*)iter->data;
		if (tk->key) g_free(tk->key);
		if (tk->data) g_free(tk->data);
		g_free(tk);
		iter=iter->next;
	}
	
	g_list_free(html_tokens);
	html_tokens=NULL;
}
/*********************************************************************************

XML to Pango parsing

**********************************************************************************/
void html_pango_plain(html_token *tk)
{
	long len;
	char *iter;
	char *next;
	
	if (!tk->data) return;
	
	len=strlen(tk->data);
	if (!len) return;
	iter=tk->data;
	
	
	
	while (1)
	{
		next=g_utf8_next_char(iter);
		if ( (next-iter)==1 )
		{
			if (iter[0]<32) iter[0]=32;
			if (iter[0]=='&') iter[0]=32;
			if (iter[0]=='<') iter[0]=32;
			if (iter[0]=='>') iter[0]=32;
		}
		
		if (!next[0]) break;
		iter=next;
		//if (tk->data[bucle]<32) tk->data[bucle]=32;
		//if (tk->data[bucle]=='&') tk->data[bucle]=' ';
	}
}

int html_pango_newline(html_token *tk)
{
	char *sust;
	int ok=0;
	
	if ( !strcmp(tk->key,"br") ) { sust="\n"; ok=1; }
	else if ( !strcmp(tk->key,"p") ) { sust="\n\n"; ok=1; }
	
	if (!ok) return 0;
	
	if (tk->type==2)
	{
		tk->type=-1;
	}
	else
	{
		tk->type=0;
		if (tk->key) { g_free(tk->key); tk->key=NULL; }
		if (tk->data) { g_free(tk->data); }
		tk->data=g_strdup(sust);
	}
	
	return 1;
}


int html_pango_direct(html_token *tk)
{
	int ok=0;
	
	if (tk->type == 3) return 0;
	
	if ( !strcmp(tk->key,"b") ) ok=1;
	else if ( !strcmp(tk->key,"i") ) ok=1;
	else if ( !strcmp(tk->key,"s") ) ok=1;
	else if ( !strcmp(tk->key,"u") ) ok=1;
	else if ( !strcmp(tk->key,"tt") ) ok=1;
	else if ( !strcmp(tk->key,"sub") ) ok=1;
	else if ( !strcmp(tk->key,"sup") ) ok=1;
	else if ( !strcmp(tk->key,"small") ) ok=1;
	else if ( !strcmp(tk->key,"big") ) ok=1;
	
	return ok;
}

int html_pango_headers(html_token *tk)
{
	char *sust;
	int ok=0;

	if (tk->type == 3) return 0;
	
	if ( !strcmp(tk->key,"h1") )      { sust="span size='xx-large' "; ok=1; }
	else if ( !strcmp(tk->key,"h2") ) { sust="span size='x-large' "; ok=1; }
	else if ( !strcmp(tk->key,"h3") ) { sust="span size='large' "; ok=1; }
	else if ( !strcmp(tk->key,"h4") ) { sust="span size='medium' "; ok=1; }
	else if ( !strcmp(tk->key,"h5") ) { sust="span size='small' "; ok=1; }
	else if ( !strcmp(tk->key,"h6") ) { sust="span size='x-small' "; ok=1; }
	
	if (ok)
	{
		if (tk->key) { g_free(tk->key); tk->key=NULL; }
		if (tk->type==1)
			tk->key=g_strdup(sust);
		else
			tk->key=g_strdup("span");
		
	}
	
	
	return ok;
}

void html_to_pango_tokens(void)
{
	GList *iter;
	html_token *tk;	
	int ok;
	
	if (!html_tokens) return;
	
	iter=g_list_first(html_tokens);
	
	while (iter)
	{
		tk=(html_token*)iter->data;
		if (tk->type)
		{
			ok=html_pango_newline(tk);
			if (!ok) ok=html_pango_direct(tk);
			if (!ok) ok=html_pango_headers(tk);
			if (!ok) tk->type=-1;
		}
		else
		{
			html_pango_plain(tk);
		}
		
		iter=iter->next;
	}
	
	iter=g_list_first(html_tokens);
	while (iter)
	{
		tk=(html_token*)iter->data;
		if (tk->type==-1)
		{
			html_tokens=g_list_remove(html_tokens,iter->data);
			iter=g_list_first(html_tokens);
			if (tk->data) g_free(tk->data);
			if (tk->key)  g_free(tk->key);
			g_free(tk);
		}
		else
		{
			iter=iter->next;
		}
	}
}

/********************************************************************************************

  HTML to XML parsing
  
*********************************************************************************************/
void html_ensure_nested_tokens(void)
{
	GList *iter;
	GList *stk=NULL;
	GList *test;
	html_token *tk1,*tk2,*tknew;
	
	
	if (!html_tokens) return;
	
	iter=g_list_first(html_tokens);
	
	while (iter)
	{
		tk1=(html_token*)iter->data;
		switch (tk1->type)
		{
			case 1:
				stk=g_list_append(stk,iter->data);
				break;
			
			case 2:
				tk1->type=-1;
				if (stk)
				{
					test=g_list_first(stk);
					while (test)
					{
						tk2=(html_token*)test->data;
						if (!strcmp(tk1->key,tk2->key))
						{
							tk1->type=2;
							break;
						}
						test=test->next;
					}
				}
				
				if (tk1->type==2)
				{
					test=g_list_last(stk);
					
					while (1)
					{
						tk2=(html_token*)test->data;
						if (!strcmp(tk1->key,tk2->key)) break;
						tknew=html_create_token();
						tknew->type=2;
						tknew->key=g_strdup(tk2->key);
						html_tokens=g_list_insert_before(html_tokens,iter,tknew);
						test=g_list_previous(test);
					}
					
					while (1)
					{
						test=g_list_last(stk);
						tk2=(html_token*)test->data;
						stk=g_list_remove(stk,test->data);
						if (!strcmp(tk1->key,tk2->key)) break;
					}
				}
			
		}
		iter=iter->next;
	}
	
	iter=g_list_first(html_tokens);
	while (iter)
	{
		tk1=(html_token*)iter->data;
		if (tk1->type==-1)
		{
			html_tokens=g_list_remove(html_tokens,iter->data);
			iter=html_tokens;
			if (tk1->data) g_free(tk1->data);
			if (tk1->key) g_free(tk1->key);
			g_free(tk1);
		}
		else
		{
			iter=iter->next;
		}
	}
	
	if (stk)
	{
		iter=g_list_last(stk);
		while (iter)
		{
			tk2=(html_token*)iter->data;
			iter=iter->prev;
			tknew=html_create_token();
			tknew->type=2;
			tknew->key=g_strdup(tk2->key);
			html_tokens=g_list_append(html_tokens,tknew);
		}
		g_list_free(stk);	
	
	}
	
}

void html_extract_tokens(void)
{
	GList *iter;
	html_token *tk;
	char *key;
	char *data;
	long bucle,len;
	
	if (!html_data) return;
	
	iter=g_list_first(html_data);
	
	while (iter)
	{
		if (iter->data)
		{
			if (strlen((const char*)iter->data))
			{
				tk=html_create_token();
				if (!tk) break;
				tk->type=0;
				tk->data=g_strdup((const gchar*)iter->data);
				html_tokens=g_list_prepend(html_tokens,tk);
			}
		}		
	
		iter=iter->next;
		if (!iter) break;
		
		data=NULL;
		key=(char*)iter->data;
		len=strlen(key);
		for (bucle=0;bucle<len;bucle++)
		{
			if (key[bucle]<=32)
			{
				key[bucle]=0;
				data=key+bucle+1;
				break;
			}
		}
		if (data) data=g_strstrip(data);
		
		tk=html_create_token();
		if (!tk) break;
		
		tk->type=1;
		if (key[0]=='/')
		{
			tk->type=2;
			key++;
		}
		else if (key[strlen(key)-1]=='/')
		{
			tk->type=3;
			key[strlen(key)-1]=0;
		}
		else if (data)
		{
			if (data[0]=='/')
			{
				data=NULL;
				tk->type=3;
			}
		} 
		
		if (data) tk->data=g_strdup(data);
		tk->key=g_utf8_strdown(key,-1);
		html_tokens=g_list_prepend(html_tokens,tk);
		
		iter=iter->next;
	}
	
	
	if (html_tokens) html_tokens=g_list_reverse(html_tokens);
	g_list_free(html_data);
	g_free(html_buffer);
	html_data=NULL;
	html_buffer=NULL;
}

void html_create_tokens(char *vl)
{
	long len,bucle;
	int inToken=0;
	char *ptr;
	GList *iter=NULL;

	if (html_buffer) { g_free(html_buffer); html_buffer=NULL; }
	if (html_data) { g_list_free(html_data); html_data=NULL; }
	html_free_tokens();
	if (!vl) return;
	
	len=strlen(vl);
	html_buffer=(char*)g_malloc(sizeof(char)*(len+1));
	if (!html_buffer) return;
	
	strcpy(html_buffer,vl);
	ptr=html_buffer;
	
	for (bucle=0;bucle<len;bucle++)
	{
		switch ( vl[bucle] )
		{
			case '<':
				if (!inToken)
				{
					inToken++;
					html_buffer[bucle]=0;
					html_data=g_list_prepend(html_data,ptr);
					ptr=html_buffer+bucle+1;
				}
				break;
				
			case '>':
				if (inToken)
				{
					inToken--;
					html_buffer[bucle]=0;
					html_data=g_list_prepend(html_data,ptr);
					ptr=html_buffer+bucle+1;
				}
			default:
				break;
		}
	}
	
	if (ptr < (html_buffer+len) ) html_data=g_list_prepend(html_data,ptr);
	
	if (html_data)
	{
		html_data=g_list_reverse(html_data);
		iter=g_list_first(html_data);
	}
	
	if (iter)
	{
		iter=iter->next;
		while (iter)
		{
			iter->data=g_strstrip((gchar*)iter->data);
			iter=iter->next;
			if (iter) iter=iter->next;
		}
	}
	
	html_extract_tokens();
	html_ensure_nested_tokens();
	
}

char* html_string_to_pango_string(char *buf)
{
	GList *iter;
	html_token *tk;
	char *ret=NULL;
	unsigned long len=1;
	
	html_create_tokens(buf);
	html_to_pango_tokens();

	if (!html_tokens) return NULL;
	
	
	iter=g_list_first(html_tokens);
	while (iter)
	{
		tk=(html_token*)iter->data;
		switch (tk->type)
		{
			case 0:
				if (tk->data) len+=strlen(tk->data);
				break;
				
			case 1:
				len+=strlen(tk->key);
				len+=2;
				break;
			
			case 2:
				len+=strlen(tk->key);
				len+=3;
				break;
			
			case 3:
				len+=strlen(tk->key);
				len+=3;
				break;
		}
		iter=iter->next;
	}
	
	ret=(char*)g_malloc (sizeof(char)*len);
	if (ret)
	{
		ret[0]=0;
		iter=g_list_first(html_tokens);
		while (iter)
		{
			tk=(html_token*)iter->data;
			switch (tk->type)
			{
				case 0:
					strcat(ret,tk->data);
					break;
					
				case 1:
					strcat(ret,"<");
					strcat(ret,tk->key);
					strcat(ret,">");
					break;
				
				case 2:
					strcat(ret,"</");
					strcat(ret,tk->key);
					strcat(ret,">");
					break;
				
				case 3:
					strcat(ret,"<");
					strcat(ret,tk->key);
					strcat(ret,"/>");
					break;
			}
			iter=iter->next;
		}
		
	}
	
	html_free_tokens();
	return ret;
}
