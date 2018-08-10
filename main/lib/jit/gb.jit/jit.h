#define GB_JIT 1

#include <math.h>
#include <setjmp.h>
//#include <stdio.h>

#define TRUE 1
#define FALSE 0

// __attribute__((noreturn)) makes gcc dizzy and slow as hell
#define NORETURN 

#define E_NEPARAM    4
#define E_TMPARAM    5
#define E_OVERFLOW   7
#define E_NOBJECT   12
#define E_NULL      13
#define E_MATH      19
#define E_ARG       20
#define E_BOUND     21
#define E_ZERO      26
#define E_IOBJECT   29

static inline double frac(double x)
{
  x = fabs(x);
  return x - floor(x);
}

#ifdef __clang__
#define __builtin_exp10 exp10
static inline double exp10(double x)
{
  return pow(10, x);
}
#endif

typedef
  unsigned char uchar;

typedef
  GB_VALUE VALUE;

typedef
  ushort PCODE;

typedef
	void (*EXEC_FUNC)();

typedef
	void (*EXEC_FUNC_CODE)(ushort);

typedef
  struct {
    GB_TYPE type;
    GB_CLASS class;
    void *super;
    }
  GB_VALUE_CLASS;

typedef
  struct {
    GB_TYPE type;
    void *object;
    void *super;
    }
  GB_VALUE_OBJECT;

typedef
  struct {
    GB_TYPE type;
    void *class;
    void *object;
    char kind;
    char defined;
    short index;
    }
  GB_VALUE_FUNCTION;

typedef
  struct {
    GB_TYPE type;
    void *addr;
    void *gp;
  }
  GB_VALUE_GOSUB;
  
typedef
	struct {
		GB_BASE object;
		int size;
		int count;
		GB_TYPE type;
		void *data;
		int *dim;
		void *ref;
		}
	GB_ARRAY_IMPL;

#define SP (*psp)
#define PC (JIT.exec->pc)
#define CP (JIT.exec->cp)
#define OP (JIT.exec->op)
#define FP (JIT.exec->fp)
#define PP (JIT.exec->pp)
#define BP (JIT.exec->bp)

#define CHECK_FINITE(_val) ({ if (!isfinite(_val)) JIT.throw(E_OVERFLOW); (_val); })

#define PARAM_b(_p) (JIT.conv(&sp[-n+(_p)], GB_T_BOOLEAN), sp[-n+(_p)]._boolean.value)
#define PARAM_c(_p) (JIT.conv(&sp[-n+(_p)], GB_T_BYTE), (uchar)(sp[-n+(_p)]._integer.value))
#define PARAM_h(_p) (JIT.conv(&sp[-n+(_p)], GB_T_SHORT), (short)(sp[-n+(_p)]._integer.value))
#define PARAM_i(_p) (JIT.conv(&sp[-n+(_p)], GB_T_INTEGER), sp[-n+(_p)]._integer.value)
#define PARAM_l(_p) (JIT.conv(&sp[-n+(_p)], GB_T_LONG), sp[-n+(_p)]._long.value)
#define PARAM_g(_p) (JIT.conv(&sp[-n+(_p)], GB_T_SINGLE), sp[-n+(_p)]._single.value)
#define PARAM_f(_p) (JIT.conv(&sp[-n+(_p)], GB_T_FLOAT), sp[-n+(_p)]._float.value)
#define PARAM_p(_p) (JIT.conv(&sp[-n+(_p)], GB_T_POINTER), sp[-n+(_p)]._pointer.value)
#define PARAM_d(_p) (JIT.conv(&sp[-n+(_p)], GB_T_DATE), *(GB_DATE *)&sp[-n+(_p)])
#define PARAM_s(_p) (JIT.conv(&sp[-n+(_p)], GB_T_STRING), *(GB_STRING *)&sp[-n+(_p)])
#define PARAM_o(_p) (JIT.conv(&sp[-n+(_p)], GB_T_OBJECT), *(GB_OBJECT *)&sp[-n+(_p)])
#define PARAM_v(_p) (JIT.conv(&sp[-n+(_p)], GB_T_VARIANT), *(GB_VARIANT *)&sp[-n+(_p)])
#define PARAM_O(_p, _type) (JIT.conv(&sp[-n+(_p)], (GB_TYPE)(_type)), *(GB_OBJECT *)&sp[-n+(_p)])

