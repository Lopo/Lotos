/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
 NETLINK FUNCTIONS - NETLINK FUNCTIONS - NETLINK FUNCTIONS - NETLINK FUNCTIONS
 *****************************************************************************/
/*****************************************************************************
                Funkcie Lotos v1.2.0 na medzitalkrove spojenie
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/
/*****************************************************************************
   POZOR !!! Tento subor sa vyuziva len ak su povolene NETLINKS
 *****************************************************************************/
#ifdef NETLINKS

#ifndef __NETLINKS_C__
#define __NETLINKS_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_nl.h"
#include "obj_sys.h"
#include "obj_syspp.h"
#include "netlinks.h"
#include "comvals.h"


/******************************************************************************
 Object functions relating to Netlinks
 *****************************************************************************/

/*** Construct link object ***/
NL_OBJECT create_netlink(void)
{
NL_OBJECT nl;

	set_crash();
if ((nl=(NL_OBJECT)malloc(sizeof(struct netlink_struct)))==NULL) {
  write_syslog(NETLOG,1,"NETLINK: Memory allocation failure in create_netlink().\n");
  return NULL;
  }
if (nl_first==NULL) { 
  nl_first=nl;  nl->prev=NULL;  nl->next=NULL;
  }
else {  
  nl_last->next=nl;  nl->next=NULL;  nl->prev=nl_last;
  }
nl_last=nl;
nl->service[0]='\0';
nl->site[0]='\0';
nl->verification[0]='\0';
nl->mail_to[0]='\0';
nl->mail_from[0]='\0';
nl->mailfile=NULL;
nl->buffer[0]='\0';
nl->ver_major=0;
nl->ver_minor=0;
nl->ver_patch=0;
nl->keepalive_cnt=0;
nl->last_recvd=0;
nl->port=0;
nl->socket=0;
nl->mesg_user=NULL;
nl->connect_room=NULL;
nl->type=UNCONNECTED;
nl->stage=DOWN;
nl->connected=0;
nl->lastcom=-1;
nl->allow=ALL;
nl->warned=0;
return nl;
}


/*** Destruct a netlink (usually a closed incoming one). ***/
void destruct_netlink(NL_OBJECT nl)
{
	set_crash();
if (nl!=nl_first) {
  nl->prev->next=nl->next;
  if (nl!=nl_last) nl->next->prev=nl->prev;
  else { nl_last=nl->prev; nl_last->next=NULL; }
  }
else {
  nl_first=nl->next;
  if (nl!=nl_last) nl_first->prev=NULL;
  else nl_last=NULL; 
  }
free(nl);
}


/******************************************************************************
 Connection functions - making a link to external talkers
 *****************************************************************************/


/*** Initialise connections to remote servers. Basically this tries to connect
  to the services listed in the config file and it puts the open sockets in 
  the NL_OBJECT linked list which the talker then uses ***/
void init_connections(void)
{
NL_OBJECT nl;
RM_OBJECT rm;
int ret,cnt=0;

	set_crash();
printf("Connecting to remote servers...\n");
errno=0;
for(rm=room_first;rm!=NULL;rm=rm->next) {
  if ((nl=rm->netlink)==NULL) continue;
  ++cnt;
  printf("  Trying service %s at %s %d: ",nl->service,nl->site,nl->port);
  fflush(stdout);
  if ((ret=connect_to_site(nl))) {
    if (ret==1) {
      printf("%s.\n",strerror(errno));
      write_syslog(NETLOG,1,"NETLINK: Failed to connect to %s: %s.\n",nl->service,strerror(errno));
      }
    else {
      printf("Unknown hostname.\n");
      write_syslog(NETLOG,1,"NETLINK: Failed to connect to %s: Unknown hostname.\n",nl->service);
      }
    }
  else {
    printf("CONNECTED.\n");
    write_syslog(NETLOG,1,"NETLINK: Connected to %s (%s %d).\n",nl->service,nl->site,nl->port);
    nl->connect_room=rm;
    }
  }
if (cnt) printf("  See netlinks log for any further information.\n");
else printf("  No remote connections configured.\n");
}


/*** Do the actual connection ***/
int connect_to_site(NL_OBJECT nl)
{
struct sockaddr_in con_addr;
struct hostent *he;
int inetnum;
char *sn;

	set_crash();
sn=nl->site;
/* See if number address */
while(*sn && (*sn=='.' || isdigit(*sn))) sn++;
/* Name address given */
if(*sn) {
  if(!(he=gethostbyname(nl->site))) return 2;
  memcpy((char *)&con_addr.sin_addr,he->h_addr,(size_t)he->h_length);
  }
/* Number address given */
else {
  if((inetnum=inet_addr(nl->site))==-1) return 1;
  memcpy((char *)&con_addr.sin_addr,(char *)&inetnum,(size_t)sizeof(inetnum));
  }
/* Set up stuff and disable interrupts */
if ((nl->socket=socket(AF_INET,SOCK_STREAM,0))==-1) return 1;
con_addr.sin_family=AF_INET;
con_addr.sin_port=htons(nl->port);
signal(SIGALRM,SIG_IGN);
/* Attempt the connect. This is where the talker may hang. */
if (connect(nl->socket,(struct sockaddr *)&con_addr,sizeof(con_addr))==-1) {
  reset_alarm();
  return 1;
  }
reset_alarm();
nl->type=OUTGOING;
nl->stage=VERIFYING;
nl->last_recvd=time(0);
return 0;
}


/******************************************************************************
 Automatic event functions that relate to the Netlinks
 *****************************************************************************/


/*** See if any net connections are dragging their feet. If they have been idle
     longer than net_idle_time the drop them. Also send keepalive signals down
     links, this saves having another function and loop to do it. ***/
