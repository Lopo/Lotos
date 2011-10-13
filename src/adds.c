/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
          Funkcie pre Lotos v1.2.0 nezaradene do specifickej skupiny
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __ADDS_C__
#define __ADDS_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_tr.h"
#include "obj_pl.h"
#include "obj_sys.h"
#include "obj_syspp.h"
#include "adds.h"
#include "comvals.h"
#include "prototypes.h"


#ifdef DEBUG
void test(UR_OBJECT user, char *inpstr)
{
	set_crash();
	crash_dump();
}
#endif


/* Writes STR to users of LEVEL and higher who are in ROOM except for USER */
void write_duty(int level, char *str, RM_OBJECT room, UR_OBJECT user, int cr)
{
	UR_OBJECT u;

	set_crash();
	if (level<WIZ) level=WIZ;
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login
		    || u->type==CLONE_TYPE
		    || u->type==BOT_TYPE
		    || u->level<level
		    || u==user
		    || u->room==NULL
		    || (u->room!=room && room!=NULL)
		    || ((user!=NULL)
		        && (check_igusers(u,user)!=-1)
		        && (user->level>GOD)
		        )
		    || u->ignore.all
		    ) continue;
		write_user(u,str);
		}
}


/*** Write usage of a command ***/
void write_usage(UR_OBJECT user, char *str, ...)
{
	va_list args;

	set_crash();
	vtext[0]='\0';
	va_start(args, str);
	vsprintf(vtext, str, args);
	va_end(args);
	strcpy(text, vtext);
	vwrite_user(user, "~OL>Pouzitie: ~FT%s\n", text);
}


int osstar_load(void)
{
	int tmp=-1;

	set_crash();
if (init_ossmain()) tmp=0;  /* Initialize main system */
if (tmp==-1) {
	printf("Lotos:  Main system did not initialize in osstar_load()!!\n           BOOT ABORTED.\n\n");
	exit(0);
	}
load_plugins();
printf("Verifikujem pluginy ");
oss_versionVerify();
printf("System plugin registry initialized.\n");
return 1;
}

int init_ossmain(void)
{
	set_crash();
printf("\nVerifikujem systemove komponenty a premenne. . .\n");
/* check modular colorcode index checksum */
if ((CDEFAULT+CHIGHLIGHT+CTEXT+CBOLD+CSYSTEM+CSYSBOLD+CWARNING+CWHOUSER+CWHOINFO+
        CPEOPLEHI+CPEOPLE+CUSER+CSELF+CEMOTE+CSEMOTE+CPEMOTE+CTHINK+CTELLUSER+
        CTELLSELF+CTELL+CSHOUT+CMAILHEAD+CMAILDATE+CBOARDHEAD+CBOARDDATE)!=300)
        { printf("Lotos:  Chybny sucet modularnych indexov color-kodov\n"); return 0; }

/* check system registry index checksum */
if ((TALKERNAME+SERIALNUM+REGUSER+SERVERDNS+SERVERIP+TALKERMAIL+TALKERHTTP+SYSOPNAME+SYSOPUNAME+PUEBLOWEB+PUEBLOPIC)!=66)
        {printf("Lotos:  System information registry index checksum FAILED.\n"); return 0; }

/* check system registry master entry */
if (reg_sysinfo[0][0]!='*') { printf("Lotos:  System registry master entry (0) must be '*'.\n   -- Temporarily fixed.");
        reg_sysinfo[0][0]='*';  reg_sysinfo[0][1]='\0'; }

printf("Lotos verzia %s inicializovana.\n\n", OSSVERSION);
return 1;
}

void oss_versionVerify(void)
{
	PL_OBJECT plugin,p;
	CM_OBJECT com;

	set_crash();
	for (plugin=plugin_first; plugin!=NULL; plugin=p) {
		p=plugin->next;
		if (atoi(RUN_VER) < atoi(plugin->req_ver)) {
			printf("\nOSS: Plugin '%s' pozaduje vyssiu verziu Lotos-u.\n",plugin->name);
			write_syslog(SYSLOG, 0, "Lotos: Plugin '%s' pozaduje Lotos verzie %s.\n",plugin->name,plugin->req_ver);
			for (com=cmds_first; com!=NULL; com=com->next) if (com->plugin==plugin) destroy_pl_cmd(com);
			destroy_plugin(plugin);
			}
		printf(".");
		}
	printf("  OK\n");
}


