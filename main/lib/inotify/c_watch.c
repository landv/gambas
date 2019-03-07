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

#include "main.h"
#include "c_watch.h"

//#define DEBUG_ME 1

#define GB_ErrorErrno()	GB.Error(strerror(errno))

enum { EV_READ, EV_STAT, EV_CLOSE, EV_CREATE, EV_DELETE, EV_WRITE, EV_MOVE, EV_MOVED_FROM, EV_MOVED_TO, EV_OPEN, NUM_EV };

typedef
	struct cinfo CINFO;
	
struct cinfo {
	struct inotify_event *iev;
	CINFO *prev;
};

typedef 
	struct {
		int fd;
		GB_HASHTABLE watches;
		CINFO *top;
	} 
	CINOTIFY;

typedef
	struct {
		CWATCH *root;
		char *path;
		int wd;
		int events[NUM_EV];
	}
	WATCH_LIST;
	
typedef
	struct {
		int *eventp;
		uint32_t mask;
	}
	EVENT_TABLE;

/* XXX: We currently only support one inotify instance */
static CINOTIFY _ino = {-1, NULL, NULL};

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

static EVENT_TABLE _event_table[] = 
{
	{&EVENT_Read,     IN_ACCESS},
	{&EVENT_Stat,     IN_ATTRIB},
	{&EVENT_Close,    IN_CLOSE_WRITE | IN_CLOSE_NOWRITE},
	{&EVENT_Create,   IN_CREATE},
	{&EVENT_Delete,   IN_DELETE | IN_DELETE_SELF},
	{&EVENT_Write,    IN_MODIFY},
	{&EVENT_Move,     IN_MOVE_SELF},
	{&EVENT_MoveFrom, IN_MOVED_FROM},
	{&EVENT_MoveTo,   IN_MOVED_TO},
	{&EVENT_Open,     IN_OPEN}
};

static bool destroy_watch(CWATCH *watch);
static void callback(int fd, int flags, CINOTIFY *ino);

static void init_inotify(void)
{
	if (_ino.fd >= 0)
		return;
	
#if DEBUG_ME
	fprintf(stderr, "init_inotify\n");
#endif
	
	_ino.fd = inotify_init();
	if (_ino.fd == -1) {
		GB_ErrorErrno();
		return;
	}
	GB.HashTable.New(&_ino.watches, GB_COMP_BINARY);
	GB.Watch(_ino.fd, GB_WATCH_READ, callback, (intptr_t) &_ino);
	_ino.top = NULL;
}

static void destroy_watch_list(WATCH_LIST *list)
{
	while (!destroy_watch(list->root));
}

static void exit_inotify(void)
{
	WATCH_LIST *list;
	int fd = _ino.fd; // prevent a recursion
	
	if (_ino.fd < 0)
		return;
	
	_ino.fd = -1;
	
#if DEBUG_ME
	fprintf(stderr, "exit_inotify\n");
#endif
	
	while (!GB.HashTable.First(_ino.watches, (void **)&list))
		destroy_watch_list(list);
	
	GB.Watch(fd, GB_WATCH_NONE, NULL, 0);
	close(fd);
	GB.HashTable.Free(&_ino.watches);
}

static WATCH_LIST *find_watch_list(CINOTIFY *ino, int wd)
{
	WATCH_LIST *list = NULL;
	GB.HashTable.Get(ino->watches, (char *)&wd, sizeof(wd), (void **)&list);
	return list;
}

static WATCH_LIST *find_watch_list_from_path(CINOTIFY *ino, const char *path, int len, bool create)
{
	WATCH_LIST *list = NULL;
	GB.HashTable.Get(ino->watches, path, len, (void **)&list);
	
	if (!list && create)
	{
#if DEBUG_ME
	fprintf(stderr, "find_watch_list_from_path: create watch list for %.*s\n", len, path);
#endif
		GB.AllocZero(POINTER(&list), sizeof(WATCH_LIST));
		list->wd = -1;
		list->path = GB.NewString(path, len);
		GB.HashTable.Add(ino->watches, path, len, list);
	}
	
	return list;
}

static void update_watch_list(WATCH_LIST *list)
{
	int i;
	uint32_t mask = 0;
	
	for (i = 0; i < NUM_EV; i++)
	{
		if (list->events[i])
			mask |= _event_table[i].mask;
	}
	
	if (mask == 0)
	{
		if (list->wd >= 0)
		{
#if DEBUG_ME
			fprintf(stderr, "update_watch_list: remove watch %d\n", list->wd);
#endif
			GB.HashTable.Remove(_ino.watches, (char *)&list->wd, sizeof(list->wd));
			inotify_rm_watch(_ino.fd, list->wd);
			list->wd = -1;
		}
	}
	else
	{
		int wd = inotify_add_watch(_ino.fd, list->path, mask);
		
		if (wd >= 0 && list->wd != wd)
		{
#if DEBUG_ME
		fprintf(stderr, "update_watch_list: add watch %d %08X\n", wd, mask);
#endif
			list->wd = wd;
			GB.HashTable.Add(_ino.watches, (char *)&wd, sizeof(wd), list);
		}
	}
}

