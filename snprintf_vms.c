// snprintf_vms.c
// 
// Copyright (c) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
//               2002, 2003, 2004 Ohio University.
// 
// The following code is sourced from the code from the tcptrace project,
// currently hosted at https://github.com/blitz/tcptrace/
// which is licensed GPLv2. This code therefore is also licensed under the terms 
// of the GNU Public License, verison 2.

#include <stdio.h>
#include <stdarg.h>

int snprintf_vms(char *str, size_t len, const char *fmt, ...)
{
   va_list ap;
   int n;

   va_start(ap, fmt);
   n = vsprintf(str, fmt, ap);
   va_end(ap);
   return n;
}