void check_nethangs_send_keepalives(void)
{
NL_OBJECT nl;
int secs;

	set_crash();
for(nl=nl_first;nl!=NULL;nl=nl->next) {
  if (nl->type==UNCONNECTED) {
    nl->warned=0;  continue;
    }  
  /* Send keepalives */
  nl->keepalive_cnt+=amsys->heartbeat;
  if (nl->keepalive_cnt>=amsys->keepalive_interval) {
    write_sock(nl->socket,"KA\n");
    nl->keepalive_cnt=0;
    }
  /* Check time outs */
  secs=(int)(time(0) - nl->last_recvd);
  if (nl->warned) {
    if (secs<amsys->net_idle_time-60) nl->warned=0;
    else {
      if (secs<amsys->net_idle_time) continue;
      sprintf(text,"~OLSYSTEM:~RS Disconnecting hung netlink to %s in the %s.\n",nl->service,nl->connect_room->name);
      write_room(NULL,text);
      shutdown_netlink(nl);
      nl->warned=0;
      }
    continue;
    }
  if (secs>amsys->net_idle_time-60) {
    sprintf(text,"~OLSYSTEM:~RS Netlink to %s in the %s has been hung for %d seconds.\n",nl->service,nl->connect_room->name,secs);
    write_level(ARCH,1,text,NULL);
    nl->warned=1;
    }
  }
destructed=0;
}


/******************************************************************************
 NUTS Netlink protocols and functions
 *****************************************************************************/


/*** Accept incoming server connection ***/
void accept_server_connection(int sock, struct sockaddr_in acc_addr)
{
NL_OBJECT nl,nl2;
RM_OBJECT rm;
char site[81];

	set_crash();
/* Send server type id and version number */
sprintf(text,"NUTS %s\n",NUTSVER);
write_sock(sock,text);
strcpy(site,(char *)inet_ntoa(acc_addr.sin_addr));
write_syslog(NETLOG,1,"NETLINK: Received request connection from site %s.\n",site);

/* See if legal site, ie site is in config sites list. */
for(nl2=nl_first;nl2!=NULL;nl2=nl2->next) if (!strcmp(nl2->site,site)) goto OK;
write_sock(sock,"DENIED CONNECT 1\n");
close(sock);
write_syslog(NETLOG,1,"NETLINK: Request denied, remote site not in valid sites list.\n");
return;

/* Find free room link */
OK:
for(rm=room_first;rm!=NULL;rm=rm->next) {
  if (rm->netlink==NULL && rm->inlink) {
    if ((nl=create_netlink())==NULL) {
      write_sock(sock,"DENIED CONNECT 2\n");  
      close(sock);  
      write_syslog(NETLOG,1,"NETLINK: Request denied, unable to create netlink object.\n");
      return;
      }
    rm->netlink=nl;
    nl->socket=sock;
    nl->type=INCOMING;
    nl->stage=VERIFYING;
    nl->connect_room=rm;
    nl->allow=nl2->allow;
    nl->last_recvd=time(0);
    strcpy(nl->service,"<verifying>");
    strcpy(nl->site,site);
    write_sock(sock,"GRANTED CONNECT\n");
    write_syslog(NETLOG,1,"NETLINK: Request granted.\n");
    return;
    }
  }
write_sock(sock,"DENIED CONNECT 3\n");
close(sock);
write_syslog(NETLOG,1,"NETLINK: Request denied, no free room links.\n");
}
		

