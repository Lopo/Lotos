/*****************************************************************************
                     Hlavne definicie pre OS Star v1.0.0
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

/* cislo verzie - TVERSION moze mat akekolvek cislo,
   ostatne musia byt nezmenene */

#define TVERSION "2.0.1"
#define OSSVERSION "1.0.0"
#define AMNUTSVER "2.2.1"
#define NUTSVER "3.3.3"
#define USERVER "0.1"
#define RUN_VER "100"

#define FSTART "???"

/* general directories */
#define ROOTDIR "/osstar100"
#define DATAFILES "datafiles"
#define CONFFILES "conffiles"
#define MAPFILES "mapfiles"
#define HELPFILES "helpfiles"
#define MAILSPOOL "mailspool"
#define MISCFILES "miscfiles"
#define PICTFILES "pictfiles"
#define MOTDFILES "motds"
#define DUMPFILES "dumpfiles"
#define TEXTFILES "textfiles"
#define ADMINFILES "adminfiles"
#define LOGFILES "logfiles"
#define BOTFILES "botfiles"
#define ROOMFILES "roomfiles"
#define PLFILES "plfiles"
#define TRFILES "trfiles"
#define VOTEFILES "votefiles"
#define TEMPFILES "tempfiles"
#define FIGLET_FONTS "fonts"
#define KILLMSGS "killmsgs"
#define COUNTFILES "counters"

/* user directories */
#define USERFILES "userfiles"
#define USERMAILS "mail"
#define USERPROFILES "profiles"
#define USERFRIENDS "friends"
#define USERHISTORYS "historys"
#define USERCOMMANDS "xgcoms"
#define USERMACROS "macros"
#define USERROOMS "rooms"
#define USERREMINDERS "reminders"
#define USERPLDATAS "pldatas"

/* files */
#define CONFIGFILE "config"
#define NEWSFILE "newsfile"
#define SITEBAN "siteban"
#define USERBAN "userban"
#define NEWBAN "newban"
#define SUGBOARD "suggestions"
#define RULESFILE "rules"
#define WIZRULESFILE "wizrules"
#define SHOWFILES "showfiles"
#define SHOWAFILES "showafiles"
#define SWEARFILE "swears"
#define LEVELFILE "levels"
#define FAQFILE "faq"
#define TALKERSFILE "talkers"
#define TCOUNTER "tcounter"
#define MCOUNTER "mcounter"
#define RESTARTFILE "restartx"

/* fun files */
#define HUGFILE "hug"
#define KISSFILE "kiss"
#define WAKEFILE "wake"

/* system logs */
#define LAST_CMD   "last_command"
#define LASTCMDLOGS "lastcmdlogs"
#define MAINSYSLOG "syslog"
#define NETSYSLOG  "netlog"
#define REQSYSLOG  "reqlog"
#define DEBSYSLOG  "deblog"
#define ERRSYSLOG  "errlog"
#define SYSLOG 0
#define REQLOG 1
#define NETLOG 2
#define DEBLOG 3
#define ERRLOG 4			/* errors */

/* general defines */
#define OUT_BUFF_SIZE  1000	/* input buffer size */
#define MAX_WORDS        10	/* max. words processed by commands */
#define WORD_LEN         80	/* length of words */
#define ARR_SIZE       1000	/* array chars size */
#define MAX_LINES        15	/* max. lines in editor */
#define REVIEW_LINES     30	/* review conversation buffer lines */
#define REVTELL_LINES    20	/* review .tells buffer lines */
#define REVIEW_LEN      400	/* review conversation buffer line capacity */
#define BUFSIZE        1000
#define ROOM_NAME_LEN    20	/* room name length */
#define ROOM_LABEL_LEN    5	/* room label length */
#define SBOFF             0
#define SBMIN             1
#define SBMAX             2
#define LASTLOGON_NUM     5
#define LOGIN_FLOOD_CNT  20
#define MAX_SWEARS       20	/* max. size of swears list */

