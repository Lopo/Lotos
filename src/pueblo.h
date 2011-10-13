/* vi: set ts=4 sw=4 ai: */
/*
 * pueblo.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __PUEBLO_H__
#define __PUEBLO_H__ 1

#include "define.h"

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

#endif /* __PUEBLO_H__ */

