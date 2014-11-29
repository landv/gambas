/*
 * c_digest.c - Digest and .Digest.Method classes
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

#define __C_DIGEST_C

#include <openssl/evp.h>

#include "main.h"
#include "c_digest.h"

/*
 * Digest
 */

static GB_ARRAY _dlist = NULL;

static void dlist_func(const EVP_MD *dgst, const char *from,
		       const char *to, void *arg)
{
	const char *src;

	if (!dgst)
		return;
	src = EVP_MD_name(dgst);
	*((const char **) GB.Array.Add(_dlist)) = GB.NewZeroString(src);
}

static void get_dlist(void)
{
	GB.Array.New(&_dlist, GB_T_STRING, 0);
	EVP_MD_do_all(dlist_func, NULL);
	sort_and_dedupe(_dlist);
}

/**G
 * Return a list of all digests present in the local OpenSSL crypto library.
 **/
BEGIN_PROPERTY(Digest_List)

	GB_FUNCTION copyfn;
	GB_VALUE *copy;

	if (!_dlist)
		get_dlist();

	if (GB.GetFunction(&copyfn, _dlist, "Copy", NULL, NULL)) {
		GB.Error("Can't copy array");
		return;
	}
	copy = GB.Call(&copyfn, 0, 0);
	GB.ReturnObject(copy->_object.value);

END_PROPERTY

BEGIN_METHOD_VOID(Digest_init)

	OpenSSL_add_all_digests();

END_METHOD

BEGIN_METHOD_VOID(Digest_exit)

	if (!_dlist)
		return;
	GB.Unref((void **) &_dlist);
	_dlist = NULL;

END_METHOD

const static EVP_MD *_method;

/**G
 * Return a virtual object representing a digest algorithm by giving its
 * name. Valid names can be looked up from Digest.List.
 **/
BEGIN_METHOD(Digest_get, GB_STRING method)

	_method = EVP_get_digestbyname(GB.ToZeroString(ARG(method)));
	if (!_method) {
		GB.Error("Unknown digest method");
		return;
	}
	RETURN_SELF();

END_METHOD

/**G
 * Check whether the named digest algorithm is valid.
 **/
BEGIN_METHOD(Digest_IsSupported, GB_STRING method)

	_method = EVP_get_digestbyname(STRING(method));
	GB.ReturnBoolean(!!_method);

END_METHOD

GB_DESC CDigest[] = {
	GB_DECLARE("Digest", 0),
	GB_NOT_CREATABLE(),

	GB_STATIC_PROPERTY_READ("List", "String[]", Digest_List),

	GB_STATIC_METHOD("_init", NULL, Digest_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, Digest_exit, NULL),
	GB_STATIC_METHOD("_get", ".Digest.Method", Digest_get, "(Method)s"),
	GB_STATIC_METHOD("IsSupported", "b", Digest_IsSupported, "(Method)s"),

	GB_END_DECLARE
};

/*
 * .Digest.Method
 */

/**G
 * Hash the given string using this digest algorithm.
 **/
BEGIN_METHOD(DigestMethod_Hash, GB_STRING data)

	EVP_MD_CTX ctx;
	char digest[EVP_MAX_MD_SIZE];
	unsigned int length;

	EVP_DigestInit(&ctx, _method);
	EVP_DigestUpdate(&ctx, STRING(data), LENGTH(data));
	EVP_DigestFinal(&ctx, (unsigned char *) digest, &length);
	GB.ReturnNewString(digest, length);

END_METHOD

GB_DESC CDigestMethod[] = {
	GB_DECLARE(".Digest.Method", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("Hash", "s", DigestMethod_Hash, "(Data)s"),
	GB_STATIC_METHOD("_call", "s", DigestMethod_Hash, "(Data)s"),

	GB_END_DECLARE
};
