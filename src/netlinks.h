/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern struct {
  char *name,*alias; int level,function;
  } command_table[];

extern UR_OBJECT user_first;
extern NL_OBJECT nl_first, nl_last;
extern RM_OBJECT room_first;
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;

extern struct {
	char *name;
	char *alias;
	} user_level[];

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern char verification[];
extern int port[];
extern int destructed;
extern int word_count;

/* frazy */
extern char *invisname, *invisleave, *invisenter;
extern char *nosuchroom;