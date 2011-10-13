/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                      Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

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

extern char *color_mods[];


extern char *restrict_string;

/* prompts */
extern char *appear_user_prompt, *appear_prompt;
extern char *disapear_user_prompt, *disapear_prompt;
extern char *profile_edit_header, *no_profile_prompt;
extern char *continue1;

#endif /* ct_user.h */
