/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
          Funkcia pre Lotos v1.2.0 zobrazujuca datum a cas kompilacie
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __BUILD_C__
#define __BUILD_C__ 1

#include <stdio.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"

void build_datetime(char *str)
{
	set_crash();
	sprintf(str, "%s   %s", __DATE__, __TIME__);
#ifdef DEBUG
	strcat(str, "  ~FR(DEBUG version)");
#endif
}

#endif /* build.c */

