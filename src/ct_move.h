/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern RM_OBJECT room_first;
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int no_prompt;
extern int word_count;

extern char *nosuchroom, *notloggedon, *nosuchuser;
extern char *invisleave, *invisname;

extern char *crypt_salt;

extern char *default_warp;
extern char *default_personal_room_desc;

extern struct {
  char *name,*alias; int level,function;
  } command_table[];

extern char *restrict_string;


/* prompts */
extern char *already_in_room;
extern char *move_prompt_user, *user_room_move_prompt;