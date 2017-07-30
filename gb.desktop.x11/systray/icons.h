/* -------------------------------
 * vim:tabstop=4:shiftwidth=4
 * icons.h
 * Tue, 24 Aug 2004 12:05:38 +0700
 * -------------------------------
 * Manipulations with the list of
 * tray icons
 * -------------------------------*/

#ifndef _ICONS_H_
#define _ICONS_H_

#include "systray.h"
#include <X11/X.h>
#include <X11/Xmd.h>

/* Simple point & rect data structures */
struct Point { int x, y; };
struct Rect { int x, y, w, h; };

/* Tray icon layout data structure */
struct Layout {
	struct Rect grd_rect;		/* The rect in the grid */
	struct Rect icn_rect;		/* Real position inside the tray */
	struct Point wnd_sz;		/* Size of the window of the icon */
};

/* Tray icon data structure */
struct TrayIcon {
	GB_BASE ob;
	struct TrayIcon *next;
	struct TrayIcon *prev;
	Window wid; 				/* Window ID */
	//Window mid_parent; 			/* Mid-parent ID */
	int x, y, w, h;
	int iw, ih;
	int cmode; 					/* Compatibility mode: CM_FDO/CM_KDE (see embed.h) */
	int num_size_resets;        /* How many times size was reset */
	unsigned long xembed_data[2];/* XEMBED data */
	long xembed_last_timestamp; /* The timestamp of last processed xembed message */
	long xembed_last_msgid; 	/* ID of the last processed xembed message */
	struct Layout l;	 		/* Layout info */
	unsigned is_embedded : 1;			/* Flag: is the icon succesfully embedded ? */
	unsigned is_invalid : 1;				/* Flag: is the icon invalid ? */
	unsigned is_visible : 1;    			/* Flag: is the icon hidden ? */
	unsigned is_resized : 1;				/* Flag: the icon has recently resized itself */
	unsigned is_layed_out : 1;			/* Flag: the icon is succesfully layed out */
	unsigned is_updated : 1;				/* Flag: the position of the icon needs to be updated */
	unsigned is_xembed_supported : 1;	/* Flag: does the icon support xembed */
	unsigned is_size_set : 1;			/* Flag: has the size for the icon been set */
	unsigned is_xembed_accepts_focus : 1;/* Flag: does the icon want focus */
	unsigned is_destroyed : 1; /* If a DestroyNotify has been received */
	unsigned invalid : 1;
};

/* Typedef for comparison function */
typedef int (*IconCmpFunc) (struct TrayIcon *, struct TrayIcon *); 

/* Typedef for callback function */
typedef int (*IconCallbackFunc) (struct TrayIcon *);

/* Add the new icon to the list */
struct TrayIcon *icon_list_new(Window w, int cmode);

/* Delete the icon from the list */
int icon_list_free(struct TrayIcon *ti);

/* Return the next/previous icon in the list after the icon specified by ti */
struct TrayIcon *icon_list_next(struct TrayIcon *ti);
struct TrayIcon *icon_list_prev(struct TrayIcon *ti);

/*************************************************
 * BIG FAT  WARNING: backup/restore routines  will 
 * memleak/fail  if  the  number  of icons in  the 
 * list has changed between  backup/restore calls.
 * (in return, it does not invalidate pointers :P)
 *************************************************/

/* Back up the list */
int icon_list_backup();

/* Restore the list from the backup */
int icon_list_restore();

/* Free the back-up list */
int icon_list_backup_purge();

/* Apply a callback specified by cbk to all icons.
 * List is traversed in a natural order. Function stops
 * and returns current_icon if cbk(current_icon) == MATCH */
struct TrayIcon *icon_list_forall(IconCallbackFunc cbk);
/* For readability sake, we sometimes use this function */
#define icon_list_advanced_find icon_list_forall

/* Same as above, but start traversal from the icon specified by tgt */
struct TrayIcon *icon_list_forall_from(struct TrayIcon *tgt, IconCallbackFunc cbk);

/* Clear the whole list, calling cbk for each icon */
void icon_list_clean(IconCallbackFunc cbk);

/* Sort the list using comparison function specified by cmp. 
 * Memo for writing comparison functions:
 * if a < b => cmp(a,b) < 0
 * if a = b => cmp(a,b) = 0
 * if a > b => cmp(a,b) > 0 */
void icon_list_sort(IconCmpFunc cmp);

/* Find the icon with wid == w */
struct TrayIcon *icon_list_find(Window w);

/* Find the icon with wid == w or parent wid == w */
struct TrayIcon *icon_list_find_ex(Window w);

int icon_get_count(void);
struct TrayIcon *icon_get(int i);

#endif
