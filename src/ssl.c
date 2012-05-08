/************************************************************************
 *   IRC - Internet Relay Chat, src/ssl.c
 *   Copyright (C) 2002 Barnaba Marcello <vjt@azzurra.org>
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
 *   
 *   SSL functions . . .
 */


#define _GNU_SOURCE
#define __USE_GNU
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "sys.h"
#include "h.h"

#ifdef HAVE_SSL

#define SAFE_SSL_READ  1
#define SAFE_SSL_WRITE  2
#define SAFE_SSL_ACCEPT  3

#ifndef MIN
#   define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef SSIZE_MAX
#define SSIZE_MAX LONG_MAX
#endif

extern int errno;
SSL_CTX *CTX_Server;

#if  defined(__FreeBSD__) 

void *
mempcpy (void *to, const void *from, size_t size)
{
  memcpy(to, from, size);
  return (char *) to + size;
}

#endif


void init_ssl(void)
{
    SSL_load_error_strings(); 
    SSL_library_init();
	SSLeay_add_ssl_algorithms();

    if (!(CTX_Server = SSL_CTX_new(SSLv23_server_method())))
    {
        ERR_print_errors_fp(stderr);
         exit(-1); 
    }
    
    SSL_CTX_set_session_cache_mode(CTX_Server, SSL_SESS_CACHE_OFF);
   
    if (!SSL_CTX_use_certificate_file(CTX_Server, SSL_Certificate, SSL_FILETYPE_PEM))
    {
        fprintf(stderr, "Error loading SSL_Certificate: %s\n", SSL_Certificate);
		 ERR_print_errors_fp(stderr);
		 SSL_CTX_free(CTX_Server);
	  
         exit(-1); 
    }
    if (!SSL_CTX_use_PrivateKey_file(CTX_Server, SSL_Keyfile, SSL_FILETYPE_PEM))
    {
        fprintf(stderr, "Error loading SSL_Keyfile: %s\n", SSL_Keyfile);
		 ERR_print_errors_fp(stderr);
		 SSL_CTX_free(CTX_Server);
	   
         exit(-1);
    }
    if (!SSL_CTX_check_private_key(CTX_Server))
    {
        fprintf(stderr, "Error checking Private Key\n");
		SSL_CTX_free(CTX_Server);
	    
         exit(-1); 
    }
}



static void disable_ssl(int do_errors)
{
    if(do_errors)
    {
	char buf[256];
	unsigned long e;

	while((e = ERR_get_error()))
	{
	    ERR_error_string_n(e, buf, sizeof(buf) - 1);
	    sendto_realops("SSL ERROR: %s", buf);
	}
    }

    if(CTX_Server)
	SSL_CTX_free(CTX_Server);
    sendto_ops("Disabling SSL support due to unrecoverable SSL errors. /Rehash again to retry.");
    
    
    return;
}


static int fatal_ssl_error(int, int, aClient *);

int SSL_smart_shutdown(SSL *ssl)
{
    int i, rc;

    /* We want to repeat the calls to SSL_shutdown,
     * the reason being it "internally dispatches through
     * a little state machine." We also limit the number of calls
     * to avoid any hangs. */

    for (rc = 0, i = 0; i < 4 ; i++) {
        if ((rc = SSL_shutdown(ssl)))
            break;
    }

    return rc;
}


/*  rehash SSL code neeed to change it up a bit..  */

int rehash_ssl(void)
{
    if(CTX_Server)
	SSL_CTX_free(CTX_Server);

     if (!(CTX_Server = SSL_CTX_new(SSLv23_server_method())))
    {
	disable_ssl(1);
	return 0;
    }

     if (!SSL_CTX_use_certificate_file(CTX_Server, SSL_Certificate, SSL_FILETYPE_PEM))
    {
	disable_ssl(1);

	return 0;
    }

   if (!SSL_CTX_use_PrivateKey_file(CTX_Server, SSL_Keyfile, SSL_FILETYPE_PEM))
    {
	disable_ssl(1);

	return 0;
    }

    if (!SSL_CTX_check_private_key(CTX_Server)) 
    {
	sendto_realops("SSL ERROR: Server certificate does not match server key");
	disable_ssl(0);

	return 0;
    }

    return 1;
}



/* Modified writev from glibc-2.3.3/sysdeps/posix/writev.c / */

#ifdef WRITEV_IOV
int SSL_writev (SSL *ssl, const struct iovec *vector, int count)
{
    char *buffer;
    register char *bp;
    size_t bytes=0, to_copy;
    ssize_t bytes_written;
    int i;

    for (i = 0; i < count; ++i) {
        /* Check for ssize_t overflow.  */
        if (SSIZE_MAX - bytes < vector[i].iov_len) {
            return -1;
        }    
        bytes += vector[i].iov_len;
    }

    if ((buffer = (char *)MyMalloc(bytes))==NULL)
        return -1;

    /* Copy the data into BUFFER.  */
    to_copy = bytes;
    bp = buffer;
    for (i = 0; i < count; ++i)
    {
      size_t copy = MIN (vector[i].iov_len, to_copy);
      bp = mempcpy ((void *) bp, (void *) vector[i].iov_base, copy);

      to_copy -= copy;
      if (to_copy == 0)
    break;
    }

    bytes_written = SSL_write (ssl, buffer, bytes);

    free (buffer);

    return bytes_written;
}

#endif

