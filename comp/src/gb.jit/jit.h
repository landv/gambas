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

#define SP (*(JIT.sp))
#define PC (*(JIT.pc))

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

#define PUSH_b(_val) (SP->type = GB_T_BOOLEAN, SP->_boolean.value = -(_val), SP++)
#define PUSH_c(_val) (SP->type = GB_T_BYTE, SP->_byte.value = (_val), SP++)
#define PUSH_h(_val) (SP->type = GB_T_SHORT, SP->_short.value = (_val), SP++)
#define PUSH_i(_val) (SP->type = GB_T_INTEGER, SP->_integer.value = (_val), SP++)
#define PUSH_l(_val) (SP->type = GB_T_LONG, SP->_long.value = (_val), SP++)
#define PUSH_g(_val) (SP->type = GB_T_SINGLE, SP->_single.value = (_val), SP++)
#define PUSH_f(_val) (SP->type = GB_T_FLOAT, SP->_float.value = (_val), SP++)
#define PUSH_p(_val) (SP->type = GB_T_POINTER, SP->_pointer.value = (_val), SP++)
#define PUSH_d(_val) (*SP++ = (GB_VALUE)(_val))
#define PUSH_s(_val) (*SP++ = (GB_VALUE)(_val))
#define PUSH_t(_val) (*SP++ = (GB_VALUE)(_val))

#define POP_b() (SP--, SP->_boolean.value)
#define POP_c() (SP--, SP->_byte.value)
#define POP_h() (SP--, SP->_short.value)
#define POP_i() (SP--, SP->_integer.value)
#define POP_l() (SP--, SP->_long.value)
#define POP_g() (SP--, SP->_single.value)
#define POP_f() (SP--, SP->_float.value)
#define POP_p() (SP--, SP->_pointer.value)
#define POP_d() (SP--, *SP)
#define POP_s() (SP--, *SP)

#define BORROW_s(_val) ({ GB_STRING _v = (_val); GB.RefString(_v.value.addr); _v; })

#define RELEASE_s(_val) ({ GB_STRING _v = (_val); GB.FreeString(&_v.value.addr); _v; })

#define CONSTANT_l(_index) ({ \
  JIT_CONSTANT *_cc = JIT.get_constant(_index); \
  _cc->_long.value; \
})

#define CONSTANT_g(_index) ({ \
  JIT_CONSTANT *_cc = JIT.get_constant(_index); \
  _cc->_single.value; \
})

#define CONSTANT_f(_index) ({ \
  JIT_CONSTANT *_cc = JIT.get_constant(_index); \
  _cc->_float.value; \
})

#define CONSTANT_s(_index) ({ \
  JIT_CONSTANT *_cc = JIT.get_constant(_index); \
  GET_CSTRING(_cc->_string.addr, 0, _cc->_string.len); \
})

#define CONSTANT_t(_index) ({ \
  JIT_CONSTANT *_cc = JIT.get_constant(_index); \
  GET_CSTRING(GB.Translate(_cc->_string.addr), 0, strlen(temp.value.addr)); \
})

#define GET_CHAR(_char) GET_CSTRING(&JIT.char_table[(_char) * 2], 0, 1)
  
#define CALL_SUBR(_code) (PC = &_code, (*(EXEC_FUNC)JIT.subr_table[_code >> 8])())
#define CALL_SUBR_CODE(_code) (PC = &_code, (*(EXEC_FUNC_CODE)JIT.subr_table[_code >> 8])(_code))

#define GET_STATIC(_index)  JIT.get_static_addr(_index)

#define GET_STATIC_i(_index) (*(int *)GET_STATIC(_index))
#define SET_STATIC_i(_index, _val) (GET_STATIC_i(_index) = (_val))

#define GET_STATIC_s(_index) GET_STRING((*(char **)GET_STATIC(_index)), 0, GB.StringLength(temp.value.addr))
#define SET_STATIC_s(_index, _val) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.Store(GB_T_STRING, &temp, GET_STATIC(_index)); })

#define GET_DYNAMIC(_index)  JIT.get_dynamic_addr(_index)

#define GET_DYNAMIC_i(_index) *(int *)GET_DYNAMIC(_index)
#define SET_DYNAMIC_i(_index, _val) ((int *)GET_DYNAMIC(_index) = (_val))