/*** Deal with netlink data on link nl ***/
void exec_netcom(NL_OBJECT nl, char *inpstr)
{
int netcom_num,lev;
char w1[ARR_SIZE],w2[ARR_SIZE],w3[ARR_SIZE],*c,ctemp;

/* The most used commands have been truncated to save bandwidth, ie ACT is
   short for action, EMSG is short for end message. Commands that don't get
   used much ie VERIFICATION have been left long for readability. */
char *netcom[]={
  "DISCONNECT","TRANS","REL","ACT","GRANTED",
  "DENIED","MSG","EMSG","PRM","VERIFICATION",
  "VERIFY","REMVD","ERROR","EXISTS?","EXISTS_NO",
  "EXISTS_YES","MAIL","ENDMAIL","MAILERROR","KA",
  "RSTAT","*"
  };

	set_crash();
/* The buffer is large (ARR_SIZE*2) but if a bug occurs with a remote system
   and no newlines are sent for some reason it may overflow and this will 
   probably cause a crash. Oh well, such is life. */
if (nl->buffer[0]) {
  strcat(nl->buffer,inpstr);  inpstr=nl->buffer;
  }
nl->last_recvd=time(0);

/* Go through data */
while(*inpstr) {
  w1[0]='\0';  w2[0]='\0';  w3[0]='\0';  lev=0;
  if (*inpstr!='\n') sscanf(inpstr,"%s %s %s %d",w1,w2,w3,&lev);
  /* Find first newline */
  c=inpstr;  ctemp=1; /* hopefully we'll never get char 1 in the string */
  while(*c) {
    if (*c=='\n') {  ctemp=*(c+1); *(c+1)='\0';  break; }
    ++c;
    }
  /* If no newline then input is incomplete, store and return */
  if (ctemp==1) {  
    if (inpstr!=nl->buffer) strcpy(nl->buffer,inpstr);  
    return;  
    }
  /* Get command number */
  netcom_num=0;
  while(netcom[netcom_num][0]!='*') {
    if (!strcmp(netcom[netcom_num],w1))  break;
    netcom_num++;
    }
  /* Deal with initial connects */
  if (nl->stage==VERIFYING) {
    if (nl->type==OUTGOING) {
      if (strcmp(w1,"NUTS")) {
	write_syslog(NETLOG,1,"NETLINK: Incorrect connect message from %s.\n",nl->service);
	shutdown_netlink(nl);
	return;
        }	
      /* Store remote version for compat checks */
      nl->stage=UP;
      w2[10]='\0'; 
      sscanf(w2,"%d.%d.%d",&nl->ver_major,&nl->ver_minor,&nl->ver_patch);
      goto NEXT_LINE;
      }
    else {
      /* Incoming */
      if (netcom_num!=9) {
	/* No verification, no connection */
	write_syslog(NETLOG,1,"NETLINK: No verification sent by site %s.\n",nl->site);
	shutdown_netlink(nl);  
	return;
        }
      nl->stage=UP;
      }
    }
  /* If remote is currently sending a message relay it to user, don't
     interpret it unless its EMSG or ERROR */
  if (nl->mesg_user!=NULL && netcom_num!=7 && netcom_num!=12) {
    /* If -1 then user logged off before end of mesg received */
    if (nl->mesg_user!=(UR_OBJECT)-1) write_user(nl->mesg_user,inpstr);   
    goto NEXT_LINE;
    }
  /* Same goes for mail except its ENDMAIL or ERROR */
  if (nl->mailfile!=NULL && netcom_num!=17) {
    fputs(inpstr,nl->mailfile);  goto NEXT_LINE;
    }
  nl->lastcom=netcom_num;
  switch(netcom_num) {
    case  0: 
      if (nl->stage==UP) {
	sprintf(text,"~OLSYSTEM:~FY~RS Disconnecting from service %s in the %s.\n",nl->service,nl->connect_room->name);
	write_room(NULL,text);
        }
      shutdown_netlink(nl);  
      break;
    case  1: nl_transfer(nl,w2,w3,lev,inpstr);  break;
    case  2: nl_release(nl,w2);  break;
    case  3: nl_action(nl,w2,inpstr);  break;
    case  4: nl_granted(nl,w2);  break;
    case  5: nl_denied(nl,w2,inpstr);  break;
    case  6: nl_mesg(nl,w2); break;
    case  7: nl->mesg_user=NULL;  break;
    case  8: nl_prompt(nl,w2);  break;
    case  9: nl_verification(nl,w2,w3,0);  break;
    case 10: nl_verification(nl,w2,w3,1);  break;
    case 11: nl_removed(nl,w2);  break;
    case 12: nl_error(nl);  break;
    case 13: nl_checkexist(nl,w2,w3);  break;
    case 14: nl_user_notexist(nl,w2,w3);  break;
    case 15: nl_user_exist(nl,w2,w3);  break;
    case 16: nl_mail(nl,w2,w3);  break;
    case 17: nl_endmail(nl);  break;
    case 18: nl_mailerror(nl,w2,w3);  break;
    case 19: /* Keepalive signal, do nothing */ break;
    case 20: nl_rstat(nl,w2);  break;
    default: 
    write_syslog(NETLOG,1,"NETLINK: Received unknown command '%s' from %s.\n",w1,nl->service);
    write_sock(nl->socket,"ERROR\n"); 
    }
NEXT_LINE:
  /* See if link has closed */
  if (nl->type==UNCONNECTED) return;
  *(c+1)=ctemp;
  inpstr=c+1;
  }
nl->buffer[0]='\0';
}


/*** Deal with user being transfered over from remote site ***/
void nl_transfer(NL_OBJECT nl, char *name, char *pass, int lev, char *inpstr)
{
UR_OBJECT u;

	set_crash();
/* link for outgoing users only */
if (nl->allow==OUT) {
  sprintf(text,"DENIED %s 4\n",name);
  write_sock(nl->socket,text);
  return;
  }
if (strlen(name)>USER_NAME_LEN) name[USER_NAME_LEN]='\0';

/* See if user is banned */
if (user_banned(name)) {
  if (nl->ver_major==3 && nl->ver_minor>=3 && nl->ver_patch>=3) 
    sprintf(text,"DENIED %s 9\n",name); /* new error for 3.3.3 */
  else sprintf(text,"DENIED %s 6\n",name); /* old error to old versions */
  write_sock(nl->socket,text);
  return;
  }

/* See if user already on here */
if ((u=get_user(name))!=NULL) {
  sprintf(text,"DENIED %s 5\n",name);
  write_sock(nl->socket,text);
  return;
  }

/* See if user of this name exists on this system by trying to load up
   datafile */
if ((u=create_user())==NULL) {		
  sprintf(text,"DENIED %s 6\n",name);
  write_sock(nl->socket,text);
  return;
  }
u->type=REMOTE_TYPE;
strcpy(u->name,name);
if (load_user_details(u)) {
  if (strcmp(u->pass,pass)) {
    /* Incorrect password sent */
    sprintf(text,"DENIED %s 7\n",name);
    write_sock(nl->socket,text);
    destruct_user(u);
    destructed=0;
    return;
    }
  }
else {
  /* Get the users description */
  if (nl->ver_major<=3 && nl->ver_minor<=3 && nl->ver_patch<1) 
    strcpy(text,remove_first(remove_first(remove_first(inpstr))));
  else strcpy(text,remove_first(remove_first(remove_first(remove_first(inpstr)))));
  text[USER_DESC_LEN]='\0';
  terminate(text);
  strcpy(u->desc,text);
  strcpy(u->in_phrase,"enters");
  strcpy(u->out_phrase,"goes");
  strcpy(u->recap,u->name);
  strcpy(u->last_site,"[remote]");
  if (nl->ver_major==3 && nl->ver_minor>=3 && nl->ver_patch>=1) {
    if (lev>amsys->rem_user_maxlevel) u->level=amsys->rem_user_maxlevel;
    else u->level=lev; 
    }
  else u->level=amsys->rem_user_deflevel;
  u->unarrest=u->level;
  }
/* See if users level is below minlogin level */
if (u->level<amsys->minlogin_level) {
  if (nl->ver_major==3 && nl->ver_minor>=3 && nl->ver_patch>=3) 
    sprintf(text,"DENIED %s 8\n",u->name); /* new error for 3.3.3 */
  else sprintf(text,"DENIED %s 6\n",u->name); /* old error to old versions */
  write_sock(nl->socket,text);
  destruct_user(u);
  destructed=1;
  return;
  }
strcpy(u->site,nl->service);
sprintf(text,"%s enters from cyberspace.\n",u->name);
write_room(nl->connect_room,text);
write_syslog(NETLOG,1,"NETLINK: Remote user %s received from %s.\n",u->name,nl->service);
u->room=nl->connect_room;
strcpy(u->logout_room,u->room->name);
u->netlink=nl;
u->read_mail=time(0);
u->last_login=time(0);
syspp->acounter[3]++;
syspp->tcounter[u->gender]++;
syspp->bcounter[u->gender]++;
u->tcount=++syspp->tcounter[3];
u->bcount=++syspp->bcounter[3];
syspp->acounter[u->gender]++;
if (syspp->acounter[u->gender]>syspp->mcounter[u->gender]) syspp->mcounter[u->gender]++;
if (syspp->acounter[3]>syspp->mcounter[3]) syspp->mcounter[3]++;
save_counters();
sprintf(text,"GRANTED %s\n",name);
write_sock(nl->socket,text);
}