void create_systempp(void)
{
	set_crash();
	if ((syspp=(SYSPP_OBJECT)malloc(sizeof(struct syspp_struct)))==NULL) {
		fprintf(stderr,"Lotos: Failed to create systempp object in create_systempp().\n");
		boot_exit(21);
		}

	syspp->oss_highlev_debug=0;
	syspp->debug_input=0; // POZOR !!! Nikdy nenastavovat na 1 !!!
	syspp->highlev_debug_on=0;
	syspp->pueblo_enh=0;
	syspp->pblo_usr_mm_def=0;
	syspp->pblo_usr_pg_def=0;
	syspp->kill_msgs=0;
	syspp->sys_access=1;
	syspp->wiz_access=1;
	syspp->auto_afk=0;
	syspp->auto_afk_time=0;
}


int show_file(UR_OBJECT user, char *filename)
{
	FILE *fp;
	char line[ARR_SIZE+1];

	set_crash();
	if ((fp=fopen(filename, "r"))==NULL) return 0;
	fgets(line, ARR_SIZE, fp);
	while(!feof(fp)) {
		line[strlen(line)-1]='\0';
		strcat(line, "\n");
		write_user(user, line);
		fgets(line, ARR_SIZE, fp);
		}
	fclose(fp);
	return 1;
		
}


void write_room_except2(RM_OBJECT rm, char *str, UR_OBJECT user, UR_OBJECT user2)
{
	UR_OBJECT u;

	set_crash();
for(u=user_first;u!=NULL;u=u->next) {
  if (u->login 
      || u->room==NULL 
      || (u->room!=rm && rm!=NULL) 
      || (u->ignore.all && !force_listen)
      || (u->ignore.shouts && (com_num==SHOUT || com_num==SEMOTE || com_num==SHOUTTO))
      || (u->ignore.logons && logon_flag)
      || (u->ignore.greets && com_num==GREET)
      || u==user
      || u==user2) continue;
  if ((check_igusers(u,user))!=-1 && user->level<ARCH) continue;
  if (u->type==CLONE_TYPE) {
    if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignore.all) continue;
    /* Ignore anything not in clones room, eg shouts, system messages
       and semotes since the clones owner will hear them anyway. */
    if (rm!=u->room) continue;
    if (u->clone_hear==CLONE_HEAR_SWEARS) {
      if (!contains_swearing(str)) continue;
      }
    sprintf(text, "~FT[ %s ]:~RS %s",u->room->name,str);
    write_user(u->owner, text);
    }
  else write_user(u,str);
  } /* end for */
}


/* zobrazenie sklonovania nicku */
void show_nick_grm(UR_OBJECT user, UR_OBJECT u)
{
	set_crash();
	vwrite_user(user, "2. p - G: %s\n", u->nameg);
	vwrite_user(user, "3. p - D: %s\n", u->named);
	vwrite_user(user, "4. p - A: %s\n", u->namea);
	vwrite_user(user, "6. p - L: %s\n", u->namel);
	vwrite_user(user, "7. p - I: %s\n", u->namei);
	vwrite_user(user, "privl. pre muzsky rod : %s\n", u->namex);
	vwrite_user(user, "privl. pre zensky rod : %s\n", u->namey);
	vwrite_user(user, "privl. pre stredny rod: %s\n", u->namez);
}


void com_nick_grm(UR_OBJECT user)
{
	UR_OBJECT u;
	int on;

	set_crash();
	if ((word_count<2) || (user->level<GOD)) {
		u=user;
		on=1;
		}
	else {
		if (!(u=get_user(word[1]))) {
			if ((u=create_user())==NULL) {
				vwrite_user(user,"%s: nemozem vytvorit docasny user objekt.\n",syserror);
				write_syslog(ERRLOG,1,"Unable to create temporary user object in examine().\n");
				return;
				}
			strcpy(u->name,word[1]);
			if (!load_user_details(u)) {
				write_user(user,nosuchuser);
				destruct_user(u);
				destructed=0;
				return;
				}
			on=0;
			}
		else on=1;
		}
	vwrite_user(user, "2. p - G: %s\n", u->nameg);
	vwrite_user(user, "3. p - D: %s\n", u->named);
	vwrite_user(user, "4. p - A: %s\n", u->namea);
	vwrite_user(user, "6. p - L: %s\n", u->namel);
	vwrite_user(user, "7. p - I: %s\n", u->namei);
	vwrite_user(user, "privl. pre muzsky rod : %s\n", u->namex);
	vwrite_user(user, "privl. pre zensky rod : %s\n", u->namey);
	vwrite_user(user, "privl. pre stredny rod: %s\n", u->namez);
	if (!on) {
		destruct_user(u);
		destructed=0;
		}
	return;
}


int port_connect(char *host, int port)
{
	int portsocket;
	struct sockaddr_in sin;
	struct hostent *he;

	set_crash();
	he=gethostbyname(host);
	if(he==NULL) return -1;

	bzero((char *)&sin,sizeof(sin));
	bcopy(he->h_addr,(char *)&sin.sin_addr,he->h_length);
	sin.sin_family=he->h_addrtype;
	sin.sin_port=htons(port);
	portsocket=socket(AF_INET, SOCK_STREAM, 0);
	if(portsocket==-1) return -1;
	if(connect(portsocket,(struct sockaddr *)&sin,sizeof(sin))==-1) return -1;
	return portsocket;   
}

