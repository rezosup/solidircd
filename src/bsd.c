/************************************************************************
 *   IRC - Internet Relay Chat, src/bsd.c
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
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

#include "struct.h"
#include "common.h"
#include "sys.h"
#include "h.h"
#include "fds.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

extern int  errno;  /* ...seems that errno.h doesn't define this everywhere */
#ifndef SYS_ERRLIST_DECLARED
extern char *sys_errlist[];
#endif

#if defined(DEBUGMODE) || defined (DNS_DEBUG)
int writecalls = 0, writeb[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int readcalls = 0;
#endif
void dummy()
{
    struct sigaction act;
    
    act.sa_handler = dummy;
    act.sa_flags = 0;
    (void) sigemptyset(&act.sa_mask);
    (void) sigaddset(&act.sa_mask, SIGALRM);
    (void) sigaddset(&act.sa_mask, SIGPIPE);
#ifdef SIGWINCH
    (void) sigaddset(&act.sa_mask, SIGWINCH);
#endif
    (void) sigaction(SIGALRM, &act, (struct sigaction *) NULL);
    (void) sigaction(SIGPIPE, &act, (struct sigaction *) NULL);
#ifdef SIGWINCH
    (void) sigaction(SIGWINCH, &act, (struct sigaction *) NULL);
#endif
}

/*
 * deliver_it 
 *
 * Attempt to send a sequence of bytes to the connection. 
 * Returns 
 *  < 0     Some fatal error occurred, (but not EWOULDBLOCK). 
 *    his return is a request to close the socket and clean up the link.
 *  >= 0    No real error occurred, returns the number of bytes actually
 *    transferred. EWOULDBLOCK and other similar conditions should be mapped
 *    to zero return.
 *    Upper level routine will have to decide what to do with
 *    those unwritten bytes... 
 * *NOTE*  alarm calls have been preserved, so this should work equally 
 *  well whether blocking or non-blocking mode is used... 
 */
#ifdef WRITEV_IOV
int deliver_it(aClient *cptr, struct iovec *iov, int len)
#else
int deliver_it(aClient *cptr, char *str, int len)
#endif
{
    int         retval;
    aListener    *lptr = cptr->lstn;    
#ifdef  DEBUGMODE
    writecalls++;
#endif

/* Allright this one is one of my favorites heh we now support
 * multiple buffers as you can see here I changed the code to work with WRITEV_IOV
 * what we are doing here is checkin if the client is a ssl connection if the client is ssl
 * we will send a ssl check, if write_iov is enable we will send a different ssl check with iov
 * if the client is not ssl  then we will use the original writev if enable or the normal send code
 *   - Sheik 24/04/2005
 *
 */


#ifdef HAVE_SSL
  if (IsSSL(cptr) && cptr->ssl)
#ifdef WRITEV_IOV
   retval = SEND_CHECK_SSL(cptr, iov, len);
#else
	retval = SEND_CHECK_SSL(cptr, str, len);
#endif
  else
#endif

#ifdef WRITEV_IOV
    retval = writev(cptr->fd, iov, len);
#else
    retval = send(cptr->fd, str, len, 0);
#endif
 
    /*
     * Convert WOULDBLOCK to a return of "0 bytes moved". This 
     * should occur only if socket was non-blocking. Note, that all is
     * Ok, if the 'write' just returns '0' instead of an error and
     * errno=EWOULDBLOCK. 
     */
    if (retval < 0 && (errno == EWOULDBLOCK || errno == EAGAIN ||
               errno == ENOBUFS))
    {
        retval = 0;
        cptr->flags |= FLAGS_BLOCKED;
        set_fd_flags(cptr->fd, FDF_WANTWRITE);
        return (retval);        /* Just get out now... */
    }
    else if (retval > 0)
    {
        if(cptr->flags & FLAGS_BLOCKED)
        {
            cptr->flags &= ~FLAGS_BLOCKED;
            unset_fd_flags(cptr->fd, FDF_WANTWRITE);
        }
    }
    
#ifdef DEBUGMODE
    if (retval < 0)
    {
        writeb[0]++;
        Debug((DEBUG_ERROR, "write error (%s) to %s",
           sys_errlist[errno], cptr->name));
    }
    else if (retval == 0)
        writeb[1]++;
    else if (retval < 16)
        writeb[2]++;
    else if (retval < 32)
        writeb[3]++;
    else if (retval < 64)
        writeb[4]++;
    else if (retval < 128)
        writeb[5]++;
    else if (retval < 256)
        writeb[6]++;
    else if (retval < 512)
        writeb[7]++;
    else if (retval < 1024)
        writeb[8]++;
    else
        writeb[9]++;
#endif
    if (retval > 0)
    {
        cptr->sendB += retval;
        if (cptr->sendB & 0x0400)
        {
            cptr->sendK += (cptr->sendB >> 10);
            cptr->sendB &= 0x03ff;  /* 2^10 = 1024, 3ff = 1023 */
        }
        me.sendB += retval;
        if (me.sendB & 0x0400)
        {
            me.sendK += (me.sendB >> 10);
            me.sendB &= 0x03ff;
        }

        if (lptr)
        {
            lptr->sendB += retval;
            if (lptr->sendB & 0x0400) 
            {
                lptr->sendK += (lptr->sendB >> 10);
                lptr->sendB &= 0x03ff;
            }
        }
    }
    return (retval);
}
