/*
 * c_cipher.c - Cipher, .Cipher.Method and CipherText classes
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

#define __C_CIPHER_C

#include <assert.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include "main.h"
#include "c_cipher.h"

/*
 * Cipher
 */

static GB_ARRAY _clist = NULL;

static void clist_func(const EVP_CIPHER *ciph, const char *from,
		       const char *to, void *arg)
{
	const char *src;

	if (!ciph)
		return;
	src = EVP_CIPHER_name(ciph);
	*((const char **) GB.Array.Add(_clist)) = GB.NewZeroString(src);
}

static void get_clist(void)
{
	GB.Array.New(&_clist, GB_T_STRING, 0);
	/* XXX: There is EVP_CIPHER_do_all_sorted() but that still returns
	 * entries twice (NOT next to each other), so we sort manually */
	EVP_CIPHER_do_all(clist_func, NULL);
	sort_and_dedupe(_clist);
}

/**G
 * Return a list of all ciphers present in the local OpenSSL crypto library.
 **/
BEGIN_PROPERTY(Cipher_List)

	GB_FUNCTION copyfn;
	GB_VALUE *copy;

	if (!_clist)
		get_clist();

	if (GB.GetFunction(&copyfn, _clist, "Copy", NULL, NULL)) {
		GB.Error("Can't copy array");
		return;
	}
	copy = GB.Call(&copyfn, 0, 0);
	GB.ReturnObject(copy->_object.value);

END_PROPERTY

BEGIN_METHOD_VOID(Cipher_init)

	OpenSSL_add_all_ciphers();

END_METHOD

BEGIN_METHOD_VOID(Cipher_exit)

	if (!_clist)
		return;
	GB.Unref((void **) &_clist);
	_clist = NULL;

END_METHOD

const static EVP_CIPHER *_method;

/**G
 * Return a virtual object representing a cipher algorithm by giving its
 * name. Valid names can be looked up from the Cipher.List property.
 **/
BEGIN_METHOD(Cipher_get, GB_STRING method)

	_method = EVP_get_cipherbyname(GB.ToZeroString(ARG(method)));
	if (!_method) {
		GB.Error("Unknown cipher method");
		return;
	}
	RETURN_SELF();

END_METHOD

/**G
 * Check if the named cipher algorithm is supported.
 **/
BEGIN_METHOD(Cipher_IsSupported, GB_STRING method)

	_method = EVP_get_cipherbyname(STRING(method));
	GB.ReturnBoolean(!!_method);

END_METHOD

GB_DESC CCipher[] = {
	GB_DECLARE("Cipher", 0),
	GB_NOT_CREATABLE(),

	GB_STATIC_PROPERTY_READ("List", "String[]", Cipher_List),

	GB_STATIC_METHOD("_init", NULL, Cipher_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, Cipher_exit, NULL),
	GB_STATIC_METHOD("_get", ".Cipher.Method", Cipher_get, "(Method)s"),
	GB_STATIC_METHOD("IsSupported", "b", Cipher_IsSupported, "(Method)s"),

	GB_END_DECLARE
};

/*
 * .Cipher.Method
 */

/**G
 * Return the key length that this cipher algorithm expects.
 *
 * See also: Cipher.Encrypt()
 **/
BEGIN_PROPERTY(CipherMethod_KeyLength)

	if (READ_PROPERTY) {
		GB.ReturnInteger(EVP_CIPHER_key_length(_method));
		return;
	}


END_PROPERTY

/**G
 * Return the initialisation vector length that this cipher algorithm
 * expects.
 *
 * See also: Cipher.Encrypt()
 **/
BEGIN_PROPERTY(CipherMethod_IvLength)

	if (READ_PROPERTY) {
		GB.ReturnInteger(EVP_CIPHER_iv_length(_method));
		return;
	}


END_PROPERTY

static char *do_cipher(const unsigned char *data, unsigned int dlen,
		       const unsigned char *key, const unsigned char *iv,
		       unsigned int *length, int enc)
{
	EVP_CIPHER_CTX ctx;
	unsigned char block[1024 + EVP_MAX_BLOCK_LENGTH];
	char *out;
	unsigned int ilen;
	int blen;

	EVP_CIPHER_CTX_init(&ctx);
	if (!EVP_CipherInit_ex(&ctx, _method, NULL, key, iv, enc))
		return NULL;

	out = NULL;
	*length = 0;
	while (dlen) {
		ilen = MIN(dlen, 1024);
		if (!EVP_CipherUpdate(&ctx, block, &blen, data, ilen))
			goto __ERROR;
		out = GB.AddString(out, (char *) block, blen);
		*length += blen;
		data += ilen;
		dlen -= ilen;
	}
	if (!EVP_CipherFinal_ex(&ctx, block, &blen))
		goto __ERROR;
	if (!EVP_CIPHER_CTX_cleanup(&ctx))
		goto __ERROR;
	if (blen) {
		out = GB.AddString(out, (char *) block, blen);
		*length += blen;
	}
	return out;

__ERROR:
	GB.FreeString(&out);
	return NULL;
}

