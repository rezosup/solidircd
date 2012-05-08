/************************************************************************
 *   IRC - Internet Relay Chat, src/match.c
 *   Copyright (C) 1990 Jarkko Oikarinen
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

#if 0 /* this code is depricated, however it may still prove useful
         in the long run to 'show' it here */

#define MAX_CALLS 512
static int  calls = 0;

/*
 * *  Compare if a given string (name) matches the given *  mask (which
 * can contain wild cards: '*' - match any *  number of chars, '?' -
 * match any single character. *
 * 
 *      return  0, if match *           1, if no match
 */
/*
 * * _match() * Iterative matching function, rather than recursive. *
 * Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
 */

static int wdmatch(const char *string, const char *wild);

static int _match(char *mask, char *name)
{
    u_char *m = (u_char *) mask, *n = (u_char *) name;
    char       *ma = mask, *na = name;
    int         wild = 0, q = 0;

    while (1)
    {
	if (calls++ > MAX_CALLS)
	    return 1;
	if (*m == '*')
	{
	    while (*m == '*')
		m++;
	    wild = 1;
	    ma = (char *) m;
	    na = (char *) n;
	}

	if (!*m)
	{
	    if (!*n)
		return 0;
	    for (m--; (m > (u_char *) mask) && (*m == '?'); m--);
	    if ((*m == '*') && (m > (u_char *) mask) &&
		(m[-1] != '\\'))
		return 0;
	    if (!wild)
		return 1;
	    m = (u_char *) ma;
	    n = (u_char *) ++na;
	}
	else if (!*n)
	{
	    while (*m == '*')
		m++;
	    return (*m != 0);
	}
	if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?')))
	{
	    m++;
	    q = 1;
	}
	else
	    q = 0;

	if ((ToLower(*m) != ToLower(*n)) && ((*m != '?') || q))
	{
	    if (!wild)
		return 1;
	    m = (u_char *) ma;
	    n = (u_char *) ++na;
	}
	else
	{
	    if (*m)
		m++;
	    if (*n)
		n++;
	}
    }
}

int match(char *n, char *m)
{
    calls=0;
    return wdmatch(n,m);
}
#endif

/*
 * * collapse a pattern string into minimal components. * This
 * particular version is "in place", so that it changes the pattern *
 * which is to be reduced to a "minimal" size.
 */
char *collapse(char *pattern)
{
    char   *s = pattern, *s1, *t;
    
    if (BadPtr(pattern))
	return pattern;
    /* Collapse all \** into \*, \*[?]+\** into \*[?]+ */
    for (; *s; s++)
	if (*s == '\\')
	{
	    if (!*(s + 1))
		break;
	    else
		s++;
	}
	else if (*s == '*')
	{
	    if (*(t = s1 = s + 1) == '*')
		while (*t == '*')
		    t++;
	    else if (*t == '?')
		for (t++, s1++; *t == '*' || *t == '?'; t++)
		    if (*t == '?')
			*s1++ = *t;
	    while ((*s1++ = *t++));
	}
    return pattern;
}

