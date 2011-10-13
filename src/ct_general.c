/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
               Funkcie Lotos v1.2.0 skupiny hlavnych, verejnych
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_GENERAL_C__
#define __CT_GENERAL_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "obj_ur.h"
#include "obj_rm.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "obj_syspp.h"
#include "obj_pl.h"
#include "obj_mc.h"
#include "ct_general.h"
#include "prototypes.h"
#include "comvals.h"



/*** Disconnect user from talker ***/
void disconnect_user(UR_OBJECT user)
{
	UR_OBJECT ur;
	RM_OBJECT rm;
#ifdef NETLINKS
	NL_OBJECT nl;
#endif
	long int onfor,hours,mins;

	set_crash();
	for (ur=user_first; ur!=NULL; ur=ur->next)
		if (ur->follow==user) ur->follow=NULL;

	rm=user->room;
	if (user->login) {
		close(user->socket);  
		destruct_user(user);
		amsys->num_of_logins--;  
		return;
		}
	if (user->type!=REMOTE_TYPE) {
		onfor=(int)(time(0)-user->last_login);
		hours=(onfor%86400)/3600;
		mins=(onfor%3600)/60;
		save_user_details(user,1);
		save_plugin_data(user);
		write_syslog(SYSLOG,1,"%s logged out.\n",user->name);
		write_user(user,"\n~OL~FBYou are removed from this reality...\n\n");
		vwrite_user(user,"You were logged on from site %s\n",user->site);
		vwrite_user(user,"On %s, %d%s %s, for a total of %d hour%s and %d minute%s.\n\n",
			day[twday],tmday,ordinal_text(tmday),month[tmonth],(int)hours,PLTEXT_S(hours),(int)mins,PLTEXT_S(mins));
		close(user->socket);
		logon_flag=1;
		if (user->vis) vwrite_room(NULL,"~OL[Leaving is:~RS %s~RS %s~RS~OL]\n",user->recap,user->desc);
		else {
			sprintf(text,"~OL~FY[ INVIS ]~RS ~OL[Leaving is:~RS %s~RS %s~RS~OL]\n",user->recap,user->desc);
			write_level(WIZ,1,text,NULL);
			}
		logon_flag=0;
#ifdef NETLINKS
		if (user->room==NULL) {
			sprintf(text,"REL %s\n",user->name);
			write_sock(user->netlink->socket,text);
			for(nl=nl_first;nl!=NULL;nl=nl->next) 
				if (nl->mesg_user==user) {  
					nl->mesg_user=(UR_OBJECT)-1;
					break;
					}
				}
#endif
		}
#ifdef NETLINKS
	else {
		save_plugin_data(user);
		write_user(user,"\n~FR~OLYou are pulled back in disgrace to your own domain...\n");
		sprintf(text,"REMVD %s\n",user->name);
		write_sock(user->netlink->socket,text);
		vwrite_room_except(rm,user,"~FR~OL%s is banished from here!\n",user->name);
		write_syslog(NETLOG,1,"NETLINK: Remote user %s removed.\n",user->name);
		}
#endif
	if (user->malloc_start!=NULL) free(user->malloc_start);
	syspp->acounter[3]--;
	syspp->acounter[user->gender]--;
	record_last_logout(user->name);
	alert_friends(user, 0);
	destroy_user_clones(user);
	destruct_user(user);
	reset_access(rm);
}


/*** Display details of room ***/
void look(UR_OBJECT user)
{
RM_OBJECT rm, rm2;
UR_OBJECT u;
char temp[181],null[1],*ptr;
char *uafk="~BR(AFK)";
int i,exits,users;

	set_crash();
rm=user->room;
if (rm->access & PRIVATE) vwrite_user(user,"\n~FTRuuma: ~FR%s\n\n",rm->name);
else vwrite_user(user,"\n~FTRuuma: ~FG%s\n\n",rm->name);
if (user->show_rdesc) write_user(user,user->room->desc);
exits=0;
null[0]='\0';
strcpy(text,"\n~FTVychody:~RS");
for(i=0;i<MAX_LINKS;++i) {
  if (rm->link[i]==NULL) break;
  if (rm->link[i]->access & PRIVATE) sprintf(temp,"  ~FR%s",rm->link[i]->name);
  else {
  	if (user->pueblo && rm->link[i]->access!=ROOT_CONSOLE)
  		sprintf(temp,"  </xch_mudtext><b><a xch_cmd=\".go %s\" xch_hint=\"Go to this room.\">%s</a></b><xch_mudtext>",
  			rm->link[i]->name, rm->link[i]->name);
  	else sprintf(temp, "  ~FG%s", rm->link[i]->name);
  	}
  strcat(text,temp);
  ++exits;
  }
for (rm2=room_first; rm2!=NULL; rm2=rm2->next) {
	if (rm2->transp==NULL) continue;
	if (rm2->transp->go) continue;
	if (rm2->link[rm2->transp->out]!=rm) continue;
	if (rm2->access & PRIVATE) sprintf(temp,"  ~RS~OL*~RS~FR%s",rm2->name);
	else {
		if (user->pueblo) sprintf(temp, "  </xch_mudtext><b><a xch_cmd=\".go %s\" xch_hint=\"Go to this room.\">*%s</a></b><xch_mudtext>",
  			rm2->name, rm2->name);
		else sprintf(temp, "  ~RS~OL*~RS~FG%s",rm2->name);
		}
	strcat(text,temp);
	++exits;
	break;
	}
#ifdef NETLINKS
if (rm->netlink!=NULL && rm->netlink->stage==UP) {
  if (rm->netlink->allow==IN) sprintf(temp,"  ~FR%s@",rm->netlink->service);
  else sprintf(temp,"  ~FG%s@",rm->netlink->service);
  strcat(text,temp);
  }
else 
#endif
  if (!exits) strcpy(text, no_exits);
if (rm->transp!=NULL) {
	if (rm->transp->go) strcpy(text, no_exits);
	else {
		if (rm->link[rm->transp->out]->access & PRIVATE) sprintf(text,"\n~FTExit:  ~RS~OL*~RS~FR%s",rm->link[rm->transp->out]->name);
		else {
			if (user->pueblo)
				sprintf(text,"\n~FTExit:  ~RS~OL*~RS</xch_mudtext><b><a xch_cmd=\".go %s\" xch_hint=\"Go to this room.\">%s</a></b><xch_mudtext>",
		  			rm->link[rm->transp->out]->name,rm->link[rm->transp->out]->name);
			else
				sprintf(text,"\n~FTExit:  ~RS~OL*~RS~FG%s",
					rm->link[rm->transp->out]->name);
			}
		}
	}
strcat(text,"\n\n");
write_user(user,text);
users=0;
for(u=user_first;u!=NULL;u=u->next) {
  if (u->room!=rm || u==user || (!u->vis && u->level>user->level)) continue;
  if (!users++) write_user(user,"~FG~OLVisia tu:\n");
  if (u->afk) ptr=uafk; else ptr=null;
  if (!u->vis) vwrite_user(user,"     ~FR*~RS%s~RS %s~RS  %s\n",u->recap,u->desc,ptr);
  else vwrite_user(user,"      %s~RS %s~RS  %s\n",u->recap,u->desc,ptr);
  }
if (!users) vwrite_user(user,"~FGSi tu sam%s.\n", grm_gnd(4, user->gender));
write_user(user,"\n");

if (user->pueblo && rm->access!=ROOT_CONSOLE) strcpy(text, "</xch_mudtext><b><a xch_cmd\".pbloenh RoomConfig_setOpt\" xch_hint=\"Access Options\">Access</a></b><xch_mudtext> is ");
else strcpy(text,"Pristup je ");
switch(rm->access) {
  case PUBLIC:  strcat(text,"nastaveny ~FGPUBLIC~RS");  break;
  case PRIVATE: strcat(text,"nastaveny ~FRPRIVATE~RS");  break;
  case FIXED_PUBLIC:  strcat(text,"~FRfixne~RS nastaveny ~FGPUBLIC~RS");  break;
  case FIXED_PRIVATE: strcat(text,"~FRfixne~RS nastaveny ~FRPRIVATE~RS");  break;
  case PERSONAL_UNLOCKED: strcat(text,"osobny ~FG(verejny)~RS");  break;
  case PERSONAL_LOCKED: strcat(text,"osobny ~FR(zamknuty)~RS");  break;
  case ROOT_CONSOLE: strcat(text, "~CRroot console~RS"); break;
  }
sprintf(temp, message_prompt, rm->mesg_cnt);
strcat(text,temp);
write_user(user,text);
if (rm->topic[0]) vwrite_user(user, topic_prompt, rm->topic);
else write_user(user, notopic_prompt);	
}



