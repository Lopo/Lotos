/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
     MAIN CODE - MAIN CODE - MAIN CODE - MAIN CODE - MAIN CODE - MAIN CODE
 *****************************************************************************/
/*****************************************************************************
                     Zakladne funkcie pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __STAR_C__
#define __STAR_C__ 1

#include <stdio.h>
#ifdef _AIX
#include <sys/select.h> 
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/file.h>
#include <dirent.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include "define.h"
#include "prototypes.h"

/* The following ifdef must be made after all of the above.
   I have had a number of requests to seperate the Amnuts code into smaller
   files and also to take out the Netlink functions.  This is a compromise.
   Whereas there is no makefile, so you have to always recompile everything,
   it is an easy way to seperate up the code.  And by using ifdef for the
   Netlinks you can compile without any Netlink functionality.
   I will openly admit that this is a bit of a hack way of doing it.  If you
   don't like the way this is done, then I don't really care :-)
*/
#ifdef NETLINKS
	#include "obj_nl.h"
#endif


/* ostatne treba vzdy */
#include "obj_sys.h"
#include "obj_syspp.h"

#include "star.h"
#include "commands.h"
#include "ignval.h"


/******************************************************************************
 The setting up of the talker configs and the main program loop
 *****************************************************************************/
int main(int argc,char *argv[])
{
	fd_set readmask; 
	int i,len; 
	char inpstr[ARR_SIZE];
	UR_OBJECT user,next;
#ifdef NETLINKS
	NL_OBJECT nl;
#endif

	strcpy(progname, argv[0]);
	if (argc<2) strcpy(confile, CONFIGFILE);
	else strcpy(confile, argv[1]);

/* Run in background automatically. */
/* moved here because of .reboot wasn't working properly with the forking after
   the rest of the initiation had taken place.
   */
//#ifndef DEBUG
	switch (fork()) {
		case -1: boot_exit(11);  /* fork failure */
		case  0: break; /* child continues */
		default: sleep(1); exit(0);  /* parent dies */
		}
//#endif

/* Startup and do initial counts and parses */
#ifdef DEBUG
	crash_step=0;
#endif
	create_system();
	create_systempp();
	set_date_time();
	init_signals();
	write_syslog(SYSLOG,0,"------------------------------------------------------------------------------\nSERVER BOOTING\n");
	printf("\n------------------------------------------------------------------------------\n");
	printf("%s %s server boot %s\n", reg_sysinfo[TALKERNAME], TVERSION, long_date(1));
	printf("Lotos system v%s\n", OSSVERSION);
	printf("------------------------------------------------------------------------------\n");
	printf("Systemove meno : %s, release %s, %s\n",amsys->sysname,amsys->sysrelease,amsys->sysversion);
	printf("Spustene na    : %s, %s\n",amsys->sysmachine,amsys->sysnodename);
#ifndef NETLINKS
	printf("Netlinky       : Zakazane\n");
#else
	printf("Netlinky       : Povolene\n");
#endif
	load_and_parse_config();

	printf("Flood ochrana je %s.\n", offon[amsys->flood_protect]);
	if (amsys->personal_rooms) {
		if (amsys->startup_room_parse) parse_user_rooms();
		else printf("Osobne miestnosti su aktivne, ale neskontrolovane pri boote.\n");
		}
	else printf("Osobne miestnosti zakazane.\n");
	printf("Kontrolujem strukturu user adresarov\n"); check_directories();
	printf("Processing user list\n"); process_users();
	printf("Pocitam userov\n"); count_users();
	printf("Rozbor struktury prikazov\n"); parse_commands();

	purge(0,NULL,0);
	if (!amsys->auto_purge) printf("PURGE: Auto-purge je vyp.\n");
	else printf("PURGE: Checked %d user%s, %d %s deleted due to lack of use.\n",
		amsys->purge_count,PLTEXT_S(amsys->purge_count),amsys->users_purged,
		PLTEXT_WAS(amsys->users_purged));

	check_messages(NULL,1);
	count_suggestions();
	printf("There %s %d suggestion%s.\n",PLTEXT_WAS(amsys->suggestion_count),amsys->suggestion_count,PLTEXT_S(amsys->suggestion_count));
	count_motds(0);
	printf("There %s %d login motd%s and %d post-login motd%s\n",
		PLTEXT_WAS(amsys->motd1_cnt),amsys->motd1_cnt,PLTEXT_S(amsys->motd1_cnt),amsys->motd2_cnt,
		PLTEXT_S(amsys->motd2_cnt));

/* open the talker after everything else has parsed */
	if (!strcmp(argv[argc-1], "-reinit")) reinit_sockets();
	else init_sockets();
	init_hostsfile();
#ifdef NETLINKS
	if (amsys->auto_connect) init_connections();
	else printf("Preskakujem pripajaciu cast.\n");
#endif

/* finish off the boot-up process */
	reset_alarm();
	osstar_load();
	load_counters();
	reloads(NULL);
	load_swear_file(NULL);
	if (!strcmp(argv[argc-1], "-reinit")) restore_structs();
#ifndef DEBUG
	clear_temps();
#endif
	create_kill_file();
	printf("------------------------------------------------------------------------------\n");
	printf("Nabootovane s PID %u\n",amsys->pid);
	printf("------------------------------------------------------------------------------\n\n");
	write_syslog(SYSLOG,0,"------------------------------------------------------------------------------\n");
	write_syslog(SYSLOG,0,"SERVER BOOTED with PID %u %s\n",amsys->pid,long_date(1));
	write_syslog(SYSLOG,0,"------------------------------------------------------------------------------\n\n");

	write_pid();

/******************************************************************************
 Main program loop
 *****************************************************************************/
	setjmp(jmpvar); /* jump to here if we crash and crash_action = IGNORE */
	while (1) {
  /* set up mask then wait */
		setup_readmask(&readmask);
		if (select(FD_SETSIZE,&readmask,0,0,0)==-1) continue;
  /* check for connection to listen sockets */
		for (i=0;i<port_total;++i) {
			if (FD_ISSET(listen_sock[i],&readmask))
				accept_connection(listen_sock[i],i);
			}
#ifdef NETLINKS
    /* Cycle through client-server connections to other talkers */
		for (nl=nl_first;nl!=NULL;nl=nl->next) {
			no_prompt=0;
			if (nl->type==UNCONNECTED || !FD_ISSET(nl->socket,&readmask)) continue;
      /* See if remote site has disconnected */
			if (!(len=read(nl->socket,inpstr,sizeof(inpstr)-3))) {
				write_syslog(NETLOG,1,"NETLINK: Remote disconnect by %s.\n",
					(nl->stage==UP)?nl->service:nl->site);
				vwrite_room(NULL,"~OLSYSTEM:~RS Stratena linka %s v %s.\n",
					nl->service,nl->connect_room->name);
				shutdown_netlink(nl);
				continue;
				}
			inpstr[len]='\0'; 
			exec_netcom(nl,inpstr);
			}
#endif
  /* Cycle through users. Use a while loop instead of a for because
     user structure may be destructed during loop in which case we
     may lose the user->next link. */
		user=user_first;
		while (user!=NULL) {
			next=user->next; /* store in case user object is destructed */
    /* If remote user or clone ignore */
			if (user->type!=USER_TYPE) {  user=next;  continue; }
    /* see if any data on socket else continue */
			if (!FD_ISSET(user->socket,&readmask)) { user=next;  continue; }
    /* see if client (eg telnet) has closed socket */
			inpstr[0]='\0';
			if (!(len=read(user->socket,inpstr,sizeof(inpstr)))) {
				disconnect_user(user);
				user=next;
				continue;
				}
    /* ignore control code replies */
			if ((unsigned char)inpstr[0]==255) { user=next;  continue; }
    /* Deal with input chars. If the following if test succeeds we
       are dealing with a character mode client so call function. */
			if (inpstr[len-1]>=32 || user->buffpos) {
				if (get_charclient_line(user,inpstr,len)) goto GOT_LINE;
				user=next;
				continue;
				}
			else terminate(inpstr);

GOT_LINE:
			no_prompt=0;  
			com_num=-1;
			force_listen=0; 
			destructed=0;
			user->buff[0]='\0';  
			user->buffpos=0;
			user->last_input=time(0);
			if (user->login>0) {
				login(user,inpstr);
				user=next;
				continue;
				}
    /* If a dot on its own then execute last inpstr unless its a misc
       op or the user is on a remote site */
			if (!user->misc_op) {
				if (!strcmp(inpstr,".") && user->inpstr_old[0]) {
					strcpy(inpstr,user->inpstr_old);
					vwrite_user(user,"%s\n",inpstr);
					}
      /* else save current one for next time */
				else {
					if (inpstr[0]) strncpy(user->inpstr_old,inpstr,REVIEW_LEN);
					}
				}
    /* Main input check */
			clear_words();
			if (!user->misc_op && !user->edit_op && user->set_mode==SET_NONE) {
				if (!check_macros(user,inpstr)) {
					prompt(user);
					user=next;
					continue;
					}
				}
			clear_words();
			word_count=wordfind(inpstr);
			if (user->afk) {
				if (user->afk==2) {
					user->tmp_int=user->terminal.blind;
					user->terminal.blind=0;
					if (!word_count) {
						if (user->command_mode) prompt(user);
						user->terminal.blind=user->tmp_int;
						user=next;
						continue;
						}
					if (!strcmp(word[0], "who")) {
						who(user, 0);
						user->terminal.blind=user->tmp_int;
						user=next;
						continue;
						}
					if (strcmp((char *)crypt(word[0],crypt_salt),user->pass)) {
						write_user(user, password_bad);
						prompt(user);
						user->terminal.blind=user->tmp_int;
						user=next;
						continue;
						}
					echo_on(user);
					cls(user);
					user->terminal.blind=0;
					write_user(user,"Session unlocked, you are no longer AFK.\n");
					}
				else write_user(user,"You are no longer AFK.\n");  
				user->afk_mesg[0]='\0';
				if (user->afkbuff[0][0]) revafk(user);
				if (user->vis) vwrite_room_except(user->room,user,"%s~RS comes back from being AFK.\n",user->recap);
				if (user->afk==2) {
					user->afk=0;
					user->status='a';
					prompt(user);
					user=next;
					continue;
					}
				user->afk=0;
				user->status='a';
				}
			if (!word_count) {
				if (misc_ops(user,inpstr))  {  user=next;  continue;  }
				if (setops(user,inpstr))  {  user=next;  continue;  }
#ifdef NETLINKS
				if (user->room==NULL) {
					sprintf(text,"ACT %s NL\n",user->name);
					write_sock(user->netlink->socket,text);
					}
#endif
				prompt(user);
				user=next;  continue;
				}
			if (misc_ops(user,inpstr))  {  user=next;  continue;  }
			if (setops(user,inpstr))  {  user=next;  continue;  }
			com_num=-1;
			if (user->command_mode || strchr(".>;:</&![@'*+-,?#",inpstr[0]))
				exec_com(user,inpstr);
			else if (!(chck_pblo(user, inpstr))) say(user,inpstr);
			if (!destructed) {
				if (user->room!=NULL && !destructed) prompt(user); 
				else {
					switch (com_num) {
	  /* case -1  :  Not in enumerated values - Unknown command */
#ifdef NETLINKS
						case HOME:
#endif
						case QUIT:
						case SUICIDE:
						case REBOOT:
						case SHUTDOWN: prompt(user);
						default: break;
						}
					}
				}
			user=next;
			} /* end while(user) */
		} /* end while(1) */
} /* main */


/******************************************************************************
 String functions - comparisons, convertions, etc
 *****************************************************************************/
/* tieto dve funkcie tu zostali kvoli NUM_COLS */

/*** Count the number of colour commands in a string ***/
int colour_com_count(char *str)
{
	char *s;
	int i,cnt;

	set_crash();
	s=str;  cnt=0;
	while(*s) {
		if (*s=='~') {
			++s;
			for(i=0;i<NUM_COLS;++i) {
				if (!strncmp(s,colour_codes[i].txt_code,2)) {
					cnt++;
					s++;
					break;
					}
				}
			continue;
			}
		++s;
		}
	return cnt;
}


/*** Strip out colour commands from string for when we are sending strings
     over a netlink to a talker that doesn't support them ***/
char *colour_com_strip(char *str)
{
	char *s,*t;
	static char text2[ARR_SIZE];
	int i;

	set_crash();
	s=str;  t=text2;
	while(*s) {
		if (*s=='~') {
			++s;
			for(i=0;i<NUM_COLS;++i) {
				if (!strncmp(s,colour_codes[i].txt_code,2)) {
					s++;
					goto CONT_M;
					}
				}
			--s;
			*t++=*s;
			}
		else *t++=*s;
CONT_M:
		s++;
		}
	*t='\0';
	return text2;
}


/******************************************************************************
 Object functions
 *****************************************************************************/

/*** Construct user/clone object ***/
UR_OBJECT create_user(void)
{
UR_OBJECT user;

	set_crash();
if ((user=(UR_OBJECT)malloc(sizeof(struct user_struct)))==NULL) {
  write_syslog(ERRLOG,1,"Memory allocation failure in create_user().\n");
  return NULL;
  }
/* Append object into linked list. */
if (user_first==NULL) {  
  user_first=user;  user->prev=NULL;  
  }
else {  
  user_last->next=user;  user->prev=user_last;  
  }
user->next=NULL;
user_last=user;

/* initialise user structure */
user->type=USER_TYPE;
user->socket=-1;
user->attempts=0;
user->login=0;
user->port=0;
user->site_port=0;
user->name[0]='\0';
user->site[0]='\0';
reset_user(user);
return user;
}


/* reset the user variables */
void reset_user(UR_OBJECT user)
{
int i;

	set_crash();
strcpy(user->email,"#UNSET");
strcpy(user->homepage,"#UNSET");
strcpy(user->verify_code,"#NONE");
strcpy(user->version,USERVER);
user->recap[0]='\0';
user->bw_recap[0]='\0';
strcpy(user->desc, default_desc);
strcpy(user->in_phrase, default_inphr);
strcpy(user->out_phrase, default_outphr);
user->afk_mesg[0]='\0';
user->pass[0]='\0';
user->last_site[0]='\0';
user->page_file[0]='\0';
user->mail_to[0]='\0';
user->inpstr_old[0]='\0';
user->buff[0]='\0'; 
user->call[0]='\0';
user->samesite_check_store[0]='\0';
user->invite_by[0]='\0';
strcpy(user->logout_room,room_first->name);
strcpy(user->date,(long_date(1)));
strcpy(user->icq,"#UNSET");
for (i=0; i<MAX_IGNORES; ++i) user->ignoreuser[i][0]='\0';
for (i=0;i<MAX_COPIES;++i) user->copyto[i][0]='\0';
for (i=0;i<MAX_FRIENDS;++i) user->friend[i][0]='\0';
for(i=0;i<REVTELL_LINES;++i) user->afkbuff[i][0]='\0';
for(i=0;i<REVTELL_LINES;++i) user->editbuff[i][0]='\0';
for(i=0;i<REVTELL_LINES;++i) user->revbuff[i][0]='\0';
#ifdef NETLINKS
 user->netlink=NULL;
 user->pot_netlink=NULL; 
#endif
user->room=NULL;
user->invite_room=NULL;
user->malloc_start=NULL;
user->malloc_end=NULL;
user->owner=NULL;
user->wrap_room=NULL;
user->t_expire=time(0)+(NEWBIE_EXPIRES*86400);
user->read_mail=time(0);
user->last_input=time(0);
user->last_login=time(0);
user->level=NEW;
user->real_level=user->level;
user->unarrest=NEW;
user->arrestby=0;
user->buffpos=0;
user->filepos=0;
user->command_mode=0;
user->vis=1;
user->muzzled=0;
user->remote_com=-1;
user->last_login_len=0;
user->total_login=0;
user->prompt=amsys->prompt_def;
user->misc_op=0;
user->edit_op=0;
user->edit_line=0;
user->charcnt=0;
user->warned=0;
user->accreq=0;
user->afk=0;
user->revline=0;
user->clone_hear=CLONE_HEAR_ALL;
user->wipe_to=0;
user->wipe_from=0;
user->logons=0;
user->expire=1;
user->lroom=0;
user->monitor=0;
user->gender=NEUTER;
user->age=0;
user->hideemail=1;
user->misses=0;
user->hits=0;
user->kills=0;
user->deaths=0;
user->bullets=6;
user->hps=10;
user->alert=0;
user->mail_verified=0;
user->autofwd=0;
user->editing=0;
user->hwrap_lev=0;
user->afkline=0;
user->editline=0;
user->show_pass=0;
user->samesite_all_store=0;
user->hwrap_id=0;
user->hwrap_same=0;
user->hwrap_func=0;
user->cmd_type=1;
user->show_rdesc=1;
user->lmail_lev=-3; /* has to be -3 */
for (i=0;i<MAX_REMINDERS;i++) {
  user->reminder[i].day=0;
  user->reminder[i].month=0;
  user->reminder[i].year=0;
  user->reminder[i].msg[0]='\0';
  }
user->temp_remind.day=0;
user->temp_remind.month=0;
user->temp_remind.year=0;
user->temp_remind.msg[0]='\0';
for (i=0;i<MAX_XCOMS;i++) user->xcoms[i]=-1;
for (i=0;i<MAX_GCOMS;i++) user->gcoms[i]=-1;
for (i=0;i<MAX_PAGES;i++) user->pages[i]=0;
user->pagecnt=0;
user->user_page_pos=0;
user->user_page_lev=0;
user->tmp_int=0;
user->first_macro=NULL;
user->last_macro=NULL;
user->prompt_str[0]='\0';
user->ltell[0]='\0';
user->nameg[0]='\0';
user->named[0]='\0';
user->namea[0]='\0';
user->namel[0]='\0';
user->namei[0]='\0';
user->pueblo=0;
user->pueblo_mm=0;
user->pueblo_pg=0;
user->voiceprompt=1;
user->pblodetect=1;
user->alarm=0;
user->atime=0;
reset_murlist(user);
user->restrict[0]='\0';
user->ign_word=NULL;
user->auth_addr=0;
user->set_mode=SET_NONE;
user->set_op=0;
user->who_type=1;
user->status='a';
user->terminal.bckg=0;
user->terminal.txt=0;
user->terminal.revers=0;
user->terminal.blink=0;
user->terminal.bold=0;
user->terminal.underline=0;
user->terminal.clear=0;
user->terminal.music=0;
user->terminal.xterm=0;
user->terminal.checho=amsys->charecho_def;
user->terminal.wrap=0;
user->terminal.blind=0;
user->terminal.pager=23;
user->ignore.all=0;
user->ignore.all_store=0;
user->ignore.tells=0;
user->ignore.logons=0;
user->ignore.shouts=0;
user->ignore.pics=0;
user->ignore.wiz=0;
user->ignore.greets=0;
user->ignore.beeps=0;
user->ignore.transp=0;
user->ignore.funs=0;
user->money=DEFAULT_MONEY;
user->bank=DEFAULT_BANK;
user->inctime=0;
user->kradnutie=0;
}


/*** Destruct an object. ***/
void destruct_user(UR_OBJECT user)
{
	set_crash();
/* Remove from linked list */
if (user==user_first) {
  user_first=user->next;
  if (user==user_last) user_last=NULL;
  else user_first->prev=NULL;
  }
else {
  user->prev->next=user->next;
  if (user==user_last) { 
    user_last=user->prev;  user_last->next=NULL; 
    }
  else user->next->prev=user->prev;
  }
free(user);
destructed=1;
}


/*** Construct room object ***/
RM_OBJECT create_room(void)
{
RM_OBJECT room;
int i;

	set_crash();
if ((room=(RM_OBJECT)malloc(sizeof(struct room_struct)))==NULL) {
  fprintf(stderr,"Lotos: Memory allocation failure in create_room().\n");
  boot_exit(1);
  }
/* Append object into linked list. */
if (room_first==NULL) {  
  room_first=room;  room->prev=NULL;  
  }
else {  
  room_last->next=room;  room->prev=room_last;
  }
room->next=NULL;
room_last=room;

room->name[0]='\0';
room->label[0]='\0';
room->desc[0]='\0';
room->topic[0]='\0';
room->map[0]='\0';
room->access=-1;
room->revline=0;
room->mesg_cnt=0;
room->transp=NULL;
#ifdef NETLINKS
  room->inlink=0;
  room->netlink=NULL;
  room->netlink_name[0]='\0';
#endif
room->next=NULL;
for(i=0;i<MAX_LINKS;++i) {
  room->link_label[i][0]='\0';  room->link[i]=NULL;
  }
for(i=0;i<REVIEW_LINES;++i) room->revbuff[i][0]='\0';
return room;
}


/*** Destruct a room object. ***/
void destruct_room(RM_OBJECT rm) {
/* Remove from linked list */
if (rm==room_first) {
  room_first=rm->next;
  if (rm==room_last) room_last=NULL;
  else room_first->prev=NULL;
  }
else {
  rm->prev->next=rm->next;
  if (rm==room_last) { 
    room_last=rm->prev;  room_last->next=NULL; 
    }
  else rm->next->prev=rm->prev;
  }
if (rm->transp!=NULL) destruct_transport(rm->transp);
free(rm);
}


/* add a command to the commands linked list.  Get which command via the passed id
   int and the enum in the header file */
int add_command(int cmd_id)
{
int inserted;
struct command_struct *cmd,*tmp;

	set_crash();
if ((cmd=(struct command_struct *)malloc(sizeof(struct command_struct)))==NULL) {
  write_syslog(ERRLOG,1,"Memory allocation failure in add_command().\n");
  return 0;
  }

/* 
   do an insertion sort on the linked list
   this could take a long time, but it only needs to be done once when booting, so it
   doesn't really matter
*/

inserted=0;
strcpy(cmd->name,command_table[cmd_id].name);
if (first_command==NULL) {
  first_command=cmd;
  cmd->prev=NULL;
  cmd->next=NULL;
  last_command=cmd;
  }
else {
  tmp=first_command;
  inserted=0;
  while(tmp!=NULL) {
    /* insert as first item in the list */
    if ((strcmp(cmd->name,tmp->name)<0) && tmp==first_command) {
      first_command->prev=cmd;
      cmd->prev=NULL;
      cmd->next=first_command;
      first_command=cmd;
      inserted=1;
      }
    /* insert in the middle of the list somewhere */
    else if (strcmp(cmd->name,tmp->name)<0) {
      tmp->prev->next=cmd;
      cmd->prev=tmp->prev;
      tmp->prev=cmd;
      cmd->next=tmp;
      inserted=1;
      }
    if (inserted) break;
    tmp=tmp->next;
    } /* end while */
  /* insert at the end of the list */
  if (!inserted) {
    last_command->next=cmd;
    cmd->prev=last_command;
    cmd->next=NULL;
    last_command=cmd;
    }
  } /* end else */

cmd->id=cmd_id;
strcpy(cmd->alias,command_table[cmd_id].alias);
cmd->min_lev=command_table[cmd_id].level;
cmd->function=command_table[cmd_id].function;
cmd->count=0;
return 1;
}


/* destruct command nodes */
int rem_command(int cmd_id)
{
struct command_struct *cmd;

	set_crash();
cmd=first_command;
/* as command object not being passed, first find what node we want to delete
   for the id integer that *is* passed.
   */
while(cmd!=NULL) {
  if (cmd->id==cmd_id) break;
  cmd=cmd->next;
  }
if (cmd==first_command) {
  first_command=cmd->next;
  if (cmd==last_command) last_command=NULL;
  else first_command->prev=NULL;
  }
else {
  cmd->prev->next=cmd->next;
  if (cmd==last_command) { 
    last_command=cmd->prev;  last_command->next=NULL; 
    }
  else cmd->next->prev=cmd->prev;
  }
free(cmd);
return 1;
}


/* add a user node to the user linked list */
int add_user_node(char *name, int level)
{
struct user_dir_struct *new;

	set_crash();
if ((new=(struct user_dir_struct *)malloc(sizeof(struct user_dir_struct)))==NULL) {
  write_syslog(ERRLOG,1,"Memory allocation failure in add_user_node().\n");
  return 0;
  }
if (first_dir_entry==NULL) {
  first_dir_entry=new;
  new->prev=NULL;
  }
else {
  last_dir_entry->next=new;
  new->prev=last_dir_entry;
  }
new->next=NULL;
last_dir_entry=new;

++amsys->user_count;
strcpy(new->name,name);
new->level=level;
new->date[0]='\0';
return 1;
}


/* remove a user node from the user linked list
   have needed to use an additional check for the correct user, ie, lev.
   if lev = -1 then don't do a check on the user node's level, else use
   the lev and the name as a check
   */
int rem_user_node(char *name, int lev)
{
int level,found;
struct user_dir_struct *entry;

	set_crash();
entry=first_dir_entry;
found=0;
if (lev!=-1) {
  while(entry!=NULL) {
    if ((!strcmp(entry->name,name)) && (entry->level==lev)) {
      level=entry->level;  found=1;  break;
      }
    entry=entry->next;
    }
  }
else {
  while(entry!=NULL) {
    if (!strcmp(entry->name,name)) {
      level=entry->level;  found=1;  break;
      }
    entry=entry->next;
    }
  }
if (!found) return 0;
if (entry==first_dir_entry) {
  first_dir_entry=entry->next;
  if (entry==last_dir_entry) last_dir_entry=NULL;
  else first_dir_entry->prev=NULL;
  }
else {
  entry->prev->next=entry->next;
  if (entry==last_dir_entry) { 
    last_dir_entry=entry->prev;  last_dir_entry->next=NULL; 
    }
  else entry->next->prev=entry->prev;
  }
free(entry);
--amsys->user_count;
return 1;
}


/* put a date string in a node in the directory linked list that
   matches name
   */
void add_user_date_node(char *name, char *date)
{
struct user_dir_struct *entry;

	set_crash();
for (entry=first_dir_entry;entry!=NULL;entry=entry->next) {
  if (!strcmp(entry->name,name)) {
    strcpy(entry->date,date);
    break;
    }
  }
return;
}


/* add a node to the wizzes linked list */
int add_wiz_node(char *name, int level)
{
struct wiz_list_struct *new;

	set_crash();
if ((new=(struct wiz_list_struct *)malloc(sizeof(struct wiz_list_struct)))==NULL) {
  write_syslog(ERRLOG,1,"Memory allocation failure in add_wiz_node().\n");
  return 0;
  }
if (first_wiz_entry==NULL) {
  first_wiz_entry=new;
  new->prev=NULL;
  }
else {
  last_wiz_entry->next=new;
  new->prev=last_wiz_entry;
  }
new->next=NULL;
last_wiz_entry=new;

strcpy(new->name,name);
new->level=level;
return 1;
}


/* remove a node from the wizzes linked list
   have needed to use an additional check for the correct user, ie, lev.
   if lev = -1 then don't do a check on the user node's level, else use
   the lev and the name as a check
   */
int rem_wiz_node(char *name)
{
int level,found;
struct wiz_list_struct *entry;

	set_crash();
entry=first_wiz_entry;
found=0;

while(entry!=NULL) {
  if (!strcmp(entry->name,name)) {
    level=entry->level;  found=1;  break;
    }
  entry=entry->next;
  }
if (!found) return 0;
if (entry==first_wiz_entry) {
  first_wiz_entry=entry->next;
  if (entry==last_wiz_entry) last_wiz_entry=NULL;
  else first_wiz_entry->prev=NULL;
  }
else {
  entry->prev->next=entry->next;
  if (entry==last_wiz_entry) { 
    last_wiz_entry=entry->prev;  last_wiz_entry->next=NULL; 
    }
  else entry->next->prev=entry->prev;
  }
free(entry);
return 1;
}


/*** alter the level of a node in the user linked list ***/
int user_list_level(char *name, int lvl)
{
struct user_dir_struct *entry;

	set_crash();
entry=first_dir_entry;
while(entry!=NULL) {
  if (!strcmp(entry->name,name)) {
    entry->level=lvl;
    return(1);
    }
  entry=entry->next;
  }
return(0);
}



/******************************************************************************
 Perfom checks and searches
 *****************************************************************************/


/*** Check to see if the pattern 'pat' appears in the string 'str'.
     Uses recursion to acheive this 
     ***/
int pattern_match(char *str, char *pat) {
int  i, slraw;

/* if end of both, strings match */
if ((*pat=='\0') && (*str=='\0')) return(1);
if (*pat=='\0') return(0);
if (*pat=='*') {
  if (*(pat+1)=='\0') return(1);
  for(i=0,slraw=strlen(str);i<=slraw;i++)
    if ((*(str+i)==*(pat+1)) || (*(pat+1)=='?'))
      if (pattern_match(str+i+1,pat+2)==1) return(1);
  } /* end if */
else {
  if (*str=='\0') return(0);
  if ((*pat=='?') || (*pat==*str))
    if (pattern_match(str+1,pat+1)==1) return(1);
  } /* end else */
return(0); 
}


/*** See if users site is banned ***/
int site_banned(char *sbanned, int new) {
FILE *fp;
char line[82],filename[500];

if (new) strcpy(filename, NEWBAN);
else strcpy(filename, SITEBAN);
if (!(fp=fopen(filename,"r"))) return (0);
fscanf(fp,"%s",line);
while(!feof(fp)) {
  /* first do full name comparison */
  if (!strcmp(sbanned,line)) {  fclose(fp);  return 1;  }
  /* check using pattern matching */
  if (pattern_match(sbanned,line)) {
    fclose(fp);
    return(1);
    }
  fscanf(fp,"%s",line);
  }
fclose(fp);
return(0);
}


