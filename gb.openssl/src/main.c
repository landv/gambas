/*
 * main.c - gb.openssl main object
 *
 * Copyright (C) 2013,4 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define __MAIN_C

#include <openssl/evp.h>

#include <gb_common.h>
#include <gbx_compare.h>
#include <gbx_c_array.h>

#include "main.h"
#include "c_digest.h"
#include "c_cipher.h"
#include "c_hmac.h"

GB_INTERFACE GB EXPORT;

GB_DESC *GB_CLASSES[] EXPORT = {
	CDigest,
	CDigestMethod,

	CCipher,
	CCipherMethod,
	CCipherText,

	CHMac,

//	CSignature,
//	CSignatureMethod,

	NULL
};

void sort_and_dedupe(GB_ARRAY list)
{
	GB_FUNCTION sortfn, removefn;
	CARRAY *arr;
	int i;

	if (GB.GetFunction(&sortfn, list, "Sort", NULL, NULL)) {
		GB.Error("Can't sort array");
		return;
	}
	GB.Push(1, GB_T_INTEGER, GB_COMP_ASCENT | GB_COMP_NOCASE);
	GB.Call(&sortfn, 1, 0);

	if (GB.GetFunction(&removefn, list, "Remove", NULL, NULL)) {
		GB.Error("Can't remove duplicates");
		return;
	}
	arr = (CARRAY *) list;
	for (i = 0; i < arr->count - 1;) {
		char *a = ((char **) arr->data)[i],
		     *b = ((char **) arr->data)[i + 1];

		if (!strcasecmp(a, b)) {
			GB.Push(1, GB_T_INTEGER, i);
			GB.Call(&removefn, 1, 0);
		} else {
			i++;
		}
	}
}

int EXPORT GB_INIT()
{
	return 0;
}

void EXPORT GB_EXIT()
{
	EVP_cleanup();
}
