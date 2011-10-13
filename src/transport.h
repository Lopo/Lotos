/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__ 1

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

#endif /* transport.h */
