/***************************************************************************

  GLselectFeedback.c

  (c) 2005-2007 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __GLSELECTPIXMAP_C

#include "GL.h"

#define MAX_BUFFER_SIZE 2048

/* for feedback and select buffers */
static GLuint selectbuffer[MAX_BUFFER_SIZE];
static GLfloat feedbuffer[MAX_BUFFER_SIZE];

BEGIN_METHOD(GLFEEDBACKBUFFER, GB_INTEGER Type)

      glFeedbackBuffer(MAX_BUFFER_SIZE, VARG(Type), feedbuffer);

END_METHOD

BEGIN_METHOD_VOID(GLINITNAMES)

      glInitNames();

END_METHOD

BEGIN_METHOD(GLLOADNAME, GB_INTEGER Name)

      glLoadName(VARG(Name));

END_METHOD

BEGIN_METHOD(GLPASSTHROUGH, GB_FLOAT Token)

      glPassThrough(VARG(Token));

END_METHOD

BEGIN_METHOD_VOID(GLPOPNAME)

      glPopName();

END_METHOD

BEGIN_METHOD(GLPUSHNAME, GB_INTEGER Name)

      glPushName(VARG(Name));

END_METHOD

BEGIN_METHOD(GLRENDERMODE, GB_INTEGER Mode)

      static GLuint oldrendermode = GL_RENDER; /* default render mode */
      GLint result, mode = VARG(Mode);

      /* invalid mode or same render mode:
         return null object, and let propagate an opengl error */
      if ((mode < GL_RENDER) || (mode > GL_SELECT) || (mode == oldrendermode))
      {
            GB.ReturnNull();
            glRenderMode(mode);
            return;
      }
      
	result = glRenderMode(mode);
 
      if (!result)
      {
            GB.ReturnNull();
            oldrendermode = mode;
            return;
      }

      if (result < 0)
      {
            GB.Error ("Gl.RenderMode, buffer is too small !");
            return;
      }
      
      if (oldrendermode == GL_SELECT)
      {
            GB_ARRAY hitArray;
            GLuint *hitbuffer = selectbuffer;
            int idxhit, idxname;
            
            GB.Array.New(&hitArray, GB.FindClass("Integer[]"), result);
            //GB.New(POINTER(&hitArray), GB.FindClass("Integer[][]"), NULL, NULL);

            for (idxhit=0; idxhit < result; idxhit++)
            {
                  GB_ARRAY childhitArray;
                  
                  int names = *hitbuffer;
                  GB.Array.New(&childhitArray, GB_T_INTEGER, names + 3);
                  *((GLuint *)GB.Array.Get(childhitArray, 0)) = *hitbuffer++; // maxname
                  *((GLuint *)GB.Array.Get(childhitArray, 1)) = *hitbuffer++; // zmin
                  *((GLuint *)GB.Array.Get(childhitArray, 2)) = *hitbuffer++; // zmax

                  for (idxname=0; idxname < names; idxname++)
                        *((GLuint *)GB.Array.Get(childhitArray, 3 + idxname)) = *hitbuffer++; // names

                  GB.Ref(childhitArray);
                  *((GB_ARRAY *)GB.Array.Get(hitArray, idxhit)) = childhitArray;
             }

             GB.ReturnObject(hitArray);
             return;
      }

      if (oldrendermode == GL_FEEDBACK)
      {
            GB_ARRAY feedArray;
            GLfloat *resultbuffer = feedbuffer;
            int idxfeed;

            GB.Array.New(&feedArray, GB_T_FLOAT, result);

            for (idxfeed=0; idxfeed < result; idxfeed++)
                  *((GLfloat *)GB.Array.Get(feedArray, idxfeed)) = *resultbuffer++;

            GB.ReturnObject(feedArray);
            return;
      }
      
END_METHOD

BEGIN_METHOD_VOID(GLSELECTBUFFER)

      glSelectBuffer(MAX_BUFFER_SIZE, selectbuffer);

END_METHOD
