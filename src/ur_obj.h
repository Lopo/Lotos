/*****************************************************************************
                    Struktura pouzivatela v OS Star v1.1.0
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

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
  int type,login,attempts,vis,ignall,prompt,command_mode,muzzled,charmode_echo;
  int gender,hideemail,edit_line,warned,accreq,ignall_store,igntells,real_level;
  int afk,clone_hear,colour,ignshouts,unarrest,arrestby,pager,expire,lroom,monitor;
  int show_rdesc,wrap,alert,mail_verified,autofwd,editing,show_pass,pagecnt,pages[MAX_PAGES];
  int ignpics,ignlogons,ignwiz,igngreets,ignbeeps,samesite_all_store;
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
  int igntr;
  char prompt_str[PROMPT_LEN-1];
  char nameg[USER_NAME_LEN+6];
  char named[USER_NAME_LEN+6];
  char namea[USER_NAME_LEN+6];
  char namel[USER_NAME_LEN+6];
  char namei[USER_NAME_LEN+6];
  /* alarm - budik */
  int atime, alarm;

  char murlist[MAX_MUSERS][USER_NAME_LEN+1]; /* multi user list */

  char ltell[USER_NAME_LEN+1];
  char restrict[MAX_RESTRICT+1];
  char *ign_word;
  struct user_struct *follow;
  long tcount, bcount;
  int ignfuns, xterm;
  unsigned long auth_addr;
  int set_mode, set_op;
  int hwrap_pl;
  int who_type;
  char status;

	struct plugin_02x100_player *plugin_02x100;
  };
typedef struct user_struct *UR_OBJECT;
