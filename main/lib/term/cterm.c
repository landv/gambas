/***************************************************************************

	cterm.c

	(c) 2000-2017 Benoît Minisini <g4mba5@gmail.com>

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

#define __CTERM_C

#include "main.h"
#include <errno.h>
#include "cterm.h"

#define STREAM_FD (GB.Stream.Handle(GB.Stream.Get(_object)))

#define THIS_SETTINGS ((CTERMINALSETTINGS *)_object)
#define SETTINGS THIS_SETTINGS->settings

static bool check_error(int ret)
{
	if (ret)
	{
		GB.Error(strerror(errno));
		return TRUE;
	}
	else
		return FALSE;
}

/*static bool _convert(CTERMINAL *_object, GB_TYPE type, GB_VALUE *conv)
{
	if (!THIS && type >= GB_T_OBJECT && GB.Is(conv->_object.value, GB.FindClass("Stream")))
	{
		
	}
	
	return TRUE;
	
	switch(type)
	{
		case GB_T_FLOAT:
			conv->_object.value = COMPLEX_create(conv->_float.value, 0);
			return FALSE;

		case GB_T_SINGLE:
			conv->_object.value = COMPLEX_create(conv->_single.value, 0);
			return FALSE;

		case GB_T_LONG:
			conv->_object.value = COMPLEX_create((double)conv->_long.value, 0);
			return FALSE;
			
		case GB_T_INTEGER:
		case GB_T_SHORT:
		case GB_T_BYTE:
			conv->_object.value = COMPLEX_create(conv->_integer.value, 0);
			return FALSE;
			
		default:
			return TRUE;
	}
}*/

//---------------------------------------------------------------------------

static CTERMINALSETTINGS *TERMINALSETTINGS_copy(CTERMINALSETTINGS *_object)
{
	CTERMINALSETTINGS *settings;
	settings = (CTERMINALSETTINGS *)GB.New(GB.FindClass("TerminalSettings"), NULL, NULL);
	settings->settings = SETTINGS;
	return settings;
}

BEGIN_METHOD_VOID(TerminalSettings_MakeRaw)

	CTERMINALSETTINGS *settings = TERMINALSETTINGS_copy(THIS_SETTINGS);
	cfmakeraw(&SETTINGS);
	GB.ReturnObject(settings);

END_METHOD

BEGIN_METHOD_VOID(TerminalSettings_Copy)

	GB.ReturnObject(TERMINALSETTINGS_copy(THIS_SETTINGS));

END_METHOD

static void handle_settings(tcflag_t *pflag, void *_param, int cst)
{
	if (READ_PROPERTY)
		GB.ReturnBoolean(*pflag & cst);
	else
	{
		if (VPROP(GB_BOOLEAN))
			*pflag |= cst;
		else
			*pflag &= ~cst;
	}
}

static void handle_settings_int(tcflag_t *pflag, void *_param, int mask)
{
	if (READ_PROPERTY)
		GB.ReturnInteger(*pflag & mask);
	else
	{
		*pflag &= ~mask;
		*pflag |= VPROP(GB_INTEGER) & mask;
	}
}

static void handle_settings_char(cc_t pcc[], void *_param, int pos)
{
	if (READ_PROPERTY)
		GB.ReturnInteger(pcc[pos]);
	else
		pcc[pos] = (cc_t)VPROP(GB_INTEGER);
}

