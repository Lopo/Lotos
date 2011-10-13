/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                      Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_GENERAL_H__
#define __CT_GENERAL_H__ 1

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
#ifdef NETLINKS
	extern NL_OBJECT nl_first;
#endif
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;
extern CM_OBJECT cmds_first;

struct wiz_list_struct {
  char name[USER_NAME_LEN+1];
  short int level;
  struct wiz_list_struct *next,*prev;
  };
extern struct wiz_list_struct *first_wiz_entry;
struct user_dir_struct {
  char name[USER_NAME_LEN+1],date[80];
  short int level;
  struct user_dir_struct *next,*prev;
  };
extern struct user_dir_struct *first_dir_entry;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int port[];
extern int tyear, tmonth, tmday, twday, thour, tmin, tsec;
extern int logon_flag;
extern int word_count;

extern int cal_days[];
extern char *cal_daynames[];

extern char *month[12], *day[7];

extern char *noyes1[], *noyes2[], *minmax[];

extern char *nosuchroom, *notloggedon, *nosuchuser;
extern char *invisname, *noswearing;

extern char *default_warp, *default_jail, *default_shoot, *default_bank;

extern struct {
  char *name;
  char *alias;
  } user_level[];

extern struct {
  char *name,*alias; int level,function;
  } command_table[];

extern struct { 
  char *name; int level; 
  } priv_room[];

extern char *colors[];

extern char *restrict_string;

extern char *reg_sysinfo[];

/* prompts */
extern char *people_here_prompt, *no_people_here_prompt;
extern char *message_prompt, *single_message_prompt, *no_message_prompt;
extern char *topic_prompt, *notopic_prompt, *no_exits;
extern char *user_knock_prompt, *user_room_knock_prompt, *room_knock_prompt;
extern char *private_review_prompt, *review_header, *no_review_prompt;
extern char *entroom_edit_header, *prooms_disabled;
extern char *continue2;

#endif /* ct_general.h */
