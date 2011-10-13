/* vi: set ts=4 sw=4 ai: */
/*
 * netlinks.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __NETLINKS_H__
#define __NETLINKS_H__ 1

extern struct {
	char *name,*alias;
	int level,function;
	} command_table[];

extern UR_OBJECT user_first;
extern NL_OBJECT nl_first, nl_last;
extern RM_OBJECT room_first;
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;

extern struct {
	char *name;
	char *alias;
	} user_level[];

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern char verification[];
extern int port[];
extern int destructed;
extern int word_count;

/* frazy */
extern char *invisname, *invisleave, *invisenter;
extern char *nosuchroom;

#endif /* __NETLINKS_C__ */

