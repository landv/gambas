/***************************************************************************

  c_joystick.c

  (C) 2015-2019 Tobias Boege <tobias@gambas-buch.de>
  with bits from Cedron Dawg

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

#define __C_JOYSTICK_C

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <linux/joystick.h>

#include "c_joystick.h"

#define GB_ErrorErrno()	GB.Error(strerror(errno))

enum joystick_direction {
	DIRECTION_X, DIRECTION_Y
};

typedef struct {
	GB_BASE ob;
	int fd;
	struct js_event e;
	int e_init; /* last event had JS_EVENT_INIT bit set? */
} CJOYSTICK;

DECLARE_EVENT(EVENT_Stick);
DECLARE_EVENT(EVENT_Button);

static void read_cb(int fd, int flags, CJOYSTICK *joy)
{
again:
	if (read(fd, &joy->e, sizeof(joy->e)) != sizeof(joy->e)) {
		if (errno == EINTR)
			goto again;
		GB_ErrorErrno();
		return;
	}

	joy->e_init = !!(joy->e.type & JS_EVENT_INIT);

	switch (joy->e.type & ~JS_EVENT_INIT) {
	case JS_EVENT_AXIS:
		GB.Raise(joy, EVENT_Stick, 0);
		break;
	case JS_EVENT_BUTTON:
		GB.Raise(joy, EVENT_Button, 0);
		break;
	default:
		fprintf(stderr, "[gb.joystick] Unknown event type '%d'. "
		                "Please report this as a bug.\n", joy->e.type);
		break;
	}
}

#define THIS	((CJOYSTICK *) _object)

/**G
 * Create a new joystick object. The ~Path~ should point to the special
 * character device for the joystick, usually something like
 * `/dev/input/js0`.
 **/
BEGIN_METHOD(Joystick_new, GB_STRING path)

	/* Need a NUL-terminated string */
	char *path = GB.TempString(STRING(path), LENGTH(path));
	int fd;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		GB_ErrorErrno();
		return;
	}
	THIS->fd = fd;
	GB.Watch(fd, GB_WATCH_READ, (void *) read_cb, (intptr_t) THIS);

END_METHOD

BEGIN_METHOD_VOID(Joystick_free)

	GB.Watch(THIS->fd, GB_WATCH_NONE, NULL, 0);
	close(THIS->fd);

END_METHOD

/**G
 * Return the number of sticks of the joystick.
 **/
BEGIN_PROPERTY(Joystick_Sticks)

	unsigned char axes;

	if (ioctl(THIS->fd, JSIOCGAXES, &axes) == -1) {
		GB_ErrorErrno();
		return;
	}
	GB.ReturnInteger(axes / 2);

END_PROPERTY

/**G
 * Return the number of buttons of the joystick.
 **/
BEGIN_PROPERTY(Joystick_Buttons)

	unsigned char buttons;

	if (ioctl(THIS->fd, JSIOCGBUTTONS, &buttons) == -1) {
		GB_ErrorErrno();
		return;
	}
	GB.ReturnInteger(buttons);

END_PROPERTY

/**G
 * Return the device ID of the joystick.
 **/
BEGIN_PROPERTY(Joystick_Name)

	char name[128];

	memset(name, 0, sizeof(name));
	if (ioctl(THIS->fd, JSIOCGNAME(128), name) == -1) {
		GB_ErrorErrno();
		return;
	}
	GB.ReturnNewZeroString(name);

END_PROPERTY

/**G
 * When a joystick is opened, it issues a series of synthetic events
 * for you to determine the initial state of axes and buttons.
 *
 * This property indicates if the latest event was such an initialisation
 * event.
 **/
BEGIN_PROPERTY(Joystick_Init)

	GB.ReturnBoolean(THIS->e_init);

END_PROPERTY

/**G
 * Returns the number of the button that caused the most recent
 * [Button](../.button) event.
 **/
BEGIN_PROPERTY(Joystick_Button)

	GB.ReturnInteger(THIS->e.number);

END_PROPERTY

/**G
 * Returns the number of the stick that caused the most recent
 * [Stick](../.stick) event.
 **/
BEGIN_PROPERTY(Joystick_Stick)

	GB.ReturnInteger(THIS->e.number / 2);

END_PROPERTY

/**G
 * Returns the direction, [Joystick.X](../x) or [Joystick.Y](../y),
 * of the most recent [Stick](../.stick) event.
 **/