#define PARAM_OPT(_p, _type, _default) (((_p) >= n || (sp[-n+(_p)].type == GB_T_VOID)) ? (_default) : PARAM_##_type(_p))
#define PARAM_OPT_b(_p) PARAM_OPT(_p, b, 0)
#define PARAM_OPT_c(_p) PARAM_OPT(_p, c, 0)
#define PARAM_OPT_h(_p) PARAM_OPT(_p, h, 0)
#define PARAM_OPT_i(_p) PARAM_OPT(_p, i, 0)
#define PARAM_OPT_l(_p) PARAM_OPT(_p, l, 0)
#define PARAM_OPT_g(_p) PARAM_OPT(_p, g, 0)
#define PARAM_OPT_f(_p) PARAM_OPT(_p, f, 0)
#define PARAM_OPT_p(_p) PARAM_OPT(_p, p, 0)
#define PARAM_OPT_d(_p) PARAM_OPT(_p, d, ({ GB_DATE _v; _v.type = GB_T_DATE; _v.value.time = _v.value.date = 0; _v; }))
#define PARAM_OPT_s(_p) PARAM_OPT(_p, d, GET_CSTRING("", 0, 0))
#define PARAM_OPT_o(_p) PARAM_OPT(_p, d, GET_OBJECT(GB_T_OBJECT, 0))
#define PARAM_OPT_v(_p) PARAM_OPT(_p, d, ({ GB_VARIANT _v; _v.type = GB_T_VARIANT; _v.value.type = GB_T_NULL; _v; }))
#define PARAM_OPT_O(_p, _type) (((_p) >= n || (sp[-n+(_p)].type == GB_T_VOID)) ? GET_OBJECT(GB_T_OBJECT, 0) : PARAM_O(_p, _type))

#define OPT(_p, _n) ({ \
  uchar _opt = 0; \
  GB_VALUE *_param = &sp[-n+(_p)]; \
  for (int _i = 0; _i < (_n); _i++) \
  { \
    if (((_i + (_p)) >= n) || _param->type == GB_T_VOID) \
      _opt |= (1 << _i); \
    _param++; \
  } \
  _opt; \
})

#define RETURN_b(_val) (GB.ReturnBoolean((int)_val))
#define RETURN_c(_val) (GB.Return(GB_T_BYTE, (int)(_val)))
#define RETURN_h(_val) (GB.Return(GB_T_SHORT, (int)(_val)))
#define RETURN_i(_val) (GB.ReturnInteger(_val))
#define RETURN_l(_val) (GB.ReturnLong(_val))
#define RETURN_g(_val) (GB.ReturnSingle(_val))
#define RETURN_f(_val) (GB.ReturnFloat(_val))
#define RETURN_d(_val) ({ GB_DATE _v = (_val); GB.ReturnDate(&_v); })
#define RETURN_p(_val) (GB.ReturnPointer(_val))
#define RETURN_o(_val) ({ GB_OBJECT _v = (_val); GB.Return(_v.type, _v.value); })
#define RETURN_v(_val) ({ GB_VARIANT _v = (_val); GB.ReturnVariant(&_v.value); })
#define RETURN_s(_val) (*(GB_STRING *)GB.GetReturnValue() = (_val))

