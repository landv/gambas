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
    void *class;
    void *super;
    }
  GB_VALUE_CLASS;

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

#define PUSH_b(_val) (sp->type = GB_T_BOOLEAN, sp->_boolean.value = -(_val), sp++)
#define PUSH_c(_val) (sp->type = GB_T_BYTE, sp->_byte.value = (_val), sp++)
#define PUSH_h(_val) (sp->type = GB_T_SHORT, sp->_short.value = (_val), sp++)
#define PUSH_i(_val) (sp->type = GB_T_INTEGER, sp->_integer.value = (_val), sp++)
#define PUSH_l(_val) (sp->type = GB_T_LONG, sp->_long.value = (_val), sp++)
#define PUSH_g(_val) (sp->type = GB_T_SINGLE, sp->_single.value = (_val), sp++)
#define PUSH_f(_val) (sp->type = GB_T_FLOAT, sp->_float.value = (_val), sp++)
#define PUSH_p(_val) (sp->type = GB_T_POINTER, sp->_pointer.value = (_val), sp++)
#define PUSH_d(_val) (*((GB_DATE *)sp) = (_val), sp++)
#define PUSH_t(_val) (*((GB_STRING *)sp) = (_val), sp++)
#define PUSH_s(_val) ({ GB_STRING _v = (_val); GB.RefString(_v.value.addr); *((GB_STRING *)sp) = _v; sp++; })
#define PUSH_o(_val) ({ GB_OBJECT _v = (_val); GB.Ref(_v.value); *((GB_OBJECT *)sp) = _v; sp++; })
#define PUSH_C(_val) ({ GB_VALUE_CLASS _v; _v.type = GB_T_CLASS; _v.class = (_val); *((GB_VALUE_CLASS *)sp) = _v; sp++; })
#define PUSH_u(_val) (*sp = (_val), GB.BorrowValue(sp), sp++)

#define POP_b() (sp--, sp->_boolean.value)
#define POP_c() (sp--, sp->_byte.value)
#define POP_h() (sp--, sp->_short.value)
#define POP_i() (sp--, sp->_integer.value)
#define POP_l() (sp--, sp->_long.value)
#define POP_g() (sp--, sp->_single.value)
#define POP_f() (sp--, sp->_float.value)
#define POP_p() (sp--, sp->_pointer.value)
#define POP_d() (sp--, *((GB_DATE*)sp)
#define POP_s() (sp--, JIT.unborrow(sp), *((GB_STRING*)sp))
#define POP_o() (sp--, JIT.unborrow(sp), *((GB_OBJECT*)sp))
#define POP_t() (sp--, *((GB_STRING*)sp))
#define POP_u() (sp--, JIT.unborrow(sp), *sp)

#define BORROW_s(_val) ({ GB_STRING _v = (_val); GB.RefString(_v.value.addr); _v; })
#define BORROW_o(_val) ({ GB_OBJECT _v = (_val); GB.Ref(_v.value); _v; })

#define RELEASE_s(_val) ({ GB_STRING _v = (_val); GB.FreeString(&_v.value.addr); _v; })
#define RELEASE_o(_val) ({ GB_OBJECT _v = (_val); GB.Unref(&_v.value); _v; })

#define GET_CLASS(_index) (JIT.get_class_ref(_index))

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
  
#define CALL_SUBR(_code) (PC = &_code, SP = sp, (*(EXEC_FUNC)JIT.subr_table[_code >> 8])(), sp = SP)
#define CALL_SUBR_CODE(_code) (PC = &_code, SP = sp, (*(EXEC_FUNC_CODE)JIT.subr_table[_code >> 8])(_code), sp = SP)

#define CALL_NEW(_code) (PC = &_code, SP = sp, (*(EXEC_FUNC)JIT.new)(), sp = SP)
#define CALL_PUSH_ARRAY(_code) (PC = &_code, SP = sp, (*(EXEC_FUNC)JIT.push_array)(_code), sp = SP)
#define CALL_POP_ARRAY(_code) (PC = &_code, SP = sp, (*(EXEC_FUNC)JIT.pop_array)(_code), sp = SP, sp++)

#define GET_STATIC(_index)  JIT.get_static_addr(_index)

#define GET_STATIC_b(_index) (*(bool *)GET_STATIC(_index))
#define GET_STATIC_c(_index) (*(uchar *)GET_STATIC(_index))
#define GET_STATIC_h(_index) (*(short *)GET_STATIC(_index))
#define GET_STATIC_i(_index) (*(int *)GET_STATIC(_index))
#define GET_STATIC_l(_index) (*(int64_t *)GET_STATIC(_index))
#define GET_STATIC_g(_index) (*(float *)GET_STATIC(_index))
#define GET_STATIC_f(_index) (*(double *)GET_STATIC(_index))
#define GET_STATIC_p(_index) (*(intrptr_t *)GET_STATIC(_index))
#define GET_STATIC_s(_index) GET_STRING((*(char **)GET_STATIC(_index)), 0, GB.StringLength(temp.value.addr))

#define SET_STATIC_b(_index, _val) (GET_STATIC_b(_index) = (_val))
#define SET_STATIC_c(_index, _val) (GET_STATIC_c(_index) = (_val))
#define SET_STATIC_h(_index, _val) (GET_STATIC_h(_index) = (_val))
#define SET_STATIC_i(_index, _val) (GET_STATIC_i(_index) = (_val))
#define SET_STATIC_l(_index, _val) (GET_STATIC_l(_index) = (_val))
#define SET_STATIC_g(_index, _val) (GET_STATIC_g(_index) = (_val))
#define SET_STATIC_f(_index, _val) (GET_STATIC_f(_index) = (_val))
#define SET_STATIC_p(_index, _val) (GET_STATIC_p(_index) = (_val))
#define SET_STATIC_s(_index, _val) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.Store(GB_T_STRING, &temp, GET_STATIC(_index)); })

#define GET_DYNAMIC(_index)  JIT.get_dynamic_addr(_index)

#define GET_DYNAMIC_b(_index) *(bool *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_c(_index) *(uchar *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_h(_index) *(short *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_i(_index) *(int *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_l(_index) *(int64_t *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_g(_index) *(float *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_f(_index) *(double *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_p(_index) *(intptr_t *)GET_DYNAMIC(_index)
#define GET_DYNAMIC_s(_index) GET_STRING((*(char **)GET_DYNAMIC(_index)), 0, GB.StringLength(temp.value.addr))

#define SET_DYNAMIC_b(_index, _val) ((bool *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_c(_index, _val) ((uchar *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_h(_index, _val) ((short *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_i(_index, _val) ((int *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_l(_index, _val) ((int64_t *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_g(_index, _val) ((float *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_f(_index, _val) ((double *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_p(_index, _val) ((intptr_t *)GET_DYNAMIC(_index) = (_val))
#define SET_DYNAMIC_s(_index, _val) ({ GB_VALUE temp = (GB_VALUE)(_val); GB.Store(GB_T_STRING, &temp, GET_DYNAMIC(_index)); })

