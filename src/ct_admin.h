/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                      Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_ADMIN_H__
#define __CT_ADMIN_H__ 1

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
#ifdef NETLINKS
	extern NL_OBJECT nl_first;
#endif
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;
extern PL_OBJECT plugin_first;
extern CM_OBJECT cmds_first;

struct user_dir_struct {
  char name[USER_NAME_LEN+1],date[80];
  short int level;
  struct user_dir_struct *next,*prev;
  };
extern struct user_dir_struct *first_dir_entry;

extern char *sex[];

extern char *opcl[];

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int listen_sock[], port[];
extern int tyear, tmonth, tmday;
extern int destructed, no_prompt;
extern int word_count;
extern int use_hostsfile;

extern char cmd_history[16][128];

extern char *reg_sysinfo[];

extern char *invisname;
extern char *notloggedon, *nosuchuser, *nosuchroom;
extern char *syserror;
extern char *empty_log;

extern char *talker_name;
extern char *default_warp, *default_jail;

extern char swear_words[MAX_SWEARS+1][WORD_LEN+1];
extern char *noyes2[];
extern char *offon[];
extern char *minmax[];

extern struct {
  char *name;
  char *alias;
  } user_level[];

struct command_struct {
  char name[15],alias[5]; /* 15 and 5 characters should be long enough */
  short int id,min_lev,function;
  int count;
  struct command_struct *next,*prev;
  };
extern struct command_struct *first_command;

struct wiz_list_struct {
  char name[USER_NAME_LEN+1];
  short int level;
  struct wiz_list_struct *next,*prev;
  };
struct wiz_list_struct *first_wiz_entry;

struct {
  char name[USER_NAME_LEN+1],time[80];
  short int on;
  } 
last_login_info[LASTLOGON_NUM+1];

extern struct {
  char *name,*alias; int level,function;
  } command_table[];


extern char *restrict_string;

/* prompts */
extern char *site_style_dns, *site_style_dns_ip, *site_style_offline;
extern char *kill_user_chant, *kill_room_chant;
extern char *promote_user_prompt, *demote_user_prompt;
extern char *muzzle_user_prompt, *muzzle_victim_prompt;
extern char *unmuzzle_user_prompt, *unmuzzle_victim_prompt;
extern char *suicide_prompt;

#endif /* ct_admin.h */
