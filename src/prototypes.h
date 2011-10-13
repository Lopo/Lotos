/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
           Hlavickovy subor s prototypmi funkcii pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__ 1

#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>

#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_tr.h"
#include "obj_pl.h"
#include "obj_mc.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif


#define args(list) list

/* external prototypes used - should you get an error with these then
   comment them out... but you shouldn't! */

extern char * crypt(const char *,const char*);
#if !defined __GLIBC__ || __GLIBC__ < 2
extern void (*signal(int,void (int)))(int);
#else
extern void (*sysv_signal(int,void (int)))(int);
#endif

/* main function */

int       main args((int argc, char *argv[]));

/* string functions - comparisons, convertions, etc */

int       get_charclient_line args((UR_OBJECT user, char *inpstr, int len));
void      terminate args((char *str));
int       wordfind args((char *inpstr));
void      clear_words args((void));
int       yn_check args((char *wd));
int       onoff_check args((char *wd));
int       minmax_check args((char *wd));
int       resolve_check args((char *wd));
void      echo_off args((UR_OBJECT user));
void      echo_on args((UR_OBJECT user));
char *    remove_first args((char *inpstr));
int       contains_swearing args((char *str));
char *    censor_swear_words args((char *has_swears));
int       colour_com_count args((char *str));
char *    colour_com_strip args((char *str));
void      strtoupper args((char *str));
void      strtolower args((char *str));
int       is_number args((char *str));
char *    istrstr args((char *str, char *pat));
char *    replace_string args((char *inpstr,char *old,char *new));
int       s_instr args((char *s1,char *s2));
void      midcpy args((char *strf,char *strt,int fr,int to));
char *    ordinal_text args((int num));
char *    long_date args((int which));
void      smiley_type args((char *str, char *type));
char *    center_string args((int cstrlen,int mark,char *marker,char *str,...));
char *    center args((char *string, int clen));
int       is_fnumber args((char *str));
int       is_inumber args((char *str));
char *    replace_swear args((char *inpstr, char *old));
void      reset_murlist args((UR_OBJECT user));
char *    grm_gnd args((int typ, int gnd));
char *    grm_num args((int typ, int n));
void      split_com_str_num args((char *inpstr, int num));
void      nick_grm args((UR_OBJECT user));
int       contains_extension args((char *str, int type));
void      resetbuff args((char *buff));
char      stricmp args((char *str1, char *str2));

/* Object functions */

void      create_system args((void));
UR_OBJECT create_user args((void));
void      reset_user args((UR_OBJECT user));
void      destruct_user args((UR_OBJECT user));
RM_OBJECT create_room args((void));
void      destruct_room args((RM_OBJECT rm));
int       add_command args((int cmd_id));
int       rem_command args((int cmd_id));
int       add_user_node args((char *name,int level));
int       rem_user_node args((char *name,int lev));
void      add_user_date_node args((char *name,char *date));
int       add_wiz_node args((char *name,int level));
int       rem_wiz_node args((char *name));
int       user_list_level args((char *name, int lvl));

/* performs checks and searchs */

void      check_directories args((void));
int       pattern_match args((char *str,char *pat));
int       site_banned args((char *sbanned,int new));
int       login_port_flood args((char *asite));
int       user_banned args((char *name));
void      reset_access args((RM_OBJECT rm));
UR_OBJECT get_user args((char *name));
UR_OBJECT get_user_name args((UR_OBJECT user,char *i_name));
RM_OBJECT get_room args((char *name));
RM_OBJECT get_room_full args((char *name));
int       get_level args((char *name));
int       has_room_access args((UR_OBJECT user,RM_OBJECT rm));
void      check_start_room args((UR_OBJECT user));
int       find_user_listed args((char *name));
int       validate_email args((char *email));
int       user_logged_on args((char *name));
int       in_private_room args((UR_OBJECT user));
int       has_gcom args((UR_OBJECT user,int cmd_id));
int       has_xcom args((UR_OBJECT user,int cmd_id));
int       is_personal_room args((RM_OBJECT rm));
int       is_my_room args((UR_OBJECT user,RM_OBJECT rm));
int       room_visitor_count args((RM_OBJECT rm));
int       has_room_key args((char *visitor,RM_OBJECT rm));

/* setting up of sockets */

void      setup_readmask args((fd_set *mask));
void      accept_connection args((int lsock,int num));
char *    get_ip_address args((struct sockaddr_in acc_addr));
char *    resolve_ip args((char *host));
void      init_sockets args((void));