int safe_SSL_accept(aClient *acptr, int fd) {

    int err;

    if(!(err = SSL_accept((SSL *)acptr->ssl))) {
        switch(err = SSL_get_error((SSL *)acptr->ssl, err)) {
            case SSL_ERROR_SYSCALL:
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
            {
                case SSL_ERROR_WANT_READ:
                case SSL_ERROR_WANT_WRITE:
                    return 1;
            }
            default:
                return fatal_ssl_error(err, SAFE_SSL_ACCEPT, acptr); 
        }
        /* NOTREACHED */
        return -1;
    }
    return 1;
}


int safe_SSL_read(struct Client *acptr, void *buf, int sz)
{
    int ssl_err, len = SSL_read(acptr->ssl, buf, sz);

    if (!len)
    {
        switch (ssl_err = SSL_get_error(acptr->ssl, len))
        {
            case SSL_ERROR_SYSCALL:
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
                {
                    case SSL_ERROR_WANT_READ:
                        errno = EWOULDBLOCK;
                    return 0;
                }
            case SSL_ERROR_SSL:
                if (errno == EAGAIN)
                    return 0;
            default:
                return fatal_ssl_error(ssl_err, SAFE_SSL_READ, acptr);
        }
    }
    return len;
}

int safe_SSL_write(aClient *acptr, const void *buf, int sz)
{
    int ssl_err, len = SSL_write(acptr->ssl, buf, sz);
    
    if (!len)
    {
        switch (ssl_err = SSL_get_error(acptr->ssl, len))
        {
            case SSL_ERROR_SYSCALL:
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
                {
                    case SSL_ERROR_WANT_WRITE:
                    case SSL_ERROR_WANT_READ:
                        errno = EWOULDBLOCK;
                    return 0;
                }
            case SSL_ERROR_SSL:
                if (errno == EAGAIN)
                    return 0;
            default:
                return fatal_ssl_error(ssl_err, SAFE_SSL_WRITE, acptr);
        }
    }
    return len;
}

#ifdef WRITEV_IOV
int safe_SSL_writev(aClient *acptr, const struct iovec *vector, int count)
{
    int ssl_err, len = SSL_writev(acptr->ssl, vector, count);
    
    if (!len)
    {
        switch (ssl_err = SSL_get_error(acptr->ssl, len))
        {
            case SSL_ERROR_SYSCALL:
                if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
                {
                    case SSL_ERROR_WANT_WRITE:
                    case SSL_ERROR_WANT_READ:
                        errno = EWOULDBLOCK;
                    return 0;
                }
            case SSL_ERROR_SSL:
                if (errno == EAGAIN)
                    return 0;
            default:
                return fatal_ssl_error(ssl_err, SAFE_SSL_WRITE, acptr);
        }
    }
    return len;
}

#endif
void SSL_set_nonblocking(SSL *s)
{
    BIO_set_nbio(SSL_get_rbio(s),1);  /* Get the pointer for the read channel */
    BIO_set_nbio(SSL_get_wbio(s),1);  /* Get the pointer for the read channel */
}









static int fatal_ssl_error(int ssl_error, int where, aClient *sptr)
{
    /* don`t alter errno */
    int errtmp = errno;
    char *errstr = strerror(errtmp);
    char *ssl_errstr, *ssl_func;

    switch(where) {
	case SAFE_SSL_READ:
	    ssl_func = "SSL_read()";
	    break;
	case SAFE_SSL_WRITE:
	    ssl_func = "SSL_write()";
	    break;
	case SAFE_SSL_ACCEPT:
	    ssl_func = "SSL_accept()";
	    break;
	default:
	    ssl_func = "undefined SSL func [this is a bug] report to ircd@solid-ircd.com";
    }

    switch(ssl_error) {
    	case SSL_ERROR_NONE:
	    ssl_errstr = "No error";
	    break;
	case SSL_ERROR_SSL:
	    ssl_errstr = "Internal OpenSSL error or protocol error";
	    break;
	case SSL_ERROR_WANT_READ:
	    ssl_errstr = "OpenSSL functions requested a read()";
	    break;
	case SSL_ERROR_WANT_WRITE:
	    ssl_errstr = "OpenSSL functions requested a write()";
	    break;
	case SSL_ERROR_WANT_X509_LOOKUP:
	    ssl_errstr = "OpenSSL requested a X509 lookup which didn`t arrive";
	    break;
	case SSL_ERROR_SYSCALL:
	    ssl_errstr = "Underlying syscall error";
	    break;
	case SSL_ERROR_ZERO_RETURN:
	    ssl_errstr = "Underlying socket operation returned zero";
	    break;
	case SSL_ERROR_WANT_CONNECT:
	    ssl_errstr = "OpenSSL functions wanted a connect()";
	    break;
	default:
	    ssl_errstr = "Unknown OpenSSL error (huh?)";
    }

    sendto_realops_lev(DEBUG_LEV, "%s to "
		"%s!%s@%s aborted with%serror (%s). [%s]", 
		ssl_func, *sptr->name ? sptr->name : "<unknown>",
		(sptr->user && sptr->user->username) ? sptr->user->
		username : "<unregistered>", sptr->sockhost,
		(errno > 0) ? " " : " no ", errstr, ssl_errstr);
#ifdef USE_SYSLOG
    syslog(LOG_ERR, "SSL error in %s: %s [%s]", ssl_func, errstr,
	    ssl_errstr);
#endif

    /* if we reply() something here, we might just trigger another
     * fatal_ssl_error() call and loop until a stack overflow... 
     * the client won`t get the ERROR : ... string, but this is
     * the only way to do it.
     * IRC protocol wasn`t SSL enabled .. --vjt
     */

    errno = errtmp ? errtmp : EIO; /* Stick a generic I/O error */
    sptr->sockerr = IRCERR_SSL;
    sptr->flags |= FLAGS_DEADSOCKET;
    return -1;
}

#endif