#define PUSH_b(_val) ({ char _v = -(_val); sp->_boolean.value = _v; sp->type = GB_T_BOOLEAN; sp++; })
#define PUSH_c(_val) ({ uchar _v = (_val); sp->_integer.value = _v; sp->type = GB_T_BYTE; sp++; })
#define PUSH_h(_val) ({ short _v = (_val); sp->_integer.value = _v; sp->type = GB_T_SHORT; sp++; })
#define PUSH_i(_val) ({ sp->_integer.value = (_val); sp->type = GB_T_INTEGER; sp++; })
#define PUSH_l(_val) ({ int64_t _v = (_val); sp->_long.value = _v; sp->type = GB_T_LONG; sp++; })
#define PUSH_g(_val) ({ float _v = (_val); sp->_single.value = _v; sp->type = GB_T_SINGLE; sp++; })
#define PUSH_f(_val) ({ double _v = (_val); sp->_float.value = _v; sp->type = GB_T_FLOAT; sp++; })
#define PUSH_p(_val) ({ intptr_t _v = (_val); sp->_pointer.value = _v; sp->type = GB_T_POINTER; sp++; })
#define PUSH_d(_val) ({ *((GB_DATE *)sp) = (_val); sp++; })
#define PUSH_t(_val) ({ *((GB_STRING *)sp) = (_val); sp++; })
#define PUSH_s(_val) ({ *((GB_STRING *)sp) = (_val); if (sp->type == GB_T_STRING) GB.RefString(sp->_string.value.addr); sp++; })
#define PUSH_o(_val) ({ *((GB_OBJECT *)sp) = (_val); GB.Ref(sp->_object.value); sp++; })
#define PUSH_v(_val) ({ *((GB_VARIANT *)sp) = (_val); GB.BorrowValue(sp); sp++; })
#define PUSH_C(_val) ({ GB_VALUE_CLASS _v; _v.type = GB_T_CLASS; _v.class = (_val); *((GB_VALUE_CLASS *)sp) = _v; sp++; })
#define PUSH_u(_val) ({ *sp = (_val); GB.BorrowValue(sp); sp++; })
#define PUSH_V() ({ sp->type = GB_T_VOID; sp++; })
#define PUSH_n(_val) ({ sp->type = GB_T_NULL; sp++; })

enum
{
  FUNCTION_NULL,
  FUNCTION_NATIVE,
  FUNCTION_PRIVATE,
  FUNCTION_PUBLIC,
  FUNCTION_EVENT,
  FUNCTION_EXTERN,
  FUNCTION_UNKNOWN,
  FUNCTION_CALL,
	FUNCTION_SUBR
};

#define PUSH_PRIVATE_FUNCTION(_index) ({ \
  GB_VALUE_FUNCTION *f = (GB_VALUE_FUNCTION *)sp; \
  f->type = GB_T_FUNCTION; \
  f->class = CP; \
  f->object = OP; \
  f->kind = FUNCTION_PRIVATE; \
  f->index = (_index); \
  f->defined = 1; \
  GB.Ref(OP); \
  sp++; \
})

#define POP_b() ({ sp--; sp->_boolean.value; })
#define POP_c() ({ sp--; (uchar)sp->_integer.value; })
#define POP_h() ({ sp--; (short)sp->_integer.value; })
#define POP_i() ({ sp--; sp->_integer.value; })
#define POP_l() ({ sp--; sp->_long.value; })
#define POP_g() ({ sp--; sp->_single.value; })
#define POP_f() ({ sp--; sp->_float.value; })
#define POP_p() ({ sp--; sp->_pointer.value; })
#define POP_d() ({ sp--; *((GB_DATE*)sp); })
#define POP_s() ({ sp--; JIT.unborrow(sp); *((GB_STRING*)sp); })
#define POP_t() ({ sp--; *((GB_STRING*)sp); })
#define POP_BORROW_s() ({ sp--; *((GB_STRING*)sp); })
#define POP_o() ({ sp--; JIT.unborrow(sp); *((GB_OBJECT*)sp); })
#define POP_BORROW_o() ({ sp--; *((GB_OBJECT*)sp); })
#define POP_v() ({ sp--; JIT.unborrow(sp); *((GB_VARIANT*)sp); })
#define POP_BORROW_v() ({ sp--; *((GB_VARIANT*)sp); })
#define POP_u() ({ sp--; JIT.unborrow(sp); *sp; })
#define POP_BORROW_u() ({ sp--; *sp })
#define POP_V() (sp--)

#define BORROW_s(_val) ({ GB_STRING _v = (_val); if ((_v).type == GB_T_STRING) GB.RefString(_v.value.addr); _v; })
#define BORROW_o(_val) ({ GB_OBJECT _v = (_val); GB.Ref(_v.value); _v; })
#define BORROW_v(_val) ({ GB_VARIANT _v = (_val); GB.BorrowValue((GB_VALUE *)&_v); _v; })

#define RELEASE_s(_val) ({ GB_STRING _v = (_val); if ((_v).type == GB_T_STRING) GB.FreeString(&_v.value.addr); _v; })
#define RELEASE_o(_val) ({ GB_OBJECT _v = (_val); GB.Unref(&_v.value); _v; })
#define RELEASE_v(_val) ({ GB_VARIANT _v = (_val); GB.ReleaseValue((GB_VALUE *)&_v); _v; })