/*** User is leaving this system ***/
void nl_release(NL_OBJECT nl, char *name)
{
UR_OBJECT u;

	set_crash();
if ((u=get_user(name))!=NULL && u->type==REMOTE_TYPE) {
  sprintf(text,"%s leaves this plain of existence.\n",u->name);
  write_room_except(u->room,text,u);
  write_syslog(NETLOG,1,"NETLINK: Remote user %s released.\n",u->name);
  destroy_user_clones(u);
  syspp->acounter[u->gender]--;
  syspp->acounter[3]--;
  destruct_user(u);
  return;
  }
write_syslog(NETLOG,1,"NETLINK: Release requested for unknown/invalid user %s from %s.\n",name,nl->service);
}


/*** Remote user performs an action on this system ***/
void nl_action(NL_OBJECT nl, char *name, char *inpstr)
{
UR_OBJECT u;
char *c,ctemp;

	set_crash();
if (!(u=get_user(name))) {
  sprintf(text,"DENIED %s 8\n",name);
  write_sock(nl->socket,text);
  return;
  }
if (u->socket!=-1) {
  write_syslog(NETLOG,1,"NETLINK: Action requested for local user %s from %s.\n",name,nl->service);
  return;
  }
inpstr=remove_first(remove_first(inpstr));
/* remove newline character */
c=inpstr; ctemp='\0';
while(*c) {
  if (*c=='\n') {  ctemp=*c;  *c='\0';  break;  }
  ++c;
  }
u->last_input=time(0);
if (u->misc_op) {
  if (!strcmp(inpstr,"NL")) misc_ops(u,"\n");  
  else misc_ops(u,inpstr+4);
  return;
  }
if (u->afk) {
  write_user(u,"You are no longer AFK.\n");  
  if (u->vis) {
    sprintf(text,"%s comes back from being AFK.\n",u->name);
    write_room_except(u->room,text,u);
    }
  u->afk=0;
  }
word_count=wordfind(inpstr);
if (!strcmp(inpstr,"NL")) return; 
exec_com(u,inpstr);
if (ctemp) *c=ctemp;
if (!u->misc_op) prompt(u);
}


/*** Grant received from remote system ***/
void nl_granted(NL_OBJECT nl, char *name)
{
UR_OBJECT u;
RM_OBJECT old_room;

	set_crash();
if (!strcmp(name,"CONNECT")) {
  write_syslog(NETLOG,1,"NETLINK: Connection to %s granted.\n",nl->service);
  /* Send our verification and version number */
  sprintf(text,"VERIFICATION %s %s\n",verification,NUTSVER);
  write_sock(nl->socket,text);
  return;
  }
if (!(u=get_user(name))) {
  write_syslog(NETLOG,1,"NETLINK: Grant received for unknown user %s from %s.\n",name,nl->service);
  return;
  }
/* This will probably occur if a user tried to go to the other site , got 
   lagged then changed his mind and went elsewhere. Don't worry about it. */
if (u->remote_com!=GO) {
  write_syslog(NETLOG,1,"NETLINK: Unexpected grant for %s received from %s.\n",name,nl->service);
  return;
  }
/* User has been granted permission to move into remote talker */
write_user(u,"~FB~OLYou traverse cyberspace...\n");
if (u->vis) {
  sprintf(text,"%s %s to the %s.\n",u->name,u->out_phrase,nl->service);
  write_room_except(u->room,text,u);
  }
else write_room_except(u->room,invisleave,u);
write_syslog(NETLOG,1,"NETLINK: %s transfered to %s.\n",u->name,nl->service);
old_room=u->room;
u->room=NULL; /* Means on remote talker */
u->netlink=nl;
u->pot_netlink=NULL;
u->remote_com=-1;
u->misc_op=0;
u->status='M';
u->filepos=0;
u->page_file[0]='\0';
reset_access(old_room);
sprintf(text,"ACT %s look\n",u->name);
write_sock(nl->socket,text);
}


/*** Deny received from remote system ***/
void nl_denied(NL_OBJECT nl, char *name, char *inpstr)
{
UR_OBJECT u;
int errnum;
char *neterr[]={
"this site is not in the remote services valid sites list",
"the remote service is unable to create a link",
"the remote service has no free room links",
"the link is for incoming users only",
"a user with your name is already logged on the remote site",
"the remote service was unable to create a session for you",
"incorrect password. Use '.go <service> <remote password>'",
"your level there is below the remote services current minlogin level",
"you are banned from that service"
};

	set_crash();
errnum=0;
sscanf(remove_first(remove_first(inpstr)),"%d",&errnum);
if (!strcmp(name,"CONNECT")) {
  write_syslog(NETLOG,1,"NETLINK: Connection to %s denied, %s.\n",nl->service,neterr[errnum-1]);
  /* If wiz initiated connect let them know its failed */
  sprintf(text,"~OLSYSTEM:~RS Connection to %s failed, %s.\n",nl->service,neterr[errnum-1]);
  write_level(command_table[CONN].level,1,text,NULL);
  close(nl->socket);
  nl->type=UNCONNECTED;
  nl->stage=DOWN;
  return;
  }
/* Is for a user */
if (!(u=get_user(name))) {
  write_syslog(NETLOG,1,"NETLINK: Deny for unknown user %s received from %s.\n",name,nl->service);
  return;
  }
write_syslog(NETLOG,1,"NETLINK: Deny %d for user %s received from %s.\n",errnum,name,nl->service);
sprintf(text,"Sorry, %s.\n",neterr[errnum-1]);
write_user(u,text);
prompt(u);
u->remote_com=-1;
u->pot_netlink=NULL;
}