char *getanswer(FILE *popfp, char *buff, int eol)
{
	int ch;
	char *in=buff;

	set_crash();
	for(;;) {
		ch=getc(popfp);
		if((eol==1) || (ch == '\n')) {
			*in='\0';
			eol=0;
			return buff;
			}
		else {
			*in=(char)ch;
			in++;
			}
		}
}


/*** Load swear words list from file ***/
void load_swear_file(UR_OBJECT user)
{
	FILE *fp;
	char line[WORD_LEN+1];
	int i;

	set_crash();
	for (i=0; i<MAX_SWEARS; i++)
		swear_words[i][0]='\0';
	i=0;
	if(user==NULL) printf("Loading swear words file ... ");
	else write_user(user,">>>Loading swear words file ... ");
	if(!(fp=fopen(SWEARFILE, "r"))) {
		strcpy(swear_words[0],"*");
		if(user==NULL) printf(" not found.\n");
		else write_user(user," not found.\n");
		return;
		}
	fgets(line,WORD_LEN+2,fp);
	while(!feof(fp)) {
		line[strlen(line)-1]='\0';
		strcpy(swear_words[i],line);
		i++;
		if(i>=MAX_SWEARS) break;
		fgets(line,WORD_LEN+2,fp);
		}
	fclose(fp);
	strcpy(swear_words[i],"*");
	if(user==NULL) printf(" done (%d words).\n",i);
	else vwrite_user(user," done ( ~FT%d~RS words ).\n",i);
}


int backup_talker(void)
{
	char fname[6][500];
	int i;

	set_crash();
	switch (double_fork()) {
		case -1 :
			sprintf(text,"~OLSYSTEM: backup_talker(): Failed to fork backup process...\n");
			write_level(ARCH, 1, text, NULL);
			write_syslog(ERRLOG, 1, "backup_talker(): Failed to fork process...\n");
			return 0; /* double_fork() failed */
		case  0 : /* Start Backup Of Files */
		  	sprintf(fname[0], "%s/%s.tgz", BACKUPDIR, BACKUPFILE);
		  	sprintf(fname[1], "%s/%s.log1", LOGFILES, BACKUPFILE);
		  	sprintf(fname[2], "%s/%s.log2", LOGFILES, BACKUPFILE);
		  	sprintf(fname[3], "%s/%s.tgz", TEMPFILES, BACKUPFILE);
		  	sprintf(fname[4], "%s/%s.log1", TEMPFILES, BACKUPFILE);
		  	sprintf(fname[5], "%s/%s.log2", TEMPFILES, BACKUPFILE);
		  	for (i=0; i<6; i++) unlink(fname[i]);
			write_syslog(SYSLOG, 1, "Backing Up Talker Files To : %s/%s.tgz\n",BACKUPDIR,BACKUPFILE);
			write_syslog(SYSLOG, 1, "For Zip Progress, Read File: %s/%s.log\n",LOGFILES,BACKUPFILE);
//		sprintf(text,"zip -v -9 -r %s/%s.zip * > %s/%s/%s.log",
//			BACKUPDIR, BACKUPFILE, ROOTDIR, LOGFILES, BACKUPFILE);
			sprintf(text, "tar -zcfp '%s' '%s' 1> '%s' 2> '%s'",
				fname[3], ROOTDIR, fname[4], fname[5]
				);
			system(text);
			for (i=0; i<3; i++) rename(fname[i+3], fname[i]);
			_exit(1);
			return 1;
		}
	return 0;
}


