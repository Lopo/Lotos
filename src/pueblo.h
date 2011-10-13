/*****************************************************************************
                      Hlavickovy subor OS Star v1.1.0
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern UR_OBJECT user_first;

extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;

extern char *colors[];
extern char *reg_sysinfo[];

extern char *nosuchroom, *noswearing;
extern char *invisname;

extern struct {
	char *name, *alias; int level, function;
	} command_table[];