/*****************************************************************************/
/* Doplnene funkcie                                                          */
/*****************************************************************************/


/*** Set rooms to public or private ***/
void set_room_access(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;
int cnt;

	set_crash();
rm=user->room;
if (word_count<2) rm=user->room;
else {
  if (user->level<amsys->gatecrash_level) {
    write_user(user,"You are not a high enough level to use the room option.\n");  
    return;
    }
  if ((rm=get_room(word[1]))==NULL) {
    write_user(user,nosuchroom);  return;
    }
  }
if (rm->access>PRIVATE) {
  if (rm==user->room) write_user(user,"This room's access is fixed.\n"); 
  else write_user(user,"That room's access is fixed.\n");
  return;
  }
if (com_num==PUBCOM && rm->access==PUBLIC) {
  if (rm==user->room) write_user(user,"Tato ruuma uz je otvorena.\n");  
  else write_user(user,"Ta ruuma uz je otvorena.\n"); 
  return;
  }
if (user->vis) name=user->recap; else name=invisname;
if (com_num==PRIVCOM) {
  if (rm->access==PRIVATE) {
    if (rm==user->room) write_user(user,"This tato ruuma uz je zavreta.\n");  
    else write_user(user,"Ta ruuma uz je zavreta.\n"); 
    return;
    }
  cnt=0;
  for(u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;
  if (cnt<amsys->min_private_users && user->level<amsys->ignore_mp_level) {
    vwrite_user(user,"You need at least %d users/clones in a room before it can be made private.\n",amsys->min_private_users);
    return;
    }
  write_user(user,"Ruuma nastavena na ~FRPRIVATE.\n");
  if (rm==user->room) vwrite_room_except(rm,user,"%s~RS nastavuje ruumu na ~FRPRIVATE.\n",name);
  else write_room(rm,"Tato ruuma bola nastavena na ~FRPRIVATE.\n");
  rm->access=PRIVATE;
  return;
  }
write_user(user,"Ruuma nastavena na ~FGPUBLIC.\n");
if (rm==user->room) vwrite_room_except(rm,user,"%s~RS has set the room to ~FGPUBLIC.\n",name);
else write_room(rm,"This room has been set to ~FGPUBLIC.\n");
rm->access=PUBLIC;
/* Reset any invites into the room & clear review buffer */
for(u=user_first;u!=NULL;u=u->next) {
  if (u->invite_room==rm) u->invite_room=NULL;
  }
clear_revbuff(rm);
}


/*** Ask to be let into a private room ***/
void letmein(UR_OBJECT user)
{
RM_OBJECT rm;
int i,got;

	set_crash();
if (word_count<2) {
  write_user(user,"Klopat do ktorej ruumy ?\n");  return;
  }
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
if (rm==user->room) {
  vwrite_user(user,"ved uz si v ruume %s!\n",rm->name);
  return;
  }
got=0;
for(i=0;i<MAX_LINKS;++i) if (user->room->link[i]==rm) { got=1;  break; }
if (!got) {
  vwrite_user(user,"The %s is not adjoined to here.\n",rm->name);
  return;
  }
if ((!(rm->access & PRIVATE)) && (rm->access!=ROOT_CONSOLE)) {
  vwrite_user(user,"%s je momentalne verejna.\n",rm->name);
  return;
  }
vwrite_user(user, user_knock_prompt, rm->name);
vwrite_room_except(user->room,user, user_room_knock_prompt, user->recap,rm->name);
vwrite_room(rm,room_knock_prompt,user->recap);
}


/* no longer invite a user to the room you're in if you invited them */
void uninvite(UR_OBJECT user)
{
	UR_OBJECT u;
	char *name;

	set_crash();
	if (word_count<2) {
		write_user(user,"Uninvite who?\n");
		return;
		}
	if (!(u=get_user_name(user,word[1]))) {
		write_user(user,notloggedon);
		return;
		}
	if (u==user) {
		write_user(user,"Nemozes pozvat seba.\n");
		return;
		}
	if (u->invite_room==NULL) {
		vwrite_user(user,"%s~RS nema pozvanku do ziadne ruumy.\n",u->recap);
		return;
		}
	if (strcmp(u->invite_by,user->name)) {
		vwrite_user(user,"%s~RS nema od teba pozvanku !\n",u->recap);
		return;
		}
	vwrite_user(user,"You cancel your invitation to %s~RS.\n",u->recap);
	if (user->vis || u->level>=user->level) name=user->recap;
	else name=invisname;
	vwrite_user(u,"%s~RS cancels your invitation.\n",name);
	u->invite_room=NULL;
	u->invite_by[0]='\0';
}


/*** Invite a user into a private room ***/
void invite(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;

	set_crash();
if (word_count<2) {
  write_user(user,"Pozvat koho ?\n");  return;
  }
if (!strcmp(word[2], "-cancel")) {
	uninvite(user);
	return;
	}
rm=user->room;
if ((!(rm->access & PRIVATE)) && (rm->access!=ROOT_CONSOLE)) {
  write_user(user,"Tato ruuma je prave verejna.\n");
  return;
  }
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);  return;
  }
if (u==user) {
  write_user(user,"Inviting yourself to somewhere is the third sign of madness.\n");
  return;
  }
if (u->room==rm) {
  vwrite_user(user,"%s~RS tu uz predsa je !\n",u->recap);
  return;
  }
if (u->invite_room==rm) {
  vwrite_user(user,"%s~RS uz ma sem pozvanku.\n",u->recap);
  return;
  }
vwrite_user(user,"%s~RS dostava od teba pozvanku.\n",u->recap);
if (user->vis || u->level>=user->level) name=user->recap; else name=invisname;
vwrite_user(u,"%s~RS ta pozyva do ruumy %s.\n",name,rm->name);
u->invite_room=rm;
strcpy(u->invite_by,user->name);
}


