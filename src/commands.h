/* vi: set ts=4 sw=4 ai: */
/*
 * commands.h
 *
 *   Lotos v1.2.1  : (c) 1999-2001 Pavol Hluchy (Lopo)
 *   last update   : 26.12.2001
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __COMMANDS_H__
#define __COMMANDS_H__ 1

/* These are the general function names of the commands */
char *command_types[]={
  "General","Social","Messages","User","Ignores","Movement","Clones","Admin",
  #ifdef NETLINKS
    "Netlink",
  #endif
  "Plugins","*"
  };


/* The enumerated type of above */
enum comtypes {
  CT_GENERAL,CT_SOCIAL,CT_MSG,CT_USER,CT_IGNORE,CT_MOVE,CT_CLONE,CT_ADMIN
  #ifdef NETLINKS
    ,CT_NETLINK
  #endif
  ,CT_PLUGINS
  };


/* Although the commands are now set up like this, you still need to add the enum
   value in the correct place in the table below.  If you don't do this then the
   commands may not work correctly and it'll be YOUR OWN FAULT!!
  */
struct {
  char *name,*alias; int level,function;
  } command_table[]={
    { "quit",       "",    JAILED,  CT_GENERAL },
    { "look",       "",    NEW,     CT_GENERAL },
    { "say",        "",    JAILED,  CT_SOCIAL  },
    { "shout",      "!",   L_3,     CT_SOCIAL  },
    { "tell",       ">",   NEW,     CT_SOCIAL  },
    { "emote",      ";",   SUPER,   CT_SOCIAL  },
    { "semote",     "#",   L_5,     CT_SOCIAL  },
    { "pemote",     "<",   L_5,     CT_SOCIAL  },
    { "echo",       "",    L_5,     CT_SOCIAL  },
    { "go",         "",    USER,    CT_MOVE    },
    { "desc",       "",    NEW,     CT_USER    },
    { "inphr",      "",    SUPER,   CT_USER    },
    { "outphr",     "",    SUPER,   CT_USER    },
    { "public",     "",    L_6,     CT_GENERAL },
    { "private",    "",    L_6,     CT_GENERAL },
    { "letmein",    "",    USER,    CT_GENERAL },
    { "invite",     "",    L_6,     CT_GENERAL },
    { "to",         "/",   USER,    CT_SOCIAL  },
    { "topic",      "",    SUPER,   CT_SOCIAL  },
    { "move",       "",    L_6,     CT_MOVE    },
    { "bcast",      "",    L_6,     CT_SOCIAL  },
    { "who",        "",    JAILED,  CT_GENERAL },
    { "people",     "",    L_6,     CT_GENERAL },
    { "help",       "?",   JAILED,  CT_GENERAL },
    { "shutdown" ,  "",    GOD,     CT_ADMIN   },
    { "news",       "",    USER,    CT_MSG     },
    { "read",       "",    USER,    CT_MSG     },
    { "write",      "",    L_3,     CT_MSG     },
    { "wipe",       "",    L_6,     CT_MSG     },
    { "search",     "",    L_3,     CT_GENERAL },
    { "review",     "",    L_3,     CT_GENERAL },
  #ifdef NETLINKS
    { "home",       "",    USER,    CT_MOVE    },
  #endif
    { "status",     "",    NEW,     CT_USER    },
    { "version",    "",    NEW,     CT_GENERAL },
    { "rmail",      "",    NEW,     CT_MSG     },
    { "smail",      "",    USER,    CT_MSG     },
    { "dmail",      "",    NEW,     CT_MSG     },
    { "from",       "",    USER,    CT_MSG     },
    { "entpro",     "",    USER,    CT_USER    },
    { "examine",    "",    L_3,     CT_USER    },
    { "rooms",      "",    SUPER,   CT_GENERAL },
  #ifdef NETLINKS
    { "rnet",       "",    WIZ,     CT_NETLINK },
    { "netstat",    "",    ARCH,    CT_NETLINK },
    { "netdata",    "",    ARCH,    CT_NETLINK },
    { "connect",    "",    GOD,     CT_NETLINK },
    { "disconnect", "",    GOD,     CT_NETLINK },
  #endif
    { "passwd",     "",    NEW,     CT_USER    },
    { "kill",       "",    ARCH,    CT_ADMIN   },
    { "promote",    "",    L_8,     CT_ADMIN   },
    { "demote",     "",    L_8,     CT_ADMIN   },
    { "lban",       "",    WIZ,     CT_ADMIN   },
    { "ban",        "",    L_8,     CT_ADMIN   },
    { "vis",        "",    L_6,     CT_USER    },
    { "invis",      "",    L_6,     CT_USER    },
    { "site",       "",    ARCH,    CT_ADMIN   },
    { "wake",       "",    SUPER,   CT_SOCIAL  },
    { "wizshout",   "",    WIZ,     CT_SOCIAL  },
    { "muzzle",     "",    WIZ,     CT_ADMIN   },
    { "map",        "",    USER,    CT_GENERAL },
    { "logging",    "",    GOD,     CT_ADMIN   },
    { "minlogin",   "",    GOD,     CT_ADMIN   },
    { "system",     "",    L_8,     CT_ADMIN   },
    { "charecho",   "",    NEW,     CT_USER    },
    { "clearline",  "",    L_8,     CT_ADMIN   },
    { "fix",        "",    ARCH,    CT_GENERAL },
    { "unfix",      "",    ARCH,    CT_GENERAL },
    { "viewlog",    "",    GOD,     CT_ADMIN   },
    { "accreq",     "",    NEW,     CT_USER    },
    { "revclear",   "",    USER,    CT_SOCIAL  },
    { "clone",      "",    L_6,     CT_CLONE   },
    { "destroy",    "",    L_6,     CT_CLONE   },
    { "myclones",   "",    L_6,     CT_CLONE   },
    { "allclones",  "",    L_6,     CT_CLONE   },
    { "switch",     "",    L_6,     CT_CLONE   },
    { "csay",       "",    L_6,     CT_CLONE   },
    { "chear",      "",    WIZ,     CT_CLONE   },
  #ifdef NETLINKS
    { "rstat",      "",    L_8,     CT_NETLINK },
  #endif
    { "swban",      "",    ARCH,    CT_ADMIN   },
    { "afk",        "",    USER,    CT_USER    },
    { "cls",        "",    NEW,     CT_GENERAL },
    { "suicide",    "",    NEW,     CT_USER    },
    { "deluser",    "",    GOD,     CT_ADMIN   },
    { "reboot",     "",    GOD,     CT_ADMIN   },
    { "recount",    "",    GOD,     CT_MSG     },
    { "revtell",    "",    USER,    CT_SOCIAL  },
    { "purge",      "",    GOD,     CT_ADMIN   },
    { "history",    "",    ARCH,    CT_ADMIN   },
    { "expire",     "",    GOD,     CT_ADMIN   },
    { "ranks",      "",    NEW,     CT_GENERAL },
    { "wizlist",    "",    NEW,     CT_GENERAL },
    { "time",       "",    USER,    CT_GENERAL },
    { "ctopic",     "",    SUPER,   CT_SOCIAL  },
    { "copyto",     "",    L_3,     CT_MSG     },
    { "nocopys",    "",    L_3,     CT_MSG     },
    { "set",        "",    NEW,     CT_USER    },
    { "mutter",     "",    USER,    CT_SOCIAL  },
    { "makevis",    "",    WIZ,     CT_USER    },
    { "makeinvis",  "",    WIZ,     CT_USER    },
    { "sos",        "",    JAILED,  CT_SOCIAL  },
    { "dajobr",     "",    L_5,     CT_SOCIAL  },
    { "ukazobr",    "",    L_5,     CT_SOCIAL  },
    { "picture",    "",    SUPER,   CT_SOCIAL  },
    { "greet",      "",    WIZ,     CT_SOCIAL  },
    { "think",      "",    L_3,     CT_SOCIAL  },
    { "sing",       "",    L_3,     CT_SOCIAL  },
    { "ewiz",       "",    WIZ,     CT_SOCIAL  },
    { "suggest",    "",    USER,    CT_MSG     },
    { "rsug",       "",    ARCH,    CT_MSG     },
    { "dsug",       "",    SYSOP,   CT_MSG     },
    { "last",       "",    SUPER,   CT_USER    },
    { "macros",     "",    L_3,     CT_USER    },
    { "rules",      "",    JAILED,  CT_GENERAL },
    { "lmail",      "",    ARCH,    CT_MSG     },
    { "jail",       "",    L_6,     CT_ADMIN   },
    { "verify",     "",    NEW,     CT_MSG     },
    { "addhistory", "",    L_8,     CT_ADMIN   },
    { "forwarding", "",    GOD,     CT_ADMIN   },
    { "revshout",   "",    L_3,     CT_SOCIAL  },
    { "cshout",     "",    SUPER,   CT_SOCIAL  },
    { "ctells",     "",    USER,    CT_SOCIAL  },
    { "monitor",    "",    L_8,     CT_ADMIN   },
    { "call",       ",",   L_3,     CT_SOCIAL  },
    { "create",     "",    ARCH,    CT_USER    },
    { "bfrom",      "",    SUPER,   CT_MSG     },
    { "samesite",   "",    ARCH,    CT_ADMIN   },
    { "save",       "",    L_6,     CT_ADMIN   },
    { "prilep",     "",    L_5,     CT_ADMIN   },
    { "join",       "",    L_5,     CT_MOVE    },
    { "cemote",     "",    L_6,     CT_CLONE   },
    { "revafk",     "",    USER,    CT_SOCIAL  },
    { "cafk",       "",    USER,    CT_SOCIAL  },
    { "revedit",    "",    USER,    CT_SOCIAL  },
    { "cedit",      "",    USER,    CT_SOCIAL  },
    { "listen",     "",    SUPER,   CT_IGNORE  },
    { "retire",     "",    ARCH,    CT_ADMIN   },
    { "memcount",   "",    GOD,     CT_ADMIN   },
    { "cmdcount",   "",    L_8,     CT_ADMIN   },
    { "rcountu",    "",    GOD,     CT_ADMIN   },
    { "recaps",     "",    GOD,     CT_ADMIN   },
    { "setcmdlev",  "",    ARCH,    CT_ADMIN   },
    { "grepu",      "",    WIZ,     CT_GENERAL },
    { "shoot",      "",    USER,    CT_GENERAL },
    { "reload",     "",    USER,    CT_GENERAL },
    { "xcom",       "",    L_8,     CT_ADMIN   },
    { "gcom",       "",    L_8,     CT_ADMIN   },
    { "sfrom",      "",    WIZ,     CT_MSG     },
    { "autopromo",  "",    GOD,     CT_ADMIN   },
    { "notify",     "",    USER,    CT_SOCIAL  },
    { "nsay",       "",    L_3,     CT_SOCIAL  },
    { "nemote",     "",    L_3,     CT_SOCIAL  },
    { "kidnap",     "",    SUPER,   CT_MOVE    },
    { "force",      "",    L_8,     CT_ADMIN   },
    { "kalendar",   "",    USER,    CT_GENERAL },
    { "myroom",     "",    L_5,     CT_MOVE    },
    { "mylock",     "",    L_6,     CT_GENERAL },
    { "visit",      "",    L_5,     CT_MOVE    },
    { "mypaint",    "",    L_5,     CT_GENERAL },
    { "beep",       "",    SUPER,   CT_SOCIAL  },
    { "rmadmin",    "",    ARCH,    CT_ADMIN   },
    { "mykey",      "",    L_6,     CT_GENERAL },
    { "mybgone",    "",    L_6,     CT_GENERAL },
    { "wrules",     "",    WIZ,     CT_GENERAL },
    { "files",      "",    USER,    CT_GENERAL },
    { "adminfiles", "",    WIZ,     CT_ADMIN   },
    { "dump",       "",    GOD,     CT_ADMIN   },
    { "tpromote",   "",    L_7,     CT_ADMIN   },
    { "rename",     "",    ARCH,    CT_ADMIN   },
    { "fmail",      "",    SUPER,   CT_MSG     },
    { "reminder",   "",    SUPER,   CT_MSG     },
    { "nsmail",     "",    L_4,     CT_MSG     },
    { "plugreg",    "",    GOD,     CT_ADMIN   },
	{ "pldebug",	"",    GOD,		CT_ADMIN   },
    { "prikazy",    "",    JAILED,  CT_GENERAL },
    { "tplane",     "",    USER,    CT_MOVE    },
    { "ignore",     "",    USER,    CT_IGNORE  },
    { "reply",      "",    L_3,     CT_SOCIAL  },
    { "shoutto",    "!>",  SUPER,   CT_SOCIAL  },
    { "tellall",    ">>",  WIZ,     CT_SOCIAL  },
    { "grmnick",    "",    NEW,     CT_ADMIN   },
    { "auth",       "",    WIZ,     CT_ADMIN   },
    { "alarm",      "",    USER,    CT_USER    },
    { "vote",       "",    USER,    CT_GENERAL },
    { "finger",     "",    GOD,     CT_ADMIN   },
    { "rloads",     "",    GOD,     CT_ADMIN   },
    { "banner",     "",    ARCH,    CT_SOCIAL  },
    { "tbanner",    "",    ARCH,    CT_SOCIAL  },
    { "sbanner",    "",    GOD,     CT_SOCIAL  },
    { "swears",     "",    GOD,     CT_ADMIN   },
    { "modify",     "",    ARCH,    CT_ADMIN   },
    { "restrict",   "",    ARCH,    CT_ADMIN   },
    { "abbrs",      "",    JAILED,  CT_GENERAL },
    { "open",       "",    ROOT,    CT_ADMIN   },
    { "close",      "",    ROOT,    CT_ADMIN   },
    { "backup",     "",    GOD,     CT_ADMIN   },
    { "follow",     "",    L_5,     CT_USER    },
    { "kick",       "",    USER,    CT_SOCIAL  },
    { "levels",     "",    NEW,     CT_GENERAL },
    { "faq",        "",    JAILED,  CT_GENERAL },
    { "talkers",    "",    USER,    CT_GENERAL },
    { "counters",   "",    ARCH,    CT_ADMIN   },
    { "hug",        "",    SUPER,   CT_SOCIAL  },
	{ "kiss",	"",	SUPER,	CT_SOCIAL	},
	{ "restart",    "",    ARCH,    CT_ADMIN   },
	{ "myxterm",    "",    USER,    CT_GENERAL },
	{ "allxterm",    "",   ARCH,    CT_GENERAL },
#ifdef DEBUG
	{ "test", 	"", 	JAILED,   CT_ADMIN },
#endif
	{ "jukebox",	"",	SUPER,	CT_USER		},
	{ "terminal",	"",	JAILED,	CT_USER		},
	{ "identify",	"",	ARCH,	CT_ADMIN	},
	{ "donate", 	"", USER,	CT_GENERAL	},
	{ "cash",		"", USER,	CT_GENERAL	},
	{ "money",		"",	ARCH,	CT_GENERAL	},
	{ "bank",		"",	USER,	CT_GENERAL	},
	{ "restore",	"", WIZ,	CT_ADMIN	},
    { "*","*",-1,-1 } /* stopping clause - do not remove */
  };
#include "comvals.h"

/* Subsection of the 'ign' command - for setting of attributes */
struct {
	char *type;
	char *desc;
	} ignstr[]={
		{"show", "zobrazi aktualny stav"},
		{"all","ignorovanie vsetkeho"},
		{"tells","vsetky telly"},
		{"shouts","vsetky vykriky"},
		{"pics","vsetky obrazky"},
		{"logons","oznamenia o prihlaseni/odhlaseni userov"},
		{"wiz","vykriky pre wizardov"},
		{"greets","bannery typu greet"},
		{"beeps","pipanie"},
		{"transp","hlasenia o prichode/odchode transportov"},
		{"user","zadanych userov"},
		{"word","nejake slovo"},
		{"funs","skratene wake, hug, kis"},
		{"*",""}
  };

#endif /* commands.h */

