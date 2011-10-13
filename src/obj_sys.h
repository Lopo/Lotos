/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                     Struktura system pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __OBJ_SYS_H__
#define __OBJ_SYS_H__ 1

#include <time.h>

#include "obj_ur.h"

/* system structure */
struct system_struct {
  int heartbeat,keepalive_interval,ignore_sigterm,mesg_life,mesg_check_hour,mesg_check_min;
  int net_idle_time,login_idle_time,user_idle_time,time_out_maxlevel,time_out_afks,crash_action;
  int num_of_logins,logons_old,logons_new,auto_purge,purge_count,purge_skip,users_purged;
  int user_count,max_users,max_clones,min_private_users,colour_def,charecho_def,prompt_def;
  int wizport_level,minlogin_level,gatecrash_level,ignore_mp_level,rem_user_maxlevel,rem_user_deflevel;
  int password_echo,auto_promote,ban_swearing,personal_rooms,startup_room_parse,auto_connect;
  int allow_recaps,suggestion_count,random_motds,motd1_cnt,motd2_cnt,forwarding,sbuffline;
  int resolve_ip,rs_countdown,level_count[ROOT+1],last_cmd_cnt,flood_protect;
  unsigned short logging;
  unsigned int pid;
  char sysname[64],sysmachine[64],sysrelease[64],sysversion[64],sysnodename[256],shoutbuff[REVIEW_LINES][REVIEW_LEN+2];
  time_t boot_time,rs_announce,rs_which,purge_date;
  UR_OBJECT rs_user;
  };
typedef struct system_struct *SYS_OBJECT;

#endif /* obj_sys.h */
