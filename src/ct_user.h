/* vi: set ts=4 sw=4 ai: */
/*
 * ct_user.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __CT_USER_H__
#define __CT_USER_H__ 1

extern RM_OBJECT room_first;
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int destructed, no_prompt;
extern int word_count;

extern char *syserror, *nosuchuser, *notloggedon;
extern char *invisname;
extern char *crypt_salt;
extern char *noyes2[], *sex[], *offon[];

extern struct {
	char *name;
	char *alias;
	} user_level[];

extern struct {
	char name[USER_NAME_LEN+1],time[80];
	short int on;
	} last_login_info[LASTLOGON_NUM+1];

extern struct {
	char *name, *alias;
	int level, function;
	} command_table[];

extern char *color_mods[];


extern char *restrict_string;

/* prompts */
extern char *appear_user_prompt, *appear_prompt;
extern char *disapear_user_prompt, *disapear_prompt;
extern char *profile_edit_header, *no_profile_prompt;
extern char *continue1;
extern char *ascii_tline, *ascii_line, *ascii_bline;

#endif /* __CT_USER_H__ */

