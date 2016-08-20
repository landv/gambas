/***************************************************************************

  c_bigint.c

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

#define __C_BIGINT_C

#include "main.h"
#include "c_bigint.h"

#define THIS ((CBIGINT *)_object)
#define NUMBER (THIS->n)

//---- BigInt number creation ----------------------------------------------

CBIGINT *BIGINT_create(mpz_t number)
{
	CBIGINT *c;

	c = (CBIGINT *)GB.New(CLASS_BigInt, NULL, NULL);
	mpz_set(c->n, number);
	mpz_clear(number);

	return c;
}

#define BIGINT_make(_a, _b, _op) \
({ \
	if ((_a)->ob.ref <= 1) \
		_op((_a)->n, (_a)->n, (_b)->n); \
	else \
	{ \
		mpz_t n; \
		mpz_init(n); \
		_op(n, (_a)->n, (_b)->n); \
		_a = BIGINT_create(n); \
	} \
	_a; \
})

#define BIGINT_make_unary(_a, _op) \
({ \
	if ((_a)->ob.ref <= 1) \
		_op((_a)->n, (_a)->n); \
	else \
	{ \
		mpz_t n; \
		mpz_init(n); \
		_op(n, (_a)->n); \
		_a = BIGINT_create(n); \
	} \
	_a; \
})

#define BIGINT_make_int(__a, _f, _op) \
({ \
	CBIGINT *_a = (__a); \
	if (_a->ob.ref <= 1) \
		_op(_a->n, _a->n, (ulong)(_f)); \
	else \
	{ \
		mpz_t n; \
		mpz_init(n); \
		_op(n, _a->n, (ulong)(_f)); \
		_a = BIGINT_create(n); \
	} \
	_a; \
})

#define BIGINT_make_int_invert(_a, _f, _op) \
({ \
	if ((_a)->ob.ref <= 1) \
		_op((_a)->n, (ulong)(_f), (_a)->n); \
	else \
	{ \
		mpz_t n; \
		mpz_init(n); \
		_op(n, (ulong)(_f), (_a)->n); \
		_a = BIGINT_create(n); \
	} \
	_a; \
})

#define BIGINT_make_bit(__a, _n, _op) \
({ \
	CBIGINT *_a = (__a); \
	if (_a->ob.ref <= 1) \
		_op(_a->n, _a->n, (_n)); \
	else \
	{ \
		mpz_t n; \
		mpz_init(n); \
		_op(n, _a->n, (_n)); \
		_a = BIGINT_create(n); \
	} \
	_a; \
})

//---- Arithmetic operators -------------------------------------------------

static CBIGINT *_addf(CBIGINT *a, double f, bool invert)
{
	if (f < 0)
		return BIGINT_make_int(a, (-f), mpz_sub_ui);
	else
		return BIGINT_make_int(a, f, mpz_add_ui);
}

static CBIGINT *_add(CBIGINT *a, CBIGINT *b, bool invert)
{
	return BIGINT_make(a, b, mpz_add);
}

static CBIGINT *_subf(CBIGINT *a, double f, bool invert)
{
	if (invert)
	{
		if (f < 0)
			return BIGINT_make_int(a, (-f), mpz_add_ui);
		else
			return BIGINT_make_int_invert(a, f, mpz_ui_sub);
	}
	else
	{
		if (f < 0)
			return BIGINT_make_int(a, (-f), mpz_add_ui);
		else
			return BIGINT_make_int(a, f, mpz_sub_ui);
	}
}

static CBIGINT *_sub(CBIGINT *a, CBIGINT *b, bool invert)
{
	return BIGINT_make(a, b, mpz_sub);
}

static CBIGINT *_mulf(CBIGINT *a, double f, bool invert)
{
	return BIGINT_make_int(a, f, mpz_mul_si);
}

static CBIGINT *_mul(CBIGINT *a, CBIGINT *b, bool invert)
{
	return BIGINT_make(a, b, mpz_mul);
}

static CBIGINT *_div(CBIGINT *a, CBIGINT *b, bool invert)
{
	if (mpz_cmp_si(b->n, 0) == 0)
	{
		GB.Error(GB_ERR_ZERO);
		return NULL;
	}
	else
		return BIGINT_make(a, b, mpz_tdiv_q);
}

static CBIGINT *_divf(CBIGINT *a, double f, bool invert)
{
	if (invert)
	{
		CBIGINT *b;
		mpz_t n;

		mpz_init_set_d(n, f);
		b = BIGINT_create(n);

		return _div(b, a, FALSE);
	}
	else
	{
		if (f > 0)
		{
			return BIGINT_make_int(a, f, mpz_tdiv_q_ui);
		}
		else if (f < 0)
		{
			a = BIGINT_make_int(a, (-f), mpz_tdiv_q_ui);
			mpz_neg(a->n, a->n);
			return a;
		}
		else
		{
			GB.Error(GB_ERR_ZERO);
			return NULL;
		}
	}
}

static int _equal(CBIGINT *a, CBIGINT *b, bool invert)
{
	return mpz_cmp(a->n, b->n) == 0;
}

static int _equalf(CBIGINT *a, double f, bool invert)
{
	return mpz_cmp_d(a->n, f) == 0;
}

static int _comp(CBIGINT *a, CBIGINT *b, bool invert)
{
	return mpz_cmp(a->n, b->n);
}

static int _compf(CBIGINT *a, double f, bool invert)
{
	return mpz_cmp_d(a->n, f);
}

static CBIGINT *_neg(CBIGINT *a)
{
	return BIGINT_make_unary(a, mpz_neg);
}

static CBIGINT *_abs(CBIGINT *a)
{
	return BIGINT_make_unary(a, mpz_abs);
}

static int _sgn(CBIGINT *a)
{
	return mpz_sgn(a->n);
}

static CBIGINT *_pow(CBIGINT *a, CBIGINT *b, bool invert)
{
	if (!mpz_fits_slong_p(b->n))
	{
		GB.Error(GB_ERR_OVERFLOW);
		return NULL;
	}

	return BIGINT_make_int(a, mpz_get_si(b->n), mpz_pow_ui);
}

static CBIGINT *_powf(CBIGINT *a, double f, bool invert)
{
	if (invert)
	{
		mpz_t b;

		if (!mpz_fits_slong_p(a->n))
			return NULL;

		mpz_init_set_si(b, (long)f);
		mpz_pow_ui(b, b, mpz_get_si(a->n));

		return BIGINT_create(b);
	}
	else
		return BIGINT_make_int(a, f, mpz_pow_ui);
}


static GB_OPERATOR_DESC _operator =
{
	.equal   = (void *)_equal,
	.equalf  = (void *)_equalf,
	.comp    = (void *)_comp,
	.compf   = (void *)_compf,
	.add     = (void *)_add,
	.addf    = (void *)_addf,
	.sub     = (void *)_sub,
	.subf    = (void *)_subf,
	.mul     = (void *)_mul,
	.mulf    = (void *)_mulf,
	.div     = (void *)_div,
	.divf    = (void *)_divf,
	.pow     = (void *)_pow,
	.powf    = (void *)_powf,
	.abs     = (void *)_abs,
	.neg     = (void *)_neg,
	.sgn     = (void *)_sgn
};

//---- Conversions ----------------------------------------------------------

char *BIGINT_to_string(mpz_t n, int base)
{
	char *str;
	int len;

	//if (mpz_cmp_si(n, 0) == 0)
	//	return GB.NewZeroString("0");

	len = mpz_sizeinbase (n, base);
	if (mpz_sgn(n) < 0)
		len++;

	str = GB.NewString(NULL, len);
	mpz_get_str(str, -base, n);

	if (str[len - 1] == 0)
		str = GB.ExtendString(str, len - 1);

	return str;
}

CBIGINT *BIGINT_from_string(char *str, int base)
{
	mpz_t n;

	if (mpz_init_set_str(n, str, base))
		return NULL;
	else
		return BIGINT_create(n);
}

static bool _convert(CBIGINT *a, GB_TYPE type, GB_VALUE *conv)
{
	if (a)
	{
		switch (type)
		{
			case GB_T_FLOAT:
				conv->_float.value = mpz_get_d(a->n);
				return FALSE;

			case GB_T_SINGLE:
				conv->_single.value = mpz_get_d(a->n);
				return FALSE;

			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				conv->_integer.value = (int)mpz_get_si(a->n);
				return FALSE;

			case GB_T_LONG:
				conv->_long.value = (int64_t)mpz_get_si(a->n);
				return FALSE;

			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_string.value.addr = BIGINT_to_string(a->n, 10); //, type == GB_T_CSTRING);
				conv->_string.value.start = 0;
				conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
				return FALSE;

			default:
				return TRUE;
		}
	}
	else
	{
		mpz_t n;

		switch(type)
		{
			case GB_T_FLOAT:
				mpz_init_set_d(n, conv->_float.value);
				conv->_object.value = BIGINT_create(n);
				return FALSE;

			case GB_T_SINGLE:
				mpz_init_set_d(n, conv->_single.value);
				conv->_object.value = BIGINT_create(n);
				return FALSE;

			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				mpz_init_set_si(n, (long)conv->_integer.value);
				conv->_object.value = BIGINT_create(n);
				return FALSE;

			case GB_T_LONG:
				mpz_init_set_si(n, (long)conv->_long.value);
				conv->_object.value = BIGINT_create(n);
				return FALSE;

			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_object.value = BIGINT_from_string(GB.ToZeroString(&conv->_string), 10);
				return conv->_object.value == NULL;

			default:
				return TRUE;
		}
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(BigInt_new)

	mpz_init_set_si(THIS->n, 0);

END_METHOD

BEGIN_METHOD_VOID(BigInt_free)

	mpz_clear(THIS->n);

END_METHOD

BEGIN_METHOD(BigInt_compare, GB_OBJECT other)

	CBIGINT *other = VARG(other);

	if (GB.CheckObject(other))
		return;

	GB.ReturnInteger(mpz_cmp(NUMBER, other->n));

END_METHOD

BEGIN_METHOD(BigInt_ToString, GB_INTEGER base)

	char *str;
	int base = VARGOPT(base, 10);

	if (base < 2 || base > 36)
	{
		GB.Error("Base must be between 2 and 36");
		return;
	}

	str = BIGINT_to_string(NUMBER, base);
	GB.FreeStringLater(str);
	GB.ReturnString(str);

END_METHOD

BEGIN_METHOD(BigInt_Shl, GB_INTEGER bits)

	GB.ReturnObject(BIGINT_make_int(THIS, VARG(bits), mpz_mul_2exp));

END_METHOD

BEGIN_METHOD(BigInt_PowM, GB_OBJECT exp; GB_OBJECT mod)

	mpz_t n;
	CBIGINT *exp = VARG(exp);
	CBIGINT *mod = VARG(mod);

	if (GB.CheckObject(exp) || GB.CheckObject(mod))
		return;

	mpz_init(n);
	mpz_powm(n, NUMBER, exp->n, mod->n);
	GB.ReturnObject(BIGINT_create(n));

END_METHOD

BEGIN_METHOD(BigInt_InvM, GB_OBJECT mod)

	mpz_t n;
	CBIGINT *mod = VARG(mod);

	if (GB.CheckObject(mod))
		return;

	mpz_init(n);
	mpz_invert(n, NUMBER, mod->n);
	GB.ReturnObject(BIGINT_create(n));

END_METHOD

BEGIN_METHOD(BigInt_Fact, GB_INTEGER n)

	mpz_t fact;
	int n = VARG(n);

	if (n < 0)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}

	mpz_init(fact);
	mpz_fac_ui(fact, n);
	GB.ReturnObject(BIGINT_create(fact));

END_METHOD

BEGIN_METHOD(BigInt_Fibonacci, GB_INTEGER n)

	mpz_t r;
	int n = VARG(n);

	if (n < 0)
	{
		GB.Error(GB_ERR_ARG);
		return;
	}

	mpz_init(r);
	mpz_fib_ui(r, n);
	GB.ReturnObject(BIGINT_create(r));

END_METHOD

BEGIN_METHOD(BigInt_GCD, GB_OBJECT a; GB_OBJECT b)

	mpz_t n;
	CBIGINT *a = VARG(a);
	CBIGINT *b = VARG(b);

	if (GB.CheckObject(a) || GB.CheckObject(b))
		return;

	mpz_init(n);
	mpz_gcd(n, a->n, b->n);
	GB.ReturnObject(BIGINT_create(n));

END_METHOD

BEGIN_METHOD(BigInt_LCM, GB_OBJECT a; GB_OBJECT b)

	mpz_t n;
	CBIGINT *a = VARG(a);
	CBIGINT *b = VARG(b);

	if (GB.CheckObject(a) || GB.CheckObject(b))
		return;

	mpz_init(n);
	mpz_lcm(n, a->n, b->n);
	GB.ReturnObject(BIGINT_create(n));

END_METHOD

BEGIN_METHOD(BigInt_FromString, GB_STRING str; GB_INTEGER base)

	CBIGINT *n;
	int base = VARGOPT(base, 10);

	if (base < 2 || base > 36)
	{
		GB.Error("Base must be between 2 and 36");
		return;
	}

	n = BIGINT_from_string(GB.ToZeroString(ARG(str)), base);
	if (!n)
	{
		GB.Error(GB_ERR_TYPE);
		return;
	}

	GB.ReturnObject(n);

END_METHOD

BEGIN_PROPERTY(BigInt_Odd)

	GB.ReturnBoolean(mpz_odd_p(NUMBER));

END_PROPERTY

BEGIN_PROPERTY(BigInt_Even)

	GB.ReturnBoolean(mpz_even_p(NUMBER));

END_PROPERTY

BEGIN_METHOD(BigInt_And, GB_OBJECT a; GB_OBJECT b)

	CBIGINT *a = VARG(a);
	CBIGINT *b = VARG(b);

	if (GB.CheckObject(a) || GB.CheckObject(b))
		return;

	GB.ReturnObject(BIGINT_make(a, b, mpz_and));

END_METHOD

BEGIN_METHOD(BigInt_Or, GB_OBJECT a; GB_OBJECT b)

	CBIGINT *a = VARG(a);
	CBIGINT *b = VARG(b);

	if (GB.CheckObject(a) || GB.CheckObject(b))
		return;

	GB.ReturnObject(BIGINT_make(a, b, mpz_ior));

END_METHOD

BEGIN_METHOD(BigInt_Xor, GB_OBJECT a; GB_OBJECT b)

	CBIGINT *a = VARG(a);
	CBIGINT *b = VARG(b);

	if (GB.CheckObject(a) || GB.CheckObject(b))
		return;

	GB.ReturnObject(BIGINT_make(a, b, mpz_xor));

END_METHOD

BEGIN_METHOD(BigInt_Not, GB_OBJECT a)

	CBIGINT *a = VARG(a);

	if (GB.CheckObject(a))
		return;

	GB.ReturnObject(BIGINT_make_unary(a, mpz_com));

END_METHOD

#define IMPLEMENT_BIT(_name, _func) \
BEGIN_METHOD(BigInt_##_name, GB_INTEGER bit) \
 \
 _func(NUMBER, VARG(bit)); \
	RETURN_SELF(); \
 \
END_METHOD

IMPLEMENT_BIT(BSet, mpz_setbit)
IMPLEMENT_BIT(BClr, mpz_clrbit)
IMPLEMENT_BIT(BChg, mpz_combit)

BEGIN_METHOD(BigInt_BTst, GB_INTEGER bit)

	GB.ReturnBoolean(mpz_tstbit(NUMBER, VARG(bit)));

END_METHOD

//---------------------------------------------------------------------------


GB_DESC BigIntDesc[] =
{
	GB_DECLARE("BigInt", sizeof(CBIGINT)),

	GB_METHOD("_new", NULL, BigInt_new, NULL),
	GB_METHOD("_free", NULL, BigInt_free, NULL),
	GB_METHOD("_compare", "i", BigInt_compare, "(Other)BigInt;"),

	GB_STATIC_METHOD("Fact", "BigInt", BigInt_Fact, "(N)i"),
	GB_STATIC_METHOD("Fibonacci", "BigInt", BigInt_Fibonacci, "(N)i"),
	GB_STATIC_METHOD("GCD", "BigInt", BigInt_GCD, "(A)BigInt;(B)BigInt;"),
	GB_STATIC_METHOD("LCM", "BigInt", BigInt_LCM, "(A)BigInt;(B)BigInt;"),
	GB_STATIC_METHOD("FromString", "BigInt", BigInt_FromString, "(String)s[(Base)i]"),

	GB_STATIC_METHOD("And", "BigInt", BigInt_And, "(A)BigInt;(B)BigInt;"),
	GB_STATIC_METHOD("Or", "BigInt", BigInt_Or, "(A)BigInt;(B)BigInt;"),
	GB_STATIC_METHOD("Xor", "BigInt", BigInt_Xor, "(A)BigInt;(B)BigInt;"),
	GB_STATIC_METHOD("Not", "BigInt", BigInt_Not, "(A)BigInt;"),

	GB_METHOD("BSet", "BigInt", BigInt_BSet, "(Bit)i"),
	GB_METHOD("BClr", "BigInt", BigInt_BClr, "(Bit)i"),
	GB_METHOD("BTst", "b", BigInt_BTst, "(Bit)i"),
	GB_METHOD("BChg", "BigInt", BigInt_BChg, "(Bit)i"),

	GB_PROPERTY_READ("Odd", "b", BigInt_Odd),
	GB_PROPERTY_READ("Even", "b", BigInt_Even),

	GB_METHOD("ToString", "s", BigInt_ToString, "[(Base)i]"),
	GB_METHOD("Shl", "BigInt", BigInt_Shl, "(Bits)i"),
	GB_METHOD("PowM", "BigInt", BigInt_PowM, "(Exp)BigInt;(Mod)BigInt;"),
	GB_METHOD("InvM", "BigInt", BigInt_InvM, "(Mod)BigInt;"),

	GB_INTERFACE("_operator", &_operator),
	GB_INTERFACE("_convert", &_convert),

	GB_END_DECLARE
};