unsigned char tolowertab[] =
{0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
 0x1e, 0x1f,
 ' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
 '*', '+', ',', '-', '.', '/',
 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
 ':', ';', '<', '=', '>', '?',
 '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
 't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^', '_',
 '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',
 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

unsigned char touppertab[] =
{0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
 0x1e, 0x1f,
 ' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
 '*', '+', ',', '-', '.', '/',
 '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
 ':', ';', '<', '=', '>', '?',
 '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', 0x5f,
 '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~',
 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff};

unsigned char char_atribs[] =
{
    /* 0-7 */
    CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL,
    /* 8-12*/
    CNTRL, CNTRL | SPACE, CNTRL | SPACE, CNTRL | SPACE, CNTRL | SPACE,
    /* 13-15 */
    CNTRL | SPACE, CNTRL, CNTRL,
    /* 16-23 */
    CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL,
    /* 24-31 */
    CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL, CNTRL,
    /* space */
    PRINT | SPACE,
    /* !"#$%&'( */
    PRINT, PRINT, PRINT, PRINT, PRINT, PRINT, PRINT, PRINT,
    /* )*+,-./ */
    PRINT, PRINT, PRINT, PRINT, PRINT, PRINT, PRINT,
    /* 0123 */
    PRINT | DIGIT, PRINT | DIGIT, PRINT | DIGIT, PRINT | DIGIT,
    /* 4567 */
    PRINT | DIGIT, PRINT | DIGIT, PRINT | DIGIT, PRINT | DIGIT,
    /* 89:; */
    PRINT | DIGIT, PRINT | DIGIT, PRINT, PRINT,
    /* <=>? */
    PRINT, PRINT, PRINT, PRINT,
    /* @ */
    PRINT,
    /* ABC */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* DEF */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* GHI */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* JKL */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* MNO */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* PQR */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* STU */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* VWX */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* YZ[ */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* \]^ */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* _` */
    PRINT, PRINT,
    /* abc */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* def */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* ghi */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* jkl */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* mno */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* pqr */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* stu */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* vwx */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* yz{ */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* \}~ */
    PRINT | ALPHA, PRINT | ALPHA, PRINT | ALPHA,
    /* del */
    0,
    /* 80-8f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 90-9f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* a0-af */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* b0-bf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* c0-cf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* d0-df */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* e0-ef */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* f0-ff */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* screw that lexocographic crap, nothing in ircd ever cares */
int mycmp(char *s1, char *s2)
{
    unsigned char *str1 = (unsigned char *) s1;
    unsigned char *str2 = (unsigned char *) s2;
    while (touppertab[*str1] == touppertab[*str2])
    {
	if (*str1 == '\0')
	    return 0;
	str1++;str2++;
    }
    return (-1);
}

int mycmp_diff(char *s1, char *s2)
{
    int ret;
    unsigned char *str1 = (unsigned char *) s1;
    unsigned char *str2 = (unsigned char *) s2;
    while ((ret = touppertab[*str1] - touppertab[*str2]) == 0)
    {
	if (*str1 == '\0')
	    return 0;
	str1++;str2++;
    }
    return ret;
}

int myncmp(char *str1, char *str2, int n)
{
    unsigned char *s1 = (unsigned char *) str1;
    unsigned char *s2 = (unsigned char *) str2;
    while (touppertab[*s1] == touppertab[*s2])
    {
	s1++;
	s2++;
	n--;
	if (n == 0 || (*s1 == '\0' && *s2 == '\0'))
	    return 0;
    }
    return (-1);
}

#define MAX_CALLS 512
static int  calls = 0;

static inline int _match(char *wild, char *string);

int match(char *n, char *m)
{
    calls=0;
    return _match(n,m);
}

static inline int _match(char *wild, char *string)
{
    if (calls++ > MAX_CALLS)
	return 1;
    
    /* the *! in a match is such a common case that we optimize
     * for it inherently */

    if(wild[0]=='*' && wild[1]=='!')
    {
	wild+=2;
	while(*string!='!' && *string)
	    string++;
	if(!*string)
	    return 1;
	string++;
    }
    /* we also optimize further for *!*@...which is also very
     * common...*/
    if(wild[0]=='*' && wild[1]=='@')
    {
	wild+=2;
	while(*string!='@' && *string)
	    string++;
	if(!*string)
	    return 1;
	string++;
    }
    while(1)
    {
	/*
	 * Logic: Match * in a string, this is confusing, sort of
	 * if * is the last thing in the wildcard, the match is
	 * definite if we've gotten this far.
	 * otherwise we try and find every occurance of the
	 * the next character in the wildcard within the string
	 * and match from there, calling the function recursively
	 * until some level below us returns positive, in which
	 * case we too return positive.  For strings with lots
	 * of wildcards this gets disgustingly recursive.
	 */
	if(!*wild)
	    return ((!*string) ? 0 : 1);
	if(*wild=='*')
	{
	    /* if ret==2 then we know it's a problem un-solvable
	     * by match, so we totally give up instead of trying
	     * a bunch of other recursions to check through */
	    wild++;
	    /* swallow all extraneous '*'s after.  */
	    while(*wild=='*')
		wild++;
	    while(*wild=='?' && *string)
	    {
		wild++;string++;
	    }
	    if(!*wild)
		return 0;
	    if(*wild=='*')
		continue;
	    while(*string)
	    {
		if(touppertab[(u_char)*string]
		   == touppertab[(u_char)*wild]
		   && !_match((wild+1), (string+1)))
		    return 0;
		string++;
	    }
	}
	if(!*string)
	    return 1;
	if(*wild!='?' && touppertab[(u_char)*string]!=
	   touppertab[(u_char)*wild])
	    return 1;
	string++;
	wild++;
    }
    return 1;
}