/* loading up and parsing of the configuration files */

void      load_and_parse_config args((void));
void      parse_init_section args((void));
void      parse_rooms_section args((void));
void      parse_topics_section args((char *topic));
#ifdef NETLINKS
void      parse_sites_section args((void));
#endif
void      parse_transports_section args((void));
void      parse_user_rooms args((void));

/* signal handlers and exit functions */

void      init_signals args((void));
void      sig_handler args((int sig));
void      boot_exit args((int code));

/* event functions */

void      do_events args((int sig));
void      reset_alarm args((void));
void      check_reboot_shutdown args((void));
void      check_idle_and_timeout args((void));
void      check_messages args((UR_OBJECT user, int chforce));
void      record_last_login args((char *name));
void      record_last_logout args((char *name));

/* initializing of the globals and other stuff */

int       load_user_details args((UR_OBJECT user));
int       load_user_olddetails args((UR_OBJECT user));
int       save_user_details args((UR_OBJECT user,int save_current));
void      set_date_time args((void));
void      process_users args((void));
void      count_users args((void));
void      parse_commands args((void));
void      count_suggestions args((void));
int       count_motds args((int forcecnt));
int       get_motd_num args((int motd));

/* file functions - reading, writing, counting of lines, etc */

void      clean_files args((char *name));
int       remove_top_bottom args((char *filename,int where));
int       count_lines args((char *filename));

/* write functions - users, rooms, system logs, etc */

void      write_sock args((int sock, char *str));
void      vwrite_user args((UR_OBJECT user,char *str,...));
void      write_user args((UR_OBJECT user, char *str));
void      write_level args((int level,int above,char *str,UR_OBJECT user));
void      vwrite_room args((RM_OBJECT rm,char *str,...));
void      write_room args((RM_OBJECT rm,char *str));
void      vwrite_room_except args((RM_OBJECT rm,UR_OBJECT user,char *str,...));
void      write_room_except args((RM_OBJECT rm,char *str,UR_OBJECT user));
void      write_friends args((UR_OBJECT user,char *str,int revt));
void      write_syslog args((int type,int write_time,char *str, ...));
void      record_last_command args((UR_OBJECT user,int comnum,int len));
void      dump_commands args((int foo));
void      write_monitor args((UR_OBJECT user,RM_OBJECT rm,int rec));
int       more args((UR_OBJECT user,int sock,char *filename));
int       more_users args((UR_OBJECT user));
void      add_history args((char *name,int showtime,char *str));

/* logon/off functions */

void      login args((UR_OBJECT user, char *inpstr));
void      attempts args((UR_OBJECT user));
void      show_login_info args((UR_OBJECT user));
void      connect_user args((UR_OBJECT user));
void      disconnect_user args((UR_OBJECT user));

/* misc and line editor functions */

int       misc_ops args((UR_OBJECT user, char *inpstr));
void      editor args((UR_OBJECT user, char *inpstr));
void      editor_done args((UR_OBJECT user));

/* user command functions and their subsids */

int       process_input_string args((UR_OBJECT user,char *inpstr));
void      split_command_string args((char *inpstr));
int       exec_com args((UR_OBJECT user, char *inpstr));
void      talker_shutdown args((UR_OBJECT user,char *str,int reboot));
void      shutdown_com args((UR_OBJECT user));
void      reboot_com args((UR_OBJECT user));
void      record args((RM_OBJECT rm,char *str));
void      record_tell args((UR_OBJECT user, char *str));
void      record_shout args((char *str));
void      record_afk args((UR_OBJECT user, char *str));
void      clear_afk args((UR_OBJECT user));
void      record_edit args((UR_OBJECT user, char *str));
void      clear_revbuff args((RM_OBJECT rm));
void      clear_tells args((UR_OBJECT user));
void      clear_shouts args((void));
void      clear_edit args((UR_OBJECT user));
void      cls args((UR_OBJECT user));
int       send_mail args((UR_OBJECT user,char *to,char *ptr,int iscopy));
#ifdef NETLINKS
  void      send_external_mail args((NL_OBJECT nl,UR_OBJECT user,char *to,char *ptr));
