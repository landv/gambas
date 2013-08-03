/***************************************************************************

  c_alc.c

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

#define __C_ALC_C

#include "c_alc.h"

//---------------------------------------------------------------------------

#define THIS ((CALCCONTEXT *)_object)

static CALCCONTEXT *_current_context = NULL;

static int check_context(void *_object)
{
	return THIS->context == NULL;
}

static void init_context(CALCCONTEXT *_object, CALCDEVICE *device, GB_ARRAY array)
{
	ALCint *attrs = NULL;
	int count;

	if (GB.CheckObject(device))
		return;

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

	THIS->context = alcCreateContext(device->device, attrs);
	THIS->device = device;
	GB.Ref(device);
}

static void set_current_context(CALCCONTEXT *_object)
{
	if (_current_context == THIS)
		return;

	GB.Unref(POINTER(&_current_context));

	_current_context = THIS;

	if (THIS)
		GB.Ref(THIS);
}

static void destroy_context(CALCCONTEXT *_object)
{
	if (_current_context == THIS)
		set_current_context(NULL);

	if (!THIS->context)
		return;

	alcDestroyContext(THIS->context);

	THIS->context = NULL;

	GB.Unref(POINTER(&THIS->device));
}

BEGIN_METHOD_VOID(AlcContext_exit)

	GB.Unref(POINTER(&_current_context));

END_METHOD

BEGIN_METHOD(AlcContext_new, GB_OBJECT device; GB_OBJECT attrs)

	init_context(THIS, VARG(device), VARGOPT(attrs, NULL));

END_METHOD

BEGIN_METHOD_VOID(AlcContext_free)

	destroy_context(THIS);

END_METHOD

BEGIN_METHOD_VOID(AlcContext_MakeCurrent)

	ALCboolean err = alcMakeContextCurrent(THIS->context);
	if (err == ALC_FALSE)
		set_current_context(THIS);
	GB.ReturnBoolean(err);

END_METHOD

//---------------------------------------------------------------------------

#undef THIS
#define THIS ((CALCDEVICE *)_object)

static int check_device(void *_object)
{
	return THIS->device == NULL;
}

static CALCDEVICE *create_device(ALCdevice *device)
{
	CALCDEVICE *_object;

	if (!device)
		return NULL;

	_object = GB.New(GB.FindClass("AlcDevice"), NULL, NULL);
	THIS->device = device;
	return THIS;
}

static bool close_device(CALCDEVICE *_object)
{
	bool err = FALSE;

	if (THIS->device)
	{
		if (THIS->capture)
			err = alcCaptureCloseDevice(THIS->device);
		else
			err = alcCloseDevice(THIS->device);

		THIS->device = NULL;
	}

	return err;
}

BEGIN_METHOD_VOID(AlcDevice_free)

	close_device(THIS);

END_METHOD

//---------------------------------------------------------------------------

BEGIN_METHOD(ALC_CreateContext, GB_OBJECT device; GB_OBJECT attrs)

	CALCCONTEXT *context;

	context = GB.Create(GB.FindClass("AlcContext"), NULL, NULL);
	init_context(context, VARG(device), VARGOPT(attrs, NULL));
	if (context->context)
		GB.ReturnObject(context);
	else
	{
		GB.Unref(POINTER(&context));
		GB.ReturnNull();
	}

END_METHOD

#define GET_CONTEXT() \
	CALCCONTEXT *context = VARG(context); \
	if (GB.CheckObject(context)) \
		return;

BEGIN_METHOD(ALC_MakeContextCurrent, GB_OBJECT context)

	CALCCONTEXT *context = VARG(context);
	ALCboolean err = alcMakeContextCurrent(context ? context->context : NULL);
	if (err == ALC_FALSE)
		set_current_context(context);
	GB.ReturnBoolean(err);

END_METHOD

BEGIN_METHOD(ALC_ProcessContext, GB_OBJECT context)

	GET_CONTEXT();
	alcProcessContext(context->context);

END_METHOD

BEGIN_METHOD(ALC_SuspendContext, GB_OBJECT context)

	GET_CONTEXT();
	alcSuspendContext(context->context);

END_METHOD

BEGIN_METHOD(ALC_DestroyContext, GB_OBJECT context)

	GET_CONTEXT();
	destroy_context(context);

END_METHOD

BEGIN_METHOD_VOID(ALC_GetCurrentContext)

	GB.ReturnObject(_current_context);

END_METHOD

BEGIN_METHOD(ALC_GetContextsDevice, GB_OBJECT context)

	GET_CONTEXT();
	GB.ReturnObject(THIS->device);

END_METHOD

BEGIN_METHOD(ALC_OpenDevice, GB_STRING name)

	GB.ReturnObject(create_device(alcOpenDevice(GB.ToZeroString(ARG(name)))));

END_METHOD

#define GET_DEVICE() \
	CALCDEVICE *device = VARG(device); \
	if (GB.CheckObject(device)) \
		return;

BEGIN_METHOD(ALC_CloseDevice, GB_OBJECT device)

	GET_DEVICE();
	GB.ReturnBoolean(close_device(device));

END_METHOD

BEGIN_METHOD(ALC_GetError, GB_OBJECT device)

	GET_DEVICE();
	GB.ReturnInteger(alcGetError(device->device));

END_METHOD

BEGIN_METHOD(ALC_IsExtensionPresent, GB_OBJECT device; GB_STRING ext)

	GET_DEVICE();
	GB.ReturnBoolean(alcIsExtensionPresent(device->device, GB.ToZeroString(ARG(ext))));

END_METHOD

BEGIN_METHOD(ALC_GetString, GB_OBJECT device; GB_INTEGER param)

	CALCDEVICE *device = VARG(device);
	ALCenum param = VARG(param);

	if (device && GB.CheckObject(device))
		return;

	if (!device && (param == ALC_DEVICE_SPECIFIER || param == ALC_CAPTURE_DEVICE_SPECIFIER))
	{
		GB.Error("This query actually returns a string array. Use ALC_GetStringv instead");
		return;
	}

	GB.ReturnNewZeroString(alcGetString(device ? device->device : NULL, param));

END_METHOD

BEGIN_METHOD(ALC_GetStringv, GB_OBJECT device; GB_INTEGER param)

	CALCDEVICE *device = VARG(device);
	ALCenum param = VARG(param);
	const char *result, *p;
	int len;
	GB_ARRAY array;

	if (!(!device && (param == ALC_DEVICE_SPECIFIER || param == ALC_CAPTURE_DEVICE_SPECIFIER)))
	{
		GB.ReturnNull();
		return;
	}

	result = alcGetString(NULL, param);
	if (!result)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&array, GB_T_STRING, 0);

	for(;;)
	{
		p = index(result, 0);
		if (!p)
			break;
		len = p - result;
		if (len <= 0)
			break;

		*((char **)GB.Array.Add(array)) = GB.NewString(result, len);
		result = p + 1;
	}

	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(ALC_GetIntegerv, GB_OBJECT device; GB_INTEGER param; GB_INTEGER size)

	GB_ARRAY array;
	int size = VARG(size);

	GET_DEVICE();

	if (size <= 0 || size >= 65536)
	{
		GB.ReturnNull();
		return;
	}

	GB.Array.New(&array, GB_T_INTEGER, size);
	alcGetIntegerv(device->device, VARG(param), size, GB.Array.Get(array, 0));
	GB.ReturnObject(array);

END_METHOD

BEGIN_METHOD(ALC_CaptureOpenDevice, GB_STRING name; GB_INTEGER freq; GB_INTEGER format; GB_INTEGER buffer_size)

	CALCDEVICE *device = create_device(alcCaptureOpenDevice(GB.ToZeroString(ARG(name)), VARG(freq), VARG(format), VARG(buffer_size)));

	if (device)
	{
		device->capture = TRUE;

		switch(VARG(format))
		{
			case AL_FORMAT_MONO8: device->sampleSize = 1; break;
			case AL_FORMAT_MONO16: case AL_FORMAT_STEREO8: device->sampleSize = 2; break;
			case AL_FORMAT_STEREO16: device->sampleSize = 4; break;
			default: device->sampleSize = 0;
		}
	}

	GB.ReturnObject(device);

END_METHOD

BEGIN_METHOD(ALC_CaptureStart, GB_OBJECT device)

	GET_DEVICE();
	alcCaptureStart(device->device);

END_METHOD

BEGIN_METHOD(ALC_CaptureStop, GB_OBJECT device)

	GET_DEVICE();
	alcCaptureStop(device->device);

END_METHOD

BEGIN_METHOD(ALC_CaptureSamples, GB_OBJECT device; GB_INTEGER samples; GB_POINTER buffer)

	GB_ARRAY array = NULL;
	int samples = VARG(samples);
	void *buffer;

	GET_DEVICE();

	if (samples <= 0)
	{
		GB.ReturnNull();
		return;
	}

	if (MISSING(buffer))
	{
		if (device->sampleSize == 0)
		{
			GB.Error("Unknown sample format");
			return;
		}
		GB.Array.New(&array, device->sampleSize == 1 ? GB_T_BYTE : device->sampleSize == 2 ? GB_T_SHORT : GB_T_INTEGER, samples);
		buffer = GB.Array.Get(array, 0);
	}
	else
		buffer = (void *)VARG(buffer);

	alcCaptureSamples(device->device, buffer, samples);

	GB.ReturnObject(array);

END_METHOD

//---------------------------------------------------------------------------

GB_DESC AlcDeviceDesc[] =
{
	GB_DECLARE("AlcDevice", sizeof(CALCDEVICE)),
	GB_NOT_CREATABLE(),
	GB_HOOK_CHECK(check_device),

	GB_METHOD("_free", NULL, AlcDevice_free, NULL),

	GB_END_DECLARE
};

GB_DESC AlcContextDesc[] =
{
	GB_DECLARE("AlcContext", sizeof(CALCCONTEXT)),
	GB_HOOK_CHECK(check_context),

	GB_STATIC_METHOD("_exit", NULL, AlcContext_exit, NULL),

	GB_METHOD("_new", NULL, AlcContext_new, "(Device)AlcDevice;[(Attributes)Integer[];]"),
	GB_METHOD("MakeCurrent", "b", AlcContext_MakeCurrent, NULL),

	GB_END_DECLARE
};

GB_DESC AlcDesc[] =
{
	GB_DECLARE_VIRTUAL("Alc"),
	
	GB_CONSTANT("FALSE", "i",                             ALC_FALSE),
	GB_CONSTANT("TRUE", "i",                              ALC_TRUE),
	GB_CONSTANT("FREQUENCY", "i",                         ALC_FREQUENCY),
	GB_CONSTANT("REFRESH", "i",                           ALC_REFRESH),
	GB_CONSTANT("SYNC", "i",                              ALC_SYNC),
	GB_CONSTANT("MONO_SOURCES", "i",                      ALC_MONO_SOURCES),
	GB_CONSTANT("STEREO_SOURCES", "i",                    ALC_STEREO_SOURCES),
	GB_CONSTANT("NO_ERROR", "i",                          ALC_NO_ERROR),
	GB_CONSTANT("INVALID_DEVICE", "i",                    ALC_INVALID_DEVICE),
	GB_CONSTANT("INVALID_CONTEXT", "i",                   ALC_INVALID_CONTEXT),
	GB_CONSTANT("INVALID_ENUM", "i",                      ALC_INVALID_ENUM),
	GB_CONSTANT("INVALID_VALUE", "i",                     ALC_INVALID_VALUE),
	GB_CONSTANT("OUT_OF_MEMORY", "i",                     ALC_OUT_OF_MEMORY),
	GB_CONSTANT("DEFAULT_DEVICE_SPECIFIER", "i",          ALC_DEFAULT_DEVICE_SPECIFIER),
	GB_CONSTANT("DEVICE_SPECIFIER", "i",                  ALC_DEVICE_SPECIFIER),
	GB_CONSTANT("EXTENSIONS", "i",                        ALC_EXTENSIONS),
	GB_CONSTANT("MAJOR_VERSION", "i",                     ALC_MAJOR_VERSION),
	GB_CONSTANT("MINOR_VERSION", "i",                     ALC_MINOR_VERSION),
	GB_CONSTANT("ATTRIBUTES_SIZE", "i",                   ALC_ATTRIBUTES_SIZE),
	GB_CONSTANT("ALL_ATTRIBUTES", "i",                    ALC_ALL_ATTRIBUTES),
	GB_CONSTANT("EXT_CAPTURE", "i",                       ALC_EXT_CAPTURE),
	GB_CONSTANT("CAPTURE_DEVICE_SPECIFIER", "i",          ALC_CAPTURE_DEVICE_SPECIFIER),
	GB_CONSTANT("CAPTURE_DEFAULT_DEVICE_SPECIFIER", "i",  ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER),
	GB_CONSTANT("CAPTURE_SAMPLES", "i",                   ALC_CAPTURE_SAMPLES),
	GB_CONSTANT("ENUMERATE_ALL_EXT", "i",                 ALC_ENUMERATE_ALL_EXT),
	GB_CONSTANT("DEFAULT_ALL_DEVICES_SPECIFIER", "i",     ALC_DEFAULT_ALL_DEVICES_SPECIFIER),
	GB_CONSTANT("ALL_DEVICES_SPECIFIER", "i",             ALC_ALL_DEVICES_SPECIFIER),

	GB_STATIC_METHOD("CreateContext", "AlcContext", ALC_CreateContext, "(Device)AlcDevice;[(Attributes)Integer[];]"),
	GB_STATIC_METHOD("MakeContextCurrent", "b", ALC_MakeContextCurrent, "(Context)AlcContext;"),
	GB_STATIC_METHOD("ProcessContext", NULL, ALC_ProcessContext, "(Context)AlcContext;"),
	GB_STATIC_METHOD("SuspendContext", NULL, ALC_SuspendContext, "(Context)AlcContext;"),
	GB_STATIC_METHOD("DestroyContext", NULL, ALC_DestroyContext, "(Context)AlcContext;"),
	GB_STATIC_METHOD("GetCurrentContext", "AlcContext", ALC_GetCurrentContext, NULL),
	GB_STATIC_METHOD("GetContextsDevice", "AlcDevice", ALC_GetContextsDevice, "(Context)AlcContext;"),
	GB_STATIC_METHOD("OpenDevice", "AlcDevice", ALC_OpenDevice, "(Name)s"),
	GB_STATIC_METHOD("CloseDevice", "b", ALC_CloseDevice, "(Device)AlcDevice;"),
	GB_STATIC_METHOD("GetError", "i", ALC_GetError, "(Device)AlcDevice;"),
	GB_STATIC_METHOD("IsExtensionPresent", "b", ALC_IsExtensionPresent, "(Device)AlcDevice;(ExtensionName)s"),
	GB_STATIC_METHOD("GetString", "s", ALC_GetString, "(Device)AlcDevice;(Param)i"),
	GB_STATIC_METHOD("GetStringv", "String[]", ALC_GetStringv, "(Device)AlcDevice;(Param)i"),
	GB_STATIC_METHOD("GetIntegerv", "Integer[]", ALC_GetIntegerv, "(Device)AlcDevice;(Param)i;(Size)i"),
	GB_STATIC_METHOD("CaptureOpenDevice", "AlcDevice", ALC_CaptureOpenDevice, "(Name)s(Frequency)i(Format)i(BufferSize)i"),
	GB_STATIC_METHOD("CaptureCloseDevice", "b", ALC_CloseDevice, "(Device)AlcDevice;"),
	GB_STATIC_METHOD("CaptureStart", NULL, ALC_CaptureStart, "(Device)AlcDevice;"),
	GB_STATIC_METHOD("CaptureStop", NULL, ALC_CaptureStop, "(Device)AlcDevice;"),
	GB_STATIC_METHOD("CaptureSamples", "v", ALC_CaptureSamples, "(Device)AlcDevice;(Samples)i[(Buffer)p]"),

	GB_END_DECLARE
};

#ifdef TOTO
// ALC_API ALCcontext *    ALC_APIENTRY alcCreateContext( ALCdevice *device, const ALCint* attrlist );
// ALC_API ALCboolean      ALC_APIENTRY alcMakeContextCurrent( ALCcontext *context );
// ALC_API void            ALC_APIENTRY alcProcessContext( ALCcontext *context );
// ALC_API void            ALC_APIENTRY alcSuspendContext( ALCcontext *context );
// ALC_API void            ALC_APIENTRY alcDestroyContext( ALCcontext *context );
// ALC_API ALCcontext *    ALC_APIENTRY alcGetCurrentContext( void );
// ALC_API ALCdevice*      ALC_APIENTRY alcGetContextsDevice( ALCcontext *context );
// ALC_API ALCdevice *     ALC_APIENTRY alcOpenDevice( const ALCchar *devicename );
// ALC_API ALCboolean      ALC_APIENTRY alcCloseDevice( ALCdevice *device );
// ALC_API ALCenum         ALC_APIENTRY alcGetError( ALCdevice *device );
// ALC_API ALCboolean      ALC_APIENTRY alcIsExtensionPresent( ALCdevice *device, const ALCchar *extname );
// ALC_API void  *         ALC_APIENTRY alcGetProcAddress( ALCdevice *device, const ALCchar *funcname );
// ALC_API ALCenum         ALC_APIENTRY alcGetEnumValue( ALCdevice *device, const ALCchar *enumname );
// ALC_API const ALCchar * ALC_APIENTRY alcGetString( ALCdevice *device, ALCenum param );
// ALC_API void            ALC_APIENTRY alcGetIntegerv( ALCdevice *device, ALCenum param, ALCsizei size, ALCint *data );
// ALC_API ALCdevice*      ALC_APIENTRY alcCaptureOpenDevice( const ALCchar *devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize );
// ALC_API ALCboolean      ALC_APIENTRY alcCaptureCloseDevice( ALCdevice *device );
// ALC_API void            ALC_APIENTRY alcCaptureStart( ALCdevice *device );
// ALC_API void            ALC_APIENTRY alcCaptureStop( ALCdevice *device );
// ALC_API void            ALC_APIENTRY alcCaptureSamples( ALCdevice *device, ALCvoid *buffer, ALCsizei samples );
#endif
