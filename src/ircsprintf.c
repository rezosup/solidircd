#include "ircsprintf.h"

char num[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char itoa_tab[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'  };
char xtoa_tab[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
		      'a', 'b', 'c', 'd', 'e', 'f'  };
char nullstring[]="(null)";

#ifdef WANT_SNPRINTF
inline int irc_printf(char *str, size_t size, const char *pattern, va_list vl) 
#else
inline int irc_printf(char *str, const char *pattern, va_list vl) 
#endif
{
	va_list ap;
	VA_COPY(ap, vl);
#ifdef WANT_SNPRINTF
	if(!size)
	{
#endif
		return vsprintf(str, pattern, ap);
#ifdef WANT_SNPRINTF
	}
	else
	{
		return vsnprintf(str, size, pattern, ap);
	}
#endif
}

int ircsprintf(char *str, const char *format, ...)
{
    int ret;
    va_list vl;
    va_start(vl, format);
#ifdef WANT_SNPRINTF
    ret=irc_printf(str, 0, format, vl);
#else
    ret=irc_printf(str, format, vl);
#endif
    va_end(vl);
    return ret;
}

#ifdef WANT_SNPRINTF
int ircsnprintf(char *str, size_t size, const char *format, ...)
{
    int ret;
    va_list vl;
    va_start(vl, format);
    ret=irc_printf(str, size, format, vl);
    va_end(vl);
    return ret;
}
#endif

int ircvsprintf(char *str, const char *format, va_list ap)
{
    int ret;
#ifdef WANT_SNPRINTF
    ret=irc_printf(str, 0, format, ap);
#else
    ret=irc_printf(str, format, ap);
#endif
    return ret;
}

#ifdef WANT_SNPRINTF
int ircvsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int ret;
    ret=irc_printf(str, size, format, ap);
    return ret;
}
#endif