BEGIN_PROPERTY(Joystick_Direction)

	GB.ReturnInteger(
		THIS->e.number % 2 == 0 ?
			DIRECTION_X :
			DIRECTION_Y
	);

END_PROPERTY

/**G
 * In a Button event, this value is 0 or 1, according to whether the
 * [button](../button) was pressed or released.
 *
 * In a Stick event, the value gives the position of the [stick](../stick)
 * along the indicated [direction](../direction) on a scale of -32767 to
 * +32767, the range of a [Short](/lang/type/short).
 *
 * [[ warning
 * The actual range of values seen in this property might be smaller
 * than the full Short range **if the joystick calibration is off**.
 * There exist external tools to recalibrate it but you may want to
 * use *thresholds* when determining the intensity of the stick tilt,
 * to make your program more robust to uncalibrated devices.
 * ]]
 **/
BEGIN_PROPERTY(Joystick_Value)

	GB.ReturnInteger(THIS->e.value);

END_PROPERTY

/**G
 * A timestamp for the event in milliseconds. You can use this to detect
 * whether buttons were pressed or sticks were moved "almost" simultaneously,
 * independently of how fast you process them in your program.
 *
 * [[ warning
 * The timestamp comes out of the kernel as an unsigned 32-bit integer,
 * but Gambas has only signed integers. As such, timestamps will overflow
 * and change sign around every 25 days and reset to zero every 50 days.
 * ]]
 **/
BEGIN_PROPERTY(Joystick_Time)

	GB.ReturnInteger(THIS->e.time);

END_PROPERTY

/**G
 * This class represents a joystick/gamepad handled by the legacy
 * [Linux Joystick API]. Once created it raises its [Button](.button)
 * and [Stick](.stick) events to indicate input from the device.
 *
 * Some properties describe the device itself, while others describe
 * the latest event that occurred. You should use them during event
 * handlers only.
 *
 * [Linux Joystick API]: https://www.kernel.org/doc/html/latest/input/joydev/joystick-api.html
 **/
GB_DESC CJoystick[] = {
	GB_DECLARE("Joystick", sizeof(CJOYSTICK)),

	/**G
	 * The value of Joystick.Direction when the Stick event was caused
	 * by movement of the X axis of the stick.
	 **/
	GB_CONSTANT("X", "i", DIRECTION_X),
	/**G
	 * The value of Joystick.Direction when the Stick event was caused
	 * by movement of the Y axis of the stick.
	 **/
	GB_CONSTANT("Y", "i", DIRECTION_Y),

	GB_PROPERTY_READ("Sticks", "i", Joystick_Sticks),
	GB_PROPERTY_READ("Buttons", "i", Joystick_Buttons),
	GB_PROPERTY_READ("Name", "s", Joystick_Name),
	//GB_PROPERTY_READ("DriverVersion", "i", Joystick_DriverVersion),
	//GB_PROPERTY("Correction", ".Joystick.Correction", Joystick_Correction),
	// There are a few more ioctls in my <linux/joystick.h>
	// which I found no documentation for...

	/**G
	 * Raised when a joystick stick changes position.
	 * The index of the stick is stored in the [Stick](../stick)
	 * property, the direction along which the position changed
	 * in the [Direction](../direction) property and the new
	 * position of the stick is the [Value](../value) property.
	 **/
	GB_EVENT("Stick", NULL, NULL, &EVENT_Stick),
	/**G
	 * Raised when a joystick button was pressed or released.
	 * The index of the button is stored in the [Button](../button)
	 * property. Its new state can be found in the [Value](../value)
	 * property.
	 **/
	GB_EVENT("Button", NULL, NULL, &EVENT_Button),

	GB_METHOD("_new", NULL, Joystick_new, "(Path)s"),
	GB_METHOD("_free", NULL, Joystick_free, NULL),

	/* Event data */
	GB_PROPERTY_READ("Init", "b", Joystick_Init),
	GB_PROPERTY_READ("Stick", "i", Joystick_Stick),
	GB_PROPERTY_READ("Direction", "i", Joystick_Direction),
	GB_PROPERTY_READ("Button", "i", Joystick_Button),
	GB_PROPERTY_READ("Value", "i", Joystick_Value),
	GB_PROPERTY_READ("Time", "i", Joystick_Time),

	GB_END_DECLARE
};