#define RELEASE_FAST_s(_val) ({ if ((_val).type == GB_T_STRING) GB.FreeString(&(_val).value.addr); })
#define RELEASE_FAST_o(_val) GB.Unref(&((_val).value))
#define RELEASE_FAST_v(_val) GB.ReleaseValue((GB_VALUE *)&(_val))

#define CLASS(_class) ((GB_CLASS)(_class))

#define CONSTANT_s(_addr, _len) GET_CSTRING((char *)_addr, 0, _len)
#define CONSTANT_t(_addr, _len) GET_CSTRING(GB.Translate((const char *)_addr), 0, strlen(temp.value.addr))

#define GET_CHAR(_char) GET_CSTRING(&JIT.char_table[(_char) * 2], 0, 1)
  
#define GET_STRING(_addr, _start, _len) ({ \
  GB_STRING temp; \
  temp.type = GB_T_STRING; \
  temp.value.addr = (char *)(_addr); \
  temp.value.start = (_start); \
  temp.value.len = (_len); \
  temp; })

#define GET_CSTRING(_addr, _start, _len) ({ \
  GB_STRING temp; \
  temp.type = GB_T_CSTRING; \
  temp.value.addr = (char *)(_addr); \
  temp.value.start = (_start); \
  temp.value.len = (_len); \
  temp; })
  
#define GET_OBJECT(_addr, _type) ({ \
  GB_OBJECT temp; \
  temp.type = (GB_TYPE)(_type); \
  temp.value = (_addr); \
  temp; })

#define GET_DATE(_addr) ({ \
  GB_DATE temp; \
  int *_val = (int *)(_addr); \
  temp.type = GB_T_DATE; \
  temp.value.date = _val[0]; \
  temp.value.time = _val[1]; \
  temp; \
})

#define GET_VARIANT(_val) ({ \
  GB_VARIANT temp; \
  temp.type = GB_T_VARIANT; \
  temp.value = (_val); \
  temp; })

#define GET_FUNCTION(_pc) ({ CALL_UNKNOWN(_pc); POP_u(); })

// TODO: automatic class
#define ADDR(_val) ({ \
  char *_object = (_val).value; \
  if (!_object) JIT.throw(E_NULL); \
  _object; \
})

#define ADDR_CHECK(_check, _val) ({ \
  char *_object = (_val).value; \
  if (!_object) JIT.throw(E_NULL); \
  if (((void (*)())_check)(_object)) JIT.throw(E_IOBJECT); \
  _object; \
})

#define ADDR_UNSAFE(_val) ((char *)((_val).value))

#define GET_b(_addr) (*(bool *)(_addr))
#define GET_c(_addr) (*(uchar *)()_addr))
#define GET_h(_addr) (*(short *)(_addr))
#define GET_i(_addr) (*(int *)(_addr))
#define GET_l(_addr) (*(int64_t *)(_addr))
#define GET_g(_addr) (*(float *)(_addr))
#define GET_f(_addr) (*(double *)(_addr))
#define GET_p(_addr) (*(intrptr_t *)(_addr))
#define GET_s(_addr) GET_STRING((*(char **)(_addr)), 0, GB.StringLength(temp.value.addr))
#define GET_o(_addr, _type) GET_OBJECT((*(char **)(_addr)), _type)
#define GET_v(_addr) GET_VARIANT((*(GB_VARIANT_VALUE *)(_addr)))
#define GET_S(_ref, _addr, _type) GET_OBJECT(JIT.static_struct((_ref), (_type), (_addr)), _type)

#define GET_A(_class, _ref, _addr, _type, _desc) ({ \
  SP = sp; \
  GB.Unref(&ra); \
  ra = JIT.static_array((_class), (_ref), (void *)(_desc), (_addr)); \
  GET_OBJECT(ra, _type); \
})

#define SET_b(_addr, _val) (GET_b(_addr) = (_val))
#define SET_c(_addr, _val) (GET_c(_addr) = (_val))
#define SET_h(_addr, _val) (GET_h(_addr) = (_val))
#define SET_i(_addr, _val) (GET_i(_addr) = (_val))
#define SET_l(_addr, _val) (GET_l(_addr) = (_val))
#define SET_g(_addr, _val) (GET_g(_addr) = (_val))
#define SET_f(_addr, _val) (GET_f(_addr) = (_val))
#define SET_p(_addr, _val) (GET_p(_addr) = (_val))
#define SET_s(_addr, _val) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.StoreString((GB_STRING *)&temp, (char **)(_addr)); })
#define SET_o(_addr, _val) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.StoreObject((GB_OBJECT *)&temp, (void **)(_addr)); })
#define SET_v(_addr, _val) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.StoreVariant((GB_VARIANT *)&temp, (GB_VARIANT_VALUE *)(_addr)); })

