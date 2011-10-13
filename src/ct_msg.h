/* vi: set ts=4 sw=4 ai: */
/*
 * ct_msg.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
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

#endif /* __CT_MSG_H__ */

