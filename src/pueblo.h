/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __PUEBLO_H__
#define __PUEBLO_H__1 

extern UR_OBJECT user_first;

extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;

extern char *colors[];
extern char *reg_sysinfo[];

extern char *nosuchroom, *noswearing;
extern char *invisname;

extern struct {
	char *name, *alias; int level, function;
	} command_table[];

#endif /* pueblo.h */

