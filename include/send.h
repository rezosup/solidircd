/************************************************************************
 *   IRC - Internet Relay Chat, include/h.h
 *   Copyright (C) 1992 Darren Reed
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
/*
 * "send.h". - Headers file.
 * 
 * all the send* functions are declared here.
 */

/* $Id$ */

#ifndef SEND_H
#define SEND_H

/* send all queued crap to aClient */
extern int  send_queued(aClient *);

#include <stdarg.h>
#include "fdlist.h"

extern void init_send();

extern void send_chatops(char *pattern, ...);
extern void send_globops(char *pattern, ...);
extern void send_operwall(aClient *, char *, char *);

extern void sendto_all_butone(aClient *one, aClient *from, char *pattern, ...);
extern void sendto_all_servmask(aClient *from, char *mask, char *pattern, ...);
extern void sendto_channel_butone(aClient *one, aClient *from, 
				  aChannel *chptr, char *pattern, ...);
extern void sendto_channel_remote_butone(aClient *one, aClient *from, 
				         aChannel *chptr, char *pattern, ...);
extern void sendto_channel_butserv(aChannel *chptr, aClient *from,
				   char *pattern, ...);
extern void sendto_channel_butserv_me(aChannel *chptr, aClient *from,
				      char *pattern, ...);
extern void sendto_channelflags_butone(aClient *, aClient *, aChannel *,
                                       int, char *, ...);
extern void sendto_common_channels(aClient *user, char *pattern, ...);
extern void send_quit_to_common_channels(aClient *from, char *reason);
extern void send_part_to_common_channels(aClient *from, char *reason);
extern void sendto_fdlist(fdlist *listp, char *pattern, ...);
extern void sendto_locops(char *pattern, ...);
extern void sendto_one(aClient *to, char *pattern, ...);
extern void sendto_alias(AliasInfo *ai, aClient *from, char *pattern, ...);
extern void sendto_ops(char *pattern, ...);
extern void sendto_ops_butone(aClient *one, aClient *from, char *pattern, ...);
extern void sendto_ops_lev(int lev, char *pattern, ...);
extern void sendto_prefix_one(aClient *to, aClient *from, char *pattern, ...);

extern void sendto_realops_lev(int lev, char *pattern, ...);
extern void sendto_realops(char *pattern, ...);
extern void sendto_non_noquit_servs_butone(aClient *one, char *pattern, ...);
extern void sendto_serv_butone(aClient *one, char *pattern, ...);
extern void sendto_serv_butone_super(aClient *one, int flag, char *pattern, ...);
extern void sendto_wallops_butone(aClient *one, aClient *from,
				  char *pattern, ...);
extern void sendto_gnotice(char *pattern, ...);
extern void sendto_conops(char *pattern, ...);

extern void ts_warn(char *pattern, ...);

extern void vsendto_fdlist(fdlist *listp, char *pattern, va_list vl);
extern void vsendto_one(aClient *to, char *pattern, va_list vl);
extern void vsendto_prefix_one(aClient *to, aClient *from,
			       char *pattern, va_list vl);
extern void vsendto_realops(char *pattern, va_list vl);

extern void flush_connections();
extern void dump_connections();
#endif
