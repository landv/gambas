/*
 * c_hmac.c - HMac class
 *
 * Copyright (C) 2013-2019 Tobias Boege <tobias@gambas-buch.de>
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
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL. If you modify
 * file(s) with this exception, you may extend this exception to
 * your version of the file(s), but you are not obligated to do so.
 * If you do not wish to do so, delete this exception statement
 * from your version. If you delete this exception statement from
 * all source files in the program, then also delete it here.
 */

#define __C_HMAC_C

#include <openssl/hmac.h>

#include "main.h"
#include "c_hmac.h"

/*
 * HMac
 */

static EVP_MD *get_method(GB_VARIANT_VALUE *varg)
{
	const EVP_MD *method;

	switch (varg->type) {
	case GB_T_INTEGER:
		GB.Deprecated("gb.openssl", "HMac() with integer method", "a string method like 'sha1'");
		method = EVP_get_digestbynid(varg->value._integer);
		break;
	case GB_T_STRING:
		method = EVP_get_digestbyname(varg->value._string);
		break;
	default:
		GB.Error("Method argument to HMac() can only be Integer or String");
		return NULL;
	}

	if (!method)
		GB.Error("Unknown method");
	return NULL;
}

/**G
 * Make an HMAC authentication code out of a key and some data.
 *
 * *Method* is any digest name such as "sha1" or "ripemd160" that is
 * supported by the Digest class. The default is "sha1".
 **/
BEGIN_METHOD(HMac_call, GB_STRING key; GB_STRING data; GB_VARIANT method)

	char hash[EVP_MAX_MD_SIZE];
	unsigned int len;
	const EVP_MD *method;

	method = MISSING(method) ?
		EVP_get_digestbynid(NID_sha1) :
		get_method(&VARG(method));
	if (!method)
		return;

	memset(hash, 0, sizeof(hash));
	HMAC(method, STRING(key), LENGTH(key), (unsigned char *) STRING(data),
	     LENGTH(data), (unsigned char *) hash, &len);
	GB.ReturnNewString(hash, len);

END_METHOD

GB_DESC CHMac[] = {
	GB_DECLARE("HMac", 0),
	GB_NOT_CREATABLE(),

	/**G HMac Sha1
	 * Use the SHA1 algorithm.
	 *
	 * Since Gambas 3.14: This constant it deprecated. Use the string
	 * "sha1" as an argument to HMac() instead.
	 **/
	GB_CONSTANT("Sha1", "i", NID_sha1),
	/**G HMac RipeMD160
	 * Use the RIPEMD160 algorithm.
	 *
	 * Since Gambas 3.14: This constant is deprecated. Use the string
	 * "ripemd160" as an argument to HMac() instead.
	 **/
	GB_CONSTANT("RipeMD160", "i", NID_ripemd160),

	GB_STATIC_METHOD("_call", "s", HMac_call, "(Key)s(Data)s[(Method)v]"),

	GB_END_DECLARE
};
