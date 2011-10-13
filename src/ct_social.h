/*****************************************************************************
                      Hlavickovy subor OS Star v1.1.0
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern SYS_OBJECT amsys;

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;

extern char *noswearing;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int force_listen, no_prompt;
extern int word_count;


extern char *invisname;
extern char *notloggedon, *nosuchuser;

extern char *say_style;

extern int biglet[26][5][5], bigsym[32][5][5];

struct user_dir_struct {
  char name[USER_NAME_LEN+1],date[80];
  short int level;
  struct user_dir_struct *next,*prev;
  };
extern struct user_dir_struct *first_dir_entry;

extern struct {
  char *name,*alias; int level,function;
  } command_table[];

extern struct {
  char *name;
  char *alias;
  } user_level[];

extern char *restrict_string;
extern char *default_warp;


/* prompts */
extern char *show_style, *wizshout_style, *wizshout_style_lev;
extern char *cbuff_prompt;
extern char *tell_review_header, *no_tell_review_prompt;
extern char *shout_review_header, *no_shout_review_prompt;
extern char *no_wizs_logged;