/*** checks to see if someone is already in login-stage on the ports ***/
int login_port_flood(char *asite)
{
UR_OBJECT u;
int cnt=0;

	set_crash();
for(u=user_first;u!=NULL;u=u->next)
  if (u->login && (!strcmp(asite,u->site) || !strcmp(asite,u->ipsite))) cnt++;
if (cnt>=LOGIN_FLOOD_CNT) return 1;
return 0;
}


/*** See if user is banned ***/
int user_banned(char *name) {
FILE *fp;
char line[82];

if (!(fp=fopen(USERBAN, "r"))) return 0;
fscanf(fp,"%s",line);
while(!feof(fp)) {
  if (!strcmp(line,name)) {  fclose(fp);  return 1;  }
  fscanf(fp,"%s",line);
  }
fclose(fp);
return 0;
}


/*** Set room access back to public if not enough users in room ***/
void reset_access(RM_OBJECT rm) {
UR_OBJECT u;
int cnt;

if (rm==NULL || rm->access!=PRIVATE) return; 
cnt=0;
for(u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;
if (cnt<amsys->min_private_users) {
  write_room(rm,"Pristup do ruumy vrateny na ~FGPUBLIC.\n");
  rm->access=PUBLIC;
  /* Reset any invites into the room & clear review buffer */
  for(u=user_first;u!=NULL;u=u->next) {
    if (u->invite_room==rm) u->invite_room=NULL;
    }
  clear_revbuff(rm);
  }
}


/*** Get user struct pointer from name ***/
UR_OBJECT get_user(char *name) {
UR_OBJECT u;

name[0]=toupper(name[0]);
/* Search for exact name */
for(u=user_first;u!=NULL;u=u->next) {
  if (u->login || u->type==CLONE_TYPE) continue;
  if (!strcmp(u->name,name))  return u;
  }
return NULL;
}


/*** Get user struct pointer from abreviated name ***/
UR_OBJECT get_user_name(UR_OBJECT user, char *i_name) {
UR_OBJECT u,last;
int found=0;
char name[USER_NAME_LEN],buff[ARR_SIZE];

last=NULL;
strncpy(name,i_name,USER_NAME_LEN);
name[0]=toupper(name[0]);  buff[0]=0;
for (u=user_first;u!=NULL;u=u->next)
  if (!strcmp(u->name,name) && u->room!=NULL) return u;
for (u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE) continue;
    if (s_instr(u->name,name) != -1) {
      strcat(buff,u->name);
      strcat(buff, "  ");
      found++;
      last=u;
      }
  } /* end for */
if (found == 0) return NULL;
if (found >1) {
  write_user(user, "~FR~OLMeno nie je unikatne.\n\n");
  vwrite_user(user,"   ~OL%s\n\n",buff);
  return NULL;
  }
else return(last);
}


/*** Get room struct pointer from abbreviated name ***/
RM_OBJECT get_room(char *name) {
RM_OBJECT rm;

for(rm=room_first;rm!=NULL;rm=rm->next)
  if (!strncmp(rm->name,name,strlen(name))) return rm;
return NULL;
}


/*** Get room struct pointer from full name ***/
RM_OBJECT get_room_full(char *name)
{
	RM_OBJECT rm;

	set_crash();
	for (rm=room_first; rm!=NULL; rm=rm->next)
		if (!strcmp(rm->name, name)) return rm;
	return NULL;
}



/*** See if a user has access to a room. If room is fixed to private then
	it is considered a wizroom so grant permission to any user of WIZ and
	above for those. ***/
int has_room_access(UR_OBJECT user, RM_OBJECT rm)
{
	int i=0;

	set_crash();
/* root check */
if (rm->access==ROOT_CONSOLE) {
	if (user->level==ROOT || user->invite_room==rm) return 1;
	else return 0;
	}
/* level room checks */
while (priv_room[i].name[0]!='*') {
  if (!strcmp(rm->name,priv_room[i].name)
      && user->level<priv_room[i].level
      && user->invite_room!=rm) return 0;
  i++;
  }
/* personal room checks */
if (rm->access==PERSONAL_UNLOCKED) return 1; /* open to all */
if ((rm->access==PERSONAL_LOCKED && is_my_room(user,rm))
    || has_room_key(user->name,rm)
    || user->level>=GOD
    || user->invite_room==rm) return 1;
if ((rm->access==PERSONAL_LOCKED && !is_my_room(user,rm))
    && user->level<GOD
    && user->invite_room!=rm) return 0;
/* fixed room checks */
if ((rm->access & PRIVATE) 
    && user->level<amsys->gatecrash_level 
    && user->invite_room!=rm
    && !((rm->access & FIXED) && user->level>=WIZ)) return 0;
/* have access */
return 1;
}


/* Check the room you're logging into isn't private */
void check_start_room(UR_OBJECT user)
{
	RM_OBJECT rm;

	set_crash();
	if (!user->logout_room[0] || !user->lroom) {
		user->room=room_first;
		return;
		}
	rm=get_room(user->logout_room);
	if (rm==NULL) {
		user->room=room_first;
		return;
		}
	if (rm->access==PRIVATE || (rm->access==FIXED_PRIVATE && user->level<WIZ) || (rm->access==ROOT_CONSOLE && user->level<ROOT)) {
		vwrite_user(user,"\nRuuma v ktorej si sa odlogoval%s je teraz ~FRPRIVAT~RS - pripajam ta do %s.\n\n", grm_gnd(4, user->gender), room_first->name);
		user->room=room_first;
		return;
		}
	user->room=rm;
	if (user->lroom==2)
		vwrite_user(user,"\nBol%s si ~FRprilepen%s~RS do ruumy ~FG%s~RS\n\n",
			grm_gnd(4, user->gender), grm_gnd(1, user->gender), rm->name);
	else
		vwrite_user(user,"\nYou are connecting into the ~FG%s~RS room.\n\n",
			rm->name);
}


/*** find out if a user is listed in the user linked list ***/
int find_user_listed(char *name) {
struct user_dir_struct *entry;

name[0]=toupper(name[0]);
for (entry=first_dir_entry;entry!=NULL;entry=entry->next)
  if (!strcmp(entry->name,name)) return 1;
return 0;
}


/*** Checks to see if a user with the given name is currently logged on ***/
int user_logged_on(char *name) {
UR_OBJECT u;

for (u=user_first;u!=NULL;u=u->next) {
  if (u->login) continue;
  if (!strcmp(name,u->name)) return 1;
  }
return 0;
}


/*** Checks to see if the room a user is in is private ***/
int in_private_room(UR_OBJECT user) {
if (user->room->access==PRIVATE || user->room->access==FIXED_PRIVATE || user->room->access==ROOT_CONSOLE)  return 1;
return 0;
}


/*** Check to see if a command has been given to a user ***/
int has_gcom(UR_OBJECT user, int cmd_id) {
int i;

for (i=0;i<MAX_GCOMS;i++) if (user->gcoms[i]==cmd_id) return 1;
return 0;
}


/*** Check to see if a command has been taken from a user ***/
int has_xcom(UR_OBJECT user, int cmd_id) {
int i;

for (i=0;i<MAX_XCOMS;i++) if (user->xcoms[i]==cmd_id) return 1;
return 0;
}


/**** check to see if the room given is a personal room ***/
int is_personal_room(RM_OBJECT rm) {
  if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) return 1;
  return 0;
}


/**** find out if the room given is the perosnal room of the user ***/
int is_my_room(UR_OBJECT user,RM_OBJECT rm)
{
	char name[ROOM_NAME_LEN+1];

	set_crash();
	sprintf(name,"(%s)",user->name);
	strtolower(name);
	if (!strcmp(name,rm->name)) return 1;
	return 0;
}


/*** check to see how many people are in a given room ***/
int room_visitor_count(RM_OBJECT rm) {
UR_OBJECT u;
int cnt=0;

if (rm==NULL) return cnt;
for (u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;
return cnt;
}


/*** See if user is banned ***/
int has_room_key(char *visitor,RM_OBJECT rm)
{
	FILE *fp;
	char line[82],filename[500],rmname[USER_NAME_LEN];

	set_crash();
	/* get user's name from room name */
	midcpy(rm->name,rmname,1,strlen(rm->name)-2);
	rmname[0]=toupper(rmname[0]);
	/* check if user is listed */
	sprintf(filename,"%s/%s.K", USERROOMS, rmname);
	if (!(fp=fopen(filename,"r"))) return 0;
	fscanf(fp,"%s",line);
	while(!feof(fp)) {
		if (!strcmp(line,visitor)) {
			fclose(fp);
			return 1;
			}
		fscanf(fp,"%s",line);
		}
	fclose(fp);
	return 0;
}



/******************************************************************************
 Setting up of the sockets
 *****************************************************************************/

/*** Set up readmask for select ***/
void setup_readmask(fd_set *mask) {
UR_OBJECT user;
#ifdef NETLINKS
  NL_OBJECT nl;
#endif
int i;
  
FD_ZERO(mask);
for(i=0;i<port_total;++i) FD_SET(listen_sock[i],mask);
/* Do users */
for (user=user_first;user!=NULL;user=user->next) 
  if (user->type==USER_TYPE) FD_SET(user->socket,mask);
/* Do client-server stuff */
#ifdef NETLINKS
  for(nl=nl_first;nl!=NULL;nl=nl->next) 
    if (nl->type!=UNCONNECTED) FD_SET(nl->socket,mask);
#endif
}


/*** Accept incoming connections on listen sockets ***/
void accept_connection(int lsock, int num) {
	UR_OBJECT user;
	char named_site[80], ip_site[16], motdname[500];
	struct sockaddr_in acc_addr;
	int accept_sock;
	unsigned int size;

	named_site[0]=ip_site[0]='\0';
	size=sizeof(struct sockaddr_in);
	accept_sock=accept(lsock,(struct sockaddr *)&acc_addr,&size);
#ifdef NETLINKS
	if (num==2) {
		accept_server_connection(accept_sock,acc_addr);
		return;
		}
#endif
/* Get addr */
get_net_addresses(acc_addr, ip_site, named_site);

if (site_banned(ip_site,0) || site_banned(named_site,0)) {
  write_sock(accept_sock,"\n\rLoginy z tvojej sajty/domeny su zabanovane.\n\n\r");
  close(accept_sock);
  write_syslog(SYSLOG,1,"Pokus o login zo zabanovanej sajty %s (%s).\n",named_site,ip_site);
  return;
  }
if (amsys->flood_protect && (login_port_flood(ip_site) || login_port_flood(named_site))) {
  write_sock(accept_sock, flood_prompt_r);
  close(accept_sock);
  write_syslog(SYSLOG,1,"Pokus o vyfloodovanie portu zo sajty %s (%s).\n",named_site,ip_site);
  auto_ban_site(ip_site);
  return;
  }
/* get random motd1 and send pre-login message */
if (amsys->motd1_cnt) {
  sprintf(motdname,"%s/motd1/motd%d", MOTDFILES, (get_motd_num(1)));
  more(NULL,accept_sock,motdname);
  }
else {
  sprintf(text,"Wytay na %s!\n\n\rSorry, zda sa, ze logovacia obrazovka sa stratila\n\r",reg_sysinfo[TALKERNAME]);
  write_sock(accept_sock,text);
  }
if (syspp->acounter[3]+amsys->num_of_logins>=amsys->max_users && !num) {
  write_sock(accept_sock,"\n\rSorac, ale momentalne nemozeme akceptovat viac pripojeni.  Skus neskor.\n\n\r");
  close(accept_sock);  
  return;
  }
	if (!num && !syspp->sys_access) {
		write_sock(accept_sock, sys_port_closed);
		close(accept_sock);
		return;
		}
	if (num && !syspp->wiz_access) {
		write_sock(accept_sock, wiz_port_closed);
		close(accept_sock);
		return;
		}
if ((user=create_user())==NULL) {
  sprintf(text,"\n\r%s: nemozem vytvorit session.\n\n\r",syserror);
  write_sock(accept_sock,text);
  close(accept_sock);  
  return;
  }
user->socket=accept_sock;
user->auth_addr=acc_addr.sin_addr.s_addr;
user->login=LOGIN_NAME;
user->last_input=time(0);
if (!num) user->port=port[0]; 
else {
  user->port=port[1];
  write_user(user,"~FT~OL** Wizport login **~RS\n\n");
  }
if (syspp->pueblo_enh)
	vwrite_user(user, "~RS~OLLotos verzia %s - Tento system je \'~FRPueblo 1.10 Enhanced~RS~OL\'.\n\n", OSSVERSION);
strcpy(user->site,named_site);
strcpy(user->ipsite,ip_site);
user->site_port=(int)ntohs(acc_addr.sin_port);
echo_on(user);
write_user(user, login_prompt);
amsys->num_of_logins++;
}


/*** Resolve an IP address if the resolve_ip set to MANUAL in config file.
     This code was written by tref, and submitted to ewtoo.org by ruGG.
     Claimed useful for BSD systems in which gethostbyaddr() calls caused
     extreme hanging/blocking of the talker.  Note, popen is a blocking 
     call however.
     ***/
char *resolve_ip(char* host) {
FILE* hp;
static char str[256];
char *txt,*t;

sprintf(str,"/usr/bin/host %s",host);
hp=popen(str,"r");

*str=0;
fgets(str,255,hp);
pclose(hp);

txt=strchr(str,':');
if (txt) {
  txt++;
  while (isspace(*txt)) txt++;  t=txt;
  while (*t && *t!='\n') t++;   *t=0;
  return(txt);
  }
return(host);
}



/******************************************************************************
 Signal handlers and exit functions
 *****************************************************************************/

/*** Talker signal handler function. Can either shutdown, ignore or reboot
	if a unix error occurs though if we ignore it we're living on borrowed
	time as usually it will crash completely after a while anyway. ***/
void sig_handler(int sig)
{
	set_crash();
	force_listen=1;
	dump_commands(sig);
	switch (sig) {
		case SIGTERM:
			if (amsys->ignore_sigterm) {
				write_syslog(SYSLOG,1,"SIGTERM signal prijaty - ignorujem.\n");
				return;
				}
			write_room(NULL,"\n\n~OLSYSTEM:~FR~LI SIGTERM prijaty, inicializujem vypnutie !\n\n");
			talker_shutdown(NULL,"ukoncovaci signal (SIGTERM)",0); 
			break; /* don't really need this here, but better safe... */
		case SIGSEGV:
			switch (amsys->crash_action) {
				case 0:	
					write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI SAKRA - Chyba segmentacie, inicializujem vypnutie!\n\n");
					talker_shutdown(NULL,"chyba segmentacie (SIGSEGV)",0); 
					break;
				case 1:	
					write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI POZOR - Vyskytla sa chyba segmentacie !\n\n");
					write_syslog(SYSLOG,1,"POZOR: Vyskytla sa chyba segmentacie !\n");
#ifdef DEBUG
					crash_dump();
#endif
					longjmp(jmpvar,0);
					break;
				case 2:
					write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI SAKRA - Chyba segmentacie, inicializujem reboot!\n\n");
#ifdef DEBUG
					crash_dump();
#endif
					talker_shutdown(NULL,"Chyba segmentacie (SIGSEGV)",1); 
					break;
				} /* end switch */
		case SIGBUS:
			switch (amsys->crash_action) {
				case 0:
					write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI SAKRA - Bus error, inicializujem vypnutie!\n\n");
					talker_shutdown(NULL,"a bus error (SIGBUS)",0);
					break;
				case 1:
					write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI POZOR - A bus error has just occured!\n\n");
					write_syslog(SYSLOG,1,"POZOR: A bus error occured!\n");
#ifdef DEBUG
					crash_dump();
#endif
					longjmp(jmpvar,0);
					break;
				case 2:
					write_room(NULL,"\n\n\07~OLSYSTEM:~FR~LI SAKRA - Bus error, inicializujem reboot!\n\n");
#ifdef DEBUG
					crash_dump();
#endif
					talker_shutdown(NULL,"a bus error (SIGBUS)",1);
					break;
				} /* end switch */
		} /* end switch */
}


/*** Exit because of error during bootup ***/
void boot_exit(int code)
{
	set_crash();
switch (code) {
  case 1:
    write_syslog(SYSLOG,1,"BOOT FAILURE: Chyba pri rozbore config fajlu.\n");
    exit(1);

  case 2:
    perror("Lotos: Can't open main port listen socket");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Can't open main port listen socket.\n");
    exit(2);

  case 3:
    perror("Lotos: Can't open wiz port listen socket");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Can't open wiz port listen socket.\n");
    exit(3);

  case 4:
    perror("Lotos: Can't open link port listen socket");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Can't open link port listen socket.\n");
    exit(4);

  case 5:
    perror("Lotos: nemozem prepojit na main port");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem prepojit na main port.\n");
    exit(5);

  case 6:
    perror("Lotos: nemozem prepojit na wiz port");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem prepojit na wiz port.\n");
    exit(6);

  case 7:
    perror("Lotos: nemozem prepojit na link port");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem prepojit na link port.\n");
    exit(7);

  case 8:
    perror("Lotos: Listen error on main port");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Listen error on main port.\n");
    exit(8);

  case 9:
    perror("Lotos: Listen error on wiz port");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Listen error on wiz port.\n");
    exit(9);

  case 10:
    perror("Lotos: Listen error on link port");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Listen error on link port.\n");
    exit(10);

  case 11:
    perror("Lotos: Failed to fork");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Failed to fork.\n");
    exit(11);

  case 12:
    perror("Lotos: Failed to parse user structure");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Failed to parse user structure.\n");
    exit(12);

  case 13:
    perror("Lotos: Failed to parse user commands");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Failed to parse user commands.\n");
    exit(13);

  case 14:
    write_syslog(SYSLOG,1,"BOOT FAILURE: mena levelov nie su unikatne.\n");
    exit(14);

  case 15:
    write_syslog(SYSLOG,1,"BOOT FAILURE: chyba citania struktury adresarov USERFILES.\n");
    exit(15);

  case 16:
    perror("Lotos: struktura adresarov v USERFILES je nekorektna");
    write_syslog(SYSLOG,1,"BOOT FAILURE: struktura adresarov v USERFILES je nekorektna.\n");
    exit(16);

  case 17:
    perror("Lotos: nemozem vytvorit docasnu user strukturu");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem vytvorit docasnu user strukturu.\n");
    exit(17);

  case 18:
    perror("Lotos: Failed to parse a user structure");
    write_syslog(SYSLOG,1,"BOOT FAILURE: Failed to parse a user structure.\n");
    exit(18);

  case 19:
    perror("Lotos: nemozem otvorit adresar USERROOMS pre citanie");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem otvorit adresar USERROOMS.\n");
    exit(19);

  case 20:
    perror("Lotos: nemozem otvorit adresar v MOTDFILES pre citanie");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem otvorit adresar MOTDFILES.\n");
    exit(20);

  case 21:
    perror("Lotos: nemozem vytvorit system object");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem vytvorit system object.\n");
    exit(21);

  case 101:
    perror("Lotos: nemozem otvorit adresar TEXTFILES pre citanie");
    write_syslog(SYSLOG, 1, "BOOT FAILURE: nemozem otvorit adresar TEXTFILES.\n");
    exit(101);

  case 102:
    perror("Lotos: nemozem vytvorit tempfajl v TMPFILES");
    write_syslog(SYSLOG, 1, "BOOT FAILURE: nemozem vytvorit tempfajl v TMPFILES.\n");
    exit(102);

  case 103:
    perror("Lotos: nemozem otvorit subor na citanie v TXTFILES");
    write_syslog(SYSLOG, 1, "BOOT FAILURE: nemozem otvorit subor na citanie v TXTFILES.\n");
    exit(103);

  case 121:
    perror("Lotos: nemozem vytvorit syspp object");
    write_syslog(SYSLOG,1,"BOOT FAILURE: nemozem vytvorit syspp object.\n");
    exit(121);
  } /* end switch */
}


/*** Records when the user last logged on for use with the .last command ***/
void record_last_login(char *name)
{
	int i;

	set_crash();
	for (i=LASTLOGON_NUM; i>0; i--) {
		strcpy(last_login_info[i].name,last_login_info[i-1].name);
		strcpy(last_login_info[i].time,last_login_info[i-1].time);
		last_login_info[i].on=last_login_info[i-1].on;
		}    
	strcpy(last_login_info[0].name,name);
	strcpy(last_login_info[0].time,long_date(1));
	last_login_info[0].on=1;
}


/*** Records when the user last logged out for use with the .last command ***/
void record_last_logout(char *name)
{
int i;

	set_crash();
i=0;
while (i<LASTLOGON_NUM) {
  if (!strcmp(last_login_info[i].name,name)) break;
  i++;
  }
if (i!=LASTLOGON_NUM) last_login_info[i].on=0;
}



/******************************************************************************
 Initializing of globals and other stuff
 *****************************************************************************/
/*** Load the users details ***/
int load_user_details(UR_OBJECT user)
{
FILE *fp;
char user_words[16][40]; /* must be at least 1 longer than max words in the _options */
char line[ARR_SIZE],fname[500],*str;
int  wn,wpos,wcnt,op,found,i,damaged;
char *userfile_options[]={
  "version","password","promote_date","times","levels","general","user_set",
  "fighting","purging","last_site","mail_verify","description",
  "in_phrase","out_phrase","email","homepage","recap_name",
  "prompt", "nick_grm", "pueblo", "restrict", "ign_word", "*" /* KEEP HERE! */
  };

	set_crash();
sprintf(fname,"%s/%s.D", USERFILES, user->name);
if (!(fp=fopen(fname,"r"))) return 0;

damaged=0;
fgets(line,ARR_SIZE-1,fp);
line[strlen(line)-1]='\0';

if (strcmp(USERVER, remove_first(line))) {
	fclose(fp);
	i=load_user_olddetails(user);
	return i;
	}

while (!feof(fp)) {
  /* make this into the functions own word array.  This allows this array to
     have a different length from the general words array. */
  wn=0; wpos=0;
  str=line;
  do {
    while (*str<33) if (!*str++) goto LUOUT;
    while (*str>32 && wpos<40) user_words[wn][wpos++]=*str++;
    user_words[wn++][wpos]='\0';
    wpos=0;
  } while (wn<16);
  wn--;
 LUOUT:
  wcnt=wn;
  /* now get the option we're on */
  op=0;  found=1;
  while (strcmp(userfile_options[op],user_words[0])) {
    if (userfile_options[op][0]=='*') { found=0; break; }
    op++;
    }
  if (found) {
    switch (op) {
      case 0:
	if (wcnt>=2) {      /* make sure more than just option string was there */
	  strcpy(user->version,remove_first(line));
	  }
	break;
      case 1:
	if (wcnt>=2) strcpy(user->pass,remove_first(line));
	break;
      case 2:
	if (wcnt>=2) strcpy(user->date,remove_first(line));
	break;
      case 3:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->last_login=(time_t)atoi(user_words[i]);  break;
	    case 2: user->total_login=(time_t)atoi(user_words[i]);  break;
	    case 3: user->last_login_len=atoi(user_words[i]);  break;
	    case 4: user->read_mail=(time_t)atoi(user_words[i]);  break;
	    }
	break;
      case 4:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->level=atoi(user_words[i]);  break;
	    case 2: user->unarrest=atoi(user_words[i]);  break;
	    case 3: user->arrestby=atoi(user_words[i]);  break;
	    case 4: user->muzzled=atoi(user_words[i]);  break;
	    }
	break;
      case 5:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->accreq=atoi(user_words[i]);  break;
	    case 2: user->command_mode=atoi(user_words[i]);  break;
	    case 3: user->prompt=atoi(user_words[i]);  break;
	    case 4: user->vis=atoi(user_words[i]);  break;
	    case 5: user->monitor=atoi(user_words[i]);  break;
	    case 6: user->mail_verified=atoi(user_words[i]);  break;
	    case 7: user->logons=atoi(user_words[i]);  break;
		case 8: user->terminal.xterm=atoi(user_words[i]);  break;
	    }
	break;
      case 6:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case  1: user->gender=atoi(user_words[i]);  break;
	    case  2: user->age=atoi(user_words[i]);  break;
	    case  3: user->hideemail=atoi(user_words[i]);  break;
	    case  4: user->lroom=atoi(user_words[i]);  break;
	    case  5: user->alert=atoi(user_words[i]);  break;
	    case  6: user->autofwd=atoi(user_words[i]);  break;
	    case  7: user->show_pass=atoi(user_words[i]);  break;
	    case  8: user->show_rdesc=atoi(user_words[i]);  break;
	    case  9: user->cmd_type=atoi(user_words[i]);  break;
	    case 10: strcpy(user->icq, user_words[i]);  break;
	    case 11: user->who_type=atoi(user_words[i]); break;
	    }
	break;
      case 7:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->hits=atoi(user_words[i]);  break;
	    case 2: user->misses=atoi(user_words[i]);  break;
	    case 3: user->deaths=atoi(user_words[i]);  break;
	    case 4: user->kills=atoi(user_words[i]);  break;
	    case 5: user->bullets=atoi(user_words[i]);  break;
	    case 6: user->hps=atoi(user_words[i]);  break;
		case 7: user->money=atoi(user_words[i]); break;
		case 8: user->bank=atoi(user_words[i]); break;
	    }
	break;
      case 8:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->expire=atoi(user_words[i]);  break;
	    case 2: user->t_expire=(time_t)atoi(user_words[i]);  break;
	    }
	break;
      case  9:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: strcpy(user->last_site,user_words[i]);  break;
	    case 2: strcpy(user->logout_room,user_words[i]);  break;
	    }
	break;
      case 10:
	if (wcnt>=2) strcpy(user->verify_code,remove_first(line));
	break;
      case 11:
	if (wcnt>=2) strcpy(user->desc,remove_first(line));
	break;
      case 12:
	if (wcnt>=2) strcpy(user->in_phrase,remove_first(line));
	break;
      case 13:
	if (wcnt>=2) strcpy(user->out_phrase,remove_first(line));
	break;
      case 14:
	if (wcnt>=2) strcpy(user->email,remove_first(line));
	break;
      case 15:
	if (wcnt>=2) strcpy(user->homepage,remove_first(line));
	break;
      case 16:
	if (wcnt>=2) strcpy(user->recap,remove_first(line));
	break;
      case 17:
        for (i=1; i<wcnt; i++)
          switch(i) {
            case 1: strcpy(user->prompt_str, user_words[i]); break;
            }
        break;
      case 18:
        for (i=1; i<wcnt; i++)
          switch(i) {
            case 1: strcpy(user->nameg, user_words[i]); break;
            case 2: strcpy(user->named, user_words[i]); break;
            case 3: strcpy(user->namea, user_words[i]); break;
            case 4: strcpy(user->namel, user_words[i]); break;
            case 5: strcpy(user->namei, user_words[i]); break;
            case 6: strcpy(user->namex, user_words[i]); break;
            case 7: strcpy(user->namey, user_words[i]); break;
            case 8: strcpy(user->namez, user_words[i]); break;
            }
        break;
      case 19:
        for (i=1; i<wcnt; i++)
          switch(i) {
            case 1: user->pueblo_mm=atoi(user_words[i]); break;
            case 2: user->pueblo_pg=atoi(user_words[i]); break;
            case 3: user->voiceprompt=atoi(user_words[i]); break;
            }
        break;
      case 20:
        if (wcnt>=2) strcpy(user->restrict, remove_first(line));
        else strcpy(user->restrict, RESTRICT_MASK);
        break;
      case 21:
        if (wcnt>=2) user->ign_word=strdup(remove_first(line));
	break;
      default: damaged++; break;
      }
    }
  else damaged++;
  fgets(line,ARR_SIZE-1,fp);
  line[strlen(line)-1]=0;
  }
fclose(fp);
/* binarne ukladane data */
	sprintf(fname, "%s/bin/%s.D", USERFILES, user->name);
	if (!(fp=fopen(fname, "rb"))) return 0;
	fread(&user->ignore, sizeof(USER_IGNORE), 1, fp);
	fread(&user->terminal, sizeof(USER_TERMINAL), 1, fp);
	fclose(fp);

if (damaged) write_syslog(SYSLOG,1,"Poskodeny userfajl '%s.D'\n",user->name);
if (!amsys->allow_recaps || !user->recap[0]) strcpy(user->recap,user->name);
strcpy(user->bw_recap,colour_com_strip(user->recap));
user->real_level=user->level;
if (user->level==ROOT && strcmp(user->name, reg_sysinfo[SYSOPUNAME])) {
	user->real_level=NEW;
	write_syslog(SYSLOG, 1, "~CRAutodemote~RS %s from %s to %s - edited userfile\n",
		user->name, user_level[user->level].name, user_level[user->real_level].name);
	sprintf(text, "~CRAutodemote~RS from %s to %s - edited userfile\n",
		user_level[user->level].name, user_level[user->real_level].name);
	add_history(user->name, 1, text);
	user->vis=1;
	add_user_date_node(user->name, long_date(1));
	user->unarrest=NEW;
	user->arrestby=ROOT;
	user->level=JAILED;
	user_list_level(user->name, user->level);
	strcpy(user->date, long_date(1));
	strcpy(user->site, user->last_site);
	save_user_details(user, 0);
	}
if (user->level>NEW) user->accreq=-1;   /* compensate for new accreq change */
get_macros(user);
get_xgcoms(user);
get_friends(user);
read_user_reminders(user);
return 1;
}


