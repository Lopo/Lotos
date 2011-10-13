/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                     Hlavne definicie pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __DEFINE_H__
#define __DEFINE_H__ 1

/* cislo verzie talkra - TVERSION moze mat akekolvek cislo,
   ostatne musia byt nezmenene */

#define TVERSION "1.0.0"
#define OSSVERSION "1.2.0"
#define AMNUTSVER "2.2.1"
#define NUTSVER "3.3.3"
#define USERVER "0.12"
#define RUN_VER "120"

#define FSTART "???"

/* general directories */
#define ROOTDIR "/home/lotos/lotos120"
#define DATAFILES ROOTDIR"/datafiles"
#define CONFFILES DATAFILES"/conffiles"
#define MAPFILES DATAFILES"/mapfiles"
#define HELPFILES DATAFILES"/helpfiles"
#define PLHELPFILES PLFILES"/helpfiles"
#define MAILSPOOL DATAFILES"/mailspool"
#define MISCFILES DATAFILES"/miscfiles"
#define PICTFILES DATAFILES"/pictfiles"
#define MOTDFILES DATAFILES"/motds"
#define DUMPFILES ROOTDIR"/dumpfiles"
#define TEXTFILES DATAFILES"/textfiles"
#define ADMINFILES TEXTFILES"/adminfiles"
#define LOGFILES ROOTDIR"/logfiles"
#define ROOMFILES DATAFILES"/roomfiles"
#define PLFILES DATAFILES"/plfiles"
#define TRFILES DATAFILES"/trfiles"
#define VOTEFILES DATAFILES"/votefiles"
#define TEMPFILES ROOTDIR"/tempfiles"
#define FIGLET_FONTS DATAFILES"/fonts"
#define KILLMSGS DATAFILES"/killmsgs"
#define COUNTFILES DATAFILES"/counters"
#define SCRFILES DATAFILES"/screens"
#define BINFILES ROOTDIR"/bin"

/* user directories */
#define USERFILES ROOTDIR"/userfiles"
#define USERMAILS USERFILES"/mail"
#define USERPROFILES USERFILES"/profiles"
#define USERFRIENDS USERFILES"/friends"
#define USERHISTORYS USERFILES"/historys"
#define USERCOMMANDS USERFILES"/xgcoms"
#define USERMACROS USERFILES"/macros"
#define USERROOMS USERFILES"/rooms"
#define USERREMINDERS USERFILES"/reminders"
#define USERPLDATAS USERFILES"/pldatas"

/* files */
#define CONFIGFILE CONFFILES"/config"
#define NEWSFILE MISCFILES"/newsfile"
#define SITEBAN MISCFILES"/siteban"
#define USERBAN MISCFILES"/userban"
#define NEWBAN MISCFILES"/newban"
#define SUGBOARD MISCFILES"/suggestions"
#define RULESFILE MISCFILES"/rules"
#define WIZRULESFILE MISCFILES"/wizrules"
#define SHOWFILES MISCFILES"/showfiles"
#define SHOWAFILES MISCFILES"/showafiles"
#define SWEARFILE MISCFILES"/swears"
#define LEVELFILE MISCFILES"/levels"
#define FAQFILE MISCFILES"/faq"
#define TALKERSFILE MISCFILES"/talkers"
#define TCOUNTER COUNTFILES"/tcounter"
#define MCOUNTER COUNTFILES"/mcounter"
#define RESTARTFILE TEMPFILES"/restartx"
#define KILLLIST MISCFILES"/killmsgs"
#define PICTLIST MISCFILES"/pictlist"
#define MAINHELP HELPFILES"/mainhelp"
#define FONTLIST MISCFILES"/fontslist"
#define CREDITS MISCFILES"/credits"
#define PIDFILE ROOTDIR"/star.pid"
#define HOSTSFILE MISCFILES"/hostsfile"
#define KILLFILE BINFILES"/kill"

/* fun files */
#define HUGFILE MISCFILES"/hug"
#define KISSFILE MISCFILES"/kiss"
#define WAKEFILE MISCFILES"/wake"

/* system logs */
#define LAST_CMD   "last_command"
#define LASTCMDLOGS "lastcmdlogs"
#define MAINSYSLOG "syslog"
#define NETSYSLOG  "netlog"
#define REQSYSLOG  "reqlog"
#ifdef DEBUG
	#define DEBSYSLOG  "deblog"
#endif
#define ERRSYSLOG  "errlog"
#define SYSLOG 0
#define REQLOG 1
#define NETLOG 2
#ifdef DEBUG
	#define DEBLOG 3
#endif
#define ERRLOG 4			/* errors */

/* general defines */
#define OUT_BUFF_SIZE  1000	/* input buffer size */
#define MAX_WORDS        10	/* max. words processed by commands */
#define WORD_LEN         80	/* length of words */
#define ARR_SIZE       1000	/* array chars size */
#define MAX_LINES        20	/* max. lines in editor */
#define REVIEW_LINES     30	/* review conversation buffer lines */
#define REVTELL_LINES    30	/* review .tells buffer lines */
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
#define SITE_NAME_LEN    80	/* site name length */
#ifdef NETLINKS
  #define SERV_NAME_LEN  80	/* server name length */
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
#define ROOM_DESC_LEN (MAX_LINES*100)+MAX_LINES /* MAX_LINES lines of 80 chars each + MAX_LINES nl */
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
#define RETIRE_LIST USERFILES"/retired_wiz"

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

/* Definicie systemovych informacii Lotos */
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

/* minimum levels */
#define MIN_LEV_AUTORST WIZ	/* to set default restrictions */
#define MIN_LEV_NOSWR   GOD	/* to ignore swear_action() */
#define MIN_LEV_BLIND   ARCH    /* to use .terminal blind */

/* pre zalohovanie talkra */
#define BACKUPDIR ROOTDIR"/backups"
#define BACKUPFILE "backup"

/* rezimy set */
#define SET_NONE 0
#define SET_MAIN 1
#define SET_TERM 2
#define SET_BANK 3

/* rozmery struktur */
#define NUM_HELP 3
#define NUM_PROMPT 2
#define NUM_WHO 6


/* error codes used by identify() */
#define ID_OK           0
#define ID_CONNERR     -1
#define ID_NOFOUND     -2
#define ID_CLOSED      -3
#define ID_READERR     -4
#define ID_WRITEERR    -5
#define ID_NOMEM       -6
#define ID_TIMEOUT     -7
#define ID_CRAP        -8
#define ID_NOUSER      -9
#define ID_INVPORT    -10
#define ID_UNKNOWNERR -11
#define ID_COMERR     -12
#define ID_UNKNOWN    -13
#define ID_HIDDENUSER -14

#define ID_BUFFLEN    196 /* max. buffer length for reads and writes in identify() */
#define ID_READTIMEOUT 30 /* number of seconds after a socket read operation is timed out. */

/* money */
#define DEFAULT_MONEY 1000
#define DEFAULT_BANK 3000
#define MAX_DONATION 5000
#define CREDITS_PER_HOUR 10
#define MIN_CREDIT_UPDATE_LEVEL SUPER

/* DEBUG inspired by NooK */
#ifdef DEBUG
#	define CRASH_HISTORY 1024
#	define set_crash() { s_crash(__FILE__, __LINE__); }
#else
#	define set_crash() ((void)0)
#endif

#endif /* define.h */
