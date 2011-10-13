/*****************************************************************************
                Struktura rozsireneho systemu v OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

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
	};
typedef struct syspp_struct *SYSPP_OBJECT;