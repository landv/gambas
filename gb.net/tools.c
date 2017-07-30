/***************************************************************************

  tools.c

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

#include "tools.h"
#include "main.h"
#include "gambas.h"
#include <sys/poll.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __CYGWIN__
#define FIONREAD TIOCINQ
#endif

void correct_url(char **buf,char *protocol)
{
	char *buftmp;
	int len;
	int myloop,myloop2;
	int pos=-1;
	int myok=1;

	len=strlen(*buf);
	for (myloop=0;myloop<len;myloop++)
	{
		if ( (*buf)[myloop]==':' )
		{
			if (myloop==(len-1))
			{
				pos=myloop;
				break;
			}
			else
			{
				if ( (*buf)[myloop+1]=='/' )
				{
					pos=myloop;
					break;
				}
				else
				{
					for (myloop2=myloop+1;myloop2<len;myloop2++)
					{
						if ( (*buf)[myloop2]=='/' ) break;
						if ( ((*buf)[myloop2]<0x30) || ((*buf)[myloop2]>0x39) )
						{
							myok=0;
							break;
						}
					}
					if (!myok) pos=myloop;
					break;
				}
			}
		}
	}

	myok=0;

	if (pos==-1)
	{

		GB.Alloc(POINTER(&buftmp),len+1);
		strcpy(buftmp,*buf);
		GB.Free(POINTER(buf));
		GB.Alloc(POINTER(buf),len+strlen(protocol)+1);
		strcpy(*buf,protocol);
		if (strlen(buftmp)>=2)
		{
			if ( buftmp[0]=='/') myok++;
			if ( buftmp[1]=='/') myok++;
		}
		strcat(*buf,buftmp+myok);
		GB.Free(POINTER(&buftmp));
	}
	else
	{
		GB.Alloc(POINTER(&buftmp),(len-pos)+1);
		strcpy(buftmp,*buf+pos+1);
		GB.Free(POINTER(buf));
		GB.Alloc(POINTER(buf),strlen(buftmp)+strlen(protocol)+1);
		strcpy(*buf,protocol);
		if (strlen(buftmp)>=2)
		{
			if ( buftmp[0]=='/') myok++;
			if ( buftmp[1]=='/') myok++;
		}
		strcat(*buf,buftmp+myok);
		GB.Free(POINTER(&buftmp));
	}
}

void Alloc_CallBack_Pointers(long nobjs,long **objs,long **scks)
{
	if (!nobjs)
	{
		if (*objs)
		{
			GB.Free((void**)objs);
			GB.Free((void**)scks);
			*objs=NULL;
		}
		return;
	}
	if (*objs)
	{
		GB.Realloc((void**)objs,nobjs*sizeof(long));
		GB.Realloc((void**)scks,nobjs*sizeof(long));
	}
	else
	{
		GB.Alloc((void**)objs,sizeof(long));
		GB.Alloc((void**)scks,sizeof(long));
	}
}

int CheckConnection(int Socket)
{
	struct pollfd mypoll;
	int numpoll;
	int retval=6;

	mypoll.fd=Socket;
	mypoll.events=POLLERR;
	mypoll.revents=0;
	numpoll=poll(&mypoll,1,0);
	if (numpoll>=0)
	{
		if (!numpoll)
		{
			mypoll.fd=Socket;
			mypoll.events=POLLIN | POLLOUT;
			mypoll.revents=0;
			numpoll=poll(&mypoll,1,0);
			if (numpoll<0)
			{
				retval=0;
			}
			else
			{
				if (numpoll>0) retval=7;
			}
		}
		else
		{
			retval=0;
		}

	}
	else
	{
		retval=0;

	}
	return retval;
}

/* free "buf" after using it! */
int IsHostPath(char *sCad, int lenCad, char **buf,int *port)
{
	/*******************
	 0 --> Error
	 1 --> TCP
	 2 -> Unix
	 *******************/
	int npos=0;
	int npoint=0;
	int myloop;
	int bufport=0;
	*port=0;
	*buf=NULL;
	if ( sCad[0] == '/' )
	{
		return 2;
	}
	for (myloop=0;myloop<lenCad;myloop++)
	{
		if (sCad[myloop]==':')
		{
			npoint++;
			npos=myloop;
		}
	}
	if (npoint != 1) return 0;

	for (myloop=npos+1;myloop<lenCad;myloop++)
	{
		if ( (sCad[myloop]<'0') || (sCad[myloop]>'9') ) return 0;
		bufport*=10;
		bufport+= (sCad[myloop]-48);
		if ( (bufport) >65535) return 0;
	}
	*port=bufport;
	if (npos>0)
	{
		GB.Alloc((void**)buf,npos);
		*buf[0]=0;
		sCad[npos]=0;
		strcpy(*buf,sCad);
		sCad[npos]=':';
	}
	return 1;

}
/********************************************************
 Watching stuff
 ********************************************************/
int search_by_integer(long *objlist,long nobj,long iData)
{
	int myloop;
	int position=0;
	for (myloop=0;myloop<nobj;myloop++)
	{
			if ( objlist[myloop] == iData )
			{
				return position;
				break;
			}
			else
				position++;
	}
	return -1;
}
/********************************************************
 Serial Port stuff
 ********************************************************/
