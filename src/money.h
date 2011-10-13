/* vi: set ts=4 sw=4 ai: */
/*
 * money.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __MONEY_H__
#define __MOENY_H__ 1

extern UR_OBJECT user_first;

extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;

extern char *invisname, *notloggedon, *default_bank;
extern char *ascii_tline, *ascii_line, *ascii_bline;

#endif /* __MONEY_H__ */

