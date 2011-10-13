/*****************************************************************************
                    Globalne konstanty OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#include "define.h"
#include "ur_obj.h"
#include "rm_obj.h"
#ifdef NETLINKS
	#include "nl_obj.h"
#endif
#include "sys_obj.h"
#include "pl_obj.h"
#include "syspp_obj.h"

UR_OBJECT user_first,user_last;

struct {
  char name[USER_NAME_LEN+1],time[80];
  short int on;
  } 
last_login_info[LASTLOGON_NUM+1];

RM_OBJECT room_first,room_last;

#ifdef NETLINKS
	NL_OBJECT nl_first,nl_last;
#endif


/* main user list structure */
struct user_dir_struct {
  char name[USER_NAME_LEN+1],date[80];
  short int level;
  struct user_dir_struct *next,*prev;
  };
struct user_dir_struct *first_dir_entry,*last_dir_entry;

/* main list of wizzes */
struct wiz_list_struct {
  char name[USER_NAME_LEN+1];
  short int level;
  struct wiz_list_struct *next,*prev;
  };
struct wiz_list_struct *first_wiz_entry,*last_wiz_entry;

/* command list */
struct command_struct {
  char name[15],alias[5]; /* 15 and 5 characters should be long enough */
  short int id,min_lev,function;
  int count;
  struct command_struct *next,*prev;
  };
struct command_struct *first_command,*last_command;
char cmd_history[16][128];


SYS_OBJECT amsys;

/* some general arrays being defined */
char *month[12]={
  "Januar","Februar","Marec","April","Maj","Jun",
  "Jul","August","September","Oktober","November","December"
  };

char *day[7]={
  "Nedela","Pondelok","Utorok","Streda","Stvrtok","Piatok","Sobota"
  };

char *noyes1[]={ "NIE","ANO" };
char *noyes2[]={ "NIE","ANO" };
char *offon[]={ "VYP","ZAP" };
char *minmax[]={"VYP","MIN","MAX"};
char *sex[]={"Novacik","Muz","Zena"};
char *opcl[]={"CLOSED", "OPEN"};

char *crypt_salt="NU";

/* you can change this for whatever sig you want - of just "" if you don't want
   to have a sig file attached at the end of emails */
char *talker_signature=
"\n\n+--------------------------------------------------------------------------+\n\
|  This message has been smailed to you on The Amnuts Talker, and this is  |\n\
|      your auto-forward.  Please do not reply directly to this email.     |\n\
|                                                                          |\n\
|               Amnuts - A talker running at foo.bar.com 666               |\n\
|         email 'me@my.place' if you have any questions/comments           |\n\
+--------------------------------------------------------------------------+\n";


char *vrf_fwd_email=
"Thank you for setting your email address, and now that you have done so you are\n
able to use the auto-forwarding function on The Talker to have any smail sent to\n
your email address.  To be able to do this though you must verify that you have\n
received this email.\n\n
Your verification code is: %s\n\n
Use this code with the 'verify' command when you next log onto the talker.\n
You will then have to use the 'set' command to turn on/off auto-forwarding.\n\n
Thank you for coming to our talker - we hope you enjoy it!\n\n   The Staff.\n\n";

char swear_words[MAX_SWEARS+1][WORD_LEN+1];


/* Other global variables */
char text[ARR_SIZE*2],vtext[ARR_SIZE*2];
char word[MAX_WORDS][WORD_LEN+1];
char wrd[8][81];
char progname[40],confile[40];
jmp_buf jmpvar;
#ifdef NETLINKS
	int listen_sock[3],port[3],port_total=3;
#else 
	int listen_sock[2],port[2],port_total=2;
#endif
#ifdef NETLINKS
	char verification[SERV_NAME_LEN+1];
#endif
int tyear,tmonth,tday,tmday,twday,thour,tmin,tsec;
int destructed,force_listen,no_prompt,logon_flag;
int config_line,word_count;


