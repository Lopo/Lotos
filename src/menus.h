/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __MENUS_H__
#define __MENUS_H__ 1

SYS_OBJECT amsys;
SYSPP_OBJECT syspp;

extern char *offon[];
extern char *sex[];

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int destructed, no_prompt;
extern int word_count;

extern char *help_style[];
extern char *who_style[];

extern struct {
	char *name, *fname, *prompt0, *prompt1;
	} menu_tab[];

extern struct {
	char cmenu, *type, *name, *desc;
	} set_tab[];
extern struct {
	char cmenu, *type, *name, *desc;
	} setterm_tab[];
extern struct {
	char cmenu, *type, *name, *desc;
	} settab_bank[];

extern struct {
	char *name, *str;
	} prompt_tab[];

/* prompts.c */
extern char *continue1;
extern char *invisname;
extern char *use_menu_prompt;
extern char *room_leave_setup, *user_bch_setup;
extern char *nosuchuser, syserror;

/* boots.h */
extern struct {
	char *name, *alias;
	} user_level[];

/* star.h */
extern char *default_bank;
#endif /* menus.h */
