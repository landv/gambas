/***************************************************************************

  c_rational.c

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

#define __C_RATIONAL_C

#include "main.h"
#include "c_bigint.h"
#include "c_rational.h"

#define THIS ((CRATIONAL *)_object)
#define NUMBER (THIS->n)

//---- Utility functions & macros --------------------------------------------

static CRATIONAL _tmp;

static void from_double(mpq_t n, double f, int level)
{
	double fa;
	int nfa;
	mpq_t ni, nn;
	bool neg;

	//fprintf(stderr, "from_double: %.14g\n", f);

	if (level >= 10)
		goto __DEFAULT;

	fa = fabs(f);
	if (fa >= 1E8 || fa <= 1E-8)
		goto __DEFAULT;

	neg = (f < 0);

	nfa = (int)fa;
	if (nfa >= 1)
		fa -= nfa;

	//fprintf(stderr, "fa = %.14g %.14g\n", fa, (fa*1E8) - (int)(fa*1E8));

	if (nfa && fa < 1E-8)
	{
		mpq_set_si(n, 0, 1);
	}
	else if (((fa*1E8) - (int)(fa*1E8)) < 1E-8)
	{
		mpq_set_si(n, (int)(fa*1E8), 100000000);
	}
	else
	{
		mpq_init(ni);
		from_double(ni, 1 / fa, level + 1);
		mpq_inv(n, ni);
		mpq_clear(ni);
	}

	mpq_init(nn);
	mpq_set_si(nn, nfa, 1);
	mpq_add(n, n, nn);
	mpq_clear(nn);

	if (neg)
		mpq_neg(n, n);

	mpq_canonicalize(n);

	return;

__DEFAULT:

	mpq_set_d(n, f);
}

static void my_mpq_set_d(mpq_t n, double f)
{
	from_double(n, f, 0);
}

#define mpq_init_set_d(_n, _f) (mpq_init(_n), my_mpq_set_d(_n, _f))
#define mpq_init_set_si(_n, _i) (mpq_init(_n), mpq_set_si(_n, _i, 1))

//---- Rational number creation ----------------------------------------------

CRATIONAL *RATIONAL_create(mpq_t number)
{
	CRATIONAL *c;

	c = (CRATIONAL *)GB.New(CLASS_Rational, NULL, NULL);
	mpq_set(c->n, number);
	//mpq_canonicalize(c->n);
	mpq_clear(number);

	return c;
}

#define RATIONAL_make(__a, _b, _op) \
({ \
	CRATIONAL *_a = __a; \
	if ((_a)->ob.ref <= 1) \
		_op((_a)->n, (_a)->n, (_b)->n); \
	else \
	{ \
		mpq_t n; \
		mpq_init(n); \
		_op(n, (_a)->n, (_b)->n); \
		_a = RATIONAL_create(n); \
	} \
	_a; \
})

#define RATIONAL_make_unary(_a, _op) \
({ \
	if ((_a)->ob.ref <= 1) \
		_op((_a)->n, (_a)->n); \
	else \
	{ \
		mpq_t n; \
		mpq_init(n); \
		_op(n, (_a)->n); \
		_a = RATIONAL_create(n); \
	} \
	_a; \
})

#define RATIONAL_make_int(__a, _f, _op) \
({ \
	CRATIONAL *_a = (__a); \
	if (_a->ob.ref <= 1) \
		_op(_a->n, _a->n, (ulong)(_f)); \
	else \
	{ \
		mpq_t n; \
		mpq_init(n); \
		_op(n, _a->n, (ulong)(_f)); \
		_a = RATIONAL_create(n); \
	} \
	_a; \
})

#define RATIONAL_make_int_invert(_a, _f, _op) \
({ \
	if ((_a)->ob.ref <= 1) \
		_op((_a)->n, (ulong)(_f), (_a)->n); \
	else \
	{ \
		mpq_t n; \
		mpq_init(n); \
		_op(n, (ulong)(_f), (_a)->n); \
		_a = RATIONAL_create(n); \
	} \
	_a; \
})

#define RATIONAL_make_bit(__a, _n, _op) \
({ \
	CRATIONAL *_a = (__a); \
	if (_a->ob.ref <= 1) \
		_op(_a->n, _a->n, (_n)); \
	else \
	{ \
		mpq_t n; \
		mpq_init(n); \
		_op(n, _a->n, (_n)); \
		_a = RATIONAL_create(n); \
	} \
	_a; \
})

//---- Arithmetic operators -------------------------------------------------

static CRATIONAL *_add(CRATIONAL *a, CRATIONAL *b, bool invert)
{
	return RATIONAL_make(a, b, mpq_add);
}

static CRATIONAL *_addf(CRATIONAL *a, double f, bool invert)
{
	my_mpq_set_d(_tmp.n, f);
	return RATIONAL_make(a, &_tmp, mpq_add);
}

static CRATIONAL *_addo(CRATIONAL *a, void *o, bool invert)
{
	if (GB.Is(o, CLASS_BigInt))
	{
		mpq_set_z(_tmp.n, ((CBIGINT *)o)->n);
		return RATIONAL_make(a, &_tmp, mpq_add);
	}
	else
		return NULL;
}

static CRATIONAL *_sub(CRATIONAL *a, CRATIONAL *b, bool invert)
{
	return RATIONAL_make(a, b, mpq_sub);
}

static CRATIONAL *_subf(CRATIONAL *a, double f, bool invert)
{
	my_mpq_set_d(_tmp.n, f);

	if (invert)
		return RATIONAL_make(&_tmp, a, mpq_sub);
	else
		return RATIONAL_make(a, &_tmp, mpq_sub);
}

static CRATIONAL *_subo(CRATIONAL *a, void *o, bool invert)
{
	if (GB.Is(o, CLASS_BigInt))
	{
		mpq_set_z(_tmp.n, ((CBIGINT *)o)->n);
		if (invert)
			return RATIONAL_make(&_tmp, a, mpq_sub);
		else
			return RATIONAL_make(a, &_tmp, mpq_sub);
	}
	else
		return NULL;
}

static CRATIONAL *_mul(CRATIONAL *a, CRATIONAL *b, bool invert)
{
	return RATIONAL_make(a, b, mpq_mul);
}

static CRATIONAL *_mulf(CRATIONAL *a, double f, bool invert)
{
	my_mpq_set_d(_tmp.n, f);
	return RATIONAL_make(a, &_tmp, mpq_mul);
}

static CRATIONAL *_mulo(CRATIONAL *a, void *o, bool invert)
{
	if (GB.Is(o, CLASS_BigInt))
	{
		mpq_set_z(_tmp.n, ((CBIGINT *)o)->n);
		return RATIONAL_make(a, &_tmp, mpq_mul);
	}
	else
		return NULL;
}

static CRATIONAL *_div(CRATIONAL *a, CRATIONAL *b, bool invert)
{
	if (mpq_cmp_si(b->n, 0, 1) == 0)
	{
		GB.Error(GB_ERR_ZERO);
		return NULL;
	}
	else
		return RATIONAL_make(a, b, mpq_div);
}

static CRATIONAL *_divf(CRATIONAL *a, double f, bool invert)
{
	my_mpq_set_d(_tmp.n, f);
	if (invert)
		return _div(&_tmp, a, FALSE);
	else
		return _div(a, &_tmp, FALSE);
}

static CRATIONAL *_divo(CRATIONAL *a, void *o, bool invert)
{
	if (GB.Is(o, CLASS_BigInt))
	{
		mpq_set_z(_tmp.n, ((CBIGINT *)o)->n);
		if (invert)
			return _div(&_tmp, a, FALSE);
		else
			return _div(a, &_tmp, FALSE);
	}
	else
		return NULL;
}

static int _equal(CRATIONAL *a, CRATIONAL *b, bool invert)
{
	return mpq_equal(a->n, b->n);
}

static int _equalf(CRATIONAL *a, double f, bool invert)
{
	my_mpq_set_d(_tmp.n, f);
	return mpq_equal(a->n, _tmp.n);
}

static int _equalo(CRATIONAL *a, void *o, bool invert)
{
	if (GB.Is(o, CLASS_BigInt))
	{
		mpq_set_z(_tmp.n, ((CBIGINT *)o)->n);
		return mpq_equal(a->n, _tmp.n);
	}
	else
		return -1;
}

static int _comp(CRATIONAL *a, CRATIONAL *b, bool invert)
{
	return mpq_cmp(a->n, b->n);
}

static int _compf(CRATIONAL *a, double f, bool invert)
{
	my_mpq_set_d(_tmp.n, f);
	return mpq_cmp(a->n, _tmp.n);
}

static int _compo(CRATIONAL *a, void *o, bool invert)
{
	if (GB.Is(o, CLASS_BigInt))
	{
		mpq_set_z(_tmp.n, ((CBIGINT *)o)->n);
		return mpq_cmp(a->n, _tmp.n);
	}
	else
		return -2;
}

static CRATIONAL *_neg(CRATIONAL *a)
{
	return RATIONAL_make_unary(a, mpq_neg);
}

static CRATIONAL *_abs(CRATIONAL *a)
{
	return RATIONAL_make_unary(a, mpq_abs);
}

static int _sgn(CRATIONAL *a)
{
	return mpq_sgn(a->n);
}

static CRATIONAL *_powf(CRATIONAL *a, double f, bool invert)
{
	ulong p;
	mpz_t num, den;
	mpq_t n;

	if (invert || (double)(int)f != f)
		return NULL;

	if (f < 0)
	{
		f = (-f);
		invert = TRUE;
	}

	p = (ulong)f;

	mpz_init(num);
	mpz_pow_ui(num, mpq_numref(a->n), p);

	mpz_init(den);
	mpz_pow_ui(den, mpq_denref(a->n), p);

	mpq_init(n);
	if (invert)
		mpz_swap(num, den);

	if (mpz_cmp_si(den, 0) == 0)
	{
		GB.Error(GB_ERR_ZERO);
		return NULL;
	}

	mpq_set_num(n, num);
	mpq_set_den(n, den);

	mpz_clear(num);
	mpz_clear(den);

	mpq_canonicalize(n);

	return RATIONAL_create(n);
}

static CRATIONAL *_powo(CRATIONAL *a, void *o, bool invert)
{
	CBIGINT *b;

	if (invert || !GB.Is(o, CLASS_BigInt))
		return NULL;

	b = (CBIGINT *)o;

	if (!mpz_fits_slong_p(b->n))
	{
		GB.Error(GB_ERR_OVERFLOW);
		return NULL;
	}

	return _powf(a, (double)mpz_get_si(b->n), invert);
}

static GB_OPERATOR_DESC _operator =
{
	.equal   = (void *)_equal,
	.equalf  = (void *)_equalf,
	.equalo  = (void *)_equalo,
	.comp    = (void *)_comp,
	.compf   = (void *)_compf,
	.compo   = (void *)_compo,
	.add     = (void *)_add,
	.addf    = (void *)_addf,
	.addo    = (void *)_addo,
	.sub     = (void *)_sub,
	.subf    = (void *)_subf,
	.subo    = (void *)_subo,
	.mul     = (void *)_mul,
	.mulf    = (void *)_mulf,
	.mulo    = (void *)_mulo,
	.div     = (void *)_div,
	.divf    = (void *)_divf,
	.divo    = (void *)_divo,
	.powo    = (void *)_powo,
	.powf    = (void *)_powf,
	.abs     = (void *)_abs,
	.neg     = (void *)_neg,
	.sgn     = (void *)_sgn
};

//---- Conversions ----------------------------------------------------------

char *RATIONAL_to_string(mpq_t n, int base)
{
	char *str;
	int len;

	len = mpz_sizeinbase (mpq_numref(n), base) + mpz_sizeinbase(mpq_denref(n), base) + 2;
	if (mpq_sgn(n) < 0)
		len++;

	str = GB.NewString(NULL, len);
	memset(str, 0, len);
	mpq_get_str(str, -base, n);

	while (len > 0 && str[len - 1] == 0)
	{
		str = GB.ExtendString(str, len - 1);
		len--;
	}

	return str;
}

CRATIONAL *RATIONAL_from_string(char *str, int base)
{
	mpq_t n;

	mpq_init(n);
	if (mpq_set_str(n, str, base))
	{
		mpq_clear(n);
		return NULL;
	}
	else
	{
		mpq_canonicalize(n);
		return RATIONAL_create(n);
	}
}

static bool _convert(CRATIONAL *a, GB_TYPE type, GB_VALUE *conv)
{
	if (a)
	{
		switch (type)
		{
			case GB_T_FLOAT:
				conv->_float.value = mpq_get_d(a->n);
				return FALSE;

			case GB_T_SINGLE:
				conv->_single.value = mpq_get_d(a->n);
				return FALSE;

			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				conv->_integer.value = (int)mpq_get_d(a->n);
				return FALSE;

			case GB_T_LONG:
				conv->_long.value = (int64_t)mpq_get_d(a->n);
				return FALSE;

			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_string.value.addr = RATIONAL_to_string(a->n, 10); //, type == GB_T_CSTRING);
				conv->_string.value.start = 0;
				conv->_string.value.len = GB.StringLength(conv->_string.value.addr);
				return FALSE;

			default:

				if (type == CLASS_BigInt)
				{
					mpz_t n;

					mpz_init(n);
					mpz_tdiv_q(n, mpq_numref(a->n), mpq_denref(a->n));

					conv->_object.value = BIGINT_create(n);

					return FALSE;
				}

				return TRUE;
		}
	}
	else
	{
		mpq_t n;

		switch(type)
		{
			case GB_T_FLOAT:
				mpq_init_set_d(n, conv->_float.value);
				conv->_object.value = RATIONAL_create(n);
				return FALSE;

			case GB_T_SINGLE:
				mpq_init_set_d(n, conv->_single.value);
				conv->_object.value = RATIONAL_create(n);
				return FALSE;

			case GB_T_INTEGER:
			case GB_T_SHORT:
			case GB_T_BYTE:
				mpq_init_set_si(n, (long)conv->_integer.value);
				conv->_object.value = RATIONAL_create(n);
				return FALSE;

			case GB_T_LONG:
				mpq_init_set_si(n, (long)conv->_long.value);
				conv->_object.value = RATIONAL_create(n);
				return FALSE;

			case GB_T_STRING:
			case GB_T_CSTRING:
				conv->_object.value = RATIONAL_from_string(GB.ToZeroString(&conv->_string), 10);
				return conv->_object.value == NULL;

			default:

				if (type == CLASS_BigInt)
				{
					mpq_init(n);
					mpq_set_z(n, ((CBIGINT *)conv->_object.value)->n);
					conv->_object.value = RATIONAL_create(n);
					return FALSE;
				}

				return TRUE;
		}
	}
}

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(Rational_init)

	_tmp.ob.ref = 2;
	mpq_init(_tmp.n);

END_METHOD

BEGIN_METHOD_VOID(Rational_exit)

	mpq_clear(_tmp.n);

END_METHOD

BEGIN_METHOD_VOID(Rational_new)

	mpq_init(THIS->n);

END_METHOD

BEGIN_METHOD_VOID(Rational_free)

	mpq_clear(THIS->n);

END_METHOD

BEGIN_METHOD(Rational_compare, GB_OBJECT other)

	CRATIONAL *other = VARG(other);

	if (GB.CheckObject(other))
		return;

	GB.ReturnInteger(mpq_cmp(NUMBER, other->n));

END_METHOD

BEGIN_METHOD(Rational_ToString, GB_INTEGER base)

	char *str;
	int base = VARGOPT(base, 10);

	if (base < 2 || base > 36)
	{
		GB.Error("Base must be between 2 and 36");
		return;
	}

	str = RATIONAL_to_string(NUMBER, base);
	GB.FreeStringLater(str);
	GB.ReturnString(str);

END_METHOD

BEGIN_METHOD(Rational_FromString, GB_STRING str; GB_INTEGER base)

	CRATIONAL *n;
	int base = VARGOPT(base, 10);

	if (base < 2 || base > 36)
	{
		GB.Error("Base must be between 2 and 36");
		return;
	}

	n = RATIONAL_from_string(GB.ToZeroString(ARG(str)), base);
	if (!n)
	{
		GB.Error(GB_ERR_TYPE);
		return;
	}

	GB.ReturnObject(n);

END_METHOD

BEGIN_PROPERTY(Rational_Num)

	if (READ_PROPERTY)
	{
		mpz_t n;
		mpz_init(n);
		mpq_get_num(n, NUMBER);
		GB.ReturnObject(BIGINT_create(n));
	}
	else
	{
		CBIGINT *num = VPROP(GB_OBJECT);

		if (GB.CheckObject(num))
			return;

		mpq_set_num(NUMBER, num->n);
		mpq_canonicalize(NUMBER);
	}

END_PROPERTY

BEGIN_PROPERTY(Rational_Den)

	if (READ_PROPERTY)
	{
		mpz_t n;
		mpz_init(n);
		mpq_get_den(n, NUMBER);
		GB.ReturnObject(BIGINT_create(n));
	}
	else
	{
		CBIGINT *den = VPROP(GB_OBJECT);

		if (GB.CheckObject(den))
			return;

		mpq_set_den(NUMBER, den->n);
		mpq_canonicalize(NUMBER);
	}

END_PROPERTY

//---------------------------------------------------------------------------


GB_DESC RationalDesc[] =
{
	GB_DECLARE("Rational", sizeof(CRATIONAL)),

	GB_STATIC_METHOD("_init", NULL, Rational_init, NULL),
	GB_STATIC_METHOD("_exit", NULL, Rational_exit, NULL),
	GB_METHOD("_new", NULL, Rational_new, NULL),
	GB_METHOD("_free", NULL, Rational_free, NULL),
	GB_METHOD("_compare", "i", Rational_compare, "(Other)Rational;"),

	GB_PROPERTY("Num", "BigInt", Rational_Num),
	GB_PROPERTY("Den", "BigInt", Rational_Den),

	GB_STATIC_METHOD("FromString", "Rational", Rational_FromString, "(String)s[(Base)i]"),

	GB_METHOD("ToString", "s", Rational_ToString, "[(Base)i]"),

	GB_INTERFACE("_operator", &_operator),
	GB_INTERFACE("_convert", &_convert),

	GB_END_DECLARE
};