static void create_watch(CWATCH *watch, const char *path, int len)
{
	int i;
	WATCH_LIST *list = find_watch_list_from_path(&_ino, path, len, TRUE);
	
	watch->root = list;
	
#if DEBUG_ME
	fprintf(stderr, "create_watch: %p %.*s\n", watch, len, path);
#endif
		
	LIST_insert(&list->root, watch, &watch->list);

	for (i = 0; i < NUM_EV; i++)
	{
		if (watch->events & (1 << i))
			list->events[i]++;
	}
	
	update_watch_list(list);
}

static bool destroy_watch(CWATCH *watch)
{
	int i;
	WATCH_LIST *list = (WATCH_LIST *)watch->root;

	if (!list)
		return FALSE;
	
	watch->root = NULL;
	
#if DEBUG_ME
	fprintf(stderr, "destroy_watch: %p %s\n", watch, list->path);
#endif
		
	GB.StoreVariant(NULL, &watch->tag);
	LIST_remove(&list->root, watch, &watch->list);
	
	for (i = 0; i < NUM_EV; i++)
	{
		if (watch->events & (1 << i))
			list->events[i]--;
	}
	
	update_watch_list(list);
	
	if (list->root == NULL)
	{
		GB.HashTable.Remove(_ino.watches, list->path, GB.StringLength(list->path));
		
#if DEBUG_ME
		fprintf(stderr, "destroy_watch: remove watch list: %s -> %d\n", list->path, GB.HashTable.Count(_ino.watches));
#endif
		
		GB.FreeString(&list->path);
		GB.Free(POINTER(&list));
		
		if (GB.HashTable.Count(_ino.watches) == 0)
			exit_inotify();
		
		return TRUE;
	}
	else
		return FALSE;
	
}

static void pause_watch(CWATCH *watch)
{
	int i;
	WATCH_LIST *list = (WATCH_LIST *)watch->root;

	if (watch->paused)
		return;
	
	watch->paused = TRUE;
	watch->save_events = watch->events;

	for (i = 0; i < NUM_EV; i++)
	{
		if (watch->events & (1 << i))
			list->events[i]--;
	}
	
	watch->events = 0;
	
	update_watch_list(list);
}

static void resume_watch(CWATCH *watch)
{
	int i;
	WATCH_LIST *list = (WATCH_LIST *)watch->root;

	if (!watch->paused)
		return;
	
	watch->paused = FALSE;
	watch->events = watch->save_events;

	for (i = 0; i < NUM_EV; i++)
	{
		if (watch->events & (1 << i))
			list->events[i]++;
	}
	
	watch->save_events = 0;
	
	update_watch_list(list);
}

static void callback(int fd, int flags, CINOTIFY *ino)
{
	struct inotify_event *iev;
	int i, j, length;
	int event;
	CINFO info;
	WATCH_LIST *list;
	CWATCH *watch;
	uint32_t mask;
	char buf[sizeof(*iev) + NAME_MAX + 1] __attribute__((aligned(sizeof(int))));

	for(;;)
	{
		if ((length = read(fd, buf, sizeof(buf))) <= 0) 
		{
			if (errno == EINTR)
				continue;
			
			GB_ErrorErrno();
			return;
		}
		
		break;
	}

	for (i = 0; i < length; i += sizeof(*iev) + iev->len) 
	{

		iev = (struct inotify_event *) &buf[i];

		list = find_watch_list(ino, iev->wd);
		
		if (!list && !(iev->mask & IN_Q_OVERFLOW)) {
			if (getenv("GB_INOTIFY_DEBUG"))
				fprintf(stderr, "gb.inotify: descriptor %d not known. Name was `%s'\n", iev->wd, iev->name);
			continue;
		}

		mask = iev->mask & ~(IN_IGNORED | IN_ISDIR | IN_Q_OVERFLOW | IN_UNMOUNT);
		
		LIST_for_each(watch, list->root)
		{
			if (watch->paused)
				continue;
			
			for (j = 0; j < NUM_EV; j++)
			{
				event = *_event_table[j].eventp;
				if (_event_table[j].mask & mask && GB.CanRaise(watch, event))
				{
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

					GB.Raise(watch, event, 0);

					_ino.top = info.prev;
				}
			}
		}
		
		/* Also remove the watch if the kernel did. */
		/*if (iev->mask & IN_IGNORED)
			destroy_watch(watch);*/
	}
}

/**G
 * Tidy up.
 **/
BEGIN_METHOD_VOID(Watch_exit)

	/* Free the remaining objects. destroy_watch() will then also take
	 * care of calling INOTIFY_exit() to free bookkeeping data. */
	exit_inotify();

END_METHOD

