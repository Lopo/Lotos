/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                     Struktura usera pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __OBJ_UR_H__
#define __OBJ_UR_H__ 1

#include <time.h>

typedef struct user_ignore_struct {
	unsigned all		: 1;
	unsigned all_store	: 1;
	unsigned tells		: 1;
	unsigned logons		: 1;
	unsigned shouts		: 1;
	unsigned pics		: 1;
	unsigned wiz		: 1;
	unsigned greets		: 1;
	unsigned beeps		: 1;
	unsigned transp		: 1;
	unsigned funs		: 1;
	} USER_IGNORE;

typedef struct user_terminal_struct {
	unsigned bckg		: 1;
	unsigned txt		: 1;
	unsigned revers		: 1;
	unsigned blink		: 1;
	unsigned bold		: 1;
	unsigned underline	: 1;
	unsigned clear		: 1;
	unsigned music		: 1;
	unsigned xterm		: 1;
	unsigned checho		: 1;
	unsigned wrap		: 1;
	unsigned blind		: 1;
	unsigned pager		: 13;
	} USER_TERMINAL;

/* user variables - some are saved in the user file, and some are not */
struct user_struct {
  char name[USER_NAME_LEN+1],desc[USER_DESC_LEN+1],pass[PASS_LEN+6];
  char in_phrase[PHRASE_LEN+1],out_phrase[PHRASE_LEN+1];
  char buff[BUFSIZE],site[81],ipsite[81],last_site[81],page_file[81];
  char mail_to[WORD_LEN+1],revbuff[REVTELL_LINES][REVIEW_LEN+2];
  char afk_mesg[AFK_MESG_LEN+1],inpstr_old[REVIEW_LEN+1];
  char logout_room[ROOM_NAME_LEN+1],version[10];
  char copyto[MAX_COPIES][USER_NAME_LEN+1],invite_by[USER_NAME_LEN+1],date[80];
  char email[81],homepage[81],ignoreuser[MAX_IGNORES][USER_NAME_LEN+1],recap[USER_NAME_LEN+USER_NAME_LEN*3];
  char bw_recap[USER_NAME_LEN+1],call[USER_NAME_LEN+1],friend[MAX_FRIENDS][USER_NAME_LEN+1];
  char verify_code[80],afkbuff[REVTELL_LINES][REVIEW_LEN+2],editbuff[REVTELL_LINES][REVIEW_LEN+2];
  char samesite_check_store[ARR_SIZE];
  char *malloc_start,*malloc_end,icq[ICQ_LEN+1];
  int type,login,attempts,vis,prompt,command_mode,muzzled;
  int gender,hideemail,edit_line,warned,accreq,real_level;
  int afk,clone_hear,unarrest,arrestby,expire,lroom,monitor;
  int show_rdesc,alert,mail_verified,autofwd,editing,show_pass,pagecnt,pages[MAX_PAGES];
  int samesite_all_store;
  int port,site_port,socket,buffpos,filepos,remote_com,charcnt,misc_op,last_login_len;
  int edit_op,revline,level,wipe_from,wipe_to,logons,cmd_type,user_page_pos,user_page_lev;
  int age,misses,hits,kills,deaths,bullets,hps,afkline,editline;
  int lmail_lev,hwrap_lev,hwrap_id,hwrap_same,hwrap_func,gcoms[MAX_GCOMS],xcoms[MAX_XCOMS];
  struct room_struct *room,*invite_room,*wrap_room;
  struct user_struct *prev,*next,*owner;
  struct {
    int day,month,year,alert; char msg[REMINDER_LEN];
    } reminder[MAX_REMINDERS],temp_remind;
  time_t last_input,last_login,total_login,read_mail,t_expire;
  #ifdef NETLINKS
    struct netlink_struct *netlink,*pot_netlink;
  #endif

	/* PUEBLO ENHANCET SESSION VARIABLES */
	int pueblo, pueblo_mm, pueblo_pg, voiceprompt, pblodetect;
  
  int tmp_int;
  char *p_tmp_ch;
  struct macro_struct *first_macro, *last_macro;
  char prompt_str[PROMPT_LEN-1];
  char nameg[USER_NAME_LEN+6];
  char named[USER_NAME_LEN+6];
  char namea[USER_NAME_LEN+6];
  char namel[USER_NAME_LEN+6];
  char namei[USER_NAME_LEN+6];
	char namex[USER_NAME_LEN+6];
	char namey[USER_NAME_LEN+6];
	char namez[USER_NAME_LEN+6];
  /* alarm - budik */
  int atime, alarm;

  char murlist[MAX_MUSERS][USER_NAME_LEN+1]; /* multi user list */

  char ltell[USER_NAME_LEN+1];
  char restrict[MAX_RESTRICT+1];
  char *ign_word;
  struct user_struct *follow;
  long tcount, bcount;
  unsigned long auth_addr;
  int set_mode, set_op;
  int hwrap_pl;
  int who_type;
  char status;

	int money, bank, inctime, kradnutie;

	struct plugin_02x100_player *plugin_02x100;

	USER_IGNORE ignore;
	USER_TERMINAL terminal;
  };
typedef struct user_struct *UR_OBJECT;

#endif /* obj_ur.h */