#define IMPLEMENT_TSP(_name, _field) \
BEGIN_PROPERTY(TerminalSettings_##_name) \
	handle_settings(&SETTINGS._field, _param, _name); \
END_PROPERTY

#define IMPLEMENT_TSP_I(_name, _field) \
BEGIN_PROPERTY(TerminalSettings_##_name) \
	handle_settings_int(&SETTINGS._field, _param, _name); \
END_PROPERTY

#define IMPLEMENT_TSP_C(_name) \
BEGIN_PROPERTY(TerminalSettings_##_name) \
	handle_settings_char(SETTINGS.c_cc, _param, _name); \
END_PROPERTY

IMPLEMENT_TSP(IGNBRK, c_iflag)
IMPLEMENT_TSP(BRKINT, c_iflag)
IMPLEMENT_TSP(IGNPAR, c_iflag)
IMPLEMENT_TSP(PARMRK, c_iflag)
IMPLEMENT_TSP(INPCK, c_iflag)
IMPLEMENT_TSP(ISTRIP, c_iflag)
IMPLEMENT_TSP(INLCR, c_iflag)
IMPLEMENT_TSP(IGNCR, c_iflag) 
IMPLEMENT_TSP(ICRNL, c_iflag) 
IMPLEMENT_TSP(IXON, c_iflag)  
IMPLEMENT_TSP(IXANY, c_iflag) 
IMPLEMENT_TSP(IXOFF, c_iflag) 
#ifdef OS_LINUX
IMPLEMENT_TSP(IUCLC, c_iflag)
IMPLEMENT_TSP(IUTF8, c_iflag)

IMPLEMENT_TSP(OLCUC, c_oflag)
IMPLEMENT_TSP(OFILL, c_oflag)
#endif
IMPLEMENT_TSP(OPOST, c_oflag)
IMPLEMENT_TSP(ONLCR, c_oflag)
IMPLEMENT_TSP(OCRNL, c_oflag)
IMPLEMENT_TSP(ONOCR, c_oflag)
IMPLEMENT_TSP(ONLRET, c_oflag)

#ifndef OS_BSD
IMPLEMENT_TSP_I(NLDLY, c_oflag)
IMPLEMENT_TSP_I(CRDLY, c_oflag)
IMPLEMENT_TSP_I(TABDLY, c_oflag)
IMPLEMENT_TSP_I(BSDLY, c_oflag)
IMPLEMENT_TSP_I(VTDLY, c_oflag)
IMPLEMENT_TSP_I(FFDLY, c_oflag)
#endif

IMPLEMENT_TSP_I(CSIZE, c_cflag)
IMPLEMENT_TSP(CSTOPB, c_cflag)
IMPLEMENT_TSP(CREAD, c_cflag)
IMPLEMENT_TSP(PARENB, c_cflag)
IMPLEMENT_TSP(PARODD, c_cflag)
IMPLEMENT_TSP(HUPCL, c_cflag)
IMPLEMENT_TSP(CLOCAL, c_cflag)
IMPLEMENT_TSP(CRTSCTS, c_cflag)

IMPLEMENT_TSP(ISIG, c_lflag)
IMPLEMENT_TSP(ICANON, c_lflag)
IMPLEMENT_TSP(ECHO, c_lflag)
IMPLEMENT_TSP(ECHOE, c_lflag)
IMPLEMENT_TSP(ECHOK, c_lflag)
IMPLEMENT_TSP(ECHONL, c_lflag)
IMPLEMENT_TSP(ECHOCTL, c_lflag)
IMPLEMENT_TSP(ECHOKE, c_lflag)
IMPLEMENT_TSP(FLUSHO, c_lflag)
IMPLEMENT_TSP(NOFLSH, c_lflag)
IMPLEMENT_TSP(TOSTOP, c_lflag)
IMPLEMENT_TSP(IEXTEN, c_lflag)

#ifdef OS_LINUX
#ifndef CMSPAR
// This is a MIPS fix
#define CMSPAR 010000000000
#endif

IMPLEMENT_TSP(CMSPAR, c_cflag)
IMPLEMENT_TSP(XCASE, c_lflag)
#endif

#ifndef OS_CYGWIN
IMPLEMENT_TSP(ECHOPRT, c_lflag)
IMPLEMENT_TSP(PENDIN, c_lflag)
#endif

IMPLEMENT_TSP_C(VDISCARD)
IMPLEMENT_TSP_C(VEOF)
IMPLEMENT_TSP_C(VEOL)
IMPLEMENT_TSP_C(VEOL2)
IMPLEMENT_TSP_C(VERASE)
IMPLEMENT_TSP_C(VINTR)
IMPLEMENT_TSP_C(VKILL)
IMPLEMENT_TSP_C(VLNEXT)
IMPLEMENT_TSP_C(VMIN)
IMPLEMENT_TSP_C(VQUIT)
IMPLEMENT_TSP_C(VREPRINT)
IMPLEMENT_TSP_C(VSTART)
IMPLEMENT_TSP_C(VSTOP)
IMPLEMENT_TSP_C(VSUSP)
IMPLEMENT_TSP_C(VTIME)
IMPLEMENT_TSP_C(VWERASE)

BEGIN_PROPERTY(TerminalSettings_InputSpeed)

	if (READ_PROPERTY)
		GB.ReturnInteger(cfgetispeed(&SETTINGS));
	else
		cfsetispeed(&SETTINGS, VPROP(GB_INTEGER));

END_PROPERTY

BEGIN_PROPERTY(TerminalSettings_OutputSpeed)

	if (READ_PROPERTY)
		GB.ReturnInteger(cfgetospeed(&SETTINGS));
	else
		cfsetospeed(&SETTINGS, VPROP(GB_INTEGER));

END_PROPERTY

//---------------------------------------------------------------------------

BEGIN_PROPERTY(StreamTerm_Name)
	
	GB.ReturnNewZeroString(ttyname(STREAM_FD));

END_PROPERTY

BEGIN_METHOD_VOID(StreamTerm_GetAttr)

	CTERMINALSETTINGS *settings;

	//fprintf(stderr, "fd = %d\n", STREAM_FD);
	
	settings = (CTERMINALSETTINGS *)GB.New(GB.FindClass("TerminalSettings"), NULL, NULL);
	if (check_error(tcgetattr(STREAM_FD, &settings->settings)))
	{
		GB.Unref(POINTER(&settings));
		return;
	}
		
	GB.ReturnObject(settings);

END_PROPERTY

BEGIN_METHOD(StreamTerm_SetAttr, GB_INTEGER mode; GB_OBJECT settings)

	CTERMINALSETTINGS *settings;
	struct termios check;

	settings = (CTERMINALSETTINGS *)VARG(settings);
	if (GB.CheckObject(settings))
		return;
		
	check_error(tcsetattr(STREAM_FD, VARG(mode), &settings->settings));
	
	tcgetattr(STREAM_FD, &check);
	if (check.c_iflag != settings->settings.c_iflag || check.c_oflag != settings->settings.c_oflag || check.c_lflag != settings->settings.c_lflag)
		GB.Error("Unable to set terminal attributes");

END_PROPERTY

BEGIN_METHOD(StreamTerm_Flush, GB_INTEGER mode)

	check_error(tcflush(STREAM_FD, VARG(mode)));

END_METHOD

BEGIN_METHOD_VOID(StreamTerm_Drain)

	check_error(tcdrain(STREAM_FD));

END_METHOD

BEGIN_METHOD(StreamTerm_Flow, GB_INTEGER mode)

	check_error(tcflow(STREAM_FD, VARG(mode)));

END_METHOD

BEGIN_METHOD_VOID(StreamTerm_SendBreak)

	check_error(tcsendbreak(STREAM_FD, 0));

END_METHOD

/*BEGIN_PROPERTY(Terminal_Control)

	GB.ReturnNewZeroString(ctermid(NULL));

END_PROPERTY*/

//---------------------------------------------------------------------------

#define __TC(_name) GB_CONSTANT(#_name, "i", _name)

GB_DESC TermDesc[] =
{
	GB_DECLARE_STATIC("Term"),
	
	__TC(TCSANOW), __TC(TCSADRAIN), __TC(TCSAFLUSH),
	
	__TC(TCIFLUSH),	__TC(TCOFLUSH),	__TC(TCIOFLUSH),
	
	__TC(TCIOFF), __TC(TCION), __TC(TCOOFF), __TC(TCOON),
#ifdef OS_LINUX
	__TC(NL0), __TC(NL1), __TC(CR0), __TC(CR1), __TC(CR2), __TC(CR3), __TC(TAB0), __TC(TAB1), __TC(TAB2), __TC(TAB3), __TC(XTABS), __TC(BS0), __TC(BS1), __TC(VT0), __TC(VT1), __TC(FF0), __TC(FF1),
#endif
	__TC(CS5), __TC(CS6), __TC(CS7), __TC(CS8),
	
	GB_CONSTANT("VDISABLE", "i", _POSIX_VDISABLE),
	
	__TC(B0), __TC(B50), __TC(B75), __TC(B110), __TC(B134), __TC(B150), __TC(B200), __TC(B300), __TC(B600), __TC(B1200), __TC(B1800), __TC(B2400), __TC(B4800), __TC(B9600), __TC(B19200), __TC(B38400), __TC(B57600), __TC(B115200), __TC(B230400),

#ifndef OS_BSD
 __TC(B460800), __TC(B500000), __TC(B576000), __TC(B921600), __TC(B1000000), __TC(B1152000), __TC(B1500000), __TC(B2000000), __TC(B2500000), __TC(B3000000),
#endif
#ifdef OS_LINUX
__TC(B3500000), __TC(B4000000),
#endif

	GB_END_DECLARE
};

#define __TSP(_name) GB_PROPERTY(#_name, "b", TerminalSettings_##_name)
#define __TSP_I(_name) GB_PROPERTY(#_name, "i", TerminalSettings_##_name)

GB_DESC TerminalSettingsDesc[] =
{
	GB_DECLARE("TerminalSettings", sizeof(CTERMINALSETTINGS)),
	GB_NOT_CREATABLE(),
	
	__TSP(IGNBRK),
	__TSP(BRKINT),
	__TSP(IGNPAR),
	__TSP(PARMRK),
	__TSP(INPCK),
	__TSP(ISTRIP),
	__TSP(INLCR),
	__TSP(IGNCR), 
	__TSP(ICRNL), 
	__TSP(IXON),  
	__TSP(IXANY), 
	__TSP(IXOFF), 
#ifdef OS_LINUX
	__TSP(IUCLC),
	__TSP(IUTF8),

	__TSP(OLCUC),
	__TSP(OFILL),
#endif
	__TSP(OPOST),
	__TSP(ONLCR),
	__TSP(OCRNL),
	__TSP(ONOCR),
	__TSP(ONLRET),

#ifndef OS_BSD
	__TSP_I(NLDLY),
	__TSP_I(CRDLY),
	__TSP_I(TABDLY),
	__TSP_I(BSDLY),
	__TSP_I(VTDLY),
	__TSP_I(FFDLY),
#endif

	__TSP_I(CSIZE),
	__TSP(CSTOPB),
	__TSP(CREAD),
	__TSP(PARENB),
	__TSP(PARODD),
	__TSP(HUPCL),
	__TSP(CLOCAL),
	__TSP(CRTSCTS),
	
	__TSP(ISIG),
	__TSP(ICANON),
	__TSP(ECHO),
	__TSP(ECHOE),
	__TSP(ECHOK),
	__TSP(ECHONL),
	__TSP(ECHOCTL),
	__TSP(ECHOKE),
	__TSP(FLUSHO),
	__TSP(NOFLSH),
	__TSP(TOSTOP),
	__TSP(IEXTEN),

#ifdef OS_LINUX
	__TSP(CMSPAR),
	__TSP(XCASE),
#endif
#ifndef OS_CYGWIN
	__TSP(ECHOPRT),
	__TSP(PENDIN),
#endif

	__TSP_I(VDISCARD),
	__TSP_I(VEOF),
	__TSP_I(VEOL),
	__TSP_I(VEOL2),
	__TSP_I(VERASE),
	__TSP_I(VINTR),
	__TSP_I(VKILL),
	__TSP_I(VLNEXT),
	__TSP_I(VMIN),
	__TSP_I(VQUIT),
	__TSP_I(VREPRINT),
	__TSP_I(VSTART),
	__TSP_I(VSTOP),
	__TSP_I(VSUSP),
	__TSP_I(VTIME),
	__TSP_I(VWERASE),
	
	GB_PROPERTY("InputSpeed", "i", TerminalSettings_InputSpeed),
	GB_PROPERTY("OutputSpeed", "i", TerminalSettings_OutputSpeed),
	
	GB_METHOD("MakeRaw", "TerminalSettings", TerminalSettings_MakeRaw, NULL),
	GB_METHOD("Copy", "TerminalSettings", TerminalSettings_Copy, NULL),
	
	GB_END_DECLARE
};

GB_DESC StreamTermDesc[] =
{
	GB_DECLARE_VIRTUAL(".Stream.Term"),

	//GB_STATIC_PROPERTY_READ("Control", "s", Terminal_Control),
	
	GB_METHOD("Flush", NULL, StreamTerm_Flush, "(Mode)i"),
	GB_METHOD("Drain", NULL, StreamTerm_Drain, NULL),
	GB_METHOD("Flow", NULL, StreamTerm_Flow, "(Mode)i"),
	GB_METHOD("SendBreak", NULL, StreamTerm_SendBreak, NULL),
	
	GB_METHOD("GetAttr", "TerminalSettings", StreamTerm_GetAttr, NULL),
	GB_METHOD("SetAttr", NULL, StreamTerm_SetAttr, "(Mode)i(Settings)TerminalSettings;"),
	
	//GB_INTERFACE("_convert", &_convert),
	
	GB_END_DECLARE
};