void follow(UR_OBJECT user)
{
	UR_OBJECT ur;
	int i;

	set_crash();
	for (ur=user_first; ur!=NULL; ur=ur->next) {
		if (ur->follow!=user) continue;
		if (ur->room==user->room) continue; /* inac by (asi) vznikol nekonecny cyklus */
		if (ur->level<command_table[FOLLOW].level) { /* kvoli demote */
			ur->follow=NULL;
			continue;
			}
		if (!has_room_access(ur, user->room)) {
			vwrite_user(ur, "Smola, ale %s is%s do miestnosti, kde ty nemas pristup,\nnemozes %s uz dalej sledovat ...\n",
				user->name, grm_gnd(7, user->gender), grm_gnd(8, user->gender));
			ur->follow=NULL;
			continue;
			}
		if (ur->level<WIZ) { /* len ak je do roomy linka */
			if (user->room->transp!=NULL) {
				if ((user->room->transp->go) || user->room->link[user->room->transp->out]!=ur->room) {
					vwrite_user(ur, "Smola, ale %s is%s do miestnosti, kde sa teraz odtialto nedostanes,\nnemozes %s dalej sledovat ...\n",
						user->name, grm_gnd(7, user->gender), grm_gnd(8, user->gender));
					continue;
					}
				vwrite_user(ur, "Sledujes %s do miestnosti %s\n",
					user->name, user->room->name);
				move_user(ur, user->room, 0);
				follow(ur);
				continue;
				}
			else {
				for (i=0; i<MAX_LINKS; ++i) {
					if (ur->room->link[i]==user->room) {
						vwrite_user(ur, "Sledujes %s do miestnosti %s\n",
							user->name, user->room->name);
						move_user(ur, user->room, 0);
						follow(ur);
						continue;
						}
					}
				}
			vwrite_user(ur, "Smola, ale %s is%s do miestnosti, kde sa teraz odtialto nedostanes,\nnemozes %s dalej sledovat ...\n",
				user->name, grm_gnd(7, user->gender), grm_gnd(8, user->gender));
			continue;
			}
		vwrite_user(ur, "Sledujes %s do miestnosti %s\n",
			user->name, user->room->name);
		move_user(ur, user->room, 0);
		follow(ur); /* ak aj jeho niekto sleduje */
		continue;
		}
}


void myxterm(UR_OBJECT user, char *inpstr)
{
	set_crash();
	if (!user->terminal.xterm) {
		write_user(user, "Ved nemas povoleny xterm !!!\n");
		return;
		}
	vwrite_user(user, "\033]0;%s\007", inpstr);
}


void allxterm(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;

	set_crash();
	for (u=user_first; u!=NULL; u=u->next) {
		if (u->login || u->type==CLONE_TYPE) continue;
		if (u->terminal.xterm) vwrite_user(u, "\033]0;%s\007", inpstr);
		else vwrite_user(user, "Window titlebar & icon changed to ~OL%s\n", inpstr);
		}
}


int inroom(RM_OBJECT rm)
{
	UR_OBJECT u;
	int users=0;

	set_crash();
	for (u=user_first; u!=NULL; u=u->next) {
		if (u->login) continue;
		if (u->room==rm) users++;
		}
	return users;
}


int file_exists(char *fname)
{
	FILE *fp;

	set_crash();
	if ((fp=fopen(fname, "r"))==NULL) return 0;
	fclose(fp);
	return 1;
}


/* Restore a user to his original rank from a temp promote.  S.C.  09/27/99 */
void restore(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if (word_count<2) {
   write_usage(user,"restore <user>");
   return;
   }
if (!(u=get_user_name(user,word[1]))) {
   write_user(user, notloggedon);
   return;
   }
if (u->level>u->real_level) {
  u->level=u->real_level;
  vwrite_user(user,"You have restored %s to their original level.\n");
  vwrite_room_except(u->room,u,"%s begins fading as their power is restored to normal.\n",u->name);
  vwrite_user(u,"Bol obnoveny tvoj originalny level.\n");
  write_syslog(SYSLOG, 1, "%s restored %s to their original level.\n",user->name,u->name);
  sprintf(text,"Had their level restored by %s.\n",user->name);
  add_history(u->name,1,text);
  return;
  }
else {
  write_user(user,"You can't restore a person to their original rank if they're already there.\n");
  return;
  }
}

#ifdef DEBUG
void s_crash(char *file, int line)
{
	if (!strcmp(crash[crash_step].lastfile, file)
		&& crash[crash_step].lastline==line
		&& crash[crash_step].n<=254)
			crash[crash_step].n++;
	else {
		crash_step++;
		if (crash_step>=1024) crash_step=0;
		strcpy(crash[crash_step].lastfile, file);
		crash[crash_step].lastline=line;
		crash[crash_step].n=1;
		}
}

void crash_dump(void)
{
	FILE *fp;
	int i;
	char fname[512];

	set_crash();
	sprintf(fname, "%s/crash_dump.%4d_%02d_%02d_%02d_%02d_%02d", LOGFILES, tyear, tmonth, tday, thour, tmin, tsec);
	if ((fp=fopen(fname, "w"))==NULL) {
		write_syslog(ERRLOG, 1, "Nemozem otvorit subor %s na zapis v crash_dump()\n", fname);
		return;
		}
	fprintf(fp, "id   file              line cnt\n");
	for (i=crash_step+1;; i++) {
		if (i==CRASH_HISTORY) i=0;
		fprintf(fp, "%4.4d %-13.13s %8d %3d\n", i, crash[i].lastfile, crash[i].lastline, crash[i].n);
		fflush(fp);
		if (i==crash_step) break;
		}
	fclose(fp);
}
#endif


#endif /* adds.c */