/* netlink defines */
#ifdef NETLINKS
  #define SERV_NAME_LEN  80	/* server name length */
  #define SITE_NAME_LEN  80	/* site name length */
  #define VERIFY_LEN     20	/* verify string length */
  #define UNCONNECTED     0
  #define INCOMING        1
  #define OUTGOING        2
  #define DOWN            0
  #define VERIFYING       1
  #define UP              2
  #define ALL             0
  #define IN              1
  #define OUT             2
#endif

/* user defines */
#define USER_NAME_LEN    12 /* user name maximum length */
#define USER_MIN_LEN      3 /* user name minimum length */
#define USER_DESC_LEN    35 /* user desc length */
#define AFK_MESG_LEN     60 /* max. length of AFK message */
#define PHRASE_LEN       40 /* in/out phrase length */
#define PASS_LEN         20 /* only the 1st 8 chars will be used by crypt() though */
#define PASS_MIN_LEN      3 /* min. password length */
#define ROOM_DESC_LEN (MAX_LINES*80)+MAX_LINES /* MAX_LINES lines of 80 chars each + MAX_LINES nl */
#define TOPIC_LEN        60 /* room topic length */
#define ICQ_LEN          20
#define NEUTER            0
#define MALE              1
#define FEMALE            2
#define NEWBIE_EXPIRES   20 /* days */
#define USER_EXPIRES     40 /* days */
#define SCREEN_WRAP      80 /* how many characters to wrap to */
#define MAX_COPIES        6 /* of smail */
#define MAX_FRIENDS      10
#define MAX_IGNORES      10 /* number of users you can ignore */
#define MAX_XCOMS        10
#define MAX_GCOMS        10
#define MAX_PAGES      1000 /* should be enough! */
#define MAX_REMINDERS    30
#define REMINDER_LEN     70
#define MAX_MUSERS       10 /* maximalny pocet userov - adresatov v jednom prikaze */

/* rooms */
#define MAX_LINKS        20 /* max. links from a room */
#define PUBLIC            0 /* type of rooms access */
#define PRIVATE           1
#define FIXED             2
#define FIXED_PUBLIC      2
#define FIXED_PRIVATE     3
#define PERSONAL_UNLOCKED 4
#define PERSONAL_LOCKED   5
#define ROOT_CONSOLE     10

/* levels */
#define L_0    0
#define L_1    1
#define L_2    2
#define L_3    3
#define L_4    4
#define L_5    5
#define L_6    6
#define L_7    7
#define L_8    8
#define L_9    9
#define L_10  10
#define L_11  11
#define L_12  12

#define JAILED 0
#define NEW    1
#define USER   2
#define SUPER  4
#define WIZ    7
#define ARCH   9
#define GOD    10

#define BOT 11
#define ROOT 12
#define SYSOP 12
#define RETIRE_LIST "retired_wiz"

/* user and clone types */
#define USER_TYPE 0
#define CLONE_TYPE 1
#define REMOTE_TYPE 2
#define BOT_TYPE 3
#define CLONE_HEAR_NOTHING 0
#define CLONE_HEAR_SWEARS 1
#define CLONE_HEAR_ALL 2

/* logon prompt stuff */
#define LOGIN_ATTEMPTS 3
#define LOGIN_NAME 1
#define LOGIN_PASSWD 2
#define LOGIN_CONFIRM 3
#define LOGIN_PROMPT 4

/* some macros that are used in the code */
/* these are for grammer */
#define PLTEXT_S(n) &"s"[(1==(n))]
#define PLTEXT_ES(n) &"es"[(1==(n))<<1]
#define PLTEXT_IS(n) ((1==(n))?"is":"are")
#define PLTEXT_WAS(n) ((1==(n))?"was":"were")

#define SIZEOF(table) (sizeof(table)/sizeof(table[0]))
#define MYSTRLEN(x) ((int)strlen(x)) /* Eliminate ANSI problem - pre fonty */

