/*
 * c_watch.c - Watch and .Watch.Events classes
 *
 * Copyright (C) 2013, 2014 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#define __C_WATCH_C

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include <limits.h>

#include "gambas.h"
#include "c_watch.h"

#define GB_ErrorErrno()	GB.Error(strerror(errno))

typedef struct {
	GB_BASE ob;
	int wd;
	uint32_t mask;
	char *path;
	int paused;
	GB_VARIANT_VALUE tag;
} CWATCH;

typedef struct cinfo CINFO;
struct cinfo {
	struct inotify_event *iev;
	CINFO *prev;
};

typedef struct {
	int fd;
	GB_HASHTABLE watches;
	CINFO *top;
} CINOTIFY;

/* XXX: We currently only support one inotify instance */
static CINOTIFY _ino = {-1, NULL, NULL};

static CWATCH *find_watch(CINOTIFY *ino, int wd)
{
	CWATCH *watch = NULL;

	GB.HashTable.Get(ino->watches, (char *) &wd, sizeof(wd),
			 (void **) &watch);
	/* If the lookup fails, `watch' isn't changed */
	return watch;
}

static CWATCH *find_watch_path(CINOTIFY *ino, const char *path)
{
	CWATCH *watch = NULL;

	GB.HashTable.Get(ino->watches, path, 0, (void **) &watch);
	return watch;
}

DECLARE_EVENT(EVENT_Read);
DECLARE_EVENT(EVENT_Stat);
DECLARE_EVENT(EVENT_Close);
DECLARE_EVENT(EVENT_Create);
DECLARE_EVENT(EVENT_Delete);
DECLARE_EVENT(EVENT_Write);
DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_MoveFrom);
DECLARE_EVENT(EVENT_MoveTo);
DECLARE_EVENT(EVENT_Open);

static struct {
	int *eventp;
	uint32_t mask;
} _event_table[] = {
	{&EVENT_Read,     IN_ACCESS},
	{&EVENT_Stat,     IN_ATTRIB},
	{&EVENT_Close,    IN_CLOSE_WRITE | IN_CLOSE_NOWRITE},
	{&EVENT_Create,   IN_CREATE},
	{&EVENT_Delete,   IN_DELETE | IN_DELETE_SELF},
	{&EVENT_Write,    IN_MODIFY},
	{&EVENT_Move,     IN_MOVE_SELF},
	{&EVENT_MoveFrom, IN_MOVED_FROM},
	{&EVENT_MoveTo,   IN_MOVED_TO},
	{&EVENT_Open,     IN_OPEN},
	{NULL, 0}
};

static int *get_event(uint32_t *maskp)
{
	int i;

	for (i = 0; _event_table[i].eventp; i++) {
		if (*maskp & _event_table[i].mask) {
			/* Remove that event */
			*maskp &= ~_event_table[i].mask;
			return _event_table[i].eventp;
		}
	}
	*maskp = 0;
	return NULL;
}

/* Forward */
static void INOTIFY_exit(void);

static void destroy_watch(CWATCH *watch)
{
	inotify_rm_watch(_ino.fd, watch->wd);
	GB.HashTable.Remove(_ino.watches, (char *) &watch->wd,
			    sizeof(watch->wd));
	GB.HashTable.Remove(_ino.watches, watch->path, 0);

	GB.FreeString(&watch->path);
	GB.StoreVariant(NULL, &watch->tag);
	/* Serves as a validity flag */
	watch->path = NULL;

	/* Close the inotify instance again if all watches were released */
	if (!GB.HashTable.Count(_ino.watches))
		INOTIFY_exit();
}

static int check_watch(CWATCH *watch)
{
	return !watch || !watch->path;
}

