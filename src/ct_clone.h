/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern UR_OBJECT user_first;
extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;
extern int destructed;

extern char *notloggedon, *nosuchroom, *syserror;
extern char *invisname;

extern char *restrict_string;


/* prompts */
extern char *all_clone_style;
extern char *clone_here_prompt, *clone_prompt;
extern char *clone_user_destroy;
extern char *clone_room_destroy1, *clone_room_destroy2;
extern char *clone_switch_prompt;