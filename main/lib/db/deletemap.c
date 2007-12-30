/***************************************************************************

  CResult.c

  The Result class

  (c) 2000-2005 Beno� Minisini <gambas@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 1, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

***************************************************************************/

#define __DELETEMAP_C

#include "gambas.h"
#include "main.h"
#include "deletemap.h"

//#define DEBUG_ME

typedef
  struct _DELETE_SLOT {
    struct _DELETE_SLOT *prev;
    struct _DELETE_SLOT *next;
    long start;
    long length;
    }
  DELETE_SLOT;

long DELETE_MAP_virtual_to_real(DELETE_MAP *dmap, long vpos)
{
  DELETE_SLOT *slot = (DELETE_SLOT *)dmap;
  long rpos = vpos;
  
  while (slot)
  {
    if (rpos < slot->start)
      break;
    rpos += slot->length;
    slot = slot->next;
  }
  
  #ifdef DEBUG_ME
  printf("DELETE_MAP_virtual_to_real: %ld => %ld\n", vpos, rpos);
  #endif
  
  return rpos;
}

long DELETE_MAP_real_to_virtual(DELETE_MAP *dmap, long rpos)
{
  DELETE_SLOT *slot = (DELETE_SLOT *)dmap;
  long vpos = rpos;
  
  while (slot)
  {
    if (rpos < slot->start)
      break;
    if (rpos < (slot->start + slot->length))
      return (-1);
    vpos -= slot->length;
    slot = slot->next;
  }
  
  #ifdef DEBUG_ME
  printf("DELETE_MAP_real_to_virtual: %ld => %ld\n", rpos, vpos);
  #endif
  
  return vpos;
}

#ifdef DEBUG_ME
static void dump(DELETE_MAP *dmap)
{
  DELETE_SLOT *slot = (DELETE_SLOT *)dmap;
  
  printf("dumping map %p\n", dmap);
  while (slot)
  {
    printf("[ %ld %ld ]\n", slot->start, slot->length);
    slot = slot->next;
  }
  printf("\n");
}
#endif

static void create_slot(DELETE_SLOT **pslot, long pos, DELETE_SLOT *before, DELETE_SLOT *after)
{
  GB.Alloc((void **)pslot, sizeof(DELETE_SLOT));
  (*pslot)->prev = before;
  (*pslot)->next = after;
  (*pslot)->start = pos;
  (*pslot)->length = 1;
  
  if (before)
    before->next = *pslot;
  if (after)
    after->prev = *pslot;
}

static DELETE_SLOT *delete_slot(DELETE_SLOT *slot)
{
  DELETE_SLOT *nslot;
  
  nslot = slot->next;
  
  if (slot->prev)
    slot->prev->next = slot->next;
  if (slot->next)
    slot->next->prev = slot->prev;
    
  GB.Free((void **)&slot);
  
  return nslot;
}

void DELETE_MAP_add(DELETE_MAP **dmap, long vpos)
{
  DELETE_SLOT *slot;
  DELETE_SLOT *nslot;
  DELETE_SLOT *bslot = NULL;
  long rpos;
  
  if (vpos < 0)
    return;
  
  rpos = DELETE_MAP_virtual_to_real(*dmap, vpos);
  
  for (slot = (DELETE_SLOT *)*dmap; slot; slot = slot->next)
  {
    if (rpos < slot->start)
      break;
    bslot = slot;
  }
  
  create_slot(&nslot, rpos, bslot, slot);
  if ((DELETE_SLOT *)*dmap == slot)
    *dmap = (DELETE_MAP *)nslot;
  
  slot = nslot;
  if (slot->prev)
    slot = slot->prev;
    
  while (slot->next)
  {
    if ((slot->start + slot->length) == (slot->next->start))
    {
      nslot = slot->next;
      slot->length += nslot->length;
      delete_slot(nslot);
    }
    else
      slot = slot->next;
  }
  
  #ifdef DEBUG_ME
  printf("DELETE_MAP_add: %ld\n", vpos);
  dump(*dmap);
  #endif
}

void DELETE_MAP_free(DELETE_MAP **dmap)
{
  DELETE_SLOT *slot;
  
  slot = (DELETE_SLOT *)*dmap;
  while (slot)
    slot = delete_slot(slot);
    
  *dmap = NULL;
}