/*** Show who is on.  type 0=who, 1=fwho, 2=people ***/
void who(UR_OBJECT user, int type)
{
UR_OBJECT u;
int cnt,total,invis,logins,friend,wholen;
long mins, idle;
char line[(USER_NAME_LEN+USER_DESC_LEN*2)+510];
char rname[ROOM_NAME_LEN+1],portstr[5],idlestr[20],sockstr[3];
#ifdef NETLINKS
 int remote=0;
#endif 

	set_crash();
total=0;  invis=0;  logins=0;  friend=0;

	if(user->restrict[RESTRICT_WHO]==restrict_string[RESTRICT_WHO]) {
		write_user(user,">>>You have no right to use .who or .people! Sorry...\n");
		return;
		}

if ((type==2) && !strcmp(word[1],"key")) {
  write_user(user,"\n+----------------------------------------------------------------------------+\n");
  write_user(user,"| ~OL~FTUser login stages are as follows~RS                                           |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  vwrite_user(user,"| ~FY~OLStage %d :~RS The user has logged onto the port and is entering their name     |\n",LOGIN_NAME);
  vwrite_user(user,"| ~FY~OLStage %d :~RS The user is entering their password for the first time           |\n",LOGIN_PASSWD);
  vwrite_user(user,"| ~FY~OLStage %d :~RS The user is new and has been asked to confirm their password     |\n",LOGIN_CONFIRM);
  vwrite_user(user,"| ~FY~OLStage %d :~RS The user has entered the pre-login information prompt            |\n",LOGIN_PROMPT);
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  return;
  }
if ((user->who_type==2 && type==0) || type!=0) {
	write_user(user,"\n+----------------------------------------------------------------------------+\n");
	write_user(user,center_string(78,0,NULL,"%sPrihlaseny juzri %s",(user->login)?"":"~FG",long_date(1)));
	}
switch (type) {
 case 0:
	switch (user->who_type) {
		case  1: who_nuts333(user); return;
/*		case  2: original Amnuts 221 */
		case  3: who_short(user); return;
		case  4: who_moebyroom(user); return;
		case  5: who_hope(user); return;
		case  6: who_stairway(user); return;
		default: break;
		};
   vwrite_user(user,"%s  Meno                                           : Ruuma           : Tm/Id\n",(user->login)?"":"~FT");
   break;
 case 1:
   write_user(user,"~FTKamos/ka                                         : Ruuma           : Tm/Id\n");
   break;
 case 2:
   write_user(user,"~FTMeno            :Lev Line Ignall Vis Idle Mins Port Site/Service\n");
   break;
  }
write_user(user,"+----------------------------------------------------------------------------+\n");
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE) continue;
  mins=(long)(time(0) - u->last_login)/60;
  idle=(long)(time(0) - u->last_input)/60;
#ifdef NETLINKS
  if (u->type==REMOTE_TYPE) strcpy(portstr,"   @");
  else {
#endif
    if (u->port==port[0]) strcpy(portstr,"MAIN");
    else strcpy(portstr," WIZ");
#ifdef NETLINKS
    }
#endif
  if (u->login) {
    if (type!=2) continue;
    vwrite_user(user,"~FY[Login stage %d]~RS : -   %2d    -     -  %4d    - %s %s:%d\n",u->login,u->socket,idle,portstr,u->site,u->site_port);
    logins++;
    continue;
    }
  if ((type==1) && !user_is_friend(user,u)) continue;
  ++total;
#ifdef NETLINKS
  if (u->type==REMOTE_TYPE) ++remote;
#endif
  if (!u->vis) { 
    ++invis;  
    if (u->level>user->level && !(user->level>=ARCH)) continue;  
    }
  if (type==2) {
    if (u->afk) strcpy(idlestr," ~FRAFK~RS");
    else if (u->editing) strcpy(idlestr,"~FTEDIT~RS");
    else sprintf(idlestr,"%4ld",idle);
#ifdef NETLINKS
    if (u->type==REMOTE_TYPE) strcpy(sockstr," @");
    else
#endif
      sprintf(sockstr,"%2d",u->socket);
    vwrite_user(user,"%-15s : %s   %s   %s   %s %s %4d %s %s\n",u->name,user_level[u->level].alias,sockstr,noyes1[u->ignore.all],noyes1[u->vis],idlestr,mins,portstr,u->site);
    continue;
    }
  /* Pueblo enhanced to examine users by clicking their name */
	if (user->pueblo && !user->login)
		sprintf(line, "  </xch_mudtext><b><a xch_cmd=\".examine %s\" xch_hint=\"Examine this user.\">[]</a></b><xch_mudtext> %s%s %s~RS",
			u->name, colors[CWHOUSER], u->recap, u->desc);
	else sprintf(line,"  %s%s~RS %s~RS",colors[CWHOUSER],u->recap,u->desc);
	wholen=152-(12-strlen(u->name));
  if (!u->vis) line[0]='*';
#ifdef NETLINKS
  if (u->type==REMOTE_TYPE) line[1]='@';
  if (u->room==NULL) sprintf(rname,"@%s",u->netlink->service);
  else 
#endif
    strcpy(rname,u->room->name);
  /* Count number of colour coms to be taken account of when formatting */
  cnt=colour_com_count(line);
  if (u->afk) strcpy(idlestr,"~FRAFK~RS");
  else if (u->editing) strcpy(idlestr,"~FTEDIT~RS");
  else if (idle>=30) strcpy(idlestr,"~FYIDLE~RS");
  else sprintf(idlestr,"%ld/%ld",mins,idle);
	if (!user->pueblo) sprintf(text,"%-*.*s~RS   %s : %-15.15s : %s\n",44+cnt*3,44+cnt*3,line,user_level[u->level].alias,rname,idlestr);
	else sprintf(text,"%-*.*s~RS   %s : %-15.15s : %s\n",wholen+cnt*3,wholen+cnt*3,line,user_level[u->level].alias,rname,idlestr);
  if ((strlen(word[1]) && strstr(text, word[1])) || !strlen(word[1]))
          write_user(user, text);
  }
write_user(user,"\n+----------------------------------------------------------------------------+\n");
switch (type) {
 case 2:
   sprintf(text," a ~OL~FG%d~RS login%s", logins, grm_num(7, logins));
 case 0:
#ifdef NETLINKS
   write_user(user,center_string(78,0,NULL,"Spolu ~OL~FG%d~RS juz%s%s: viditeny ~OL~FT%d~RS, neviditelny ~OL~FT%d~RS, vzdialeny ~OL~FT%d~RS",
				 total, grm_num(10, total), (type==2)?text:"",syspp->acounter[3]-invis,invis,remote));
#else
   write_user(user,center_string(78,0,NULL,"Spolu ~OL~FG%d~RS juz%s%s: viditelny ~OL~FT%d~RS, neviditelny ~OL~FT%d~RS",
				 total, grm_num(10, total), (type==2)?text:"",syspp->acounter[3]-invis,invis));
#endif
   vwrite_user(user, "Rekord naraz nahlasenych: ~FR~OL %ld\n", syspp->mcounter[3]);
   vwrite_user(user, "Pocet loginov od %s: ~OL~FT%ld\n", FSTART, syspp->tcounter[3]);
   break;
 case 1:
   write_user(user,center_string(78,0,NULL,"Total of ~OL~FG%d~RS friend%s : ~OL~FT%d~RS visible, ~OL~FT%d~RS invisible",
				 total,PLTEXT_S(total),total-invis,invis));
   break;
 }
write_user(user,"+----------------------------------------------------------------------------+\n");
}


/*** Search all the boards for the words given in the list. Rooms fixed to
     private will be ignore if the users level is less than gatecrash_level ***/
void search_boards(UR_OBJECT user)
{
RM_OBJECT rm;
FILE *fp;
char filename[500],line[82],buff[(MAX_LINES+1)*82],w1[81],rmname[USER_NAME_LEN];
int w,cnt,message,yes,room_given;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <word list>\n", command_table[SEARCH].name);
  return;
  }
/* Go through rooms */
cnt=0;
for(rm=room_first;rm!=NULL;rm=rm->next) {
  if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
    sscanf(rm->name,"%s",rmname);
    rmname[0]=toupper(rmname[0]);
    sprintf(filename,"%s/%s.B", USERROOMS,rmname);
    }
  else {
  	if (rm->transp==NULL) sprintf(filename,"%s/%s.B", ROOMFILES, rm->name);
	else sprintf(filename,"%s/%s.B", TRFILES, rm->name);
	}
  if (!(fp=fopen(filename,"r"))) continue;
  if (!has_room_access(user,rm)) {  fclose(fp);  continue;  }
  /* Go through file */
  fgets(line,81,fp);
  yes=0;  message=0;  
  room_given=0;  buff[0]='\0';
  while(!feof(fp)) {
    if (*line=='\n') {
      if (yes) {  strcat(buff,"\n");  write_user(user,buff);  }
      message=0;  yes=0;  buff[0]='\0';
      }
    if (!message) {
      w1[0]='\0';  
      sscanf(line,"%s",w1);
      if (!strcmp(w1,"PT:")) {  
	message=1;  
	strcpy(buff,remove_first(remove_first(line)));
        }
      }
    else strcat(buff,line);
    for(w=1;w<word_count;++w) {
      if (!yes && strstr(line,word[w])) {  
	if (!room_given) {
	  vwrite_user(user,"~BB*** %s ***\n\n",rm->name);
	  room_given=1;
	  }
	yes=1;  cnt++;  
        }
      }
    fgets(line,81,fp);
    }
  if (yes) {  strcat(buff,"\n");  write_user(user,buff);  }
  fclose(fp);
  }
if (cnt) vwrite_user(user,"Total of %d matching message%s.\n\n",cnt,PLTEXT_S(cnt));
else write_user(user,"No occurences found.\n");
}


/*** See review of conversation ***/
void review(UR_OBJECT user)
{
	RM_OBJECT rm=user->room;
	int i,line,cnt;

	set_crash();
	if (user->restrict[RESTRICT_VIEW]==restrict_string[RESTRICT_VIEW]) {
		write_user(user,">>>Sorry, you have no right to review the conversation.\n");
		return;
		}
	if (word_count<2 || user->level<GOD) rm=user->room;
	else {
		if ((rm=get_room(word[1]))==NULL) {
			write_user(user,nosuchroom);
			return;
			}
		if (!has_room_access(user,rm)) {
			write_user(user, private_review_prompt);
			return;
			}
		vwrite_user(user, review_header, rm->name);
		}
	cnt=0;
	for(i=0;i<REVIEW_LINES;++i) {
		line=(rm->revline+i)%REVIEW_LINES;
		if (rm->revbuff[line][0]) {
			cnt++;
			if (cnt==1) vwrite_user(user, "\n%s\n", center("~BB~FG*** Zaznam kecov pre tuto miestnost ***", 81));
			write_user(user,rm->revbuff[line]); 
			}
		}
	if (!cnt) write_user(user, no_review_prompt);
	else vwrite_user(user, "\n%s\n", center("~BB~FG*** Koniec ***", 81));
}