#define GET_ARRAY_UNSAFE(_type, _array, _index) ({ \
  GB_ARRAY_IMPL *_a = (_array).value; \
  int _i = (_index); \
  &((_type *)_a->data)[_i]; \
})

#define GET_ARRAY(_type, _array, _index) ({ \
  GB_ARRAY_IMPL *_a = (_array).value; \
  int _i = (_index); \
  if (!_a) JIT.throw(E_NOBJECT); \
  if (_i < 0 || _i >= _a->count) JIT.throw(E_BOUND); \
  &((_type *)_a->data)[_i]; \
})

#define PUSH_ARRAY(_type, _array, _index, _unsafe) *GET_ARRAY##_unsafe(_type, _array, _index)

#define PUSH_ARRAY_b(_array, _index, _unsafe) PUSH_ARRAY(bool, _array, _index, _unsafe)
#define PUSH_ARRAY_c(_array, _index, _unsafe) PUSH_ARRAY(uchar, _array, _index, _unsafe)
#define PUSH_ARRAY_h(_array, _index, _unsafe) PUSH_ARRAY(short, _array, _index, _unsafe)
#define PUSH_ARRAY_i(_array, _index, _unsafe) PUSH_ARRAY(int, _array, _index, _unsafe)
#define PUSH_ARRAY_l(_array, _index, _unsafe) PUSH_ARRAY(int64_t, _array, _index, _unsafe)
#define PUSH_ARRAY_g(_array, _index, _unsafe) PUSH_ARRAY(float, _array, _index, _unsafe)
#define PUSH_ARRAY_f(_array, _index, _unsafe) PUSH_ARRAY(double, _array, _index, _unsafe)
#define PUSH_ARRAY_p(_array, _index, _unsafe) PUSH_ARRAY(intptr_t, _array, _index, _unsafe)
#define PUSH_ARRAY_d(_array, _index, _unsafe) GET_DATE(GET_ARRAY##_unsafe(int, _array, _index))
#define PUSH_ARRAY_s(_array, _index, _unsafe) GET_STRING(PUSH_ARRAY(char *, _array, _index, _unsafe), 0, GB.StringLength(temp.value.addr))
#define PUSH_ARRAY_o(_array, _index, _unsafe) GET_OBJECT(PUSH_ARRAY(void *, _array, _index, _unsafe), GB_T_OBJECT)
#define PUSH_ARRAY_O(_array, _index, _type, _unsafe) GET_OBJECT(PUSH_ARRAY(void *, _array, _index, _unsafe), _type)
#define PUSH_ARRAY_v(_array, _index, _unsafe) GET_VARIANT(PUSH_ARRAY(GB_VARIANT_VALUE, _array, _index, _unsafe))

#define POP_ARRAY(_type, _array, _index, _val, _unsafe) (*GET_ARRAY##_unsafe(_type, _array, _index) = (_val))

#define POP_ARRAY_b(_array, _index, _val, _unsafe) POP_ARRAY(bool, _array, _index, _val, _unsafe)
#define POP_ARRAY_c(_array, _index, _val, _unsafe) POP_ARRAY(uchar, _array, _index, _val, _unsafe)
#define POP_ARRAY_h(_array, _index, _val, _unsafe) POP_ARRAY(short, _array, _index, _val, _unsafe)
#define POP_ARRAY_i(_array, _index, _val, _unsafe) POP_ARRAY(int, _array, _index, _val, _unsafe)
#define POP_ARRAY_l(_array, _index, _val, _unsafe) POP_ARRAY(int64_t, _array, _index, _val, _unsafe)
#define POP_ARRAY_g(_array, _index, _val, _unsafe) POP_ARRAY(float, _array, _index, _val, _unsafe)
#define POP_ARRAY_f(_array, _index, _val, _unsafe) POP_ARRAY(double, _array, _index, _val, _unsafe)
#define POP_ARRAY_p(_array, _index, _val, _unsafe) POP_ARRAY(intptr_t, _array, _index, _val, _unsafe)
#define POP_ARRAY_d(_array, _index, _val, _unsafe) ({ GB_DATE temp = (GB_DATE)(_val); int *_p = (int *)GET_ARRAY##_unsafe(int, _array, _index)); _p[0] = temp.value.date; _p[1] = temp.value.time; })
#define POP_ARRAY_s(_array, _index, _val, _unsafe) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.StoreString((GB_STRING *)&temp, GET_ARRAY##_unsafe(char *, _array, _index)); })
#define POP_ARRAY_o(_array, _index, _val, _unsafe) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.StoreObject((GB_OBJECT *)&temp, GET_ARRAY##_unsafe(void *, _array, _index)); })
#define POP_ARRAY_v(_array, _index, _val, _unsafe) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.StoreVariant((GB_VARIANT *)&temp, GET_ARRAY##_unsafe(GB_VARIANT_VALUE, _array, _index)); })

