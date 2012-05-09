/*
 *   glines.c - gline interface and storage
 *   Copyright (C) 2005 Trevor Talbot and
 *                      the DALnet coding team
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

/*
 * This is a simple K-Line journal implementation.  When a K-Line with a
 * duration greater than gline_MIN_STORE_TIME is added, it is written to the
 * journal file (.glines):
 *   + expireTS usermask hostmask reason
 * 
 * When a K-Line is manually removed, it also results in a journal entry:
 *   - usermask hostmask
 * 
 * This allows K-Lines to be saved across restarts and rehashes.
 *
 * To keep the journal from getting larger than it needs to be, it is
 * periodically compacted: all active K-Lines are dumped into a new file, which
 * then replaces the active journal.  This is done on startup as well as every
 * gline_STORE_COMPACT_THRESH journal entries.
 */
 

#include "struct.h"
#include "common.h"
#include "sys.h"
#include "h.h"

#include <fcntl.h>

#include "userban.h"
#include "numeric.h"

static int journal = -1;
static char journalfilename[512];
static int journalcount;

void glinestore_add(struct userBan *);
void glinestore_remove(struct userBan *);

/* ircd.c */
extern int forked;

/* s_misc.c */
extern char *smalldate(time_t);


/*
 * m_gline
 * Add a local user@host ban.
 *
 *    parv[0] = sender
 *    parv[1] = duration (optional)
 *    parv[2] = nick or user@host mask
 *    parv[3] = reason (optional)
 */
int
m_gline(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    char rbuf[512];
    char *target;
    char *user;
    char *host;
    char *reason = "<no reason>";
    int tgminutes = DEFAULT_GLINE_TIME;
    int tgseconds;
    long lval;
    struct userBan *ban;
    struct userBan *existing;

    if (!IsPrivileged(sptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }

    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, parv[0],
                   "gline");
        return 0;
    }

    lval = strtol(parv[1], &target, 10);
    if (*target != 0)
    {
        target = parv[1];
        if (parc > 2)
            reason = parv[2];
    }
    else
    {
        /* valid expiration time */
        tgminutes = lval;

        if (parc < 3)
        {
            sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, parv[0],
                       "Gline");
            return 0;
        }

        target = parv[2];

        if (parc > 3)
            reason = parv[3];
    }

    /* negative times, or times greater than a year, are permanent */
    if (tgminutes < 0 || tgminutes > (365 * 24 * 60))
        tgminutes = 0;
    tgseconds = tgminutes * 60;

    if ((host = strchr(target, '@')))
    {
        *host++ = 0;
        user = target;
    }
    else
    {
        user = "*";
        host = target;
    }

    if (!match(user, "akjhfkahfasfjd") &&
        !match(host, "ldksjfl.kss...kdjfd.jfklsjf"))
    {
        sendto_one(sptr, ":%s NOTICE %s :gline: %s@%s mask is too wide",
                   me.name, parv[0], user, host);
        return 0;
    }

    /*
     * XXX: nick target support to be re-added
     */

    if (!(ban = make_hostbased_ban(user, host)))
    {
        sendto_one(sptr, ":%s NOTICE %s :gline: invalid ban mask %s@%s",
                   me.name, parv[0], user, host);
        return 0;
    }

    ban->flags |= UBAN_GLINE;

    /* only looks for duplicate glines, not akills */
    if ((existing = find_userban_exact(ban, UBAN_GLINE)))
    {
        if (!IsServer(sptr))
        sendto_one(sptr, ":%s NOTICE %s :gline: %s@%s is already %s: %s",
                   me.name, parv[0], user, host, NETWORK_GLINE_NAME,
                   existing->reason ? existing->reason : "<no reason>");
        userban_free(ban);
        return 0;
    }

    if (MyClient(sptr) && user_match_ban(sptr, ban))
    {
        sendto_one(sptr, ":%s NOTICE %s :gline: %s@%s matches you, rejected",
                   me.name, parv[0], user, host);
        userban_free(ban);
        return 0;
    }

    if (!IsServer(sptr))
        ircsnprintf(rbuf, sizeof(rbuf), "%s (%s)", reason, smalldate(0));
    else
        ircsnprintf(rbuf, sizeof(rbuf), "%s", reason);

    ban->reason = MyMalloc(strlen(rbuf) + 1);
    strcpy(ban->reason, rbuf);

    if (tgseconds)
    {
        ban->flags |= UBAN_TEMPORARY;
        ban->timeset = NOW;
        ban->duration = tgseconds;
    }

    add_hostbased_userban(ban);

    if (!tgminutes || tgminutes >= GLINE_MIN_STORE_TIME)
        glinestore_add(ban);

    userban_sweep(ban);

    host = get_userban_host(ban, rbuf, sizeof(rbuf));

    sendto_serv_butone(MyConnect(sptr) ? NULL : cptr, ":%s GLINE %l %s@%s :%s", sptr->name, tgseconds, user, host, reason);

    if (tgminutes)

        sendto_realops("%s added temporary %d min. "NETWORK_GLINE_NAME" for"
                       " [%s@%s] [%s]", parv[0], tgminutes, user, host,
                       reason);
    else

        sendto_realops("%s added "NETWORK_GLINE_NAME" for [%s@%s] [%s]", parv[0],
                       user, host, reason);

    return 0;
}

