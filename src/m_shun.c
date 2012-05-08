/************************************************************************
 *   Solid IRCd - Solid Internet Relay Chat Daemon, src/m_shun.c
 *   Copyright (C) 2004 Juan L. Baez
 *
 *   See file AUTHORS in IRC package for additional names of
 *   the programmers.
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

#include "sys.h"
#include "struct.h"
#include "numeric.h"
#include "h.h"
#include "find.h"
#include "userban.h"
#include "common.h"

extern char *smalldate(time_t);

static int isallnum(char *s)
{
    int i;

    for (i = 0;i < strlen(s) && s[i]!='\0';i++)
    {
        if (!IsDigit(s[i]))
            return 0;
    }
    return 1;
}

/*
 * m_shun
 * ----- client parameters -----
 * parv[0] = sender prefix 
 * parv[1] = duration (must be in seconds)
 * parv[2] = user@host or nickname
 * parv[3] = reason
 */

int m_shun(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    aClient *acptr;
    struct userBan *ban, *oban;
    char *string, *user, *host, *date, reason[TOPICLEN+1];
    char suser[USERLEN+1], shost[HOSTLEN+1], buffer[1024];
    int duration, i;
    time_t shun_time;

    if (!IsPrivileged(cptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }

    if (parc < 4)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
                   me.name, parv[0], "SHUN");
        return 0;
    }
    if (parv[1] <= 0 || !isallnum(parv[1]))
    {
        sendto_one(sptr, ":%s NOTICE %s :The duration of a \2SHUN\2 must be in seconds, and"
                         "it cant be less than or equal to 0.", me.name, parv[0]);
        return 0;
    }

    bzero(buffer,sizeof(buffer));
    duration = atoi(parv[1]);
    shun_time = (time_t) duration;
    string = parv[2];

    if ((host = strchr(string, '@')))
    {
        user = string;
        *(host++) = '\0';
    }
    else
    {
        user = "*";
        host = string;
    }
    strncpyzt(suser,user,USERLEN+1); user = suser;
    strncpyzt(shost,host,HOSTLEN+1); host = shost;
    strncpyzt(reason,parv[3],TOPICLEN+1);

    /* Shun's range is too big, kill it. */
    if (!match(user,"jkjlkajflkajf") && !match(host,"afls...afplsf"))
    {
        sendto_one(sptr, ":%s NOTICE %s :Can't Shun *@*", me.name, parv[0]);
        return 0;
    }

    if (!(ban = make_hostbased_ban(user, host)))
    {
        sendto_one(sptr, ":%s NOTICE %s :Malformed shun %s@%s", me.name, parv[0], user, host);
        return 0;
    }
    if ((oban = find_userban_exact(ban, 0)) && (oban->flags & UBAN_SHUN))
    {
        if (!IsServer(sptr))
            sendto_one(sptr, ":%s NOTICE %s :[%s@%s] already shunned for %s",
                   me.name, parv[0], user, host, oban->reason);
        userban_free(ban);
        return 0;
    }

    if (!IsServer(sptr))
    {
        date = smalldate((time_t) 0);
        ircsprintf(buffer, "%s (%s)", reason, date);
    }
    else
        ircsprintf(buffer, "%s", reason);

    ban->flags |= (UBAN_SHUN|UBAN_TEMPORARY);
    ban->reason = (char *) MyMalloc(strlen(buffer) + 1);
    strcpy(ban->reason, buffer);
    ban->timeset = timeofday;
    ban->duration = shun_time;

    if(MyClient(sptr) && user_match_ban(sptr, ban))
    {
        sendto_one(sptr, ":%s NOTICE %s :You attempted to add a shun [%s@%s]"
                         " which would affect yourself. Aborted.",
                   me.name, parv[0], user, host);
        userban_free(ban);
        return 0;
    }
    add_hostbased_userban(ban);

    for (i = 0; i <= highest_fd; i++)
    {
        if (!(acptr = local[i]) || IsMe(acptr) || IsLog(acptr))
            continue;

        if (IsPerson(acptr) && !IsShunned(acptr) && user_match_ban(acptr, ban))
        {
            sendto_ops(SHUN_NAME" active for %s",
                       get_client_name(acptr, FALSE));
            SetShun(acptr);
            i--;
        }
    }

    if (MyConnect(sptr)) {

#if 0
        sendto_one(target, ":%s NOTICE %s :*** %s waves a magic wand "
                                "and shuns you",me.name, parv[1], parv[0]);
#endif
        send_globops("Shun added for (%s@%s) by %s, expires in %d seconds (Reason: %s)", 
            user, host, parv[0], shun_time, buffer);
        sendto_serv_butone(NULL, ":%s SHUN %d %s@%s :%s", parv[0], shun_time, user, host, buffer);
    }
    else
        sendto_serv_butone(cptr, ":%s SHUN %d %s@%s :%s", parv[0], shun_time, user, host, buffer);

    return 0;
}

/*
 * m_unshun
 * ----- client parameters -----
 * parv[0] = sender prefix 
 * parv[1] = user@host or nickname
 */

int m_unshun(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    aClient *acptr;
    struct userBan *ban;
    char *string, *user, *host;
    int i;

    if (!IsPrivileged(cptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }

    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
                   me.name, parv[0], "UNSHUN");
        return 0;
    }

    string = parv[1];

    if ((host = strchr(string, '@')))
    {
        user = string;
        *(host++) = '\0';
    }
    else
    {
        user = "*";
        host = string;
    }

    if (!match(user,"jkjlkajflkajf") && !match(host,"afls...afplsf"))
    {
        sendto_one(sptr, ":%s NOTICE %s :Can't Unshun *@*", me.name, parv[0]);
        return 0;
    }

    if ((ban = make_hostbased_ban(user, host)))
    {
        struct userBan *oban;

        ban->flags |= UBAN_SHUN|UBAN_TEMPORARY;

        if ((oban = find_userban_exact(ban, UBAN_SHUN|UBAN_TEMPORARY)))
        {
            char tmp[512];

            host = get_userban_host(oban, tmp, 512);

            for (i = 0; i <= highest_fd; i++)
            {
                if (!(acptr = local[i]) || IsMe(acptr) || IsLog(acptr))
                    continue;

                if (IsPerson(acptr) && IsShunned(acptr) && user_match_ban(acptr, ban))
                {
                    ClearShun(acptr);
                    i--;
                }
            }
            remove_userban(oban);
            userban_free(oban);
            userban_free(ban);

            if (MyClient(sptr))
            {
                send_globops("Shun for [%s@%s] removed by %s.", user, host, parv[0]);
                sendto_serv_butone(NULL, ":%s UNSHUN %s@%s", parv[0], user, host);
            }
            else
                sendto_serv_butone(cptr, ":%s UNSHUN %s@%s", parv[0], user, host);

            

            return 0;
        }
        userban_free(ban);
    }
    if (MyClient(sptr))
    sendto_one(sptr, ":%s NOTICE %s :No shun found for [%s@%s]",
            me.name, parv[0], user, host);

    return 0;
}