/*** Show version number and some small stats of the talker ***/
void show_version(UR_OBJECT user)
{
	int i;

	set_crash();
	write_user(user,".----------------------------------------------------------------------------.\n");
	sprintf(text, "(C) %s", reg_sysinfo[SYSOPUNAME]);
	vwrite_user(user,"| ~FT~OLStar talker verzia %-8.8s                           %20.20s~RS |\n", TVERSION, text);
	vwrite_user(user,"| ~FT~OLLotos verzia %-8.8s                         (C) Pavol Hluchy, April 2001~RS |\n", OSSVERSION);
	vwrite_user(user,"| ~FT~OLAmnuts version %-5s                 (C) Andrew Collington, September 1999~RS |\n", AMNUTSVER);
	vwrite_user(user,"| NUTS version %5s                       (C) Neil Robertson, November 1996 |\n", NUTSVER);
	if (user->level>=ARCH) {
	write_user(user,"+----------------------------------------------------------------------------+\n");
	vwrite_user(user,"| Total number of users    : ~OL%-4d~RS  Maximum online users     : ~OL%-3d~RS            |\n",amsys->user_count,amsys->max_users);
	vwrite_user(user,"| Maximum smail copies     : ~OL%-3d~RS   Names can be recapped    : ~OL%s~RS            |\n",MAX_COPIES,noyes2[amsys->allow_recaps]);
	vwrite_user(user,"| Personal rooms active    : ~OL%-3s~RS   Maximum user idle time   : ~OL%-3d~RS mins~RS       |\n",noyes2[amsys->personal_rooms],amsys->user_idle_time/60);
#ifdef NETLINKS
	write_user(user,"| Compiled netlinks        : ~OLANO~RS                                             |\n");
#else
	write_user(user,"| Compiled netlinks        : ~OLNIE~RS                                             |\n");
#endif
	write_user(user,"+----------------------------------------------------------------------------+\n");
	for (i=JAILED;i<=ROOT;i++)
		vwrite_user(user,"| Number of users at level %-6s : ~OL%-4d~RS                                     |\n",user_level[i].alias,amsys->level_count[i]);
		}
	write_user(user,"`----------------------------------------------------------------------------'\n");
}


/*** Show talker rooms ***/
void rooms(UR_OBJECT user, int show_topics, int wrap)
{
RM_OBJECT rm;
UR_OBJECT u;
#ifdef NETLINKS
  NL_OBJECT nl;
  char serv[SERV_NAME_LEN+1],nstat[9];
#endif
char rmaccess[9];
int cnt,rm_cnt,rm_pub,rm_priv,rm_tr;

	set_crash();
if (word_count<2) {
  if (!wrap) user->wrap_room=room_first;
  if (show_topics) write_user(user,"\n~FTRuuma                : pristup Users  sprav  Topic\n\n");
  else write_user(user,"\n~FTRuuma                : pristup Users  sprav  Inlink  LStat  Service\n\n");
  rm_cnt=0;
  for(rm=user->wrap_room;rm!=NULL;rm=rm->next) {
    if (rm->access==PERSONAL_UNLOCKED
        || rm->access==PERSONAL_LOCKED
        ) continue;
    if (rm_cnt==user->terminal.pager-4) {   /* -4 for the 'Room name...' header */
      switch (show_topics) {
	case 0: user->misc_op=10; user->status='R'; break;
	case 1: user->misc_op=11; user->status='R'; break;
	}
      write_user(user, continue2);
      return;
      }
    if (rm->access & PRIVATE) strcpy(rmaccess," ~FRPRIV");
    else strcpy(rmaccess,"  ~FGPUB");
    if (rm->transp!=NULL) {
    	if (rm->access & PRIVATE) strcpy(rmaccess," ~FRTRAN");
    	else strcpy(rmaccess," ~FGTRAN");
    	}
    if (rm->access & FIXED) rmaccess[0]='*';
    if (rm->access==ROOT_CONSOLE) strcpy(rmaccess, "* ~FRRTC");
    cnt=0;
    for(u=user_first;u!=NULL;u=u->next) 
      if (u->type!=CLONE_TYPE && u->room==rm) ++cnt;
    if (show_topics)
      sprintf(text,"%-20s : %9s~RS    %3d    %3d  %s\n",rm->name,rmaccess,cnt,rm->mesg_cnt,rm->topic);
#ifdef NETLINKS
    else {
      nl=rm->netlink;  serv[0]='\0';
      if (nl==NULL) {
	if (rm->inlink) strcpy(nstat,"~FRDOWN");
	else strcpy(nstat,"   -");
        }
      else {
	if (nl->type==UNCONNECTED) strcpy(nstat,"~FRDOWN");
	else if (nl->stage==UP) strcpy(nstat,"  ~FGUP");
	else strcpy(nstat," ~FYVER");
        }
      if (nl!=NULL) strcpy(serv,nl->service);
      sprintf(text,"%-20s : %9s~RS    %3d    %3d     %s   %s~RS  %s\n",rm->name,rmaccess,cnt,rm->mesg_cnt,noyes1[rm->inlink],nstat,serv);
      }
#endif
    write_user(user,text);
    ++rm_cnt;  user->wrap_room=rm->next;
    }
  user->misc_op=0;
  user->status='a';
  rm_pub=rm_priv=0;
  rm_tr=0;
  for (rm=room_first;rm!=NULL;rm=rm->next) {
    if (rm->access==PERSONAL_UNLOCKED || rm->access==PERSONAL_LOCKED) continue;
    if ((rm->access & PRIVATE) || (rm->access==ROOT_CONSOLE)) ++rm_priv;
    else ++rm_pub;
    if (rm->transp!=NULL) rm_tr++;
    }
  vwrite_user(user,"\nSpolu %d ruum%s. %d verejn%s, %d privatn%s.\n",rm_priv+rm_pub, grm_num(8, rm_priv+rm_pub), rm_pub, grm_num(9, rm_pub), rm_priv, grm_num(9, rm_priv));
  vwrite_user(user,"Z toho %d transport%s\n\n", rm_tr, grm_num(7, rm_tr));
  return;
  }
rm_cnt=0;  cnt=0;
if (!strcasecmp(word[1],"level")) {
  write_user(user,"Ruumy specialneho urcenia :\n");
  vwrite_user(user,"Prva    : ~OL%s~RS\n",room_first->name);
  vwrite_user(user,"Hlavna  : ~OL%s~RS\n",default_warp);
  vwrite_user(user,"Vazenie : ~OL%s~RS\n",default_jail);
  vwrite_user(user,"Banka   : ~OL%s~RS\n", default_bank);
  while(priv_room[rm_cnt].name[0]!='*') {
    if (++cnt==1) write_user(user,"\nPristup podla levelu:\n");
    vwrite_user(user,"~FT%s~RS pre level ~OL%s~RS a vyssi.\n",
    	priv_room[rm_cnt].name,user_level[priv_room[rm_cnt].level].name);
    ++rm_cnt;
    }
  if (cnt) vwrite_user(user,"~OL%d~RS ruum%s pristupn%s podla levelu\n",rm_cnt, grm_num(9, rm_cnt), grm_num(9, rm_cnt));
  else write_user(user,"Neni su take ruumy.\n\n");
  return;
  }
write_usage(user,"%s [level]", command_table[RMST].name);
}


