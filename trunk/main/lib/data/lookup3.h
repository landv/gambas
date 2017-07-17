/*
 * lookup3.h
 *
 * These routines were originally written by Bob Jenkins. The original file
 * http://burtleburtle.net/bob/c/lookup3.c (10th Aug 2013) contained these
 * credits:
 *
 * ---
 * lookup3.c, by Bob Jenkins, May 2006, Public Domain.
 *
 * These are functions for producing 32-bit hashes for hash table lookup.
 * hashword(), hashlittle(), hashlittle2(), hashbig(), mix(), and final()
 * are externally useful functions.  Routines to test the hash are included
 * if SELF_TEST is defined.  You can use this free for any purpose.  It's in
 * the public domain.  It has no warranty.
 * ---
 *
 * This file is a (rigorously) modified version for Gambas.
 *
 * Copyright (C) 2013 Tobias Boege <tobias@gambas-buch.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef __LOOKUP3_H
#define __LOOKUP3_H

#include <stdint.h>

#define lookup3_size(n)		((uint32_t) 1 << (n))
#define lookup3_mask(n)		(lookup3_size(n) - 1)
#define lookup3_rot(x, k)	(((x) << (k)) | ((x) >> (32 - (k))))

/* lookup3_mix -- mix 3 32-bit values reversibly. */
#define lookup3_mix(a, b, c)			\
{						\
	a -= c;  a ^= lookup3_rot(c,  4);  c += b;	\
	b -= a;  b ^= lookup3_rot(a,  6);  a += c;	\
	c -= b;  c ^= lookup3_rot(b,  8);  b += a;	\
	a -= c;  a ^= lookup3_rot(c, 16);  c += b;	\
	b -= a;  b ^= lookup3_rot(a, 19);  a += c;	\
	c -= b;  c ^= lookup3_rot(b,  4);  b += a;	\
}

/* lookup3_final -- final mixing of 3 32-bit values (a,b,c) into c */
#define lookup3_final(a,b,c)		\
{					\
	c ^= b; c -= lookup3_rot(b, 14);	\
	a ^= c; a -= lookup3_rot(c, 11);	\
	b ^= a; b -= lookup3_rot(a, 25);	\
	c ^= b; c -= lookup3_rot(b, 16);	\
	a ^= c; a -= lookup3_rot(c,  4);	\
	b ^= a; b -= lookup3_rot(a, 14);	\
	c ^= b; c -= lookup3_rot(b, 24);	\
}

/*
 * __lookup3_64() -- Hashes an array of 32 bit values into a 64 bit value
 * @k     : the key, an array of uint32_t values
 * @length: the length of the key, in uint32_ts
 * @pc    : seed #1
 * @pb    : seed #2
 *
 * If 'pb' is zero then the result will be valid a 32 bit value.
 */
static inline uint64_t __lookup3_64(const uint32_t *k, size_t length,
				    uint32_t pc, uint32_t pb)
{
	uint32_t a, b, c;

	/* Set up the internal state */
	a = b = c = 0xdeadbeef + ((uint32_t) (length << 2)) + pc;
	c += pb;

	/* Handle most of the key */
	while (length > 3) {
		a += k[0];
		b += k[1];
		c += k[2];
		lookup3_mix(a, b, c);
		length -= 3;
		k += 3;
	}

	/* Handle the last 3 uint32_t's */
	switch(length) { /* All the case statements fall through */
	case 3:
		c += k[2];
	case 2:
		b += k[1];
	case 1:
		a += k[0];
		lookup3_final(a, b, c);
	case 0:     /* case 0: nothing left to add */
		break;
	}
	/* Report the result. Mirror the 32 bit version if 'pb' was zero. */
	return c + pb ? ((uint64_t) b) << 32 : 0;
}

/*
 * Arbitrary string hash wrappers. They pad the string with '\0' to a 32
 * bits boundary. So two (key,length) pairs ("a",1) and ("a\0",2) would
 * give identical hashes. Beware!
 */

static inline uint32_t lookup3_32(const char *k, size_t length,
				  uint32_t initval)
{
	size_t len = (length + 31) / 32;
	uint32_t key[len];

	bzero(key, len * sizeof(key[0]));
	memcpy(key, k, length);
	return __lookup3_64(key, len, initval, 0);
}

static inline uint64_t lookup3_64(const char *k, size_t length,
				  uint32_t pc, uint32_t pb)
{
	size_t len = (length + 31) / 32;
	uint32_t key[len];

	bzero(key, len * sizeof(key[0]));
	memcpy(key, k, length);
	return __lookup3_64(key, len, pc, pb);
}

#endif /* __LOOKUP3_H */