/*** Load the old users details ***/
int load_user_olddetails(UR_OBJECT user)
{
FILE *fp;
char user_words[15][20]; /* must be at least 1 longer than max words in the _options */
char line[ARR_SIZE],filename[500],*str;
int  wn,wpos,wcnt,op,found,i,damaged,version_found;
char *userfile_options[]={
  "version","password","promote_date","times","levels","general","user_set",
  "user_ignores","fighting","purging","last_site","mail_verify","description",
  "in_phrase","out_phrase","email","homepage","recap_name",
  "prompt", "nick_grm", "pueblo", "restrict", "ign_word", "*" /* KEEP HERE! */
  };

	set_crash();
write_syslog(SYSLOG, 0, "Nahravam staru verziu userfajlu pre %s, v0.1 > 1.0.0-1.1.1\n", user->name);

sprintf(filename,"%s/%s.D", USERFILES, user->name);
if (!(fp=fopen(filename,"r"))) return 0;

damaged=version_found=0;
fgets(line,ARR_SIZE-1,fp);
line[strlen(line)-1]='\0';

while(!feof(fp)) {
  /* make this into the functions own word array.  This allows this array to
     have a different length from the general words array. */
  wn=0; wpos=0;
  str=line;
  do {
    while(*str<33) if (!*str++) goto LUOUT;
    while(*str>32 && wpos<20) user_words[wn][wpos++]=*str++;
    user_words[wn++][wpos]='\0';
    wpos=0;
  } while (wn<15);
  wn--;
 LUOUT:
  wcnt=wn;
  /* now get the option we're on */
  op=0;  found=1;
  while(strcmp(userfile_options[op],user_words[0])) {
    if (userfile_options[op][0]=='*') { found=0; break; }
    op++;
    }
  if (found) {
    switch(op) {
      case 0:
	if (wcnt>=2) {      /* make sure more than just option string was there */
	  strcpy(user->version,remove_first(line));
	  version_found=1;  /* gotta compensate still for old versions */
	  }
	break;
      case 1:
	if (wcnt>=2) strcpy(user->pass,remove_first(line));
	break;
      case 2:
	if (wcnt>=2) strcpy(user->date,remove_first(line));
	break;
      case 3:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->last_login=(time_t)atoi(user_words[i]);  break;
	    case 2: user->total_login=(time_t)atoi(user_words[i]);  break;
	    case 3: user->last_login_len=atoi(user_words[i]);  break;
	    case 4: user->read_mail=(time_t)atoi(user_words[i]);  break;
	    }
	break;
      case 4:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->level=atoi(user_words[i]);  break;
	    case 2: user->unarrest=atoi(user_words[i]);  break;
	    case 3: user->arrestby=atoi(user_words[i]);  break;
	    case 4: user->muzzled=atoi(user_words[i]);  break;
	    }
	break;
      case 5:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->accreq=atoi(user_words[i]);  break;
	    case 2: user->terminal.checho=atoi(user_words[i]); break;
	    case 3: user->command_mode=atoi(user_words[i]);  break;
	    case 4: user->prompt=atoi(user_words[i]); break;
	    case 5: user->vis=atoi(user_words[i]);  break;
	    case 6: user->monitor=atoi(user_words[i]);  break;
	    case 7: user->mail_verified=atoi(user_words[i]);  break;
	    case 8: user->logons=atoi(user_words[i]);  break;
		case 9: user->terminal.xterm=atoi(user_words[i]);  break;
	    }
	break;
      case 6:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1 : user->gender=atoi(user_words[i]);  break;
	    case 2 : user->age=atoi(user_words[i]);  break;
	    case 3 : user->terminal.wrap=atoi(user_words[i]);  break;
	    case 4 : user->terminal.pager=atoi(user_words[i]);  break;
	    case 5 : user->hideemail=atoi(user_words[i]);  break;
	    case 6 : user->terminal.txt=user->terminal.bckg=atoi(user_words[i]); break;
	    case 7 : user->lroom=atoi(user_words[i]);  break;
	    case 8 : user->alert=atoi(user_words[i]);  break;
	    case 9 : user->autofwd=atoi(user_words[i]);  break;
	    case 10: user->show_pass=atoi(user_words[i]);  break;
	    case 11: user->show_rdesc=atoi(user_words[i]);  break;
	    case 12:
	    	user->cmd_type=atoi(user_words[i]);
		if (user->cmd_type==0) user->cmd_type=1;
		break;
	    case 13: strcpy(user->icq, user_words[i]);  break;
	    }
	break;
      case 7:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case  1: user->ignore.all=atoi(user_words[i]);  break;
	    case  2: user->ignore.tells=atoi(user_words[i]);  break;
	    case  3: user->ignore.shouts=atoi(user_words[i]);  break;
	    case  4: user->ignore.pics=atoi(user_words[i]);  break;
	    case  5: user->ignore.logons=atoi(user_words[i]);  break;
	    case  6: user->ignore.wiz=atoi(user_words[i]);  break;
	    case  7: user->ignore.greets=atoi(user_words[i]);  break;
	    case  8: user->ignore.beeps=atoi(user_words[i]);  break;
	    case  9: user->ignore.transp=atoi(user_words[i]); break;
	    case 10: user->ignore.funs=atoi(user_words[i]); break;
	    }
	break;
      case 8:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->hits=atoi(user_words[i]);  break;
	    case 2: user->misses=atoi(user_words[i]);  break;
	    case 3: user->deaths=atoi(user_words[i]);  break;
	    case 4: user->kills=atoi(user_words[i]);  break;
	    case 5: user->bullets=atoi(user_words[i]);  break;
	    case 6: user->hps=atoi(user_words[i]);  break;
	    }
	break;
      case 9:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: user->expire=atoi(user_words[i]);  break;
	    case 2: user->t_expire=(time_t)atoi(user_words[i]);  break;
	    }
	break;
      case 10:
	for (i=1;i<wcnt;i++)
	  switch(i) {
	    case 1: strcpy(user->last_site,user_words[i]);  break;
	    case 2: strcpy(user->logout_room,user_words[i]);  break;
	    }
	break;
      case 11:
	if (wcnt>=2) strcpy(user->verify_code,remove_first(line));
	break;
      case 12:
	if (wcnt>=2) strcpy(user->desc,remove_first(line));
	break;
      case 13:
	if (wcnt>=2) strcpy(user->in_phrase,remove_first(line));
	break;
      case 14:
	if (wcnt>=2) strcpy(user->out_phrase,remove_first(line));
	break;
      case 15:
	if (wcnt>=2) strcpy(user->email,remove_first(line));
	break;
      case 16:
	if (wcnt>=2) strcpy(user->homepage,remove_first(line));
	break;
      case 17:
	if (wcnt>=2) strcpy(user->recap,remove_first(line));
	break;
      case 18:
        for (i=1; i<wcnt; i++)
          switch(i) {
            case 1: user->prompt=atoi(user_words[i]); break;
            case 2: strcpy(user->prompt_str, user_words[i]); break;
            }
        break;
      case 19:
        for (i=1; i<wcnt; i++)
          switch(i) {
            case 1: strcpy(user->nameg, user_words[i]); break;
            case 2: strcpy(user->named, user_words[i]); break;
            case 3: strcpy(user->namea, user_words[i]); break;
            case 4: strcpy(user->namel, user_words[i]); break;
            case 5: strcpy(user->namei, user_words[i]); break;
            }
        break;
      case 20:
        for (i=1; i<wcnt; i++)
          switch(i) {
            case 1: user->pueblo_mm=atoi(user_words[i]); break;
            case 2: user->pueblo_pg=atoi(user_words[i]); break;
            case 3: user->voiceprompt=atoi(user_words[i]); break;
            }
        break;
      case 21:
        if (wcnt>=2) strcpy(user->restrict, remove_first(line));
        else strcpy(user->restrict, RESTRICT_MASK);
        break;
      case 22:
        if (wcnt>=2) user->ign_word=strdup(remove_first(line));
	break;
      default: damaged++; break;
      }
    }
  else damaged++;
  fgets(line,ARR_SIZE-1,fp);
  line[strlen(line)-1]=0;
  }
fclose(fp);
if (damaged) write_syslog(SYSLOG,1,"Poskodeny userfajl '%s.D'\n",user->name);
if (!amsys->allow_recaps || !user->recap[0]) strcpy(user->recap,user->name);
strcpy(user->bw_recap,colour_com_strip(user->recap));
user->real_level=user->level;
if (user->level>NEW) user->accreq=-1;   /* compensate for new accreq change */
get_macros(user);
get_xgcoms(user);
get_friends(user);
read_user_reminders(user);
return 1;
}


/*** Save the details for the user ***/
int save_user_details(UR_OBJECT user, int save_current)
{
	FILE *fp;
	char fname[500];
	int i;

	set_crash();
	if (user->type==REMOTE_TYPE || user->type==CLONE_TYPE) return 0;
	sprintf(fname,"%s/%s.D", USERFILES, user->name);
	if (!(fp=fopen(fname,"w"))) {
		vwrite_user(user,"%s: chyba pri ukladani tvojich detailov.\n",syserror);
		write_syslog(ERRLOG,1,"SAVE_USER_STATS: pri ukladani detaijlov pre %s\n",user->name);
		return 0;
		}
	/* reset normal level */
	if (user->real_level<user->level) user->level=user->real_level;
	if (!strcmp(user->name, reg_sysinfo[SYSOPUNAME])) {
		user->level=user->real_level=ROOT;
		}
	/* print out the file */
	fprintf(fp,"version       %s\n", USERVER);     /* of user struct */
	fprintf(fp,"password      %s\n", user->pass);
	fprintf(fp,"promote_date  %s\n", user->date);
	if (save_current)
		fprintf(fp,"times         %d %d %d %d\n",
			(int)time(0), (int)user->total_login,
			(int)(time(0)-user->last_login), (int)user->read_mail);
	else
		fprintf(fp,"times         %d %d %d %d\n",
			(int)user->last_login, (int)user->total_login,
			user->last_login_len, (int)user->read_mail);
	fprintf(fp,"levels        %d %d %d %d\n",
		user->level, user->unarrest, user->arrestby, user->muzzled);
	fprintf(fp,"general       %d %d %d %d %d %d %d\n",
		user->accreq, user->command_mode,
		user->prompt, user->vis, user->monitor, user->mail_verified,
		user->logons);
	fprintf(fp,"user_set      %d %d %d %d %d %d %d %d %d %s %d\n",
		user->gender, user->age,
		user->hideemail, user->lroom, user->alert,
		user->autofwd, user->show_pass, user->show_rdesc,
		user->cmd_type, user->icq, user->who_type);
	fprintf(fp,"fighting      %d %d %d %d %d %d %d %d\n",
		user->hits, user->misses, user->deaths, user->kills,
		user->bullets, user->hps, user->money, user->bank);
	if (!save_current)
		fprintf(fp,"purging       %d %d\n",
			user->expire, (int)user->t_expire);
	else {
		if (user->level==NEW)
			fprintf(fp,"purging       %d %d\n",
				user->expire,(int)(time(0)+(NEWBIE_EXPIRES*86400)));
		else
			fprintf(fp,"purging       %d %d\n",
				user->expire,(int)(time(0)+(USER_EXPIRES*86400)));
		}
	if (save_current)
		fprintf(fp,"last_site     %s %s\n",
			user->site, user->room->name);
	else
		 fprintf(fp,"last_site     %s %s\n",
		 	user->last_site, user->logout_room);
	fprintf(fp,"mail_verify   %s\n", user->verify_code);
	fprintf(fp,"description   %s\n", user->desc);
	fprintf(fp,"in_phrase     %s\n", user->in_phrase);
	fprintf(fp,"out_phrase    %s\n", user->out_phrase);
	fprintf(fp,"email         %s\n", user->email);
	fprintf(fp,"homepage      %s\n", user->homepage);
	fprintf(fp,"recap_name    %s\n", user->recap);
	fprintf(fp,"prompt        %s\n", user->prompt_str);
	fprintf(fp,"nick_grm      %s %s %s %s %s %s %s %s\n",
		user->nameg, user->named, user->namea, user->namel, user->namei,
		user->namex, user->namey, user->namez);
	fprintf(fp,"pueblo        %d %d %d\n",
		user->pueblo_mm, user->pueblo_pg, user->voiceprompt);
	/* set default restriction for superior users */
	if (user->level>=MIN_LEV_AUTORST) {
		for (i=0; i<strlen(susers_restrict); i++)
			if (susers_restrict[i]!='.')
				user->restrict[i]=susers_restrict[i];
		}
	fprintf(fp,"restrict      %s\n", user->restrict);
	if (user->ign_word!=NULL)
		fprintf(fp,"ign_word      %s\n", user->ign_word);
	else fprintf(fp, "ign_word\n");
	fclose(fp);

/* cast s binarne ukladanymi datami */
	sprintf(fname, "%s/bin/%s.D", USERFILES, user->name);
	if (!(fp=fopen(fname, "wb"))) {
		vwrite_user(user,"%s: chyba pri ukladani tvojich detailov.\n",syserror);
		write_syslog(ERRLOG,1,"SAVE_USER_STATS: pri ukladani detailov pre %s - bin cast\n",user->name);
		return 0;
		}
	fwrite(&user->ignore, sizeof(USER_IGNORE), 1, fp);
	fwrite(&user->terminal, sizeof(USER_TERMINAL), 1, fp);
	fclose(fp);

	return 1;
}


/* count up how many users of a certain level and in total in the linked user list */
void count_users(void)
{
struct user_dir_struct *entry;
int i;

	set_crash();
/* first zero out all level counts */
amsys->user_count=0;
for (i=JAILED;i<=ROOT;i++) amsys->level_count[i]=0;
/* count up users */
for (entry=first_dir_entry;entry!=NULL;entry=entry->next) {
  amsys->level_count[entry->level]++;
  amsys->user_count++;
  }
}



/* work out how many motds are stored - if an motd file is deleted after
   this count has been made then you need to recount them with the recount
   command.  If you fail to do this and something buggers up because the count
   is wrong then it's your own fault.
   */
int count_motds(int forcecnt)
{
char filename[500];
DIR *dirp;
struct dirent *dp; 
int i;

	set_crash();
amsys->motd1_cnt=0;  amsys->motd2_cnt=0;
for (i=1;i<=2;i++) {
  /* open the directory file up */
  sprintf(filename,"%s/motd%d", MOTDFILES, i);
  if ((dirp=opendir(filename))==NULL) {
    switch(forcecnt) {
      case 0:
	fprintf(stderr,"Lotos: Chyba pri otvarani adresara v count_motds().\n");
	boot_exit(20);
      case 1: return 0;
      }
    }
  /* count up how many files in the directory - not including . and .. */
  while((dp=readdir(dirp))!=NULL) {
    if (strstr(dp->d_name,"motd")) (i==1) ? ++amsys->motd1_cnt : ++amsys->motd2_cnt;
    }
  (void) closedir(dirp);
  }
return 1;
}


/* return a random number for an motd file - if 0 then return 1 */
int get_motd_num(int motd)
{
int num;
srand(time(0));

	set_crash();
if (!amsys->random_motds) return 1;
switch(motd) {
  case 1: num=rand()%amsys->motd1_cnt+1;  break;
  case 2: num=rand()%amsys->motd2_cnt+1;  break;
  default: num=0;  break;
  }
return (!num) ? 1:num;
}



/******************************************************************************
 File functions - reading, writing, counting of lines, etc
 *****************************************************************************/

/* wipes ALL the files belonging to the user with name given */
void clean_files(char *name)
{
	DIR *dirp;
	struct dirent *dp;
	char filename[500];

	set_crash();
	name[0]=toupper(name[0]);
	sprintf(filename,"%s/%s.D", USERFILES, name);
	unlink(filename);
	sprintf(filename,"%s/%s.M", USERMAILS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.P", USERPROFILES, name);
	unlink(filename);
	sprintf(filename,"%s/%s.H", USERHISTORYS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.C", USERCOMMANDS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.MAC", USERMACROS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.F", USERFRIENDS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.R", USERROOMS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.B", USERROOMS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.K", USERROOMS, name);
	unlink(filename);
	sprintf(filename,"%s/%s.REM", USERREMINDERS, name);
	unlink(filename);
	strcpy(filename, USERPLDATAS);
	if ((dirp=opendir(filename))==NULL) {
		write_syslog(ERRLOG, 1, "Chyba pri otvarani adresara v clean_files()\n");
		return;
		}
	sprintf(filename, "%s.", name);
	while ((dp=readdir(dirp))!=NULL)
		if (!strncmp(dp->d_name, filename, strlen(filename))) {
			sprintf(text, "%s/%s", USERPLDATAS, dp->d_name);
			unlink(text);
			}
	(void) closedir(dirp);
}


/* remove a line from the top or bottom of a file.
   where==0 then remove from the top
   where==1 then remove from the bottom 
 */
int remove_top_bottom(char *filename, int where)
{
char line[ARR_SIZE];
FILE *fpi, *fpo;
int i,cnt;

	set_crash();
if (!(fpi=fopen(filename,"r"))) return 0;
if (!(fpo=fopen("temp_file","w"))) {
  fclose(fpi);  return 0;
  }
/* remove from top */
if (where==0) {
  /* get the first line */
  fgets(line,ARR_SIZE,fpi);
  /* get rest of file */
  fgets(line,ARR_SIZE,fpi);
  while(!feof(fpi)) {
    line[strlen(line)-1]='\0';
    fprintf(fpo,"%s\n",line);
    fgets(line,ARR_SIZE-1,fpi);
    }
  }
/* remove from bottom of file */
else {
  i=0;
  cnt=count_lines(filename);
  cnt--;
  fgets(line,ARR_SIZE,fpi);
  while(!feof(fpi)) {
    if (i<cnt) {
      line[strlen(line)-1]='\0';
      fprintf(fpo,"%s\n",line);
      }
    i++;
    fgets(line,ARR_SIZE-1,fpi);
    }
  }
fclose(fpi);  fclose(fpo);
rename("temp_file",filename);
if (!(cnt=count_lines(filename))) unlink(filename);
return 1;
}


/* counts how many lines are in a file */
int count_lines(char *filename)
{
int i,c;
FILE *fp;

	set_crash();
i=0;
if (!(fp=fopen(filename,"r"))) return i;
c=getc(fp);
while (!feof(fp)) {
  if (c=='\n') i++;
  c=getc(fp);
  }
fclose(fp);
return i;
}



/******************************************************************************
 Write functions - users, rooms, system logs
 *****************************************************************************/

/*** Write a NULL terminated string to a socket ***/
void write_sock(int sock, char *str)
{
	set_crash();
  write(sock,str,strlen(str));
}


/* a vargs wrapper for the write_user function.  This will enable you
   to send arguments directly to this function.
   before ya mention it, Squirt, I already had this in a file before your snippet...
   but I gotta admit that your snippet kicked my butt into actually implementing this ;)
*/
void vwrite_user(UR_OBJECT user, char *str, ...)
{
va_list args;

	set_crash();
vtext[0]='\0';
va_start(args,str);
vsprintf(vtext,str,args);
va_end(args);
write_user(user,vtext);
}


/*** Send message to user ***/
void write_user(UR_OBJECT user, char *str)
{
	int buffpos,sock,i,j, cnt;
	char *start,buff[OUT_BUFF_SIZE];
#ifdef NETLINKS
	char mesg[ARR_SIZE];
#endif

	set_crash();
	if (user==NULL) return;
#ifdef NETLINKS
	if (user->type==REMOTE_TYPE) {
		if (user->netlink->ver_major<=3 
			&& user->netlink->ver_minor<2) str=colour_com_strip(str);
		if (str[strlen(str)-1]!='\n') sprintf(mesg,"MSG %s\n%s\nEMSG\n",user->name,str);
		else sprintf(mesg,"MSG %s\n%sEMSG\n",user->name,str);
		write_sock(user->netlink->socket,mesg);
		return;
		}
#endif
	start=str;
	buffpos=0;
	sock=user->socket;
/* Process string and write to buffer. We use pointers here instead of arrays 
   since these are supposedly much faster (though in reality I guess it depends
   on the compiler) which is necessary since this routine is used all the 
   time. */
	cnt=0;
	while (*str) {
		if (*str=='\n') {
			if (buffpos>OUT_BUFF_SIZE-6) {
				write(sock,buff,buffpos);
				buffpos=0;
				}
    /* Reset terminal before every newline */
			memcpy(buff+buffpos,"\033[0m",4);
			buffpos+=4;
			if (user->terminal.txt
				|| user->terminal.revers
				|| user->terminal.bold) {
					memcpy(buff+buffpos, "\033[37m", 5);
					buffpos+=5;
				}
			if (user->terminal.bckg
				|| user->terminal.revers) {
					memcpy(buff+buffpos, "\033[40m", 5);
					buffpos+=5;
				}
   			*(buff+buffpos)='\n';
			*(buff+buffpos+1)='\r';  
			buffpos+=2;
			++str;
			cnt=0;
			}
		else if (user->terminal.wrap && cnt==SCREEN_WRAP) {
			*(buff+buffpos)='\n';
			*(buff+buffpos+1)='\r';  
			buffpos+=2;
			cnt=0;
			}
		else {  
    /* See if its a ^ before a ~ , if so then we print colour command
       as text */
			if (*str=='^' && *(str+1)=='~') {  ++str;  continue;  }
			if (str!=start && *str=='~' && *(str-1)=='^') {
				*(buff+buffpos)=*str;
				goto CONT;
				}
    /* Process colour commands eg ~FR. We have to strip out the commands 
       from the string even if user doesnt have colour switched on hence 
       the user->colour check isnt done just yet */
			if (*str=='~') {
			if (buffpos>OUT_BUFF_SIZE-6) {
				write(sock,buff,buffpos);
				buffpos=0;
				}
			++str;
			for (i=0;i<NUM_COLS;++i) {
				if (!strncmp(str, colour_codes[i].txt_code, 2)) {
					switch (i) {
						case  0: /* RS */
							memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
							buffpos+=strlen(colour_codes[i].esc_code)-1;
							break;
						case  1: /* OL */
							if (user->terminal.bold) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case  2: /* UL */
							if (user->terminal.underline) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case  3: /* LI */
							if (user->terminal.blink) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case  4: /* RV */
							if (user->terminal.revers) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case  5: /* FK */
						case  6: /* FR */
						case  7: /* FG */
						case  8: /* FY */
						case  9: /* FB */
						case 10: /* FM */
						case 11: /* FT */
						case 12: /* FW */
							if (user->terminal.txt) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case 13: /* BK */
						case 14: /* BR */
						case 15: /* BG */
						case 16: /* BY */
						case 17: /* BB */
						case 18: /* BM */
						case 19: /* BT */
						case 20: /* BW */
							if (user->terminal.bckg) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case 21: /* BP */
							if (!user->ignore.beeps) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case 22: /* CS */
							if (user->terminal.clear) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else {
								for (j=0; j<50; j++) {
									memcpy(buff+buffpos, "\n", strlen("\n"));
									buffpos+=strlen("\n")-1;
									}
								}
							break;
						case 23: /* MS */
						case 24: /* MM */
							if (user->terminal.music) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else buffpos--;
							break;
						case 25: /* CK */
						case 26: /* CR */
						case 27: /* CG */
						case 28: /* CY */
						case 29: /* CB */
						case 30: /* CM */
						case 31: /* CT */
						case 32: /* CW */
							if (user->terminal.txt && user->terminal.bold) {
								memcpy(buff+buffpos,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
								buffpos+=strlen(colour_codes[i].esc_code)-1;
								}
							else if (user->terminal.txt) {
								memcpy(buff+buffpos,colour_codes[i-20].esc_code,strlen(colour_codes[i-20].esc_code));
								buffpos+=strlen(colour_codes[i-20].esc_code)-1;
								}
							else if (user->terminal.bold) {
								memcpy(buff+buffpos,colour_codes[1].esc_code,strlen(colour_codes[1].esc_code));
								buffpos+=strlen(colour_codes[1].esc_code)-1;
								}
							else buffpos--;
							break;
						}
					++str;
					--cnt;
					goto CONT;
					}
				}
			*(buff+buffpos)=*(--str);
			}
		else *(buff+buffpos)=*str;
CONT:
		++buffpos;
		++str;
		cnt++;
		}
	if (buffpos==OUT_BUFF_SIZE) {
			write(sock,buff,OUT_BUFF_SIZE);
			buffpos=0;
			}
		}
	if (buffpos) write(sock,buff,buffpos);
/* Reset terminal at end of string */
	write_sock(sock,"\033[0m");
	if (user->terminal.txt
		|| user->terminal.bold
		|| user->terminal.revers) write_sock(sock, "\033[37m");
	if (user->terminal.bckg
		|| user->terminal.revers) write_sock(sock, "\033[40m"); 
}

/*** Write to users of level 'level' and above or below depending on above
     variable; if 1 then above else below.  If user given then ignore that user
     when writing to their level.
     ***/
void write_level(int level, int above, char *str, UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
for(u=user_first;u!=NULL;u=u->next) {
  if ((check_igusers(u,user))!=-1 && user->level<GOD) continue;
  if ((u->ignore.wiz && (com_num==WIZSHOUT || com_num==WIZEMOTE)) || (u->ignore.logons && logon_flag)) continue;
  if (u!=user && !u->login && u->type!=CLONE_TYPE) {
    if ((above && u->level>=level) || (!above && u->level<=level)) {
      if (u->afk) {
	record_afk(u,str);
	continue;
        }
      if (u->editing) {
	record_edit(u,str);
	continue;
        }
      if (!u->ignore.all && !u->editing) write_user(u,str);
      record_tell(u,str);
      }
    }
  }
}


/* a vargs wrapper for the write_room function.  This will enable you
   to send arguments directly to this function 
   This is more a subsid than a wrapper as it doesn't directly call write_room
*/
void vwrite_room(RM_OBJECT rm, char *str, ...)
{
va_list args;

	set_crash();
vtext[0]='\0';
va_start(args,str);
vsprintf(vtext,str,args);
va_end(args);
write_room_except(rm,vtext,NULL);
}


/*** Subsid function to below but this one is used the most ***/
void write_room(RM_OBJECT rm, char *str)
{
	set_crash();
  write_room_except(rm,str,NULL);
}


/* a vargs wrapper for the write_room_except function.  This will enable you
   to send arguments directly to this function 
*/
void vwrite_room_except(RM_OBJECT rm, UR_OBJECT user, char *str, ...)
{
va_list args;

	set_crash();
vtext[0]='\0';
va_start(args,str);
vsprintf(vtext,str,args);
va_end(args);
write_room_except(rm,vtext,user);
}


/*** Write to everyone in room rm except for "user". If rm is NULL write 
     to all rooms. ***/
void write_room_except(RM_OBJECT rm, char *str, UR_OBJECT user)
{
	UR_OBJECT u;
	char text2[ARR_SIZE*2];

	set_crash();
for(u=user_first;u!=NULL;u=u->next) {
  if (u->login 
      || u->room==NULL 
      || (u->room!=rm && rm!=NULL) 
      || (u->ignore.all && !force_listen)
      || (u->ignore.shouts && (com_num==SHOUT || com_num==SEMOTE || com_num==SHOUTTO))
      || (u->ignore.logons && logon_flag)
      || (u->ignore.greets && com_num==GREET)
      || u==user) continue;
  if ((check_igusers(u,user))!=-1 && user->level<ARCH) continue;
  if (u->type==CLONE_TYPE) {
    if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignore.all) continue;
    /* Ignore anything not in clones room, eg shouts, system messages
       and semotes since the clones owner will hear them anyway. */
    if (rm!=u->room) continue;
    if (u->clone_hear==CLONE_HEAR_SWEARS) {
      if (!contains_swearing(str)) continue;
      }
    sprintf(text2, "~FT[ %s ]:~RS %s",u->room->name,str);
    write_user(u->owner, text2);
    }
  else write_user(u,str);
  } /* end for */
}


/*** Write to everyone on the users friends list
     if revt=1 then record to the friends tell buffer
     ***/
void write_friends(UR_OBJECT user, char *str, int revt)
{
UR_OBJECT u;

	set_crash();
for(u=user_first;u!=NULL;u=u->next) {
  if (u->login 
      || u->room==NULL
      || u->type==CLONE_TYPE
      || (u->ignore.all && !force_listen)
      || u==user) continue;
  if ((check_igusers(u,user))!=-1 && user->level<GOD) continue;
  if (!(user_is_friend(user,u))) continue;
  write_user(u,str); 
  if (revt) record_tell(u,str);
  } /* end for */
}


/*** Write a string to system log 
     type = what syslog to write to
     write_time = whether or not you have a time stamp imcluded
     str = string passed - possibly with %s, %d, etc
     ... = variable length args passed
***/
void write_syslog(int type, int write_time, char *str, ...)
{
	FILE *fp;
	int i=0;
	va_list args;
	char filename[500];
	char ext[9];

	set_crash();
	fp=NULL;
	vtext[0]='\0';
	sprintf(ext, "%04d%02u%02u", tyear, tmonth+1, tmday);
	switch(type) {
		case SYSLOG:
			sprintf(filename,"%s/%s.%s", LOGFILES,MAINSYSLOG,ext);
			if (!BIT_TEST(amsys->logging,SYSLOG) || !(fp=fopen(filename,"a"))) return;
			break;
		case REQLOG:
			sprintf(filename,"%s/%s.%s", LOGFILES,REQSYSLOG,ext);
			if (!BIT_TEST(amsys->logging,REQLOG) || !(fp=fopen(filename,"a"))) return;
			break;
#ifdef DEBUG
		case DEBLOG:
			sprintf(filename,"%s/%s.%s", LOGFILES,DEBSYSLOG,ext);
			if (!BIT_TEST(amsys->logging,DEBLOG) || !(fp=fopen(filename,"a"))) return;
			break;
#endif
		case ERRLOG:
			sprintf(filename,"%s/%s.%s", LOGFILES,ERRSYSLOG,ext);
			if (!BIT_TEST(amsys->logging,ERRLOG) || !(fp=fopen(filename,"a"))) return;
			break;
		case NETLOG:
#ifdef NETLINKS
			sprintf(filename,"%s/%s.%s", LOGFILES,NETSYSLOG,ext);
			if (!BIT_TEST(amsys->logging,NETLOG) || !(fp=fopen(filename,"a"))) return;
			break;
#else
			return;
#endif
		}
	va_start(args,str);
	if (write_time) {
		sprintf(vtext,"%02d/%02d %02d:%02d:%02d: ",tmday,tmonth+1,thour,tmin,tsec);
		i=strlen(vtext);
		vsprintf(vtext+i,str,args);
		}
	else vsprintf(vtext,str,args);
	va_end(args);
	fputs(vtext,fp);
	fclose(fp);
}


/* this version of the the last command log - the two procedures below - are
   thanks to Karri (The Bat) Kalpio who makes KTserv */

/* record the last command executed - helps find crashes */
void record_last_command(UR_OBJECT user, int comnum, int len)
{
	set_crash();
sprintf(cmd_history[amsys->last_cmd_cnt & 15],"[%5d] %02d/%02d, %02d:%02d:%02d - %-15s - %3d/%-2d - %s",
	amsys->last_cmd_cnt,tmday,tmonth+1,thour,tmin,tsec,(comnum!=999)?command_table[comnum].name:"pbloenh",len,word_count,user->name);
amsys->last_cmd_cnt++;
}


/* write the commands to the files */
void dump_commands(int foo)
{
	FILE *fp;
	char filename[500];

	set_crash();
	sprintf(filename,"%s/%s.%04d%02u%02u", LOGFILES,LAST_CMD, tyear, tmonth+1, tmday);
	if((fp=fopen(filename, "w"))) {
		int i,j;
		for (i=((j=amsys->last_cmd_cnt-16)>0?j:0); i<amsys->last_cmd_cnt; i++)
			fprintf(fp, "%s\n", cmd_history[i&15]);
		fclose(fp);
		}
}  


/*** shows the name of a user if they are invis.  Records to tell buffer if rec=1 ***/
void write_monitor(UR_OBJECT user, RM_OBJECT rm, int rec)
{
UR_OBJECT u;

	set_crash();
for(u=user_first;u!=NULL;u=u->next) {
  if (u==user || u->login || u->type==CLONE_TYPE) continue;
  if (u->level<command_table[MONITOR].level || !u->monitor) continue;
  if (u->room==rm || rm==NULL) {
    if (!u->ignore.all && !u->editing) vwrite_user(u,"~BB~FG[%s]~RS ",user->name);
    if (rec) record_tell(u,text);
    }
  } /* end for */
}


/*** Page a file out to user. Colour commands in files will only work if 
     user!=NULL since if NULL we dont know if his terminal can support colour 
     or not. Return values: 
       0 = cannot find file, 1 = found file, 2 = found and finished ***/
int more(UR_OBJECT user, int sock, char *filename)
{
	int i,j,buffpos,num_chars,lines,retval,len,cnt,pager,fsize,fperc;
	char buff[OUT_BUFF_SIZE],text2[83],*str;
	struct stat stbuf;
	FILE *fp;

	set_crash();
/* check if file exists */
	if (!(fp=fopen(filename,"r"))) {
		if (user!=NULL) user->filepos=0;  
		return 0;
		}
/* get file size */
	if (stat(filename,&stbuf)==-1) fsize=0;
	else fsize=stbuf.st_size;
/* jump to reading posn in file */
	if (user!=NULL) fseek(fp,user->filepos,0);
	if (user==NULL) pager=23;
	else {
		if (user->terminal.pager<MAX_LINES || user->terminal.pager>99) pager=23;
		else pager=user->terminal.pager;
		}
	text[0]='\0';  
	buffpos=0;  
	num_chars=0;
	retval=1; 
	len=0;
	cnt=0;

/* If user is remote then only do 1 line at a time */
	if (sock==-1) {
		lines=1;
		fgets(text2,82,fp);
		}
	else {
		lines=0;
		fgets(text,sizeof(text)-1,fp);
		}

/* Go through file */
	while(!feof(fp) && (lines<pager || user==NULL)) {
#ifdef NETLINKS
		if (sock==-1) {
			lines++;  
			if (user->netlink->ver_major<=3 && user->netlink->ver_minor<2) str=colour_com_strip(text2);
			else str=text2;
			if (str[strlen(str)-1]!='\n') sprintf(text,"MSG %s\n%s\nEMSG\n",user->name,str);
			else sprintf(text,"MSG %s\n%sEMSG\n",user->name,str);
			write_sock(user->netlink->socket,text);
			num_chars+=strlen(str);
			fgets(text2,82,fp);
			continue;
			}
#endif
		str=text;
  /* Process line from file */
		while (*str) {
			if (*str=='\n') {
				if (buffpos>OUT_BUFF_SIZE-6) {
					write(sock,buff,buffpos);
					buffpos=0;
					}
      /* Reset terminal before every newline */
				if (user!=NULL) {
					memcpy(buff+buffpos,"\033[0m",4);
					buffpos+=4;
					if (user->terminal.txt
						|| user->terminal.revers
						|| user->terminal.bold) {
							memcpy(buff+buffpos, "\033[37m", 5);
							buffpos+=5;
						}
					if (user->terminal.bckg
						|| user->terminal.revers) {
							memcpy(buff+buffpos, "\033[40m", 5);
							buffpos+=5;
						}
					}
				*(buff+buffpos)='\n';
				*(buff+buffpos+1)='\r';  
				buffpos+=2;
				++str;
				cnt=0;
				}
			else if (user!=NULL && user->terminal.wrap && cnt==SCREEN_WRAP) {
				*(buff+buffpos)='\n';
				*(buff+buffpos+1)='\r';  
				buffpos+=2;
				cnt=0;
				}
			else {
      /* Process colour commands in the file. See write_user()
	 function for full comments on this code. */
				if (*str=='^' && *(str+1)=='~') {  ++str;  continue;  }
				if (str!=text && *str=='~' && *(str-1)=='^') {
					*(buff+buffpos)=*str;
					goto CONT;
					}
				if (*str=='~') {
					++str;
	/* process if user name variable */
					if (*str=='$') {
						if (buffpos>OUT_BUFF_SIZE-USER_NAME_LEN) {
							write(sock,buff,buffpos);
							buffpos=0;
							}
						if (user!=NULL) {
							memcpy(buff+buffpos,user->bw_recap,strlen(user->bw_recap));
							buffpos+=strlen(user->bw_recap)-1;
							cnt+=strlen(user->bw_recap)-1;
							}
						else {
							buffpos--;
							cnt--;
							}
						goto CONT;
						}
	/* process if colour variable */
					if (buffpos>OUT_BUFF_SIZE-6) {
						write(sock,buff,buffpos);
						buffpos=0;
						}
	/* ++str; */
					for (i=0;i<NUM_COLS;++i) {
						if (!strncmp(str,colour_codes[i].txt_code,2)) {
							if (user!=NULL) {
								switch (i) {
									case 0: /* RS */
										memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
										buffpos+=strlen(colour_codes[i].esc_code)-1;
										break;
									case 1: /* OL */
										if (user->terminal.bold) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 2: /* UL */
										if (user->terminal.underline) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 3: /* LI */
										if (user->terminal.blink) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 4: /* RV */
										if (user->terminal.revers) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 5: /* FK */
									case 6: /* FR */
									case 7: /* FG */
									case 8: /* FY */
									case 9: /* FB */
									case 10:/* FM */
									case 11:/* FT */
									case 12:/* FW */
										if (user->terminal.txt) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 13: /* BK */
									case 14: /* BR */
									case 15: /* BG */
									case 16: /* BY */
									case 17: /* BB */
									case 18: /* BM */
									case 19: /* BT */
									case 20: /* BW */
										if (user->terminal.bckg) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 21: /* BP */
										if (!user->ignore.beeps) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else buffpos--;
										break;
									case 22: /* CS */
										if (user->terminal.clear) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else {
											for (j=0; j<50; j++) {
												memcpy(buffpos+buff, "\n", strlen("\n"));
												buffpos+=strlen("\n")-1;
												}
											}
										break;
									case 23: /* MS */
									case 24: /* ME */
										if (user->terminal.music) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
										}
										else buffpos--;
										break;
									case 25: /* CK */
									case 26: /* CR */
									case 27: /* CG */
									case 28: /* CY */
									case 29: /* CB */
									case 30: /* CM */
									case 31: /* CT */
									case 32: /* CW */
										if (user->terminal.txt && user->terminal.bold) {
											memcpy(buffpos+buff,colour_codes[i].esc_code,strlen(colour_codes[i].esc_code));
											buffpos+=strlen(colour_codes[i].esc_code)-1;
											}
										else if (user->terminal.txt) {
											memcpy(buffpos+buff,colour_codes[i-20].esc_code,strlen(colour_codes[i-20].esc_code));
											buffpos+=strlen(colour_codes[i-20].esc_code)-1;
											}
										else if (user->terminal.bold) {
											memcpy(buffpos+buff,colour_codes[1].esc_code,strlen(colour_codes[1].esc_code));
											buffpos+=strlen(colour_codes[1].esc_code)-1;
											}
										else buffpos--;
										break;
									}
								}
							else buffpos--;
							++str;
							--cnt;
							goto CONT;
							}
						}
					*(buff+buffpos)=*(--str);
					}
				else *(buff+buffpos)=*str;
CONT:
				++buffpos;
				++str;
				++cnt;
				}
			if (buffpos==OUT_BUFF_SIZE) {
				write(sock,buff,OUT_BUFF_SIZE);
				buffpos=0;
				}
			}
		len=strlen(text);
		num_chars+=len;
		lines+=len/80+(len<80);
		fgets(text,sizeof(text)-1,fp);
		}
	if (buffpos && sock!=-1) write(sock,buff,buffpos);

/* if user is logging on dont page file */
	if (user==NULL) {
		fclose(fp);
		return 2;
		write_sock(sock,"\n");
		};
	if (feof(fp)) {
		user->filepos=0;
		no_prompt=0;
		retval=2;
		for (i=0;i<MAX_PAGES;i++) user->pages[i]=0;
		user->pagecnt=0;
		write_user(user,"\n");
		}
	else  {
  /* store file position and file name */
		user->filepos+=num_chars;
		user->pages[++user->pagecnt]=user->filepos;
		strcpy(user->page_file,filename);
  /* work out % of file displayed */
		fperc=(int)(user->filepos*100)/fsize;
  /* We use E here instead of Q because when on a remote system and
     in COMMAND mode the Q will be intercepted by the home system and 
     quit the user */
		vwrite_user(user, more_prompt, fperc);
		no_prompt=1;
		}
	fclose(fp);
	return retval;
}


