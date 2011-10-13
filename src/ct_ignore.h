/* vi: set ts=4 sw=4 ai: */
/*
 * ct_ignore.h
 *
 *   Lotos v1.2.1  : (c) 1999-2001 Pavol Hluchy (Lopo)
 *   last update   : 26.12.2001
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __CT_IGNORE_H__
#define __CT_IGNORE_H__ 1

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

#endif /* ct_ignore.h */