int ConvertDataBits(int nBits)
{
	switch(nBits)
	{
		case 8: return CS8;
		case 7: return CS7;
		case 6: return CS6;
		case 5: return CS5;
		default: return -1;
	}
}

int ConvertParity(int parity)
{
  switch(parity)
  {
    case 0: return 0;
    case 1: return PARENB;
    case 2: return PARENB | PARODD;
    default: return -1;
  }
}

int ConvertStopBits(int nStop)
{
	switch(nStop)
	{
		case 1: return 0;
		case 2: return CSTOPB;
		default: return -1;
	}
}

int ConvertBaudRate(int nBauds)
{
	switch(nBauds)
	{
		case 0:	return  B0; /*Hang UP*/
		case 50:return  B50;
		case 75:return  B75;
		case 110:return  B110;
		case 134:return  B134;
		case 150:return  B150;
		case 200:return  B200;
		case 300:return  B300;
		case 600:return  B600;
		case 1200:return  B1200;
		case 1800:return  B1800;
		case 2400:return  B2400;
		case 4800:return  B4800;
		case 9600:return  B9600;
		case 19200:return  B19200;
		case 38400:return  B38400;
		case 57600:return    B57600;
		case 115200:return   B115200;
		case 230400:return   B230400;
    #ifdef B460800
		case 460800:return   B460800;
    #endif
    #ifdef B500000
		case 500000:return   B500000;
    #endif
    #ifdef B576000
		case 576000:return   B576000;
    #endif
    #ifdef B921600
		case 921600:return   B921600;
    #endif
    #ifdef B1000000
		case 1000000:return  B1000000;
    #endif
    #ifdef B1152000
		case 1152000:return  B1152000;
    #endif
    #ifdef B1500000
		case 1500000:return  B1500000;
    #endif
    #ifdef B2000000
		case 2000000:return  B2000000;
    #endif
    #ifdef B2500000
		case 2500000:return  B2500000;
    #endif
    #ifdef B3000000
		case 3000000:return  B3000000;
    #endif
    #ifdef B3500000
		case 3500000:return  B3500000;
    #endif
    #ifdef B4000000
		case 4000000:return  B4000000;
    #endif
		default: return -1;
	}
}

void CloseSerialPort(int fd,struct termios *oldtio)
{
	if (oldtio) tcsetattr(fd,TCSANOW,oldtio);
	close(fd);
}

int OpenSerialPort(int *fd,int iflow,struct termios *oldtio,char *sName,int nBauds,int parity,int nBits,int nStop)
{
	int Of_Baud;
	int Of_Par;
	int Of_Bits;
	int Of_Stop;
	int HardFlow=0;
	int SoftFlow=0;
	struct termios newtio;

  switch(iflow)
	{
		case 1: HardFlow=CRTSCTS; break;
		case 2: SoftFlow=IXON | IXOFF | IXANY; break;
		case 3: HardFlow=CRTSCTS; SoftFlow=IXON | IXOFF | IXANY; break;
	}

	if ( (Of_Baud = ConvertBaudRate(nBauds)) == -1 ) return 1;
	if ( (Of_Par = ConvertParity(parity)) == -1 ) return 2;
	if ( (Of_Bits = ConvertDataBits(nBits)) == -1 ) return 3;
	if ( (Of_Stop = ConvertStopBits(nStop)) == -1 ) return 4;

  *fd=open(sName,O_RDWR | O_NOCTTY | O_NDELAY );
	if ( (*fd) < 0 ) return 5;
	if (oldtio)
	{
		if ( tcgetattr (*fd,oldtio) == -1)
		{
			close(*fd);
			return 6;
		}
	}

	if (tcgetattr (*fd,&newtio) == -1 )
	{
		close(*fd);
		return 6;
	}
	// cleaning default options
	newtio.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD | CRTSCTS);
	newtio.c_iflag &= ~( INPCK | ISTRIP | IGNPAR | IXON | IXOFF | IXANY | ICRNL | INLCR );
	newtio.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );
	// setting options
	newtio.c_cflag=  Of_Bits | Of_Stop | Of_Par | CLOCAL | CREAD | HUPCL | HardFlow;

	if ( Of_Par & PARENB )
		newtio.c_iflag |= INPCK; // Why stripping the eight bit when parity is enabled?? | ISTRIP;
	else
		newtio.c_iflag |= IGNPAR;

	newtio.c_iflag |= SoftFlow;
	
	newtio.c_oflag=0;
	
	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=1;
	newtio.c_cc[VSTART]=17; //DC1;
	newtio.c_cc[VSTOP]=19;  //DC3;
	
	cfsetispeed(&newtio,Of_Baud);
	cfsetospeed(&newtio,Of_Baud);
	tcflush(*fd,TCIFLUSH);
	if ( tcsetattr(*fd,TCSANOW,&newtio) == -1)
	{
		close(*fd);
		return 7;
	}

	return 0;
}