/*
 * m_ungline
 * Remove a local user@host ban.
 *
 *     parv[0] = sender
 *     parv[1] = user@host mask
 */
int m_ungline(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
    char hbuf[512];
    char *user;
    char *host;
    struct userBan *ban;
    struct userBan *existing;

    if (!IsPrivileged(sptr))
    {
        sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
        return 0;
    }

    if (parc < 2)
    {
        sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS), me.name, parv[0],
                   "UNGline");
        return 0;
    }

    if ((host = strchr(parv[1], '@')))
    {
        *host++ = 0;
        user = parv[1];
    }
    else
    {
        user = "*";
        host = parv[1];
    }

    if (!(ban = make_hostbased_ban(user, host)))
    {
        sendto_one(sptr, ":%s NOTICE %s :UNGline: No such ban %s@%s", me.name,
                   parv[0], user, host);
        return 0;
    }

    ban->flags |= UBAN_GLINE;
    existing = find_userban_exact(ban, UBAN_GLINE);
    host = get_userban_host(ban, hbuf, sizeof(hbuf));
    userban_free(ban);

    if (!existing)
    {
        sendto_one(sptr, ":%s NOTICE %s :UNGLINE: No such ban %s@%s", me.name,
                   parv[0], user, host);
        return 0;
    }

    if (existing->flags & UBAN_CONF)
    {
        sendto_one(sptr, ":%s NOTICE %s :UNGLINE: %s@%s is specified in the"
                   " configuration file and cannot be removed online", me.name,
                   parv[0], user, host);
        return 0;
    }

    remove_userban(existing);
    glinestore_remove(existing);
    userban_free(existing);

    sendto_ops("%s has removed the G-Line for: [%s@%s]", sptr->name, user, host);
    sendto_serv_butone(MyConnect(sptr) ? NULL : cptr, ":%s UNGLINE %s@%s", sptr->name, user, host);
    return 0;
}


static void
gs_error(char *msg)
{
    if (!forked)
        puts(msg);
    else
        sendto_ops("%s", msg);
}

/*
 * Writes a G-Line to the appropriate file.
 */
void
gs_write(int f, char type, struct userBan *ub)
{
    char outbuf[1024];
    char cidr[4] = "";
    time_t expiretime = 0;
    char *user = "*";
    char *reason = "";
    char *host = ub->h;
    int len;

    /* userban.c */
    unsigned int netmask_to_cidr(unsigned int);

    if (ub->flags & UBAN_TEMPORARY)
        expiretime = ub->timeset + ub->duration;

    if (ub->u)
        user = ub->u;

    if (ub->reason)
        reason = ub->reason;

    if (ub->flags & (UBAN_CIDR4|UBAN_CIDR4BIG))
    {
        host = inetntoa((char *)&ub->cidr4ip);
        ircsprintf(cidr, "/%d", netmask_to_cidr(ntohl(ub->cidr4mask)));
    }

    if (type == '+')
        len = ircsprintf(outbuf, "%c %d %s %s%s %s\n", type, (int)expiretime,
                         user, host, cidr, reason);
    else
        len = ircsprintf(outbuf, "%c %s %s%s\n", type, user, host, cidr);

    write(f, outbuf, len);
}

/*
 * Parses a G-Line entry from a storage journal line.
 * Returns 0 on invalid input, 1 otherwise.
 */