/*** Text received to display to a user on here ***/
void nl_mesg(NL_OBJECT nl, char *name)
{
UR_OBJECT u;

	set_crash();
if (!(u=get_user(name))) {
  write_syslog(NETLOG,1,"NETLINK: Message received for unknown user %s from %s.\n",name,nl->service);
  nl->mesg_user=(UR_OBJECT)-1;
  return;
  }
nl->mesg_user=u;
}


/*** Remote system asking for prompt to be displayed ***/
void nl_prompt(NL_OBJECT nl, char *name)
{
UR_OBJECT u;

	set_crash();
if (!(u=get_user(name))) {
  write_syslog(NETLOG,1,"NETLINK: Prompt received for unknown user %s from %s.\n",name,nl->service);
  return;
  }
if (u->type==REMOTE_TYPE) {
  write_syslog(NETLOG,1,"NETLINK: Prompt received for remote user %s from %s.\n",name,nl->service);
  return;
  }
prompt(u);
}


/*** Verification received from remote site ***/
void nl_verification(NL_OBJECT nl, char *w2, char *w3, int com)
{
NL_OBJECT nl2;

	set_crash();
if (!com) {
  /* We're verifiying a remote site */
  if (!w2[0]) {
    shutdown_netlink(nl);  return;
    }
  for(nl2=nl_first;nl2!=NULL;nl2=nl2->next) {
    if (!strcmp(nl->site,nl2->site) && !strcmp(w2,nl2->verification)) {
      switch(nl->allow) {
        case IN : write_sock(nl->socket,"VERIFY OK IN\n");  break;
        case OUT: write_sock(nl->socket,"VERIFY OK OUT\n");  break;
        case ALL: write_sock(nl->socket,"VERIFY OK ALL\n"); 
        }
      strcpy(nl->service,nl2->service);
      /* Only 3.2.0 and above send version number with verification */
      sscanf(w3,"%d.%d.%d",&nl->ver_major,&nl->ver_minor,&nl->ver_patch);
      write_syslog(NETLOG,1,"NETLINK: Connected to %s in the %s.\n",nl->service,nl->connect_room->name);
      sprintf(text,"~OLSYSTEM:~RS New connection to service %s in the %s.\n",nl->service,nl->connect_room->name);
      write_room(NULL,text);
      return;
      }
    }
  write_sock(nl->socket,"VERIFY BAD\n");
  shutdown_netlink(nl);
  return;
  }
/* The remote site has verified us */
if (!strcmp(w2,"OK")) {
  /* Set link permissions */
  if (!strcmp(w3,"OUT")) {
    if (nl->allow==OUT) write_syslog(NETLOG,1,"NETLINK: WARNING - Permissions deadlock, both sides are outgoing only.\n");
    else nl->allow=IN;
    }
  else {
    if (!strcmp(w3,"IN")) {
      if (nl->allow==IN) write_syslog(NETLOG,1,"NETLINK: WARNING - Permissions deadlock, both sides are incoming only.\n");
      else nl->allow=OUT;
      }
    }
  write_syslog(NETLOG,1,"NETLINK: Connection to %s verified.\n",nl->service);
  sprintf(text,"~OLSYSTEM:~RS New connection to service %s in the %s.\n",nl->service,nl->connect_room->name);
  write_room(NULL,text);
  return;
  }
if (!strcmp(w2,"BAD")) {
  write_syslog(NETLOG,1,"NETLINK: Connection to %s has bad verification.\n",nl->service);
  /* Let wizes know its failed , may be wiz initiated */
  sprintf(text,"~OLSYSTEM:~RS Connection to %s failed, bad verification.\n",nl->service);
  write_level(command_table[CONN].level,1,text,NULL);
  shutdown_netlink(nl);  
  return;
  }
write_syslog(NETLOG,1,"NETLINK: Unknown verify return code from %s.\n",nl->service);
shutdown_netlink(nl);
}


/* Remote site only sends REMVD (removed) notification if user on remote site 
   tries to .go back to his home site or user is booted off. Home site doesn't
   bother sending reply since remote site will remove user no matter what. */
void nl_removed(NL_OBJECT nl, char *name)
{
UR_OBJECT u;

	set_crash();
if (!(u=get_user(name))) {
  write_syslog(NETLOG,1,"NETLINK: Removed notification for unknown user %s received from %s.\n",name,nl->service);
  return;
  }
if (u->room!=NULL) {
  write_syslog(NETLOG,1,"NETLINK: Removed notification of local user %s received from %s.\n",name,nl->service);
  return;
  }
write_syslog(NETLOG,1,"NETLINK: %s returned from %s.\n",u->name,u->netlink->service);
u->room=u->netlink->connect_room;
u->netlink=NULL;
if (u->vis) {
  sprintf(text,"%s %s\n",u->name,u->in_phrase);
  write_room_except(u->room,text,u);
  }
else write_room_except(u->room,invisenter,u);
look(u);
prompt(u);
}


/*** Got an error back from site, deal with it ***/
void nl_error(NL_OBJECT nl)
{
	set_crash();
if (nl->mesg_user!=NULL) nl->mesg_user=NULL;
/* lastcom value may be misleading, the talker may have sent off a whole load
   of commands before it gets a response due to lag, any one of them could
   have caused the error */
write_syslog(NETLOG,1,"NETLINK: Received ERROR from %s, lastcom = %d.\n",nl->service,nl->lastcom);
}