/* these are for bit manipulation */
#define BIT_BOOL(x) (!(!(x)))
#define BIT_SET(arg,pos) ((arg) | (1L << (pos)))
#define BIT_CLR(arg,pos) ((arg) & ~(1L << (pos)))
#define BIT_TEST(arg,pos) BIT_BOOL((arg) & (1L << (pos)))
#define BIT_FLIP(arg,pos) ((arg) ^ (1L << (pos)))


/* attempt to stop freezing time.  Thanks to Arny ('Paris' code creator)
   and Cygnus ('Ncohafmuta' code creator) for this */
#if !defined(__GLIBC__) || (__GLIBC__ < 2)
#define SIGNAL(x,y) signal(x,y)
#else
#define SIGNAL(x,y) sysv_signal(x,y)
#endif

/* Define ISO to be 1 for ISO (Mon-Sun) calendars
   ISO defines the first week with 4 or more days in it to be week #1.
   */
#ifndef ISO
 #define ISO 1
#endif

#if (ISO!=0 && ISO!=1)
 #error ISO must be set to either 0 or 1
#endif


#define NOLEAVE "noleave" /* oznacenie roomy bez vychodu v configu */
#define NUM_COLMODS 5 /* pocet farebnych modov */

/* Define Modular ColorCode Array Index */
#define CDEFAULT 0
#define CHIGHLIGHT 1
#define CTEXT 2
#define CBOLD 3
#define CSYSTEM 4
#define CSYSBOLD 5
#define CWARNING 6
#define CWHOUSER 7
#define CWHOINFO 8
#define CPEOPLEHI 9
#define CPEOPLE 10
#define CUSER 11
#define CSELF 12
#define CEMOTE 13
#define CSEMOTE 14
#define CPEMOTE 15
#define CTHINK 16
#define CTELLUSER 17
#define CTELLSELF 18
#define CTELL 19
#define CSHOUT 20
#define CMAILHEAD 21
#define CMAILDATE 22
#define CBOARDHEAD 23
#define CBOARDDATE 24

/* Definicie systemovych informacii OS Star */
#define TALKERNAME 1
#define SERIALNUM  2
#define REGUSER    3
#define SERVERDNS  4
#define SERVERIP   5 
#define TALKERMAIL 6
#define TALKERHTTP 7
#define SYSOPNAME  8
#define SYSOPUNAME 9
#define PUEBLOWEB 10
#define PUEBLOPIC 11
#define SYSOPPASSWD 12

/* makra */
#define MC_NAME_LEN 10
#define MC_COM_LEN 50

/* prompt */
#define PROMPT_LEN 50

/* figlets - fonty */
#define MAXFIRSTLINELEN 1000

/* spravy */
#define WAKEMSG_LEN 50


/* Restrictions stuff */
#define MAX_RESTRICT   	14               /* max. restrictions */

#define RESTRICT_GO    	0                /* .go */
#define RESTRICT_MOVE  	1		/* .move user */
#define RESTRICT_PROM  	2		/* .promote */
#define RESTRICT_DEMO  	3		/* .demote */
#define RESTRICT_MUZZ  	4		/* .muzzle */
#define RESTRICT_UNMU  	5		/* .unmuzzle */
#define RESTRICT_KILL  	6		/* .kill */
#define RESTRICT_HELP  	7		/* access .help */
#define RESTRICT_SUIC  	8		/* .suicide */
#define RESTRICT_WHO   	9		/* .who */
#define RESTRICT_RUN  	10		/* .run commands  - nepouzite*/
#define RESTRICT_CLON 	11		/* .create, .destroy clones */
#define RESTRICT_VIEW 	12		/* .review, .revtell */
#define RESTRICT_EXEC	13		/* execution of commands */
#define RESTRICT_MASK  ".............." /* mask used by load_user_details() */

#define MIN_LEV_AUTORST WIZ	/* minumum level to set default restrictions */
#define MIN_LEV_NOSWR   GOD	/* minimum level to ignore swear_action() */

/* pre zalohovanie talkra */
#define BACKUPDIR "backups"
#define BACKUPFILE "backup"
