/*****************************************************************************
                      Hlavickovy subor OS Star v1.1.0
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

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

extern char text[], vtext[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern char progname[], confile[];
extern int listen_sock[], port[];
extern int destructed, force_listen, logon_flag;
extern int word_count;

extern char *syserror, *nosuchuser, *notloggedon;
extern char *invisname;
extern char *default_jail;
extern char *clone_desc;
extern char *restart_prompt, *restart_ok;

extern char *reg_sysinfo[];


extern struct {
	char *name,*alias; int level,function;
	} command_table[];

extern struct {
	char *name, *fname, *prompt;
	} menu_tab[];
