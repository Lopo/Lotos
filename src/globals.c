/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                     Globalne konstanty Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __GLOBALS_C__
#define __GLOBALS_C__ 1

#include <stdio.h>
#include <time.h>
#include <setjmp.h>

#include "define.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_tr.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "obj_pl.h"
#include "obj_syspp.h"

UR_OBJECT user_first,user_last;

struct {
  char name[USER_NAME_LEN+1],time[80];
  short int on;
  } 
last_login_info[LASTLOGON_NUM+1];

RM_OBJECT room_first,room_last;
TR_OBJECT transport_first, transport_last;

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
|        Tato sprava ti pola poslana prikazom .smail na talkri Star        |\n\
|    a toto je tvoj Auto-forward. Neodpovedaj priamo na tento mail !!!!    |\n\
|                                                                          |\n\
|  Star - talker spusteny na star.losys.net:7000    email talker@losys.net |\n\
+--------------------------------------------------------------------------+\n";


char *vrf_fwd_email=
"Vdaka za nastavnie tvojej email adresy. Teraz po overeni korektneho zadania\n
email adresy budes moct pouzivat forwardovanie sprav z talkra na tuto adresu.\n
Este ale budes musiet verifikovat prijatie tohoto emailu na samotnom talkri.\n\n
Verifikacny kod je: %s\n\n
Pouzi tento kod s prikazom '.verify' pri tvojom najblizsom prihlaseni na talker.
Potom mozes pouzit prikaz '.set' na zapnutie'vypnutie forvardovania .smail sprav.\n\n
Vdaka za zawislacenie na tomto talkri - dobre si to uzi !\n\n
                     Lopo\n\n";

char swear_words[MAX_SWEARS+1][WORD_LEN+1];


/* Other global variables */
char text[ARR_SIZE*2],vtext[ARR_SIZE*2];
char word[MAX_WORDS][WORD_LEN+1];
char wrd[8][81];
char progname[40],confile[140];
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
int use_hostsfile;


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
/* PEmote    */ "~FM",    /* Think     */ "~FY",    /* Tell User */ "~OL~FT",
/* Tell Self */ "~FY",    /* Tell Text */ "~RS",    /* Shout     */ "~OL~FY",
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
/* Server DNS      */ "www.fa.stuba.sk",
/* Server IP       */ "100.100.100.200",
/* Talker E-Mail   */ "talker@losys.net",
/* Talker Website  */ "http://100.100.100.200/talker/",
/* Sysop Real Name */ "Pavol Hluchy",
/* Sysop User Name */ "Fred",
/* Pueblo Web Dir. */ "media/",
/* Graphic Image   */ "img_title.gif",
/* Rootovske heslo */ "NUKyNCCLvgLH."
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

/* DEBUG */
#ifdef DEBUG
struct {
	char lastfile[512+1];
	int lastline;
	unsigned char n;
} crash[CRASH_HISTORY];
int crash_step;
#endif

/************ menu_tab *******************/
struct {
	char *name, *fname, *prompt0, *prompt1;
	} menu_tab[]={
		{"*", "*", ""},
		{"setup", "set_main", "~FGPozitie~FB =>~RS .set <param> <hodnota> ~FB<=\n", "~FRVloz volbu:~RS "},
		{"terminal", "set_term", "~FGPouzitie~FB =>~RS .terminal <param> <hodnota> ~FB<=\n", "~FRVloz volbu:~RS "},
		{"bank", "bank_ops", "~FGPouzitie~FB =>~RS .bank <operacia> <obnos> ~FB<=\n", "~FRVloz volbu:~RS "},
		{"*", "*", ""}
		};

char *help_style[NUM_HELP+1]={
	"*", "level", "function", "commands only"
	};

char *who_style[NUM_WHO+1]={
	"*", "NUTS 333", "Amnuts 221", "Short who", "Moe Byroom",
	"House Of Pleasure And Ecstacy", "Stairway To Heaven"
	};

