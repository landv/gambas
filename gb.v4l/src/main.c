/***************************************************************************

  main.c

  Webcam Component

  (C) 2005 Daniel Campos Fernández <danielcampos@netcourrier.com>

  Based on the GPL code from "video-capture":

      (#) video-capture.c - modified from vidcat.c
      Copyright (C) 1998 Rasca, Berlin
      EMail: thron@gmx.de
      Modifications (C) 2001, Nick Andrew <nick@nick-andrew.net>

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

#define __MAIN_C

#include <stdio.h>

#include "CWebcam.h"
#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif


GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT =
{
  CWebcamDesc,
  CTunerDesc,
  CFeaturesDesc,
  NULL
};



int EXPORT GB_INIT(void)
{
	return 0;
}



void EXPORT GB_EXIT()
{

}


#ifdef _cpluscplus
}
#endif

