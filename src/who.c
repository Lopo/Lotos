/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                  Funkcie Lotos v1.2.0 pre zoznamy typu who
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __WHO_C__
#define __WHO_C__ 1

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_syspp.h"
#include "who.h"


void who_nuts333(UR_OBJECT user)
{
	UR_OBJECT u;
	int mins, idle, total=0, invis=0, remote=0, cnt;
	char line[USER_NAME_LEN+USER_DESC_LEN*2];
	char rname[ROOM_NAME_LEN+1], portstr[5];

	set_crash();
	sprintf(text, "\n~BB*** Current users %s ***\n\n", long_date(1));
	write_user(user, text);
	for (u=user_first; u!=NULL; u=u->next) {
		if (u->type==CLONE_TYPE) continue;
		mins=(int)(time(0) - u->last_login)/60;
		idle=(int)(time(0) - u->last_input)/60;
		if (u->type==REMOTE_TYPE) strcpy(portstr, "   -");
		else {
			if (u->port==port[0]) strcpy(portstr,"MAIN");
			else strcpy(portstr, " WIZ");
			}
		if (u->login) continue;
		++total;
		if (u->type==REMOTE_TYPE) ++remote;
		if (!u->vis) {
			++invis;
			if (u->level>user->level) continue;
			}
		sprintf(line, "  %s %s~RS", u->name, u->desc);
		if (!u->vis) line[0]='*';
		if (u->type==REMOTE_TYPE) line[1]='@';
		if (u->room==NULL)
#ifdef NETLINKS
			sprintf(rname, "@%s", u->netlink->service);
#else
			strcpy(rname, "???");
#endif
		else strcpy(rname, u->room->name);
		cnt=colour_com_count(line);
		sprintf(text, "%-*s : %-4.4s : %-12.12s : %d mins.", 40+cnt*3, line, user_level[u->level].name, rname, mins);
		if (u->afk) strcat(text, "~BR(AFK)\n");
		else strcat(text, "\n");
		write_user(user, text);
		}
	vwrite_user(user, "\nThere are %d visible, %d invisible, %d remote users.\nTotal of %d users.\n\n", syspp->acounter[3]-invis, invis, remote, total);
}


void who_short(UR_OBJECT user)
{
	int ret,cnt,invis,logins,hidden;
	char line[USER_NAME_LEN+USER_DESC_LEN*2];
	UR_OBJECT u;

	set_crash();
	cnt=0; ret=0; invis=0; logins=0; hidden=0;
	write_user(user,"\n");
	write_user(user,center_string(80,0,NULL,"~FM-~OL=~FR[ ~RSPeople in the %s %s ~OL~FR]~FM=~RS~FM-",reg_sysinfo[TALKERNAME],long_date(1)));
	write_user(user,"\n~OL~FM------------------------------------------------------------------------------\n");
for(u=user_first;u!=NULL;u=u->next) {
     if (u->type==CLONE_TYPE) continue;
     if (u->login) { logins++; continue; }
//     if (u->room->hidden && user->level<ROOT) continue;
//     if (u->hidden) hidden++;
//     if (u->hidden && user->level<ROOT) continue;
     if (!u->vis) invis++;
     if (!u->vis && (u->level>user->level)) continue;
     cnt++;
//     if (u->hidden) cnt--;
/*     if (u->room->hidden) sprintf(line,"   ~OL~FY&~OL~FR%-15.15s",u->bw_recap);
     else*/ sprintf(line,"    ~OL~FT%-15.15s", u->bw_recap);
     if (!u->vis) line[2]='!';
//     if (u->hidden) line[1]='#';
     write_user(user,line);
     ret++;
     if (ret==4) { ret=0; write_user(user,"\n"); }
     }        
if (ret>0 && ret<4) write_user(user,"\n");
write_user(user,"~OL~FM------------------------------------------------------------------------------\n");
if (user->level>=WIZ) vwrite_user(user,"   ~OL~FBThere are ~FM%d ~FBinvisible and ~FM%d ~FBlogins.\n",invis,logins);
if (user->level==ROOT) vwrite_user(user,"   ~OL~FRThere are ~FM%d ~FRhidden users.\n",hidden);
vwrite_user(user,"   ~OL~FGThere are ~FM%d~FG users online!\n",cnt);
write_user(user,"~OL~FM------------------------------------------------------------------------------\n");
}

