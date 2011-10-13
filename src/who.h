/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

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

#endif /* who.h */
