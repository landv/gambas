/***************************************************************************

  tools.h

  (c) 2003-2004 Daniel Campos Fernández <dcamposf@gmail.com>

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

#include <termios.h>

#ifdef __sun__

  #include <sys/filio.h>
  #include <strings.h>

#endif

#ifdef MSG_PEEK
#ifdef MSG_NOSIGNAL

  #define USE_MSG_NOSIGNAL(_code) _code

#else

  #include <signal.h>
  #define MSG_NOSIGNAL 0
  #define USE_MSG_NOSIGNAL(_code) \
    { \
      void (*_oldsigpipe)(int) = signal(SIGPIPE, SIG_IGN); \
      _code ; \
      signal(SIGPIPE, _oldsigpipe); \
    }

#endif
#endif

void Alloc_CallBack_Pointers(long nobjs,long **objs,long **scks);

int search_by_integer(long *objlist,long nobj,long iData);

int CheckConnection(int Socket);
int IsHostPath(char *sCad, int lenCad, char **buf,int *port);

void correct_url(char **buf,char *protocol);
int ConvertBaudRate(int nBauds);
int ConvertStopBits(int nStop);
int ConvertDataBits(int nBits);
int ConvertParity(int parity);
void CloseSerialPort(int fd,struct termios *oldtio);
int OpenSerialPort(int *fd,int iflow,struct termios *oldtio, \
                   char *sName,int nBauds,int parity,int nBits,int nStop);