/*** Does user exist? This is a question sent by a remote mailer to
     verifiy mail id's. ***/
void nl_checkexist(NL_OBJECT nl, char *to, char *from)
{
int ld;

	set_crash();
ld=0;
if (!(find_user_listed(to))) {
  sprintf(text,"EXISTS_NO %s %s\n",to,from);
  write_sock(nl->socket,text);
  return;
  }
sprintf(text,"EXISTS_YES %s %s\n",to,from);
write_sock(nl->socket,text);
}


/*** Remote user doesnt exist ***/
void nl_user_notexist(NL_OBJECT nl, char *to, char *from)
{
UR_OBJECT user;
char filename[80];
char text2[ARR_SIZE];

	set_crash();
if ((user=get_user(from))!=NULL) {
  sprintf(text,"~OLSYSTEM:~RS User %s does not exist at %s, your mail bounced.\n",to,nl->service);
  write_user(user,text);
  }
else {
  sprintf(text2,"There is no user named %s at %s, your mail bounced.\n",to,nl->service);
  send_mail(NULL,from,text2,0);
  }
sprintf(filename,"%s/OUT_%s_%s@%s",MAILSPOOL,from,to,nl->service);
unlink(filename);
}


/*** Remote users exists, send him some mail ***/
void nl_user_exist(NL_OBJECT nl, char *to, char *from)
{
UR_OBJECT user;
FILE *fp;
char text2[ARR_SIZE],filename[500],line[82];

	set_crash();
sprintf(filename,"%s/OUT_%s_%s@%s",MAILSPOOL,from,to,nl->service);
if (!(fp=fopen(filename,"r"))) {
  if ((user=get_user(from))!=NULL) {
    sprintf(text,"~OLSYSTEM:~RS An error occured during mail delivery to %s@%s.\n",to,nl->service);
    write_user(user,text);
    }
  else {
    sprintf(text2,"An error occured during mail delivery to %s@%s.\n",to,nl->service);
    send_mail(NULL,from,text2,0);
    }
  return;
  }
sprintf(text,"MAIL %s %s\n",to,from);
write_sock(nl->socket,text);
fgets(line,80,fp);
while(!feof(fp)) {
  write_sock(nl->socket,line);
  fgets(line,80,fp);
  }
fclose(fp);
write_sock(nl->socket,"\nENDMAIL\n");
unlink(filename);
}


/*** Got some mail coming in ***/
void nl_mail(NL_OBJECT nl, char *to, char *from)
{
char filename[500];

	set_crash();
write_syslog(NETLOG,1,"NETLINK: Mail received for %s from %s.\n",to,nl->service);
sprintf(filename,"%s/IN_%s_%s@%s",MAILSPOOL,to,from,nl->service);
if (!(nl->mailfile=fopen(filename,"w"))) {
  write_syslog(ERRLOG,1,"Couldn't open file %s to write in nl_mail().\n",filename);
  sprintf(text,"MAILERROR %s %s\n",to,from);
  write_sock(nl->socket,text);
  return;
  }
strcpy(nl->mail_to,to);
strcpy(nl->mail_from,from);
}


/*** End of mail message being sent from remote site ***/

void nl_endmail(NL_OBJECT nl)
{
FILE *infp,*outfp;
char c,infile[500],mailfile[500];
int amount,size,tmp1,tmp2;
struct stat stbuf;

	set_crash();
fclose(nl->mailfile);
nl->mailfile=NULL;
sprintf(mailfile,"%s/IN_%s_%s@%s",MAILSPOOL,nl->mail_to,nl->mail_from,nl->service);
/* Copy to users mail file to a tempfile */
if (!(outfp=fopen("tempfile","w"))) {
  write_syslog(ERRLOG,1,"Couldn't open tempfile in netlink_endmail().\n");
  sprintf(text,"MAILERROR %s %s\n",nl->mail_to,nl->mail_from);
  write_sock(nl->socket,text);
  goto END;
  }
/* Copy old mail file to tempfile */
sprintf(infile,"%s/%s.M",USERMAILS,nl->mail_to);
/* first get old file size if any new mail, and also new mail count */
amount=mail_sizes(nl->mail_to,1);
if (!amount) {
  if (stat(infile,&stbuf)==-1) size=0;
  else size=stbuf.st_size;
  }
else size=mail_sizes(nl->mail_to,2);
/* write size of file and amount of new mail */
fprintf(outfp,"%d %d\r",++amount,size);

if (!(infp=fopen(infile,"r"))) goto SKIP;
fscanf(infp,"%d %d\r",&tmp1,&tmp2);
c=getc(infp);
while(!feof(infp)) {  putc(c,outfp);  c=getc(infp);  }
fclose(infp);

/* Copy received file */
SKIP:
if (!(infp=fopen(mailfile,"r"))) {
  write_syslog(ERRLOG,1,"Couldn't open file %s to read in netlink_endmail().\n",mailfile);
  sprintf(text,"MAILERROR %s %s\n",nl->mail_to,nl->mail_from);
  write_sock(nl->socket,text);
  goto END;
  }
fprintf(outfp,"~OLFrom: %s@%s  %s",nl->mail_from,nl->service,long_date(0));
c=getc(infp);
while(!feof(infp)) {  putc(c,outfp);  c=getc(infp);  }
fclose(infp);
fclose(outfp);
rename("tempfile",infile);
write_user(get_user(nl->mail_to),"\07~FT~OL~LI** YOU HAVE NEW MAIL **\n");

END:
nl->mail_to[0]='\0';
nl->mail_from[0]='\0';
unlink(mailfile);
}


