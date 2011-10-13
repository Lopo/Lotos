/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

#ifdef NETLINKS
	extern NL_OBJECT nl_first;
#endif

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;

extern char *syserror;
extern char *nosuchuser;
extern char *talker_name, *talker_signature;

struct user_dir_struct {
  char name[USER_NAME_LEN+1],date[80];
  short int level;
  struct user_dir_struct *next,*prev;
  };
extern struct user_dir_struct *first_dir_entry;

extern struct {
  char *name;
  char *alias;
  } user_level[];

extern char *reg_sysinfo[];

/* prompts */
extern char *smail_edit_header;
extern char *dmail_nomail, *dmail_too_many;