typedef struct {
	GB_BASE ob;
	char *cipher, *key, *iv;
} CCIPHERTEXT;

/**G
 * Encrypt a plaintext using the given key and initialisation vector.
 **/
BEGIN_METHOD(CipherMethod_Encrypt, GB_STRING plain; GB_STRING key;
				   GB_STRING iv)

	unsigned char key[EVP_CIPHER_key_length(_method)];
	unsigned char iv[EVP_CIPHER_iv_length(_method)];
	unsigned int length;
	char *cipher;
	CCIPHERTEXT *res;

	bzero(key, sizeof(key));
	bzero(iv, sizeof(iv));
	if (MISSING(key)) {
		assert(RAND_bytes(key, sizeof(key)));
	} else {
		if (LENGTH(key) != sizeof(key)) {
			GB.Error("Key length does not match method");
			return;
		}
		memcpy(key, STRING(key), sizeof(key));
	}
	if (MISSING(iv)) {
		assert(RAND_bytes(iv, sizeof(iv)));
	} else {
		if (LENGTH(iv) != sizeof(iv)) {
			GB.Error("InitVector length does not match method");
			return;
		}
		memcpy(iv, STRING(iv), sizeof(iv));
	}

	cipher = do_cipher((uchar *) STRING(plain), LENGTH(plain), key, iv,
			   &length, 1);
	if (!cipher) {
		GB.Error("Encryption failed");
		return;
	}

	GB.Push(3, GB_T_STRING, cipher, length,
		   GB_T_STRING, key, sizeof(key),
		   GB_T_STRING, iv, sizeof(iv));
	res = GB.New(GB.FindClass("CipherText"), NULL, (void*)(intptr_t) 3);
	GB.FreeString(&cipher);
	GB.ReturnObject(res);

END_METHOD

/**G
 * Decrypt a ciphertext generated by Encrypt().
 **/
BEGIN_METHOD(CipherMethod_Decrypt, GB_OBJECT ciph)

	CCIPHERTEXT *ciph = VARG(ciph);
	unsigned int length;
	char *plain;

	plain = do_cipher((uchar *) ciph->cipher,
			  GB.StringLength(ciph->cipher),
			  (uchar *) ciph->key, (uchar *) ciph->iv,
			  &length, 0);
	if (!plain) {
		GB.Error("Decryption failed");
		return;
	}
	GB.ReturnString(plain);
	GB.ReturnBorrow();
	GB.FreeString(&plain);
	GB.ReturnRelease();

END_METHOD

/*
 * Retain compatibility with the 'openssl' program for now.
 */

#define ITER 1

/**G#
 * Encrypt a plaintext with a password to a standalone string.
 *
 * If 'salt' is given, it should be an 8-byte string. If it is not, it is
 * padded with zeros or truncated to a size of 8. If the parameter is not
 * given, a pseudo-random salt is generated.
 *
 * This uses a less up-to-date method (PKCS#5 v1.5) to generate an
 * encryption key from the password. This is for compatibility with the
 * openssl program when invoked like:
 *
 * `echo -n Plain | openssl CipherMethod -k Password -S HexSalt`
 **/
BEGIN_METHOD(CipherMethod_EncryptSalted, GB_STRING plain; GB_STRING passwd;
					 GB_STRING salt)

	unsigned char salt[8];
	unsigned char key[EVP_CIPHER_key_length(_method)];
	unsigned char iv[EVP_CIPHER_iv_length(_method)];
	unsigned int length;
	char *cipher, *res;

	bzero(salt, sizeof(salt));
	if (MISSING(salt)) {
		assert(RAND_pseudo_bytes(salt, sizeof(salt)));
	} else {
		bzero(salt, sizeof(salt));
		memcpy(salt, STRING(salt), MIN(sizeof(salt), LENGTH(salt)));
	}

	EVP_BytesToKey(_method, EVP_md5(), salt, (uchar *) STRING(passwd),
		       LENGTH(passwd), ITER, key, iv);
	cipher = do_cipher((uchar *) STRING(plain), LENGTH(plain), key, iv,
			   &length, 1);
	if (!cipher) {
		GB.Error("Encryption failed");
		return;
	}
	res = GB.NewZeroString("Salted__");
	res = GB.AddString(res, (char *) salt, sizeof(salt));
	res = GB.AddString(res, cipher, length);
	GB.FreeString(&cipher);
	GB.ReturnString(res);
	GB.ReturnBorrow();
	GB.FreeString(&res);
	GB.ReturnRelease();

END_METHOD

/**G#
 * Decrypt a ciphertext obtained from EncryptSalted().
 **/
