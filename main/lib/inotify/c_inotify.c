/*
 * c_inotify.c - Inotify and .InotifyEvents classes
 *
 * Copyright (C) 2013 Tobias Boege <tobias@gambas-buch.de>
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

#define __C_INOTIFY_C

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>
#include <limits.h>

#include "gambas.h"
#include "c_inotify.h"

#define GB_ErrorErrno()	GB.Error(strerror(errno))

typedef struct cwatch {
	GB_BASE ob;
	int wd;
	uint32_t mask;
	char *path;
	/* TODO: Using a linked list can get expensive in searching! Prepare
	 *       a C interface for AvlTree and use it here. */
	struct cwatch *next;
	GB_VARIANT_VALUE tag;
} CWATCH;

typedef struct {
	/*
	 * Not sure if this will be a Gambas object. Would be better if this
	 * native code would detect when one instance's watch list is big
	 * enough and allocate and manage a new instance behind the scenes.
	 */
	/* GB_BASE ob; */
	int fd;
	CWATCH *first;
} CINOTIFY;

/* XXX: We currently only support one inotify instance */
static CINOTIFY _ino = {-1, NULL};

static CWATCH *find_watch(CWATCH *watches, int wd)
{
	while (watches) {
		if (watches->wd == wd)
			return watches;
		watches = watches->next;
	}
	return NULL;
}

static CWATCH *find_watch_path(CWATCH *watches, const char *path)
{
	while (watches) {
		if (!strcmp(watches->path, path))
			return watches;
		watches = watches->next;
	}
	return NULL;
}

DECLARE_EVENT(EVENT_Access);
DECLARE_EVENT(EVENT_Attribute);
DECLARE_EVENT(EVENT_CloseWrite);
DECLARE_EVENT(EVENT_CloseNoWrite);
DECLARE_EVENT(EVENT_Create);
DECLARE_EVENT(EVENT_Delete);
DECLARE_EVENT(EVENT_Modify);
DECLARE_EVENT(EVENT_Move);
DECLARE_EVENT(EVENT_MovedFrom);
DECLARE_EVENT(EVENT_MovedTo);
DECLARE_EVENT(EVENT_Open);
DECLARE_EVENT(EVENT_Unmount);

static struct {
	int *eventp;
	uint32_t mask;
} _event_table[] = {
	{&EVENT_Access,       IN_ACCESS},
	{&EVENT_Attribute,    IN_ATTRIB},
	{&EVENT_CloseWrite,   IN_CLOSE_WRITE},
	{&EVENT_CloseNoWrite, IN_CLOSE_NOWRITE},
	{&EVENT_Create,       IN_CREATE},
	{&EVENT_Delete,       IN_DELETE | IN_DELETE_SELF},
	{&EVENT_Modify,       IN_MODIFY},
	{&EVENT_Move,         IN_MOVE_SELF},
	{&EVENT_MovedFrom,    IN_MOVED_FROM},
	{&EVENT_MovedTo,      IN_MOVED_TO},
	{&EVENT_Open,         IN_OPEN},
	{&EVENT_Unmount,      IN_UNMOUNT},
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

static void callback(int fd, int flags, CWATCH **firstp)
{
	struct inotify_event *iev;
	char buf[sizeof(*iev) + NAME_MAX + 1], *name;
	int i, length;
	CWATCH *watch;

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
		iev = (struct inotify_event *) &buf[i];

		watch = find_watch(*firstp, iev->wd);
		if (!watch) {
			if (getenv("GB_INOTIFY_DEBUG")) {
				fprintf(stderr, "Inotify: Descriptor "
					"%d not known. Name was '%s'\n",
					iev->wd, iev->name);
			}
			continue;
		}
		name = iev->len ? iev->name : NULL;

		while (iev->mask) {
			uint32_t oldmask;
			int *eventp;

			oldmask = iev->mask;
			eventp = get_event(&iev->mask);
			if (!eventp) {
				if (getenv("GB_INOTIFY_DEBUG")) {
					fprintf(stderr, "Inotify: Unhandled "
						"event 0x%x of watch '%s'\n",
						oldmask ^ iev->mask,
						watch->path);
				}
				continue;
			}
			if (!GB.CanRaise(watch, *eventp))
				continue;
			/* `name' may be NULL which is fine and indicates
			 * that the watched directory itself triggered the
			 * event. */
			/*
			 * TODO: Implement the move cookie.
			 */
			GB.Raise(watch, *eventp, 1, GB_T_STRING, name, 0);
		}
	}
}

static void INOTIFY_init(void)
{
	_ino.fd = inotify_init();
	if (_ino.fd == -1) {
		GB_ErrorErrno();
		return;
	}
	GB.Watch(_ino.fd, GB_WATCH_READ, callback, (intptr_t) &_ino.first);
}

static void INOTIFY_exit(void)
{
	GB.Watch(_ino.fd, GB_WATCH_NONE, NULL, (intptr_t) NULL);
	close(_ino.fd);
	_ino.first = NULL;
}