/*** Change whether a rooms access is fixed or not ***/
void change_room_fix(UR_OBJECT user, int fix)
{
RM_OBJECT rm;
char *name;

	set_crash();
if (word_count<2) rm=user->room;
else {
  if ((rm=get_room(word[1]))==NULL) {
    write_user(user,nosuchroom);
    return;
    }
  }
if (user->vis) name=user->recap; else name=invisname;
if (fix) {	
  if (rm->access & FIXED) {
    if (rm==user->room) write_user(user,"Tato ruuma uz je fixnuta.\n");
    else write_user(user,"Ta ruuma uz je fixnuta.\n");
    return;
    }
  sprintf(text,"Access for room %s is now ~FRFIXED.\n",rm->name);
  write_user(user,text);
  if (user->room==rm) vwrite_room_except(rm,user,"%s~RS has ~FRFIXED~RS access for this room.\n",name);
  else write_room(rm,"This room's access has been ~FRFIXED.\n");
  write_syslog(SYSLOG,1,"%s FIXED access to room %s.\n",user->name,rm->name);
  rm->access+=2;
  return;
  }
if (!(rm->access & FIXED)) {
  if (rm==user->room) write_user(user,"Tato ruuma neni fixnuta.\n");
  else write_user(user,"Ta ruuma neni fixnuta.\n");
  return;
  }
vwrite_user(user,"Access for room %s is now ~FGUNFIXED.\n",rm->name);
if (user->room==rm) vwrite_room_except(rm,user,"%s~RS has ~FGUNFIXED~RS access for this room.\n",name);
else write_room(rm,"This room's access has been ~FGUNFIXED.\n");
write_syslog(SYSLOG,1,"%s UNFIXED access to room %s.\n",user->name,rm->name);
rm->access-=2;
reset_access(rm);
}


/*** Clear the screen ***/
void cls(UR_OBJECT user)
{
	int i;

	set_crash();
	if (user->terminal.clear) write_user(user, "~CS\n");
	else for (i=0; i<50; ++i)
		write_user(user,"\n");
}


/* show the ranks and commands per level for the talker */
void show_ranks(UR_OBJECT user)
{
	CM_OBJECT pcom;
	int i,total,cnt[ROOT+1];
	char *ptr, chr;
	char *cl[ROOT+1]={
		"~CG", "~CG", "~FG", "~FG", "~FB",
		"~FB", "~FB", "~FM", "~FR", "~FR",
		"~CR", "~CW", "~CT"
		};
	char *al="~CY";

	set_crash();
	for (i=JAILED;i<=ROOT;i++) cnt[i]=0;
	total=i=0;
	while (command_table[i].name[0]!='*') {
		cnt[command_table[i].level]++;
		i++;
		}
	i=0;
	pcom=cmds_first;
	while (pcom!=NULL) {
		cnt[pcom->req_lev]++;
		pcom=pcom->next;
		}
	write_user(user," ~OL~FTLevely na talkri~RS\n");
	write_user(user,"+----------------------------------------------------------------------------+\n");
	for (i=JAILED; i<=ROOT; i++) {
		if (i!=user->level) {
			ptr=cl[i];
			chr=' ';
			}
		else {
			ptr=al;
			chr='>';
			}
		vwrite_user(user,"%s(%s)%c:%c%-15.15s :%clev %2d : %3d prik spolu : %2d prik tento level~RS\n",
			ptr,user_level[i].alias,chr,chr,
			user_level[i].name,chr,i,total+=cnt[i],cnt[i]);
		}
	write_user(user,"+----------------------------------------------------------------------------+\n\n");
}


/* Show the wizzes that are currently logged on, and get a list of names from the lists saved */
void wiz_list(UR_OBJECT user)
{
UR_OBJECT u;
int some_on=0,count=0,cnt,i,inlist;
char text2[ARR_SIZE], temp[ARR_SIZE];
char *clrs[]={"~FT","~FM","~FG","~FB","~OL","~FR","~FY"};
struct wiz_list_struct *entry;

	set_crash();
write_user(user,"+----- ~FGWiz List~RS -------------------------------------------------------------+\n\n");
for (i=ROOT;i>=WIZ;i--) {
  text2[0]='\0';  cnt=0;  inlist=0;
  sprintf(text,"~OL%s%-11s~RS: ",clrs[i%4],user_level[i].name);
  for(entry=first_wiz_entry;entry!=NULL;entry=entry->next) {
    if (in_retire_list(entry->name)) continue;
    if (entry->level==i) {
      if (cnt>3) { strcat(text2,"\n             ");  cnt=0; }
      sprintf(temp,"~OL%s%-*s~RS  ",clrs[rand()%7],USER_NAME_LEN,entry->name);
      strcat(text2,temp);
      cnt++;
      inlist=1;
      }
    }
  if (!cnt && !inlist) sprintf(text2,"~FR[none listed]\n~RS");
  strcat(text,text2);
  write_user(user,text);
  if (cnt) write_user(user,"\n");
  }
write_user(user,"\n+----- ~FGThose currently on~RS ---------------------------------------------------+\n\n");
for (u=user_first;u!=NULL;u=u->next)
if (u->room!=NULL)  {
  if (u->level>=WIZ) {
    if (!u->vis && (user->level<u->level && !(user->level>=ARCH)))  { ++count;  continue;  }
    else {
      if (u->vis) sprintf(text2,"  %s~RS %s~RS",u->recap,u->desc);
      else sprintf(text2,"* %s~RS %s~RS",u->recap,u->desc);
      cnt=colour_com_count(text2);
      vwrite_user(user,"%-*.*s : %15s : (%s) %s\n",43+cnt*3,43+cnt*3,text2,u->room->name,user_level[u->level].alias,user_level[u->level].name);
      }
    }
  some_on=1;
  }
if (count>0) vwrite_user(user,"pocet pre teba neviditelnych wizardof : %d\n",count);
if (!some_on) write_user(user,"Sorac, neni su tu wizardi ...\n");
write_user(user,"\n");
write_user(user,"+----------------------------------------------------------------------------+\n");
}


/*** Get current system time ***/
void get_time(UR_OBJECT user)
{
	char bstr[40],temp[80];
	int secs,mins,hours,days;

	set_crash();
	/* Get some values */
	strcpy(bstr,ctime(&amsys->boot_time));
	bstr[strlen(bstr)-1]='\0';
	secs=(int)(time(0)-amsys->boot_time);
	days=secs/86400;
	hours=(secs%86400)/3600;
	mins=(secs%3600)/60;
	secs=secs%60;
	write_user(user,"     ~OL~FTPresny cas~RS\n");
	write_user(user,"+----------------------------------------------------------------------------+\n");
	sprintf(temp,"%s, %d %s, %02d:%02d:%02d %d",day[twday],tmday,month[tmonth],thour,tmin,tsec,tyear);
	vwrite_user(user,"| Aktualny systemovy cas     : ~OL%-45s~RS |\n",temp);
	if (user->level>=ARCH) {
		vwrite_user(user,"| Start talkra               : ~OL%-45s~RS |\n",bstr);
		sprintf(temp,"%d d, %d h, %d min, %d s",days,hours,mins,secs);
		vwrite_user(user,"| Uptime                     : ~OL%-45s~RS |\n",temp);
		}
	write_user(user,"+----------------------------------------------------------------------------+\n\n");
}


/*** This command allows you to do a search for any user names that match
     a particular pattern ***/
void grep_users(UR_OBJECT user)
{
int found,x;
char name[USER_NAME_LEN+1],pat[ARR_SIZE];
struct user_dir_struct *entry;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <retazec>", command_table[GREPUSER].name);
  return;
  }
if (strstr(word[1],"**")) {
  write_user(user,"You cannot have ** in your pattern.\n");
  return;
  }
if (strstr(word[1],"?*")) {
  write_user(user,"You cannot have ?* in your pattern.\n");
  return;
  }
if (strstr(word[1],"*?")) {
  write_user(user,"You cannot have *? in your pattern.\n");
  return;
  }
write_user(user,"\n+----------------------------------------------------------------------------+\n");
sprintf(text,"| ~FT~OLUser grep for pattern:~RS ~OL%-51s~RS |\n",word[1]);
write_user(user,text);
write_user(user,"+----------------------------------------------------------------------------+\n");
x=0; found=0; pat[0]='\0';
strcpy(pat,word[1]);
strtolower(pat);
for (entry=first_dir_entry;entry!=NULL;entry=entry->next) {
  strcpy(name,entry->name);
  name[0]=tolower(name[0]);
  if (pattern_match(name,pat)) {
    if (!x) vwrite_user(user,"| %-*s  ~FT%-20s~RS   ",USER_NAME_LEN,entry->name,user_level[entry->level].name);
    else vwrite_user(user,"   %-*s  ~FT%-20s~RS |\n",USER_NAME_LEN,entry->name,user_level[entry->level].name);
    x=!x;
    ++found;
    }
  }