#endif
void      smail args((UR_OBJECT user, char *inpstr,int done_editing));
void      rmail args((UR_OBJECT user));
void      read_specific_mail args((UR_OBJECT user));
void      read_new_mail args((UR_OBJECT user));
void      dmail args((UR_OBJECT user));
void      mail_from args((UR_OBJECT user));
void      copies_to args((UR_OBJECT user));
void      send_copies args((UR_OBJECT user,char *ptr));
void      level_mail args((UR_OBJECT user, char *inpstr,int done_editing));
int       send_broadcast_mail args((UR_OBJECT user,char *ptr,int lvl,int type));
int       mail_sizes args((char *name,int type));
int       reset_mail_counts args((UR_OBJECT user));
void      set_forward_email args((UR_OBJECT user));
void      verify_email args((UR_OBJECT user));
void      forward_email args((char *name,char *from,char *message));
int       send_forward_email args((char *send_to,char *mail_file));
int       double_fork args((void));
void      forward_specific_mail args((UR_OBJECT user));
void      read_board args((UR_OBJECT user));
void      read_board_specific args((UR_OBJECT user,RM_OBJECT rm,int msg_number));
void      write_board args((UR_OBJECT user, char *inpstr,int done_editing));
void      wipe_board args((UR_OBJECT user));
int       check_board_wipe args((UR_OBJECT user));
void      board_from args((UR_OBJECT user));
void      search_boards args((UR_OBJECT user));
void      suggestions args((UR_OBJECT user,int done_editing));
void      delete_suggestions args((UR_OBJECT user));
void      suggestions_from args((UR_OBJECT user));
int       get_wipe_parameters args((UR_OBJECT user));
int       wipe_messages args((char *filename,int from,int to,int type));
void      listbans args((UR_OBJECT user));
void      ban args((UR_OBJECT user));
void      auto_ban_site args((char *asite));
void      ban_site args((UR_OBJECT user));
void      ban_user args((UR_OBJECT user));
void      ban_new args((UR_OBJECT user));
void      unban args((UR_OBJECT user));
void      unban_site args((UR_OBJECT user));
void      unban_user args((UR_OBJECT user));
void      unban_new args((UR_OBJECT user));
void      look args((UR_OBJECT user));
void      who args((UR_OBJECT user,int type));
void      login_who args((UR_OBJECT user));
void      display_files args((UR_OBJECT user,int admins));
void      help args((UR_OBJECT user));
void      help_commands_level args((UR_OBJECT user,int is_wrap));
void      help_commands_function args((UR_OBJECT user,int is_wrap));
void      help_commands_only args((UR_OBJECT user,int is_wrap));
void      help_credits args((UR_OBJECT user));
void      say args((UR_OBJECT user, char *inpstr));
void      say_to args((UR_OBJECT, char*inpstr));
void      shout args((UR_OBJECT user, char *inpstr));
void      tell args((UR_OBJECT user, char *inpstr));
void      emote args((UR_OBJECT user, char *inpstr));
void      semote args((UR_OBJECT user, char *inpstr));
void      pemote args((UR_OBJECT user, char *inpstr));
void      s_echo args((UR_OBJECT user, char *inpstr));
void      mutter args((UR_OBJECT user, char *inpstr));
void      plead args((UR_OBJECT user, char *inpstr));
void      wizshout args((UR_OBJECT user, char *inpstr));
void      wizemote args((UR_OBJECT user, char *inpstr));
void      picture_tell args((UR_OBJECT user));
void      preview args((UR_OBJECT user));
void      picture_all args((UR_OBJECT user));
void      greet args((UR_OBJECT user, char *inpstr));
void      think_it args((UR_OBJECT user, char *inpstr));
void      sing_it args((UR_OBJECT user, char *inpstr));
void      bcast args((UR_OBJECT user, char *inpstr));
void      wake args((UR_OBJECT user, char *inpstr));
void      s_beep args((UR_OBJECT user,char *inpstr));
void      quick_call args((UR_OBJECT user));
void      revedit args((UR_OBJECT user));
void      revafk args((UR_OBJECT user));
void      revclr args((UR_OBJECT user));
void      revshout args((UR_OBJECT user));
void      revtell args((UR_OBJECT user));
void      review args((UR_OBJECT user));
void      status args((UR_OBJECT user));
void      examine args((UR_OBJECT user));
void      prompt args((UR_OBJECT user));
void      toggle_charecho args((UR_OBJECT user));
void      set_desc args((UR_OBJECT user, char *inpstr));
void      set_iophrase args((UR_OBJECT user, char *inpstr));
void      enter_profile args((UR_OBJECT user,int done_editing));
void      account_request args((UR_OBJECT user, char *inpstr));
void      afk args((UR_OBJECT user, char *inpstr));
void      visibility args((UR_OBJECT user,int vis));
void      make_invis args((UR_OBJECT user));
void      make_vis args((UR_OBJECT user));
void      show_igusers args((UR_OBJECT user));
int       check_igusers args((UR_OBJECT user,UR_OBJECT ignoring));
void      set_igusers args((UR_OBJECT user));
void      set_ignores args((UR_OBJECT user));
void      show_ignlist args((UR_OBJECT user));
void      toggle_ignall args((UR_OBJECT user));
void      user_listen args((UR_OBJECT user));
void      create_clone args((UR_OBJECT user));
void      destroy_user_clones args((UR_OBJECT user));
void      destroy_clone args((UR_OBJECT user));
void      myclones args((UR_OBJECT user));
void      allclones args((UR_OBJECT user));
void      clone_switch args((UR_OBJECT user));
void      clone_say args((UR_OBJECT user, char *inpstr));
void      clone_emote args((UR_OBJECT user, char *inpstr));
void      clone_hear args((UR_OBJECT user));
void      go args((UR_OBJECT user));
void      move_user args((UR_OBJECT user,RM_OBJECT rm,int teleport));
void      s_move args((UR_OBJECT user));
void      set_room_access args((UR_OBJECT user));
void      change_room_fix args((UR_OBJECT user,int fix));
void      invite args((UR_OBJECT user));
void      uninvite args((UR_OBJECT user));
void      letmein args((UR_OBJECT user));
void      rooms args((UR_OBJECT user,int show_topics,int wrap));
void      clear_topic args((UR_OBJECT user));
void      join args((UR_OBJECT user));
void      shackle args((UR_OBJECT user));
void      unshackle args((UR_OBJECT user));
void      set_topic args((UR_OBJECT user, char *inpstr));
void      check_autopromote args((UR_OBJECT user,int attrib));
void      temporary_promote args((UR_OBJECT user));
void      promote args((UR_OBJECT user, char *inpstr));
void      demote args((UR_OBJECT user, char *inpstr));
void      muzzle args((UR_OBJECT user));
void      unmuzzle args((UR_OBJECT user));
void      arrest args((UR_OBJECT user, int type));
void      unarrest args((UR_OBJECT user));
void      change_pass args((UR_OBJECT user));
void      kill_user args((UR_OBJECT user, char *inpstr));
void      suicide args((UR_OBJECT user));
void      delete_user args((UR_OBJECT user,int this_user));
int       purge args((int type,char *purge_site,int purge_days));
void      purge_users args((UR_OBJECT user));
void      user_expires args((UR_OBJECT user));
void      create_account args((UR_OBJECT user));
void      force_save args((UR_OBJECT user));
void      viewlog args((UR_OBJECT user));
void      show_last_login args((UR_OBJECT user));
void      samesite args((UR_OBJECT user,int stage));
void      site args((UR_OBJECT user));
void      manual_history args((UR_OBJECT user, char *inpstr));
void      user_history args((UR_OBJECT user));
void      logging args((UR_OBJECT user));
void      minlogin args((UR_OBJECT user));
void      system_details args((UR_OBJECT user));
void      clearline args((UR_OBJECT user));
void      toggle_swearban args((UR_OBJECT user));
void      display_colour args((UR_OBJECT user));
void      show args((UR_OBJECT user, char *inpstr));
void      show_ranks args((UR_OBJECT user));
void      wiz_list args((UR_OBJECT user));
void      get_time args((UR_OBJECT user));
void      show_version args((UR_OBJECT user));
void      show_memory args((UR_OBJECT user));
void      play_hangman args((UR_OBJECT user));
void      guess_hangman args((UR_OBJECT user));
void      retire_user args((UR_OBJECT user));
void      unretire_user args((UR_OBJECT user));
int       in_retire_list args((char *name));
void      add_retire_list args((char *name));
void      clean_retire_list args((char *name));
void      show_command_counts args((UR_OBJECT user));
void      recount_users args((UR_OBJECT user,int ok));
void      set_command_level args((UR_OBJECT user));
void      grep_users args((UR_OBJECT user));
void      shoot_user args((UR_OBJECT user));
void      reload_gun args((UR_OBJECT user));
void      set_command_level args((UR_OBJECT user));
void      user_xcom args((UR_OBJECT user));
void      user_gcom args((UR_OBJECT user));
int       set_xgcom args((UR_OBJECT user,UR_OBJECT u,int id,int banned,int set));
int       get_xgcoms args((UR_OBJECT user));
void      reload_room_description args((UR_OBJECT user, int w));