/*** Page out a list of users of the level given.  If lev is -1 then page out
     all of the users
     ***/
int more_users(UR_OBJECT user)
{
struct user_dir_struct *entry;
int i,lines,retval;

	set_crash();
retval=1;  entry=NULL;
/* page all of the users */
if (first_dir_entry==NULL) return 0;
if (user->user_page_lev==-1) {
  /* get to where user is so far */
  if (!user->user_page_pos) entry=first_dir_entry;
  else for (i=0;i<user->user_page_pos;i++) entry=entry->next;
  lines=0;
  while (entry!=NULL) {
    if (lines>=user->terminal.pager) {
      write_user(user, continue2);
      no_prompt=1;
      user->misc_op=16;
      user->status='R';
      return retval;
      }
    vwrite_user(user,"%d) %-*s : %s\n",entry->level,USER_NAME_LEN,entry->name,entry->date);
    entry=entry->next;
    user->user_page_pos++;
    lines++;
    } /* end while */
  retval=2;
  user->user_page_pos=0;  user->user_page_lev=0;  user->misc_op=0;
  user->status='a';
  return retval;
  } /* end if for listing all users */

/* list only those users of a specific level */
if (!amsys->level_count[user->user_page_lev]) return 0;
if (!user->user_page_pos) entry=first_dir_entry;
else {
  /* get to the position of the user level */
  entry=first_dir_entry;
  i=0;
  while (entry!=NULL) {
    if (entry->level!=user->user_page_lev) {
      entry=entry->next;
      continue;
      }
    if (i==user->user_page_pos) break;
    ++i;
    entry=entry->next;
    }
  }
lines=0;
while (entry!=NULL) {
  if (entry->level!=user->user_page_lev) {
    entry=entry->next;
    continue;
    }
  if (lines>=user->terminal.pager) {
    write_user(user, continue2);
    no_prompt=1;
    return retval;
    }
  /* doesn't matter if didn't boot in verbose mode as entry->date will be null anyway */
  vwrite_user(user,"%-*s : %s\n",USER_NAME_LEN,entry->name,entry->date);
  entry=entry->next;
  user->user_page_pos++;
  lines++;
  } /* end while */
write_user(user,"\n");
retval=2;
user->user_page_pos=0;  user->user_page_lev=0;  user->misc_op=0;
user->status='a';
return retval;
}


/* adds a string to the user's history list */
void add_history(char *name, int showtime, char *str)
{
FILE *fp;
char filename[500];

	set_crash();
name[0]=toupper(name[0]);
/* add to actual history listing */
sprintf(filename,"%s/%s.H", USERHISTORYS,name);
if ((fp=fopen(filename,"a"))) {
  if (!showtime) fprintf(fp,"%s",str);
  else fprintf(fp,"%02d/%02d %02d:%02d: %s",tmday,tmonth+1,thour,tmin,str);
  fclose(fp);
  write_syslog(SYSLOG,1,"HISTORY added for %s.\n",name);
  }
}



/******************************************************************************
 Logon/off functions
 *****************************************************************************/

/*** Login function. Lots of nice inline code :) ***/
void login(UR_OBJECT user, char *inpstr)
{
UR_OBJECT u;
int i;
char name[ARR_SIZE],passwd[ARR_SIZE],motdname[500];

	set_crash();
	name[0]='\0';
	passwd[0]='\0';

	switch(user->login) {
		case LOGIN_NAME:
			if (syspp->pueblo_enh && user->pueblo!=-1) {
				if (!strncmp(inpstr,"PUEBLOCLIENT",12)) {
					user->pueblo = -1; /* Set to -1 so we don't keep repeating the welcome screen */
					cls(user);
					vwrite_user(user, "</xch_mudtext><center><img src=\"%s%s%s\"></center><xch_mudtext>",reg_sysinfo[TALKERHTTP],reg_sysinfo[PUEBLOWEB],reg_sysinfo[PUEBLOPIC]);
					vwrite_user(user, "</xch_mudtext><br><br><center><b><font size=+3>Welcome to %s v%s !</font></b><br><font size=+1>Lotos verzia %s</font></center><br><xch_mudtext>\n",reg_sysinfo[TALKERNAME], TVERSION, OSSVERSION);
					sprintf(text,"%s<>~RS %sMultimedia client has been Auto-detected.\n",colors[CWARNING],colors[CSYSBOLD]);
					write_duty(ARCH,text,NULL,NULL,2);
					user->login=LOGIN_NAME;
					name[0]='\0';
					write_user(user, login_prompt);
					return; 
					}
 				}
			sscanf(inpstr,"%s",name);
			if(name[0]<33) {
				write_user(user, login_prompt);
				return;
				}
			if (!strcmp(name,"quit")) {
				write_user(user, login_quit);
				disconnect_user(user);
				return;
				}
			if (!strcmp(name,"who")) {
      /* if you don't like this logon who, then replace it with the normal
	 one of who(user,0); */
				login_who(user);
				write_user(user, login_prompt);
				return;
				}
			if (!strcasecmp(name,"version")) {
				vwrite_user(user,"\n%s v%s (Lotos v%s (AmNUTS v%s (NUTS v%s)))\n",
					reg_sysinfo[TALKERNAME], TVERSION, OSSVERSION, AMNUTSVER, NUTSVER);
				write_user(user, login_prompt);
				return;
				}
			if (strlen(name)<USER_MIN_LEN) {
				write_user(user, login_shortname);  
				attempts(user);
				return;
				}
			if (strlen(name)>USER_NAME_LEN) {
				write_user(user, login_longname);
				attempts(user);
				return;
				}
    /* see if only letters in login */
    for (i=0;i<strlen(name);++i) {
      if (!isalpha(name[i])) {
	write_user(user, login_lettersonly);
	attempts(user);  return;
        }
      }
    if (contains_swearing(name)) {
      write_user(user, login_swname);
      attempts(user);  return;
      }
    if (!amsys->allow_recaps) strtolower(name);
    name[0]=toupper(name[0]);
    if (user_banned(name) && strcmp(name, reg_sysinfo[SYSOPUNAME])) {
      write_user(user, user_banned_prompt);
      sprintf(text, "%s<>~RS %sPokus o login zabanovaneho usera!~RS%s [%s] (%s:%d)\n",
		colors[CWARNING], colors[CSYSBOLD], colors[CSYSTEM], name, user->site, user->site_port);
	write_duty(ARCH, text, NULL, NULL, 2);
      disconnect_user(user);
      write_syslog(SYSLOG,1,"Pokus o login zabanovaneho usera %s.\n",name);
      return;
      }
    strcpy(user->name,name);
    strtolower(name);
    if (!strcmp(name, "puebloclient")) {
    	write_user(user, login_pbloname);
    	return;
    	}
    /* If user has hung on another login clear that session */
    for(u=user_first;u!=NULL;u=u->next) {
      if (u->login && u!=user && !strcmp(u->name,user->name)) {
	disconnect_user(u);  break;
        }
      }
    if (!load_user_details(user)) {
      /* test na ROOTovsky login */
      if (!strcmp(user->name, reg_sysinfo[SYSOPUNAME])) {
      	user->level=ROOT;
      	strcpy(user->pass, reg_sysinfo[SYSOPPASSWD]);
      	}
      /* we presume that port 1 is the wizport */
      if (user->port==port[1]) {
	write_user(user, login_nonewatwiz);
	disconnect_user(user);  
	return;
        }
      if (amsys->minlogin_level>-1) {
	write_user(user, login_nonewacc);
	disconnect_user(user);  
	return;
        }
      if (site_banned(user->site,1) && user->level<ROOT) {
	write_user(user, login_nonewatbanned);
	write_syslog(SYSLOG,1,"Attempted login by a new user from banned new users site %s.\n",user->site);
	disconnect_user(user);
	return;
        }
      if (user->level<ROOT) {
        vwrite_user(user, login_welcome, reg_sysinfo[TALKERNAME]);
        write_user(user, login_new_user);
        }
      }
    else {
      if (!strcmp(user->name, reg_sysinfo[SYSOPUNAME])) user->level=ROOT;
      if (user->port==port[1] && user->level<amsys->wizport_level) {
	vwrite_user(user, login_minwizlev,user_level[amsys->wizport_level].name);
	disconnect_user(user);  
	return;
        }
      if (user->level<amsys->minlogin_level) {
	write_user(user, login_minlev);
	disconnect_user(user);  
	return;
        }
      }
	if (user->terminal.xterm && !user->attempts)
		vwrite_user(user, "\033]0;%s\007", reg_sysinfo[TALKERNAME]);
    write_user(user, password_prompt);
    echo_off(user);
    user->login=LOGIN_PASSWD;
    return;

  case LOGIN_PASSWD:
    sscanf(inpstr,"%s",passwd);
    if (strlen(passwd)<PASS_MIN_LEN) {
      write_user(user, password_short);  
      attempts(user);
      return;
      }
    if (strlen(passwd)>PASS_LEN) {
      write_user(user, password_long);
      attempts(user);  return;
      }
    /* if new user... */
    if (!user->pass[0]) {
      strcpy(user->pass,(char *)crypt(passwd,crypt_salt));
      write_user(user,"\n");
      if (more(NULL,user->socket,RULESFILE)) {
	write_user(user, login_rules_prompt);
        }
      write_user(user, password_again);
      user->login=LOGIN_CONFIRM;
      }
    else {
      if (!strcmp(user->pass,(char *)crypt(passwd,crypt_salt))) {
	echo_on(user);
	amsys->logons_old++;
	/* Instead of connecting the user with:  'connect_user(user);  return;'
	   Show the user the MOTD2 so that they can read it.  If you wanted, you
	   could make showing the MOTD2 optional, in which case use an 'if' clause
	   to either do the above or the following...
	   */
	cls(user);
	/* If there is no motd2 files then don't display them */
	if (amsys->motd2_cnt) {
	  sprintf(motdname,"%s/motd2/motd%d", MOTDFILES,(get_motd_num(2)));
	  more(user,user->socket,motdname);
	  }
	vwrite_user(user, "%s~RS pouziva ~FT~OLLotos~FW v~FR%s~RS - S/N %s (%s)\n\n",
		reg_sysinfo[TALKERNAME], OSSVERSION, reg_sysinfo[SERIALNUM], reg_sysinfo[REGUSER]);
	write_user(user, enterprompt);
	user->login=LOGIN_PROMPT;
	return;
        }
      write_user(user, password_wrong);
      attempts(user);
      }
    return;

  case LOGIN_CONFIRM:
    sscanf(inpstr,"%s",passwd);
    if (strcmp(user->pass,(char*)crypt(passwd,crypt_salt))) {
      write_user(user, password_nomatch);
      attempts(user);
      return;
      }
    echo_on(user);
	nick_grm(user);
    strcpy(user->desc, default_desc);
    strcpy(user->in_phrase, default_inphr);
    strcpy(user->out_phrase, default_outphr);
    strcpy(user->date,(long_date(1)));
    strcpy(user->recap,user->name);
    strcpy(user->bw_recap,colour_com_strip(user->recap));
    user->last_site[0]='\0';
    user->level=NEW;
    user->unarrest=NEW;
    user->muzzled=0;
    user->command_mode=0;
    user->prompt=amsys->prompt_def;
    user->terminal.txt=amsys->colour_def;
    user->terminal.checho=amsys->charecho_def;
    save_user_details(user,1);
    add_user_node(user->name,user->level);
    add_user_date_node(user->name,(long_date(1)));
    add_history(user->name,1,"Was initially created.\n");
    user->pueblo_mm=syspp->pblo_usr_mm_def;
    user->pueblo_pg=syspp->pblo_usr_pg_def;
    strcpy(user->restrict, RESTRICT_MASK);
    write_syslog(SYSLOG,1,"Novy juzer \"%s\" vytvoreny.\n",user->name);
    amsys->logons_new++;
    amsys->level_count[user->level]++;
    /* Check out above for explaination of this */
    cls(user);
    /* If there is no motd2 files then don't display them */
    if (amsys->motd2_cnt) {
      sprintf(motdname,"%s/motd2/motd%d", MOTDFILES, (get_motd_num(2)));
      more(user,user->socket,motdname);
      } 
    write_user(user, enterprompt);
    user->login=LOGIN_PROMPT;
    return;

  case LOGIN_PROMPT:
    if(user->level==ROOT) user->accreq=-1;
    user->login=0;
    write_user(user,"\n\n");
    connect_user(user);
    return;
  } /* end switch */
}
	


/*** Count up attempts made by user to login ***/
void attempts(UR_OBJECT user)
{
	set_crash();
	user->attempts++;
	if (user->attempts==LOGIN_ATTEMPTS) {
		write_user(user, login_attempts);
		disconnect_user(user);
		if (load_user_details(user)) {
			sprintf(text, "~FR~OLNiekto sa pokusal dostat na tvoje konto !\nAdresa: %s - %s\n",
				 user->site, user->ipsite);
			send_mail(user, user->name, text, 0);
			}
		return;
		}
	reset_user(user);
	user->login=LOGIN_NAME;
	user->pass[0]='\0';
	write_user(user, login_prompt);
	echo_on(user);
}



/*** Display better stats when logging in.  I personally use this rather than the MOTD2
     but you can use it where you want.  Gives better output than that "you last logged in"
     line tht was in connect_user
***/
void show_login_info(UR_OBJECT user)
{
char temp[ARR_SIZE],text2[ARR_SIZE],*see[]={"~OL~FYneviditeln","~OL~FTviditeln"},*myoffon[]={"~OL~FTvyp","~OL~FRzap"};
char *times[]={"rano","den","vecer"};
int yes,cnt,phase,exline;

	set_crash();
cnt=yes=exline=0;
write_user(user,"\n+----------------------------------------------------------------------------+\n");
if (thour>=0 && thour<10) phase=0;
else if (thour>=11 && thour<18) phase=1;
else phase=2;
sprintf(text,"Dobr%s %s, %s~RS, a witay v ~FT~OL%s~RS",
	grm_gnd(1, (phase==0)?0:1), times[phase], user->recap, reg_sysinfo[TALKERNAME]);
cnt=colour_com_count(text);
vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
sprintf(text,"prixadzas k nam ~OL%s~RS",long_date(1));
cnt=colour_com_count(text);
vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
write_user(user,"+----------------------------------------------------------------------------+\n");
if (user->last_site[0]) {
  sprintf(temp,"%s",ctime(&user->last_login));
  temp[strlen(temp)-1]='\0';
  sprintf(text,"posledny login : ~OL%-25s~RS   tvoj level : ~OL%s~RS",temp,user_level[user->level].name);
  cnt=colour_com_count(text);
  vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
  sprintf(text,"posledna sajta : ~OL%s~RS",user->last_site);
  cnt=colour_com_count(text);
  vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
  exline++;
  }
if (user->level>=command_table[INVIS].level) {
  sprintf(text,"teraz si %s%s~RS",see[user->vis], grm_gnd(1, user->gender));
  if (user->level>=command_table[MONITOR].level) {
    sprintf(text2," a tvoj monitor je %s~RS",myoffon[user->monitor]);
    strcat(text,text2);
    }
  else strcat(text,"\n");
  cnt=colour_com_count(text);
  vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
  exline++;
  }
else if (user->level>=command_table[MONITOR].level) {
  sprintf(text,"tvoj monitor je aktualne %s~RS",myoffon[user->monitor]);
  cnt=colour_com_count(text);
  vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
  exline++;
  }
yes=0;  text2[0]='\0';  text[0]='\0';  temp[0]='\0';
if (user->ignore.all) { strcat(text2,"~FR~OLVSETKO !");  yes=1; }
if (!yes) {
  if (user->ignore.tells) { strcat(text2,"telly   ");  yes=1; }
  if (user->ignore.shouts) { strcat(text2,"shouty   ");  yes=1; }
  if (user->ignore.pics) { strcat(text2,"obr   ");  yes=1; }
  if (user->ignore.logons) { strcat(text2,"loginy   ");  yes=1; }
  if (user->ignore.wiz) { strcat(text2,"wiztelly   ");  yes=1; }
  if (user->ignore.greets) { strcat(text2,"Greets   ");  yes=1; }
  if (user->ignore.beeps) { strcat(text2,"Beeps   ");  yes=1; }
  if (user->ignore.transp) { strcat(text2,"transporty");  yes=1; }
  }
if (yes) {
  sprintf(text,"ignorujes: ~OL%s~RS",text2);
  cnt=colour_com_count(text);
  vwrite_user(user,"| %-*.*s |\n",74+cnt*3,74+cnt*3,text);
  exline++;
  }
if (exline) write_user(user,"+----------------------------------------------------------------------------+\n");
if (user->level>=command_table[REMINDER].level) {
  cnt=has_reminder_today(user);
  sprintf(text,"Mas ~OL%d~RS pripomien%s na dnes", cnt, grm_num(2, cnt));
  vwrite_user(user,"| %-80.80s |\n",text);
  cnt=remove_old_reminders(user);
  if (cnt) {
    sprintf(text, "~OL%d~RS pripomien%s zrusen%s kvoli staremu datumu", cnt, grm_num(2, cnt), grm_num(3, cnt));
    vwrite_user(user,"| %-80.80s |\n",text);
    }
  write_user(user,"+----------------------------------------------------------------------------+\n");
  }
}



