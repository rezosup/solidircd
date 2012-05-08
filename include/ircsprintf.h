/*
 *  bahamut ircd - an Internet Relay Chat Daemon, include/ircsprintf.h
 *
 *  Copyright (C) 1990-2005 by the past and present ircd coders, and others.
 *  Refer to the documentation within doc/authors/ for full credits and copyrights.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *  $Id$
 *
 */

#ifndef IRCSPRINTF_H
#define IRCSPRINTF_H
#include <stdarg.h>
#include <stdio.h>
#include "setup.h"

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

/* define this if you intend to use ircsnprintf or ircvsnprintf */
/* It's not used, and sNprintf functions are not in all libraries */
#define WANT_SNPRINTF

int ircsprintf(char *str, const char *format, ...);
int ircvsprintf(char *str, const char *format, va_list ap);
#ifdef WANT_SNPRINTF
int ircvsnprintf(char *str, size_t size, const char *format, va_list ap);
int ircsnprintf(char *str, size_t size, const char *format, ...);
#endif
/* This code contributed by Rossi 'vejeta' Marcello <vjt@users.sourceforge.net>
 * Originally in va_copy.h, however there wasnt much there, so i stuck it in
 * here.  Thanks Rossi!  -epi
 */

/* va_copy hooks for IRCd */

#if defined(__powerpc__)
# if defined(__NetBSD__)
#  define VA_COPY va_copy
# elif defined(__FreeBSD__) || defined(__linux__)
#  define VA_COPY __va_copy
# endif
#elif defined (__x86_64)
# define VA_COPY __va_copy
#else
# define VA_COPY(x, y) x = y
#endif

#endif