static void callback(int fd, int flags, CINOTIFY *ino)
{
	struct inotify_event *iev;
	char buf[sizeof(*iev) + NAME_MAX + 1]
		__attribute__((aligned(sizeof(int))));
	int i, length;

again:
	if ((length = read(fd, buf, sizeof(buf))) <= 0) {
		if (errno == EINTR) {
			goto again;
		} else {
			GB_ErrorErrno();
			return;
		}
	}

	for (i = 0; i < length; i += sizeof(*iev) + iev->len) {
		CWATCH *watch;
		uint32_t mask;

		iev = (struct inotify_event *) &buf[i];

		watch = find_watch(ino, iev->wd);
		if (!watch && !(iev->mask & IN_Q_OVERFLOW)) {
			if (getenv("GB_INOTIFY_DEBUG")) {
				fprintf(stderr, "gb.inotify: Descriptor "
					"%d not known. Name was `%s'\n",
					iev->wd, iev->name);
			}
			continue;
		}

		mask = iev->mask & ~(IN_IGNORED | IN_ISDIR | IN_Q_OVERFLOW
				   | IN_UNMOUNT);
		while (flags) {
			uint32_t oldmask = mask;
			int *eventp;
			CINFO info;

			eventp = get_event(&mask);
			if (!eventp) {
				if (getenv("GB_INOTIFY_DEBUG")) {
					fprintf(stderr, "gb.inotify: "
						"Unhandled event 0x%x "
						"of watch `%s'\n",
						oldmask ^ mask,
						watch->path);
				}
				break;
			}
			if (!GB.CanRaise(watch, *eventp) || watch->paused)
				continue;

			/*
			 * In case, the event loop kicks in at GB.Raise()
			 * and preempts this callback with itself.
			 *
			 * The problem is then that the event info structure
			 * may get replaced by another, thus we lose data.
			 * That's why we save it here. However, it is
			 * sufficient to keep the CINFO on the stack!
			 */
			info.iev = iev;
			info.prev = _ino.top;
			_ino.top = &info;

			GB.Raise(watch, *eventp, 0);

			_ino.top = info.prev;
		}
		/* Also remove the watch if the kernel did. */
		if (iev->mask & IN_IGNORED)
			destroy_watch(watch);
	}
}

static void INOTIFY_init(void)
{
	_ino.fd = inotify_init();
	if (_ino.fd == -1) {
		GB_ErrorErrno();
		return;
	}
	GB.HashTable.New(&_ino.watches, GB_COMP_BINARY);
	GB.Watch(_ino.fd, GB_WATCH_READ, callback, (intptr_t) &_ino);
	_ino.top = NULL;
}

static void INOTIFY_exit(void)
{
	GB.Watch(_ino.fd, GB_WATCH_NONE, NULL, (intptr_t) NULL);
	close(_ino.fd);
	_ino.fd = -1;
	GB.HashTable.Free(&_ino.watches);
}

/**G
 * Tidy up.
 **/
BEGIN_METHOD_VOID(Watch_exit)

	if (!_ino.watches)
		return;
	/* Free the remaining objects. destroy_watch() will then also take
	 * care of calling INOTIFY_exit() to free bookkeeping data. */
	GB.HashTable.Enum(_ino.watches, (GB_HASHTABLE_ENUM_FUNC) destroy_watch);

END_METHOD

/**G
 * Return a Watch from its path. If the path is not watched, Null is
 * returned.
 **/
BEGIN_METHOD(Watch_get, GB_STRING path)

	CWATCH *watch;

	watch = find_watch_path(&_ino, GB.ToZeroString(ARG(path)));
	if (!watch) {
		GB.ReturnNull();
		return;
	}
	GB.ReturnObject(watch);

END_METHOD

/**G
 * The name of the file or directory which is subject to the event, relative
 * to the Watch object which triggered the event. If Null, the directory
 * itself is subject.
 **/
BEGIN_PROPERTY(Watch_Name)

	struct inotify_event *iev = _ino.top->iev;

	GB.ReturnNewZeroString(iev->len ? iev->name : "");

END_PROPERTY

/**G
 * Return whether the event subject is/was a directory.
 **/
BEGIN_PROPERTY(Watch_IsDir)

	GB.ReturnBoolean(!!(_ino.top->iev->mask & IN_ISDIR));

END_PROPERTY

/**G
 * Return whether the filesystem on which the watched path resided, was
 * unmounted. In this case, the Watch object is invalidated just after the
 * event.
 **/
BEGIN_PROPERTY(Watch_Unmount)

	GB.ReturnBoolean(!!(_ino.top->iev->mask & IN_UNMOUNT));

END_PROPERTY

/**G
 * A cookie used to associate events. This is currently only used to connect
 * MoveFrom and MoveTo events of the same file.
 **/
