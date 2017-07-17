/***************************************************************************

  GL.c

  (c) 2009 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#define __GL_C

#include "GL.h"

#include "GLshader.h"
#include "GLprogram.h"
#include "GLuniform.h"
#include "GLattributes.h"

GB_DESC Cgl[] =
{
	GB_DECLARE("Gl",0), GB_NOT_CREATABLE(),
	
	/* GLshader.c */
	GB_STATIC_METHOD("AttachShader", NULL, GLATTACHSHADER, "(Program)i(Shader)i"),
	GB_STATIC_METHOD("CompileShader", NULL, GLCOMPILESHADER, "(Shader)i"),
	GB_STATIC_METHOD("CreateShader", "i", GLCREATESHADER, "(ShaderType)i"),
	GB_STATIC_METHOD("DeleteShader", NULL, GLDELETESHADER, "(Shader)i"),
	GB_STATIC_METHOD("DetachShader", NULL, GLDETACHSHADER, "(Program)i(Shader)i"),
	GB_STATIC_METHOD("GetAttachedShaders", "Integer[]", GLGETATTACHEDSHADERS, "(Program)i"),
	GB_STATIC_METHOD("GetShaderInfoLog", "s", GLGETSHADERINFOLOG, "(Shader)i"),
	GB_STATIC_METHOD("GetShaderiv", "Integer[]", GLGETSHADERIV, "(Shader)i(Pname)i"),
	GB_STATIC_METHOD("GetShaderSource", "s", GLGETSHADERSOURCE, "(Shader)i"),
	GB_STATIC_METHOD("IsShader", "b", GLISSHADER, "(Shader)i"),
	GB_STATIC_METHOD("ShaderSource", NULL, GLSHADERSOURCE, "(Shader)i(Source)s"),
	/* GLprogram.c */
	GB_STATIC_METHOD("CreateProgram", "i", GLCREATEPROGRAM, NULL),
	GB_STATIC_METHOD("DeleteProgram", NULL, GLDELETEPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("GetProgramInfoLog", "s", GLGETPROGRAMINFOLOG, "(Program)i"),
	GB_STATIC_METHOD("GetProgramiv", "Integer[]", GLGETPROGRAMIV, "(Program)i(Pname)i"),
	GB_STATIC_METHOD("IsProgram", "b", GLISPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("LinkProgram", NULL, GLLINKPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("UseProgram", NULL, GLUSEPROGRAM, "(Program)i"),
	GB_STATIC_METHOD("ValidateProgram", NULL, GLVALIDATEPROGRAM, "(Program)i"),
	/* GLuniform.c */
	GB_STATIC_METHOD("GetUniformLocation", "i", GLGETUNIFORMLOCATION, "(Program)i(Name)s"),
	GB_STATIC_METHOD("Uniform1f", NULL, GLUNIFORM1F, "(Location)i(V0)f"),
	GB_STATIC_METHOD("Uniform2f", NULL, GLUNIFORM2F, "(Location)i(V0)f(V1)f"),
	GB_STATIC_METHOD("Uniform3f", NULL, GLUNIFORM3F, "(Location)i(V0)f(V1)f(V2)f"),
	GB_STATIC_METHOD("Uniform4f", NULL, GLUNIFORM4F, "(Location)i(V0)f(V1)f(V3)f(V3)f"),
	GB_STATIC_METHOD("Uniform1i", NULL, GLUNIFORM1I, "(Location)i(V0)i"),
	GB_STATIC_METHOD("Uniform2i", NULL, GLUNIFORM2I, "(Location)i(V0)i(V1)i"),
	GB_STATIC_METHOD("Uniform3i", NULL, GLUNIFORM3I, "(Location)i(V0)i(V1)i(V2)i"),
	GB_STATIC_METHOD("Uniform4i", NULL, GLUNIFORM4I, "(Location)i(V0)i(V1)i(V3)i(V3)i"),
	GB_STATIC_METHOD("Uniform1fv", NULL, GLUNIFORM1FV, "(Location)i(Values)Float[]"),
	GB_STATIC_METHOD("Uniform2fv", NULL, GLUNIFORM2FV, "(Location)i(Values)Float[]"),
	GB_STATIC_METHOD("Uniform3fv", NULL, GLUNIFORM3FV, "(Location)i(Values)Float[]"),
	GB_STATIC_METHOD("Uniform4fv", NULL, GLUNIFORM4FV, "(Location)i(Values)Float[]"),
	GB_STATIC_METHOD("Uniform1iv", NULL, GLUNIFORM1IV, "(Location)i(Values)Integer[]"),
	GB_STATIC_METHOD("Uniform2iv", NULL, GLUNIFORM2IV, "(Location)i(Values)Integer[]"),
	GB_STATIC_METHOD("Uniform3iv", NULL, GLUNIFORM3IV, "(Location)i(Values)Integer[]"),
	GB_STATIC_METHOD("Uniform4iv", NULL, GLUNIFORM4IV, "(Location)i(Values)Integer[]"),
	GB_STATIC_METHOD("UniformMatrix2fv", NULL, GLUNIFORMMATRIX2FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix3fv", NULL, GLUNIFORMMATRIX3FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix4fv", NULL, GLUNIFORMMATRIX4FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix2x3fv", NULL, GLUNIFORMMATRIX2X3FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix3x2fv", NULL, GLUNIFORMMATRIX3X2FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix2x4fv", NULL, GLUNIFORMMATRIX2X4FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix4x2fv", NULL, GLUNIFORMMATRIX4X2FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix3x4fv", NULL, GLUNIFORMMATRIX3X4FV, "(Location)i(Transpose)b(Values)Float[]"),
	GB_STATIC_METHOD("UniformMatrix4x3fv", NULL, GLUNIFORMMATRIX4X3FV, "(Location)i(Transpose)b(Values)Float[]"),
	/* GLattributes.c */
	GB_STATIC_METHOD("BindAttribLocation",NULL, GLBINDATTRIBLOCATION, "(Program)i(Index)i(Name)s"),
	GB_STATIC_METHOD("VertexAttrib1f", NULL, GLVERTEXATTRIB1F, "(Index)i(X)f"),
	GB_STATIC_METHOD("VertexAttrib1fv", NULL, GLVERTEXATTRIB1FV, "(Index)i(V)Float[]"),
	GB_STATIC_METHOD("VertexAttrib2f", NULL, GLVERTEXATTRIB2F, "(Index)i(X)f(Y)f"),
	GB_STATIC_METHOD("VertexAttrib2fv", NULL, GLVERTEXATTRIB2FV, "(Index)i(V)Float[]"),
	GB_STATIC_METHOD("VertexAttrib3f", NULL, GLVERTEXATTRIB3F, "(Index)i(X)f(Y)f(Z)f"),
	GB_STATIC_METHOD("VertexAttrib3fv", NULL, GLVERTEXATTRIB3FV, "(Index)i(V)Float[]"),
	GB_STATIC_METHOD("VertexAttrib4f", NULL, GLVERTEXATTRIB4F, "(Index)i(X)f(Y)f(Z)f(W)f"),
	GB_STATIC_METHOD("VertexAttrib4fv", NULL, GLVERTEXATTRIB4FV, "(Index)i(V)Float[]"),

	/* Contants */
	GB_CONSTANT("ACTIVE_ATTRIBUTES", "i", GL_ACTIVE_ATTRIBUTES),
	GB_CONSTANT("ACTIVE_ATTRIBUTE_MAX_LENGTH", "i", GL_ACTIVE_ATTRIBUTE_MAX_LENGTH),
	GB_CONSTANT("ACTIVE_UNIFORMS", "i", GL_ACTIVE_UNIFORMS),
	GB_CONSTANT("ACTIVE_UNIFORM_MAX_LENGTH", "i", GL_ACTIVE_UNIFORM_MAX_LENGTH),
	GB_CONSTANT("ATTACHED_SHADERS", "i", GL_ATTACHED_SHADERS),
	GB_CONSTANT("COMPILE_STATUS", "i", GL_COMPILE_STATUS),
	GB_CONSTANT("CURRENT_PROGRAM", "i", GL_CURRENT_PROGRAM),
	GB_CONSTANT("DELETE_STATUS", "i", GL_DELETE_STATUS),
	GB_CONSTANT("FRAGMENT_SHADER", "i", GL_FRAGMENT_SHADER),
	GB_CONSTANT("INFO_LOG_LENGTH", "i", GL_INFO_LOG_LENGTH),
	GB_CONSTANT("LINK_STATUS", "i", GL_LINK_STATUS),
	GB_CONSTANT("SHADER_SOURCE_LENGTH", "i", GL_SHADER_SOURCE_LENGTH),
	GB_CONSTANT("SHADER_TYPE", "i", GL_SHADER_TYPE),
	GB_CONSTANT("VALIDATE_STATUS", "i", GL_VALIDATE_STATUS),
	GB_CONSTANT("VERTEX_SHADER", "i", GL_VERTEX_SHADER),
	
	GB_END_DECLARE
};
