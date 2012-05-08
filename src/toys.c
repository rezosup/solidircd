/*
 * Solid Internet Relay Chat daemon, src/toys.c
 * Copyright (C) 2004 Juan L. BAez
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

 /*****************************************************************
  * $Id$
  * $Name:  $ 
  * $Source: /cvsroot/solidircd/solidircd-stable/src/toys.c,v $
  * $State: Exp $
  *****************************************************************
  */


#include "sys.h"
#include "struct.h"
#include "common.h"
#include "msg.h"
#include "h.h"
#include "numeric.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#undef CHECK

#define STATES_NUM 512

typedef struct {
	short next_state[128];
	char *replacement;
} STATE;

static STATE state[STATES_NUM];
static int next_free_state;

char *LowerAll(char *str)
{
  char *s;

  for(s = str; *s; s++)
    *s = ToLower(*s);

  return str;
}

/*****************************************
* Start whee routines
*****************************************/

char *add_punctuation(char *old_msg, char *new_msg)
{
        int i, len;

        len = strlen(new_msg);
        
        for (i=0; old_msg[i]; i++) {

                switch(old_msg[i]) {

                        case '!':
                        case '?':
                        case '.':
                                new_msg[len++] = old_msg[i];
                                new_msg[len] = 0;
                                if (len == 256)
                                        break;
                }
        }
        return new_msg;
}

/*****************************************
* End whee routines
*****************************************/

/*****************************************
* Start ELMER routines
*****************************************/

void add_translate(char *old_text, char *new_text)
{
	int i, len, curr_state, curr_char;
	short *next_state_p;

        old_text = LowerAll(old_text);
	len = strlen(old_text);
	curr_state = 0;
 
	for (i=0; i<len; i++) {

		curr_char = old_text[i];
		next_state_p = &state[curr_state].next_state[curr_char];

		if (*next_state_p)
			curr_state = *next_state_p;
		else
			*next_state_p = curr_state = next_free_state++;
	}
	DupString(state[curr_state].replacement, new_text);
}

void encode_chef(char *old_text, char *new_text, int chars_left)

{
        char *old_frag, *last_replacement, *add_frag, *temp;
        int curr_state, old_frag_len, add_frag_len;
        int last_replacement_len=0;
        old_text = LowerAll(old_text);
        chars_left--;   /* Leave space for null terminator */
        
        temp = new_text;

        do {
                old_frag     = old_text;
                old_frag_len = 0;
                curr_state   = 0;
                last_replacement = 0;
                do {
                        curr_state = state[curr_state].next_state[*old_frag++ & 0x7f];
                        old_frag_len++;

                        if (state[curr_state].replacement) {
                                last_replacement = state[curr_state].replacement;
                                last_replacement_len = old_frag_len;
                        }

                } while (curr_state);

                if (last_replacement) {
                        add_frag = last_replacement;
                        add_frag_len = strlen(last_replacement);
                        old_text += last_replacement_len;
                } else {
                        add_frag = old_text++;
                        add_frag_len = 1;
                }

                if (add_frag_len > chars_left)
                        add_frag_len = chars_left;

                strncpy(new_text, add_frag, add_frag_len);
                new_text   += add_frag_len;
                chars_left -= add_frag_len;

                if (!chars_left)
                        break;

        } while (*old_text);

        *new_text = 0;
}

void init_chef(void)
{
	memset(state, 0, sizeof(state));
	next_free_state = 1;
#ifdef CHECK
	fprintf(stderr, "states used: %d\n", next_free_state);
#endif
}

/*****************************************
* End ELMER routines
*****************************************/

int m_elmer(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    aClient *acptr;

    if (!IsPrivileged(sptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
                   me.name, parv[0], "ELMER");
        return 0;
    }
    if ((acptr = find_client(parv[1], NULL))==NULL)
    {
        if (MyClient(sptr))
            sendto_one (sptr, err_str (ERR_NOSUCHNICK),me.name, sptr->name, parv[1]);
        return 0;
    }
    if (IsPrivileged(acptr)) /* Enough said. */
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    acptr->user->special = 2;
    if (IsServer(cptr))
    {

#ifdef MAGICWAND_ELMER

        sendto_one(acptr, ":%s NOTICE %s :*** You have now adquired a new vocabulary.", me.name, parv[1], parv[0]);
#endif
        sendto_serv_butone(cptr, ":%s ELMER :%s", me.name, parv[1]);
        return 0;
    }
    send_globops("%s set \2ELMER\2 on %s (%s@%s)", parv[0], 
        acptr->name, acptr->user->username, acptr->user->realhost);
    sendto_serv_butone(NULL, ":%s ELMER %s", me.name, parv[1]);
    return 0;
}

int m_silly(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    aClient *acptr;

    if (!IsPrivileged(sptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
                   me.name, parv[0], "SILLY");
        return 0;
    }
    if ((acptr = find_client(parv[1], NULL))==NULL)
    {
        if (MyClient(sptr))
            sendto_one (sptr, err_str (ERR_NOSUCHNICK),me.name, sptr->name, parv[1]);
        return 0;
    }
    if (IsPrivileged(acptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    acptr->user->special = 1;
    if (IsServer(cptr))
    {

#ifdef MAGICWAND_SILLY

        sendto_one(acptr, ":%s NOTICE %s :***  You have now adquired an new vocabulary. ", me.name, parv[1], parv[0]);
#endif

        sendto_serv_butone(cptr, ":%s SILLY %s", me.name, parv[1]);
        return 0;
    }
    send_globops("%s set \2SILLY\2 on %s (%s@%s)", parv[0], 
        acptr->name, acptr->user->username, acptr->user->realhost);
    sendto_serv_butone(NULL, ":%s SILLY %s", me.name, parv[1]);

    return 0;
}

int m_normal(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    aClient *acptr;

    if (!IsPrivileged(sptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
                   me.name, parv[0], "NORMAL");
        return 0;
    }
    if ((acptr = find_client(parv[1], NULL))==NULL)
    {
        if (MyClient(sptr))
            sendto_one (sptr, err_str (ERR_NOSUCHNICK),me.name, sptr->name, parv[1]);
        return 0;
    }
    if (IsPrivileged(acptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }
    acptr->user->special = 0;
    if (IsServer(cptr))
    {

#ifdef MAGICWAND_NORMAL

        sendto_one(acptr, ":%s NOTICE %s :*** You have been returned to normality.", me.name, parv[1], parv[0]);
#endif
        sendto_serv_butone(cptr, ":%s NORMAL %s", me.name, parv[1]);
        return 0;
    }
    send_globops("%s set \2NORMAL\2 on %s (%s@%s)", parv[0], 
        acptr->name, acptr->user->username, acptr->user->realhost);
    sendto_serv_butone(NULL, ":%s NORMAL %s", me.name, parv[1]);

    return 0;
}

/*****************************************
* End ELMER routines
*****************************************/

