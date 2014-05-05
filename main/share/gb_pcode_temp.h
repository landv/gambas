/***************************************************************************

  gb_pcode_temp.h

  (c) 2000-2013 Benoît Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gb_common.h"
#include "gb_error.h"

#ifdef PROJECT_COMP
#include "gb_limit.h"
#include "gbc_compile.h"
#endif

#include "gb_pcode.h"

/*#define DEBUG*/

short PCODE_dump(FILE *out, ushort addr, PCODE *code)
{
	static const char *op_comp[] = { "=", "<>", ">", "<=", "<", ">=", "==", "CASE" };
	static const char *op_arith[] = { "+", "-" , "*", "/", "NEG", "\\", "MOD", "POW", "AND", "OR", "XOR", "NOT", "&", "LIKE", "&/" };

	int j;
	unsigned short op;
	unsigned short digit;
	int value;
	#ifdef PROJECT_COMP
	int index;
	TABLE *table;
	bool trans;
	#endif
	int ncode;

	op = *code;

	switch (op & 0xFF00)
	{
		case C_PUSH_UNKNOWN: case C_POP_UNKNOWN:
		case C_PUSH_INTEGER:
		case C_JUMP: case C_JUMP_IF_TRUE: case C_JUMP_IF_FALSE: case C_GOSUB:
		case C_NEXT: case C_JUMP_NEXT:
		case C_TRY:

			ncode = 2;
			break;

		case C_PUSH_LONG:

			ncode = 3;
			break;
			
		case C_BYREF:
			ncode = 2 + (op & 0xFF);
			break;
			
		case C_ON:
			ncode = 1 + (op & 0xFF);
			break;
			
		case C_PUSH_EVENT:
			ncode = 1 + ((op & 0xFF) == 0xFF);
			break;

		default:

			if ((op & 0xFF00) == (C_PUSH_CONST | 0xF00))
				ncode = 2;
			else
				ncode = 1;
	}

	fprintf(out, "%04d : ", addr);

	for (j = 0; j < ncode; j++)
	{
		if (j > 2 && (j % 3) == 0)
			fprintf(out, "\n     : ");
		fprintf(out, " %04hX", code[j]);
	}

	for (j = 0; j < (3 - (ncode % 3)); j++)
		fprintf(out, "     ");

	fprintf(out, "  ");

	digit = (op >> 12);
	value = op & 0xFFF;
	if (value >= 0x800) value |= 0xFFFFF000;

	switch (digit)
	{
		#ifdef PROJECT_COMP

		case 0xF:
			fprintf(out, "PUSH QUICK %d", (short)value);
			break;

		case 0xE:
			if ((op & 0xF00) == 0xF00)
			{
				value = code[1];
			}

			fprintf(out, "PUSH CONST %d", value);

			switch(JOB->class->constant[value].type.t.id)
			{
				case T_STRING:
					table = JOB->class->string;
					trans = FALSE;
					break;

				case T_CSTRING:
					table = JOB->class->string;
					trans = TRUE;
					break;

				default:
					table = JOB->class->table;
					trans = FALSE;
					break;
			}

			if (trans)
				fprintf(out, " (\"%s\")", TABLE_get_symbol_name(table, JOB->class->constant[value].value));
			else
				fprintf(out, " \"%s\"", TABLE_get_symbol_name(table, JOB->class->constant[value].value));

			break;

		case 0xD: case 0xC:
			fprintf(out, "%s %s ", (digit == 0xD ? "POP" : "PUSH"), (value & 0x800) ? "STATIC" : "DYNAMIC");
			index = ((value & 0x800) ?  JOB->class->stat[value & 0x7FF].index : JOB->class->dyn[value & 0x7FF].index);
			fprintf(out, "%s", TABLE_get_symbol_name(JOB->class->table, index));
			break;

		case 0xB:
			if (value & 0x800)
			{
				fprintf(out, "PUSH FUNCTION ");
				index = JOB->class->function[value & 0x7FF].name;
			}
			else
			{
				fprintf(out, "PUSH CLASS ");
				index = JOB->class->class[value].index;
			}
			fprintf(out, "%s", TABLE_get_symbol_name(JOB->class->table, index));
			break;

		case 0xA:
			fprintf(out, "ADD QUICK %d", (short)value);
			break;

		#else

		case 0xF:
			fprintf(out, "PUSH QUICK %d", (short)value);
			break;

		case 0xE:
			fprintf(out, "PUSH CONST %d", (short)value);
			break;

		case 0xD:
			fprintf(out, "POP %s %d", (value & 0x800) ? "STATIC" : "DYNAMIC", value & 0x7FF);
			break;

		case 0xC:
			fprintf(out, "PUSH %s %d", (value & 0x800) ? "STATIC" : "DYNAMIC", value & 0x7FF);
			break;

		case 0xB:
			fprintf(out, "PUSH %s %d", (value & 0x800) ? "FUNCTION" : "CLASS", value & 0x7FF);
			break;

		case 0xA:
			fprintf(out, "ADD QUICK %d", (short)value);
			break;

		#endif

		default:

			digit = op & 0xFF00;
			value = op & 0xFF;
			if (value >= 0x80) value |= 0xFFFFFF00;

			if (digit >= C_PUSH_LOCAL && digit < C_QUIT && digit != C_BYREF)
				fprintf(out, "PUSH ");
			else if (digit >= C_POP_LOCAL && digit < C_BREAK)
				fprintf(out, "POP ");

			switch(digit)
			{
				case C_PUSH_LOCAL: case C_POP_LOCAL: case C_PUSH_PARAM: case C_POP_PARAM:
					if (value >= 0)
						fprintf(out, "LOCAL %d", (short)value);
					else
						fprintf(out, "PARAM %d", (short)value);
					break;

				case C_POP_CTRL:
					fprintf(out, "CTRL %d", (short)value);
					break;

				case C_POP_OPTIONAL:
					fprintf(out, "OPTIONAL %d", (short)value);
					break;

				case C_PUSH_UNKNOWN: case C_POP_UNKNOWN:
					value = code[1];
					#ifdef PROJECT_COMP
					fprintf(out, "UNKNOWN %s", TABLE_get_symbol_name(JOB->class->table, JOB->class->unknown[value]));
					#else
					fprintf(out, "UNKNOWN %d", (short)value);
					#endif

					break;

				/*case C_PUSH_SPECIAL:
					fprintf(out, "SPECIAL %d", (short)value);
					break;*/

				case C_PUSH_EXTERN:
					fprintf(out, "EXTERN %d", (short)value);
					break;

				case C_PUSH_EVENT:
					if ((unsigned char)value == 0xFF)
					{
						value = code[1];
						#ifdef PROJECT_COMP
						fprintf(out, "UNKNOWN EVENT %s", TABLE_get_symbol_name(JOB->class->table, JOB->class->unknown[value]));
						#else
						fprintf(out, "UNKNOWN EVENT %d", (short)value);
						#endif
					}
					else
						fprintf(out, "EVENT %d", (short)value);
					break;

				case C_PUSH_ARRAY: case C_POP_ARRAY:
					fprintf(out, "ARRAY (%d)", (short)value);
					break;

				case C_CALL: case C_CALL_QUICK: case C_CALL_SLOW:
					if (digit == C_CALL)
						fprintf(out, "CALL ");
					else if (digit == C_CALL_QUICK)
						fprintf(out, "CALL QUICK ");
					else
						fprintf(out, "CALL SLOW ");
					if (value & CODE_CALL_VARIANT)
						fprintf(out, "VARIANT ");
					//if (value & CODE_CALL_VOID)
					//  fprintf(out, "VOID ");

					fprintf(out, "(%d)", (short)value & 0x3F);
					break;
					
				case C_BYREF:
					fprintf(out, "BYREF (%d) ", (short)value & 0x3F);
					for (j = 1; j < ncode; j++)
						fprintf(out, "%04X", code[j]);
					break;

				case C_PUSH_INTEGER:
					value = code[1];
					fprintf(out, "PUSH SHORT %d", (short)value);
					break;

				case C_PUSH_LONG:
					// FIXME: endianness
					value = code[1] | (code[2] << 16); //*((int *)&code[1]);
					fprintf(out, "PUSH INTEGER %d", value);
					break;

				case C_PUSH_ME:
					fprintf(out, "PUSH %s", (value & 2) ? "SUPER": "ME");
					break;

				case C_PUSH_MISC:
					switch (value)
					{
						case CPM_NULL: fprintf(out, "PUSH NULL"); break;
						case CPM_VOID: fprintf(out, "PUSH VOID"); break;
						case CPM_FALSE: fprintf(out, "PUSH FALSE"); break;
						case CPM_TRUE: fprintf(out, "PUSH TRUE"); break;
						case CPM_LAST: fprintf(out, "PUSH LAST"); break;
						case CPM_STRING: fprintf(out, "PUSH NULL STRING"); break;
						case CPM_PINF: fprintf(out, "PUSH +INF"); break;
						case CPM_MINF: fprintf(out, "PUSH -INF"); break;
						case CPM_COMPLEX: fprintf(out, "PUSH COMPLEX"); break;
						case CPM_VARGS: fprintf(out, "PUSH VARGS"); break;
					}
					break;

				case C_JUMP: case C_JUMP_IF_TRUE: case C_JUMP_IF_FALSE:
					value = code[1];
					fprintf(out, "JUMP%s %04d",
						(digit == C_JUMP ? "" :
						digit == C_JUMP_IF_TRUE ? " IF TRUE" :
						" IF FALSE"),
						(short)(addr + value + 2));
					break;

				case C_JUMP_FIRST:

					fprintf(out, "JUMP FIRST LOCAL %d", (short)value);
					break;

				case C_JUMP_NEXT:

					fprintf(out, "JUMP NEXT ");

					value = code[1];
					fprintf(out, "%04d ", (short)(addr + value + 2));

					break;

				case C_GOSUB:
					value = code[1];
					fprintf(out, "GOSUB %04d", (short)(addr + value + 2));
					break;

				case C_ON:
					fprintf(out, "ON %d", (short)value);
					break;

				case C_FIRST:

					fprintf(out, "ENUM FIRST LOCAL %d", (short)value);
					break;

				case C_NEXT:

					fprintf(out, "ENUM NEXT ");
					if (value & 0xFF)
						fprintf(out, "DROP ");
					value = code[1];
					fprintf(out, "%04d ", (short)(addr + value + 2));
					break;

				case C_DROP:
					fprintf(out, "DROP (%d)", (short)value);
					break;

				case C_DUP:
					fprintf(out, "DUP");
					break;

				case C_NEW:
					fprintf(out, "NEW ");
					if (value & CODE_NEW_EVENT)
						fprintf(out, "EVENT ");
					if (value & CODE_NEW_ARRAY)
						fprintf(out, "ARRAY ");
					fprintf(out, "(%d)", (short)value & 0x3F);
					break;

				case C_BREAK:
					fprintf(out, "BREAK");
					break;

				case C_RETURN:
					fprintf(out, "RETURN (%d)", (short)value);
					break;

				case C_QUIT:
					switch(value)
					{
						case 0: fprintf(out, "QUIT"); break;
						case 1: fprintf(out, "STOP"); break;
						case 2: default: fprintf(out, "STOP EVENT"); break;
						case 3: fprintf(out, "QUIT (1)"); break;
					}
					break;

				case C_PUSH_CHAR:
					fprintf(out, "PUSH CHAR (%d)", (short)value);
					break;

				case C_TRY:
					value = code[1];
					fprintf(out, "TRY %04d", (short)(addr + value + 2));
					break;

				case C_END_TRY:
					fprintf(out, "END TRY");
					break;

				case C_CATCH:
					fprintf(out, "CATCH");
					break;

				default:
					digit = (digit >> 8);
					if (digit >= CODE_FIRST_SUBR)
					{
						#ifdef PROJECT_COMP
						fprintf(out, "SUBR %s ", SUBR_get_from_opcode(digit - CODE_FIRST_SUBR, (short)value & 0x3F)->name);
						#else
						fprintf(out, "SUBR #%d ", digit - CODE_FIRST_SUBR);
						#endif

						if (value & CODE_CALL_VARIANT)
							fprintf(out, "VARIANT ");
						//if (value & CODE_CALL_VOID)
						//  fprintf(out, "VOID ");

						fprintf(out, "(%d)", (short)value & 0x3F);
					}
					else if (digit >= 0x28 && digit <= 0x2F)
						fprintf(out, "%s (%d)", op_comp[digit - 0x28], (short)value);
					else if (digit >= 0x30 && digit <= 0x3E)
						fprintf(out, "%s (%d)", op_arith[digit - 0x30], (short)value);
					else if (digit == 0x3F)
						fprintf(out, "IS (%d)", (short)value);
					else
						fprintf(out, "ILLEGAL");
			}
	}

	fprintf(out, "\n");
	return ncode;
}


