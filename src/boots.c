/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                    Funkcie pre Lotos v1.2.0 na start systemu
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __BOOTS_C__
#define __BOOTS_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "define.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_tr.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "obj_pl.h"
#include "obj_mc.h"
#include "obj_syspp.h"
#include "boots.h"
#include "prototypes.h"


#ifndef NUM_LEVELS
	#define NUM_LEVELS SIZEOF(user_level)
#endif

/*** Construct system object and reset some global variables ***/
void create_system(void)
{
int i;
struct utsname uts;

set_crash();
if ((amsys=(SYS_OBJECT)malloc(sizeof(struct system_struct)))==NULL) {
  fprintf(stderr,"Lotos: Failed to create system object in create_system().\n");
  boot_exit(21);
  }

amsys->auto_connect=1;
amsys->max_users=50;
amsys->max_clones=1;
amsys->ban_swearing=0;
amsys->heartbeat=2;
amsys->keepalive_interval=60; /* DO NOT TOUCH!!! */
amsys->net_idle_time=300; /* Must be > than the above */
amsys->login_idle_time=180;
amsys->user_idle_time=300;
amsys->time_out_afks=0;
amsys->wizport_level=WIZ;
amsys->minlogin_level=-1;
amsys->mesg_life=1;
amsys->num_of_logins=0;
amsys->logging=BIT_SET(amsys->logging,SYSLOG);
amsys->logging=BIT_SET(amsys->logging,REQLOG);
amsys->logging=BIT_SET(amsys->logging,NETLOG);
#ifdef DEBUG
	amsys->logging=BIT_SET(amsys->logging,DEBLOG);
#endif
amsys->logging=BIT_SET(amsys->logging,ERRLOG);
amsys->password_echo=0;
amsys->ignore_sigterm=0;
amsys->crash_action=2;
amsys->prompt_def=1;
amsys->colour_def=1;
amsys->charecho_def=0;
amsys->time_out_maxlevel=USER;
amsys->mesg_check_hour=0;
amsys->mesg_check_min=0;
amsys->rs_countdown=0;
amsys->rs_announce=0;
amsys->rs_which=-1;
amsys->rs_user=NULL;
amsys->gatecrash_level=GOD+1; /* minimum user level which can enter private rooms */
amsys->min_private_users=2; /* minimum num. of users in room before can set to priv */
amsys->ignore_mp_level=GOD; /* User level which can ignore the above var. */
amsys->rem_user_maxlevel=USER;
amsys->rem_user_deflevel=USER;
amsys->logons_old=0;
amsys->logons_new=0;
amsys->purge_count=0;
amsys->purge_skip=0;
amsys->users_purged=0;
amsys->purge_date=1;
amsys->suggestion_count=0;
amsys->forwarding=1;
amsys->auto_purge=0;
amsys->user_count=0;
amsys->allow_recaps=1;
amsys->auto_promote=1;
amsys->personal_rooms=1;
amsys->startup_room_parse=1;
amsys->motd1_cnt=0;
amsys->motd2_cnt=0;
amsys->random_motds=1;
amsys->last_cmd_cnt=0;
amsys->resolve_ip=1; /* auto resolve ip */
amsys->flood_protect=1;
amsys->pid=(unsigned int)getpid();
time(&amsys->boot_time);
if ((uname(&uts))<0) {
  strcpy(amsys->sysname,"[undetermined]");
  strcpy(amsys->sysmachine,"[undetermined]");
  strcpy(amsys->sysrelease,"[undetermined]");
  strcpy(amsys->sysversion,"[undetermined]");
  strcpy(amsys->sysnodename,"[undetermined]");
  }
else {
  strncpy(amsys->sysname,uts.sysname,63);
  strncpy(amsys->sysmachine,uts.machine,63);
  strncpy(amsys->sysrelease,uts.release,63);
  strncpy(amsys->sysversion,uts.version,63);
  strncpy(amsys->sysnodename,uts.nodename,63);
  }
user_first=NULL;
user_last=NULL;
room_first=NULL;
room_last=NULL; /* This variable isn't used yet */
transport_first=NULL;
transport_last=NULL;
first_dir_entry=NULL;
first_command=NULL;
first_wiz_entry=NULL;
last_wiz_entry=NULL;
force_listen=0;
no_prompt=0;
logon_flag=0;
for (i=0;i<port_total;i++) {
  port[i]=0;
  listen_sock[i]=0;
  }
for (i=0;i<LASTLOGON_NUM;i++) {
  last_login_info[i].name[0]='\0';
  last_login_info[i].time[0]='\0';
  last_login_info[i].on=0;
  }
for (i=0;i<16;i++) cmd_history[i][0]='\0';
clear_words();
#ifdef NETLINKS
 verification[0]='\0';
 nl_first=NULL;
 nl_last=NULL;
#endif
strcpy(susers_restrict, RESTRICT_MASK);
}


