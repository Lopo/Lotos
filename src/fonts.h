/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int force_listen;
extern int word_count;

extern long *inchrline;
extern int inchrlinelen, inchrlinelenlimit;
typedef struct fc {
	long ord;
	char **thechar;  /* Alloc'd char thechar[charheight][]; */
	struct fc *next;
	} fcharnode;
fcharnode *fcharlist;
extern char **currchar;
extern int currcharwidth;
extern char **outline;
extern int outlinelen;
extern int justification, right2left;
extern int outputwidth;
extern int outlinelenlimit;
extern char hardblank;
extern int charheight, defaultmode;

/* prompts */
extern char *invisname;
extern char *notloggedon;
extern char *noswearing;