/*** Connect the user to the talker proper ***/
void connect_user(UR_OBJECT user)
{
UR_OBJECT u,u2;
RM_OBJECT rm;
char rmname[ROOM_NAME_LEN+20],*bp,null[1];
int cnt,newmail;

	set_crash();
null[0]='\0';
if (user->ignore.beeps) bp=NULL;  else bp="\007";

/* See if user already connected */
for(u=user_first;u!=NULL;u=u->next) {
  if (user!=u && user->type!=CLONE_TYPE && !strcmp(user->name,u->name)) {
    rm=u->room;
#ifdef NETLINKS
    /* deal with a login if the user is connected to a remote talker */
    if (u->type==REMOTE_TYPE) {
      write_user(u,"\n~FB~OLYou are pulled back through cyberspace...\n");
      sprintf(text,"REMVD %s\n",u->name);
      write_sock(u->netlink->socket,text);
      vwrite_room(rm,"%s mizne.\n",u->name);
      syspp->acounter[u->gender]--;
      syspp->acounter[3]--;
      destruct_user(u);
      reset_access(rm);
      break;
      }
#endif
    if (user->type==BOT_TYPE) close(user->socket); /* Don't want ppl logging in as the bot. */
    vwrite_user(user,"\n\nUz si pripojen%s - prepinam do starej session...\n",
 		grm_gnd(1, user->gender));
	vwrite_user(user, "Stara adresa bola %s\n", u->site);
    write_syslog(SYSLOG,1,"%s~RS  %s (%s)\n",session_swap, user->name,user->site);
    vwrite_user(u, "\n~OL~FRSwapujem tuto session na adresu ~FW%s\n\n", user->site);
    close(u->socket);
    u->socket=user->socket;
    strcpy(u->site,user->site);
    u->site_port=user->site_port;
    destruct_user(user);
    amsys->num_of_logins--;

	/* Reset pueblo status */
	if (u->pueblo!=-1) {     /* if -1 then already been detected. */
		u->pueblo=0;     /* Default to pueblo-incompatible */
		u->pblodetect=1; /* Enable pueblo to be re-detected */
		}
    if (rm==NULL) {
#ifdef NETLINKS
      sprintf(text,"ACT %s look\n",u->name);
      write_sock(u->netlink->socket,text);
#endif
      }
    else {
      look(u);
      prompt(u);
      }
    /* Reset the sockets on any clones */
    for(u2=user_first;u2!=NULL;u2=u2->next) {
      if (u2->type==CLONE_TYPE && u2->owner==user) {
	u2->socket=u->socket;  u->owner=u;
        }
      }
    return;
    }
  }
alert_friends(user, 1);
logon_flag=1;
if (!user->vis && user->level<command_table[INVIS].level) user->vis=1;
if (user->level==JAILED) {
  user->room=get_room(default_jail);
  if (user->room==NULL) user->room=room_first;
  }
else check_start_room(user);
if (user->room==room_first) rmname[0]='\0';
else if (user->room->access==PERSONAL_LOCKED || user->room->access==PERSONAL_UNLOCKED) sprintf(rmname,"~RS %s~OL",user->room->name);
else sprintf(rmname,"~RS (%s)~OL",user->room->name);
if (user->level==JAILED) {
  vwrite_room_except(NULL,user,"~OL[Being thrown into jail is:~RS %s~RS %s~RS~OL]\n",user->recap,user->desc);
  sprintf(text,"~FT%s (Site %s : Site Port : %d : Talker Port %d)\n",user->name,user->site,user->site_port,user->port);
  write_level(ARCH,1,text,user);
  }
else {
  if (user->vis) {
    if (user->level<WIZ) vwrite_room_except(NULL, user, "~OL[Entering%s is:~RS %s~RS %s~RS~OL]\n",rmname,user->recap,user->desc);
    else vwrite_room_except(NULL, user, "\07~OL[Entering%s is:~RS %s~RS %s~RS~OL]\n",rmname,user->recap,user->desc);
    sprintf(text,"~FT%s (Site %s : Site Port %d : Talker Port %d)\n",user->name,user->site,user->site_port,user->port);
    write_level(ARCH,1,text,user);
    }
  else {
    if (user->level<ARCH) write_room_except(user->room,invisenter,user);
    sprintf(text,"~OL~FY[ INVIS ]~RS ~OL[Entering%s is:~RS %s~RS %s~RS~OL]\n",rmname,user->recap,user->desc);
    write_level(WIZ,1,text,user);
    sprintf(text,"~OL~FY[ INVIS ]~RS ~FT%s (Site %s : Site Port %d : Talker Port : %d)\n",user->name,user->site,user->site_port,user->port);
    write_level(ARCH,1,text,user); 
    }
  }
logon_flag=0;
user->logons++;
if (user->level==NEW) user->t_expire=time(0)+(NEWBIE_EXPIRES*86400);
else user->t_expire=time(0)+(USER_EXPIRES*86400);
show_login_info(user);
user->last_login=time(0); /* set to now */
if (user->pueblo==-1) user->pueblo=1;
load_plugin_data(user);
look(user);
audioprompt(user, 0, 0); /* Audio greeting */
/* show how much mail the user has */
newmail=mail_sizes(user->name,1);
if (newmail)
	vwrite_user(user,"%s~FT~OL*** Mas ~RS~OL%d~FT neprecitan%s MAIL sprav%s ***\n",
		bp, newmail, grm_num(3, newmail), grm_num(1, newmail));
else if ((cnt=mail_sizes(user->name,0))) 
  vwrite_user(user,"~FT*** Mas ~RS~OL%d~RS~FT sprav%s v mail schranke ***\n",cnt, grm_num(1, cnt));
/* should they get the autopromote message? */
if (user->accreq!=-1 && amsys->auto_promote) {
  vwrite_user(user, autopromo_prompt, bp);
  }

prompt(user);
record_last_login(user->name);
write_syslog(SYSLOG,1,"%s logged in on port %d from %s:%d.\n",user->name,user->port,user->site,user->site_port);
syspp->acounter[3]++;
amsys->num_of_logins--;
syspp->tcounter[user->gender]++;
syspp->bcounter[user->gender]++;
user->tcount=++syspp->tcounter[3];
user->bcount=++syspp->bcounter[3];
syspp->acounter[user->gender]++;
if (syspp->acounter[user->gender]>syspp->mcounter[user->gender]) syspp->mcounter[user->gender]++;
if (syspp->acounter[3]>syspp->mcounter[3]) syspp->mcounter[3]++;
save_counters();
}


/******************************************************************************
 Misc and line editor functions
 *****************************************************************************/

/*** Stuff that is neither speech nor a command is dealt with here ***/
int misc_ops(UR_OBJECT user, char *inpstr)
{
char filename[200];
int i=0;

	set_crash();
switch(user->misc_op) {
  case 1: 
    if (toupper(inpstr[0])=='Y') {
      if (amsys->rs_countdown && !amsys->rs_which) {
	if (amsys->rs_countdown>60) 
	  vwrite_room(NULL,"\n\07~OLSYSTEM: ~FR~LISHUTDOWN INITIATED, vypinam za %d minut%s, %d sekund%s!\n\n",
		  amsys->rs_countdown/60,grm_num(1, amsys->rs_countdown/60),amsys->rs_countdown%60, grm_num(1, amsys->rs_countdown%60));
	else vwrite_room(NULL,"\n\07~OLSYSTEM: ~FR~LISHUTDOWN INITIATED, vypinam za %d sekund%s!\n\n",amsys->rs_countdown,grm_num(1, amsys->rs_countdown));
	audioprompt(NULL, 6, 0);
	write_syslog(SYSLOG,1,"%s initiated a %d second%s SHUTDOWN countdown.\n",user->name,amsys->rs_countdown,PLTEXT_S(amsys->rs_countdown));
	amsys->rs_user=user;
	amsys->rs_announce=time(0);
	user->misc_op=0;  
	prompt(user);
	return 1;
        }
      talker_shutdown(user,NULL,0); 
      }
    /* This will reset any reboot countdown that was started, oh well */
    amsys->rs_countdown=0;
    amsys->rs_announce=0;
    amsys->rs_which=-1;
    amsys->rs_user=NULL;
    user->misc_op=0;  
    prompt(user);
    return 1;
    
  case 2:
    if (toupper(inpstr[0])=='E') {
      user->misc_op=0;  user->filepos=0;  user->page_file[0]='\0';
      user->status='a';
      for (i=0;i<MAX_PAGES;i++) user->pages[i]=0;
      user->pagecnt=0;
      prompt(user);
      return 1;
      }
    else if (toupper(inpstr[0])=='R') {
      if (!user->pagecnt) user->pagecnt=0;
      else user->pagecnt--;
      user->filepos=user->pages[user->pagecnt];
      }
    else if (toupper(inpstr[0])=='B') {
      if (user->pagecnt<2) user->pagecnt=0;
      else user->pagecnt=user->pagecnt-2;
      user->filepos=user->pages[user->pagecnt];
      }
    if (more(user,user->socket,user->page_file)!=1) {
      user->misc_op=0;
      user->status='a';
      user->filepos=0;
      user->page_file[0]='\0';
      for (i=0;i<MAX_PAGES;i++) user->pages[i]=0;
      user->pagecnt=0;
      prompt(user);
      }
    return 1;

  case 3: /* writing on board */
  case 4: /* Writing mail */
  case 5: /* doing profile */
    editor(user,inpstr);
    return 1;

  case 6:
    if (toupper(inpstr[0])=='Y') delete_user(user,1); 
    else {  user->misc_op=0;  prompt(user);  }
    return 1;
    
  case 7:
    if (toupper(inpstr[0])=='Y') {
      if (amsys->rs_countdown && amsys->rs_which==1) {
	if (amsys->rs_countdown>60) 
	  vwrite_room(NULL,"\n\07~OLSYSTEM: ~FY~LIREBOOT INITIATED, restartujem za %d minut%s, %d sekund%s!\n\n",
		  amsys->rs_countdown/60, grm_num(1, amsys->rs_countdown/60), amsys->rs_countdown%60, grm_num(1, amsys->rs_countdown%60));
	else vwrite_room(NULL,"\n\07~OLSYSTEM: ~FY~LIREBOOT INITIATED, restartujem za %d sekund%s!\n\n",
		amsys->rs_countdown, grm_num(1, amsys->rs_countdown));
	audioprompt(NULL, 6, 0);
	write_syslog(SYSLOG,1,"%s initiated a %d second%s REBOOT countdown.\n",user->name,amsys->rs_countdown,PLTEXT_S(amsys->rs_countdown));
	amsys->rs_user=user;
	amsys->rs_announce=time(0);
	user->misc_op=0;  
	prompt(user);
	return 1;
        }
      talker_shutdown(user,NULL,1); 
      }
    if (amsys->rs_which==1 && amsys->rs_countdown && amsys->rs_user==NULL) {
      amsys->rs_countdown=0;
      amsys->rs_announce=0;
      amsys->rs_which=-1;
      }
    user->misc_op=0;  
    prompt(user);
    return 1;

  case 8: /* Doing suggestion */
  case 9: /* Level specific mail */
    editor(user,inpstr);
    return 1;

  case 10:
    if (toupper(inpstr[0])=='E') {
      user->misc_op=0;  user->wrap_room=NULL;  prompt(user);
      user->status='a';
      }
    else rooms(user,0,1);
    return 1;
    
  case 11:
    if (toupper(inpstr[0])=='E') {
      user->misc_op=0;  user->wrap_room=NULL;  prompt(user);
      user->status='a';
      }
    else rooms(user,1,1);
    return 1;
    
  case 12:
    if (!inpstr[0]) {
      write_user(user,"Abandoning your samesite look-up.\n");
      user->misc_op=0;  user->samesite_all_store=0;  user->samesite_check_store[0]='\0';
      user->status='a';
      prompt(user);
      }
    else {
      user->misc_op=0;
      word[0][0]=toupper(word[0][0]);
      strcpy(user->samesite_check_store,word[0]);
      samesite(user,1);
      }
    return 1;
    
  case 13:
    if (!inpstr[0]) {
      write_user(user,"Abandoning your samesite look-up.\n");
      user->misc_op=0;  user->samesite_all_store=0;  user->samesite_check_store[0]='\0';
      user->status='a';
      prompt(user);
      }
    else {
      user->misc_op=0;
      strcpy(user->samesite_check_store,word[0]);
      samesite(user,2);
      }
    return 1;
    
  case 14:
    if (toupper(inpstr[0])=='E') {
      user->misc_op=0;
      user->hwrap_lev=0;
      user->hwrap_id=0;
      user->hwrap_same=0;
      user->tmp_int=0;
      user->status='a';
      prompt(user);
      }
    else help_commands_level(user,1+user->hwrap_pl);
    return 1;
    
  case 15:
    if (toupper(inpstr[0])=='E') {
      user->misc_op=0;  user->hwrap_lev=0;  user->hwrap_func=0;  user->hwrap_id=0;  user->hwrap_same=0;
      user->status='a';
      prompt(user);
      }
    else help_commands_function(user,1);
    return 1;

  case 16:
    if (toupper(inpstr[0])=='E' || more_users(user)!=1) {
      user->user_page_pos=0;  user->user_page_lev=0;  user->misc_op=0;
      user->status='a';
      prompt(user); 
      }
    return 1;

  case 17:
    if (toupper(inpstr[0])=='Y') {
      recount_users(user,1);
      }
    else { user->misc_op=0; prompt(user); }
    return 1;

  case 18:
    if (toupper(inpstr[0])=='Y') {
      sprintf(filename,"%s/%s.M", USERMAILS, user->name);
      unlink(filename);
      write_user(user,"\n~OL~FRVsetky spravy zmazane\n\n");
      }
    else write_user(user,"\nNo mail messages were deleted.\n\n");
    user->misc_op=0;
    return 1;

  case 19: /* decorating room */
    editor(user,inpstr);
    return 1;

  case 20:
    user->temp_remind.day=atoi(inpstr);
    show_reminders(user,1);
    return 1;

  case 21:
    user->temp_remind.month=atoi(inpstr);
    show_reminders(user,2);
    return 1;

  case 22:
    if (!inpstr[0]) user->temp_remind.year=tyear;
    else user->temp_remind.year=atoi(inpstr);
    show_reminders(user,3);
    return 1;

  case 23:
    strncpy(user->temp_remind.msg,inpstr,REMINDER_LEN);
    show_reminders(user,4);
    return 1;

  case 24: /* friends mail */
    editor(user,inpstr);
    return 1;

  case 101: /* restart - reinit */
    if (toupper(inpstr[0])=='Y') {
    user->misc_op=0;
    prompt(user);
      restart(user);
      }
    else {
    	user->misc_op=0;
	prompt(user);
    	}
    return 1;

	case 102: /* otazka na pouzitie set menu */
		if (toupper(inpstr[0])=='A') {
			user->misc_op=0;
			prompt(user);
			user->set_op=1;
			print_menu(user);
			no_prompt=1;
			user->ignore.all_store=user->ignore.all;
			user->ignore.all=1;
			user->status='S';
			vwrite_room_except(user->room, user, room_setup_enter, user->recap);
			}
		else {
			print_menu(user);
			user->misc_op=0;
			user->set_op=0;
			user->set_mode=SET_NONE;
			prompt(user);
			}
		return 1;
	case 103: /* help_commands_only */
		if (toupper(inpstr[0])=='E') {
			user->misc_op=0;
			user->hwrap_lev=0;
			user->hwrap_id=0;
			user->hwrap_same=0;
			user->status='a';
			prompt(user);
			}
		else help_commands_only(user, 1+user->hwrap_pl);
		return 1;
  } /* end switch */
return 0;
}


/*** The editor used for writing profiles, mail and messages on the boards ***/
void editor(UR_OBJECT user, char *inpstr)
{
int cnt,line;
char *ptr,*name;

	set_crash();
if (user->edit_op) {
  switch(toupper(*inpstr)) {
    case 'U':
      /* if (*user->malloc_end--!='\n') *user->malloc_end--='\n'; */
      if (user->vis) name=user->recap;  else  name=invisname;
      if (!user->vis) write_monitor(user,user->room,0);
      vwrite_room_except(user->room,user,"%s~RS dokoncil%s pisanie textu.\n",name, grm_gnd(4, user->gender));
      switch(user->misc_op) {
        case 3:  write_board(user,NULL,1);  break;
        case 4:  smail(user,NULL,1);  break;
        case 5:  enter_profile(user,1);  break;
        case 8:  suggestions(user,1);  break;
        case 9:  level_mail(user,inpstr,1);  break;
        case 19: personal_room_decorate(user,1);  break;
        case 24: friend_smail(user,NULL,1);  break;
        }
      editor_done(user);
      return;

    case 'P':
      user->edit_op=0;
      user->edit_line=1;
      user->charcnt=0;
      user->malloc_end=user->malloc_start;
      *user->malloc_start='\0';
      write_user(user,"\nZnovupisanie textu ...\n\n");
      vwrite_user(user, edit_markers, user->edit_line);
      return;
      
    case 'Z':
      write_user(user,"\nSprava zrusena.\n");
      if (user->vis) name=user->recap;
      else name=invisname;
      if (!user->vis)
      	write_monitor(user,user->room,0);
      vwrite_room_except(user->room,user,"%s~RS zrusil%s pisanie textu.\n", name, grm_gnd(4, user->gender));
      editor_done(user);  
      return;

    case 'K':
      vwrite_user(user,"\nNapisal%s si nasledujuci text ...\n\n", grm_gnd(4, user->gender));
      write_user(user,user->malloc_start);
      write_user(user, edit_prompt);
      return;
      
    default: 
      write_user(user, edit_prompt);
      return;
    } /* end switch */
  }
/* Allocate memory if user has just started editing */
if (user->malloc_start==NULL) {
  if ((user->malloc_start=(char *)malloc(MAX_LINES*81))==NULL) {
    vwrite_user(user,"%s: failed to allocate buffer memory.\n",syserror);
    write_syslog(ERRLOG,1,"Failed to allocate memory in editor().\n");
    user->misc_op=0;
    user->status='a';
    prompt(user);
    return;
    }
  clear_edit(user);
  user->ignore.all_store=user->ignore.all;
  user->ignore.all=1; /* Dont want chat mucking up the edit screen */
  user->status='E';
  user->edit_line=1;
  user->charcnt=0;
  user->editing=1;
  user->malloc_end=user->malloc_start;
  *user->malloc_start='\0';
  vwrite_user(user, "~FTMaximalne %d riad%s, koniec s '.' na samostatnom riadku.\n\n",
		MAX_LINES, grm_num(4, MAX_LINES));
  vwrite_user(user, edit_markers, 1);
  if (user->vis) name=user->recap;  else  name=invisname;
  if (!user->vis) write_monitor(user,user->room,0);
  vwrite_room_except(user->room,user,"%s~RS zacal%s pisat nejaky text ...\n",name, grm_gnd(4, user->gender));
  return;
  }
/* Check for empty line */
if (!word_count) {
  if (!user->charcnt) {
    sprintf(text,"~FG%d>~RS",user->edit_line);
    write_user(user,text);
    return;
    }
  *user->malloc_end++='\n';
  if (user->edit_line==MAX_LINES) goto END;
  vwrite_user(user,"~FG%d>~RS",++user->edit_line);
  user->charcnt=0;
  return;
  }
/* If nothing carried over and a dot is entered then end */
if (!user->charcnt && !strcmp(inpstr,".")) goto END;
line=user->edit_line;
cnt=user->charcnt;
/* loop through input and store in allocated memory */
while(*inpstr) {
  *user->malloc_end++=*inpstr++;
  if (++cnt==80) {  user->edit_line++;  cnt=0;  }
  if (user->edit_line>MAX_LINES 
      || user->malloc_end - user->malloc_start>=MAX_LINES*81) goto END;
  }
if (line!=user->edit_line) {
  ptr=(char *)(user->malloc_end-cnt);
  *user->malloc_end='\0';
  vwrite_user(user,"~FG%d>~RS%s",user->edit_line,ptr);
  user->charcnt=cnt;
  return;
  }
else {
  *user->malloc_end++='\n';
  user->charcnt=0;
  }
if (user->edit_line!=MAX_LINES) {
  vwrite_user(user,"~FG%d>~RS",++user->edit_line);
  return;
  }

/* User has finished his message , prompt for what to do now */
END:
*user->malloc_end='\0';
if (*user->malloc_start) {
  write_user(user, edit_prompt);
  user->edit_op=1;  return;
  }
write_user(user,"\nZiadny text.\n");
if (user->vis) name=user->recap;  else  name=invisname;
if (!user->vis) write_monitor(user,user->room,0);
vwrite_room_except(user->room,user,"%s~RS zrusil%s pisanie textu.\n",name, grm_gnd(4, user->gender));
editor_done(user);
}


/*** Reset some values at the end of editing ***/
void editor_done(UR_OBJECT user)
{
	set_crash();
user->misc_op=0;
user->edit_op=0;
user->edit_line=0;
free(user->malloc_start);
user->malloc_start=NULL;
user->malloc_end=NULL;
user->ignore.all=user->ignore.all_store;
user->editing=0;
user->status='a';
if (user->editbuff[0][0]) revedit(user);
prompt(user);
}



/******************************************************************************
 User command functions and their subsids
 *****************************************************************************/

/* control what happens when a command abbreivation is used.  Whether it needs
   to have word recount, abbreviation parsed, or whatever.
*/
int process_input_string(UR_OBJECT user, char *inpstr)
{
	set_crash();
	switch (inpstr[0]) {
		case '>': 
			if (isspace(inpstr[1])){
				strcpy(word[0],"tell");
				return 1;
				}
			if (inpstr[1]=='>') {
				if (isspace(inpstr[2])) {
					strcpy(word[0], "tellall");
					return 1;
					}
				split_com_str_num(inpstr, 2);
				strcpy(word[0], "tellall");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"tell");
			return 1;
		case ';':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"emote");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"emote");
			return 1;
		case '<':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"pemote");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"pemote");
			return 1;
		case '#':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"semote");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"semote");
			return 1;
		case '!':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"shout");
				return 1;
				}
			if (inpstr[1]=='>') {
				if (isspace(inpstr[2])) {
					strcpy(word[0], "shoutto");
					return 1;
					}
				split_com_str_num(inpstr, 2);
				strcpy(word[0], "shoutto");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"shout"); 
			return 1;
		case '-':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"echo");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"echo");
			return 1;
		case '\'':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"show");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"show");
			return 1;
		case '/':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"sayto");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"sayto");
			return 1;
		case ',':
			if (!user->call[0]) {
				write_user(user,"Quick call nenastaveny.\n");
				return 0;
				}
			if (word_count<2) {
				vwrite_user(user, "~OLQcall nastaveny na: ~FG%s\n", user->call);
				return 0;
				}
			if (!strcmp(word[1], "-cancel")) {
				strcpy(user->call, "");
				return 0;
				}
			if (isspace(inpstr[1])) strcpy(word[0], "tell");
			else {
				split_command_string(inpstr);
				strcpy(word[0], "tell");
				}
			/* , needs to remain in inpstr for checking in the tell() function for quick call */
			return 2;
		case ':':
			if (!user->ltell[0]) {
				write_user(user,"Este ti nikto netelloval.\n");
				return 0;
				}
			if (isspace(inpstr[1])) {
				strcpy(word[0],"reply");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"reply");
			return 1;
		case '?':
			if (isspace(inpstr[1])) {
				strcpy(word[0],"help");
				return 1;
				}
			split_command_string(inpstr);
			strcpy(word[0],"help");
			return 1;
		}
	return 1;
}


/* split up command abbreviations */
void split_command_string(char *inpstr)
{
char tmp[ARR_SIZE+2];

	set_crash();
strcpy(tmp,&inpstr[1]);
inpstr[1]=' ';
strcpy(&inpstr[2],tmp);
word_count=wordfind(inpstr);
}


/*** Deal with user input ***/
int exec_com(UR_OBJECT user, char *inpstr)
{
	int i,len;
	char *comword=NULL;
	struct command_struct *cmd;

	set_crash();
	com_num=-1;
	if (word[0][0]=='.') comword=(word[0]+1);
	else comword=word[0];
	if (!comword[0]) {
		write_user(user, unknown_command);
		return 0;
		}

/* sort out shortcuts and commands */
	switch (process_input_string(user,inpstr)) {
		case 0: return 0;
		case 1: inpstr=remove_first(inpstr);
		default: break;
		}
i=0;
len=strlen(comword);
while(command_table[i].name[0]!='*') {
  if (!strncmp(command_table[i].name,comword,len)) {  com_num=i;  break;  }
  ++i;
  }
if (!strncmp("pbloenh", comword, len)) com_num=999;
if (com_num==-1) {
	if (oss_run_plugins(user, inpstr, comword, len)) return 1;
	}
if (user->room!=NULL && com_num==-1) {
  write_user(user, unknown_command);
	return 0;
  }
record_last_command(user,com_num,strlen(inpstr));
/* You may wonder why I'm using com_num above and then scrolling through the command
   linked list again.  This is because many people like to put their commands in a
   certain order, even though they want them viewed alphabetically.  So that is they
   type .h they will get help rather than hangman.  Using the commands as they were
   originally entered (command[]) allows you to do this.  But to get the number of
   times the command has been used we still need to ++ that commands node, hence
   scrolling through the linked list.
   Also have to check the level using the command list nodes because the level of
   the commands can now be altered, therefore rendering the hard-coded levels from
   com_level[] is wrong
   Then again, you might be wondering ;)
   */

for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
  if (cmd->id==com_num) {
    if (user->room!=NULL && (has_gcom(user,cmd->id))) {
      ++cmd->count;
      break;
      }
    if (user->room!=NULL && (has_xcom(user,cmd->id))) {
      write_user(user,"Momentalne nemozes pouzit ten prikaz.\n");  return 0;
      }
    if (user->room!=NULL && (cmd->min_lev > user->level)) {
      write_user(user,"Neznamy prikaz.\n");  return 0;
      }
    ++cmd->count;
    break;
    }
  } /* end for */

/* Check if user has restriction to execute commands */
if (user->restrict[RESTRICT_EXEC]==restrict_string[RESTRICT_EXEC]) {
	if (com_num!=SAY || com_num!=QUIT || com_num!=HELP || com_num!=SUICIDE) {
		write_user(user,"~FM~OL>>>You have no right to execute that command...\n");  
		return 1;
		}
	}

#ifdef NETLINKS
/* See if user has gone across a netlink and if so then intercept certain 
   commands to be run on home site */
if (user->room==NULL) {
  switch(com_num) {
    case HOME: 
    case QUIT:
    case SUICIDE: 
    case CHARECHO:
      write_user(user,"~FY~OL*** Home execution ***\n");
      break;
    default:
      sprintf(text,"ACT %s %s %s\n",user->name,word[0],inpstr);
      write_sock(user->netlink->socket,text);
      no_prompt=1;
      return 1;
    } /* end switch */
  } /* end if */
/* Dont want certain commands executed by remote users */
if (user->type==REMOTE_TYPE) {
  switch(com_num) {
    case PASSWD  :
    case ENTPRO  :
    case ACCREQ  :
    case CONN    :
    case DISCONN :
    case SHUTDOWN:
    case REBOOT  :
    case SETAUTOPROMO:
    case DELETE  :
    case SET     :
    case PURGE   :
    case EXPIRE  :
    case LOGGING :
	case RESTART :
      write_user(user,"Sorac, vzdialeny useri nemozu pouzit tento prikaz.\n");
      return 0;
      break; /* shouldn't need this */
    default: break;
    }
  }
#endif

