/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                    Struktury pre plug-in Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __OBJ_PL_H__
#define __OBJ_PL_H__ 1

struct plugin_struct {
        char    name[27],author[22],ver[4],req_ver[4];
        char    registration[8];
        int     id,req_userfile,triggerable;
        struct plugin_struct *prev,*next;
        };
typedef struct plugin_struct *PL_OBJECT;

struct plugin_cmd {
        char    command[10];    /* What will the command name be? */
        int     id,req_lev;     /* id = reference ... req_lev = required level */
        int     comnum;         /* Identify the command per plugin since
                                   some plugins may have more than 1. */
        struct plugin_cmd *prev,*next;
        struct plugin_struct *plugin;
        };
typedef struct plugin_cmd *CM_OBJECT;

#endif /* obj_pl.h */