/**G
 * Return an inotify watch from its path. If the path is not watched, Null
 * is returned.
 **/
BEGIN_METHOD(Inotify_get, GB_STRING path)

	CWATCH *watch;

	watch = find_watch_path(_ino.first, GB.ToZeroString(ARG(path)));
	if (!watch) {
		GB.ReturnNull();
		return;
	}
	GB.ReturnObject(watch);

END_METHOD

/**G
 * Automatically free all remaining watches.
 **/
BEGIN_METHOD_VOID(Inotify_exit)

	CWATCH *watch = _ino.first, *next;

	while (watch) {
		next = watch->next;
		GB.Unref((void **) &watch);
		watch = next;
	}
	_ino.first = NULL;

END_METHOD

#define THIS	((CWATCH *) _object)

/**G
 * Create a new watch for the given path.
 *
 * If the NoFollowLink parameter is given, symlinks are not followed.
 *
 * Events is a bitmask specifying which events are to be reported at all. If
 * you know that you only want to watch certain events, you should give them
 * here as this increases performance. By default, all events are watched.
 *
 * TODO: Maybe add support for IN_EXCL_UNLINK, IN_ONESHOT and IN_ONLYDIR.
 **/
BEGIN_METHOD(Inotify_new, GB_STRING path; GB_BOOLEAN nofollow;
			  GB_INTEGER mask)

	int wd;
	uint32_t mask = VARGOPT(mask, IN_ALL_EVENTS);
	char *path = GB.NewZeroString(GB.ToZeroString(ARG(path)));

	/* If this is the first watch, we need an inotify instance first.
	 * We don't use the component's init function to set up _ino because
	 * the inotify instance needs a GB.Watch() which would keep the
	 * program open even if no watches were registered. So this is done
	 * ad-hoc if there are watches. */
	if (!_ino.first)
		INOTIFY_init();

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
	THIS->tag.type = GB_T_NULL;

	THIS->next = _ino.first;
	_ino.first = THIS;

	/* Give the object a reference because it is watched and available
	 * in the list of watches. */
	GB.Ref(_object);

END_METHOD

/**G
 * Deallocate the watch.
 **/
BEGIN_METHOD_VOID(Inotify_free)

	CWATCH *watches, *last = NULL;

	inotify_rm_watch(_ino.fd, THIS->wd);
	GB.FreeString(&THIS->path);
	GB.StoreVariant(NULL, &THIS->tag);

	/* Remove from the list */
	watches = _ino.first;
	while (watches) {
		if (watches == THIS) {
			if (!last)
				_ino.first = THIS->next;
			else
				last->next = THIS->next;
		}
		watches = watches->next;
	}
	/* Close the inotify instance again if all watches were released */
	if (!_ino.first)
		INOTIFY_exit();

END_METHOD

/**G
 * Drop the reference which the object received because it is watched. When
 * the Inotify object is not referenced anywhere in your program, it will
 * now be freed.
 **/
BEGIN_METHOD_VOID(Inotify_Release)

	GB.Unref(&_object);

END_METHOD

/**G
 * Return the watched path.
 **/
BEGIN_PROPERTY(Inotify_Path)

	GB.ReturnString(THIS->path);

END_PROPERTY

/**G
 * This Variant is free for use by the Gambas programmer.
 **/
BEGIN_PROPERTY(Inotify_Tag)

	if (READ_PROPERTY) {
		GB.ReturnVariant(&THIS->tag);
		return;
	}
	GB.StoreVariant(PROP(GB_VARIANT), &THIS->tag);

END_PROPERTY

