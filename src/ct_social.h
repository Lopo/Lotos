/* vi: set ts=4 sw=4 ai: */
/*
 * ct_social.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __CT_SOCIAL_H__
#define __CT_SOCIAL_H__ 1

extern SYS_OBJECT amsys;

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;

extern char *colors[];

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
	char *name,*alias;
	int level,function;
	} command_table[];

extern struct {
	char *name;
	char *alias;
	} user_level[];

extern char *restrict_string;
extern char *default_warp;


/* prompts */
extern char *wizshout_style, *wizshout_style_lev;
extern char *cbuff_prompt;
extern char *tell_review_header, *no_tell_review_prompt;
extern char *shout_review_header, *no_shout_review_prompt;
extern char *no_wizs_logged;
extern char *muzzled_cannot;
extern char *room_topic_clrd, *ctopic_in_room;

#endif /* __CT_SOCIAL_H__ */

