/***************************************************************************

  c_alure.c

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

#define __C_ALURE_C

#include "c_alure.h"

static char *_version = NULL;
static GB_ARRAY _stop = NULL;
static bool _register_stop = FALSE;

static void cb_play(void *userData, ALuint source)
{
	if (_register_stop)
	{
		if (!_stop)
			GB.Array.New(&_stop, GB_T_INTEGER, 0);

		*((ALuint *)GB.Array.Add(_stop)) = source;
	}
}

//---------------------------------------------------------------------------

#define THIS ((CALURESTREAM *)_object)

static int check_stream(CALURESTREAM *_object)
{
	return THIS->stream == NULL;
}

CALURESTREAM *create_stream(int num_buf)
{
	CALURESTREAM *stream = GB.New(GB.FindClass("AlureStream"), NULL, NULL);

	if (num_buf > 0)
		GB.Array.New(&stream->buffers, GB_T_INTEGER, num_buf);

	return stream;
}

bool destroy_stream(CALURESTREAM *_object)
{
	ALsizei nbuf;
	ALuint *buffers;
	bool err;

	if (THIS->buffers)
	{
		nbuf = GB.Array.Count(THIS->buffers);
		buffers = GB.Array.Get(THIS->buffers, 0);
	}
	else
	{
		nbuf = 0;
		buffers = NULL;
	}

	err = alureDestroyStream(THIS->stream, nbuf, buffers);

	GB.Unref(POINTER(&THIS->buffers));

	if (THIS->addr)
	{
		GB.ReleaseFile(THIS->addr, THIS->len);
		THIS->addr = NULL;
	}

	THIS->stream = NULL;

	return err;
}

BEGIN_METHOD_VOID(AlureStream_free)

	destroy_stream(THIS);

END_METHOD

//---------------------------------------------------------------------------

BEGIN_METHOD_VOID(Alure_exit)

	GB.FreeString(&_version);

END_METHOD

BEGIN_METHOD_VOID(Alure_GetVersion)

	ALuint major, minor;
	char buffer[32];

	if (!_version)
	{
		alureGetVersion(&major, &minor);
		sprintf(buffer, "%d.%d", major, minor);
		_version = GB.NewZeroString(buffer);
	}

	GB.ReturnString(_version);

END_METHOD

BEGIN_METHOD_VOID(Alure_GetErrorString)

	GB.ReturnNewZeroString(alureGetErrorString());

END_METHOD

BEGIN_METHOD(Alure_GetDeviceNames, GB_BOOLEAN all)

	GB_ARRAY array;
	const ALchar **devices;
	ALsizei count;
	int i;

	devices = alureGetDeviceNames(VARG(all), &count);

	GB.Array.New(&array, GB_T_STRING, count);
	for (i = 0; i < count; i++)
		*((char**)GB.Array.Get(array, i)) = GB.NewZeroString(devices[i]);

	alureFreeDeviceNames(devices);

	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(Alure_InitDevice, GB_STRING name; GB_OBJECT array)

	GB_ARRAY array = VARG(array);
	ALCint *attrs = NULL;
	int count;

	if (array)
	{
		count = GB.Array.Count(array);
		if (count)
		{
			attrs = alloca(sizeof(ALCint) * (count + 1));
			memcpy(attrs, GB.Array.Get(array, 0), count * sizeof(ALCint));
			attrs[count] = 0;
		}
	}

	GB.ReturnBoolean(alureInitDevice(MISSING(name) ? NULL : GB.ToZeroString(ARG(name)), attrs));

END_METHOD

BEGIN_METHOD_VOID(Alure_ShutdownDevice)

	GB.ReturnBoolean(alureShutdownDevice());

END_METHOD

BEGIN_METHOD(Alure_GetSampleFormat, GB_INTEGER channels; GB_INTEGER bits; GB_INTEGER fbits)

	GB.ReturnInteger(alureGetSampleFormat(VARG(channels), VARG(bits), VARG(fbits)));

END_METHOD

BEGIN_METHOD(Alure_Sleep, GB_FLOAT duration)

	GB.ReturnBoolean(alureSleep(VARG(duration)));

END_METHOD

BEGIN_METHOD(Alure_StreamSizeIsMicroSec, GB_BOOLEAN useus)

	GB.ReturnBoolean(alureStreamSizeIsMicroSec(VARG(useus)));

END_METHOD

BEGIN_METHOD(Alure_CreateBufferFromFile, GB_STRING path)

	char *addr;
	int len;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
		return;

	GB.ReturnInteger(alureCreateBufferFromMemory((const ALubyte *)addr, len));

	GB.ReleaseFile(addr, len);


END_METHOD

BEGIN_METHOD(Alure_BufferDataFromFile, GB_STRING path; GB_INTEGER buffer)

	char *addr;
	int len;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
		return;

	GB.ReturnBoolean(alureBufferDataFromMemory((const ALubyte *)addr, len, VARG(buffer)));

	GB.ReleaseFile(addr, len);


END_METHOD

BEGIN_METHOD(Alure_CreateStreamFromFile, GB_STRING path; GB_INTEGER chunck_length; GB_INTEGER num_buf)

	CALURESTREAM *stream;
	char *addr;
	int len;
	int num_buf = VARG(num_buf);
	ALuint *buffers;

	if (GB.LoadFile(STRING(path), LENGTH(path), &addr, &len))
		return;

	stream = create_stream(num_buf);
	stream->addr = addr;
	stream->len = len;

	if (stream->buffers)
		buffers = (ALuint *)GB.Array.Get(stream->buffers, 0);
	else
		buffers = NULL;

	stream->stream = alureCreateStreamFromStaticMemory((const ALubyte *)addr, len, VARG(chunck_length), num_buf, buffers);

	GB.ReturnObject(stream);

END_METHOD

#define GET_STREAM() \
	CALURESTREAM *stream = VARG(stream); \
	if (GB.CheckObject(stream)) \
		return;

BEGIN_METHOD(Alure_GetStreamBuffers, GB_OBJECT stream)

	GET_STREAM();
	GB.ReturnObject(stream->buffers);

END_METHOD

BEGIN_METHOD(Alure_GetStreamLength, GB_OBJECT stream)

	GET_STREAM();
	GB.ReturnLong(alureGetStreamLength(stream->stream));

END_METHOD

BEGIN_METHOD(Alure_GetStreamFrequency, GB_OBJECT stream)

	GET_STREAM();
	GB.ReturnInteger(alureGetStreamFrequency(stream->stream));

END_METHOD

BEGIN_METHOD(Alure_BufferDataFromStream, GB_OBJECT stream; GB_OBJECT buffers)

	GB_ARRAY buffers = VARG(buffers);

	GET_STREAM();

	if (GB.CheckObject(buffers))
		return;

	GB.ReturnInteger(alureBufferDataFromStream(stream->stream, GB.Array.Count(buffers), GB.Array.Get(buffers, 0)));

END_METHOD

BEGIN_METHOD(Alure_RewindStream, GB_OBJECT stream)

	GET_STREAM();
	GB.ReturnBoolean(alureRewindStream(stream->stream));

END_METHOD

BEGIN_METHOD(Alure_SetStreamOrder, GB_OBJECT stream; GB_INTEGER order)

	GET_STREAM();
	GB.ReturnBoolean(alureSetStreamOrder(stream->stream, VARG(order)));

END_METHOD

BEGIN_METHOD(Alure_SetStreamPatchset, GB_OBJECT stream; GB_STRING patchset)

	GET_STREAM();
	GB.ReturnBoolean(alureSetStreamPatchset(stream->stream, GB.ToZeroString(ARG(patchset))));

END_METHOD

BEGIN_METHOD(Alure_DestroyStream, GB_OBJECT stream)

	GET_STREAM();
	GB.ReturnBoolean(destroy_stream(stream));

END_METHOD

BEGIN_METHOD_VOID(Alure_Update)

	GB_ARRAY stop;

	_register_stop = TRUE;
	alureUpdate();
	_register_stop = FALSE;

	stop = _stop;
	_stop = NULL;
	GB.ReturnObject(stop);

END_METHOD

BEGIN_METHOD(Alure_UpdateInterval, GB_FLOAT interval)

	GB.ReturnBoolean(alureUpdateInterval(VARG(interval)));

END_METHOD

BEGIN_METHOD(Alure_PlaySourceStream, GB_INTEGER source; GB_OBJECT stream; GB_INTEGER num_bufs; GB_INTEGER loop_count)

	GET_STREAM();

	GB.ReturnBoolean(alurePlaySourceStream(VARG(source), stream->stream, VARG(num_bufs), VARG(loop_count), cb_play, NULL));

END_METHOD

BEGIN_METHOD(Alure_PlaySource, GB_INTEGER source)

	GB.ReturnBoolean(alurePlaySource(VARG(source), cb_play, NULL));

END_METHOD

BEGIN_METHOD(Alure_StopSource, GB_INTEGER source)

	GB.ReturnBoolean(alureStopSource(VARG(source), TRUE));

END_METHOD

BEGIN_METHOD(Alure_ResumeSource, GB_INTEGER source)

	GB.ReturnBoolean(alureResumeSource(VARG(source)));

END_METHOD

//---------------------------------------------------------------------------

GB_DESC AlureStreamDesc[] =
{
	GB_DECLARE("AlureStream", sizeof(CALURESTREAM)),
	GB_NOT_CREATABLE(),
	GB_HOOK_CHECK(check_stream),

	GB_METHOD("_free", NULL, AlureStream_free, NULL),

	GB_END_DECLARE
};

GB_DESC AlureDesc[] =
{
	GB_DECLARE_VIRTUAL("Alure"),

	GB_STATIC_METHOD("_exit", NULL, Alure_exit, NULL),
	
	GB_STATIC_METHOD("GetVersion", "s", Alure_GetVersion, NULL),
	GB_STATIC_METHOD("GetErrorString", "s", Alure_GetErrorString, NULL),
	GB_STATIC_METHOD("GetDeviceNames", "String[]", Alure_GetDeviceNames, "(All)b"),
 	GB_STATIC_METHOD("InitDevice", "b", Alure_InitDevice, "[(Name)s(Attributes)Integer[];]"),
 	GB_STATIC_METHOD("ShutdownDevice", "b", Alure_ShutdownDevice, NULL),
 	GB_STATIC_METHOD("GetSampleFormat", "i", Alure_GetSampleFormat, "(Channels)i(Bits)i(FloatBits)i"),
 	GB_STATIC_METHOD("Sleep", "b", Alure_Sleep, "(Duration)f"),
 	GB_STATIC_METHOD("StreamSizeIsMicroSec", "b", Alure_StreamSizeIsMicroSec, "(UseMicroSeconds)b"),
 	GB_STATIC_METHOD("CreateBufferFromFile", "i", Alure_CreateBufferFromFile, "(Path)s"),
// 	GB_STATIC_METHOD("CreateBufferFromMemory", "i", Alure_CreateBufferFromMemory, "(Data)p(Size)i"),
 	GB_STATIC_METHOD("BufferDataFromFile", "b", Alure_BufferDataFromFile, "(Path)s(Buffer)i"),
// 	GB_STATIC_METHOD("BufferDataFromMemory", "b", Alure_BufferDataFromMemory, "(Data)p(Size)i(Buffer)i"),
 	GB_STATIC_METHOD("CreateStreamFromFile", "AlureStream", Alure_CreateStreamFromFile, "(Path)s(ChunkLength)i(NumBuf)i"),
// 	GB_STATIC_METHOD("CreateStreamFromMemory", "AlureStream", Alure_CreateStreamFromFile, "(Data)p(Length)i(ChunkLength)i(NumBuf)i"),
// 	GB_STATIC_METHOD("CreateStreamFromStaticMemory", "AlureStream", Alure_CreateStreamFromFile, "(Data)p(Length)i(ChunkLength)i(NumBuf)i"),
 	GB_STATIC_METHOD("GetStreamBuffers", "Integer[]", Alure_GetStreamBuffers, "(Stream)AlureStream;"),
 	GB_STATIC_METHOD("GetStreamLength", "l", Alure_GetStreamLength, "(Stream)AlureStream;"),
 	GB_STATIC_METHOD("GetStreamFrequency", "i", Alure_GetStreamFrequency, "(Stream)AlureStream;"),
 	GB_STATIC_METHOD("BufferDataFromStream", "i", Alure_BufferDataFromStream, "(Stream)AlureStream;(Buffers)Integer[];"),
 	GB_STATIC_METHOD("RewindStream", "b", Alure_RewindStream, "(Stream)AlureStream;"),
 	GB_STATIC_METHOD("SetStreamOrder", "b", Alure_SetStreamOrder, "(Stream)AlureStream;(Order)i"),
 	GB_STATIC_METHOD("SetStreamPatchset", "b", Alure_SetStreamPatchset, "(Stream)AlureStream;(Patchset)s"),
 	GB_STATIC_METHOD("DestroyStream", "b", Alure_DestroyStream, "(Stream)AlureStream;"),
 	GB_STATIC_METHOD("Update", "Integer[]", Alure_Update, NULL),
 	GB_STATIC_METHOD("UpdateInterval", "b", Alure_UpdateInterval, "(Interval)f"),
 	GB_STATIC_METHOD("PlaySourceStream", "b", Alure_PlaySourceStream, "(Source)i(Stream)AlureStream;(NumBufs)i(LoopCount)i"),
 	GB_STATIC_METHOD("PlaySource", "b", Alure_PlaySource, "(Source)i"),
 	GB_STATIC_METHOD("StopSource", "b", Alure_StopSource, "(Source)i"),
 	GB_STATIC_METHOD("ResumeSource", "b", Alure_StopSource, "(Source)i"),

	GB_END_DECLARE
};