GB_DESC CInotify[] = {
	GB_DECLARE("Inotify", sizeof(CWATCH)),

	/*
	 * Inotify event bits
	 */
	GB_CONSTANT("Access", "i", IN_ACCESS),
	GB_CONSTANT("Attribute", "i", IN_ATTRIB),
	GB_CONSTANT("CloseWrite", "i", IN_CLOSE_WRITE),
	GB_CONSTANT("CloseNoWrite", "i", IN_CLOSE_NOWRITE),
	GB_CONSTANT("Close", "i", IN_CLOSE),
	GB_CONSTANT("Create", "i", IN_CREATE),
	GB_CONSTANT("Delete", "i", IN_DELETE),
	GB_CONSTANT("DeleteSelf", "i", IN_DELETE_SELF),
	GB_CONSTANT("Modify", "i", IN_MODIFY),
	GB_CONSTANT("MoveSelf", "i", IN_MOVE_SELF),
	GB_CONSTANT("MovedFrom", "i", IN_MOVED_FROM),
	GB_CONSTANT("MovedTo", "i", IN_MOVED_TO),
	GB_CONSTANT("Move", "i", IN_MOVE),
	GB_CONSTANT("Open", "i", IN_OPEN),
	GB_CONSTANT("Unmount", "i", IN_UNMOUNT),
	GB_CONSTANT("All", "i", IN_ALL_EVENTS),

	GB_EVENT("Access", NULL, "(File)s", &EVENT_Access),
	GB_EVENT("Attribute", NULL, "(File)s", &EVENT_Attribute),
	GB_EVENT("CloseWrite", NULL, "(File)s", &EVENT_CloseWrite),
	GB_EVENT("CloseNoWrite", NULL, "(File)s", &EVENT_CloseNoWrite),
	GB_EVENT("Create", NULL, "(File)s", &EVENT_Create),
	GB_EVENT("Delete", NULL, "(File)s", &EVENT_Delete),
	GB_EVENT("Modify", NULL, "(File)s", &EVENT_Modify),
	GB_EVENT("Move", NULL, "(File)s", &EVENT_Move),
	GB_EVENT("MovedFrom", NULL, "(File)s", &EVENT_MovedFrom),
	GB_EVENT("MovedTo", NULL, "(File)s", &EVENT_MovedTo),
	GB_EVENT("Open", NULL, "(File)s", &EVENT_Open),
	GB_EVENT("Unmount", NULL, "(File)s", &EVENT_Unmount),

	GB_STATIC_METHOD("_exit", NULL, Inotify_exit, NULL),
	GB_STATIC_METHOD("_get", "Inotify", Inotify_get, "(Path)s"),

	GB_METHOD("_new", NULL, Inotify_new, "(Path)s[(NoFollowLink)b(Events)i]"),
	GB_METHOD("_free", NULL, Inotify_free, NULL),
	GB_METHOD("Release", NULL, Inotify_Release, NULL),

	GB_PROPERTY_SELF("Events", ".InotifyEvents"),
	GB_PROPERTY_READ("Path", "s", Inotify_Path),
	GB_PROPERTY("Tag", "v", Inotify_Tag),

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

#define IMPLEMENT_InotifyEvent(which, bits)		\
BEGIN_PROPERTY(InotifyEvents_ ## which)			\
							\
	if (READ_PROPERTY) {				\
		GB.ReturnBoolean(THIS->mask & bits);	\
		return;					\
	}						\
	change_mask(THIS, VPROP(GB_BOOLEAN), bits);	\
							\
END_PROPERTY

IMPLEMENT_InotifyEvent(Access, IN_ACCESS)
IMPLEMENT_InotifyEvent(Attribute, IN_ATTRIB)
IMPLEMENT_InotifyEvent(CloseWrite, IN_CLOSE_WRITE)
IMPLEMENT_InotifyEvent(CloseNoWrite, IN_CLOSE_NOWRITE)
IMPLEMENT_InotifyEvent(Close, IN_CLOSE)
IMPLEMENT_InotifyEvent(Create, IN_CREATE)
IMPLEMENT_InotifyEvent(Delete, IN_DELETE)
IMPLEMENT_InotifyEvent(DeleteSelf, IN_DELETE_SELF)
IMPLEMENT_InotifyEvent(Modify, IN_MODIFY)
IMPLEMENT_InotifyEvent(MoveSelf, IN_MOVE_SELF)
IMPLEMENT_InotifyEvent(MovedFrom, IN_MOVED_FROM)
IMPLEMENT_InotifyEvent(MovedTo, IN_MOVED_TO)
IMPLEMENT_InotifyEvent(Move, IN_MOVE)
IMPLEMENT_InotifyEvent(Open, IN_OPEN)
IMPLEMENT_InotifyEvent(Unmount, IN_UNMOUNT)
IMPLEMENT_InotifyEvent(All, IN_ALL_EVENTS)

GB_DESC CInotifyEvents[] = {
	GB_DECLARE(".InotifyEvents", 0),
	GB_VIRTUAL_CLASS(),

	GB_PROPERTY("Access", "b", InotifyEvents_Access),
	GB_PROPERTY("Attribute", "b", InotifyEvents_Attribute),
	GB_PROPERTY("CloseWrite", "b", InotifyEvents_CloseWrite),
	GB_PROPERTY("CloseNoWrite", "b", InotifyEvents_CloseNoWrite),
	GB_PROPERTY("Close", "b", InotifyEvents_Close),
	GB_PROPERTY("Create", "b", InotifyEvents_Create),
	GB_PROPERTY("Delete", "b", InotifyEvents_Delete),
	GB_PROPERTY("DeleteSelf", "b", InotifyEvents_DeleteSelf),
	GB_PROPERTY("Modify", "b", InotifyEvents_Modify),
	GB_PROPERTY("MoveSelf", "b", InotifyEvents_MoveSelf),
	GB_PROPERTY("MovedFrom", "b", InotifyEvents_MovedFrom),
	GB_PROPERTY("MovedTo", "b", InotifyEvents_MovedTo),
	GB_PROPERTY("Move", "b", InotifyEvents_Move),
	GB_PROPERTY("Open", "b", InotifyEvents_Open),
	GB_PROPERTY("Unmount", "b", InotifyEvents_Unmount),
	GB_PROPERTY("All", "b", InotifyEvents_All),

	GB_END_DECLARE
};
