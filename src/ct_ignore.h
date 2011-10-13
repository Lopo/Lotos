/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

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