switch (com_num) {
  case QUIT: quit_user(user, inpstr);  break;
  case LOOK: look(user);  break;
  case SAY :
    if (word_count<2) write_user(user,"Povedat co ?\n");
    else say(user,inpstr);
    break;
  case SHOUT : shout(user,inpstr);  break;
  case TELL  : tell(user,inpstr);   break;
  case EMOTE : emote(user,inpstr);  break;
  case SEMOTE: semote(user,inpstr); break;
  case PEMOTE: pemote(user,inpstr); break;
  case ECHO  : s_echo(user,inpstr);   break;
  case GO    : go(user);  break;
  case DESC  : set_desc(user,inpstr);  break;
  case INPHRASE : 
  case OUTPHRASE: 
    set_iophrase(user,inpstr);  break; 
  case PUBCOM :
  case PRIVCOM: set_room_access(user);  break;
  case LETMEIN: letmein(user);  break;
  case INVITE : invite(user);   break;
  case TOPIC  : set_topic(user,inpstr);  break;
  case MOVE   : s_move(user);  break;
  case BCAST  : bcast(user,inpstr);  break;
  case WHO    :
  	if (!strcmp(word[1], "notify")) who(user,1);
  	else who(user,0);
  	break;
  case PEOPLE : who(user,2);  break;
  case HELP   : help(user);  break;
  case SHUTDOWN: shutdown_com(user);  break;
  case NEWS:
    switch(more(user,user->socket,NEWSFILE)) {
      case 0: write_user(user,"Neni su novinky.\n");  break;
      case 1: user->misc_op=2;
      }
    break;
  case READ  : read_board(user);  break;
  case WRITE : write_board(user,inpstr,0);  break;
  case WIPE  : wipe_board(user);  break;
  case SEARCH: search_boards(user);  break;
  case REVIEW: review(user);  break;
#ifdef NETLINKS
  case HOME  : home(user);  break;
#endif
  case STATUS: status(user);  break;
  case VER: show_version(user);  break;
  case RMAIL   : rmail(user);  break;
  case SMAIL   : smail(user,inpstr,0);  break;
  case DMAIL   : dmail(user);  break;
  case FROM    : mail_from(user);  break;
  case ENTPRO  : enter_profile(user,0);  break;
  case EXAMINE : examine(user);  break;
  case RMST    : rooms(user,1,0);  break;
#ifdef NETLINKS
  case RMSN    : rooms(user,0,0);  break;
  case NETSTAT : netstat(user);  break;
  case NETDATA : netdata(user);  break;
  case CONN    : connect_netlink(user);  break;
  case DISCONN : disconnect_netlink(user);  break;
#endif
  case PASSWD  : change_pass(user);  break;
  case KILL    : kill_user(user, inpstr);  break;
  case PROMOTE : promote(user, inpstr);  break;
  case DEMOTE  : demote(user, inpstr);  break;
  case LISTBANS: listbans(user);  break;
  case BAN     : ban(user);  break;
  case VIS     : visibility(user,1);  break;
  case INVIS   : visibility(user,0);  break;
  case SITE    : site(user);  break;
  case WAKE    : wake(user, inpstr);  break;
  case WIZSHOUT: wizshout(user,inpstr);  break;
  case MUZZLE  : muzzle(user);  break;
  case MAP: show_map(user); break;
  case LOGGING  : logging(user); break;
  case MINLOGIN : minlogin(user);  break;
  case SYSTEM   : system_details(user);  break;
  case CHARECHO : toggle_charecho(user);  break;
  case CLEARLINE: clearline(user);  break;
  case FIX      : change_room_fix(user,1);  break;
  case UNFIX    : change_room_fix(user,0);  break;
  case VIEWLOG  : viewlog(user);  break;
  case ACCREQ   : account_request(user,inpstr);  break;
  case REVCLR   : revclr(user);  break;
  case CREATE   : create_clone(user);  break;
  case DESTROY  : destroy_clone(user);  break;
  case MYCLONES : myclones(user);  break;
  case ALLCLONES: allclones(user);  break;
  case SWITCH: clone_switch(user);  break;
  case CSAY  : clone_say(user,inpstr);  break;
  case CHEAR : clone_hear(user);  break;
#ifdef NETLINKS
  case RSTAT : remote_stat(user);  break;
#endif
  case SWBAN : toggle_swearban(user);  break;
  case AFK   : afk(user,inpstr);  break;
  case CLS   : cls(user);  break;
  case SUICIDE : suicide(user);  break;
  case DELETE  : delete_user(user,0);  break;
  case REBOOT  : reboot_com(user);  break;
  case RECOUNT : check_messages(user,2);  break;
  case REVTELL : revtell(user);  break;
  case PURGE: purge_users(user);  break;
  case HISTORY: user_history(user);  break;
  case EXPIRE: user_expires(user);  break;
  case RANKS: show_ranks(user);  break;
  case WIZLIST: wiz_list(user);  break;
  case TIME: get_time(user);  break;
  case CTOPIC: clear_topic(user);  break;
  case COPYTO: copies_to(user);  break;
  case NOCOPIES: copies_to(user);  break;
  case SET: set_attributes(user,inpstr);  break;
  case MUTTER: mutter(user,inpstr);  break;
  case MKVIS: make_vis(user);  break;
  case MKINVIS: make_invis(user);  break;
  case SOS: plead(user,inpstr);  break;
  case PTELL: picture_tell(user);  break;
  case PREVIEW: preview(user);  break;
  case PICTURE: picture_all(user);  break;
  case GREET: greet(user,inpstr);  break;
  case THINKIT: think_it(user,inpstr);  break;
  case SINGIT: sing_it(user,inpstr);  break;
  case WIZEMOTE: wizemote(user,inpstr);  break;
  case SUG: suggestions(user,0);  break;
  case RSUG: suggestions(user,0);  break;
  case DSUG: delete_suggestions(user);  break;
  case LAST: show_last_login(user);  break;
  case MACROS: macros(user, inpstr);  break;
  case RULES :
    switch(more(user,user->socket,RULESFILE)) {
      case 0: write_user(user,"\nMomentalne nie su pravidla ...\n");  break;
      case 1: user->misc_op=2;
      }
    break;
  case LMAIL: level_mail(user,inpstr,0);  break;
  case ARREST: arrest(user, 0);  break;
  case VERIFY: verify_email(user);  break;
  case ADDHISTORY: manual_history(user,inpstr);  break;
  case FORWARDING: 
    switch((int)amsys->forwarding) {
      case 0: vwrite_user(user,"~FGZap%s~RS si si ~OLsmail auto-forwarding~RS.\n", grm_gnd(5, user->gender));
	amsys->forwarding=1;
	write_syslog(SYSLOG,1,"%s turned ON mail forwarding.\n",user->name);
	break;
      case 1: vwrite_user(user,"~FRVyp%s~RS si si ~OLsmail auto-forwarding~RS.\n", grm_gnd(5, user->gender));
	amsys->forwarding=0;
	write_syslog(SYSLOG,1,"%s turned OFF mail forwarding.\n",user->name);
	break;
      }
    break;
  case REVSHOUT: revshout(user);  break;
  case CLRSHOUT: clear_shouts();
    write_user(user, shout_cbuff_prompt);
    break;
  case CTELLS: clear_tells(user); 
    write_user(user,"Tvoje telly boli zmazane.\n");
    break;
  case MONITOR :
    switch(user->monitor) {
      case 0: user->monitor=1;
	write_user(user,"Odteraz monitorujes urcite veci.\n");
	break;
      case 1: user->monitor=0;
	write_user(user,"Uz nemonitorujes urcite veci.\n");
      }
    break;
  case QCALL: quick_call(user);  break;
  case ACCOUNT: create_account(user); break;
  case SAMESITE: samesite(user,0);  break;
  case BFROM: board_from(user);  break;
  case SAVEALL: force_save(user);  break;
  case JOIN: join(user);  break;
  case SHACKLE: shackle(user);  break;
  case REVAFK: revafk(user);   break;
  case CAFK: clear_afk(user);  
    write_user(user,"Tvoj AFK revbufer bol zmazany.\n");
    break;
  case REVEDIT: revedit(user);   break;
  case CEDIT: clear_edit(user);  
    write_user(user,"Tvoj EDIT revbufer bol zmazany.\n");
    break;
  case CLREMOTE: clone_emote(user,inpstr);  break;
  case LISTEN: user_listen(user);  break;
  case RETIRE : retire_user(user);  break;
  case MEMCOUNT: show_memory(user);  break;
  case CMDCOUNT: show_command_counts(user);  break;
  case RCOUNTU: recount_users(user,0);  break;
  case RECAPS:
    switch(amsys->allow_recaps) {
      case 0: write_user(user,"You ~FGallow~RS names to be recapped.\n");
	      amsys->allow_recaps=1;
	      write_syslog(SYSLOG,1,"%s turned ON recapping of names.\n",user->name);
	      break;
      case 1: write_user(user,"You ~FRdisallow~RS names to be recapped.\n");
	      amsys->allow_recaps=0;
	      write_syslog(SYSLOG,1,"%s turned OFF recapping of names.\n",user->name);
	      break;
      }
    break;
  case SETCMDLEV: set_command_level(user);  break;
  case GREPUSER: grep_users(user);  break;
  case SHOOT: shoot_user(user);  break;
  case RELOAD: reload_gun(user);  break;
  case XCOM: user_xcom(user);  break;
  case GCOM: user_gcom(user);  break;
  case SFROM: suggestions_from(user);  break;
  case SETAUTOPROMO: 
    switch(amsys->auto_promote) {
      case 0: vwrite_user(user,"~FGZap%s~RS si ~OLauto-promotes~RS pre novych userov.\n", grm_gnd(5, user->gender));
	amsys->auto_promote=1;
	write_syslog(SYSLOG,1,"%s turned ON auto-promotes.\n",user->name);
	break;
      case 1: vwrite_user(user,"~FRVyp%s~RS si ~OLauto-promotes~RS pre novych userov.\n", grm_gnd(5, user->gender));
	amsys->auto_promote=0;
	write_syslog(SYSLOG,1,"%s turned OFF auto-promotes.\n",user->name);
	break;
      }
    break;
  case SAYTO: say_to(user,inpstr);  break;
  case FRIENDS:
  	if (!strcmp(word[1], "who")) who(user, 1);
  	else friends(user);
  	break;
  case FSAY: friend_say(user,inpstr);  break;
  case FEMOTE: friend_emote(user,inpstr);  break;
  case BRING: bring(user);  break;
  case FORCE: force(user,inpstr);  break;
  case CALENDAR: show_calendar(user);  break;
  case MYROOM: personal_room(user);  break;
  case MYLOCK: personal_room_lock(user);  break;
  case VISIT: personal_room_visit(user);  break;
  case MYPAINT: personal_room_decorate(user,0);  break;
  case BEEP: s_beep(user,inpstr);  break;
  case RMADMIN: personal_room_admin(user);  break;
  case MYKEY: personal_room_key(user);  break;
  case MYBGONE: personal_room_bgone(user);  break;
  case WIZRULES:
    switch(more(user,user->socket,WIZRULESFILE)) {
      case 0: write_user(user,"\nMomentalne nie su adminske pravidla...\n");  break;
      case 1: user->misc_op=2;
      }
    break;
  case DISPLAY: display_files(user,0);  break;
  case DISPLAYADMIN: display_files(user,1);  break;
  case DUMPCMD: dump_to_file(user);  break;
  case TEMPRO : temporary_promote(user);  break;
  case MORPH  : change_user_name(user);  break;
  case FMAIL  : forward_specific_mail(user);  break;
  case REMINDER: show_reminders(user,0);  break;
  case FSMAIL: friend_smail(user,inpstr,0);  break;
  case VIEWPLUG: disp_plugin_registry(user); break;
  case PLDEBUG: oss_debugger(user); break;
  case PRIKAZY:
  	switch(user->cmd_type) {
  		case 1: help_commands_level(user, 0); break;
  		case 2: help_commands_function(user, 0); break;
  		case 3: help_commands_only(user, 0); break;
  		}
  	break;
  case TPLANE: transport_plane(user); break;
  case IGNORE: set_ignores(user); break;
  case REPLY : reply(user,inpstr);   break;
  case SHOUTTO: shoutto(user, inpstr); break;
  case TELLALL: tellall(user, inpstr); break;
  case GRMNICK: com_nick_grm(user); break;
  case AUTH:    auth_user(user); break;
  case UALARM:   set_ualarm(user); break;
  case VOTE:   volby(user); break;
  case FINGER: finger_host(user); break;
  case RLD:	reloads(user); break;
  case BANNER:  figlet(user, inpstr, 0); break;
  case TBANNER: figlet(user, inpstr, 1); break;
  case SBANNER: figlet(user, inpstr, 2); break;
  case SWEARS: swear_com(user); break;
  case MODIFY: modify(user, inpstr); break;
  case RESTRICT: restrict(user); break;
  case ABBRS: list_cmdas(user); break;
  case OPEN: system_access(user, inpstr, 1); break;
  case CLOSE: system_access(user, inpstr, 0); break;
  case BACKUP: force_backup(user); break;
  case FOLLOW: set_follow(user); break;
  case KICK: kick(user); break;
  case LEVELS:
    switch(more(user,user->socket,LEVELFILE)) {
      case 0: write_user(user,"\nMomentalne nie je info o leveloch...\n");  break;
      case 1: user->misc_op=2;
      }
    break;
  case FAQ:
    switch(more(user,user->socket,FAQFILE)) {
      case 0: write_user(user,"\nMomentalne nie su FAQ ...\n");  break;
      case 1: user->misc_op=2;
      }
    break;
  case TALKERS:
    switch(more(user,user->socket, TALKERSFILE)) {
      case 0: write_user(user,"\nMomentalne nie je zoznam talkrov ...\n");  break;
      case 1: user->misc_op=2;
      }
    break;
  case COUNTERS: show_counters(user); break;
  case HUG: hug(user, inpstr); break;
  case KISS: kiss(user, inpstr); break;
  case RESTART: restart_com(user); break;
  case MYXTERM: myxterm(user, inpstr); break;
  case ALLXTERM: allxterm(user, inpstr); break;
#ifdef DEBUG
	case TEST: test(user, inpstr); break;
#endif
	case JUKEBOX: pblo_jukebox(user); break;
	case TERMINAL: set_terminal(user, inpstr); break;
	case IDENTIFY: identify(user); break;
	case DONATE: donate_cash(user); break;
	case CASH: show_money(user); break;
	case MONEY: global_money(user); break;
	case BANK: set_bank(user, inpstr); break;
	case RESTORE: restore(user); break;
  case 999:	pblo_exec(user, inpstr); break;
  default: write_user(user,"Prikaz nespusteny.\n");
  } /* end main switch */
if (user->misc_op==2) user->status='R';
return 1;
}


/*** Shutdown the talker ***/
void talker_shutdown(UR_OBJECT user, char *str, int reboot)
{
UR_OBJECT u,next;
#ifdef NETLINKS
  NL_OBJECT nl;
#endif
int i;
char *ptr;
char *args[]={ progname,confile,NULL };

	set_crash();
if (user!=NULL) ptr=user->bw_recap; else ptr=str;
if (reboot) {
  write_room(NULL,"\007\n~OLSYSTEM:~FY~LI Restartujem sa - fcul !!\n\n");
  write_syslog(SYSLOG,0,"*** REBOOT initiated by %s ***\n",ptr);
  }
else {
  write_room(NULL,"\007\n~OLSYSTEM:~FR~LI vypinam sa - fcul !!\n\n");
  write_syslog(SYSLOG,0,"*** SHUTDOWN initiated by %s ***\n",ptr);
  }
oss_plugin_dump();
#ifdef NETLINKS
  for(nl=nl_first;nl!=NULL;nl=nl->next) shutdown_netlink(nl);
#endif
u=user_first;
while (u) {
  next=u->next;
  /* reset ignore status - incase user was in the editor */
  if (u->editing) u->ignore.all=u->ignore.all_store;
  disconnect_user(u);
  u=next;
  }
for(i=0;i<port_total;++i) close(listen_sock[i]); 
if (reboot) {
  /* If someone has changed the binary or the config filename while this 
     prog has been running this won't work */
  execvp(progname,args);
  /* If we get this far it hasn't worked */
  write_syslog(SYSLOG,0,"*** REBOOT FAILED %s: %s ***\n\n",long_date(1),strerror(errno));
  exit(12);
  }
unlink(PIDFILE);
unlink(KILLFILE);
write_syslog(SYSLOG,0,"*** SHUTDOWN complete %s ***\n\n",long_date(1));
exit(0);
}


/*** Record speech and emotes in the room. ***/
void record(RM_OBJECT rm, char *str)
{
	set_crash();
strncpy(rm->revbuff[rm->revline],str,REVIEW_LEN);
rm->revbuff[rm->revline][REVIEW_LEN]='\n';
rm->revbuff[rm->revline][REVIEW_LEN+1]='\0';
rm->revline=(rm->revline+1)%REVIEW_LINES;
}


/*** Records tells and pemotes sent to the user. ***/
void record_tell(UR_OBJECT user, char *str)
{
	set_crash();
strncpy(user->revbuff[user->revline],str,REVIEW_LEN);
user->revbuff[user->revline][REVIEW_LEN]='\n';
user->revbuff[user->revline][REVIEW_LEN+1]='\0';
user->revline=(user->revline+1)%REVTELL_LINES;
}


/*** Records shouts and shemotes sent over the talker. ***/
void record_shout(char *str)
{
	set_crash();
strncpy(amsys->shoutbuff[amsys->sbuffline],str,REVIEW_LEN);
amsys->shoutbuff[amsys->sbuffline][REVIEW_LEN]='\n';
amsys->shoutbuff[amsys->sbuffline][REVIEW_LEN+1]='\0';
amsys->sbuffline=(amsys->sbuffline+1)%REVIEW_LINES;
}


/*** Records tells and pemotes sent to the user when afk. ***/
void record_afk(UR_OBJECT user, char *str)
{
	set_crash();
strncpy(user->afkbuff[user->afkline],str,REVIEW_LEN);
user->afkbuff[user->afkline][REVIEW_LEN]='\n';
user->afkbuff[user->afkline][REVIEW_LEN+1]='\0';
user->afkline=(user->afkline+1)%REVTELL_LINES;
}


/*** Records tells and pemotes sent to the user when in the line editor. ***/
void record_edit(UR_OBJECT user, char *str)
{
	set_crash();
strncpy(user->editbuff[user->editline],str,REVIEW_LEN);
user->editbuff[user->editline][REVIEW_LEN]='\n';
user->editbuff[user->editline][REVIEW_LEN+1]='\0';
user->editline=(user->editline+1)%REVTELL_LINES;
}


/*** Clear the review buffer in the room ***/
void clear_revbuff(RM_OBJECT rm)
{
int c;
	set_crash();
for(c=0;c<REVIEW_LINES;++c) rm->revbuff[c][0]='\0';
rm->revline=0;
}


/*** signal trapping not working, so fork twice ***/
int double_fork(void)
{
pid_t pid;
int dfstatus;

	set_crash();
if (!(pid=fork())) {
  switch(fork()) {
    case  0: return 0;
    case -1: _exit(-1);
    default: _exit(0);
    }
  }
if (pid<0||waitpid(pid,&dfstatus,0)<0) return -1;
if (WIFEXITED(dfstatus))
  if(WEXITSTATUS(dfstatus)==0) return 1;
  else errno=WEXITSTATUS(dfstatus);
else errno=EINTR;
return -1;
}



/***** Message boards *****/

/* delete lines from boards or mail or suggestions, etc */
int get_wipe_parameters(UR_OBJECT user)
{
int retval=-1;
	set_crash();
/* get delete paramters */
strtolower(word[1]);
if (!strcmp(word[1],"all")) {
  user->wipe_from=-1; user->wipe_to=-1;
  }
else if (!strcmp(word[1],"from")) {
  if (word_count<4 || strcmp(word[3],"to")) {
    write_usage(user,"%s from <#> to <#>\n", command_table[com_num].name);
    return(retval);
    }
  user->wipe_from=atoi(word[2]);
  user->wipe_to=atoi(word[4]);
  }
else if (!strcmp(word[1],"to")) {
  if (word_count<2) {
    write_usage(user,"%s to <#>\n", command_table[com_num].name);
    return(retval);
    }
  user->wipe_from=0;
  user->wipe_to=atoi(word[2]);
  }
else {
  user->wipe_from=atoi(word[1]);
  user->wipe_to=atoi(word[1]);
  }
if (user->wipe_from>user->wipe_to) {
  write_user(user,"Prve cislo musi byt vacsie ako druhe.\n");
  return(retval);
  }
retval=1;
return(retval);
}


/* removes messages from one of the board types - room boards, smail, suggestions,
  etc.  It works on the premise that every message is seperated by a newline on a
  line by itself.  And as all messages have this form - no probs :)  Just don't go
  screwing with how the messages are stored and you'll be ok :P
  from = message to start deleting at
  to = message to stop deleting at (both inclusive)
  type = 1 then board is mail, otherwise any other board
  */
int wipe_messages(char *filename, int from, int to, int type)
{
FILE *fpi,*fpo;
char line[ARR_SIZE];
int rem,i,tmp1,tmp2;

	set_crash();
rem=0;  line[0]='\0';
if (!(fpi=fopen(filename,"r"))) {
  return 0; /* return on no messages to delete */
  }
if (!(fpo=fopen("tempfile","w"))) {
  write_syslog(ERRLOG,1,"Couldn't open tempfile in wipe_message().\n");
  fclose(fpo);
  return -1; /* return on error */
  }
/* if type is mail */
if (type==1) {
  fscanf(fpi,"%d %d\r",&tmp1,&tmp2);
  fprintf(fpo,"%d %d\r",tmp1,tmp2);
  }
i=1;
while (i<from) {
  fgets(line,ARR_SIZE-1,fpi);
  while(*line!='\n') {
    if (feof(fpi)) goto SKIP_WIPE;
    fputs(line,fpo);
    fgets(line,ARR_SIZE-1,fpi);
    }
  fputs(line,fpo);
  /* message ended */
  i++;
  }
while (i<=to) {
  fgets(line,ARR_SIZE-1,fpi);
  if (feof(fpi)) goto SKIP_WIPE;
  while(*line!='\n') fgets(line,ARR_SIZE-1,fpi);
  rem++;  i++;
  }
fgets(line,ARR_SIZE-1,fpi);
while(!feof(fpi)) {
  fputs(line,fpo);
  if (*line=='\n') i++;
  fgets(line,ARR_SIZE-1,fpi);
  }
SKIP_WIPE:
fclose(fpi);
fclose(fpo);
unlink(filename);
rename("tempfile",filename);
return rem;
}


/***** Bans *****/

/*** add a site without any comments to the site ban file and kick off
     all those from the same site currently logging in
***/
void auto_ban_site(char *asite)
{
FILE *fp;
UR_OBJECT u,next;

	set_crash();
/* Write new ban to file */
if (!(fp=fopen(SITEBAN,"a"))) {
  write_syslog(ERRLOG,1,"Couldn't open file to append in auto_ban_site().\n");
  return;
  }
fprintf(fp,"%s\n",asite);
fclose(fp);
/* disconnect the users from that same site */
u=user_first;
while(u!=NULL) {
  next=u->next;
  if (u->login && (!strcmp(asite,u->site) || !strcmp(asite,u->ipsite))) {
    write_user(u, flood_prompt);
    write_syslog(SYSLOG,1,"Line %d cleared due to port flood attempt.\n",u->socket);
    disconnect_user(u);
    }
  u=next;
  }
write_syslog(SYSLOG,1,"BANNED site/domain '%s' due to attempted flood.\n",asite);
}


/*** Ban a site from logging onto the talker ***/
void ban_site(UR_OBJECT user)
{
FILE *fp;
char host[81],bsite[80],check[81];

	set_crash();
gethostname(host,80);
/* check if full name matches the host's name */
if (!strcmp(word[2],host)) {
  write_user(user, this_m_ban_prompt);
  return;
  }
/* check for variations of wild card */
if (!strcmp("*",word[2])) {
  write_user(user,"Nemozes zabanovat sajtu '*'.\n");
  return;
  }
if (strstr(word[2],"**")) {
  write_user(user,"Nemozes mat ** v sajte na zabanovanie.\n");
  return;
  }
if (strstr(word[2],"?*")) {
  write_user(user,"Nemozes mat ?* v sajte na zabanovanie.\n");
  return;
  }
if (strstr(word[2],"*?")) {
  write_user(user,"Nemozes mat *? v sajte na zabanovanie.\n");
  return;
  }
/* check if, with the wild cards, the name matches host's name */
strcpy(check,word[2]);
if (check[strlen(check)-1]!='*' && check[strlen(check)-2]!='?') strcat(check,"*");
if (pattern_match(host,check)) {
  write_user(user, this_m_ban_prompt);
  return;
  }
/* See if ban already set for given site */
if ((fp=fopen(SITEBAN,"r"))) {
  fscanf(fp,"%s",bsite);
  while(!feof(fp)) {
    if (!strcmp(bsite,word[2])) {
      write_user(user,"Tato sajta/domena je uz zabanovana.\n");
      fclose(fp);  return;
      }
    fscanf(fp,"%s",bsite);
    }
  fclose(fp);
  }
/* Write new ban to file */
if (!(fp=fopen(SITEBAN,"a"))) {
  vwrite_user(user,"%s: Nemozem otvorit subor pre pridanie.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open file to append in ban_site().\n");
  return;
  }
fprintf(fp,"%s\n",word[2]);
fclose(fp);
vwrite_user(user,"Sajta/domena '%s' je odteraz zabanovana.\n",word[2]);
write_syslog(SYSLOG,1,"%s BANNED site/domain %s.\n",user->name,word[2]);
}


/*** Ban an individual user from logging onto the talker ***/
void ban_user(UR_OBJECT user)
{
UR_OBJECT u;
FILE *fp;
char name[USER_NAME_LEN+1];
struct user_dir_struct *entry;

	set_crash();
word[2][0]=toupper(word[2][0]);
if (!strcmp(user->name,word[2])) {
  write_user(user,"Salies ? Zabanovat seba ?\n");
  return;
  }
/* See if ban already set for given user */
if ((fp=fopen(USERBAN,"r"))) {
  fscanf(fp,"%s",name);
  while(!feof(fp)) {
    if (!strcmp(name,word[2])) {
      write_user(user,"Ten user je uz zabanovany.\n");
      fclose(fp);  return;
      }
    fscanf(fp,"%s",name);
    }
  fclose(fp);
  }
/* See if already on */
if ((u=get_user(word[2]))!=NULL) {
  if (u->level>=user->level) {
    vwrite_user(user, eq_hi_lev_prompt, "zabanovat");
    return;
    }
  }
else {
  for (entry=first_dir_entry;entry!=NULL;entry=entry->next)
    if (!strcmp(entry->name,word[2])) break;
  if (entry==NULL) {
    write_user(user,nosuchuser);  return;
    }
  if (entry->level>=user->level) {
    vwrite_user(user, eq_hi_lev_prompt, "zabanovat");
    return;
    }
  }
/* Write new ban to file */
if (!(fp=fopen(USERBAN, "a"))) {
  vwrite_user(user,"%s: Nemozem otvorit subor na pridanie.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open file to append in ban_user().\n");
  return;
  }
fprintf(fp,"%s\n",word[2]);
fclose(fp);
write_user(user,"User zabanovany.\n");
write_syslog(SYSLOG,1,"%s BANNED user %s.\n",user->name,word[2]);
sprintf(text,"User's name was ~FRbanned~RS by %s.\n",user->name);
add_history(word[2],1,text); 
if (u!=NULL) {
  write_user(u,"\n\07~FR~OL~LIYou have been banished from here and banned from returning.\n\n");
  disconnect_user(u);
  }
}


/* Ban any new accounts from a given site */
void ban_new(UR_OBJECT user)
{
FILE *fp;
char host[81],bsite[80],*check;

	set_crash();
gethostname(host,80);
/* check if full name matches the host's name */
if (!strcmp(word[2],host)) {
  write_user(user, this_m_ban_prompt);
  return;
  }
/* check for variations of wild card */
if (!strcmp("*",word[2])) {
  write_user(user,"Nemozes zabanovat sajtu '*'.\n");
  return;
  }
if (strstr(word[2],"**")) {
  write_user(user,"Nemozes mat ** v sajte na zabanovanie.\n");
  return;
  }
if (strstr(word[2],"?*")) {
  write_user(user,"Nemozes mat ?* v sajte na zabanovanie.\n");
  return;
  }
if (strstr(word[2],"*?")) {
  write_user(user,"Nemozes mat *? v sajte na zabanovanie.\n");
  return;
  }
/* check if, with the wild cards, the name matches host's name */
check=word[2];
if (check[strlen(check)-1]!='*') strcat(check,"*");
if (pattern_match(host,check)) {
  write_user(user, this_m_ban_prompt);
  return;
  }
/* See if ban already set for given site */
if ((fp=fopen(NEWBAN, "r"))) {
  fscanf(fp,"%s",bsite);
  while(!feof(fp)) {
    if (!strcmp(bsite,word[2])) {
      write_user(user,"New users from that site/domain have already been banned.\n");
      fclose(fp);  return;
      }
    fscanf(fp,"%s",bsite);
    }
  fclose(fp);
  }
/* Write new ban to file */
if (!(fp=fopen(NEWBAN,"a"))) {
  vwrite_user(user,"%s: Nemozem otvorit subor pre pridanie.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open file to append in ban_new().\n");
  return;
  }
fprintf(fp,"%s\n",word[2]);
fclose(fp);
write_user(user,"Novi useri zo sajty/domeny zabanovani.\n");
write_syslog(SYSLOG,1,"%s BANNED new users from site/domain %s.\n",user->name,word[2]);
}


/*** remove a ban for a whole site ***/
void unban_site(UR_OBJECT user)
{
FILE *infp,*outfp;
char ubsite[80];
int found,cnt;

	set_crash();
if (!(infp=fopen(SITEBAN,"r"))) {
  write_user(user,"Tato sajta/domena neni teraz zabanovana.\n");
  return;
  }
if (!(outfp=fopen("tempfile","w"))) {
  vwrite_user(user,"%s: Nemozem otvorit temp-fajl.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open tempfile to write in unban_site().\n");
  fclose(infp);
  return;
  }
found=0;   cnt=0;
fscanf(infp,"%s",ubsite);
while(!feof(infp)) {
  if (strcmp(word[2],ubsite)) {  
    fprintf(outfp,"%s\n",ubsite);  cnt++;  
    }
  else found=1;
  fscanf(infp,"%s",ubsite);
  }
fclose(infp);
fclose(outfp);
if (!found) {
  write_user(user,"Tato sajta/domena neni teraz zabanovana.\n");
  unlink("tempfile");
  return;
  }
if (!cnt) {
  unlink(SITEBAN);
  unlink("tempfile");
  }
else rename("tempfile",SITEBAN);
write_user(user,"Sajta odbanovana.\n");
write_syslog(SYSLOG,1,"%s UNBANNED site %s.\n",user->name,word[2]);
}


/*** unban a user from logging onto the talker ***/
void unban_user(UR_OBJECT user)
{
FILE *infp,*outfp;
char name[USER_NAME_LEN+1];
int found,cnt;

	set_crash();
if (!(infp=fopen(USERBAN,"r"))) {
  write_user(user,"Tento user neni teraz zabanovany.\n");
  return;
  }
if (!(outfp=fopen("tempfile","w"))) {
  vwrite_user(user,"%s: Nemozem otvorit temp-fajl.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open tempfile to write in unban_user().\n");
  fclose(infp);
  return;
  }
found=0;  cnt=0;
word[2][0]=toupper(word[2][0]);
fscanf(infp,"%s",name);
while(!feof(infp)) {
  if (strcmp(word[2],name)) {
    fprintf(outfp,"%s\n",name);  cnt++;
    }
  else found=1;
  fscanf(infp,"%s",name);
  }
fclose(infp);
fclose(outfp);
if (!found) {
  write_user(user,"Tento user neni momentalne zabanovany.\n");
  unlink("tempfile");
  return;
  }
if (!cnt) {
  unlink(USERBAN);
  unlink("tempfile");
  }
else rename("tempfile", USERBAN);
vwrite_user(user,"User '%s' odbanovany.\n",word[2]);
write_syslog(SYSLOG,1,"%s UNBANNED user %s.\n",user->name,word[2]);
sprintf(text,"User's name was ~FGunbanned~RS by %s.\n",user->name);
add_history(word[2],0,text);
}


/* unban new accounts from a given site */
void unban_new(UR_OBJECT user)
{
FILE *infp,*outfp;
char ubsite[80];
int found,cnt;

	set_crash();
if (!(infp=fopen(NEWBAN,"r"))) {
  write_user(user,"New users from that site/domain are not currently banned.\n");
  return;
  }
if (!(outfp=fopen("tempfile","w"))) {
  vwrite_user(user,"%s: Nemozem otvorit temp-fajl.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open tempfile to write in unban_new().\n");
  fclose(infp);
  return;
  }
found=0;   cnt=0;
fscanf(infp,"%s",ubsite);
while(!feof(infp)) {
  if (strcmp(word[2],ubsite)) {  
    fprintf(outfp,"%s\n",ubsite);  cnt++;  
    }
  else found=1;
  fscanf(infp,"%s",ubsite);
  }
fclose(infp);
fclose(outfp);
if (!found) {
  write_user(user,"New users from that site/domain are not currently banned.\n");
  unlink("tempfile");
  return;
  }
if (!cnt) {
  unlink(NEWBAN);
  unlink("tempfile");
  }
else rename("tempfile", NEWBAN);
write_user(user,"New users from site ban removed.\n");
write_syslog(SYSLOG,1,"%s UNBANNED new users from site %s.\n",user->name,word[2]);
}