/* friends stuff */

int       user_is_friend args((UR_OBJECT user, UR_OBJECT u));
void      alert_friends args((UR_OBJECT user, int mode));
void      get_friends args((UR_OBJECT user));
void      friends args((UR_OBJECT user));
void      friend_say args((UR_OBJECT user, char *inpstr));
void      friend_emote args((UR_OBJECT user, char *inpstr));
void      friend_smail args((UR_OBJECT user, char *inpstr,int done_editing));

void      bring args((UR_OBJECT user));
void      force args((UR_OBJECT user,char *inpstr));

/* calendar and reminders stuff */

int       is_leap args((unsigned yr));
unsigned  months_to_days args((unsigned mn));
long      years_to_days args((unsigned yr));
long      ymd_to_scalar args((unsigned yr,unsigned mo,unsigned dy));
void      scalar_to_ymd args((long scalar,unsigned *yr,unsigned *mo,unsigned *dy));
int       is_ymd_today args((unsigned yr,unsigned mo,unsigned dy));
void      show_calendar args((UR_OBJECT user));
int       has_reminder args((UR_OBJECT user,int dd,int mm,int yy));
int       has_reminder_today args((UR_OBJECT user));
int       remove_old_reminders args((UR_OBJECT user));
int       read_user_reminders args((UR_OBJECT user));
int       write_user_reminders args((UR_OBJECT user));
void      show_reminders args((UR_OBJECT user,int stage));

