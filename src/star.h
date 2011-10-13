/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

 /* attempt to stop freezing time.  Thanks to Arny ('Paris' code creator)
   and Cygnus ('Ncohafmuta' code creator) for this */
#if !defined __GLIBC__ || __GLIBC__ < 2
#define SIGNAL(x,y) signal(x,y)
#else
#define SIGNAL(x,y) sysv_signal(x,y)
#endif

extern UR_OBJECT user_first,user_last;

/* structure to see who last logged on */
extern struct {
  char name[USER_NAME_LEN+1],time[80];
  short int on;
  } last_login_info[];

extern RM_OBJECT room_first,room_last;

#ifdef NETLINKS
	extern NL_OBJECT nl_first,nl_last;
#endif

/* main user list structure */
struct user_dir_struct {
  char name[USER_NAME_LEN+1],date[80];
  short int level;
  struct user_dir_struct *next,*prev;
  };
extern struct user_dir_struct *first_dir_entry,*last_dir_entry;

/* main list of wizzes */
struct wiz_list_struct {
  char name[USER_NAME_LEN+1];
  short int level;
  struct wiz_list_struct *next,*prev;
  };
extern struct wiz_list_struct *first_wiz_entry,*last_wiz_entry;

/* command list */
struct command_struct {
  char name[15],alias[5]; /* 15 and 5 characters should be long enough */
  short int id,min_lev,function;
  int count;
  struct command_struct *next,*prev;
  };
extern struct command_struct *first_command,*last_command;
extern char cmd_history[16][128];

extern SYS_OBJECT amsys;

extern SYSPP_OBJECT syspp;


/* levels used on the talker */

extern struct {
  char *name;
  char *alias;
  } user_level[];

/* default rooms */
char *default_jail="windows";
char *default_warp="procesor";
char *default_shoot="null";

/* The rooms listed here are just examples of what can be added
   You may add more or remove as many as you like, but you MUST
   keep the stopping clause in */
struct { 
  char *name; int level; 
  } priv_room[]={
    { "linux", ARCH }, /* a room for wizzes+ only */
    { "*", 0 } /* stopping clause */
    };

/* you can set a standard room desc for those people who are creating a new
   personal room */
char *default_personal_room_desc=
"The walls are stark and the floors are bare.  Either the owner is new\n\
or just plain lazy.  Perhaps they just don't know how to use the .mypaint\n\
command yet?\n";


/* colour code values */
struct {
  char *esc_code;
  char *txt_code;
  } colour_codes[]={
    /* Standard stuff */
    { "\033[0m", "RS" }, /* reset */
    { "\033[1m", "OL" }, /* bold */
    { "\033[4m", "UL" }, /* underline */
    { "\033[5m", "LI" }, /* blink */
    { "\033[7m", "RV" }, /* reverse */
    /* Foreground colour */
    { "\033[30m", "FK" }, /* black */
    { "\033[31m", "FR" }, /* red */
    { "\033[32m", "FG" }, /* green */
    { "\033[33m", "FY" }, /* yellow */
    { "\033[34m", "FB" }, /* blue */
    { "\033[35m", "FM" }, /* magenta */
    { "\033[36m", "FT" }, /* turquiose */
    { "\033[37m", "FW" }, /* white */
    /* Background colour */
    { "\033[40m", "BK" }, /* black */
    { "\033[41m", "BR" }, /* red */
    { "\033[42m", "BG" }, /* green */
    { "\033[43m", "BY" }, /* yellow */
    { "\033[44m", "BB" }, /* blue */
    { "\033[45m", "BM" }, /* magenta */
    { "\033[46m", "BT" }, /* turquiose */
    { "\033[47m", "BW" }, /* white */
    /* Beep - ascii bell */
    { "\07",      "BP" }, /* beep */
    /* Clear screen */
    { "\033[2J",  "CS" }, /* Clear screen */
    /* Ansi music */
    { "\033[M",   "MS" },
    { "\0x0E",    "ME" },
  };
#define NUM_COLS SIZEOF(colour_codes)


/* some general arrays being defined */
extern char *month[12];
extern char *day[7];

extern char *noyes1[];
extern char *noyes2[];
extern char *offon[];
extern char *minmax[];
extern char *sex[];





/* other strings used on the talker */

extern char *syserror, *invisenter, *invisleave, *invisname;
extern char *nosuchroom, *nosuchuser, *notloggedon;
extern char *talker_name;
extern char *crypt_salt;

extern char *talker_signature;

extern char swear_words[MAX_SWEARS+1][WORD_LEN+1];
extern char *swear_censor;
extern char *noswearing;

extern char text[ARR_SIZE*2], vtext[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern char wrd[8][81];
extern char progname[], confile[];
extern jmp_buf jmpvar;
#ifdef NETLINKS
	extern char verification[];
#endif
extern int listen_sock[], port[], port_total;
extern int tyear, tmonth, tday, tmday, twday, thour, tmin, tsec;
extern int destructed, force_listen, no_prompt, logon_flag;
extern int config_line, word_count;

extern int biglet[26][5][5], bigsym[32][5][5];

extern int cal_days[];
extern char *cal_daynames[];

extern PL_OBJECT plugin_first;
extern CM_OBJECT cmds_first;

extern char *colors[];

extern char *reg_sysinfo[];

extern char susers_restrict[];
extern char *restrict_string;

/* prompts */
extern char *help_levelname_style, *help_header, *help_footer1, *help_footer2;
extern char *more_prompt, *more_prompt_n, *enterprompt;
extern char *session_swap, *unknown_command, *edit_markers;
extern char *default_inphr, *default_outphr, *default_desc;
extern char *login_timeout, *login_quit, *login_welcome, *login_attempts;
extern char *login_shortname, *login_longname, *login_swname;
extern char *login_prompt, *login_lettersonly, *login_rules_prompt;
extern char *login_pbloname;
extern char *password_short, *password_long, *password_again;
extern char *password_wrong, *password_nomatch, *password_prompt;
extern char *password_bad;
extern char *user_banned_prompt, *shout_cbuff_prompt;
extern char *flood_prompt, *flood_prompt_r, *autopromo_prompt;
extern char *eq_hi_lev_prompt, *this_m_ban_prompt;
extern char *nuts_credits, *amnuts_credits, *star_credits;
extern char *sys_port_closed, *wiz_port_closed;