void who_moebyroom(UR_OBJECT user)
{
	UR_OBJECT u, cu;
	RM_OBJECT rm;
	char txt[ARR_SIZE];
	int cnt,total,invis,mins,idle,logins,hidden;
	char line[USER_NAME_LEN+USER_DESC_LEN*2];
	char rname[ROOM_NAME_LEN+1],levelname[20];
	char gender[7];
	char *msg_afk=" %s ~RS~FRis away from the keyboard.";
	char *msg_edit=" %s ~RS~FTis using the text editor";
	char *msg_sleep=" %s ~RS~FMappears to be sleeping.";

	set_crash();
	total=0;  invis=0;  logins=0;  hidden=0;

/*** Print Who List Header ***/

	write_user(user,"\n\n");
	write_user(user,"~FB_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n");
	strcpy(txt, center_string(66,0,NULL,"People At %s %s",reg_sysinfo[TALKERNAME],long_date(1)));
	txt[strlen(txt)-2]='\0';
	sprintf(text, "~FB_.-[ ~OL~FT%-67.67s ~RS~FB]-._\n", txt);
	write_user(user, text);
	write_user(user,"~FB_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n");
	for (rm=room_first; rm!=NULL; rm=rm->next) {
		if (inroom(rm)) {  /* Check To See If Room Is Empty */
			strcpy(txt, center_string(66, 0, NULL, rm->name));
			txt[strlen(txt)-1]='\0';
			sprintf(text, "~FB-~OL=~FT+  ~FG%-67.67s  ~FT+~FB=~RS~FB-\n", txt);
			write_user(user, text);
			cu=NULL;
			for (u=user_first; u!=NULL; u=u->next) {
				if (u->login) { logins++; continue; }
				if (u->type==CLONE_TYPE) cu=get_user(u->name);
				if (u->type==CLONE_TYPE && cu->room!=rm) continue;
				if (u->room!=rm) continue;
				mins=(int)(time(0) - u->last_login)/60;
				idle=(int)(time(0) - u->last_input)/60;
				if (u->type==CLONE_TYPE) {
					mins=(int)(time(0) - cu->last_login)/60;
					idle=(int)(time(0) - cu->last_input)/60; 
					}
/*          if (!u->level && u->muzzled & F_SCUM) strcpy(levelname,"Scum");
          else*/ 
				strcpy(levelname,user_level[u->level].name);
				if (u->type==CLONE_TYPE) {
/*               if (!cu->level && cu->muzzled & F_SCUM) strcpy(levelname,"Scum");
               else*/ strcpy(levelname,user_level[cu->level].name);
					}
				if (u->arrestby) strcpy(levelname,"Jailed");
				sprintf(gender, "~OL%c~RS", sex[u->gender][0]);
				if (u->type==CLONE_TYPE) sprintf(gender, "~OL%c~RS", sex[cu->gender][0]);
				++total;
				if (u->login) logins++;
//          if (u->hidden && user->level<ROOT) { total--; hidden++; continue; }
				if (!u->vis) {
				--total; ++invis;
				if (u->level>user->level) continue;
				}
			strcpy(rname,u->room->name);
			sprintf(line," %s %s",u->recap,u->desc);
			if (u->afk) sprintf(line, msg_afk, u->recap);
			else if (idle>10) sprintf(line, msg_sleep, u->recap);
			else if (u->malloc_start!=NULL) sprintf(line, msg_edit, u->recap);
			if (u->type==CLONE_TYPE) {
				if (cu->afk) sprintf(line, msg_afk, u->recap);
				else if (idle>10) sprintf(line, msg_sleep, u->recap);
				else if (cu->malloc_start!=NULL) sprintf(line, msg_edit, u->recap);
				}
	  /* Process User Status */
			if (!u->vis) line[0]='!';
//          if (u->hidden) line[0]='#';
			cnt=colour_com_count(line);
			vwrite_user(user,"~FB-~OL=~FT+ ~FY%s ~RS~FR: ~OL~FM%-14.14s ~RS~FR:~OL~FG%4d~FW:~FR%-4d~RS~FR:~RS%-*.*s ~RS~OL~FT+~FB=~RS~FB-\n",gender,levelname,mins,idle,39+cnt*3,39+cnt*3,line);
			continue;
			}
		}
}

/*** Display Footer ***/

     write_user(user,"~FB_.-._.-._.-._.-._.-._.+._.-._.-._;-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n");
     if (user->level>WIZ) vwrite_user(user,"  ~FRThere are %d invisible and %d people at the login stage.\n",invis,logins);
    vwrite_user(user,"  ~OL~FBThere %s currently %d %s in the %s.\n",(total==1?"is":"are"),total,(total==1?"person":"people"),reg_sysinfo[TALKERNAME]);
    write_user(user,"~FB_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n\n");
}