/* personal rooms stuff */

void      personal_room args((UR_OBJECT user));
void      personal_room_lock args((UR_OBJECT user));
void      personal_room_visit args((UR_OBJECT user));
void      personal_room_decorate args((UR_OBJECT user,int done_editing));
int       personal_room_store args((char *name,int store,RM_OBJECT rm));
void      personal_room_admin args((UR_OBJECT user));
void      personal_room_key args((UR_OBJECT user));
int       personal_key_add args((UR_OBJECT user, char *name));
int       personal_key_remove args((UR_OBJECT user, char *name));
void      personal_room_bgone args((UR_OBJECT user));

void      dump_to_file args((UR_OBJECT user));
void      change_user_name args((UR_OBJECT user));

#ifdef NETLINKS
NL_OBJECT create_netlink args((void));
void      destruct_netlink args((NL_OBJECT nl));

/* Connection functions */

void      init_connections args((void));
int       connect_to_site args((NL_OBJECT nl));

/* Event functions relating to netlinks */

void      check_nethangs_send_keepalives args((void));

/* NUTS Netlink protocol */

void      accept_server_connection args((int sock,struct sockaddr_in acc_addr));
void      exec_netcom args((NL_OBJECT nl, char *inpstr));
void      nl_transfer args((NL_OBJECT nl,char *name,char *pass,int lev,char *inpstr));
void      nl_release args((NL_OBJECT nl,char *name));
void      nl_action args((NL_OBJECT nl,char *name, char *inpstr));
void      nl_granted args((NL_OBJECT nl,char *name));
void      nl_denied args((NL_OBJECT nl,char *name, char *inpstr));
void      nl_mesg args((NL_OBJECT nl,char *name));
void      nl_prompt args((NL_OBJECT nl,char *name));
void      nl_verification args((NL_OBJECT nl,char *w2,char *w3,int com));
void      nl_removed args((NL_OBJECT nl,char *name));
void      nl_error args((NL_OBJECT nl));
void      nl_checkexist args((NL_OBJECT nl,char *to,char *from));
void      nl_user_notexist args((NL_OBJECT nl,char *to,char *from));
void      nl_user_exist args((NL_OBJECT nl,char *to,char *from));
void      nl_mail args((NL_OBJECT nl,char *to,char *from));
void      nl_endmail args((NL_OBJECT nl));
void      nl_mailerror args((NL_OBJECT nl,char *to,char *from));
void      nl_rstat args((NL_OBJECT nl,char *to));
void      shutdown_netlink args((NL_OBJECT nl));

/* User commands relating to netlink functions */

