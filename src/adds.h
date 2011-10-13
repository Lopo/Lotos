/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                      Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __ADDS_H__
#define __ADDS_H__ 1

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
#ifdef NETLINKS
	extern NL_OBJECT nl_first;
#endif
extern PL_OBJECT plugin_first;
extern CM_OBJECT cmds_first;
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;

extern char swear_words[MAX_SWEARS+1][WORD_LEN+1];

extern char *offon[];

extern char text[], vtext[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int tyear, tmonth, tday, thour, tmin, tsec;
extern int destructed, force_listen, logon_flag, no_prompt;
extern int word_count;

extern char *syserror, *nosuchuser, *notloggedon;
extern char *invisname;
extern char *continue1;

extern char *reg_sysinfo[];

extern struct {
	char lastfile[512+1];
	int lastline;
	unsigned char n;
	} crash[];
extern int crash_step;

extern struct {
	char *name,*alias; int level,function;
	} command_table[];

#endif /* adds.h */