if (x) write_user(user,"                                      |\n");
if (!found) {
  write_user(user,"|                                                                            |\n");
  write_user(user,"| ~OL~FRNo users have that pattern~RS                                                 |\n");
  write_user(user,"|                                                                            |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  return;
  }
write_user(user,"+----------------------------------------------------------------------------+\n");
vwrite_user(user,"  ~OL%d~RS user%s had the pattern ~OL%s\n",found,PLTEXT_S(found),word[1]);
write_user(user,"+----------------------------------------------------------------------------+\n\n");
}


/*** Shoot another user... Fun! Fun! Fun! ;) ***/
void shoot_user(UR_OBJECT user)
{
UR_OBJECT user2;
RM_OBJECT rm;
int prob1,prob2;

	set_crash();
rm=get_room(default_shoot);
if (rm==NULL) {
  write_user(user,"Nemas kde strielat.\n");
  return;
  }
if (user->room!=rm) {
  vwrite_user(user,"Tu nemozes strielat. Chod do ruumy ~OL%s~RS.\n",rm->name);
  return;
  }
if (word_count<2) {
  if (user->bullets==0) {
    vwrite_room_except(rm,user,"%s~RS's gun goes *click* as they pull the trigger.\n",user->recap);
    write_user(user,"Your gun goes *click* as you pull the trigger.\n");
    return;
    }
  vwrite_room_except(rm,user,"%s~RS fires their gun off into the air.\n",user->recap);
  write_user(user,"You fire your gun off into the air.\n");
  --user->bullets;
  return;
  }
prob1=rand()%100;
prob2=rand()%100;
if (!(user2=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);  return;
  }
if (!user->vis) {
  write_user(user,"Be fair!  At least make a decent target - don't be invisible!\n");
  return;
  }
if ((!user2->vis && user2->level<user->level) || user2->room!=rm) {
  write_user(user,"You cannot see that person around here.\n");
  return;
  }
if (user==user2) {
  write_user(user,"Watch it!  You might shoot yourself in the foot!\n");
  return;
  }
if (user->bullets==0) {
  vwrite_room_except(rm,user,"%s~RS's gun goes *click* as they pull the trigger.\n",user->recap);
  write_user(user,"Your gun goes *click* as you pull the trigger.\n");
  return;
  }
if (prob1>prob2) {
  vwrite_room(rm,"A bullet flies from %s~RS's gun and ~FR~OLhits~RS %s.\n",user->recap,user2->recap);
  --user->bullets;
  ++user->hits;
  --user2->hps;
  write_user(user2,"~FR~OLYou've been hit!\n");
  write_user(user,"~FG~OLGood shot!\n");
  if (user2->hps<1) {
    ++user2->deaths;
    vwrite_user(user,"\nYou have won the shoot out, %s~RS is dead!  You may now rejoice!\n",user2->recap);
    write_user(user2,"\nYou have received a fatal wound, and you feel your warm ~FRblood ~OLooze~RS out of you.\n");
    write_user(user2,"The room starts to fade and grow grey...\n");
    write_user(user2,"In the bleak mist of Death's shroud you see a man walk towards you.\n");
    write_user(user2,"The man is wearing a tall black hat, and a wide grin...\n\n");
    user2->hps=5*user2->level;
    write_syslog(SYSLOG,1,"%s shot dead by %s\n",user2->name,user->name);
    disconnect_user(user2);
    ++user->kills;
    user->hps=user->hps+5;
    return;
    }
  return;
  }
vwrite_room(rm,"A bullet flies from %s~RS's gun and ~FG~OLmisses~RS %s~RS.\n",user->recap,user2->recap);
--user->bullets;
++user->misses;
write_user(user2,"~FGThat was a close shave!\n");
write_user(user,"~FRYou couldn't hit the side of a barn!\n");
}


/*** well.. Duh!  Reload the gun ***/
void reload_gun(UR_OBJECT user)
{
RM_OBJECT rm;

	set_crash();
rm=get_room(default_shoot);
if (rm==NULL) {
  write_user(user,"Nemas kde strielat.\n");
  return;
  }
if (user->room!=rm) {
  vwrite_user(user,"Tu nemozes trielat. Chod do ruumy ~OL%s~RS.\n",rm->name);
  return;
  }
if (user->bullets>0) {
  vwrite_user(user,"You have ~OL%d~RS bullets left.\n",user->bullets);
  return;
  }
vwrite_room_except(user->room,user,"~FY%s si nabil kanon.\n",user->bw_recap);
vwrite_user(user,"~FYNabil%s si si zbran.\n", grm_gnd(4, user->gender));
user->bullets=6;
}


/* display the calendar to the user */
void show_calendar(UR_OBJECT user) {
int iday,day_1,numdays,j;
unsigned yr,mo;
char temp[ARR_SIZE];

if (word_count>3) {
  write_usage(user,"%s [<m> [<y>]]", command_table[CALENDAR].name);
  write_user(user,"   kde <m> = mesiac od 1 po 12\n");
  write_user(user,"       <y> = rok od 1 po 99, alebo 1800 az 3000\n");
  return;
  }
/* full date given */
if (word_count==3) {
  yr=atoi(word[2]);
  mo=atoi(word[1]);
  if (yr<100) yr+=1900;
  if ((yr>3000) || (yr<1800) || !mo || (mo>12)) {
    write_usage(user,"%s [<m> [<y>]]", command_table[CALENDAR].name);
    write_user(user,"   kde <m> = mesiac od 1 po 12\n");
    write_user(user,"       <y> = rok od 1 po 99, alebo 1800 po 3000\n");
    return;
    }
  }
/* only month given, so show for this year */
else if (word_count==2) {
  yr=tyear;
  mo=atoi(word[1]);
  if (!mo || (mo>12)) {
    write_usage(user,"%s [<m> [<y>]]", command_table[CALENDAR].name);
    write_user(user,"   kde <m> = mesiac od 1 po 12\n");
    write_user(user,"       <y> = rok od 1 po 99, alebo 1800 po 3000\n");
    return;
    }
  }
/* todays month and year */
else {
  yr=tyear;
  mo=tmonth+1;
  }
/* show calendar */
numdays=cal_days[mo-1];
if (2==mo && is_leap(yr)) ++numdays;
day_1=(int)((ymd_to_scalar(yr,mo,1) - (long)ISO)%7L);
temp[0]='\n';  text[0]='\n';
write_user(user,"\n+-----------------------------------+\n");
write_user(user,center_string(37,1,"|","~OL~FT%s %d~RS",month[mo-1],yr));
write_user(user,"+-----------------------------------+\n");
text[0]='\0';
strcat(text,"  ");
for (j=0;j<7;) {
   sprintf(temp,"~OL~FY%s~RS",cal_daynames[ISO+j]);
   strcat(text,temp);
   if (7!=++j) strcat(text,"  ");
   }
strcat(text,"\n+-----------------------------------+\n");
for (iday=0;iday<day_1;++iday) strcat(text,"     ");
for (iday=1;iday<=numdays;++iday,++day_1,day_1%=7) {
   if (!day_1 && 1!=iday) strcat(text,"\n\n");
   if ((has_reminder(user,iday,mo,yr))) {
     sprintf(temp," ~OL~FR%3d~RS ",iday);
     strcat(text,temp);
     }
   else if (is_ymd_today(yr,mo,iday)) {
     sprintf(temp," ~OL~FG%3d~RS ",iday);
     strcat(text,temp);
     }
   else {
     sprintf(temp," %3d ",iday);
     strcat(text,temp);
     }
   }
for (;day_1;++day_1,day_1%=7) strcat(text,"      ");
strcat(text,"\n+-----------------------------------+\n\n");
write_user(user,text);
}


/* allows a user to lock their room out to access from anyone */
void personal_room_lock(UR_OBJECT user) {
char name[ROOM_NAME_LEN+1];
RM_OBJECT rm;

if (!amsys->personal_rooms) {
  write_user(user, prooms_disabled);
  return;
  }
sprintf(name,"(%s)",user->name);
strtolower(name);
/* get room that user is in */
if ((rm=get_room_full(name))==NULL) {
  write_user(user,"Sorry, but you cannot use the room locking feature at this time.\n");
  return;
  }
if (user->room!=rm) {
  write_user(user,"You have to be in your personal room to lock and unlock it.\n");
  return;
  }
switch(rm->access) {
  case PERSONAL_UNLOCKED:
    rm->access=PERSONAL_LOCKED;
    write_user(user,"You have now ~OL~FRlocked~RS your room to all the other users.\n");
    break;
  case PERSONAL_LOCKED:
    rm->access=PERSONAL_UNLOCKED;
    write_user(user,"You have now ~OL~FGunlocked~RS your room to all the other users.\n");
    break;
  }
if (!personal_room_store(user->name,1,rm))
  write_syslog(ERRLOG,1,"Unable to save personal room status in personal_room_decorate()\n");
}