void      home args((UR_OBJECT user));
void      netstat args((UR_OBJECT user));
void      netdata args((UR_OBJECT user));
void      connect_netlink args((UR_OBJECT user));
void      disconnect_netlink args((UR_OBJECT user));
void      remote_stat args((UR_OBJECT user));
#endif   //NETLINKS



/* doplnene z inych talkrov a moje vlastne -netriedene */
void      oss_versionVerify args((void));
int       show_file args((UR_OBJECT user, char *filename));
void      reply args((UR_OBJECT user, char *inpstr));
void      shoutto args((UR_OBJECT user, char *inpstr));
void      tellall args((UR_OBJECT user, char *inpstr));
void      write_room_except2 args((RM_OBJECT rm, char *str, UR_OBJECT user, UR_OBJECT user2));
void      show_nick_grm args((UR_OBJECT user, UR_OBJECT u));
void      com_nick_grm args((UR_OBJECT user));
void      auth_user args((UR_OBJECT user));
char *    getanswer args((FILE *popfp, char *buff, int eol));
void      check_alarm args((void));
void      set_ualarm args((UR_OBJECT user));
void      finger_host args((UR_OBJECT user));
void      clear_temps args((void));
void      check_autosave args((void));
void      list_txt_files args((UR_OBJECT user));
void      list_pic_files args((UR_OBJECT user));
void      reloads args((UR_OBJECT user));
int       count_musers args((UR_OBJECT user, char *inpstr));
void      quit_user args((UR_OBJECT user, char *inpstr));
void      show_map args((UR_OBJECT user));
void      list_kill_msgs args((UR_OBJECT user));
void      load_swear_file args((UR_OBJECT user));
void      swear_com args((UR_OBJECT user));
void      modify args((UR_OBJECT user, char *inpstr));
void      restrict args((UR_OBJECT user));
void      list_cmdas args((UR_OBJECT user));
void      system_access args((UR_OBJECT user, char *inpstr, int co));
void      force_backup args((UR_OBJECT user));
void      set_follow args((UR_OBJECT user));
void      follow args((UR_OBJECT user));
void      load_counters args((void));
void      save_counters args((void));
void      show_counters args((UR_OBJECT user));
void      hug args((UR_OBJECT user, char *inpstr));
void      kiss args((UR_OBJECT user, char *inpstr));
void      write_duty args((int level, char *str, RM_OBJECT room, UR_OBJECT user, int cr));
void      write_usage args((UR_OBJECT user, char *msg, ...));
int       osstar_load args((void));
int       init_ossmain args((void));
void      create_systempp args((void));
int       port_connect args((char *host, int port));
int       backup_talker args((void));
void      main_help args((UR_OBJECT user));
void      volby args((UR_OBJECT user));
void      set_ign_word args((UR_OBJECT user));
void      kick args((UR_OBJECT user));
void      restart_com args((UR_OBJECT user));
void      restart args((UR_OBJECT user));
void      reinit_sockets args((void));
void      restore_structs args((void));
void      myxterm args((UR_OBJECT user, char *inpstr));
void      allxterm args((UR_OBJECT user, char *inpstr));
void      print_menu args((UR_OBJECT user));
int       setmain_ops args((UR_OBJECT user, char *inpstr));
void      who_short args((UR_OBJECT user));
void      who_moebyroom args((UR_OBJECT user));
void      who_hope args((UR_OBJECT user));
void      who_stairway args((UR_OBJECT user));
void      who_nuts333 args((UR_OBJECT user));
int       inroom args((RM_OBJECT rm));
int       reinit_save_user args((UR_OBJECT user));
int       reinit_load_user args((UR_OBJECT user, int stage));
int       reinit_save_user_malloc args((UR_OBJECT user));
int       reinit_load_user_malloc args((UR_OBJECT user));
int       reinit_save_room args((RM_OBJECT room));
int       reinit_load_room args((RM_OBJECT room));
#ifdef DEBUG
void      test args((UR_OBJECT user, char *inpstr));
#endif
void      write_pid args((void));
void      identify args((UR_OBJECT user));
struct hostent *fgethostbyname args((char *hostname));
int       isproces args((char *procname, char *prg));
int       isrunning args((char *prog));
int       file_exists args((char *fname));
void      restore args((UR_OBJECT user));
void      build_datetime args((char *str));
void      create_kill_file args((void));

