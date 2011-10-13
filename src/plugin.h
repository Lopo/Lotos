/* vi: set ts=4 sw=4 ai: */
/*
 * plugin.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__ 1

extern SYS_OBJECT amsys;

extern struct {
	char *name;
	char *alias;
	} user_level[];

extern char *noyes1[];
extern char *noyes2[];

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];

extern int thour, tmin;
extern int destructed;
extern int word_count;

extern char *colors[];


extern char *syserror, *nosuchuser, *notloggedon;
extern char *invisname;


extern struct {
	char *name,*alias;
	int level,function;
	} command_table[];

extern PL_OBJECT plugin_first, plugin_last;
extern CM_OBJECT cmds_first, cmds_last;

extern SYSPP_OBJECT syspp;

#endif /* __PLUGIN_H__ */

