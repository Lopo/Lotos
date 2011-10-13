/* vi: set ts=4 sw=4 ai: */
/*
 * transport.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__ 1

#include "define.h"

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
extern TR_OBJECT transport_first, transport_last;
extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int force_listen;
extern int word_count;

extern char *invisenter, *invisleave;

extern char *nosuchtr;

#endif /* __TRANSPORT_H__ */