/* net */
void      init_hostsfile args((void));
int       check_host args((char *ip_site, char *named_site));
void      add_host args((char *siteaddr, char *sitename));
void      get_net_addresses args((struct sockaddr_in acc_addr, char *ip_site, char *named_site));
int       ident_request args((struct hostent *rhost, int rport, int lport, char *accname));
int       mail_id_request args((struct hostent *rhost, char *accname, char *email));

/* pluginy */
void      destroy_pl_cmd args((CM_OBJECT c));
void      destroy_plugin args((PL_OBJECT p));
void      save_plugin_data args((UR_OBJECT user));
int       call_plugin_exec args((UR_OBJECT user, char *str, PL_OBJECT plugin, int comnum));
int       oss_run_plugins args((UR_OBJECT user, char *str, char *comword, int len));
int       oss_plugin_dump args((void));
void      plugin_triggers args((UR_OBJECT user, char *inpstr));
void      load_plugin_data args((UR_OBJECT user));
void      disp_plugin_registry args((UR_OBJECT user));
PL_OBJECT create_plugin args((void));
void      oss_debugger args((UR_OBJECT user));
void      oss_debug_commands args((UR_OBJECT user));
void      oss_debug_allinput args((UR_OBJECT user));
void      oss_debug_plugindata args((UR_OBJECT user));
void      load_plugins args((void));

/* transport */
void      transport_plane args((UR_OBJECT user));
void      write_transport args((TR_OBJECT tr, char *str));
void      write_transport_except args((TR_OBJECT tr, char *str, UR_OBJECT user));
void      transport args((void));
TR_OBJECT create_transport args((void));
void      destruct_transport args((TR_OBJECT tr));

/* makra */
void      save_macros args((UR_OBJECT user));
void      get_macros args((UR_OBJECT user));
void      macros args((UR_OBJECT user, char *inpstr));
int       check_macros args((UR_OBJECT user, char *inpstr));
void      show_macros args((UR_OBJECT user));
void      delete_macro args((UR_OBJECT user, MC_OBJECT mc));
MC_OBJECT create_macro args((void));

/* Pueblo */
int       chck_pblo args((UR_OBJECT user, char *str));
int       contains_pueblo args((char *str));
void      pblo_exec args((UR_OBJECT user, char *inpstr));
void      pblo_jukebox args((UR_OBJECT user));
void      click_rm_access args((UR_OBJECT user));
void      disp_song args((UR_OBJECT user, int num));
void      pblo_listexits args((UR_OBJECT user));
int       audioprompt args((UR_OBJECT user, int prmpt, int pager));
void      query_img args((UR_OBJECT user, char *inpstr));
void      query_aud args((UR_OBJECT user, char *inpstr));

/* figlets - fonty */
#ifdef __STDC__
	char *myalloc args((size_t size));
#else
	char *myalloc args((int size));
#endif
void      figlet args((UR_OBJECT user, char *inpstr, int typ));
int       readfont args((char *fontname));
void      write_text_figlet args((UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm, char *fig_text, char *name, char *font));
void      fclearline args((void));
void      printline args((UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm));
int       addchar args((long c));
void      putstring args((UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm, char *string));
void      splitline args((UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm));
void      skiptoeol args((FILE *fp));
void      readfontchar args((FILE *file, long theord, char *line, int maxlen));
void      getletter args((long c));
void      write_broadcast_figlet args((UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm, char *fig_text));
void      list_fnt_files args((UR_OBJECT user));

/* money */
void      donate_cash args((UR_OBJECT));
void      show_money args((UR_OBJECT));
void      check_credit_updates args((void));
void      global_money args((UR_OBJECT));

/* menu */
int       setops args((UR_OBJECT user, char *inpstr));
// main
void      show_attributes args((UR_OBJECT user));
void      set_attributes args((UR_OBJECT user, char *inpstr));
// terminal
void      showattribs_term args((UR_OBJECT user));
void      set_terminal args((UR_OBJECT user, char *inpstr));
int       setops_term args((UR_OBJECT user, char *inpstr));
// bank
void      showattribs_bank args((UR_OBJECT user));
void      set_bank args((UR_OBJECT user, char *inpstr));
int       setops_bank args((UR_OBJECT user, char *inpstr));

/* debug */
#ifdef DEBUG
void      s_crash args((char *file, int line));
void      crash_dump args((void));
#endif

#endif /* prototypes.h */