/* Letter array map - for greet() */
int biglet[26][5][5] = {
  {{0,1,1,1,0},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
  {{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0}},
  {{0,1,1,1,1},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{0,1,1,1,1}},
  {{1,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,0}},
  {{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
  {{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
  {{0,1,1,1,0},{1,0,0,0,0},{1,0,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
  {{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1},{1,0,0,0,1},{1,0,0,0,1}},
  {{0,1,1,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
  {{0,0,0,0,1},{0,0,0,0,1},{0,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
  {{1,0,0,0,1},{1,0,0,1,0},{1,0,1,0,0},{1,0,0,1,0},{1,0,0,0,1}},
  {{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,0,0,0,0},{1,1,1,1,1}},
  {{1,0,0,0,1},{1,1,0,1,1},{1,0,1,0,1},{1,0,0,0,1},{1,0,0,0,1}},
  {{1,0,0,0,1},{1,1,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{1,0,0,0,1}},
  {{0,1,1,1,0},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{0,1,1,1,0}},
  {{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,0,0},{1,0,0,0,0}},
  {{0,1,1,1,0},{1,0,0,0,1},{1,0,1,0,1},{1,0,0,1,1},{0,1,1,1,0}},
  {{1,1,1,1,0},{1,0,0,0,1},{1,1,1,1,0},{1,0,0,1,0},{1,0,0,0,1}},
  {{0,1,1,1,1},{1,0,0,0,0},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
  {{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
  {{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,0,0,0,1},{1,1,1,1,1}},
  {{1,0,0,0,1},{1,0,0,0,1},{0,1,0,1,0},{0,1,0,1,0},{0,0,1,0,0}},
  {{1,0,0,0,1},{1,0,0,0,1},{1,0,1,0,1},{1,1,0,1,1},{1,0,0,0,1}},
  {{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{1,0,0,0,1}},
  {{1,0,0,0,1},{0,1,0,1,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0}},
  {{1,1,1,1,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,1,1,1,1}}
  };

/* Symbol array map - for greet() */
int bigsym[32][5][5] = {
  {{0,0,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0}},
  {{0,1,0,1,0},{0,1,0,1,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
  {{0,1,0,1,0},{1,1,1,1,1},{0,1,0,1,0},{1,1,1,1,1},{0,1,0,1,0}},
  {{0,1,1,1,1},{1,0,1,0,0},{0,1,1,1,0},{0,0,1,0,1},{1,1,1,1,0}},
  {{1,1,0,0,1},{1,1,0,1,0},{0,0,1,0,0},{0,1,0,1,1},{1,0,0,1,1}},
  {{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
  {{0,0,1,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
  {{0,0,1,1,0},{0,1,0,0,0},{0,1,0,0,0},{0,1,0,0,0},{0,0,1,1,0}},
  {{0,1,1,0,0},{0,0,0,1,0},{0,0,0,1,0},{0,0,0,1,0},{0,1,1,0,0}},
  {{1,0,1,0,1},{0,1,1,1,0},{1,1,1,1,1},{0,1,1,1,0},{1,0,1,0,1}},
  {{0,0,1,0,0},{0,0,1,0,0},{1,1,1,1,1},{0,0,1,0,0},{0,0,1,0,0}},
  {{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,1,0,0,0},{1,1,0,0,0}},
  {{0,0,0,0,0},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{0,0,0,0,0}},
  {{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{1,1,0,0,0},{1,1,0,0,0}},
  {{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{1,0,0,0,0}},
  {{0,1,1,1,0},{1,0,0,1,1},{1,0,1,0,1},{1,1,0,0,1},{0,1,1,1,0}},
  {{0,0,1,0,0},{0,1,1,0,0},{0,0,1,0,0},{0,0,1,0,0},{0,1,1,1,0}},
  {{1,1,1,1,0},{0,0,0,0,1},{0,1,1,1,0},{1,0,0,0,0},{1,1,1,1,1}},
  {{1,1,1,1,0},{0,0,0,0,1},{0,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
  {{0,0,1,1,0},{0,1,0,0,0},{1,0,0,1,0},{1,1,1,1,1},{0,0,0,1,0}},
  {{1,1,1,1,1},{1,0,0,0,0},{1,1,1,1,0},{0,0,0,0,1},{1,1,1,1,0}},
  {{0,1,1,1,0},{1,0,0,0,0},{1,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
  {{1,1,1,1,1},{0,0,0,0,1},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0}},
  {{0,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0},{1,0,0,0,1},{0,1,1,1,0}},
  {{1,1,1,1,1},{1,0,0,0,1},{1,1,1,1,1},{0,0,0,0,1},{0,0,0,0,1}},
  {{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0}},
  {{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,1,0,0,0}},
  {{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0},{0,0,1,0,0},{0,0,0,1,0}},
  {{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0},{1,1,1,1,1},{0,0,0,0,0}},
  {{0,1,0,0,0},{0,0,1,0,0},{0,0,0,1,0},{0,0,1,0,0},{0,1,0,0,0}},
  {{0,1,1,1,1},{0,0,0,0,1},{0,0,1,1,1},{0,0,0,0,0},{0,0,1,0,0}},
  {{0,1,0,0,0},{1,0,1,1,1},{1,0,1,0,1},{1,0,1,1,1},{0,1,1,1,0}}
  };

 /*****************************************************************************
            Calendar code taken from Way Out West version 4.0.0
                      Copyright (C) Andrew Collington

               based upon scalar date routines by Ray Gardner   
 *****************************************************************************/
int cal_days[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char *cal_daynames[8]={"Ned","Pon","Uto","Str","Stv","Pia","Sob","Ned"};



char *no_leave="noleave";

PL_OBJECT plugin_first, plugin_last;
CM_OBJECT cmds_first, cmds_last;
SYSPP_OBJECT syspp;

/* Modular ColorCode Array
   -----------------------
   Changing these values will alter the color layout in your talker.
   You do not need to go through each line in the .c file to do this.
   Simply change the values in the following array. ORDER IS IMPORTANT,
   so please be careful when altering.  The description of the item
   you are altering is listed to the LEFT of the item.  */

char *colors[]={
/* Default   */ "~RS",    /* Highlight */ "~BB",    /* Text      */ "~FW",
/* Bold      */ "~OL",    /* System    */ "~FR",    /* Sys. Bold */ "~OL~FY",
/* Warning   */ "~FM",    /* Who User  */ "~FY",    /* Who Info  */ "~FT",
/* People Hi.*/ "~BR",    /* People    */ "~FR",    /* User      */ "~FR",
/* Self      */ "~FT",    /* Emote     */ "~RS",    /* SEmote    */ "~FT",
/* PEmote    */ "~FM",    /* Think     */ "~FY",    /* Tell User */ "~FG",
/* Tell Self */ "~FY",    /* Tell Text */ "~RS",    /* Shout     */ "~FT",
/* Mail Head */ "~OL~FG", /* Mail Date */ "~RS~FY", /* BoardHead */ "~OL~FT",
/* BoardDate */ "~RS"
};

/* System Information
   ------------------
   Set the following values to complete the TalkerOS setup.
   It is important that you fill in each value with the correct
   information. */
char *reg_sysinfo[13]={
/* NEMENIT !!!     */ "*",
/* Talker meno     */ "Star talker",
/* Seriove cislo   */ "000000000000",
/* Registrovany user */ "Lopo",
/* Server DNS      */ "star.sjf.stuba.sk",
/* Server IP       */ "127.0.0.1",
/* Talker E-Mail   */ "osstar@star.sjf.stuba.sk",
/* Talker Website  */ "http://star.sjf.stuba.sk/osstar",
/* Sysop Real Name */ "Pavol Hluchy",
/* Sysop User Name */ "Lopo",
/* Pueblo Web Dir. */ "media/",
/* Graphic Image   */ "img_title.gif",
/* Rootovske heslo */ "NUlZHJjWuC4uQ"
};


/******************** Figlet globals & defines ************************/
long *inchrline;  /* Alloc'd inchr inchrline[inchrlinelenlimit+1]; */

int inchrlinelen,inchrlinelenlimit;

char **currchar;
int currcharwidth;
char **outline;    /* Alloc'd char outline[charheight][outlinelenlimit+1]; */
int outlinelen;
int justification,right2left;
int outputwidth;
int outlinelenlimit;
char hardblank;
int charheight,defaultmode;


char *restrict_string="GMPDZUKHSWRCVX"; /* restrictions codes */
char susers_restrict[MAX_RESTRICT+1]; /* superior users default restrictions */     