/*** An error occured at remote site ***/
void nl_mailerror(NL_OBJECT nl, char *to, char *from)
{
UR_OBJECT user;

	set_crash();
if ((user=get_user(from))!=NULL) {
  sprintf(text,"~OLSYSTEM:~RS An error occured during mail delivery to %s@%s.\n",to,nl->service);
  write_user(user,text);
  }
else {
  sprintf(text,"An error occured during mail delivery to %s@%s.\n",to,nl->service);
  send_mail(NULL,from,text,0);
  }
}


/*** Send statistics of this server to requesting user on remote site ***/
void nl_rstat(NL_OBJECT nl, char *to)
{
char str[80];

	set_crash();
gethostname(str,80);
if (nl->ver_major<=3 && nl->ver_minor<2) sprintf(text,"MSG %s\n\n*** Remote statistics ***\n\n",to);
else sprintf(text,"MSG %s\n\n~BB*** Remote statistics ***\n\n",to);
write_sock(nl->socket,text);
sprintf(text,"NUTS version         : %s\nHost                 : %s\n",NUTSVER,str);
write_sock(nl->socket,text);
sprintf(text,"Ports (Main/Wiz/Link): %d ,%d, %d\n",port[0],port[1],port[2]);
write_sock(nl->socket,text);
sprintf(text,"Number of users      : %ld\nRemote user maxlevel : %s\n",syspp->acounter[3],user_level[amsys->rem_user_maxlevel].name);
write_sock(nl->socket,text);
sprintf(text,"Remote user deflevel : %s\n\nEMSG\nPRM %s\n",user_level[amsys->rem_user_deflevel].name,to);
write_sock(nl->socket,text);
}


/*** Shutdown the netlink and pull any remote users back home ***/
void shutdown_netlink(NL_OBJECT nl)
{
UR_OBJECT u;
char mailfile[500];

	set_crash();
if (nl->type==UNCONNECTED) return;

/* See if any mail halfway through being sent */
if (nl->mail_to[0]) {
  sprintf(text,"MAILERROR %s %s\n",nl->mail_to,nl->mail_from);
  write_sock(nl->socket,text);
  fclose(nl->mailfile);
  sprintf(mailfile,"%s/IN_%s_%s@%s",MAILSPOOL,nl->mail_to,nl->mail_from,nl->service);
  unlink(mailfile);
  nl->mail_to[0]='\0';
  nl->mail_from[0]='\0';
  }
write_sock(nl->socket,"DISCONNECT\n");
close(nl->socket);  
for(u=user_first;u!=NULL;u=u->next) {
  if (u->pot_netlink==nl) {  u->remote_com=-1;  continue;  }
  if (u->netlink==nl) {
    if (u->room==NULL) {
      write_user(u,"~FB~OLYou feel yourself dragged back across the ether...\n");
      u->room=u->netlink->connect_room;
      u->netlink=NULL;
      if (u->vis) {
	sprintf(text,"%s %s\n",u->name,u->in_phrase);
	write_room_except(u->room,text,u);
        }
      else write_room_except(u->room,invisenter,u);
      look(u);  prompt(u);
      write_syslog(NETLOG,1,"NETLINK: %s recovered from %s.\n",u->name,nl->service);
      continue;
      }
    if (u->type==REMOTE_TYPE) {
      sprintf(text,"%s vanishes!\n",u->name);
      write_room(u->room,text);
      syspp->acounter[u->gender]--;
      syspp->acounter[3]--;
      destruct_user(u);
      }
    }
  }
if (nl->stage==UP) write_syslog(NETLOG,1,"NETLINK: Disconnected from %s.\n",nl->service);
else write_syslog(NETLOG,1,"NETLINK: Disconnected from site %s.\n",nl->site);
if (nl->type==INCOMING) {
  nl->connect_room->netlink=NULL;
  destruct_netlink(nl);  
  return;
  }
nl->type=UNCONNECTED;
nl->stage=DOWN;
nl->warned=0;
}


/******************************************************************************
 User executed commands that relate to the Netlinks
 *****************************************************************************/


/*** Return to home site ***/
void home(UR_OBJECT user)
{
	set_crash();
if (user->room!=NULL) {
  write_user(user,"You are already on your home system.\n");
  return;
  }
write_user(user,"~FB~OLYou traverse cyberspace...\n");
sprintf(text,"REL %s\n",user->name);
write_sock(user->netlink->socket,text);
write_syslog(NETLOG,1,"NETLINK: %s returned from %s.\n",user->name,user->netlink->service);
user->room=user->netlink->connect_room;
user->netlink=NULL;
if (user->vis) {
  sprintf(text,"%s %s\n",user->recap,user->in_phrase);
  write_room_except(user->room,text,user);
  }
else write_room_except(user->room,invisenter,user);
look(user);
}


/*** List defined netlinks and their status ***/
void netstat(UR_OBJECT user)
{
NL_OBJECT nl;
UR_OBJECT u;
char *allow[]={ "  ?","ALL"," IN","OUT" };
char *type[]={ "  -"," IN","OUT" };
char portstr[6],stat[9],vers[8];
int iu,ou,a;

	set_crash();
if (nl_first==NULL) {
  write_user(user,"No remote connections configured.\n");  return;
  }
write_user(user,"\n~BB*** Netlink data & status ***\n\n~FTService name    : Allow Type Status IU OU Version  Site\n\n");
for(nl=nl_first;nl!=NULL;nl=nl->next) {
  iu=0;  ou=0;
  if (nl->stage==UP) {
    for(u=user_first;u!=NULL;u=u->next) {
      if (u->netlink==nl) {
	if (u->type==REMOTE_TYPE)  ++iu;
	if (u->room==NULL) ++ou;
        }
      }
    }
  if (nl->port) sprintf(portstr,"%d",nl->port);  else portstr[0]='\0';
  if (nl->type==UNCONNECTED) {
    strcpy(stat,"~FRDOWN");  strcpy(vers,"-");
    }
  else {
    if (nl->stage==UP) strcpy(stat,"  ~FGUP");
    else strcpy(stat," ~FYVER");
    if (!nl->ver_major) strcpy(vers,"3.?.?"); /* Pre - 3.2 version */  
    else sprintf(vers,"%d.%d.%d",nl->ver_major,nl->ver_minor,nl->ver_patch);
    }
  /* If link is incoming and remoter vers < 3.2 we have no way of knowing 
     what the permissions on it are so set to blank */
  if (!nl->ver_major && nl->type==INCOMING && nl->allow!=IN) a=0; 
  else a=nl->allow+1;
  sprintf(text,"%-15s :   %s  %s   %s~RS %2d %2d %7s  %s %s\n",nl->service,allow[a],type[nl->type],stat,iu,ou,vers,nl->site,portstr);
  write_user(user,text);
  }
write_user(user,"\n");
}