void who_hope(UR_OBJECT user)
{
	UR_OBJECT u, cu=user;

	int total,invis,mins,idle,logins,hidden;
	char line[USER_NAME_LEN+USER_DESC_LEN*2];
	char rname[ROOM_NAME_LEN+1],levelname[20];
	char gender[7], text2[ARR_SIZE+1];
	char txt[ARR_SIZE];

	set_crash();
	total=0;  invis=0;  logins=0;  hidden=0;
	write_user(user,"\n\n");
	write_user(user,"~FB_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n");
	sprintf(text2,"People At %s %s",reg_sysinfo[TALKERNAME],long_date(1));
	strcpy(txt, center_string(66,0,NULL,text2));
	txt[strlen(txt)-2]='\0';
	vwrite_user(user,"~FB_.-[ ~OL~FT%-67.67s ~RS~FB]-._\n", txt);
	write_user(user,"~FB_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n");
	write_user(user,"~FB|    ~OL~FTRoom Name:   ~RS~FB|~OL~FYG~RS~FB| ~OL~FMRank    ~RS~FB|~OL~FGTime~FW:~FRIdle~RS~FB| ~FTName and Description\n");
	write_user(user,"~FB`.-._.-._.-._.-._.+._.+._.-._.-._.-._.-._.-.+.-._.-._.-._.-._.-._.-._.-._.-._\n");
	for(u=user_first;u!=NULL;u=u->next) {
		if (u->login) {logins++; continue; }
//          if (u->room->hidden && user->level<ROOT) continue;
		if (u->type==CLONE_TYPE) cu=get_user(u->name);
		mins=(int)(time(0) - u->last_login)/60;
		idle=(int)(time(0) - u->last_input)/60;
		if (u->type==CLONE_TYPE) {
			mins=(int)(time(0) - cu->last_login)/60;
			idle=(int)(time(0) - cu->last_input)/60; 
			}
/*          if (!u->level && u->muzzled & F_SCUM) strcpy(levelname,"Scum");
          else*/ 
		strcpy(levelname,user_level[u->level].name);
		if (u->type==CLONE_TYPE) {
/*               if (!cu->level && cu->muzzled & F_SCUM) strcpy(levelname,"Scum");
               else*/
			strcpy(levelname,user_level[cu->level].name);
			}
		if (u->arrestby) strcpy(levelname,"Jailed");
		sprintf(gender, "~OL%c~RS", sex[u->gender][0]);
		if (u->type==CLONE_TYPE) sprintf(gender, "~OL%c~RS", sex[cu->gender][0]);
		++total;
		if (u->login) logins++;
//          if (u->hidden && user->level<ROOT) { total--; hidden++; continue; }
		if (!u->vis) {
			--total; ++invis;
			if (u->level>user->level) continue;
			}
		strcpy(rname,u->room->name);
		sprintf(line," %s %s",u->recap,u->desc);
		if (u->afk) sprintf(line," %s ~RS~FRis away from the keyboard.",u->recap);
		else if (idle>10) sprintf(line," %s ~RS~FMappears to be sleeping.",u->recap);
		else if (u->malloc_start!=NULL) sprintf(line," %s ~RS~FTis using the text editor",u->recap);
		if (u->type==CLONE_TYPE) {
			if (cu->afk) sprintf(line," %s ~RS~FRis away from the keyboard.",u->recap);
			else if (idle>10) sprintf(line," %s ~RS~FRappears to be sleeping.",u->recap);
			else if (cu->malloc_start!=NULL) sprintf(line," %s ~RS~FTis using the editor.",u->recap);
			}
	  /* Process User Status */
		if (!u->vis) line[0]='!';
//          if (u->hidden) line[0]='#';
//          if (u->room->hidden) line[0]='&';
		vwrite_user(user,"~OL~FT  %-15.15s ~RS~FB|~OL~FY%s~RS~FB| ~OL~FM%-7.7s ~RS~FB|~OL~FG%4d~FW:~FR%-4d~RS~FB|~RS%s\n",rname,gender,levelname,mins,idle,line);
		continue;
		}
	write_user(user,"~FB_.-._.-._.-._.-._.-._.+._.-._.-._;-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n");
	if (user->level>WIZ) vwrite_user(user,"  ~FRThere are %d invisible and %d people at the login stage.\n",invis,logins);
	vwrite_user(user,"  ~OL~FBThere %s currently %d %s in the %s.\n",(total==1?"is":"are"),total,(total==1?"person":"people"),reg_sysinfo[TALKERNAME]);
	write_user(user,"~FB_.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._.-._\n\n");
}

