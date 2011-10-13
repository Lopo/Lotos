/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern UR_OBJECT user_first, user_last;

extern struct {
  char name[USER_NAME_LEN+1],time[80];
  short int on;
  } 
last_login_info[];

extern RM_OBJECT room_first, room_last;
#ifdef NETLINKS
	extern NL_OBJECT nl_first, nl_last;
#endif
extern SYSPP_OBJECT syspp;

extern struct user_dir_struct *first_dir_entry;
extern struct wiz_list_struct *first_wiz_entry, *last_wiz_entry;
extern struct command_struct *first_command;
extern char cmd_history[16][128];

extern SYS_OBJECT amsys;


extern char verification[];
extern int listen_sock[], port[], port_total;

extern char text[];
extern char wrd[8][81];
extern char confile[];
extern int force_listen, no_prompt, logon_flag;
extern int config_line;

extern char susers_restrict[];


extern char *no_leave;

struct {
  char *name;
  char *alias;
  } user_level[]={
    { "VAZEN",       "J" },
    { "PP06",        "N" },
    { "286",         "2" },
    { "386",         "3" },
    { "486",         "4" },
    { "PENTIUM",     "5" },
    { "PENTIUM_MMX", "6" },
    { "CELERON",     "7" },
    { "PENTIUM_II",  "8" },
    { "PENTIUM_III", "9" },
    { "XEON",        "X" },
    { "BOT",         "B" },
    { "ROOT",        "R" }
  };

extern struct {
  char *name,*alias; int level,function;
  } command_table[];