BEGIN_METHOD(CipherMethod_DecryptSalted, GB_STRING cipher; GB_STRING passwd)

	unsigned char salt[8], *cipher;
	unsigned char key[EVP_CIPHER_key_length(_method)];
	unsigned char iv[EVP_CIPHER_iv_length(_method)];
	unsigned int clen, length;
	char *plain;

	if (!strstr(STRING(cipher), "Salted__")) {
		GB.Error("Unrecognised cipher string format");
		return;
	}
	/* salt begins at STRING(cipher) + strlen("Salted__") */
	memcpy(salt, STRING(cipher) + 8, sizeof(salt));

	EVP_BytesToKey(_method, EVP_md5(), salt, (uchar *) STRING(passwd),
		       LENGTH(passwd), ITER, key, iv);
	cipher = (uchar *) STRING(cipher) + 8 + sizeof(salt);
	clen = LENGTH(cipher) - (uint) (cipher - (uchar *) STRING(cipher));
	plain = do_cipher((uchar *) cipher, clen, key, iv, &length, 0);
	if (!plain) {
		GB.Error("Decryption failed");
		return;
	}
	GB.ReturnString(plain);
	GB.ReturnBorrow();
	GB.FreeString(&plain);
	GB.ReturnRelease();

END_METHOD

#if 0
/**G
 * Encrypt a plaintext with a password to a standalone string.
 *
 * We use the PKCS#5-v2.0-conforming PBKDF2 HMAC-SHA1 to generate an
 * encryption key from a password.
 **/
BEGIN_METHOD(CipherMethod_EncryptSalted, GB_STRING plain; GB_STRING passwd)

	char salt[8]; /* 8 is recommended as minimum */
	char key[EVP_CIPHER_key_length(_method)];

	bzero(key, sizeof(key));
	RAND_pseudo_bytes(salt, sizeof(salt));
	PKCS5_PBKDF_HMAC_SHA1(STRING(passwd), LENGTH(passwd), salt,
			      sizeof(salt), ITER, sizeof(key), key);
	

END_METHOD

/**G
 * Decrypt a ciphertext obtained from EncryptSalted().
 **/
BEGIN_METHOD(CipherMethod_DecryptSalted, GB_STRING cipher; GB_STRING passwd)

	

END_METHOD
#endif

GB_DESC CCipherMethod[] = {
	/*
	 * TODO: Maybe add a stream interface?
	 */
	GB_DECLARE(".Cipher.Method", 0),
	GB_VIRTUAL_CLASS(),

	GB_STATIC_PROPERTY_READ("KeyLength", "i", CipherMethod_KeyLength),
	GB_STATIC_PROPERTY_READ("IvLength", "i", CipherMethod_IvLength),

	GB_STATIC_METHOD("Encrypt", "CipherText", CipherMethod_Encrypt, "(Plain)s[(Key)s(InitVector)s]"),
	GB_STATIC_METHOD("Decrypt", "s", CipherMethod_Decrypt, "(Cipher)CipherText"),

	GB_STATIC_METHOD("EncryptSalted", "s", CipherMethod_EncryptSalted, "(Plain)s(Password)s[(Salt)s]"),
	GB_STATIC_METHOD("DecryptSalted", "s", CipherMethod_DecryptSalted, "(Cipher)s(Password)s"),

	GB_END_DECLARE
};

/*
 * CipherText
 */

#define THIS	((CCIPHERTEXT *) _object)

BEGIN_PROPERTY(CipherText_Cipher)

	GB.ReturnString(THIS->cipher);

END_PROPERTY

BEGIN_PROPERTY(CipherText_Key)

	GB.ReturnString(THIS->key);

END_PROPERTY

BEGIN_PROPERTY(CipherText_InitVector)

	GB.ReturnString(THIS->iv);

END_PROPERTY

BEGIN_METHOD(CipherText_new, GB_STRING ciph; GB_STRING key; GB_STRING iv)

	THIS->cipher = GB.NewString(STRING(ciph), LENGTH(ciph));
	THIS->key = GB.NewString(STRING(key), LENGTH(key));
	THIS->iv = GB.NewString(STRING(iv), LENGTH(iv));

END_METHOD

BEGIN_METHOD_VOID(CipherText_free)

	GB.FreeString(&THIS->cipher);
	GB.FreeString(&THIS->key);
	GB.FreeString(&THIS->iv);

END_METHOD

GB_DESC CCipherText[] = {
	GB_DECLARE("CipherText", sizeof(CCIPHERTEXT)),

	GB_PROPERTY_READ("Cipher", "s", CipherText_Cipher),
	GB_PROPERTY_READ("Key", "s", CipherText_Key),
	GB_PROPERTY_READ("InitVector", "s", CipherText_InitVector),

	GB_METHOD("_new", NULL, CipherText_new, "(Cipher)s(Key)s(InitVector)s"),
	GB_METHOD("_free", NULL, CipherText_free, NULL),

	GB_END_DECLARE
};
