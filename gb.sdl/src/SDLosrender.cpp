/***************************************************************************

  SDLosrender.cpp

  (c) 2006-2008 Laurent Carlier <lordheavy@users.sourceforge.net>

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

#include "SDLosrender.h"

#include <iostream>

/****** FBO render ******/
bool FBOrender::hBinded = false;

FBOrender::FBOrender()
{
	glGenFramebuffersEXT(1, &hFbo);
}

FBOrender::~FBOrender()
{
	if (hFbo)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDeleteFramebuffersEXT(1, &hFbo);
	}
}

void FBOrender::Bind(GLuint texid)
{
	GLenum status;
	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hFbo);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texid, 0);
	
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		std::cerr << "FBO can't be completed : "<< std::hex << status << std::endl;
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, hFbo);
	
	FBOrender::hBinded = true;
	std::cout << "FBO: binding " <<  hFbo << " with tex " << texid << std::endl;
}

void FBOrender::Unbind(void )
{
	if (!FBOrender::hBinded)
		return;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	std::cout << "FBO: unbinding " <<  std::endl;
	FBOrender::hBinded = false;
}

bool FBOrender::Check(void )
{
	return (GLEW_ARB_framebuffer_object || GLEW_EXT_framebuffer_object);
}