BEGIN_PROPERTY(Watch_Cookie)

	GB.ReturnInteger(_ino.top->iev->cookie);

END_PROPERTY

#define THIS	((CWATCH *) _object)

/**G
 * Create a new watch for the given path.
 *
 * If the NoFollowLink parameter is set and true, symlinks are not followed.
 *
 * Events is a bitmask specifying which events are to be reported at all.
 * The default mask is determined by the event handlers defined for this
 * object.
 *
 * TODO: Maybe add support for IN_EXCL_UNLINK, IN_ONESHOT and IN_ONLYDIR.
 **/
BEGIN_METHOD(Watch_new, GB_STRING path; GB_BOOLEAN nofollow;
			GB_INTEGER mask)

	int wd, i;
	uint32_t mask = VARGOPT(mask, 0);
	char *path = GB.NewString(STRING(path), LENGTH(path));

	/* If this is the first watch, we need an inotify instance first.
	 * We don't use the component's init function to set up _ino because
	 * the inotify instance needs a GB.Watch() which would keep the
	 * program open even if no watches were registered. So this is done
	 * ad-hoc if there are watches. */
	if (_ino.fd == -1)
		INOTIFY_init();

	/* Get the mask */
	if (!mask) {
		for (i = 0; _event_table[i].eventp; i++) {
			if (GB.CanRaise(THIS, *_event_table[i].eventp))
				mask |= _event_table[i].mask;
		}
		/* But we need at least to watch one event. IN_DELETE_SELF
		 * seems a good one as it doesn't trigger too often :-) */
		mask |= IN_DELETE_SELF;
	}

	if (VARGOPT(nofollow, 0))
		mask |= IN_DONT_FOLLOW;

	wd = inotify_add_watch(_ino.fd, path, mask);
	if (wd == -1) {
		GB.FreeString(&path);
		GB_ErrorErrno();
		return;
	}
	THIS->wd = wd;
	THIS->mask = mask;
	THIS->path = path;
	THIS->paused = 0;
	THIS->tag.type = GB_T_NULL;

	GB.HashTable.Add(_ino.watches, (char *) &wd, sizeof(wd), THIS);
	GB.HashTable.Add(_ino.watches, path, 0, THIS);

END_METHOD

/**G
 * Deallocate the watch.
 **/
BEGIN_METHOD_VOID(Watch_free)

	destroy_watch(THIS);

END_METHOD

/**G
 * Resume a paused watch. Whether the watch was actually paused does not
 * matter.
 */
BEGIN_METHOD_VOID(Watch_Resume)

	inotify_add_watch(_ino.fd, THIS->path, THIS->mask);

END_METHOD

/**G
 * Pause a watch. You can pause a watch multiple times. This has no effect.
 *
 * A paused watch will not monitor any events -- without formally altering
 * the events bitmask -- and thus not inflict any interruption by events
 * from the kernel.
 */
BEGIN_METHOD_VOID(Watch_Pause)

	/* We can't really pause as the kernel won't allow us to set an
	 * empty mask. We pick an unlikely one here and add special logic
	 * to the event dispatching code to prevent an event from being
	 * raised if the watch is paused. */
	inotify_add_watch(_ino.fd, THIS->path, IN_DELETE_SELF);

END_METHOD

/**G
 * Return the watched path.
 **/
BEGIN_PROPERTY(Watch_Path)

	GB.ReturnString(THIS->path);

END_PROPERTY

/**G
 * Return or set if the watch is paused.
 *
 * {seealso
 *   Pause(), Resume()
 * }
 **/
BEGIN_PROPERTY(Watch_IsPaused)

	if (READ_PROPERTY) {
		GB.ReturnBoolean(THIS->paused);
		return;
	}
	THIS->paused = VPROP(GB_BOOLEAN);
	if (THIS->paused)
		CALL_METHOD_VOID(Watch_Pause);
	else
		CALL_METHOD_VOID(Watch_Resume);

END_PROPERTY

/**G
 * This Variant is free for use by the Gambas programmer.
 **/
BEGIN_PROPERTY(Watch_Tag)

	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->tag);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->tag);

END_PROPERTY