void who_stairway(UR_OBJECT user)
{
	UR_OBJECT u, cu=user;

int cnt,total,invis,mins,idle,logins,hidden;
char line[USER_NAME_LEN+USER_DESC_LEN*2];
char rname[ROOM_NAME_LEN+1],levelname[20];
char status[6], gender[7];

	set_crash();
     total=0;  invis=0;  logins=0;  hidden=0;/* Init Vars to 0 */
     write_user(user,"\n");
     sprintf(text,"~FMPeople roaming %s %s",reg_sysinfo[TALKERNAME],long_date(1));
     write_user(user,center_string(75,0,NULL,text));
     write_user(user,"\n\n");
        write_user(user,"~OL~FB+--------------------------------------------------------------------------+\n");
        write_user(user,"~OL~FB|   ~FTName and Description                  ~FB|~FTm/f~FB|~FTLevel~FB|~FTRoom      ~FB|~FTMins~FB|~FTIdle~FB|~FTS~FB|\n");
        write_user(user,"~OL~FB+------------------------------------------+-+------+----------+----+----+-+\n");
     for(u=user_first;u!=NULL;u=u->next) {
          if (u->login) {logins++; continue; }
          if (u->type==CLONE_TYPE) cu=get_user(u->name);
          mins=(int)(time(0) - u->last_login)/60;
          idle=(int)(time(0) - u->last_input)/60;
          if (u->type==CLONE_TYPE) {
               mins=(int)(time(0) - cu->last_login)/60;
               idle=(int)(time(0) - cu->last_input)/60; 
               }
/*          if (!u->level && u->muzzled & F_SCUM) strcpy(levelname,"Scum");
          else*/ strcpy(levelname,user_level[u->level].name);
          if (u->type==CLONE_TYPE) {
/*               if (!cu->level && cu->muzzled & F_SCUM) strcpy(levelname,"Scum");
               else*/ strcpy(levelname,user_level[cu->level].name);
               }
          if (u->muzzled) strcpy(levelname,"Muzzled");
	  sprintf(gender, "%c", sex[u->gender][0]);
          if (u->type==CLONE_TYPE) sprintf(gender, "%c", sex[u->gender][0]);
          ++total;
          if (u->login) logins++;
//          if (u->hidden && user->level<ROOT) { total--; hidden++; continue; }
          if (!u->vis) {
               --total;
               ++invis;
               if (u->level>user->level) continue;
               }
          sprintf(line,"  %s %s",u->recap,u->desc); 
          if (!u->vis) line[0]='!';
//          if (u->hidden) line[1]='#';
//          if (u->room->hidden) line[1]='&';
          strcpy(rname,u->room->name);
	  sprintf(status, "%c", u->status);
		if (idle) strcpy(status,"i");
          if (u->type==CLONE_TYPE) {
               if (cu->afk) strcat(status,"A");
               else if (idle) strcpy(status,"i");
	  	else sprintf(status, "%c", cu->status);
               }
          /* Count number of colour coms to be taken account of when formatting */
          cnt=colour_com_count(line);
          vwrite_user(user,"~OL~FB|~RS %-*s ~RS~OL~FB|~FM%-1.1s~FB|~FR%-6.6s~FB|~FT%-10.10s~FB|~FY%4.4d~FB|~FR%4.4d~FB|~FG%-1.1s~FB|\n",40+cnt*3,line,gender,levelname,rname,mins,idle,status);
          continue;
          }
     write_user(user,"~OL~FB+------------------------------------------+-+------+----------+----+----+-+\n");
     if (user->level>WIZ) vwrite_user(user,"  ~FRThere are %d invisible users and %d logins.\n",invis,logins);
    vwrite_user(user,"  ~FGThere are %d people roaming around.\n",total);
    write_user(user,"~OL~FB----------------------------------------------------------------------------\n");
}

#endif /* who.c */
