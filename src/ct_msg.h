/* vi: set ts=4 sw=4 ai: */
/*
 * ct_msg.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __CT_MSG_H__
#define __CT_MSG_H__ 1

#include "define.h"
#include "obj_sys.h"

extern SYS_OBJECT amsys;

extern char *month[];

extern char word[MAX_WORDS][WORD_LEN+1];
extern int tyear, tmonth, tmday;
extern int destructed;
extern int word_count;

extern struct {
	char *name, *alias;
	int level, function;
	} command_table[];

//prompts
extern char *syserror;
extern char *nosuchuser;
extern char *icq_page_email;
extern char *ascii_tline, *ascii_line, *ascii_bline;

#endif /* __CT_MSG_H__ */