/*** Initialise the signal traps etc ***/
void init_signals(void)
{
	set_crash();
	SIGNAL(SIGTERM,sig_handler);
	SIGNAL(SIGSEGV,sig_handler);
	SIGNAL(SIGBUS,sig_handler);
	SIGNAL(SIGILL,SIG_IGN);
	SIGNAL(SIGTRAP,SIG_IGN);
	SIGNAL(SIGIOT,SIG_IGN);
	SIGNAL(SIGTSTP,SIG_IGN);
	SIGNAL(SIGCONT,SIG_IGN);
	SIGNAL(SIGHUP,SIG_IGN);
	SIGNAL(SIGINT,SIG_IGN);
	SIGNAL(SIGQUIT,SIG_IGN);
	SIGNAL(SIGABRT,SIG_IGN);
	SIGNAL(SIGFPE,SIG_IGN);
	SIGNAL(SIGPIPE,SIG_IGN);
	SIGNAL(SIGTTIN,SIG_IGN);
	SIGNAL(SIGTTOU,SIG_IGN);
}


/******************************************************************************
 The loading up and parsing of the configuration file
 *****************************************************************************/



void load_and_parse_config(void) {
FILE *fp;
char line[81]; /* Should be long enough */
char filename[500], c;
int i,section_in,got_init,got_rooms,got_topics, got_transport;
RM_OBJECT rm1,rm2;
#ifdef NETLINKS
  NL_OBJECT nl;
#endif

set_crash();
section_in=0;
got_init=0;
got_rooms=0;
got_topics=0;
got_transport=0;

printf("Parsing config file \"%s\"...\n", confile);
if (!(fp=fopen(confile,"r"))) {
  perror("Lotos: Can't open config file\n");  boot_exit(1);
  }
/* Main reading loop */
config_line=0;
fgets(line,81,fp);
while(!feof(fp)) {
  config_line++;
  for(i=0;i<8;++i) wrd[i][0]='\0';
  sscanf(line,"%s %s %s %s %s %s %s %s",wrd[0],wrd[1],wrd[2],wrd[3],wrd[4],wrd[5],wrd[6],wrd[7]);
  if (wrd[0][0]=='#' || wrd[0][0]=='\0') {
    fgets(line,81,fp);  continue;
    }
  /* See if new section */
  if (wrd[0][strlen(wrd[0])-1]==':') {
    if (!strcmp(wrd[0],"INIT:")) section_in=1;
    else if (!strcmp(wrd[0],"ROOMS:")) section_in=2;
    else if (!strcmp(wrd[0],"TOPICS:")) section_in=3;
    else if (!strcmp(wrd[0],"SITES:")) section_in=4;
    else if (!strcmp(wrd[0],"TRANSPORTS:")) section_in=5;
    else {
      fprintf(stderr,"Lotos: Unknown section header on line %d.\n",config_line);
      fclose(fp);  boot_exit(1);
      }
    }
  switch(section_in) {
    case 1: parse_init_section();  got_init=1; break;
    case 2: parse_rooms_section(); got_rooms=1; break;
    case 3: parse_topics_section(remove_first(line)); got_topics=1; break;
    case 4:
#ifdef NETLINKS
        parse_sites_section(); break;
#else
	break;
#endif
    case 5: parse_transports_section(); got_transport=1; break;
    default:
      fprintf(stderr,"Lotos: Section header expected on line %d.\n",config_line);
      fclose(fp);  boot_exit(1);
    }
  fgets(line,81,fp);
  }
fclose(fp);
/* See if required sections were present (SITES and TOPICS is optional) and if
   required parameters were set. */
if (!got_init) {
  fprintf(stderr,"Lotos: INIT section missing from config file.\n");
  boot_exit(1);
  }
if (got_topics && !got_rooms) {
  fprintf(stderr,"Lotos: TOPICS section must come after ROOMS section in the config file.\n");
  boot_exit(1);
  }
if (got_transport && !got_rooms) {
  fprintf(stderr,"Lotos: TRANSPORTS section must come after ROOMS section in the config file.\n");
  boot_exit(1);
  }
if (got_topics && !got_transport) {
  fprintf(stderr,"Lotos: TOPICS section must come after TRANSPORTS section in the config file.\n");
  boot_exit(1);
  }
if (!got_rooms) {
  fprintf(stderr,"Lotos: ROOMS section missing from config file.\n");
  boot_exit(1);
  }
if (!got_transport) {
  fprintf(stderr,"Lotos: TRANSPORTS section missing from config file.\n");
  boot_exit(1);
  }
if (!port[0]) {
  fprintf(stderr,"Lotos: Main port number not set in config file.\n");
  boot_exit(1);
  }
if (!port[1]) {
  fprintf(stderr,"Lotos: Wiz port number not set in config file.\n");
  boot_exit(1);
  }
#ifdef NETLINKS
	if (!port[2]) {
		fprintf(stderr,"Lotos: Link port number not set in config file.\n");
		boot_exit(1);
		}
	if (!verification[0]) {
		fprintf(stderr,"Lotos: Verification not set in config file.\n");
		boot_exit(1);
		}
	if (port[0]==port[1]
	    || port[1]==port[2]
	    || port[0]==port[2]
	    ) {
#else
	if (port[0]==port[1]) {
#endif
  fprintf(stderr,"Lotos: Port numbers must be unique.\n");
  boot_exit(1);
  }
if (room_first==NULL) {
  fprintf(stderr,"Lotos: No rooms configured in config file.\n");
  boot_exit(1);
  }
if (!syspp->auto_save) {
  fprintf(stderr,"Lotos: autosave period not set in config file.\n");
  boot_exit(1);
  }
	if (syspp->auto_afk && syspp->auto_afk_time>=(amsys->user_idle_time-60)) {
		fprintf(stderr, "Lotos: Auto_afk_time musi byt vacsie ako user_idle_time-60.\n");
		exit(1);
		}

/* Parsing done, now check data is valid. Check room stuff first. */
for (rm1=room_first; rm1!=NULL; rm1=rm1->next) {
	if (rm1->transp!=NULL && rm1->link_label[1]=='\0') {
		fprintf(stderr, "Rooma %s je transport, preto musi mat minimalne 2 linky\n",
		rm1->name);
		boot_exit(1);
		}
  for(i=0;i<MAX_LINKS;++i) {
    if (!rm1->link_label[i][0]) break;
    if ((!strcmp(rm1->link_label[i], no_leave)) && (rm1->transp==NULL)) break;
    if ((!strcmp(rm1->link_label[i], no_leave)) && rm1->transp!=NULL) {
    	fprintf(stderr,"Lotos: Rooma %s je transport, preto nemoze byt bez linkov\n",
    		rm1->name);
    	boot_exit(1);
    	}
    for(rm2=room_first;rm2!=NULL;rm2=rm2->next) {
      if (rm1==rm2) continue;
      if (!strcmp(rm1->link_label[i],rm2->label)) {
        if (rm2->transp!=NULL) {
        	fprintf(stderr,"Rooma %s nemoze byt nalinkovana na iny transport\n",
        		rm1->name);
        	boot_exit(1);
        	}
	rm1->link[i]=rm2;
	break;
        }
      }
    if (rm1->link[i]==NULL) {
      fprintf(stderr,"Lotos: Room %s has undefined link label '%s'.\n",rm1->name,rm1->link_label[i]);
      boot_exit(1);
      }
    }
  }

#ifdef NETLINKS
/* Check external links */
for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
  if (rm1->netlink_name[0] && rm1->transp!=NULL) {
  	fprintf(stderr, "Rooma %s je nadefinovana ako transport a preto nemoze mat netlink\n",
  	rm1->name);
  	boot_exit(1);
  	}
  for(nl=nl_first;nl!=NULL;nl=nl->next) {
    if (!strcmp(nl->service,rm1->name)) {
      fprintf(stderr,"Lotos: Service name %s is also the name of a room.\n",nl->service);
      boot_exit(1);
      }
    if (rm1->netlink_name[0] && !strcmp(rm1->netlink_name,nl->service)) {
      rm1->netlink=nl;
      break;
      }
    }
  if (rm1->netlink_name[0] && rm1->netlink==NULL) {
    fprintf(stderr,"Lotos: Service name %s not defined for room %s.\n",rm1->netlink_name,rm1->name);
    boot_exit(1);
    }
  }
#endif

/* Load room descriptions */
for(rm1=room_first;rm1!=NULL;rm1=rm1->next) {
 if (rm1->transp!=NULL) sprintf(filename,"%s/%s.R", TRFILES, rm1->name);
 else sprintf(filename,"%s/%s.R", ROOMFILES, rm1->name);
  if (!(fp=fopen(filename,"r"))) {
    fprintf(stderr,"Lotos: Can't open description file for room %s.\n",rm1->name);
    write_syslog(ERRLOG,1,"Couldn't open description file for room %s.\n",rm1->name);
    continue;
    }
  i=0;
  c=getc(fp);
  while(!feof(fp)) {
    if (i==ROOM_DESC_LEN) {
      fprintf(stderr,"Lotos: Description too long for room %s.\n",rm1->name);
      write_syslog(ERRLOG,1,"Description too long for room %s.\n",rm1->name);
      break;
      }
    rm1->desc[i]=c;  
    c=getc(fp);
    ++i;
    }
  rm1->desc[i]='\0';
  fclose(fp);
  }
}


