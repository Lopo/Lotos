/* vi: set ts=4 sw=4 ai: */
/*
 * ct_clone.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __CT_CLONE_H__
#define __CT_CLONE_H__ 1

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
extern char *clone_desc;

#endif /* __CT_CLONE_H__ */