static int
gs_read(char *s)
{
    char type;
    time_t duration = 0;
    char *user;
    char *host;
    char *reason = "";
    struct userBan *ban;
    struct userBan *existing;

    type = *s++;

    /* bad type */
    if (type != '+' && type != '-')
        return 0;

    /* malformed */
    if (*s++ != ' ')
        return 0;

    if (type == '+')
    {
        duration = strtol(s, &s, 0);
        if (duration)
        {
            /* already expired */
            if (NOW >= duration)
                return 1;

            duration -= NOW;
        }

        /* malformed */
        if (*s++ != ' ')
            return 0;
    }

    /* usermask */
    user = s;
    while (*s && *s != ' ')
        s++;

    /* malformed */
    if (*s != ' ')
        return 0;

    /* mark end of user mask */
    *s++ = 0;

    /* hostmask */
    host = s;
    while (*s && *s != ' ')
        s++;

    if (type == '+')
    {
        /* malformed */
        if (*s != ' ')
            return 0;

        /* mark end of host mask */
        *s++ = 0;

        /* reason is the only thing left */
        reason = s;
    }

    ban = make_hostbased_ban(user, host);
    if (!ban)
        return 0;

    ban->flags |= UBAN_GLINE;

    if (type == '+')
    {
        if (duration)
        {
            ban->flags |= UBAN_TEMPORARY;
            ban->timeset = NOW;
            ban->duration = duration;
        }

        if (*reason)
            DupString(ban->reason, reason);

        add_hostbased_userban(ban);
    }
    else
    {
        existing = find_userban_exact(ban, UBAN_GLINE|UBAN_CONF);
        userban_free(ban);

        /* add may have been skipped due to being expired, so not an error */
        if (!existing)
            return 1;

        remove_userban(existing);
        userban_free(existing);
    }

    return 1;
}

/*
 * Compact G-Line store: dump active glines to a new file and remove the
 * current journal.
 * Returns 1 on success, 0 on failure.
 */
int
glinestore_compact(void)
{
    char buf1[512];
    int newfile;

    /* userban.c */
    extern void gs_dumpglines(int);

    if (forked)
        sendto_ops_lev(DEBUG_LEV, "Compacting G-Line store...");
    journalcount = 0;

    /* open a compaction file to dump all active glines to */
    ircsnprintf(buf1, sizeof(buf1), "%s/.glines_c", runpath);
    newfile = open(buf1, O_WRONLY|O_CREAT|O_TRUNC, 0700);
    if (newfile < 0)
    {
        ircsnprintf(buf1, sizeof(buf1), "ERROR: Unable to create G-Line"
                    " compaction file .glines_c: %s",
                    strerror(errno));
        gs_error(buf1);
        return 0;
    }

    /* do the dump */
    gs_dumpglines(newfile);
    close(newfile);

    /* close active storage file, rename compaction file, and reopen */
    if (journal >= 0)
    {
        close(journal);
        journal = -1;
    }
    if (rename(buf1, journalfilename) < 0)
    {
        ircsnprintf(buf1, sizeof(buf1), "ERROR: Unable to rename G-Line"
                    " compaction file .glines_c to .glines: %s",
                    strerror(errno));
        gs_error(buf1);
        return 0;
    }
    journal = open(journalfilename, O_WRONLY|O_APPEND, 0700);
    if (journal < 0)
    {
        ircsnprintf(buf1, sizeof(buf1), "ERROR: Unable to reopen G-Line"
                    " storage file .glines: %s", strerror(errno));
        gs_error(buf1);
        return 0;
    }

    return 1;
}

/*
 * Add a G-Line to the active store.
 */
void
glinestore_add(struct userBan *ban)
{
    if (journal >= 0)
        gs_write(journal, '+', ban);

    if (++journalcount > GLINE_STORE_COMPACT_THRESH)
        glinestore_compact();
}

/*
 * Remove a G-Line from the active store.
 */
void
glinestore_remove(struct userBan *ban)
{
    if (journal >= 0)
        gs_write(journal, '-', ban);

    if (++journalcount > GLINE_STORE_COMPACT_THRESH)
        glinestore_compact();
}

/*
 * Initialize G-Line storage.  Pass 1 when glines don't need to be reloaded.
 * Returns 0 on failure, 1 otherwise.
 */
int
glinestore_init(int noreload)
{
    char buf1[1024];
    FILE *jf;

    ircsnprintf(journalfilename, sizeof(journalfilename), "%s/.glines", runpath);

    if (journal >= 0)
    {
        if (noreload)
            return 1;

        close(journal);
        journal = -1;
    }

    /* "a+" to create if it doesn't exist */
    jf = fopen(journalfilename, "a+");
    if (!jf)
    {
        ircsnprintf(buf1, sizeof(buf1), "ERROR: Unable to open G-Line storage"
                    " file .glines: %s", strerror(errno));
        gs_error(buf1);
        return 0;
    }
    rewind(jf);

    /* replay journal */
    while (fgets(buf1, sizeof(buf1), jf))
    {
        char *s = strchr(buf1, '\n');

        /* no newline, consider it malformed and stop here */
        if (!s)
            break;

        *s = 0;

        if (!gs_read(buf1))
            break;
    }

    fclose(jf);

    /* this will reopen the journal for appending */
    return glinestore_compact();
}

