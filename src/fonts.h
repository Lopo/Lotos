/* vi: set ts=4 sw=4 ai: */
/*
 * fonts.h
 *
 *   Lotos v1.2.1  : (c) 1999-2001 Pavol Hluchy (Lopo)
 *   last update   : 26.12.2001
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __FONTS_H__
#define __FONTS_H__ 1

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

#endif /* fonts.h */