struct {
	char cmenu, *type, *name, *desc;
	} set_tab[]={
		{' ', "show", "show", "ukaze aktualne nastavenia"},
		{'G', "gnd", "gender", "tvoje pohlavie (muz, zena)"},
		{'A', "age", "age", "nastavi aky tvoj vek budu useri vidiet"},
		{'E', "email", "email", "tvoja e-mail adresa"},
		{'M', "www", "homepage", "adresa tvojej homepage"},
		{'H', "hide", "hideemail", "prepne viditelnost tvojej e-mail adresy pre userov"},
		{'R', "room", "loginroom", "zapne prihlasenie do roomy v ktorej sa odhlasis"},
		{'F', "fwd", "autofwd", "zapne automaticke posielanie smail sprav na email"},
		{'S', "pswd", "password", "zapne zobrazovanie hesla pri prihlasovani"},
		{'D', "rdesc", "roomdesc", "zapne ignorovanie popisov miestnosti"},
		{'O', "cmd", "command", "typ zobrazovania zoznamu prikazov"},
		{'N', "recap", "recap", "nastavenie velkych pismen v nicku"},
		{'Q', "icq", "icq", "nastavi tvoje ICQ cislo"},
		{'T', "alert", "alert", "zapne upozornovanie na logovanie userov z notify listu"},
		{'I', "audio", "audio", "prepne 'Pueblo Audio Prompting'"},
		{'U', "ppa", "ppa", "prepne 'Pueblo Pager Audio'"},
		{'V', "voice", "voice", "Audio Prompt Gender (muz/zena)"},
		{'1', "mode", "mode", "Prepina KECACI a PRIKAZOVY mod"},
		{'2', "prompt", "prompt", "Nastavuje typ a vzhlad promptu"},
		{'3', "who", "who", "Nastavuje typ listingu userov"},
		{'*', "*", "*", ""}
		};
struct {
	char *name, *str;
	} prompt_tab[]={
		{"vypnuty", ""},
		{"standard", ""},
		{"*",""}
		};

char *color_mods[NUM_COLMODS+1]={
	"all off", "all on", "blink off", "bckg off", "beep off",
	"bckg&beep off"
	};

struct {
	char cmenu, *type, *name, *desc;
	} setterm_tab[]={
		{' ', "show", "show", "ukaze aktualne nastavenia"},
		{'B', "bckg", "backgrounds", "zapne/vypne pouzivanie farebneho pozadia"},
		{'T', "txt", "textcolors", "zapne/vypne pouzivanie farebneho pisma"},
		{'R', "revers", "revers", "zapne/vypne pouzivanie reverzovania farieb"},
		{'K', "blink", "blink", "zapne/vypne pouzivanie blikania textu"},
		{'O', "bold", "bold", "zapne/vypne pouzivanie zvyrazneneho textu"},
		{'U', "undln", "underline", "zapne/vypne pouzivanie podciarkovania"},
		{'C', "clear", "clear", "prepne sposob mazania obrazovky"},
		{'M', "music", "music", "zapne/vypne pouzivanie kodov na ansimusic"},
		{'X', "xterm", "xterm", "zapne/vypne kompatibilitu s xterm-om"},
		{'C', "checho", "charecho", "zapne/vypne echovanie pisanych znakov"},
		{'W', "wrap", "wrap", "zapne/vypne zalamovanie riadkov"},
		{'I', "blind", "blind", "vypne vsetky vypisy na obrazovku"},
		{'P', "pager", "pager", "nastavi pocet naraz zobrazenych riadkov"},
		{'*', "*", "*", ""}
		};

struct {
	char cmenu, *type, *name, *desc;
	} settab_bank[]={
		{'Z', "show", "show", "ukaze aktualny stav"},
		{'D', "dep", "deposit", "ulozi peniaze do banky"},
		{'W', "wit", "withdraw", "vyberie peniaze z banky"},
		{'S', "send", "send", "posle peniaze z uctu na iny ucet"},
		{'*', "*", "*", ""}
		};

#endif /* globals.c */