#define CONV(_val, _src, _dest, _type) ({ PUSH_##_src(_val); JIT.conv(sp - 1, (GB_TYPE)(_type)); POP_##_dest(); })

#define CONV_d_b(_val) ({ GB_DATE _v = (_val); _v.value.date != 0 || _v.value.time != 0; })
#define CONV_d_c(_val) ((uchar)((_val).value.date))
#define CONV_d_h(_val) ((short)((_val).value.date))
#define CONV_d_i(_val) ((_val).value.date)
#define CONV_d_l(_val) ((int64_t)((_val).value.date))
#define CONV_d_g(_val) ({ GB_DATE _v = (_val); (float)((float)_v.value.date + (float)_v.value.time / 86400000.0); })
#define CONV_d_f(_val) ({ GB_DATE _v = (_val); (double)((double)_v.value.date + (double)_v.value.time / 86400000.0); })
#define CONV_d_p(_val) (JIT.throw(E_CONV))
#define CONV_d_s(_val) CONV(_val, d, s, GB_T_STRING)
#define CONV_d_o(_val) (JIT.throw(E_CONV))

#define CONV_o_O(_val, _class) CONV(_val, o, o, CLASS(_class))

#define GET_NULL_o() ({ GB_OBJECT temp; temp.type = GB_T_OBJECT; temp.value = NULL; temp; })
#define GET_NULL_v() ({ GB_VARIANT temp; temp.type = GB_T_VARIANT; temp.value.type = GB_T_NULL; temp; })
#define GET_NULL_s() GET_CSTRING("", 0, 0)
#define GET_NULL_d() ({ GB_DATE temp; temp.type = GB_T_DATE; temp.value.date = 0; temp.value.time = 0; temp; })

#define PUSH_GOSUB(_label) ({ \
  GB_VALUE_GOSUB *_p = (GB_VALUE_GOSUB *)sp; \
  _p->type = GB_T_VOID; \
  _p->addr = &&_label; \
  _p->gp = gp; \
  gp = _p; \
  sp++; \
})

#define LEAVE_TRY() ERROR_leave(__err)

#define RETURN_LEAVE_TRY() ({ \
  void *_addr = &&__L0; \
  if (!gp) { LEAVE_TRY(); goto __RETURN; } \
  _addr = gp->addr; \
  sp = (GB_VALUE *)gp; \
  gp = gp->gp; \
  goto *_addr; \
})

#define RETURN() ({ \
  void *_addr; \
  if (!gp) goto __RETURN; \
  _addr = gp->addr; \
  sp = (GB_VALUE *)gp; \
  gp = gp->gp; \
  goto *_addr; \
})

#define RELEASE_GOSUB() ({ \
  while(gp) \
  { \
    gp = gp->gp; \
    SP--; \
  } \
})

#define CALL_SUBR(_pc, _func) ({ PC = &pc[_pc]; SP = sp; (*((EXEC_FUNC)_func))(); sp = SP; })
#define CALL_SUBR_CODE(_pc, _func, _code) ({ PC = &pc[_pc]; SP = sp; (*((EXEC_FUNC_CODE)_func))(_code); sp = SP; })
#define CALL_SUBR_UNKNOWN(_pc) ({ sp = JIT.call_unknown(&pc[_pc], sp); })

#define CALL_PUSH_ARRAY(_pc, _code) ({ PC = &pc[_pc]; SP = sp; (*(EXEC_FUNC)JIT.push_array)(_code); sp = SP; })
#define CALL_POP_ARRAY(_pc, _code) ({ PC = &pc[_pc]; SP = sp; (*(EXEC_FUNC)JIT.pop_array)(_code); sp = SP; sp++; })

