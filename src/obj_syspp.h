/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                Struktura rozsireneho systemu pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __OBJ_SYSPP_H__
#define __OBJ_SYSPP_H__ 1

/* doplnujuca systemova struktura */
struct syspp_struct {
	int oss_highlev_debug;
	int debug_input; // POZOR !!! nikdy nenastavovat na 1 !!!
	int highlev_debug_on;
	int pueblo_enh, pblo_usr_mm_def, pblo_usr_pg_def;
	long autosave, auto_save;
	int kill_msgs;
	int num_of_www, max_www;
	int sys_access, wiz_access, www_access;
	long tcounter[4], bcounter[4], acounter[4], mcounter[4];
	int auto_afk, auto_afk_time;
	};
typedef struct syspp_struct *SYSPP_OBJECT;

#endif /* obj_syspp.h */
