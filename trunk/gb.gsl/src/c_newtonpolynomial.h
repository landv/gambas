/***************************************************************************

  c_newtonpolynomial.h

  gb.gsl component

  (c) 2012 Randall Morgan <rmorgan62@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA 02110-1301, USA.

***************************************************************************/

#ifndef __C_GSL_NEWTONPOLYNOMIAL_H
#define __C_GSL_NEWTONPOLYNOMIAL_H

#include "gambas.h"
#include <gsl/gsl_poly.h>
#include <stdio.h>

 
GB_INTERFACE GB EXPORT;

extern GB_DESC CNewtonPolynomialDesc[];

typedef
  struct {
    GB_BASE ob;
    double *dd;
    double *xa;
    int len;
    }
  CNEWTONPOLYNOMIAL;


#endif /* __C_GSL_NEWTONPOLYNOMIAL_H */