/*** Show type of data being received down links (this is usefull when a
     link has hung) ***/
void netdata(UR_OBJECT user)
{
NL_OBJECT nl;
char from[80],name[USER_NAME_LEN+1];
int cnt;

	set_crash();
cnt=0;
write_user(user,"\n~BB*** Mail receiving status ***\n\n");
for(nl=nl_first;nl!=NULL;nl=nl->next) {
  if (nl->type==UNCONNECTED || nl->mailfile==NULL) continue;
  if (++cnt==1) write_user(user,"To              : From                       Last recv.\n\n");
  sprintf(from,"%s@%s",nl->mail_from,nl->service);
  sprintf(text,"%-15s : %-25s  %d seconds ago.\n",nl->mail_to,from,(int)(time(0)-nl->last_recvd));
  write_user(user,text);
  }
if (!cnt) write_user(user,"No mail being received.\n\n");
else write_user(user,"\n");
cnt=0;
write_user(user,"\n~BB*** Message receiving status ***\n\n");
for(nl=nl_first;nl!=NULL;nl=nl->next) {
  if (nl->type==UNCONNECTED || nl->mesg_user==NULL) continue;
  if (++cnt==1) write_user(user,"To              : From             Last recv.\n\n");
  if (nl->mesg_user==(UR_OBJECT)-1) strcpy(name,"<unknown>");
  else strcpy(name,nl->mesg_user->name);
  sprintf(text,"%-15s : %-15s  %d seconds ago.\n",name,nl->service,(int)(time(0)-nl->last_recvd));
  write_user(user,text);
  }
if (!cnt) write_user(user,"No messages being received.\n\n");
else write_user(user,"\n");
}


/*** Connect a netlink. Use the room as the key ***/
void connect_netlink(UR_OBJECT user)
{
RM_OBJECT rm;
NL_OBJECT nl;
int ret,tmperr;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <room service is linked to>", command_table[CONN].name);
  return;
  }
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
if ((nl=rm->netlink)==NULL) {
  write_user(user,"That room is not linked to a service.\n");
  return;
  }
if (nl->type!=UNCONNECTED) {
  write_user(user,"That rooms netlink is already up.\n");  return;
  }
write_user(user,"Attempting connect (this may cause a temporary hang)...\n");
write_syslog(NETLOG,1,"NETLINK: Connection attempt to %s initiated by %s.\n",nl->service,user->name);
errno=0;
if (!(ret=connect_to_site(nl))) {
  write_user(user,"~FGInitial connection made...\n");
  write_syslog(NETLOG,1,"NETLINK: Connected to %s (%s %d).\n",nl->service,nl->site,nl->port);
  nl->connect_room=rm;
  return;
  }
tmperr=errno; /* On Linux errno seems to be reset between here and sprintf */
write_user(user,"~FRConnect failed: ");
write_syslog(NETLOG,1,"NETLINK: Connection attempt failed: ");
if (ret==1) {
  vwrite_user(user,"%s.\n",strerror(errno));
  write_syslog(NETLOG,0,"%s.\n",strerror(errno));
  return;
  }
write_user(user,"Unknown hostname.\n");
write_syslog(NETLOG,0,"Unknown hostname.\n");
}


/*** Disconnect a link ***/
void disconnect_netlink(UR_OBJECT user)
{
RM_OBJECT rm;
NL_OBJECT nl;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <room service is linked to>", command_table[DISCONN].name);
  return;
  }
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
nl=rm->netlink;
if (nl==NULL) {
  write_user(user,"That room is not linked to a service.\n");
  return;
  }
if (nl->type==UNCONNECTED) {
  write_user(user,"That rooms netlink is not connected.\n");  return;
  }
/* If link has hung at verification stage don't bother announcing it */
if (nl->stage==UP) {
  sprintf(text,"~OLSYSTEM:~RS Disconnecting from %s in the %s.\n",nl->service,rm->name);
  write_room(NULL,text);
  write_syslog(NETLOG,1,"NETLINK: Link to %s in the %s disconnected by %s.\n",nl->service,rm->name,user->name);
  }
else write_syslog(NETLOG,1,"NETLINK: Link to %s disconnected by %s.\n",nl->service,user->name);
shutdown_netlink(nl);
write_user(user,"Disconnected.\n");
}


/*** Stat a remote system ***/
void remote_stat(UR_OBJECT user)
{
NL_OBJECT nl;
RM_OBJECT rm;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <room service is linked to>", command_table[RSTAT].name);
  return;
  }
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
if ((nl=rm->netlink)==NULL) {
  write_user(user,"That room is not linked to a service.\n");
  return;
  }
if (nl->stage!=2) {
  write_user(user,"Not (fully) connected to service.\n");
  return;
  }
if (nl->ver_major<=3 && nl->ver_minor<1) {
  write_user(user,"The NUTS version running that service does not support this facility.\n");
  return;
  }
sprintf(text,"RSTAT %s\n",user->name);
write_sock(nl->socket,text);
write_user(user,"Request sent.\n");
}

#endif /* netlinks.c */
#endif /* NETLINKS */
