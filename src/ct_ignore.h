/* vi: set ts=4 sw=4 ai: */
/*
 * ct_ignore.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __CT_IGNORE_H__
#define __CT_IGNORE_H__ 1

extern char word[MAX_WORDS][WORD_LEN+1];
extern int word_count;

extern char *invisname, *notloggedon;
extern char *noyes2[];

extern struct {
	char *name,*alias;
	int level,function;
	} command_table[];

extern struct {
	char *type;
	char *desc;
	} ignstr[];

extern char *ascii_tline, *ascii_line, *ascii_bline;

#endif /* __CT_IGNORE_H__ */