GB_DESC CWatch[] = {
	GB_DECLARE("Watch", sizeof(CWATCH)),

	GB_HOOK_CHECK(check_watch),

	/*
	 * Watch event bits
	 */
	GB_CONSTANT("Read", "i", IN_ACCESS),
	GB_CONSTANT("Stat", "i", IN_ATTRIB),
	GB_CONSTANT("Close", "i", IN_CLOSE_WRITE | IN_CLOSE_NOWRITE),
	GB_CONSTANT("Create", "i", IN_CREATE),
	GB_CONSTANT("Delete", "i", IN_DELETE | IN_DELETE_SELF),
	GB_CONSTANT("Write", "i", IN_MODIFY),
	GB_CONSTANT("Move", "i", IN_MOVE_SELF),
	GB_CONSTANT("MoveFrom", "i", IN_MOVED_FROM),
	GB_CONSTANT("MoveTo", "i", IN_MOVED_TO),
	GB_CONSTANT("Open", "i", IN_OPEN),
	GB_CONSTANT("All", "i", IN_ALL_EVENTS),

	GB_EVENT("Read", NULL, NULL, &EVENT_Read),
	GB_EVENT("Stat", NULL, NULL, &EVENT_Stat),
	GB_EVENT("Close", NULL, NULL, &EVENT_Close),
	GB_EVENT("Create", NULL, NULL, &EVENT_Create),
	GB_EVENT("Delete", NULL, NULL, &EVENT_Delete),
	GB_EVENT("Write", NULL, NULL, &EVENT_Write),
	GB_EVENT("Move", NULL, NULL, &EVENT_Move),
	GB_EVENT("MoveFrom", NULL, NULL, &EVENT_MoveFrom),
	GB_EVENT("MoveTo", NULL, NULL, &EVENT_MoveTo),
	GB_EVENT("Open", NULL, NULL, &EVENT_Open),

	GB_STATIC_METHOD("_exit", NULL, Watch_exit, NULL),
	GB_STATIC_METHOD("_get", "Watch", Watch_get, "(Path)s"),

	GB_METHOD("Resume", NULL, Watch_Resume, NULL),
	GB_METHOD("Pause", NULL, Watch_Pause, NULL),

	GB_STATIC_PROPERTY_READ("Name", "s", Watch_Name),
	GB_STATIC_PROPERTY_READ("IsDir", "b", Watch_IsDir),
	GB_STATIC_PROPERTY_READ("Unmount", "b", Watch_Unmount),
	GB_STATIC_PROPERTY_READ("Cookie", "i", Watch_Cookie),

	GB_METHOD("_new", NULL, Watch_new, "(Path)s[(NoFollowLink)b(Events)i]"),
	GB_METHOD("_free", NULL, Watch_free, NULL),

	GB_PROPERTY_SELF("Events", ".Watch.Events"),
	GB_PROPERTY_READ("Path", "s", Watch_Path),
	GB_PROPERTY("IsPaused", "b", Watch_IsPaused),
	GB_PROPERTY("Tag", "v", Watch_Tag),

	GB_END_DECLARE
};

static void change_mask(CWATCH *watch, int set, uint32_t bits)
{
	if (set)
		watch->mask |= bits;
	else
		watch->mask &= ~bits;
	/* Re-add with new mask */
	inotify_add_watch(_ino.fd, watch->path, watch->mask);
}

/**G
 * Return if the given flag is set. You can also combine multiple flags to
 * check if at least one of them is set.
 **/
BEGIN_METHOD(WatchEvents_get, GB_INTEGER flags)

	GB.ReturnBoolean(!!(THIS->mask & VARG(flags)));

END_METHOD

/**G
 * Set or clear a flag. You can combine multiple flags to set or clear them
 * en masse.
 **/
BEGIN_METHOD(WatchEvents_put, GB_BOOLEAN value; GB_INTEGER flags)

	change_mask(THIS, !!VARG(value), VARG(flags));

END_METHOD

GB_DESC CWatchEvents[] = {
	GB_DECLARE(".Watch.Events", 0),
	GB_VIRTUAL_CLASS(),

	GB_METHOD("_get", "b", WatchEvents_get, "(Flags)i"),
	GB_METHOD("_put", NULL, WatchEvents_put, "(Value)b(Flags)i"),

	GB_END_DECLARE
};
