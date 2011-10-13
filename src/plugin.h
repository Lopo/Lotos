/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern SYS_OBJECT amsys;

extern struct {
  char *name;
  char *alias;
  } user_level[];

extern char *noyes1[];
extern char *noyes2[];

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];

extern thour, tmin;
extern int destructed;
extern int word_count;

extern char *colors[];


extern char *syserror, *nosuchuser, *notloggedon;
extern char *invisname;


extern struct {
	char *name,*alias; int level,function;
	} command_table[];


extern PL_OBJECT plugin_first, plugin_last;
extern CM_OBJECT cmds_first, cmds_last;

extern SYSPP_OBJECT syspp;