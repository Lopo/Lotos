/*****************************************************************************
                      Hlavickovy subor OS Star v1.1.0
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;

extern char *invisname, *notloggedon;
extern char *noyes2[];

extern struct {
  char *name,*alias; int level,function;
  } command_table[];

extern struct {
	char *type;
	char *desc;
	} ignstr[];
