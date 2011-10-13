/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __RESTART_H__
#define __RESTART_H__ 1

extern UR_OBJECT user_first;
extern RM_OBJECT room_first;
#ifdef NETLINKS
	extern NL_OBJECT nl_first;
#endif
extern PL_OBJECT plugin_first;
extern SYS_OBJECT amsys;
extern SYSPP_OBJECT syspp;

extern char progname[], confile[];
extern int listen_sock[], port[];


extern char *syserror;
extern char *default_jail;
extern char *clone_desc;
extern char *restart_prompt, *restart_ok;

#endif /* restart.h */