/*** Enter a description for a personal room ***/
void personal_room_decorate(UR_OBJECT user, int done_editing) {
char *c,name[ROOM_NAME_LEN+1];
RM_OBJECT rm;
int i;

if (!amsys->personal_rooms) {
  write_user(user, prooms_disabled);
  return;
  }
if (!done_editing) {
  sprintf(name,"(%s)",user->name);
  strtolower(name);
  /* get room that user is in */
  if ((rm=get_room_full(name))==NULL) {
    write_user(user,"Sorry, but you cannot use the room decorating feature at this time.\n");
    return;
    }
  if (user->room!=rm) {
    write_user(user,"najprv musis byt vo svojej osobne ruumke.\n");
    return;
    }
  write_user(user, entroom_edit_header);
  user->misc_op=19;
  editor(user,NULL);
  return;
  }
/* rm should be personal room as check is done above */
rm=user->room;
i=0;
c=user->malloc_start;
while(c!=user->malloc_end) {
  if (i==ROOM_DESC_LEN) {
    vwrite_user(user,"Pridlhy popis pre ruumu '%s'.\n",rm->name);
    write_syslog(ERRLOG,1,"Description too long when reloading for room %s.\n",rm->name);
    break;
    } /* end if */
  rm->desc[i++]=*c++;
  }
rm->desc[i]='\0';
write_user(user,"Mas novu vymalovku vo svojej ruumke.\n");
if (!personal_room_store(user->name,1,rm))
  write_syslog(ERRLOG,1,"Unable to save personal room status in personal_room_decorate()\n");
}


/*** this function allows users to give access to others even if their personal room
     has been locked
     ***/
void personal_room_key(UR_OBJECT user) {
char name[ROOM_NAME_LEN+1],filename[500],line[USER_NAME_LEN+2],text2[ARR_SIZE];
RM_OBJECT rm;
FILE *fp;
int cnt=0;

if (!amsys->personal_rooms) {
  write_user(user, prooms_disabled);
  return;
  }
sprintf(name,"(%s)",user->name);
strtolower(name);
/* see if user has a room created */
if ((rm=get_room_full(name))==NULL) {
  write_user(user,"nemas este osobnu ruumku.\n");
  return;
  }
/* if no name was given then display keys given */
if (word_count<2) {
  sprintf(filename,"%s/%s.K", USERROOMS,user->name);
  if (!(fp=fopen(filename,"r"))) {
    write_user(user,"You have not given anyone a personal room key yet.\n");
    return;
    }
  write_user(user,"+----------------------------------------------------------------------------+\n");
  write_user(user,"| ~OL~FTYou have given the following people a key to your room~RS                     |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  text2[0]='\0';
  fscanf(fp,"%s",line);
  while(!feof(fp)) {
    switch (++cnt) {
      case 1: sprintf(text,"| %-24s",line);  strcat(text2,text);  break;
      case 2: sprintf(text," %-24s",line);  strcat(text2,text);  break;
      default:
	sprintf(text," %-24s |\n",line);
	strcat(text2,text);
	write_user(user,text2);
	cnt=0;  text2[0]='\0';
	break;
      }
    fscanf(fp,"%s",line);
    }
  fclose(fp);
  if (cnt==1) {
    strcat(text2,"                                                   |\n");
    write_user(user,text2);
    }
  else {
    if (cnt==2) strcat(text2,"                          |\n");
    write_user(user,text2);
    }
  write_user(user,"+----------------------------------------------------------------------------+\n");
  return;
  }
strtolower(word[1]);
word[1][0]=toupper(word[1][0]);
if (!strcmp(user->name,word[1])) {
  write_user(user,"You already have access to your own room!\n");
  return;
  }
/* check to see if the user is already listed before the adding part.  This is to
   ensure you can remove a user even if they have, for instance, suicided.
   */
if (has_room_key(word[1],rm)) {
  if (personal_key_remove(user,word[1])) {
    vwrite_user(user,"You take your personal room key away from ~FT~OL%s~RS.\n",word[1]);
    vwrite_user(get_user(word[1]),"%s takes back their personal room key.\n",user->name);
    return;
    }
  else {
    write_user(user,"There was an error taking the key away from that user.\n");
    return;
    }
  }
/* see if there is such a user */
if (!find_user_listed(word[1])) {
  write_user(user,nosuchuser);
  return;
  }
/* give them a key */
if (personal_key_add(user,word[1])) {
  vwrite_user(user,"You give ~FT~OL%s~RS a key to your personal room.\n",word[1]);
  vwrite_user(get_user(word[1]),"%s gives you a key to their room.\n",user->name);
  }
else write_user(user,"There was an error taking the key away from that user.\n");
}


/*** allow a user to bump others from their personal room ***/
void personal_room_bgone(UR_OBJECT user)
{
RM_OBJECT rm, rmto;
UR_OBJECT u;
char name[ROOM_NAME_LEN+1];

	set_crash();
if (!amsys->personal_rooms) {
  write_user(user, prooms_disabled);
  return;
  }
if (word_count<2) {
  write_usage(user,"%s <user>/all", command_table[MYBGONE].name);
  return;
  }
sprintf(name,"(%s)",user->name);
strtolower(name);
/* get room that user is in */
if ((rm=get_room_full(name))==NULL) {
  write_user(user,"Sorry, but you cannot use the personal room bgone feature at this time.\n");
  return;
  }
if (user->room!=rm) {
  write_user(user,"You have to be in your personal room bounce people from it.\n");
  return;
  }
/* get room to bounce people to */
if (!(rmto=get_room(default_warp))) {
  write_user(user,"No one can be bounced from your personal room at this time.\n");
  return;
  }
strtolower(word[1]);
/* bounce everyone out - except GODS */
if (!strcmp(word[1],"all")) {
  for (u=user_first;u!=NULL;u=u->next) {
    if (u==user || u->room!=rm || u->level>=GOD) continue;
    vwrite_user(user,"%s~RS is forced to leave the room.\n",u->recap);
    write_user(u,"You are being forced to leave the room.\n");
    move_user(u,rmto,0);
    }
  return;
  }
/* send out just the one user */
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (u->room!=rm) {
  vwrite_user(user,"%s neni v tvojej osobnej ruumke.\n",u->name);
  return;
  }
if (u->level>=GOD) {
  vwrite_user(user,"%s cannot be forced to leave your personal room.\n",u->name);
  return;
  }
vwrite_user(user,"%s~RS is forced to leave the room.\n",u->recap);
write_user(u,"You are being forced to leave the room.\n");
move_user(u,rmto,0);
}


/*** Display some files to the user.  This was once intergrated with the '.help' command,
     but due to the new processing that goes through it was moved into its own command.
     The files listed are now stored in MISCFILES rather than HELPFILES as they may not
     necessarily be for any help commands.
***/
void display_files(UR_OBJECT user,int admins)
{
char filename[500],*c;
int ret;

	set_crash();
if (word_count<2) {
  if (!admins) strcpy(filename, SHOWFILES);
  else strcpy(filename, SHOWAFILES);
  if (!(ret=more(user,user->socket,filename))) {
    if (!admins) write_user(user,"Neni zoznam suborof.\n");
    else write_user(user,"Neni zoznam adminskyx textof.\n");
    return;
    }
  if (ret==1) user->misc_op=2;
  return;
  }
/* check for any illegal characters */
c=word[1];
while(*c) {
  if (*c=='.' || *c++=='/') {
    if (!admins) write_user(user,"Neni taky text.\n");
    else write_user(user,"Neni taky adminsky text.\n");
    return;
    }
  }
/* show the file */
if (!admins) sprintf(filename,"%s/%s", TEXTFILES,word[1]);
else sprintf(filename,"%s/%s", ADMINFILES, word[1]);
if (!(ret=more(user,user->socket,filename))) {
  if (!admins) write_user(user,"Neni taky text.\n");
  else write_user(user,"Neni taky adminsky text.\n");
  return;
  }
if (ret==1) user->misc_op=2;
return;
}


/*****************************************************************************/
/* Doplnene funkcie                                                          */
/*****************************************************************************/

void main_help(UR_OBJECT user)
{
	int ret;
	
	set_crash();
	if (!(ret=more(user,user->socket,MAINHELP))) {
		write_user(user,"Sorrac, momentalne neni help.\n");
		return;
		}
	if (ret==1) user->misc_op=2;
	return;
}


void volby(UR_OBJECT user)
{
	FILE *fp;
	DIR *dirp;
	struct dirent *dp;
	int ret, i, p1, p2, pm, v=0;
	int pochlasov;
	char filename[500], dirname[500];
	char *pn;

	set_crash();
	if (word_count<2) {
		sprintf(filename, "%s/mainlist", VOTEFILES);
		if (!(ret=more(user,user->socket,filename))) {
			write_user(user,"Sorrac, momentalne nenisu volby.\n");
			return;
			}
		if (ret==1) user->misc_op=2;
		return;
		}
	p1=atoi(word[1]);
	if (word_count==2) {
		if (!is_number(word[1])) {
			write_user(user, "Chybny parameter !\n");
			return;
			}
		sprintf(filename,"%s/vote_%s", VOTEFILES, word[1]);
		if (!more(user,user->socket,filename)) {
			write_user(user, "Nemozem najst subor s inf. o hlasovani - asi take prave neprebieha ...\n");
			return;
			}
		sprintf(dirname, "%s/%d", VOTEFILES, p1);
		if (!(dirp=opendir(dirname))) {
			write_user(user, "Nemozem otvorit adresar s vysledkami hlasovania - asi take prave neprebieha ...\n");
			return;
			}
		pm=0;
		while ((dp=readdir(dirp))!=NULL) {
			if (!strcmp(dp->d_name,".")
			    || !strcmp(dp->d_name,"..")
			    || dp->d_name[0]=='_'
			    || !strstr(dp->d_name, "_")
			    ) continue;
			pn=strchr(dp->d_name, '_');
			pn++;
			if ((i=atoi(pn))>pm) pm=i;
			}
		(void)closedir(dirp);
		for(i=1; i<=pm; i++) {
			sprintf(filename,"%s/%d/v_%d", VOTEFILES, p1 ,i);
			if ((fp=fopen(filename,"r"))!=NULL) {
				pochlasov=0;
				while(!feof(fp)) {
					text[0]='\0';
					fscanf(fp,"%s\n", text);
					if (strlen(text)) {
						pochlasov++;
						if (!strcmp(user->name, text)) v=1;
						}
					}
				fclose(fp);
				vwrite_user(user, "moznost %d : %d hlas%s\n",i,pochlasov, grm_num(7, pochlasov));
				}
			}
		write_user(user,"\n");
		vwrite_user(user, "V tychto volbach si uz hlasoval%s: %s\n",
			grm_gnd(4, user->gender), noyes1[v]);
		return;
		}

	sprintf(dirname, "%s/%d", VOTEFILES, p1);
	if (!(dirp=opendir(dirname))) {
		write_user(user, "Nemozem otvorit adresar s vysledkami hlasovania - asi take prave neprebieha ...\n");
		return;
		}
	pm=0;
	while ((dp=readdir(dirp))!=NULL) {
		if (!strcmp(dp->d_name,".")
		    || !strcmp(dp->d_name,"..")
		    || dp->d_name[0]=='_'
		    || !strstr(dp->d_name, "_")
		    ) continue;
		pn=strchr(dp->d_name, '_');
		pn++;
		if ((i=atoi(pn))>pm) pm=i;
		}
	(void) closedir(dirp);

	if(!is_number(word[1]) || !is_number(word[2])) {
		write_user(user, "Chybny parameter ...\n");
		return;
		}
	p1=atoi(word[1]);
	p2=atoi(word[2]);
	for(i=1; i<pm; i++) {
		sprintf(filename,"%s/%d/v_%d", VOTEFILES, p1 , i);
		if ((fp=fopen(filename,"r"))!=NULL) {
			while(!feof(fp)) {
				fscanf(fp,"%s\n",text);
				if (!strcmp(user->name, text)) {
					vwrite_user(user, "Nikto nemoze hlasovat dva krat v tyx istyx volbax.\n");
					fclose(fp);
					return;
					}
				}
			}
		fclose(fp);
		}

	sprintf(filename,"%s/%d/v_%d", VOTEFILES, p1, p2);
	if (!(fp=fopen(filename,"r"))) {
		write_user(user,"Sorry, taka moznost neexisuje.\n");
		return;
		}
	fclose(fp);
	fp=fopen(filename,"a");
	fprintf(fp,"%s\n",user->name);
	fclose(fp);

	write_user(user,"Uspesne odovzdany hlas. Gratulujem.\n");
}


/*** Quits... ***/
void quit_user(UR_OBJECT user, char *inpstr)
{
	set_crash();
	if (word_count<2 || user->muzzled) {
		disconnect_user(user);
		return;
		}
	if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
		switch(amsys->ban_swearing) {
			case SBMIN:
				inpstr=censor_swear_words(inpstr);
				break;
			case SBMAX:
				write_user(user,noswearing);
				return;
			default : break; /* do nothing as ban_swearing is off */
			}
		}
	sprintf(text,"\n>>>%s uz odchadza z ~FT~OL%s~RS [ %s~RS ]\n",
		user->name, reg_sysinfo[TALKERNAME], inpstr
		);
	write_room_except(NULL,text, user);
	disconnect_user(user);
}


