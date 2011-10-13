/* vi: set ts=4 sw=4 ai: */
/*
 * who.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
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

