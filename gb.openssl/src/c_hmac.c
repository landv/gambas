/*
 * c_hmac.c - HMac class
 *
 * Copyright (C) 2013 Tobias Boege <tobias@gambas-buch.de>
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

#define __C_HMAC_C

#include <openssl/hmac.h>

#include "main.h"
#include "c_hmac.h"

/*
 * HMac
 */

/**G
 * Make an HMAC authentication code out of a key and some data.
 *
 * *Method* may be HMac.Sha1 or HMac.RipeMD160. By default it is HMac.Sha1.
 **/
BEGIN_METHOD(HMac_call, GB_STRING key; GB_STRING data; GB_INTEGER method)

	char *hash;
	unsigned int len;
	const EVP_MD *method;

	method = EVP_get_digestbynid(VARGOPT(method, NID_sha1));
	if (!method) {
		GB.Error("Unknown method");
		return;
	}

	hash = (char *) HMAC(method, STRING(key), LENGTH(key),
			     (unsigned char *) STRING(data),
			     LENGTH(data), NULL, &len);
	GB.ReturnNewString(hash, len);

END_METHOD

GB_DESC CHMac[] = {
	GB_DECLARE("HMac", 0),
	GB_NOT_CREATABLE(),

	/**G HMac Sha1
	 * Use the SHA1 algorithm.
	 **/
	GB_CONSTANT("Sha1", "i", NID_sha1),
	/**G HMac RipeMD160
	 * Use the RIPEMD160 algorithm.
	 **/
	GB_CONSTANT("RipeMD160", "i", NID_ripemd160),

	GB_STATIC_METHOD("_call", "s", HMac_call, "(Key)s(Data)s[(Method)i]"),

	GB_END_DECLARE
};