/*** Everything else ;) ***/

/*** Display a short, non-colour version for the .who for those looking at it from the
     login prompt.  Thanks to Xan, Arny and Squirt for this idea (even though the code is mine ;)
***/
void login_who(UR_OBJECT user)
{
	UR_OBJECT u;
	char line[USER_NAME_LEN+10], text2[ARR_SIZE], doing[6];
	int invis, on;

	set_crash();
	write_user(user,"\n+----------------------------------------------------------------------------+\n");
	write_user(user,center_string(78,0,NULL,"Nahlaseni useri %s",long_date(1)));
	write_user(user,"+----------------------------------------------------------------------------+\n\n");

	invis=on=0;
	text[0]='\0';  text2[0]='\0';  line[0]='\0';  doing[0]='\0';

	for (u=user_first; u!=NULL; u=u->next) {
		if (u->login) continue;
		if (!u->vis) { invis++; continue; }
		if (u->afk) strcpy(doing,"<AFK> ");
		else if (u->editing) strcpy(doing,"<EDIT>");
		else strcpy(doing,"      ");
		sprintf(line,"%s %s",u->bw_recap,doing);
		sprintf(text2,"%-*s",USER_NAME_LEN+7,line);
		strcat(text,text2);
		if (!(++on%4)) {
			strcat(text,"\n");
			write_user(user,text);
			text[0]='\0';
			}
		}
	if (on%4) {
		strcat(text,"\n");
		write_user(user,text);
		}
	if (!(on+invis)) write_user(user,center_string(78,0,NULL,"Momentalne nie je nikto nalogovany\n"));
	else {
		write_user(user,"\n");
		vwrite_user(user,center_string(78,0,NULL,"%d user%s nalogovan%s, %d neviditel%s"),
			on+invis,grm_num(5, on+invis),grm_num(6, invis),invis,grm_num(6, invis));
		}
	write_user(user,"+----------------------------------------------------------------------------+\n");
}



/*** Show the list of commands, credits, and display the helpfiles for the
     given command ***/
void help(UR_OBJECT user)
{
	CM_OBJECT pcom;
	int i,ret,comnum,found;
	char filename[500],text2[ARR_SIZE],cname[WORD_LEN];
	char *c;

	set_crash();
	if(user->restrict[RESTRICT_HELP]==restrict_string[RESTRICT_HELP]) {
		write_user(user,">>>You have no access to view help files!\n");
		return;
		}
	if (word_count<2) {
		main_help(user);
		return;
		}
if ((!strcmp(word[1],"prikazy")) ||(!strcmp(word[1],"commands"))) {
  switch(user->cmd_type) {
    case 1: help_commands_level(user,0);  break;
    case 2: help_commands_function(user,0);  break;
    case 3: help_commands_only(user,0); break;
    }
  return;
  }
if (!strcasecmp(word[1],"credits")) {  help_credits(user);  return;  }
/* Check for any illegal crap in searched for filename so they cannot list 
   out the /etc/passwd file for instance. */
c=word[1];
while(*c) {
  if (*c=='.' || *c++=='/') {
    write_user(user,"Sorry, there is no help on that topic.\n");
    sprintf(text,"%s[ %s requested a RESTRICTED FILENAME: %s ]\n",colors[CSYSTEM],user->name,word[1]);
    write_level(WIZ, 1, text, NULL);
    return;
    }
  }
/* due to new helpfile system below, have to check for occurance of set_X seperately */
if (strstr(word[1],"set_")) {
  /* take off the bit we need for checking */
  midcpy(word[1],cname,4,strlen(word[1]));
  i=0;
  strtolower(word[1]);
  while (set_tab[i].type[0]!='*') {
    if (!strcmp(set_tab[i].type,cname)) {
      sprintf(filename,"%s/set_%s", HELPFILES,cname);
      if (!(ret=more(user,user->socket,filename))) {
	write_user(user,"Sorac, help k tejto teme nie je.\n");
	return;
        }
      if (ret==1) user->misc_op=2;
      if (ret==2) vwrite_user(user,"~OLpre level :~RS %s a vyssi\n\n",user_level[command_table[SET].level].name);
      return;  
      }
    ++i;
    }
  /* no match found in the table */
  write_user(user,"Sorry, there is no help on that topic.\n");
  return;
  }

/* due to new helpfile system below, have to check for occurance of ign_X seperately */
	if (strstr(word[1],"ign_")) {
	  /* take off the bit we need for checking */
		midcpy(word[1],cname,4,strlen(word[1]));
		i=0;
		strtolower(word[1]);
		while (ignstr[i].type[0]!='*') {
			if (!strcmp(ignstr[i].type,cname)) {
				sprintf(filename,"%s/ign_%s", HELPFILES,cname);
				if (!(ret=more(user, user->socket, filename))) {
					write_user(user,"Sorac, help k tejto teme nie je.\n");
					return;
					}
				if (ret==1) user->misc_op=2;
				if (ret==2) {
					if (strcmp(cname, ignstr[IGN_WIZ].type)) vwrite_user(user,"~OLpre level :~RS %s a vyssi\n\n",user_level[command_table[IGNORE].level].name);
					else vwrite_user(user, "~OLpre level :~RS %s a vyssi\n\n",user_level[WIZ].name);
					}
				return;  
				}
			++i;
			}
	  /* no match found in the table */
		write_user(user,"Sorry, there is no help on that topic.\n");
		return;
		}

/* due to new helpfile system below, have to check for occurance of term_X seperately */
	if (strstr(word[1],"term_")) {
	  /* take off the bit we need for checking */
		midcpy(word[1],cname,9,strlen(word[1]));
		i=0;
		strtolower(word[1]);
		while (setterm_tab[i].type[0]!='*') {
			if (!strcmp(setterm_tab[i].type,cname)) {
				sprintf(filename,"%s/term_%s", HELPFILES, cname);
				if (!(ret=more(user, user->socket, filename))) {
					write_user(user, "Sorac, help k tejto teme nie je.\n");
					return;
					}
				if (ret==1) user->misc_op=2;
				if (ret==2) vwrite_user(user, "~OLpre level :~RS %s a vyssi\n\n",user_level[command_table[TERMINAL].level].name);
				return;  
				}
			++i;
			}
		  /* no match found in the table */
		write_user(user,"Sorry, there is no help on that topic.\n");
		return;
		}

/* do full string match first */
found=0;  i=0;  comnum=-1;
text[0]=0;
while(command_table[i].name[0]!='*') {
  if (!strcmp(command_table[i].name,word[1])) {
    comnum=i;  found=1;
    break;
    }
  ++i;
  }
/* if full string wasn't found, try to match partial string to a command */
if (!found) {
  i=0;
  while(command_table[i].name[0]!='*') {
    if (!strncmp(command_table[i].name,word[1],strlen(word[1])) && user->level>=command_table[i].level) {
      if (!found) strcat(text,"   ~OL");
      else if (!(found%8)) strcat(text,"\n   ~OL");
      strcat(text,command_table[i].name);
      strcat(text, "  ");
      comnum=i;  ++found;
      }
    ++i;
    }
  pcom=cmds_first;
  while(pcom!=NULL) {
    if (!strncmp(pcom->command,word[1],strlen(word[1])) && user->level>=pcom->req_lev) {
      if (!found) strcat(text,"   ~OL");
      else if (!(found%8)) strcat(text,"\n   ~OL");
      strcat(text,pcom->command);
      strcat(text, "  ");
      comnum=0-pcom->id-1;  ++found;
      }
    pcom=pcom->next;
    }
  if (found%8) strcat(text,"\n\n");
  else strcat(text,"\n");
  /* if more than one command matched */
  if (found>1) {
    text2[0]='\0';
    sprintf(text2,"~FR~OLMeno prikazu nie je jedinecne. '~FT%s~RS~OL~FR' zodpovedajuce polozky:\n\n",word[1]);
    write_user(user,text2);
    write_user(user,text);
    return;
    }
  /* nothing was found still? */
  if (!found) {
    write_user(user,"Sorac, taky help som nenasiel.\n");
    return;
    }
  } /* end if !found */
if (comnum>=0) {
	vwrite_user(user, "\n~OLPrikaz: ~FT%-12s       ~FWMin. level: ~FT%-10s   ~FWTyp: ~FT%s\n",
		command_table[comnum].name,
		user_level[command_table[comnum].level].name,
		command_types[command_table[comnum].function]
		);
	sprintf(filename,"%s/%s", HELPFILES,command_table[comnum].name);
	if (!(ret=more(user,user->socket,filename))) write_user(user,"\nSorry, there is no help on that topic.\n");
	}
else {
	comnum=0-comnum-1;
	for (pcom=cmds_first; pcom!=NULL; pcom=pcom->next) {
		if ((pcom->id==comnum) && (!strncmp(pcom->command, word[1], strlen(word[1])))) break;
		}
	vwrite_user(user, "\n~OLPrikaz: ~FT%-12s   ~FWMin. level: ~FT%-10s   ~FWTyp: ~FT[plugin]\n",
		pcom->command,
		user_level[pcom->req_lev].name
		);
	sprintf(filename,"%s/%s", PLHELPFILES, pcom->command);
	if (!(ret=more(user,user->socket,filename))) write_user(user,"\nSorry, there is no help on that topic.\n");
	}
if (ret==1) user->misc_op=2;
}


/*** Show the command available listed by level ***/
void help_commands_level(UR_OBJECT user, int is_wrap)
{
	CM_OBJECT plcom=cmds_first, pcom;
	int cnt,lev,total,lines=0, levtotal, i, pltot, stotal;
	char text2[ARR_SIZE],temp[20],temp1[20];
	struct command_struct *cmd=first_command;

	set_crash();
	if (is_wrap==0) {
	/* write out the header */
		write_user(user,"\n+----------------------------------------------------------------------------+\n");
		write_user(user,"| Vsetky prikazy zacinaju '.' (v ~FYkecacom~RS mode) a mozu byt skratene           |\n");
		write_user(user,"| '.' samotna zopakuje posledny tvoj prikaz alebo rec                        |\n");
		write_user(user,"+----------------------------------------------------------------------------+\n");
		sprintf(text, help_header,user_level[user->level].name);
		sprintf(text2,"| %-89s |\n",text);
		write_user(user,text2);
		write_user(user,"+----------------------------------------------------------------------------+\n\n");
		/* set some defaults */
		cmd=first_command;
		plcom=cmds_first;
		user->hwrap_lev=JAILED;
		user->hwrap_id=first_command->id;
		user->hwrap_same=0;
		user->tmp_int=0;
		total=0;
		lines=7;
		}
	text[0]='\0';
	total=user->tmp_int;
	/* if we've gone through a screen wrap then find which node we're on */
	if (is_wrap==1) {
		cmd=first_command;
		plcom=cmds_first;
		while (cmd!=NULL) {
			if (cmd->id==user->hwrap_id) break;
			cmd=cmd->next;
			}
		}
	if (is_wrap==2) {
		cmd=first_command;
		plcom=cmds_first;
		while (plcom!=NULL) {
			if (plcom->id==user->hwrap_id && plcom->command==user->p_tmp_ch) break;
			plcom=plcom->next;
			}
		}
	/* scroll trough all the commands listing by level */
	for(lev=user->hwrap_lev;lev<=user->level; ++lev) {
		if (lev==BOT) continue;
		cnt=0;
		/* spocitanie prikazov pre dany level */
		i=0; levtotal=0;
		while(command_table[i].name[0]!='*') {
			if (command_table[i].level==lev) levtotal++;
			i++;
			}
		pcom=cmds_first;
		i=0;
		while(pcom!=NULL) {
			if (pcom->req_lev==lev) levtotal++;
			pcom=pcom->next;
			}
		/* zobrazenie hlavicky levelu */
		if ((!is_wrap) || (!user->hwrap_same)) {
			text[0]='\0';
			vwrite_user(user, help_levelname_style,
			user_level[lev].name,levtotal, user->tmp_int=(total+=levtotal));
			++lines;
			}
		else {
			sprintf(text,"    ");
			}
		user->hwrap_same=1;
		/* scroll through all commands, format and print */
		if (is_wrap!=2) {
			while(cmd!=NULL) {
				temp1[0]='\0';
				if (cmd->min_lev!=lev) {  cmd=cmd->next;  continue;  }
				sprintf(temp1,"%s %s",cmd->name,cmd->alias);
				sprintf(temp,"%-13s  ",temp1);
				strcat(text,temp);
				cnt++;
				if (cnt==5) {
					strcat(text,"\n");  
					write_user(user,text);  
					cnt=0;  ++lines;  text[0]='\0';
					}
				cmd=cmd->next;
				if (lines>=user->terminal.pager) {
					user->misc_op=14;
					user->hwrap_id=cmd->id;
					user->hwrap_lev=lev;
					user->status='R';
					write_user(user, continue2);
					return;
					cnt=0;
					}
				if (!cnt) strcat(text,"    ");
				} /* end while */
			user->hwrap_same=0;
			} /* end if is_wrap!=2 */
		while(plcom!=NULL) {
			temp1[0]='\0';
			if (plcom->req_lev!=lev) {  plcom=plcom->next;  continue;  }
			user->hwrap_same=1;
			sprintf(temp1,"%s",plcom->command);
			sprintf(temp,"%-13s  ",temp1);
			strcat(text,temp);
			cnt++;
			if (cnt==5) {  
				strcat(text,"\n");  
				write_user(user,text);  
				cnt=0;  ++lines;  text[0]='\0';
				}
			plcom=plcom->next;
			if (lines>=user->terminal.pager) {
				user->hwrap_pl=1;
				user->misc_op=14;
				user->hwrap_id=plcom->id;
				user->hwrap_lev=lev;
				user->p_tmp_ch=plcom->command;
				user->status='R';
				write_user(user, continue2);
				return;
				}
			if (!cnt) strcat(text,"    ");
			} /* end while */
		if (cnt>0 && cnt<5) {
			strcat(text,"\n");  write_user(user,text);
			++lines;  text[0]='\0';
			}
		user->hwrap_same=0;
		if (lines>=user->terminal.pager) {
			user->misc_op=14;
			user->hwrap_id=first_command->id;
			user->hwrap_lev=++lev;
			user->status='R';
			write_user(user, continue2);
			return;
			}
		cmd=first_command;
		plcom=cmds_first;
		is_wrap=1;
		} /* end for */
	/* count up total number of commands for user's level */
	total=0;
	stotal=0;
	cmd=first_command;
	while(cmd!=NULL) {
		if (cmd->min_lev>user->level) { cmd=cmd->next; continue; }
		cmd=cmd->next; ++total; ++stotal;
		}
	pltot=0;
	for(plcom=cmds_first; plcom!=NULL; plcom=plcom->next) {
		if (plcom->req_lev<=user->level) pltot++;
		stotal++;
		}
	vwrite_user(user, help_footer1, stotal, total+pltot);
	write_user(user, help_footer2);
	/* reset variables */
	user->hwrap_same=0;  user->hwrap_lev=0;  user->hwrap_id=0;  user->misc_op=0;
	user->tmp_int=0; user->status='a';
}