#define PUSH_UNKNOWN(_pc) ({ PC = &pc[_pc]; SP = sp; JIT.push_unknown(); sp = SP; })
//#define PUSH_UNKNOWN(_pc) CALL_UNKNOWN(_pc)
#define POP_UNKNOWN(_pc) ({ PC = &pc[_pc]; SP = sp; JIT.pop_unknown(); sp = SP; })

#define PUSH_COMPLEX(_val) ({ PUSH_f(_val); SP = sp; JIT.push_complex(); POP_o(); })

#define GET_LAST() GET_OBJECT(*(JIT.event_last), GB_T_OBJECT)

#define GET_ME(_type) ({ \
  GB_OBJECT _temp; \
  if (OP) \
  { \
    GB_VALUE_OBJECT *_p = (GB_VALUE_OBJECT *)&_temp; \
    _p->type = (GB_TYPE)(_type); \
    _p->object = OP; \
  } \
  else \
  { \
    GB_VALUE_CLASS *_p = (GB_VALUE_CLASS *)&_temp; \
    _p->type = GB_T_CLASS; \
    _p->class = (GB_TYPE)(_type); \
  } \
  _temp; })

#define GET_SUPER(_type) ({ \
  GB_OBJECT _temp; \
  if (OP) \
  { \
    GB_VALUE_OBJECT *_p = (GB_VALUE_OBJECT *)&_temp; \
    _p->type = (GB_TYPE)(_type); \
    _p->object = OP; \
    _p->super = *JIT.exec_super; \
  } \
  else \
  { \
    GB_VALUE_CLASS *_p = (GB_VALUE_CLASS *)&_temp; \
    _p->type = GB_T_CLASS; \
    _p->class = (GB_TYPE)(_type); \
    _p->super = *JIT.exec_super; \
  } \
  *JIT.exec_super = sp; \
  _temp; })

#define LEAVE_SUPER() { \
  while (sp < *JIT.exec_super) \
    *JIT.exec_super = ((GB_VALUE_OBJECT *)*JIT.exec_super)->super; \
}

#define CALL_UNKNOWN(_pc) ({ sp = JIT.call_unknown(&pc[_pc], sp); })

#define ENUM_FIRST(_code, _plocal, _penum) ({ GB.Unref(&(_penum).value); (_penum).type = 0; JIT.enum_first(_code, (GB_VALUE *)&_plocal, (GB_VALUE*)&_penum); })

#define ENUM_NEXT(_code, _plocal, _penum, _label) ({ \
  SP = sp; \
  bool _t = JIT.enum_next(_code, (GB_VALUE *)&_plocal, (GB_VALUE *)&_penum); \
  sp = SP; \
  if (_t) goto _label; \
})

#define CALL_MATH(_func) ({ double _v = _func; if (!isfinite(_v)) JIT.throw(E_MATH); _v; })
#define CALL_MATH_UNSAFE(_func) (_func)

#define ERROR_current (*(ERROR_CONTEXT **)(JIT.error_current))
#define ERROR_handler (*(ERROR_HANDLER **)(JIT.error_handler))
#define ERROR_reset JIT.error_reset
  
#define QUIT(_code) (SP = sp, JIT.exec_quit(_code))

#define RAISE_EVENT(_event, _narg) ({ SP = sp; GB.Raise(OP, (_event), -(_narg)); sp = SP; })
#define RAISE_UNKNOWN_EVENT(_pc) ({ PC = &pc[_pc]; SP = sp; JIT.push_unknown_event(0); sp = SP; })

#define MATH_ABS(_val) ({ \
  __typeof(_val) _v = (_val); \
  (_v < 0) ? (- _v) : _v; \
})

#define MATH_SGN(_val) ({ \
  __typeof(_val) _v = (_val); \
  (_v < 0) ? -1 : ((_v > 0) ? 1 : 0); \
})

#define MATH_FIX_g(_val) ({ \
  __typeof(_val) _v = (_val); \
  (_v >= 0) ? floorf(_v) : -floorf(fabsf(_v)); \
})

#define MATH_FIX_f(_val) ({ \
  __typeof(_val) _v = (_val); \
  (_v >= 0) ? floor(_v) : -floor(fabs(_v)); \
})