#if 0
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
#endif

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
BEGIN_METHOD(Watch_new, GB_STRING path; GB_BOOLEAN nofollow; GB_INTEGER events)

	int i;
	ushort events = VARGOPT(events, 0);

	if (LENGTH(path) == 0)
	{
		GB.Error("Null path");
		return;
	}
	
	//fprintf(stderr, "Watch_new: %p\n", THIS);
	
	/* If this is the first watch, we need an inotify instance first.
	 * We don't use the component's init function to set up _ino because
	 * the inotify instance needs a GB.Watch() which would keep the
	 * program open even if no watches were registered. So this is done
	 * ad-hoc if there are watches. */
	init_inotify();

	/* Get the mask */
	if (!events) 
	{
		for (i = 0; i < NUM_EV; i++) 
		{
			if (GB.CanRaise(THIS, *_event_table[i].eventp))
				events |= (1 << i); //_event_table[i].mask;
		}
		/* But we need at least to watch one event. IN_DELETE_SELF
		 * seems a good one as it doesn't trigger too often :-) */
		//mask |= IN_DELETE_SELF;
	}

	THIS->events = events;
	THIS->nofollow = VARGOPT(nofollow, FALSE);
	THIS->tag.type = GB_T_NULL;

	create_watch(THIS, STRING(path), LENGTH(path));

END_METHOD

/**G
 * Deallocate the watch.
 **/
BEGIN_METHOD_VOID(Watch_free)

	//fprintf(stderr, "Watch_free: %p\n", THIS);
	
	destroy_watch(THIS);

END_METHOD

BEGIN_METHOD_VOID(Watch_Resume)

	resume_watch(THIS);

END_METHOD

BEGIN_METHOD_VOID(Watch_Pause)

	pause_watch(THIS);

END_METHOD

/**G
 * Return the watched path.
 **/
BEGIN_PROPERTY(Watch_Path)

	GB.ReturnString(((WATCH_LIST *)THIS->root)->path);

END_PROPERTY

/**G
 * Return or set if the watch is paused.
 *
 * {seealso
 *   Pause(), Resume()
 * }
 **/
BEGIN_PROPERTY(Watch_IsPaused)

	if (READ_PROPERTY)
		GB.ReturnBoolean(THIS->paused);
	else if (THIS->paused != VPROP(GB_BOOLEAN))
	{
		if (VPROP(GB_BOOLEAN))
			pause_watch(THIS);
		else
			resume_watch(THIS);
	}

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

GB_DESC CWatch[] = 
{
	GB_DECLARE("Watch", sizeof(CWATCH)),

	GB_CONSTANT("Read", "i", 1 << EV_READ),
	GB_CONSTANT("Stat", "i", 1 << EV_STAT),
	GB_CONSTANT("Close", "i", 1 << EV_CLOSE),
	GB_CONSTANT("Create", "i", 1 << EV_CREATE),
	GB_CONSTANT("Delete", "i", 1 << EV_DELETE),
	GB_CONSTANT("Write", "i", 1 << EV_WRITE),
	GB_CONSTANT("Move", "i", 1 << EV_MOVE),
	GB_CONSTANT("MoveFrom", "i", 1 << EV_MOVED_FROM),
	GB_CONSTANT("MoveTo", "i", 1 << EV_MOVED_TO),
	GB_CONSTANT("Open", "i", 1 << EV_OPEN),
	GB_CONSTANT("All", "i", -1),

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
	//GB_STATIC_METHOD("_get", "Watch", Watch_get, "(Path)s"),

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

/**G
 * Return if the given flag is set. You can also combine multiple flags to
 * check if at least one of them is set.
 **/
BEGIN_METHOD(WatchEvents_get, GB_INTEGER events)

	GB.ReturnBoolean(!!((THIS->paused ? THIS->save_events : THIS->events) & VARG(events)));

END_METHOD

#define update_events(obj_events_p, events, value)	\
do {							\
	if (value)					\
		*(obj_events_p) |=  (events);		\
	else						\
		*(obj_events_p) &= ~(events);		\
} while (0)

/**G
 * Set or clear a flag. You can combine multiple flags to set or clear them
 * en masse.
 **/
BEGIN_METHOD(WatchEvents_put, GB_BOOLEAN value; GB_INTEGER events)

	WATCH_LIST *list = (WATCH_LIST *)THIS->root;
	int i;
	int events = VARG(events);
	int value = VARG(value);

	if (!THIS->paused)
	{
		for (i = 0; i < NUM_EV; i++)
		{
			/* events is the bitmask of events to operate on,
			 * value is what to set all of them to. */
			if ((events & (1 << i)) == 0)
				continue;
			/* Don't update counts unless the value
			 * actually changed. */
			if (!!value == !!(THIS->events & (1 << i)))
				continue;

			if (value)
				list->events[i]++;
			else
				list->events[i]--;
		}

		update_events(&THIS->events, events, value);
		update_watch_list(list);
	}
	else
		update_events(&THIS->save_events, events, value);

END_METHOD

GB_DESC CWatchEvents[] =
{
	GB_DECLARE_VIRTUAL(".Watch.Events"),

	GB_METHOD("_get", "b", WatchEvents_get, "(Events)i"),
	GB_METHOD("_put", NULL, WatchEvents_put, "(Value)b(Events)i"),

	GB_END_DECLARE
};