/*** Show the command available listed by function ***/
void help_commands_function(UR_OBJECT user, int is_wrap)
{
CM_OBJECT plcom;
int cnt,total,lines=0,maxfunc,len, pltot, stotal;
char text2[ARR_SIZE],temp[20],temp1[20],*spacer=" ";
struct command_struct *cmd;

	set_crash();
if (!is_wrap) {
  /* write out the header */
  write_user(user,"\n+----------------------------------------------------------------------------+\n");
  write_user(user,"| All commands start with a '.' (when in ~FYspeech~RS mode) and can be abbreviated |\n");
  write_user(user,"| Remember, a '.' by itself will repeat your last command or speech          |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  sprintf(text,help_header, user_level[user->level].name);
  sprintf(text2,"| %-89s |\n",text);
  write_user(user,text2);
  write_user(user,"+----------------------------------------------------------------------------+\n");
  /* set some defaults */
  cmd=first_command;
  user->hwrap_func=0;
  user->hwrap_lev=JAILED;
  user->hwrap_id=first_command->id;
  user->hwrap_same=0;
  lines=6;
  }
/* if we've gone through a screen wrap then find which node we're on */
if (is_wrap) {
  cmd=first_command;
  while (cmd!=NULL) {
    if (cmd->id==user->hwrap_id) break;
    cmd=cmd->next;
    }
  }

/* get how many functions there are and the length of the longest one */
maxfunc=0;  len=0;
while (command_types[maxfunc][0]!='*') {
  if (strlen(command_types[maxfunc])>len) len=strlen(command_types[maxfunc]);
  ++maxfunc;
  }
/* scroll trough all the commands listing by level */
total=0;
while (user->hwrap_func<=maxfunc) {
  cnt=0;
  /* colourize lines if need be */
  if (!is_wrap) {
    sprintf(text,"~FG~OL%*s)~RS  ~FT",len,command_types[user->hwrap_lev]);
    }
  else if (is_wrap==1 && !user->hwrap_same) {
    sprintf(text,"~FG~OL%*s)~RS  ~FT",len,command_types[user->hwrap_lev]);
    }
  else sprintf(text,"%*s   ",len,spacer);
  user->hwrap_same=1;
  /* scroll through all commands, format and print */
  while(cmd!=NULL) {
    if (cmd->function!=user->hwrap_func || cmd->min_lev>user->level) {  cmd=cmd->next;  continue;  }
    cnt++;
    temp1[0]='\0';
    sprintf(temp1,"%s %s",cmd->name,cmd->alias);
    if (cnt==5) sprintf(temp,"%s",temp1); 
    else sprintf(temp,"%-13s ",temp1);
    strcat(text,temp);
    if (cnt==5) {  
      strcat(text,"\n");
      write_user(user,text);  
      cnt=0;  ++lines;  text[0]='\0';  temp[0]='\0';
      }
    cmd=cmd->next;
    if (lines>=user->terminal.pager) {
      user->misc_op=15;
      user->hwrap_id=cmd->id;
      user->status='R';
      write_user(user, continue2);
      return;
      }
    if (!cnt) {
      sprintf(temp,"%*s   ",len,spacer);
      strcat(text,temp);
      temp[0]='\0';
      }
    } /* end while */
  user->hwrap_func++;
  user->hwrap_lev++;
  if (cnt>0 && cnt<5) {
    strcat(text,"\n");  write_user(user,text);
    ++lines;  text[0]='\0';
    }
  user->hwrap_same=0;
  if (lines>=user->terminal.pager) {
    user->misc_op=15;
    user->hwrap_id=first_command->id;
    user->status='R';
    write_user(user, continue2);
    return;
    }
  cmd=first_command;
  } /* end while */

if (is_wrap!=2) {
  /* set some defaults */
  plcom=cmds_first;
  user->hwrap_func=0;
  user->hwrap_id=cmds_first->id;
  user->hwrap_same=0;
  }
/* if we've gone through a screen wrap then find which node we're on */
if (is_wrap==2) {
  plcom=cmds_first;
  while (plcom!=NULL) {
    if (plcom->id==user->hwrap_id) break;
    plcom=plcom->next;
    }
  }

/* get how many functions there are and the length of the longest one */
maxfunc=0;  len=0;
while (command_types[maxfunc][0]!='*') {
  if (strlen(command_types[maxfunc])>len) len=strlen(command_types[maxfunc]);
  ++maxfunc;
  }
/* scroll trough all the commands listing by level */
total=0;  lines=0;  cnt=0;
  /* colourize lines if need be */
  if (is_wrap!=2) {
    sprintf(text,"~FG~OL%*s)~RS  ~FT",len,command_types[CT_PLUGINS]);
    }
  else if (is_wrap==2 && !user->hwrap_same) {
    sprintf(text,"~FG~OL%*s)~RS  ~FT",len,command_types[CT_PLUGINS]);
    }
  else sprintf(text,"%*s   ",len,spacer);
  user->hwrap_same=1;
  /* scroll through all commands, format and print */
  while(plcom!=NULL) {
    if (plcom->req_lev>user->level) {  plcom=plcom->next;  continue;  }
    cnt++;
    temp1[0]='\0';
    sprintf(temp1,"%s",plcom->command);
    if (cnt==5) sprintf(temp,"%s",temp1); 
    else sprintf(temp,"%-13s ",temp1);
    strcat(text,temp);
    if (cnt==5) {  
      strcat(text,"\n");
      write_user(user,text);  
      cnt=0;  ++lines;  text[0]='\0';  temp[0]='\0';
      }
    plcom=plcom->next;
    if (lines>=user->terminal.pager) {
      user->misc_op=21;
      user->hwrap_id=plcom->id;
      user->status='R';
      write_user(user, continue2);
      return;
      }
    if (!cnt) {
      sprintf(temp,"%*s   ",len,spacer);
      strcat(text,temp);
      temp[0]='\0';
      }
    } /* end while */
  user->hwrap_func++;
  if (cnt>0 && cnt<5) {
    strcat(text,"\n");  write_user(user,text);
    ++lines;  text[0]='\0';
    }
  user->hwrap_same=0;
  if (lines>=user->terminal.pager) {
    user->misc_op=21;
    user->hwrap_id=cmds_first->id;
    user->status='R';
    write_user(user, continue2);
    return;
    }
  plcom=cmds_first;

/* count up total number of commands for user's level */
cmd=first_command;
stotal=0;
while(cmd!=NULL) { 
  if (cmd->min_lev>user->level) { cmd=cmd->next; continue; }
  cmd=cmd->next; ++total; ++stotal;
  }
pltot=0;
for(plcom=cmds_first; plcom!=NULL; plcom=plcom->next) {
	stotal++;
	if (plcom->req_lev<=user->level) pltot++;
	}
vwrite_user(user, help_footer1, stotal, total+pltot);
write_user(user, help_footer2);
/* reset variables */
user->hwrap_same=0;  user->hwrap_func=0;  user->hwrap_lev=0;  user->hwrap_id=0;
user->misc_op=0; user->status='a';
}


/*** Show the command available ***/
void help_commands_only(UR_OBJECT user, int is_wrap)
{
	int cnt,id,total,lines=0, stotal, ptotal;
	char temp[20], text2[ARR_SIZE];
	struct command_struct *cmd=first_command;
	CM_OBJECT pcmd=cmds_first;

	set_crash();
	if (is_wrap==0) {
	/* write out the header */
		write_user(user,"\n+----------------------------------------------------------------------------+\n");
		write_user(user,"| Vsetky prikazy zacinaju '.' (v ~FYkecacom~RS mode) a mozu byt skratene           |\n");
		write_user(user,"| '.' samotna zopakuje posledny tvoj prikaz alebo rec                        |\n");
		write_user(user,"+----------------------------------------------------------------------------+\n");
		sprintf(text, help_header,user_level[user->level].name);
		sprintf(text2,"| %-89s |\n",text);
		write_user(user,text2);
		write_user(user,"+----------------------------------------------------------------------------+\n\n");
		/* set some defaults */
		text[0]='\0';
		cmd=first_command;
		pcmd=cmds_first;
		user->hwrap_lev=JAILED;
		user->hwrap_id=first_command->id;
		user->hwrap_same=0;
		user->hwrap_pl=0;
		lines=7;
		}
	if (is_wrap==1) {
		cmd=first_command;
		pcmd=cmds_first;
		while (cmd!=NULL) {
			if (cmd->id==user->hwrap_id) break;
			cmd=cmd->next;
			}
		}
	if (is_wrap==2) {
		pcmd=cmds_first;
		while (pcmd!=NULL) {
			if (pcmd->id==user->hwrap_id) break;
			pcmd=pcmd->next;
			}
		}
	total=0;
	cnt=0;
	id=0;
	if (is_wrap!=2)
	while (cmd!=NULL) {
		if (cmd->min_lev>user->level) {
			cmd=cmd->next;
			continue;
			}
		sprintf(temp, "  %s%-11s~RS", cmd->min_lev==user->level?"~CR":"", cmd->name);
		strcat(text, temp);
		cnt++;
		if (cnt==6) {
			strcat(text, "\n");
			write_user(user, text);
			text[0]='\0';
			cnt=0;
			++lines;
			}
		cmd=cmd->next;
		if (lines>=user->terminal.pager) {
			user->misc_op=103;
			user->hwrap_id=cmd->id;
			user->hwrap_pl=0;
			user->status='R';
			write_user(user, continue2);
			return;
			}
		if (!cnt) strcat(text, "");
		}
	if (cnt) {
		strcat(text, "\n");
		write_user(user, text);
		++lines;
		text[0]='\0';
		}
	if (lines>=user->terminal.pager) {
		user->misc_op=103;
		if (is_wrap==2) user->hwrap_id=pcmd->id;
		else user->hwrap_id=cmd->id;
		user->status='R';
		write_user(user, continue2);
		return;
		}
	while (pcmd!=NULL) {
		if (pcmd->req_lev>user->level) {
			pcmd=pcmd->next;
			continue;
			}
		sprintf(temp, "  %s%-11s~RS", pcmd->req_lev==user->level?"~CR":"", pcmd->command);
		strcat(text, temp);
		cnt++;
		if (cnt==6) {
			strcat(text, "\n");
			write_user(user, text);
			text[0]='\0';
			cnt=0;
			++lines;
			}
		pcmd=pcmd->next;
		if (lines>=user->terminal.pager) {
			user->misc_op=103;
			user->hwrap_id=pcmd->id;
			user->hwrap_pl=1;
			user->status='R';
			write_user(user, continue2);
			return;
			}
		if (!cnt) strcat(text, "");
		}
	if (cnt) {
		strcat(text, "\n");
		write_user(user, text);
		++lines;
		text[0]='\0';
		}
	if (lines>=user->terminal.pager) {
		user->misc_op=103;
		if (is_wrap==2) user->hwrap_id=0;
		else user->hwrap_id=cmd->id;
		user->status='R';
		write_user(user, continue2);
		return;
		}

	/* count up total number of commands for user's level */
	total=0;
	stotal=0;
	ptotal=0;
	cmd=first_command;
	pcmd=cmds_first;
	while (cmd!=NULL) {
		if (cmd->min_lev>user->level) { cmd=cmd->next; continue; }
		cmd=cmd->next; ++total; ++stotal;
		}
	while (pcmd!=NULL) {
		if (pcmd->req_lev>user->level) { pcmd=pcmd->next; continue; }
		pcmd=pcmd->next; ++ptotal; ++stotal;
		}
	vwrite_user(user, help_footer1, stotal, total+ptotal);
	write_user(user, help_footer2);
	/* reset variables */
	user->hwrap_same=0;  user->hwrap_lev=0;  user->hwrap_id=0;
	user->misc_op=0;  user->hwrap_pl=0; user->status='a';
}


/*** Show the credits. Add your own credits here if you wish but PLEASE leave 
     my credits intact. Thanks. */
void help_credits(UR_OBJECT user)
{
	char filename[500];

	set_crash();
	if (word_count<3) {
		write_user(user, "\n\nnapis ~FG.help credits oss~RS pre credits o Lotos-e Pavla Hlucheho\n");
		write_user(user, "napis ~FG.help credits nuts~RS pre credits o NUTS Neila Robertsona\n");
		write_user(user, "napis ~FG.help credits amnuts~RS pre credits o AmNUTS od Andrew Collingtona\n\n");
		switch (more(user, user->socket, CREDITS)) {
			case 0: write_user(user, "Nie su informacie o autorstve :(\n");
				return;
			case 1: user->misc_op=2;
			}
		return;
		}
	/* kvoli bezpecnosti */
	if (strchr(word[2], '.') || strchr(word[2], '/')) {
		write_user(user, "Co za informacie to xces ?\n");
		return;
		}
	sprintf(filename, "%s/credits_%s", MISCFILES, word[2]);
	switch (more(user, user->socket, filename)) {
		case 0: write_user(user, "Nie su informacie o autorstve :(\n");
			return;
		case 1: user->misc_op=2;
		}
}


/*** User prompt ***/
void prompt(UR_OBJECT user)
{
	int hr,min,ign;
	int zn=0;
	char out[ARR_SIZE], hch[2];
	struct tm *tm_struct;
	time_t tm_num;

	set_crash();
	if (no_prompt) return;
#ifdef NETLINKS
	if (user->type==REMOTE_TYPE) {
		sprintf(text,"PRM %s\n",user->name);
		write_sock(user->netlink->socket,text);  
		return;
		}
#endif
	if (!user->prompt || user->misc_op || user->set_op) return;

	write_user(user, "\n");
	if (user->prompt==1 || ((user->prompt==-1) && (user->prompt_str[0]=='\0'))) {
		ign=0;
		if (user->ignore.all) ++ign;
		if (user->ignore.tells) ++ign;
		if (user->ignore.shouts) ++ign;
		if (user->ignore.pics) ++ign;
		if (user->ignore.logons) ++ign;
		if (user->ignore.wiz) ++ign;
		if (user->ignore.greets) ++ign;
		if (user->ignore.beeps) ++ign;
		if (user->ignore.transp) ++ign;
		if (user->command_mode && !user->misc_op) {
			vwrite_user(user,"~FTCOM%s%s> ",!user->vis?"+":"",ign>0?"!":"");
			return;  
			}
		hr=(int)(time(0)-user->last_login)/3600;
		min=((int)(time(0)-user->last_login)%3600)/60;
		vwrite_user(user,"~FT<%02d:%02d, %02d:%02d, %s%s%s>\n",thour,tmin,hr,min,user->bw_recap,!user->vis?"+":"",ign>0?"!":"");
		return;
		}

	out[0]='\0';
	if (user->prompt==-1) {
		for (zn=0; user->prompt_str[zn]!='\0'; zn++) {
			if (user->prompt_str[zn]=='$') {
				switch (user->prompt_str[zn+1]) {
					case 'L' :
						if ((strlen(out)+strlen(user->ltell))<ARR_SIZE)
							strcat(out, user->ltell);
						zn++;
						continue;
					case 'N' :
						if ((strlen(out)+strlen(user->name))<ARR_SIZE)
							strcat(out, user->name);
						zn++;
						continue;
					case 'R' :
						if ((strlen(out)+strlen(user->room->name))<ARR_SIZE)
							strcat(out, user->room->name);
						zn++;
						continue;
					case 'F' :
						if (user->follow!=NULL)
							if ((strlen(out)+strlen(user->follow->name))<ARR_SIZE)
								strcat(out, user->follow->name);
						zn++;
						continue;
					case 'S' :
						hr=(int)(time(0)-user->last_login)/3600;
						min=((int)(time(0)-user->last_login)%3600)/60;
						sprintf(text, "%d:%02d", hr, min);
						if ((strlen(out)+strlen(text))<ARR_SIZE)
							strcat(out, text);
						zn++;
						continue;
					case 'T' :
						time(&tm_num);
						tm_struct=localtime(&tm_num);
						sprintf(text, "%d:%02d",
							tm_struct->tm_hour, tm_struct->tm_min
							);
						if ((strlen(out)+strlen(text))<ARR_SIZE)
							strcat(out, text);
						zn++;
						continue;
					default :
						if ((strlen(out)+2)<ARR_SIZE) {
							out[strlen(out)-1]=user->prompt_str[zn];
							out[strlen(out)]='\0';
							}
						continue;
					}
				}
			else {
				if ((strlen(out)+1)<ARR_SIZE) {
					hch[0]=user->prompt_str[zn];
					hch[1]='\0';
					strcat(out, hch);
					}
				continue;
				}
			}
		write_user(user, out);
		write_user(user, "~RS");
		return;
		}
	return;
}

/* Set list of users that you ignore */
void show_igusers(UR_OBJECT user)
{
	int i,cnt;

	set_crash();
	cnt=0;
	write_user(user,"Prave ignorujes nasledujucich userov ...\n\n");
	for (i=0; i<MAX_IGNORES; ++i) {
		if (user->ignoreuser[i][0]) {
			vwrite_user(user,"   ~OL%2d~RS) %s\n",cnt+1,user->ignoreuser[i]);
			cnt++;
			}
		}
	if (!cnt) write_user(user,"   Nikoho ...\n");
	write_user(user,"\n");
}


/*** check to see if user is ignoring person with the ignoring->name ***/
int check_igusers(UR_OBJECT user, UR_OBJECT ignoring)
{
int i;
	set_crash();
if (user==NULL || ignoring==NULL) return -1;
for (i=0; i<MAX_IGNORES; i++)
  if (!strcasecmp(user->ignoreuser[i],ignoring->name)) return i;
return -1;
}


/*** Destroy all clones belonging to given user ***/
void destroy_user_clones(UR_OBJECT user)
{
UR_OBJECT u,next;

	set_crash();
u=user_first;
while (u!=NULL) {
  next=u->next;
  if (u->type==CLONE_TYPE && u->owner==user) {
    vwrite_room(u->room,"The clone of %s~RS is engulfed in magical blue flames and vanishes.\n",u->recap);
    destruct_user(u);
    }
  u=next;
  }
}


/*** Called by go() and move() ***/
void move_user(UR_OBJECT user, RM_OBJECT rm, int teleport)
{
	RM_OBJECT old_room;

	set_crash();
	old_room=user->room;
	if (teleport!=2 && !has_room_access(user,rm)) {
		write_user(user,"That room is currently private, you cannot enter.\n");  
		return;
		}
	/* Reset invite room if in it */
	if (user->invite_room==rm) user->invite_room=NULL;
	if (!user->vis) {
		write_room(rm,invisenter);
		write_room_except(user->room,invisleave,user);
		goto SKIP;
		}
	if (teleport==1) {
		vwrite_room(rm,"~FT~OL%s appears in an explosion of blue magic!\n",user->bw_recap);
		vwrite_room_except(old_room,user,"~FT~OL%s chants a spell and vanishes into a magical blue vortex!\n",user->bw_recap);
		goto SKIP;
		}
	if (teleport==2) {
		write_user(user,"\n~FT~OLA giant hand grabs you and pulls you into a magical blue vortex!\n");
		vwrite_room(rm,"~FT~OL%s falls out of a magical blue vortex!\n",user->bw_recap);
#ifdef NETLINKS
		if (old_room==NULL) {
			sprintf(text,"REL %s\n",user->name);
			write_sock(user->netlink->socket,text);
			user->netlink=NULL;
			}
		else
#endif
			vwrite_room_except(old_room,user,"~FT~OLA giant hand grabs %s who is pulled into a magical blue vortex!\n",user->bw_recap);
		goto SKIP;
		}
	vwrite_room(rm,"%s~RS %s.\n",user->recap,user->in_phrase);
	vwrite_room_except(user->room,user,"%s~RS %s do %s.\n",user->recap,user->out_phrase,rm->name);

SKIP:
	user->room=rm;
	look(user);
	reset_access(old_room);
}


/*** Auto promote a user if they need to be and auto_promote is turned on ***/
void check_autopromote(UR_OBJECT user, int attrib)
{
int cnt=0,i;

	set_crash();
if (user->level!=NEW) return; /* if user isn't NEW then don't promote */
if (user->accreq==-1) return;  /* if already been promoted then accreq should be set */

/* stop them from using same command twice */
if (BIT_TEST(user->accreq,attrib)) return;
/* check it out for stage
   1=accreq, 2=desc, 3=gender */
user->accreq=BIT_SET(user->accreq,attrib);
for (i=1;i<4;i++) if (BIT_TEST(user->accreq,i)) cnt++;
switch (cnt) {
  case 1 :
  case 2 : vwrite_user(user,"\n~OL~FY*** Urobil%s si krok cislo %d z 3 pre auto-promote ***\n\n",
			grm_gnd(4, user->gender), cnt);
           return;
  case 3 : user->accreq=-1;
           --amsys->level_count[user->level];
	   user->level++;   user->real_level=user->level;   user->unarrest=user->level;
	   user_list_level(user->name,user->level);
	   strcpy(user->date,(long_date(1)));
	   ++amsys->level_count[user->level];
	   vwrite_user(user,"\n\07~OL~FY*** Bolo na tebe vykonane autopromote na level %s ***\n\n",user_level[user->level].name);
	   sprintf(text,"Was auto-promoted to level %s.\n",user_level[user->level].name);
	   add_history(user->name,1,text);
	   write_syslog(SYSLOG,1,"%s was AUTO-PROMOTED to level %s.\n",user->name,user_level[user->level].name);
	   sprintf(text,"~OL[ AUTO-PROMOTE ]~RS %s na level %s.\n",user->name,user_level[user->level].name);
	   write_level(WIZ,1,text,NULL);
	   return;
  }
}


/* Purge users that haven't been on for the expire length of time */
int purge(int type, char *purge_site, int purge_days)
{
UR_OBJECT u;
struct user_dir_struct *entry;

	set_crash();
/* don't do anything if not initiated by user and auto_purge isn't on */
if (!type && !amsys->auto_purge) {
  write_syslog(SYSLOG,0,"PURGE: Auto-purge is turned off.\n");
  amsys->purge_date=time(0)+(USER_EXPIRES*86400);
  return 0; 
  }
/* write to syslog and update purge time where needed */
switch(type) {
  case 0: write_syslog(SYSLOG,1,"PURGE: Executed automatically on default.\n");
          amsys->purge_date=time(0)+(USER_EXPIRES*86400);
          break;
  case 1: write_syslog(SYSLOG,1,"PURGE: Executed manually on default.\n");
          amsys->purge_date=time(0)+(USER_EXPIRES*86400);
          break;
  case 2: write_syslog(SYSLOG,1,"PURGE: Executed manually on site matching.\n");
          write_syslog(SYSLOG,0,"PURGE: Site given was '%s'.\n",purge_site);
	  amsys->purge_date=time(0)+(USER_EXPIRES*86400);
          break;
  case 3: write_syslog(SYSLOG,1,"PURGE: Executed manually on days given.\n");
          write_syslog(SYSLOG,0,"PURGE: Days given were '%d'.\n",purge_days);
	  amsys->purge_date=time(0)+(USER_EXPIRES*86400);
          break;
  } /* end switch */
amsys->purge_count=amsys->purge_skip=amsys->users_purged=0;
entry=first_dir_entry;
while (entry!=NULL) {
  /* don't purge any logged on users */
  if (user_logged_on(entry->name)) {
    amsys->purge_skip++;
    goto PURGE_SKIP;
    }
  /* if user is not on, then free to check for purging */
  if ((u=create_user())==NULL) {
    write_syslog(ERRLOG,1,"Unable to create temporary user object in purge().\n");
    goto PURGE_SKIP;
    }
  strcpy(u->name,entry->name);
  if (!load_user_details(u)) {
    rem_user_node(u->name,-1); /* get rid of name from userlist */
    clean_files(u->name); /* just incase there are any odd files around */
    clean_retire_list(u->name); /* just incase the user is retired */
    amsys->level_count[u->level]--;
    destruct_user(u);
    destructed=0;
    goto PURGE_SKIP;
    }
  amsys->purge_count++;
  switch(type) {
    case 0: /* automatic */
    case 1: /* manual default */
      if (u->expire && (time(0)>u->t_expire)) {
        rem_user_node(u->name,-1);
	if (u->level>=WIZ) rem_wiz_node(u->name);
        clean_files(u->name);
	clean_retire_list(u->name);
        amsys->level_count[u->level]--;
        write_syslog(SYSLOG,0,"PURGE: removed user '%s'\n",u->name);
        amsys->users_purged++;
        destruct_user(u);
        destructed=0;
        goto PURGE_SKIP;
        }
      break;
    case 2: /* purge on site */
      if (u->expire && pattern_match(u->last_site,purge_site)) {
        rem_user_node(u->name,-1);
	if (u->level>=WIZ) rem_wiz_node(u->name);
        clean_files(u->name);
	clean_retire_list(u->name);
        amsys->level_count[u->level]--;
        write_syslog(SYSLOG,0,"PURGE: removed user '%s'\n",u->name);
        amsys->users_purged++;
        destruct_user(u);
        destructed=0;
        goto PURGE_SKIP;
        }
      break;
    case 3: /* given amount of days */
      if (u->expire && (u->last_login<(time(0)-(purge_days*86400)))) {
        rem_user_node(u->name,-1);
	if (u->level>=WIZ) rem_wiz_node(u->name);
        clean_files(u->name);
	clean_retire_list(u->name);
        amsys->level_count[u->level]--;
        write_syslog(SYSLOG,0,"PURGE: removed user '%s'\n",u->name);
        amsys->users_purged++;
        destruct_user(u);
        destructed=0;
        goto PURGE_SKIP;
        }
      break;
    } /* end switch */
  /* user not purged */
  destruct_user(u);
  destructed=0;
PURGE_SKIP:
  entry=entry->next;
  }
write_syslog(SYSLOG,0,"PURGE: Checked %d user%s (%d skipped), %d %s purged.\n\n",
	amsys->purge_count,PLTEXT_S(amsys->purge_count),amsys->purge_skip,amsys->users_purged,PLTEXT_WAS(amsys->users_purged));
/* just make sure that the count is all ok */
count_users();
return 1;
}


/*** Display colours to user ***/
void display_colour(UR_OBJECT user)
{
	set_crash();
	if (user->room==NULL) {
		write_user(user, "Sorrac, nemozes si zobrazit test\n");
		prompt(user);
		return;
		}
	write_user(user, "^~RS: ~OL~LI~FT~BY~RSreset\n");
	write_user(user, "^~OL: ~OLbold\n");
	write_user(user, "^~UL: ~ULunderline\n");
	write_user(user, "^~LI: ~LIblink\n");
	write_user(user, "^~RV: ~RVreverse\n");
	write_user(user, "^~FK: ~FKblack\n");
	write_user(user, "^~FR: ~FRred\n");
	write_user(user, "^~FG: ~FGgreen\n");
	write_user(user, "^~FY: ~FYyellow\n");
	write_user(user, "^~FB: ~FBblue\n");
	write_user(user, "^~FM: ~FMmagenta\n");
	write_user(user, "^~FT: ~FTtyrquiose\n");
	write_user(user, "^~FW: ~FWwhite\n");
	write_user(user, "^~BK: ~BKblack\n");
	write_user(user, "^~BR: ~BRred\n");
	write_user(user, "^~BG: ~BGgreen\n");
	write_user(user, "^~BY: ~BYyellow\n");
	write_user(user, "^~BB: ~BBblue\n");
	write_user(user, "^~BM: ~BMmagenta\n");
	write_user(user, "^~BT: ~BTtyrquiose\n");
	write_user(user, "^~BW: ~BWwhite\n");
	write_user(user, "^~BP: ~BPbeep\n");
	write_user(user, "^~CK: ~CKblack\n");
	write_user(user, "^~CR: ~CRred\n");
	write_user(user, "^~CG: ~CGgreen\n");
	write_user(user, "^~CY: ~CYyellow\n");
	write_user(user, "^~CB: ~CBblue\n");
	write_user(user, "^~CM: ~CMmagenta\n");
	write_user(user, "^~CT: ~CTtyrquiose\n");
	write_user(user, "^~CW: ~CWwhite\n");
}

/*** checks a name to see if it's in the retire list ***/
int in_retire_list(char *name)
{
	char check[USER_NAME_LEN+1];
	FILE *fp;

	set_crash();
	if (!(fp=fopen(RETIRE_LIST,"r"))) return 0;

	name[0]=toupper(name[0]);
	fscanf(fp,"%s",check);
	while(!(feof(fp))) {
		check[0]=toupper(check[0]);
		if (!strcmp(name,check)) {
			fclose(fp); return 1;
			}
		fscanf(fp,"%s",check);
		}
	fclose(fp);
	return 0;
}


/*** adds a name to the retire list ***/
void add_retire_list(char *name)
{
	FILE *fp;

	set_crash();
	if ((fp=fopen(RETIRE_LIST,"a"))) {
		fprintf(fp,"%-*s : %s\n",USER_NAME_LEN,name,long_date(1));
		fclose(fp);
		}
}


/*** removes a user from the retired list ***/
void clean_retire_list(char *name)
{
	char line[82], check[USER_NAME_LEN];
	FILE *fpi,*fpo;
	int cnt=0;

	set_crash();
	if (!(fpi=fopen(RETIRE_LIST,"r"))) return;
	if (!(fpo=fopen("templist","w"))) { fclose(fpi);  return; }

	name[0]=toupper(name[0]);
	fgets(line,82,fpi);
	sscanf(line,"%s",check);
	while(!(feof(fpi))) {
		check[0]=toupper(check[0]);
		if (strcmp(name,check)) { fprintf(fpo,"%s",line); cnt++; }
		fgets(line,82,fpi);
		sscanf(line,"%s",check);
		}
	fclose(fpi);  fclose(fpo);
	unlink(RETIRE_LIST);
	rename("templist",RETIRE_LIST);
	if (!cnt) unlink(RETIRE_LIST);
	return;
}



/*** set command bans/unbans
     banned=0/1 - 0 is unban and 1 is ban
     set=0/1 - 0 is unset and 1 is set
     ***/
int set_xgcom(UR_OBJECT user, UR_OBJECT u, int id, int banned, int set)
{
int cnt,i;
FILE *fp;
char filename[500];

	set_crash();
/* if banning a command with .xcom */
if (banned) {
  /* if removing the command */
  if (!set) {
    for (i=0;i<MAX_XCOMS;++i) {
      if (u->xcoms[i]==id) {
	u->xcoms[i]=-1;
	goto XGCOM_SKIP;
        }
      } /* end for */
    write_user(user,"ERROR: Nemozem odbanovat tento prikaz.\n");
    return 0;
    } /* end if */
  /* if adding the command */
  for (i=0;i<MAX_XCOMS;++i) {
    if (u->xcoms[i]==-1) {
      u->xcoms[i]=id;
      goto XGCOM_SKIP;
      }
    } /* end for */
  vwrite_user(user,"%s ma maximum zabanovanych prikazov.\n",u->name);
  return 0;
  } /* end if */

/* if giving a command with .gcom */
/* if removing the command */
if (!set) {
  for (i=0;i<MAX_GCOMS;++i) {
    if (u->gcoms[i]==id) {
      u->gcoms[i]=-1;
      goto XGCOM_SKIP;
      }
    } /* end for */
  write_user(user,"ERROR: Nemozem odbanovat tento prikaz.\n");
  return 0;
  } /* end if */
/* if adding the command */
for (i=0;i<MAX_GCOMS;++i) {
  if (u->gcoms[i]==-1) {
    u->gcoms[i]=id;
    goto XGCOM_SKIP;
    }
  } /* end for */
vwrite_user(user,"%s ma maximalny pocet pridanych prikazov.\n",u->name);
return 0;
/* write out the commands to a file */
XGCOM_SKIP:
sprintf(filename,"%s/%s.C", USERCOMMANDS,u->name);
if (!(fp=fopen(filename,"w"))) {
  write_user(user,"ERROR: Unable to open the command list file.\n");
  write_syslog(ERRLOG,1,"Unable to open %s's command list in set_xgcom().\n",u->name);
  return 0;
  }
cnt=0;
for (i=0;i<MAX_XCOMS;++i) {
  if (u->xcoms[i]==-1) continue;
  fprintf(fp,"0 %d\n",u->xcoms[i]);
  cnt++;
  }
for (i=0;i<MAX_GCOMS;++i) {
  if (u->gcoms[i]==-1) continue;
  fprintf(fp,"1 %d\n",u->gcoms[i]);
  cnt++;
  }
fclose(fp);
if (!cnt) unlink(filename);
return 1;
}


/*** read any banned commands that a user may have ***/
int get_xgcoms(UR_OBJECT user)
{
	int i,type,tmp;
	FILE *fp;
	char filename[500];

	set_crash();
	sprintf(filename,"%s/%s.C", USERCOMMANDS,user->name);
	if (!(fp=fopen(filename,"r"))) return 0;
	i=0;
	fscanf(fp,"%d %d",&type,&tmp);
	while (!feof(fp)) {
		if (!type) user->xcoms[i]=tmp;
		else user->gcoms[i]=tmp;
		i++;
		fscanf(fp,"%d %d",&type,&tmp);
		}
	fclose(fp);
	return 1;
}



/*****************************************************************************
 Friend commands and their subsids
 *****************************************************************************/

/* Determine whether user has u listed on their friends list */
int user_is_friend(UR_OBJECT user, UR_OBJECT u)
{
	int i;

	set_crash();
	for (i=0;i<MAX_FRIENDS;++i)
		if (!strcmp(user->friend[i],u->name))
			return 1;
	return 0;
}


/* Alert anyone logged on who has user in their friends 
   list that the user has just loged on */ 
void alert_friends(UR_OBJECT user, int mode)
{
	UR_OBJECT u;

	set_crash();
	for (u=user_first;u!=NULL;u=u->next) {
		if (!u->alert || u->login) continue;
		if ((user_is_friend(u,user)) && user->vis) {
			if (mode)
				vwrite_user(u,"\n\07~FG~OL~LIHEY!~RS~OL~FG  tvoj%s kamos%s ~RS%s~FG~OL sa prilogoval%s\n",
					grm_gnd(3, user->gender), grm_gnd(2, user->gender), user->recap, grm_gnd(4, user->gender));
			else
				vwrite_user(u,"\n\07~FR~OL~LIHEY!~RS~OL~FR  tvoj%s kamos%s ~RS%s~FR~OL sa odlogoval%s\n",
					grm_gnd(3, user->gender), grm_gnd(2, user->gender), user->recap, grm_gnd(4, user->gender));
			}
		}
}


/* Read from the friends file into the user structure */
void get_friends(UR_OBJECT user)
{
	char filename[500],name[USER_NAME_LEN];
	FILE *fp;
	int i;

	set_crash();
	sprintf(filename,"%s/%s.F", USERFRIENDS,user->name);
	if (!(fp=fopen(filename,"r"))) return;
	i=0;
	fscanf(fp,"%s",name);
	while(!feof(fp)) {
		strcpy(user->friend[i],name);
		i++;
		fscanf(fp,"%s",name);
		}
	fclose(fp);
}


/*****************************************************************************
              based upon scalar date routines by Ray Gardner
                and CAL - a calendar for DOS by Bob Stout
 *****************************************************************************/

/* determine if year is a leap year */
int is_leap(unsigned yr)
{
	set_crash();
	return yr%400==0 || (yr%4==0 && yr%100!=0);
}


/* convert months to days */
unsigned months_to_days(unsigned mn)
{
	set_crash();
	return (mn*3057-3007)/100;
}


/* convert years to days */
long years_to_days(unsigned yr)
{
	set_crash();
	return yr*365L+yr/4-yr/100+yr/400;
}


/* convert a given date (y/m/d) to a scalar */
long ymd_to_scalar(unsigned yr, unsigned mo, unsigned dy)
{
	long scalar;

	set_crash();
	scalar=dy+months_to_days(mo);
	/* adjust if past February */
	if (mo>2) scalar-=is_leap(yr)?1:2;
	yr--;
	scalar+=years_to_days(yr);
	return scalar;
}


/* converts a scalar date to y/m/d */
void scalar_to_ymd(long scalar, unsigned *yr, unsigned *mo, unsigned *dy)
{
	unsigned n;

	set_crash();
	/* 146097 == years_to_days(400) */
	for (n=(unsigned)((scalar*400L)/146097);years_to_days(n)<scalar;) n++;
	*yr=n;
	n=(unsigned)(scalar-years_to_days(n-1));
	/* adjust if past February */
	if (n>59) {                       
		n+=2;
		if (is_leap(*yr)) n-=n>62?1:2;
		}
	/* inverse of months_to_days() */
	*mo=(n*100+3007)/3057;
	*dy=n-months_to_days(*mo);
}


/* determine if the y/m/d given is todays date */
int is_ymd_today(unsigned yr, unsigned mo, unsigned dy)
{
	set_crash();
	if (((int)yr==(int)tyear)
	    && ((int)mo==(int)tmonth+1)
	    && ((int)dy==(int)tmday))
		return 1;
	  return 0;
}


/*** check to see if a user has a reminder for a given date ***/
int has_reminder(UR_OBJECT user, int dd, int mm, int yy)
{
	int i,cnt;

	set_crash();
	for (i=0,cnt=0;i<MAX_REMINDERS;i++)
		if (user->reminder[i].day==dd && user->reminder[i].month==mm && user->reminder[i].year==yy) cnt++;
	return cnt;
}


/*** check to see if a user has a reminder for today ***/
int has_reminder_today(UR_OBJECT user)
{
	int i,d,m,y,cnt;

	set_crash();
	d=tmday;
	m=tmonth+1;
	y=tyear;

	for (i=0,cnt=0;i<MAX_REMINDERS;i++)
		if (user->reminder[i].day==d
		    && user->reminder[i].month==m
		    && user->reminder[i].year==y)
			cnt++;
	return cnt;
}


/*** cleans any reminders that have expired - no point keeping them! ***/
int remove_old_reminders(UR_OBJECT user)
{
	int d,m,y,tdate,odate,cnt_total,i;

	set_crash();
	d=tmday;
	m=tmonth+1;
	y=tyear;
	tdate=(int)ymd_to_scalar(y,m,d);
	cnt_total=0;
	for (i=0;i<MAX_REMINDERS;i++) {
		if (!user->reminder[i].msg[0]) continue;
		odate=(int)ymd_to_scalar(user->reminder[i].year,user->reminder[i].month,user->reminder[i].day);
		if (odate<tdate) {
			user->reminder[i].day=0;
			user->reminder[i].month=0;
			user->reminder[i].year=0;
			user->reminder[i].msg[0]='\0';
			cnt_total++;
			}
		}
	if (cnt_total) write_user_reminders(user);
	return cnt_total;
}



/*** read in the user's reminder file ***/
int read_user_reminders(UR_OBJECT user)
{
	FILE *fp;
	char filename[500],line[REMINDER_LEN+1];
	int ln,i;

	set_crash();
	ln=i=0;
	sprintf(filename,"%s/%s.REM", USERREMINDERS,user->name);
	if (!(fp=fopen(filename,"r"))) return 0;
	fscanf(fp,"%d %d %d %d\n",
		&user->reminder[i].day,&user->reminder[i].month,&user->reminder[i].year,&user->reminder[i].alert);
	ln=1;
	while (!(feof(fp))) {
		if (i>MAX_REMINDERS) break;
		if (ln) {
			fgets(line,REMINDER_LEN,fp);
			line[strlen(line)-1]=0;
			strcpy(user->reminder[i].msg,line);
			i++;
			ln=0;
			}
		else {
			fscanf(fp,"%d %d %d %d\n",
				&user->reminder[i].day,&user->reminder[i].month,&user->reminder[i].year,&user->reminder[i].alert);
			ln=1;
			}
		}
	fclose(fp);
	return i;
}


/*** store a user's reminders to their .R file ***/
int write_user_reminders(UR_OBJECT user)
{
	FILE *fp;
	char filename[500];
	int i,cnt;

	set_crash();
	sprintf(filename,"%s/%s.REM", USERREMINDERS,user->name);
	if (!(fp=fopen(filename,"w"))) {
		write_syslog(ERRLOG,1,"Could not open %s reminder file for writing in write_reminders()\n",user->name);
		return 0;
		}
	cnt=0;
	for (i=0;i<MAX_REMINDERS;i++) {
		if (!user->reminder[i].msg[0]) continue;
		fprintf(fp,"%d %d %d 0\n",
			user->reminder[i].day,user->reminder[i].month,user->reminder[i].year);
		fputs(user->reminder[i].msg,fp);
		fprintf(fp,"\n");
		++cnt;
		}
	fclose(fp);
	if (!cnt) unlink(filename);
	return 1;
}



/*****************************************************************************/

/* save and load personal room information for the user of name given.
   if store=0 then read info from file else store.
   */
int personal_room_store(char *name, int store, RM_OBJECT rm) {
FILE *fp;
char filename[500],line[TOPIC_LEN+1],c;
int i;

if (rm==NULL) return 0;
/* load the info */
if (!store) {
  strtolower(name);
  name[0]=toupper(name[0]);
  sprintf(filename,"%s/%s.R", USERROOMS,name);
  if (!(fp=fopen(filename,"r"))) {
    /* if can't open the file then just put in default attributes */
    rm->access=PERSONAL_UNLOCKED;
    strcpy(rm->desc, default_personal_room_desc);
    strcpy(rm->topic, default_personal_room_topic);
    return 0;
    }
  fscanf(fp,"%d\n",&rm->access);
  fgets(line,TOPIC_LEN+1,fp);
  i=0;
  c=getc(fp);
  while(!feof(fp)) {
    if (i==ROOM_DESC_LEN) {
      write_syslog(ERRLOG,1,"Description too long when reloading for room %s.\n",rm->name);
      break;
      } /* end if */
    rm->desc[i]=c;
    c=getc(fp);
    ++i;
    } /* end while */
  rm->desc[i]='\0';
  fclose(fp);
  line[strlen(line)-1]='\0';
  if (!strcmp(line,"#UNSET")) rm->topic[0]='\0';
  else strcpy(rm->topic,line);
  rm->link[0]=room_first;
  return 1;
  }
/* save info */
strtolower(name);  name[0]=toupper(name[0]);
sprintf(filename,"%s/%s.R", USERROOMS,name);
if (!(fp=fopen(filename,"w"))) return 0;
fprintf(fp,"%d\n",rm->access);
(!rm->topic[0]) ? fprintf(fp,"#UNSET\n") : fprintf(fp,"%s\n",rm->topic);
i=0;
while (i!=ROOM_DESC_LEN) {
  if (rm->desc[i]=='\0') break;
  putc(rm->desc[i++],fp);
  }
fclose(fp);
return 1;
}


/*** adds a name to the user's personal room key list ***/
int personal_key_add(UR_OBJECT user,char *name)
{
	FILE *fp;
	char filename[500];

	set_crash();
	sprintf(filename,"%s/%s.K", USERROOMS,user->name);
	if ((fp=fopen(filename,"a"))) {
		fprintf(fp,"%s\n",name);
		fclose(fp);
		return 1;
		}
	return 0;
}


/*** remove a name from the user's personal room key list ***/
int personal_key_remove(UR_OBJECT user,char *name)
{
	char filename[500], fname[500], line[USER_NAME_LEN+2];
	FILE *fpi,*fpo;
	int cnt=0;

	set_crash();
	sprintf(filename,"%s/%s.K", USERROOMS,user->name);
	if (!(fpi=fopen(filename,"r"))) return 0;
	sprintf(fname, "%s/tempkeylist", TEMPFILES);
	if (!(fpo=fopen(fname, "w"))) {
		fclose(fpi);
		return 0;
		}

	fscanf(fpi,"%s",line);
	while(!(feof(fpi))) {
		if (strcmp(name,line)) {
			fprintf(fpo,"%s\n",line);
			cnt++;
			}
		fscanf(fpi,"%s",line);
		}
	fclose(fpi);
	fclose(fpo);
	unlink(filename);
	rename(fname, filename);
	if (!cnt) unlink(filename);
	return 1;
}

/*****************************************************************************/
#endif /* star.c */
