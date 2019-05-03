/*
 * c_digest.c - Digest and .Digest.Method classes
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

#define __C_DIGEST_C

#include <openssl/crypto.h>
#include <openssl/engine.h>
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

/*
 * Forward compatibility for openssl 1.1
 *
 * OPENSSL_zalloc() and OPENSSL_clear_free() have been written according to
 * the behaviour described in their manpage.
 */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
/*
 * Special treatment for OpenSSL 0.9.8*. The version hex below is 0.9.8zh.
 * The *_new() and *_free() routines were called *_create() and *_destroy()
 * there.
 */
#if OPENSSL_VERSION_NUMBER <= 0x0090821fL
inline int EVP_MD_CTX_cleanup(EVP_MD_CTX *ctx)
{
	/* Don't assume ctx->md_data was cleaned in EVP_Digest_Final,
	 * because sometimes only copies of the context are ever finalised.
	 */
	if (ctx->digest && ctx->digest->cleanup
	 && !M_EVP_MD_CTX_test_flags(ctx,EVP_MD_CTX_FLAG_CLEANED))
		ctx->digest->cleanup(ctx);
	if (ctx->digest && ctx->digest->ctx_size && ctx->md_data
	 && !M_EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_REUSE)) {
		OPENSSL_cleanse(ctx->md_data,ctx->digest->ctx_size);
		OPENSSL_free(ctx->md_data);
	}
#ifndef OPENSSL_NO_ENGINE
	if(ctx->engine)
	/* The EVP_MD we used belongs to an ENGINE, release the
	 * functional reference we held for this reason. */
	do_engine_finish(ctx->engine);
#endif
	memset(ctx,'\0',sizeof *ctx);
	return 1;
}

inline void EVP_MD_CTX_init(EVP_MD_CTX *ctx)
{
	memset(ctx, '\0', sizeof *ctx);
}

inline EVP_MD_CTX *EVP_MD_CTX_new(void)
{
	EVP_MD_CTX *ctx = OPENSSL_malloc(sizeof *ctx);

	if (ctx)
		EVP_MD_CTX_init(ctx);
	return ctx;
}

inline void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
	EVP_MD_CTX_cleanup(ctx);
	OPENSSL_free(ctx);
}
#else /* Anything recent */
static void OPENSSL_clear_free(void *str, size_t num)
{
	OPENSSL_cleanse(str, num);
	OPENSSL_free(str);
}

static void *OPENSSL_zalloc(size_t num)
{
	void *ret = OPENSSL_malloc(num);

	if (!ret)
		return NULL;
	memset(ret, 0, num);
	return ret;
}

static int EVP_MD_CTX_reset(EVP_MD_CTX *ctx)
{
    if (ctx == NULL)
        return 1;

    /*
     * Don't assume ctx->md_data was cleaned in EVP_Digest_Final, because
     * sometimes only copies of the context are ever finalised.
     */
    if (ctx->digest && ctx->digest->cleanup
        && !EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_CLEANED))
        ctx->digest->cleanup(ctx);
    if (ctx->digest && ctx->digest->ctx_size && ctx->md_data
        && !EVP_MD_CTX_test_flags(ctx, EVP_MD_CTX_FLAG_REUSE)) {
        OPENSSL_clear_free(ctx->md_data, ctx->digest->ctx_size);
    }
    EVP_PKEY_CTX_free(ctx->pctx);
#ifndef OPENSSL_NO_ENGINE
    ENGINE_finish(ctx->engine);
#endif
    OPENSSL_cleanse(ctx, sizeof(*ctx));

    return 1;
}

static EVP_MD_CTX *EVP_MD_CTX_new(void)
{
    return OPENSSL_zalloc(sizeof(EVP_MD_CTX));
}

static void EVP_MD_CTX_free(EVP_MD_CTX *ctx)
{
    EVP_MD_CTX_reset(ctx);
    OPENSSL_free(ctx);
}
#endif
#endif

/**G
 * Hash the given string using this digest algorithm.
 **/
BEGIN_METHOD(DigestMethod_Hash, GB_STRING data)

	EVP_MD_CTX *ctx;
	char digest[EVP_MAX_MD_SIZE];
	unsigned int length;

	ctx = EVP_MD_CTX_new();
	if (!ctx) {
		GB.Error("Could not allocate digest context");
		return;
	}
	memset(digest, 0, sizeof(digest));
	EVP_DigestInit(ctx, _method);
	EVP_DigestUpdate(ctx, STRING(data), LENGTH(data));
	EVP_DigestFinal(ctx, (unsigned char *) digest, &length);
	EVP_MD_CTX_free(ctx);
	GB.ReturnNewString(digest, length);

END_METHOD

GB_DESC CDigestMethod[] = {
	GB_DECLARE(".Digest.Method", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_METHOD("Hash", "s", DigestMethod_Hash, "(Data)s"),
	GB_STATIC_METHOD("_call", "s", DigestMethod_Hash, "(Data)s"),

	GB_END_DECLARE
};