/*** show map(s) ***/
void show_map(UR_OBJECT user)
{
	DIR *dirp;
	struct dirent *dp;
	int i, cnt;
	char filename[500], mapname[ROOM_NAME_LEN+1];

	set_crash();
	if (word_count<2 || user->level<ARCH) {
		if (user->room==NULL) {
			write_user(user,"You don't need no map - where you are is where it's at!\n");
			return;
			}
		sprintf(filename, "%s/%s.map", MAPFILES, user->room->map);
		switch(more(user,user->socket,filename)) {
			case 0:
				write_user(user,">>> Neni mapa. Sorac...\n");
				break;
			case 1:
				user->misc_op=2;
			}
		return;	
		}

	if(!strcmp(word[1],"-l")) {
		write_user(user,"~BB--->>> Zoznam map <<<---\n");
		if (!(dirp=opendir(MAPFILES))) {
			write_user(user, "Nemozem otvorit adresar s mapami - nie su mapy :(\n");
			return;
			}
		i=0;
		cnt=0;
		while ((dp=readdir(dirp))!=NULL) {
			if (!strcmp(dp->d_name, ".")
			    || !strcmp(dp->d_name, "..")
			    || !strstr(dp->d_name, ".map")
			    || !strncmp(dp->d_name, ".map", 4)
			    ) continue;
			strcpy(mapname, dp->d_name);
			strchr(mapname, '.')[0]='\0';
			vwrite_user(user, "%-19.19s", mapname);
			i++;
			cnt++;
			if (i==4) {
				write_user(user, "\n");
				i=0;
				}
			}
		(void)closedir(dirp);
		write_user(user, "\n");
		if (cnt==0) write_user(user, "\nNie su mapy ...\n");
		else vwrite_user(user, "\n~OLSpolu %d map%s\n", cnt, grm_num(8, cnt));
		return;
		}

	sprintf(filename, "%s/%s.map", MAPFILES, word[1]);
	vwrite_user(user, "\nMapa '~FT~OL%s~RS' :\n", word[1]);
	switch(more(user, user->socket, filename)) {
		case 0:
			write_user(user, "Neni taka mapa.\n");
			break;
		case 1:
			user->misc_op=2;
		}
}


void list_cmdas(UR_OBJECT user)
{
	int i=0;

	set_crash();
	write_user(user, "~FG~OLZoznam pouzitelnych skratiek :\n");
	for (i=0; command_table[i].name[0]!='*'; i++) {
		if (command_table[i].level>user->level) continue;
		if (!strlen(command_table[i].alias)) continue;
		vwrite_user(user, "%-15.15s   %s\n",
			command_table[i].name, command_table[i].alias);
		}
}

#endif /* ct_general.c */
