/* vi: set ts=4 sw=4 ai: */
/*
 * who.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __WHO_H__
#define __WHO_H__ 1

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
extern SYSPP_OBJECT syspp;

extern struct {
	char *name;
	char *alias;
	} user_level[];

extern char *sex[];
extern char text[];
extern int port[];

extern char *reg_sysinfo[];

#endif /* __WHO_H__ */

