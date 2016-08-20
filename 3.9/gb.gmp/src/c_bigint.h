/***************************************************************************

  c_bigint.h

  gb.gmp component

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

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

#ifndef __C_BIGINT_H
#define __C_BIGINT_H

#include <gmp.h>

#ifndef __C_BIGINT_C
extern GB_DESC BigIntDesc[];
#endif

typedef
	struct {
		GB_BASE ob;
		mpz_t n;
	}
	CBIGINT;

CBIGINT *BIGINT_create(mpz_t number);

#endif /* __C_BIGINT_H */
