/************************************************************************
 *   IRC - Internet Relay Chat, include/patchlevel.h
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* $Id$ */

#ifndef __patchlevel_header__
#define __patchlevel_header__

/* these don't go in the version itself, but they're importany anyways */
#define RELEASE 1
#define BETA    2
#define CURRENT 3

#define BRANCHSTATUS RELEASE

#define BASENAME "solid-ircd"
#define MAJOR 3
#define MINOR 4
#define PATCH 8

#define PATCHES "stable+rz2e-cho7"

void build_version(void);

#endif