/*** Parse the user rooms ***/
void parse_user_rooms(void) {
DIR *dirp;
struct dirent *dp;
char name[USER_NAME_LEN];
RM_OBJECT rm;

if (!(dirp=opendir(USERROOMS))) {
  fprintf(stderr,"Lotos: Directory open failure in parse_user_rooms().\n");
  boot_exit(19);
  }
/* parse the names of the files but don't include . and .. */
while((dp=readdir(dirp))!=NULL) {
  if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")) continue;
  if (strstr(dp->d_name,".R")) {
    strcpy(name,dp->d_name);
    name[strlen(name)-2]='\0';
    rm=create_room();
    if (!(personal_room_store(name,0,rm))) {
      write_syslog(ERRLOG,1,"Could not read personal room attributes.  Using standard config.\n");
      }
    strtolower(name);
    sprintf(rm->name,"(%s)",name);
    }
  }
(void) closedir(dirp);
}


/*** Check to see if the directory structure in USERFILES is correct, ie, there
     is one directory for each of the level names given in *user_level[]
     Also, check if level names are unique.
     ***/
void check_directories(void) {
struct stat stbuf;
int levels,found,i,j;

levels=found=0;
/* Check for unique directory names */
for (i=0; i<NUM_LEVELS; ++i) {
  for (j=i+1; j<NUM_LEVELS; ++j) {
    if (!strcmp(user_level[i].name,user_level[j].name)) {
      fprintf(stderr,"Lotos: Level names are not unique.\n");
      boot_exit(14);
      }
    }
  }
i=0;
/* check the directories needed exist */
if (stat(USERFILES, &stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERMAILS, &stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERPROFILES,&stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERFRIENDS,&stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERHISTORYS,&stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERCOMMANDS,&stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERMACROS,&stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
if (stat(USERROOMS,&stbuf)==-1) {
  fprintf(stderr,"Lotos: Directory stat failure in check_directories().\n");
  boot_exit(15);
  }
if ((stbuf.st_mode & S_IFMT)!=S_IFDIR) goto SKIP;
return;
SKIP:
fprintf(stderr,"Lotos: Directory structure is incorrect.\n");
boot_exit(16);
}


/*** Get all users from the user directories and add them to the user lists.
     If verbose mode is on, then attempt to get date string as well
     ***/
void process_users(void)
{
char name[USER_NAME_LEN+3];
DIR *dirp;
struct dirent *dp;
UR_OBJECT u;

/* open the directory file up */
dirp=opendir(USERFILES);
if (dirp==NULL) {
  fprintf(stderr,"Lotos: Directory open failure in process_users().\n");
  boot_exit(12);
  }
if ((u=create_user())==NULL) {
  fprintf(stderr,"Lotos: Create user failure in process_users().\n");
  (void) closedir(dirp);
  boot_exit(17);
  }
/* count up how many files in the directory - this include . and .. */
while((dp=readdir(dirp))!=NULL) {
  if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")) continue;
  if (strstr(dp->d_name,".D")) {
    strcpy(name,dp->d_name);
    name[strlen(name)-2]='\0';
    strcpy(u->name,name);
    if (load_user_details(u)) {
      add_user_node(u->name,u->level);
      if (u->level>=WIZ) add_wiz_node(u->name,u->level);
      add_user_date_node(u->name,u->date);
      } /* end if */
    else {
      fprintf(stderr,"Lotos: Could not load userfile for '%s' in process_users().\n",name);
      (void) closedir(dirp);
      boot_exit(18);
      }
    } /* end if */
  reset_user(u);
  } /* end while */
destruct_user(u);
(void) closedir(dirp);
}


/* Put commands in an ordered linked list for viewing with .help */
void parse_commands(void)
{
	int cnt;

	cnt=0;
	while(command_table[cnt].name[0]!='*') {
		if (!(add_command(cnt))) {
			fprintf(stderr,"Lotos: Memory allocation failure in parse_commands().\n");
			boot_exit(13);
			}
		++cnt;
		}
	return;
}


/* needs only doing once when booting */
void count_suggestions(void)
{
	char line[82],id[20];
	FILE *fp;
	int valid;

	if (!(fp=fopen(SUGBOARD, "r"))) return;
	valid=1;
	fgets(line,82,fp);
	while(!feof(fp)) {
		if (line[0]=='\n') valid=1;
		sscanf(line,"%s",id);
		if (valid && strstr(id,"From:")) {
			++amsys->suggestion_count;
			valid=0;
			}
		fgets(line,82,fp);
		}
	fclose(fp);
}


/*** Initialise sockets on ports ***/
void init_sockets(void) {
struct sockaddr_in bind_addr;
int i,on,size;

#ifdef NETLINKS
	printf("Initialising sockets on ports: %d, %d, %d\n",port[0],port[1],port[2]);
#else
	printf("Initialising sockets on ports: %d, %d\n",port[0],port[1]);
#endif

on=1;
size=sizeof(struct sockaddr_in);
bind_addr.sin_family=AF_INET;
bind_addr.sin_addr.s_addr=INADDR_ANY;
for(i=0;i<port_total;++i) {
  /* create sockets */
  if ((listen_sock[i]=socket(AF_INET,SOCK_STREAM,0))==-1) boot_exit(i+2);
  /* allow reboots on port even with TIME_WAITS */
  setsockopt(listen_sock[i],SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
  /* bind sockets and set up listen queues */
  bind_addr.sin_port=htons(port[i]);
  if (bind(listen_sock[i],(struct sockaddr *)&bind_addr,size)==-1) boot_exit(i+5);
  if (listen(listen_sock[i],10)==-1) boot_exit(i+8);
  /* Set to non-blocking */
  fcntl(listen_sock[i],F_SETFL,O_NDELAY);
  }
}


/*** Return level value based on level name ***/
int get_level(char *name)
{
	int i;

	for (i=0;i<NUM_LEVELS;i++) {
		if (!strcasecmp(user_level[i].name,name))
			return i;
		}
	return -1;
}

/*** Parse init section ***/
void parse_init_section(void) {
static int in_section=0;
int op,val,tmp;
char *options[]={ 
  "mainport","wizport","linkport","system_logging","minlogin_level","mesg_life",
  "wizport_level","prompt_def","gatecrash_level","min_private","ignore_mp_level",
  "rem_user_maxlevel","rem_user_deflevel","verification","mesg_check_time",
  "max_users","heartbeat","login_idle_time","user_idle_time","password_echo",
  "ignore_sigterm","auto_connect","max_clones","ban_swearing","crash_action",
  "colour_def","time_out_afks","charecho_def","time_out_maxlevel","auto_purge",
  "allow_recaps","auto_promote","personal_rooms","random_motds","startup_room_parse",
  "resolve_ip","flood_protect", "pueblo_enh", "auto_save", "susers_restrict",
  "auto_afk", "use_hosts_file", "*"
  };

if (!strcmp(wrd[0],"INIT:")) { 
  if (++in_section>1) {
    fprintf(stderr,"Lotos: Unexpected INIT section header on line %d.\n",config_line);
    boot_exit(1);
    }
  return;
  }
op=0; tmp=0;
while(strcmp(options[op],wrd[0])) {
  if (options[op][0]=='*') {
    fprintf(stderr,"Lotos: Unknown INIT option on line %d.\n",config_line);
    boot_exit(1);
    }
  ++op;
  }
if (!wrd[1][0]) {
  fprintf(stderr,"Lotos: Required parameter missing on line %d.\n",config_line);
  boot_exit(1);
  }
if (wrd[2][0] && wrd[2][0]!='#') {
  fprintf(stderr,"Lotos: Unexpected word following init parameter on line %d.\n",config_line);
  boot_exit(1);
  }
val=atoi(wrd[1]);
switch (op) {
  case 0: /* main port */
  case 1: /* wiz */
#ifdef NETLINKS
  case 2: /* link */
#endif
    if ((port[op]=val)<1 || val>65535) {
      fprintf(stderr,"Lotos: Illegal port number on line %d.\n",config_line);
      boot_exit(1);
      }
    return;
  
  case 3:  
    if ((tmp=onoff_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: System_logging must be ON or OFF on line %d.\n",config_line);
      boot_exit(1);
      }
    /* set the bits correctly */
    if (tmp) {
      amsys->logging=BIT_SET(amsys->logging,SYSLOG);
      amsys->logging=BIT_SET(amsys->logging,REQLOG);
      amsys->logging=BIT_SET(amsys->logging,NETLOG);
#ifdef DEBUG
      amsys->logging=BIT_SET(amsys->logging,DEBLOG);
#endif
      amsys->logging=BIT_SET(amsys->logging,ERRLOG);
      }
    else amsys->logging=0;
    return;
  
  case 4:
    if ((amsys->minlogin_level=get_level(wrd[1]))==-1) {
      if (strcmp(wrd[1],"NONE")) {
	fprintf(stderr,"Lotos: Unknown level specifier for minlogin_level on line %d.\n",config_line);
	boot_exit(1);	
        }
      amsys->minlogin_level=-1;
      }
    return;
    
  case 5:  /* message lifetime */
    if ((amsys->mesg_life=val)<1) {
      fprintf(stderr,"Lotos: Illegal message lifetime on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 6: /* wizport_level */
    if ((amsys->wizport_level=get_level(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Unknown level specifier for wizport_level on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

  case 7: /* prompt defaults */
    if ((amsys->prompt_def=onoff_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Prompt_def must be ON or OFF on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 8: /* gatecrash level */
    if ((amsys->gatecrash_level=get_level(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Unknown level specifier for gatecrash_level on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

  case 9:
    if (val<1) {
      fprintf(stderr,"Lotos: Number too low for min_private_users on line %d.\n",config_line);
      boot_exit(1);
      }
    amsys->min_private_users=val;
    return;

  case 10:
    if ((amsys->ignore_mp_level=get_level(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Unknown level specifier for ignore_mp_level on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

  case 11: 
    /* Max level a remote user can remotely log in if he doesn't have a local
       account. ie if level set to WIZ a GOD can only be a WIZ if logging in 
       from another server unless he has a local account of level GOD */
    if ((amsys->rem_user_maxlevel=get_level(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Unknown level specifier for rem_user_maxlevel on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

  case 12:
    /* Default level of remote user who does not have an account on site and
       connection is from a server of version 3.3.0 or lower. */
    if ((amsys->rem_user_deflevel=get_level(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Unknown level specifier for rem_user_deflevel on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

#ifdef NETLINKS
  case 13:
    if (strlen(wrd[1])>VERIFY_LEN) {
      fprintf(stderr,"Lotos: Verification too long on line %d.\n",config_line);
      boot_exit(1);	
      }
    strcpy(verification,wrd[1]);
    return;
#endif

  case 14: /* mesg_check_time */
    if (wrd[1][2]!=':'
	|| strlen(wrd[1])>5
	|| !isdigit(wrd[1][0]) 
	|| !isdigit(wrd[1][1])
	|| !isdigit(wrd[1][3]) 
	|| !isdigit(wrd[1][4])) {
      fprintf(stderr,"Lotos: Invalid message check time on line %d.\n",config_line);
      boot_exit(1);
      }
    sscanf(wrd[1],"%d:%d",&amsys->mesg_check_hour,&amsys->mesg_check_min);
    if (amsys->mesg_check_hour>23 || amsys->mesg_check_min>59) {
      fprintf(stderr,"Lotos: Invalid message check time on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

  case 15:
    if ((amsys->max_users=val)<1) {
      fprintf(stderr,"Lotos: Invalid value for max_users on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 16:
    if ((amsys->heartbeat=val)<1) {
      fprintf(stderr,"Lotos: Invalid value for heartbeat on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 17:
    if ((amsys->login_idle_time=val)<10) {
      fprintf(stderr,"Lotos: Invalid value for login_idle_time on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 18:
    if ((amsys->user_idle_time=val)<10) {
      fprintf(stderr,"Lotos: Invalid value for user_idle_time on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 19: 
    if ((amsys->password_echo=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Password_echo must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 20: 
    if ((amsys->ignore_sigterm=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Ignore_sigterm must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 21:
    if ((amsys->auto_connect=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Auto_connect must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 22:
    if ((amsys->max_clones=val)<0) {
      fprintf(stderr,"Lotos: Invalid value for max_clones on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 23:
    if ((amsys->ban_swearing=minmax_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Ban_swearing must be OFF, MIN or MAX on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 24:
    if (!strcmp(wrd[1],"NONE")) amsys->crash_action=0;
    else if (!strcmp(wrd[1],"IGNORE")) amsys->crash_action=1;
    else if (!strcmp(wrd[1],"REBOOT")) amsys->crash_action=2;
    else {
      fprintf(stderr,"Lotos: Crash_action must be NONE, IGNORE or REBOOT on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 25:
    if ((amsys->colour_def=onoff_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Colour_def must be ON or OFF on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 26:
    if ((amsys->time_out_afks=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Time_out_afks must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 27:
    if ((amsys->charecho_def=onoff_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Charecho_def must be ON or OFF on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 28:
    if ((amsys->time_out_maxlevel=get_level(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Unknown level specifier for time_out_maxlevel on line %d.\n",config_line);
      boot_exit(1);	
      }
    return;

  case 29: /* auto purge on boot up */
    if ((amsys->auto_purge=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Auto_purge must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 30: /* allow recapping of names */
    if ((amsys->allow_recaps=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Allow_recaps must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 31: /* define whether auto promotes are on or off */
    if ((amsys->auto_promote=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: auto_promote must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      } 
    return;

  case 32:
    if ((amsys->personal_rooms=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Personal_rooms must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 33:
    if ((amsys->random_motds=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Random_motds must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 34:
    if ((amsys->startup_room_parse=yn_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Startup_room_parse must be YES or NO on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 35: 
    if ((amsys->resolve_ip=resolve_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Resolve_ip must be OFF, AUTO or MANUAL on line %d.\n",config_line);
      boot_exit(1);
      }
    return;

  case 36: /* turns flood protection and auto-baning on and off */
    if ((amsys->flood_protect=onoff_check(wrd[1]))==-1) {
      fprintf(stderr,"Lotos: Flood_protect must be ON or OFF on line %d.\n",config_line);
      boot_exit(1);
      }
    return;
  
  case 37: /* povoli rozsirenie pre Pueblo */
    if ((syspp->pueblo_enh=onoff_check(wrd[1]))==-1) {
      fprintf(stderr, "Lotos: Pueblo_enh musi byt ON alebo OFF na riadku %d.\n", config_line);
      boot_exit(1);
      }
    return;

  case 38: /* perioda ukladania userfajlov v minutach */
    if (!is_inumber(wrd[1])) {
      fprintf(stderr, "Lotos: Auto_save na riadku %d musi byt cele kladne cislo alebo -1.\n", config_line);
      boot_exit(1);
      }
    syspp->auto_save=atoi(wrd[1]);
    if (syspp->auto_save==0 || syspp->auto_save<(-1)) {
      fprintf(stderr, "Lotos: Auto_save na riadku %d ma chybny parameter.\n", config_line);
      boot_exit(1);
      }
    return;
  case 39: /* default restrictions mask for superior users */
    strcpy(susers_restrict, wrd[1]);
    return;
  case 40: /* auto_afk */
    if (!is_number(wrd[1])) {
      	fprintf(stderr, "Lotos: Auto_afk na riadku %d musi byt cele kladne cislo\n", config_line);
      	boot_exit(1);
      	}
    syspp->auto_afk_time=atoi(wrd[1]);
    if (syspp->auto_afk_time>0) syspp->auto_afk=1;
    else syspp->auto_afk=0;
    return;
  case 41: /* use_hosts_file */
    if ((use_hostsfile=yn_check(wrd[1]))==-1) {
      fprintf(stderr, "Lotos: use_hosts_file musi byt YES alebo NO na riadku %d.\n", config_line);
      boot_exit(1);
      }
    return;

  } /* end switch */
}



/*** Parse rooms section ***/
void parse_rooms_section(void) {
static int in_section=0;
int i;
char *ptr1,*ptr2,c;
RM_OBJECT room;

if (!strcmp(wrd[0],"ROOMS:")) {
  if (++in_section>1) {
    fprintf(stderr,"Lotos: Unexpected ROOMS section header on line %d.\n",config_line);
    boot_exit(1);
    }
  return;
  }
if (!wrd[3][0]) {
  fprintf(stderr,"Lotos: Required parameter(s) missing on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[0])>ROOM_NAME_LEN) {
  fprintf(stderr,"Lotos: Room map name too long on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[1])>ROOM_LABEL_LEN) {
  fprintf(stderr,"Lotos: Room label too long on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[2])>ROOM_NAME_LEN) {
  fprintf(stderr,"Lotos: Room name too long on line %d.\n",config_line);
  boot_exit(1);
  }
/* Check for duplicate label or name */
for(room=room_first;room!=NULL;room=room->next) {
  if (!strcmp(room->label,wrd[1])) {
    fprintf(stderr,"Lotos: Duplicate room label on line %d.\n",config_line);
    boot_exit(1);
    }
  if (!strcmp(room->name,wrd[2])) {
    fprintf(stderr,"Lotos: Duplicate room name on line %d.\n",config_line);
    boot_exit(1);
    }
  }
room=create_room();
strcpy(room->map,wrd[0]);
strcpy(room->label,wrd[1]);
strcpy(room->name,wrd[2]);

/* Parse internal links bit ie hl,gd,of etc. MUST NOT be any spaces between
   the commas */
i=0;
ptr1=wrd[3];
ptr2=wrd[3];
while(1) {
  while(*ptr2!=',' && *ptr2!='\0') ++ptr2;
  if (*ptr2==',' && *(ptr2+1)=='\0') {
    fprintf(stderr,"Lotos: Missing link label on line %d.\n",config_line);
    boot_exit(1);
    }
  c=*ptr2;  *ptr2='\0';
  if (!strcmp(ptr1,room->label)) {
    fprintf(stderr,"Lotos: Room has a link to itself on line %d.\n",config_line);
    boot_exit(1);
    }
  strcpy(room->link_label[i],ptr1);
  if (c=='\0') break;
  if (++i>=MAX_LINKS) {
    fprintf(stderr,"Lotos: Too many links on line %d.\n",config_line);
    boot_exit(1);
    }
  *ptr2=c;
  ptr1=++ptr2;  
  }

/* Parse access privs */
if (wrd[4][0]=='#') {  room->access=PUBLIC;  return;  }
if (!wrd[4][0] || !strcmp(wrd[4],"BOTH")) room->access=PUBLIC; 
else if (!strcmp(wrd[4],"PUB")) room->access=FIXED_PUBLIC; 
else if (!strcmp(wrd[4],"PRIV")) room->access=FIXED_PRIVATE;
else if (!strcmp(wrd[4],"RTC")) room->access=ROOT_CONSOLE;
else {
  fprintf(stderr,"Lotos: Unknown room access type on line %d.\n",config_line);
  boot_exit(1);
  }
/* Parse external link stuff */
#ifdef NETLINKS
  if (!wrd[5][0] || wrd[5][0]=='#') return;
  if (!strcmp(wrd[5],"ACCEPT")) {  
    if (wrd[6][0] && wrd[6][0]!='#') {
      fprintf(stderr,"Lotos: Unexpected word following ACCEPT keyword on line %d.\n",config_line);
      boot_exit(1);
      }
    room->inlink=1;  
    return;
    }
  if (!strcmp(wrd[5],"CONNECT")) {
    if (!wrd[6][0]) {
      fprintf(stderr,"Lotos: External link name missing on line %d.\n",config_line);
      boot_exit(1);
      }
    if (wrd[7][0] && wrd[7][0]!='#') {
      fprintf(stderr,"Lotos: Unexpected word following external link name on line %d.\n",config_line);
      boot_exit(1);
      }
    strcpy(room->netlink_name,wrd[6]);
    return;
    }
  fprintf(stderr,"Lotos: Unknown connection option on line %d.\n",config_line);
  boot_exit(1);
#else
  return;
#endif
}



/*** Parse rooms desc (topic) section ***/
void parse_topics_section(char *topic) {
static int in_section=0;
int exists;
RM_OBJECT room;

if (!strcmp(wrd[0],"TOPICS:")) {
  if (++in_section>1) {
    fprintf(stderr,"Lotos: Unexpected TOPICS section header on line %d.\n",config_line);
    boot_exit(1);
    }
  return;
  }
if (!wrd[1][0]) {
  fprintf(stderr,"Lotos: Required parameter(s) missing on line %d.\n",config_line);
  boot_exit(1);
  }
/* Check to see if room exists */
exists=0;
for(room=room_first;room!=NULL;room=room->next)
  if (!strcmp(room->name,wrd[0])) {  ++exists;  break;  }
if (!exists) {
  fprintf(stderr,"Lotos: Room does not exist on line %d.\n",config_line);
  boot_exit(1);
  }
if (topic[strlen(topic)-1]=='\n') topic[strlen(topic)-1]='\0';
strncpy(room->topic,topic,TOPIC_LEN);
}


#ifdef NETLINKS

/*** Parse sites section ***/
void parse_sites_section(void) {
NL_OBJECT nl;
static int in_section=0;

if (!strcmp(wrd[0],"SITES:")) { 
  if (++in_section>1) {
    fprintf(stderr,"Lotos: Unexpected SITES section header on line %d.\n",config_line);
    boot_exit(1);
    }
  return;
  }
if (!wrd[3][0]) {
  fprintf(stderr,"Lotos: Required parameter(s) missing on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[0])>SERV_NAME_LEN) {
  fprintf(stderr,"Lotos: Link name length too long on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[3])>VERIFY_LEN) {
  fprintf(stderr,"Lotos: Verification too long on line %d.\n",config_line);
  boot_exit(1);
  }
if ((nl=create_netlink())==NULL) {
  fprintf(stderr,"Lotos: Memory allocation failure creating netlink on line %d.\n",config_line);
  boot_exit(1);
  }
if (!wrd[4][0] || wrd[4][0]=='#' || !strcmp(wrd[4],"ALL")) nl->allow=ALL;
else if (!strcmp(wrd[4],"IN")) nl->allow=IN;
else if (!strcmp(wrd[4],"OUT")) nl->allow=OUT;
else {
  fprintf(stderr,"Lotos: Unknown netlink access type on line %d.\n",config_line);
  boot_exit(1);
  }
if ((nl->port=atoi(wrd[2]))<1 || nl->port>65535) {
  fprintf(stderr,"Lotos: Illegal port number on line %d.\n",config_line);
  boot_exit(1);
  }
strcpy(nl->service,wrd[0]);
strtolower(wrd[1]);
strcpy(nl->site,wrd[1]);
strcpy(nl->verification,wrd[3]);
}

#endif


/*** Parse transports section ***/
void parse_transports_section(void) {
static int in_section=0;
int i;
char *ptr1,*ptr2,c;
RM_OBJECT room;

if (!strcmp(wrd[0],"TRANSPORTS:")) {
  if (++in_section>1) {
    fprintf(stderr,"Lotos: Unexpected TRANSPORTS section header on line %d.\n",config_line);
    boot_exit(1);
    }
  return;
  }
if (!wrd[5][0]) {
  fprintf(stderr,"Lotos: Required parameter(s) missing on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[0])>ROOM_NAME_LEN) {
  fprintf(stderr,"Lotos: Room map name too long on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[1])>ROOM_LABEL_LEN) {
  fprintf(stderr,"Lotos: Room label too long on line %d.\n",config_line);
  boot_exit(1);
  }
if (strlen(wrd[2])>ROOM_NAME_LEN) {
  fprintf(stderr,"Lotos: Room name too long on line %d.\n",config_line);
  boot_exit(1);
  }
/* Check for duplicate label or name */
for(room=room_first;room!=NULL;room=room->next) {
  if (!strcmp(room->label,wrd[1])) {
    fprintf(stderr,"Lotos: Duplicate room label on line %d.\n",config_line);
    boot_exit(1);
    }
  if (!strcmp(room->name,wrd[2])) {
    fprintf(stderr,"Lotos: Duplicate room name on line %d.\n",config_line);
    boot_exit(1);
    }
  }
room=create_room();
room->transp=create_transport();
room->transp->room=room;
strcpy(room->map,wrd[0]);
strcpy(room->label,wrd[1]);
strcpy(room->name,wrd[2]);
room->transp->place=atoi(wrd[4]);
room->transp->route=atoi(wrd[5]);
room->transp->smer=1;
room->access=FIXED_PUBLIC;

/* Parse internal links bit ie hl,gd,of etc. MUST NOT be any spaces between
   the commas */
i=0;
ptr1=wrd[3];
ptr2=wrd[3];
while(1) {
  while(*ptr2!=',' && *ptr2!='\0') ++ptr2;
  if (*ptr2==',' && *(ptr2+1)=='\0') {
    fprintf(stderr,"Lotos: Missing link label on line %d.\n",config_line);
    boot_exit(1);
    }
  c=*ptr2;  *ptr2='\0';
  if (!strcmp(ptr1,room->label)) {
    fprintf(stderr,"Lotos: Room has a link to itself on line %d.\n",config_line);
    boot_exit(1);
    }
  strcpy(room->link_label[i],ptr1);
  if (c=='\0') break;
  if (++i>=MAX_LINKS) {
    fprintf(stderr,"Lotos: Too many links on line %d.\n",config_line);
    boot_exit(1);
    }
  *ptr2=c;
  ptr1=++ptr2;  
  }
}


void clear_temps(void)
{
	char filename[250];
	DIR *dirp;
	struct dirent *dp;

	printf("Removing temp files ... ");
	if (!(dirp=opendir(TEMPFILES))) {
		printf("Nemozem otvorit TEMPFILES adresar\n");
		return;
		}
	while ((dp=readdir(dirp))!=NULL) {
		if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")) continue;
		sprintf(filename, "%s/%s", TEMPFILES, dp->d_name);
		unlink(filename);
		printf(".");
		}
	(void) closedir(dirp);
	printf(" hotovo\n");
}


void write_pid(void)
{
	FILE *fp;

	if ((fp=fopen(PIDFILE, "rb+"))==NULL) return;
	fprintf(fp, "%d", getpid());
	fclose(fp);
	return;
}


void create_kill_file(void)
{
	FILE *fp;
	int i=0;

	set_crash();
	if ((fp=fopen(KILLFILE, "wb"))==NULL) {
		write_syslog(ERRLOG, 1, "nemozem vytvorit KILLFILE v create_kill_file()\n");
		return;
		}
	sprintf(text, "kill -9 %d\n", getpid());
	while (text[i]) {
		fwrite(text+i, 1, sizeof(char), fp);
		i++;
		}
	fclose(fp);
	sprintf(text, "chmod 500 %s", KILLFILE);
	system(text);
}


#endif /* boots.c */
