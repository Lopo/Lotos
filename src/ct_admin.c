/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                  Funkcie pre Lotos v1.2.0 na administratorov
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_ADMIN_C__
#define __CT_ADMIN_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_pl.h"
#include "obj_sys.h"
#include "obj_syspp.h"
#include "ct_admin.h"
#include "comvals.h"


/*** Shutdown talker interface func. Countdown time is entered in seconds so
	we can specify less than a minute till reboot. ***/
void shutdown_com(UR_OBJECT user)
{
	set_crash();
if (amsys->rs_which==1) {
  write_user(user,"The reboot countdown is currently active, you must cancel it first.\n");
  return;
  }
if (!strcmp(word[1],"-cancel")) {
  if (!amsys->rs_countdown || amsys->rs_which!=0) {
    write_user(user,"The shutdown countdown is not currently active.\n");
    return;
    }
  if (amsys->rs_countdown && !amsys->rs_which && amsys->rs_user==NULL) {
    write_user(user,"Someone else is currently setting the shutdown countdown.\n");
    return;
    }
  write_room(NULL,"~OLSYSTEM:~RS~FG Shutdown cancelled.\n");
  write_syslog(SYSLOG,1,"%s cancelled the shutdown countdown.\n",user->name);
  amsys->rs_countdown=0;
  amsys->rs_announce=0;
  amsys->rs_which=-1;
  amsys->rs_user=NULL;
  return;
  }
if (word_count>1 && !is_number(word[1])) {
  write_usage(user,"%s [<secs>]|[-cancel]", command_table[SHUTDOWN].name);
  return;
  }
if (amsys->rs_countdown && !amsys->rs_which) {
  write_user(user,"The shutdown countdown is currently active, you must cancel it first.\n");
  return;
  }
if (word_count<2) {
  amsys->rs_countdown=0;  
  amsys->rs_announce=0;
  amsys->rs_which=-1; 
  amsys->rs_user=NULL;
  }
else {
  amsys->rs_countdown=atoi(word[1]);
  amsys->rs_which=0;
  }
save_counters();
audioprompt(user, 4, 0);
write_user(user,"\n\07~FR~OL~LI*** POZOR - Toto vypne talker ! ***\n\nNaozaj to chces (y/n)? ");
user->misc_op=1;  
no_prompt=1;  
}


/*** Kill a user ***/
void kill_user(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT victim;
	FILE *fp;
	char *name, *msg_txt=NULL;
	char line[200], filename[500];
	int msg;

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user> [<typ>/<text>/!/-l]", command_table[KILL].name);
		return;
		}
	if (!strcmp(inpstr, "-l")) {
		switch(more(user, user->socket, KILLLIST)) {
			case 0:
				write_user(user, "Neni zoznam 'kill hlasok'\n");
				return;
			case 1:
				user->misc_op=2;
			}
		return;
		}
	msg=0;
	if (word_count>2) {
		if (is_number(word[2])) msg=atoi(word[2]);
		else if (word[2][0]=='!') msg=rand()%syspp->kill_msgs;
		if (!msg) {
			msg=-1;
			msg_txt=remove_first(inpstr);
			}
		}
	if (!(victim=get_user_name(user,word[1]))) {
		write_user(user,notloggedon);
		return;
		}
	if (user==victim) {
		write_user(user,"Trying to commit suicide this way is the sixth sign of madness.\n");
		return;
		}
	if (victim->level>=user->level) {
		write_user(user,"You cannot kill a user of equal or higher level than yourself.\n");
		vwrite_user(victim,"%s~RS tried to kill you!\n",user->recap);
		return;
		}
	if (victim->restrict[RESTRICT_KILL]==restrict_string[RESTRICT_KILL]) {
		write_user(user,">>>You cannot kill a protected user!\n");
		vwrite_user(victim, ">>>%s tried to kill you, but you are under protection :)\n",user->vis?user->name:invisname);
		return;
		}
	write_syslog(SYSLOG,1,"%s KILLED %s.\n",user->name,victim->name);
	write_user(user, kill_user_chant);
	if (user->vis) name=user->bw_recap;
	else name=invisname;
	vwrite_room_except(user->room,user, kill_room_chant, name);
	/* display random kill message.  if you only want one message to be displayed
	   then only have one message listed in kill_mesgs[].
	   */
	if (!msg) {
		write_user(victim, "You are killed\n");
		vwrite_room_except(victim->room, victim, "%s is killed\n", victim->bw_recap);
		sprintf(text,"~FRKilled~RS by %s.\n",user->name);
		add_history(victim->name, 1, text);
		disconnect_user(victim);
		write_monitor(user, NULL, 0);
		return;
		}
	if (msg==(-1)) {
		vwrite_user(victim, "You are killed (%s~RS)\n", msg_txt);
		vwrite_room_except(victim->room, victim, "%s is killed (%s~RS)\n",
			victim->bw_recap, msg_txt);
		sprintf(text,"~FRKilled~RS by %s (%s~RS).\n",
			user->name, msg_txt);
		add_history(victim->name, 1, text);
		disconnect_user(victim);
		write_monitor(user, NULL, 0);
		return;
		}
	sprintf(filename, "%s/kill.%d", KILLMSGS, msg);
	if (!(fp=fopen(filename, "r"))) {
		write_user(user, "Nemozem otvorit subor s hlaskou ...\n");
		write_user(victim, "You are killed\n");
		vwrite_room_except(victim->room, victim, "%s is killed\n", victim->bw_recap);
		sprintf(text,"~FRKilled~RS by %s.\n", user->name);
		add_history(victim->name, 1, text);
		disconnect_user(victim);
		write_monitor(user, NULL, 0);
		return;
		}
	else {
		fgets(line, 81, fp); /* preskocenie prveho riadku */
		fgets(line, 81, fp);
		while (!feof(fp)) {
			replace_string(line, "{uname}", user->name);
			replace_string(line, "{ulevel}", user_level[user->level].name);
			replace_string(line, "{kname}", victim->name);
			replace_string(line, "{klevel}", user_level[victim->level].name);
			write_user(user, line);
			fgets(line, 81, fp);
			write_room_except(victim->room, line, user);
			}
		fclose(fp);
		sprintf(text,"~FRKilled~RS by %s.\n", user->name);
		add_history(victim->name, 1, text);
		disconnect_user(victim);
		write_monitor(user, NULL, 0);
		}
}


/*** Promote a user ***/
void promote(UR_OBJECT user, char *inpstr)
{
UR_OBJECT u;
char text2[180],*name, *msg=NULL, *tmp;
int level;

	set_crash();
level=-1;
if (word_count<2) {
  write_usage(user,"%s <user> [<level>][<dovod>]", command_table[PROMOTE].name);
  return;
  }
if (word_count>2) {
  tmp=strdup(word[2]);
  strtoupper(tmp);
  if ((level=get_level(tmp))==-1) msg=remove_first(inpstr);
  else if (word_count>3) msg=remove_first(remove_first(inpstr));
  free(tmp);
  }
if (level>=user->level-1) {
	write_user(user, "Nemozes promotnut usera na tvoj alebo vyssi level\n");
	return;
	}
if (level>GOD) {
	vwrite_user(user, "Nemozes promotnut usera na level %s a vyssi\n", user_level[GOD+1].name);
	return;
	}
/* See if user is on atm */
if ((u=get_user(word[1]))!=NULL) {
  /* first, gotta reset the user's level if they've been temp promoted */
  if (u->real_level<u->level) u->level=u->real_level;
 if ((word_count>2) && (level==-1)) level=u->level+1;
  /* now runs checks if level option was used */
  if (word_count>2 && level<=u->level) {
    write_user(user,"You cannot promote a user to a level less than or equal to what they are now.\n");
    return;
    }
  if (word_count>2 && level>=user->level) {
    write_user(user,"You cannot promote a user to a level higher than or equal to your own.\n");
    return;
    }
  /* now normal checks */
  if (u->level>=user->level) {
    write_user(user,"You cannot promote a user to a level higher than your own.\n");
    return;
    }
  if (u->level==JAILED) {
    vwrite_user(user,"You cannot promote a user of level %s.\n",user_level[JAILED].name);
    return;
    }
  if (u->level>=GOD) {
  	vwrite_user(user, "Nemozes promotnut usera s levelom %s a vyssim\n",user_level[u->level].name);
  	return;
  	}
  if (u->restrict[RESTRICT_PROM]==restrict_string[RESTRICT_PROM]) {
	if (user->level<ROOT) {
		write_user(user,">>>Tento user nemoze byt promotnuty ! (restrict)\n");
		return;
		}
	vwrite_user(user, "%s ma nastavene ~CRrestrict~RS na promote !\n", u->name);
	}
	if (u->accreq!=-1) { /* check autopromote first */
		vwrite_user(user, "~FR~OL~LI>>>~RS~OL~FRNa %s este nebolo vykonane autopromote !\n", u->namel);
		if (user->level<ROOT)
			return;
		}
  if (user->vis) name=user->bw_recap;
  else name=invisname;
  if (u->level>=WIZ) rem_wiz_node(u->name);
  --amsys->level_count[u->level];
  (word_count>2)?u->level=level:u->level++; 
  u->unarrest=u->level;
  user_list_level(u->name,u->level);
  strcpy(u->date,(long_date(1)));
  ++amsys->level_count[u->level];
  if (u->level>=WIZ) add_wiz_node(u->name,u->level);
  sprintf(text,"~FG~OL%s bol%s povysen%s na level: ~RS~OL%s.\n",
    u->bw_recap, grm_gnd(4, u->gender), grm_gnd(1, u->gender), user_level[u->level].name);
  write_level(u->level,1,text,u);
  if (msg!=NULL) {
  	sprintf(text, "~OLDovod:~RS %s\n", msg);
  	write_level(u->level, 1, text, u);
	}
  vwrite_user(u,"~FG~OL%s ta povysil%s na level: ~RS~OL%s!\n",
    name, grm_gnd(4, user->gender), user_level[u->level].name);
  if (msg!=NULL) vwrite_user(u, "~OLDovod:~RS %s\n", msg);
  write_syslog(SYSLOG,1,"%s PROMOTED %s to level %s, %s\n",user->name,u->name,user_level[u->level].name, msg);
  sprintf(text,"Was ~FGpromoted~RS by %s to level %s, %s\n",user->name,user_level[u->level].name, msg);
  add_history(u->name,1,text);
  u->accreq=1;
  u->real_level=u->level;
  add_user_date_node(u->name,(long_date(1)));
  return;
  }
/* Create a temp session, load details, alter , then save. This is inefficient
   but its simpler than the alternative */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in promote().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);
  destruct_user(u);
  destructed=0;
  return;
  }
	if (u->restrict[RESTRICT_PROM]==restrict_string[RESTRICT_PROM]) {
		write_user(user,">>>This user cannot be promoted !\n");
		destruct_user(u);
		destructed=0;
		return;
		}
if (word_count>2 && level==-1) level=u->level+1;
if (word_count>2 && level<=u->level) {
  write_user(user,"You cannot promote a user to a level less than or equal to what they are now.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
if (word_count>2 && level>=user->level) {
  write_user(user,"You cannot promote a user to a level higher than or equal to your own.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
  /* now normal checks */
if (u->level>=user->level) {
  write_user(user,"You cannot promote a user to a level higher than your own.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level==JAILED) {
  vwrite_user(user,"You cannot promote a user of level %s.\n",user_level[JAILED].name);
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level>=WIZ) rem_wiz_node(u->name);
--amsys->level_count[u->level];
(word_count>2)?u->level=level:u->level++;
u->unarrest=u->level;
u->real_level=u->level;
user_list_level(u->name,u->level);
++amsys->level_count[u->level];
if (u->level>=WIZ) add_wiz_node(u->name,u->level);
u->socket=-2;
u->accreq=1;
sprintf(text,"%s",long_date(1));
strcpy(u->date,text);
strcpy(u->site,u->last_site);
save_user_details(u,0);
vwrite_user(user, promote_user_prompt,u->name,user_level[u->level].name);
if (msg!=NULL) {
	sprintf(text2,"~FG~OLBol%s si promotnut%s na level: ~RS~OL%s.\nDovod: ",
		grm_gnd(4, u->gender), grm_gnd(1, u->gender), user_level[u->level].name);
	tmp=(char *)malloc(strlen(text2)+strlen(msg)+2);
	sprintf(tmp, "%s%s\n", text2, msg);
	send_mail(user,word[1],tmp,0);
	free(tmp);
	}
else {
	sprintf(text2,"~FG~OLBol%s si promotnut%s na level: ~RS~OL%s.\n",
		grm_gnd(4, u->gender), grm_gnd(1, u->gender), user_level[u->level].name);
	send_mail(user,word[1],text2,0);
	}
write_syslog(SYSLOG,1,"%s PROMOTED %s to level %s, %s\n",user->name,word[1],user_level[u->level].name, msg);
sprintf(text,"Was ~FGpromoted~RS by %s to level %s.\n",user->name,user_level[u->level].name);
add_history(u->name,1,text);
if (msg!=NULL) {
	sprintf(text, "Dovod: %s\n", msg);
	add_history(u->name, 1, text);
	}
add_user_date_node(u->name,(long_date(1)));
destruct_user(u);
destructed=0;
}


/*** Demote a user ***/
void demote(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char text2[180],*name, *msg=NULL, *tmp;
	int level;

	set_crash();
	level=-1;
	if (word_count<2) {
		write_usage(user,"%s <user> [<level>]", command_table[DEMOTE].name);
		return;
		}
	if (word_count>2) {
		tmp=strdup(word[2]);
		strtoupper(tmp);
		if ((level=get_level(tmp))==-1) msg=remove_first(inpstr);
		else if (word_count>3) msg=remove_first(remove_first(inpstr));
		free(tmp);
		}
	/* See if user is on atm */
	if ((u=get_user(word[1]))!=NULL) {
		/* first, gotta reset the user's level if they've been temp promoted */
		if (u->real_level<u->level) u->level=u->real_level;
		if ((word_count>2) && (level==-1)) level=u->level-1;
		/* now runs checks if level option was used */
		if (word_count>2 && level>=u->level) {
			write_user(user,"You cannot demote a user to a level higher than or equal to what they are now.\n");
			return;
			}
		if (word_count>2 && level==JAILED) {
			vwrite_user(user,"You cannot demote a user to the level %s.\n",user_level[JAILED].name);
			return;
			}
		/* now normal checks */
		if (u->level<=NEW) {
			vwrite_user(user,"You cannot demote a user of level %s or %s.\n",user_level[NEW].name,user_level[JAILED].name);
			return;
			}
		if (u->level>=user->level) {
			write_user(user,"You cannot demote a user of an equal or higher level than yourself.\n");
			return;
			}
		if (u->restrict[RESTRICT_DEMO]==restrict_string[RESTRICT_DEMO]) {
			write_user(user,">>>This user cannot be demoted ! (restrict)\n");
			return;
			}
		if (user->vis) name=user->bw_recap;
		else name=invisname;
		if (u->level>=WIZ) rem_wiz_node(u->name);
  --amsys->level_count[u->level];
  /* if a user was a wiz then remove from retire list */
  if (u->level==WIZ) clean_retire_list(u->name);
  /* was a level given? */
  (word_count>2)?u->level=level:u->level--;
  u->unarrest=u->level;
  user_list_level(u->name,u->level);
  ++amsys->level_count[u->level];
  if (u->level>=WIZ) add_wiz_node(u->name,u->level);
  strcpy(u->date,(long_date(1)));
  sprintf(text,"~FR~OL%s is demoted to level: ~RS~OL%s.\n",u->bw_recap,user_level[u->level].name);
  write_level(u->level,1,text,u);
  if (msg!=NULL) {
  	sprintf(text, "~OLDovod:~RS %s\n", msg);
  	write_level(u->level, 1, text, u);
	}
  vwrite_user(u,"~FR~OL%s has demoted you to level: ~RS~OL%s!\n",name,user_level[u->level].name);
  if (msg!=NULL) vwrite_user(u, "~OLDovod:~RS %s\n", msg);
  write_syslog(SYSLOG,1,"%s DEMOTED %s to level %s, %s.\n",user->name,u->name,user_level[u->level].name, msg);
  sprintf(text,"Was ~FRdemoted~RS by %s to level %s, %s.\n",user->name,user_level[u->level].name, msg);
  add_history(u->name,1,text);
  add_user_date_node(u->name,(long_date(1)));
  u->vis=1;
  u->real_level=u->level;
  return;
  }
/* User not logged on */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in demote().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
/* first runs checks if level option was used */
if (word_count>2 && level==-1) level=u->level-1;
if (word_count>2 && level>=u->level) {
  write_user(user,"You cannot demote a user to a level higher than or equal to what they are now.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
if (word_count>2 && level==JAILED) {
  vwrite_user(user,"You cannot demote a user to the level %s.\n",user_level[JAILED].name);
  destruct_user(u);
  destructed=0;
  return;
  }
  /* now normal checks */
if (u->level<=NEW) {
  vwrite_user(user,"You cannot demote a user of level %s or %s.\n",user_level[NEW].name,user_level[JAILED].name);
  destruct_user(u);
  destructed=0;
  return;
  }
	if (u->restrict[RESTRICT_DEMO]==restrict_string[RESTRICT_DEMO]) {
		write_user(user,">>>Tento user nemoze byt demotnuty !\n");
		destruct_user(u);
		destructed=0;
		return;
		}
if (u->level>=user->level) {
  write_user(user,"You cannot demote a user of an equal or higher level than yourself.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level>=WIZ) rem_wiz_node(u->name);
--amsys->level_count[u->level];
/* if a user was a wiz then remove from retire list */
if (u->level==WIZ) clean_retire_list(u->name);
/* was a level given? */
(word_count>2)?u->level=level:u->level--;
u->unarrest=u->level;
u->real_level=u->level;
user_list_level(u->name,u->level);
++amsys->level_count[u->level];
if (u->level>=WIZ) add_wiz_node(u->name,u->level);
u->socket=-2;
u->vis=1;
strcpy(u->site,u->last_site);
strcpy(u->date,(long_date(1)));
save_user_details(u,0);
if (msg!=NULL) {
	sprintf(text2,"~FG~OLBol%s si demotnut%s na level: ~RS~OL%s.\nDovod: ",
		grm_gnd(4, u->gender), grm_gnd(1, u->gender), user_level[u->level].name);
	tmp=(char *)malloc(strlen(text2)+strlen(msg)+2);
	sprintf(tmp, "%s%s\n", text2, msg);
	send_mail(user,word[1],tmp,0);
	free(tmp);
	}
else {
	sprintf(text2,"~FG~OLBol%s si demotnut%s na level: ~RS~OL%s.\n",
		grm_gnd(4, u->gender), grm_gnd(1, u->gender), user_level[u->level].name);
	send_mail(user,word[1],text2,0);
	}
write_syslog(SYSLOG,1,"%s DEMOTED %s to level %s, %s.\n",user->name,word[1],user_level[u->level].name, msg);
sprintf(text,"Was ~FRdemoted~RS by %s to level %s.\n",user->name,user_level[u->level].name);
add_history(u->name,1,text);
if (msg!=NULL) {
	sprintf(text, "Dovod: %s\n", msg);
	add_history(u->name, 1, text);
	}
add_user_date_node(u->name,(long_date(1)));
destruct_user(u);
destructed=0;
}


/*** List banned sites or users ***/
void listbans(UR_OBJECT user)
{
int i;

	set_crash();
strtolower(word[1]);
if (!strcmp(word[1],"sites")) {
  write_user(user,"\n~BB*** Banned sites/domains ***\n\n"); 
  switch(more(user,user->socket, SITEBAN)) {
    case 0:
      write_user(user,"There are no banned sites/domains.\n\n");
      return;
    case 1:
    	user->misc_op=2;
    }
  return;
  }
if (!strcmp(word[1],"users")) {
  write_user(user,"\n~BB*** Banned users ***\n\n");
  switch(more(user,user->socket, USERBAN)) {
    case 0:
      write_user(user,"There are no banned users.\n\n");
      return;
    case 1:
    	user->misc_op=2;
    }
  return;
  }
if (!strcmp(word[1],"swears")) {
  write_user(user,"\n~BB*** Banned swear words ***\n\n");
  i=0;
  while(swear_words[i][0]!='*') {
    write_user(user,swear_words[i]);
    write_user(user,"\n");
    ++i;
    }
  if (!i) write_user(user,"There are no banned swear words.\n");
  if (amsys->ban_swearing) write_user(user,"\n");
  else write_user(user,"\n(Swearing ban is currently off)\n\n");
  return;
  }
if (strstr(word[1],"new")) {
  write_user(user,"\n~BB*** New users banned from sites/domains **\n\n");
  switch(more(user,user->socket, NEWBAN)) {
    case 0:
      write_user(user,"There are no sites/domains where new users have been banned.\n\n");
      return;
    case 1: user->misc_op=2;
    }
  return;
  }
write_usage(user,"%s sites/users/new/swears", command_table[LISTBANS].name);
}


/*** uban a site (or domain) or user ***/
void unban(UR_OBJECT user)
{
	char *usage="unban site/user/new <site/user name/site>";

	set_crash();
	if (word_count<3) {
		write_usage(user, usage);
		return;
		}
	strtolower(word[1]);
	if (!strcmp(word[1],"site")) {
		unban_site(user);
		return;
		}
	if (!strcmp(word[1],"user")) {
		unban_user(user);
		return;
		}
	if (!strcmp(word[1],"new")) {
		unban_new(user);
		return;
		}
	write_usage(user, usage);
}


/*** Ban a site/domain or user ***/
void ban(UR_OBJECT user)
{
	char *usage="ban site/user/new <site/user name/site>";

	set_crash();
	if (word_count<3) {
		write_usage(user,usage);
		return;
		}
	if (!strcmp(word[3], "-cancel")) {
		unban(user);
		return;
		}
	strtolower(word[1]);
	if (!strcmp(word[1],"site")) {
		ban_site(user);
		return;
		}
	if (!strcmp(word[1],"user")) {
		ban_user(user);
		return;
		}
	if (!strcmp(word[1],"new"))  {
		ban_new(user);
		return;
		}
	write_usage(user, usage);
}


/*** Site a user ***/
void site(UR_OBJECT user)
{
	UR_OBJECT u;

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user>", command_table[SITE].name);
		return;
	}
/* User currently logged in */
if ((u=get_user(word[1]))) {
#ifdef NETLINKS
  if (u->type==REMOTE_TYPE) vwrite_user(user, site_style_dns, u->name,u->site);
  else
#endif
    vwrite_user(user, site_style_dns_ip, u->name,u->site,u->ipsite,u->site_port);
  return;
  }
/* User not logged in */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in site().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);
  destruct_user(u);
  destructed=0;
  return;
  }
vwrite_user(user, site_style_offline, word[1], u->last_site);
destruct_user(u);
destructed=0;
}


/*** Umuzzle the bastard now he's apologised and grovelled enough via email ***/
void unmuzzle(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if ((u=get_user(word[1]))!=NULL) {
  if (u==user) {
    write_user(user,"Trying to unmuzzle yourself is the tenth sign of madness.\n");
    return;
    }
  if (!u->muzzled) {
    vwrite_user(user,"%s~RS is not muzzled.\n",u->recap);
    return;
    }
	if (u->restrict[RESTRICT_UNMU]==restrict_string[RESTRICT_UNMU]) {
		write_user(user,">>>This user cannot be unmuzzled !\n");
		return;
		}
  if (u->muzzled>user->level) {
    vwrite_user(user,"%s's muzzle is set to level %s, you do not have the power to remove it.\n",u->name,user_level[u->muzzled].name);
    return;
    }
  vwrite_user(user, unmuzzle_user_prompt, u->name);
  write_user(u,unmuzzle_victim_prompt);
  write_syslog(SYSLOG,1,"%s unmuzzled %s.\n",user->name,u->name);
  u->muzzled=0;
  sprintf(text,"~FGUnmuzzled~RS by %s, level %d (%s).\n",user->name,user->level,user_level[user->level].name);
  add_history(u->name,0,text);
  return;
  }
/* User not logged on */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in unmuzzle().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
	if (u->restrict[RESTRICT_UNMU]==restrict_string[RESTRICT_UNMU]) {
		write_user(user,">>>This user cannot be unmuzzled !\n");
		destruct_user(u);
		destructed=0;
		return;
		}
if (u->muzzled>user->level) {
  vwrite_user(user,"%s's muzzle is set to level %s, you do not have the power to remove it.\n",u->name,user_level[u->muzzled].name);
  destruct_user(u);
  destructed=0;
  return;
  }
u->socket=-2;
u->muzzled=0;
strcpy(u->site,u->last_site);
save_user_details(u,0);
vwrite_user(user,unmuzzle_user_prompt,u->bw_recap);
send_mail(user,word[1], unmuzzle_victim_prompt,0);
write_syslog(SYSLOG,1,"%s unmuzzled %s.\n",user->name,u->name);
sprintf(text,"~FGUnmuzzled~RS by %s, level %d (%s).\n",user->name,user->level,user_level[user->level].name);
add_history(u->name,0,text);
destruct_user(u);
destructed=0;
}


/*** Muzzle an annoying user so he cant speak, emote, echo, write, smail
     or bcast. Muzzles have levels from WIZ to GOD so for instance a wiz
     cannot remove a muzzle set by a god.  ***/
void muzzle(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user> [-cancel]", command_table[MUZZLE].name);
  return;
  }
if (!strcmp(word[2], "-cancel")) {
	unmuzzle(user);
	return;
	}
if ((u=get_user(word[1]))!=NULL) {
  if (u==user) {
    write_user(user,"Trying to muzzle yourself is the ninth sign of madness.\n");
    return;
    }
  if (u->level>=user->level) {
    write_user(user,"You cannot muzzle a user of equal or higher level than yourself.\n");
    return;
    }
  if (u->muzzled>=user->level) {
    vwrite_user(user,"%s~RS is already muzzled.\n",u->recap);
    return;
    }
	if (u->restrict[RESTRICT_MUZZ]==restrict_string[RESTRICT_MUZZ]) {
		write_user(user,">>>This user cannot be muzzled !\n");
		return;
		}
  vwrite_user(user,muzzle_user_prompt,u->bw_recap,user_level[user->level].name);
  write_user(u,muzzle_victim_prompt);
  write_syslog(SYSLOG,1,"%s muzzled %s (level %d).\n",user->name,u->name,user->level);
  u->muzzled=user->level;
  sprintf(text,"Level %d (%s) ~FRmuzzle~RS put on by %s.\n",user->level,user_level[user->level].name,user->name);
  add_history(u->name,1,text);
  return;
  }
/* User not logged on */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in muzzle().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
	if (u->restrict[RESTRICT_MUZZ]==restrict_string[RESTRICT_MUZZ]) {
		write_user(user,">>>This user cannot be muzzled !\n");
		destruct_user(u);
		destructed=0;
		return;
		}
if (u->level>=user->level) {
  write_user(user,"You cannot muzzle a user of equal or higher level than yourself.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->muzzled>=user->level) {
  vwrite_user(user,"%s is already muzzled.\n",u->name);
  destruct_user(u);
  destructed=0;
  return;
  }
u->socket=-2;
u->muzzled=user->level;
strcpy(u->site,u->last_site);
save_user_details(u,0);
vwrite_user(user,"~FR~OL%s given a muzzle of level: ~RS~OL%s.\n",u->bw_recap,user_level[user->level].name);
send_mail(user,word[1],muzzle_victim_prompt,0);
write_syslog(SYSLOG,1,"%s muzzled %s (level %d).\n",user->name,u->name,user->level);
sprintf(text,"Level %d (%s) ~FRmuzzle~RS put on by %s.\n",user->level,user_level[user->level].name,user->name);
add_history(u->name,1,text);
destruct_user(u);
destructed=0;
}


/*** Turn on and off each individual system log, or globally on/off ***/
void logging(UR_OBJECT user)
{
char temp[ARR_SIZE];
int cnt;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s -l/-s/-r/-n/-e/-on/-off", command_table[LOGGING].name);
  return;
  }
cnt=0;
strtolower(word[1]);
/* deal with listing the log status first */
if (!strcmp(word[1],"-l")) {
  write_user(user,"\n+----------------------------------------------------------------------------+\n");
  write_user(user,"| ~OL~FTSystem log status~RS                                                          |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  sprintf(temp,"General system log : ~OL%s~RS   Account request logs : ~OL%s~RS   Netlinks log : ~OL%s~RS",
	  (BIT_TEST(amsys->logging,SYSLOG))?"ON ":"OFF",(BIT_TEST(amsys->logging,REQLOG))?"ON ":"OFF",(BIT_TEST(amsys->logging,NETLOG))?"ON ":"OFF");
  cnt=colour_com_count(temp);
  vwrite_user(user,"| %-*s |\n",74+cnt*3,temp);
#ifdef DEBUG
  sprintf(temp,"Debug log          : ~OL%s~RS   Error log            : ~OL%s~RS",
		(BIT_TEST(amsys->logging, DEBLOG))?"ON ":"OFF", (BIT_TEST(amsys->logging, ERRLOG))?"ON ":"OFF");
#else
  sprintf(temp,"Error log          : ~OL%s~RS",
		(BIT_TEST(amsys->logging, ERRLOG))?"ON ":"OFF");
#endif
  cnt=colour_com_count(temp);
  vwrite_user(user,"| %-*s |\n", 74+cnt*3, temp);
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  return;
  }
/* (un)set syslog bit */
if (!strcmp(word[1],"-s")) {
  /* if on already */
  if (BIT_TEST(amsys->logging,SYSLOG)) {
    write_syslog(SYSLOG,1,"%s switched general system logging OFF.\n",user->name);
    amsys->logging=BIT_FLIP(amsys->logging,SYSLOG);
    }
  else {
    amsys->logging=BIT_FLIP(amsys->logging,SYSLOG);
    write_syslog(SYSLOG,1,"%s switched general system logging ON.\n",user->name);
    }
  vwrite_user(user,"You have now turned the general system logging %s.\n",(BIT_TEST(amsys->logging,SYSLOG))?"~OL~FGON~RS":"~OL~FROFF~RS");
  return;
  }
/* (un)set reqlog bit */
if (!strcmp(word[1],"-r")) {
  /* if on already */
  if (BIT_TEST(amsys->logging,REQLOG)) {
    write_syslog(REQLOG,1,"%s switched account request logging OFF.\n",user->name);
    amsys->logging=BIT_FLIP(amsys->logging,REQLOG);
    }
  else {
    amsys->logging=BIT_FLIP(amsys->logging,REQLOG);
    write_syslog(REQLOG,1,"%s switched account request logging ON.\n",user->name);
    }
  vwrite_user(user,"You have now turned the account logging logging %s.\n",(BIT_TEST(amsys->logging,REQLOG))?"~OL~FGON~RS":"~OL~FROFF~RS");
  return;
  }
/* (un)set netlog bit */
if (!strcmp(word[1],"-n")) {
#ifndef NETLINKS
  write_user(user,"Netlinks are not currently active.\n");
  return;
#endif
  /* if on already */
  if (BIT_TEST(amsys->logging,NETLOG)) {
    write_syslog(NETLOG,1,"%s switched netlink logging OFF.\n",user->name);
    amsys->logging=BIT_FLIP(amsys->logging,NETLOG);
    }
  else {
    amsys->logging=BIT_FLIP(amsys->logging,NETLOG);
    write_syslog(NETLOG,1,"%s switched netlink logging ON.\n",user->name);
    }
  vwrite_user(user,"You have now turned the netlink logging %s.\n",(BIT_TEST(amsys->logging,NETLOG))?"~OL~FGON~RS":"~OL~FROFF~RS");
  return;
  }
/* (un)set deblog bit */
if (!strcmp(word[1],"-d")) {
#ifdef DEBUG
  /* if on already */
  if (BIT_TEST(amsys->logging, DEBLOG)) {
    write_syslog(DEBLOG,1,"%s switched debug logging OFF.\n",user->name);
    amsys->logging=BIT_FLIP(amsys->logging, DEBLOG);
    }
  else {
    amsys->logging=BIT_FLIP(amsys->logging, DEBLOG);
    write_syslog(DEBLOG,1,"%s switched debug logging ON.\n",user->name);
    }
  vwrite_user(user,"You have now turned the debug logging %s.\n",(BIT_TEST(amsys->logging,DEBLOG))?"~OL~FGON~RS":"~OL~FROFF~RS");
  return;
#else
		write_user(user, "Debug logovanie je vypnute - nezakompilovane\n");
#endif
  }
/* (un)set errlog bit */
if (!strcmp(word[1],"-e")) {
  /* if on already */
  if (BIT_TEST(amsys->logging,ERRLOG)) {
    write_syslog(ERRLOG,1,"%s switched error logging OFF.\n",user->name);
    amsys->logging=BIT_FLIP(amsys->logging,ERRLOG);
    }
  else {
    amsys->logging=BIT_FLIP(amsys->logging,ERRLOG);
    write_syslog(ERRLOG,1,"%s switched error logging ON.\n",user->name);
    }
  vwrite_user(user,"You have now turned the error logging %s.\n",(BIT_TEST(amsys->logging,ERRLOG))?"~OL~FGON~RS":"~OL~FROFF~RS");
  return;
  }
/* set all bit */
if (!strcmp(word[1],"-on")) {
  if (!(BIT_TEST(amsys->logging,SYSLOG))) {
    amsys->logging=BIT_SET(amsys->logging,SYSLOG);
    write_syslog(SYSLOG,1,"%s switched general system logging ON.\n",user->name);
    }
  if (!(BIT_TEST(amsys->logging,REQLOG))) {
    amsys->logging=BIT_SET(amsys->logging,REQLOG);
    write_syslog(REQLOG,1,"%s switched acount request logging ON.\n",user->name);
    }
  if (!(BIT_TEST(amsys->logging,NETLOG))) {
    amsys->logging=BIT_SET(amsys->logging,NETLOG);
    write_syslog(NETLOG,1,"%s switched netlink logging ON.\n",user->name);
    }
#ifdef DEBUG  
  if (!(BIT_TEST(amsys->logging,DEBLOG))) {
    amsys->logging=BIT_SET(amsys->logging,DEBLOG);
    write_syslog(DEBLOG,1,"%s switched debug logging ON.\n",user->name);
    }
#endif
  if (!(BIT_TEST(amsys->logging,ERRLOG))) {
    amsys->logging=BIT_SET(amsys->logging,ERRLOG);
    write_syslog(ERRLOG,1,"%s switched error logging ON.\n",user->name);
    }
  write_user(user,"You have now turned all logging ~OL~FGON~RS.\n");
  return;
  }
/* unset all bit */
if (!strcmp(word[1],"-off")) {
  if (BIT_TEST(amsys->logging,SYSLOG)) {
    write_syslog(SYSLOG,1,"%s switched general system logging OFF.\n",user->name);
    amsys->logging=BIT_CLR(amsys->logging,SYSLOG);
    }
  if (BIT_TEST(amsys->logging,REQLOG)) {
    write_syslog(REQLOG,1,"%s switched acount request logging OFF.\n",user->name);
    amsys->logging=BIT_CLR(amsys->logging,REQLOG);
    }
  if (BIT_TEST(amsys->logging,NETLOG)) {
    write_syslog(NETLOG,1,"%s switched netlink logging OFF.\n",user->name);
    amsys->logging=BIT_CLR(amsys->logging,NETLOG);
    }
#ifdef DEBUG
  if (BIT_TEST(amsys->logging,DEBLOG)) {
    write_syslog(DEBLOG,1,"%s switched debug logging OFF.\n",user->name);
    amsys->logging=BIT_CLR(amsys->logging,DEBLOG);
    }
#endif
  if (BIT_TEST(amsys->logging,ERRLOG)) {
    write_syslog(ERRLOG,1,"%s switched acount request logging OFF.\n",user->name);
    amsys->logging=BIT_CLR(amsys->logging,ERRLOG);
    }
  write_user(user,"You have now turned all logging ~OL~FROFF~RS.\n");
  return;
  }
	write_usage(user,"%s -l/-s/-r/-n/-e/-on/-off", command_table[LOGGING].name);
}


/*** Set minlogin level ***/
void minlogin(UR_OBJECT user)
{
UR_OBJECT u,next;
char *usage="minlogin NONE/<user level>";
char levstr[5],*name;
int lev,cnt;

	set_crash();
if (word_count<2) {
  write_usage(user,usage);
  return;
  }
strtoupper(word[1]);
if ((lev=get_level(word[1]))==-1) {
  if (strcmp(word[1],"NONE")) {
    write_usage(user, usage);  return;
    }
  lev=-1;
  strcpy(levstr,"NONE");
  }
else strcpy(levstr,user_level[lev].name);
if (lev>user->level) {
  write_user(user,"You cannot set minlogin to a higher level than your own.\n");
  return;
  }
if (amsys->minlogin_level==lev) {
  write_user(user,"It is already set to that.\n");  return;
  }
amsys->minlogin_level=lev;
vwrite_user(user,"Minlogin level set to: ~OL%s.\n",levstr);
if (user->vis) name=user->name; else name=invisname;
vwrite_room_except(NULL,user,"%s has set the minlogin level to: ~OL%s.\n",name,levstr);
write_syslog(SYSLOG,1,"%s set the minlogin level to %s.\n",user->name,levstr);

/* Now boot off anyone below that level */
cnt=0;
u=user_first;
while(u) {
  next=u->next;
  if (!u->login && u->type!=CLONE_TYPE && u->level<lev) {
    write_user(u,"\n~FY~OLYour level is now below the minlogin level, disconnecting you...\n");
    disconnect_user(u);
    ++cnt;
    }
  u=next;
  }
vwrite_user(user,"Total of ~OL%d~RS users were disconnected.\n",cnt);
destructed=0;
}


/*** Show talker system parameters etc ***/
void system_details(UR_OBJECT user)
{
#ifdef NETLINKS
  NL_OBJECT nl;
#endif
RM_OBJECT rm;
UR_OBJECT u;
PL_OBJECT plugin;
CM_OBJECT plcmd;
char bstr[40],min_login[5];
char *ca[]={ "NONE  ","IGNORE","REBOOT" };
char *rip[]={"OFF","AUTO","MANUAL"};
int days,hours,mins,secs;
int netlinks,live,inc,outg;
int rms,inlinks,num_clones,mem,size;

	set_crash();
write_user(user,"\n+----------------------------------------------------------------------------+\n");
vwrite_user(user,"~OL~FTSystem details for %s~RS v%s (Lotos version %s)\n", reg_sysinfo[TALKERNAME], TVERSION, OSSVERSION);
write_user(user,"+----------------------------------------------------------------------------+\n");

/* Get some values */
strcpy(bstr,ctime(&amsys->boot_time));
secs=(int)(time(0)-amsys->boot_time);
days=secs/86400;
hours=(secs%86400)/3600;
mins=(secs%3600)/60;
secs=secs%60;
num_clones=0;
mem=0;
size=sizeof(struct user_struct);
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE) num_clones++;
  mem+=size;
  }
rms=0;  
inlinks=0;
size=sizeof(struct room_struct);
for(rm=room_first;rm!=NULL;rm=rm->next) {
#ifdef NETLINKS
  if (rm->inlink) ++inlinks;
#endif
  ++rms;  mem+=size;
  }
netlinks=0;  
live=0;
inc=0; 
outg=0;
#ifdef NETLINKS
  size=sizeof(struct netlink_struct);
  for(nl=nl_first;nl!=NULL;nl=nl->next) {
    if (nl->type!=UNCONNECTED && nl->stage==UP) live++;
    if (nl->type==INCOMING) ++inc;
    if (nl->type==OUTGOING) ++outg;
    ++netlinks;  mem+=size;
    }
#endif
size=sizeof(struct plugin_struct);
for(plugin=plugin_first; plugin!=NULL; plugin=plugin->next) mem+=size;
size=sizeof(struct plugin_cmd);
for(plcmd=cmds_first; plcmd!=NULL; plcmd=plcmd->next) mem+=size;
if (amsys->minlogin_level==-1) strcpy(min_login,"NONE");
else strcpy(min_login,user_level[amsys->minlogin_level].name);

/* Show header parameters */
#ifdef NETLINKS
  vwrite_user(user,"~FTProcess ID   : ~FG%-20u   ~FTPorts (M/W/L): ~FG%d,  %d,  %d\n",amsys->pid,port[0],port[1],port[2]);
#else
  vwrite_user(user,"~FTProcess ID   : ~FG%-20u   ~FTPorts (M/W): ~FG%d,  %d\n",amsys->pid,port[0],port[1]);
#endif
if (user->level>=GOD) {
	build_datetime(text);
	vwrite_user(user, "~FTBuild from   : ~FG%s\n", text);
	}
vwrite_user(user,"~FTTalker booted: ~FG%s~FTUptime       : ~FG%d day%s, %d hour%s, %d minute%s, %d second%s\n",
	bstr,days,PLTEXT_S(days),hours,PLTEXT_S(hours),mins,PLTEXT_S(mins),secs,PLTEXT_S(secs));
write_user(user,"+----------------------------------------------------------------------------+\n");
/* Show others */
vwrite_user(user,"Max users              : %-3d          Current num. of users  : %d\n",amsys->max_users,syspp->acounter[3]);
vwrite_user(user,"New users this boot    : %-3d          Old users this boot    : %d\n",amsys->logons_new,amsys->logons_old);
vwrite_user(user,"Max clones             : %-2d           Current num. of clones : %d\n",amsys->max_clones,num_clones);
vwrite_user(user,"Current minlogin level : %-4s         Login idle time out    : %d secs.\n",min_login,amsys->login_idle_time);
vwrite_user(user,"User idle time out     : %-4d secs.   Heartbeat              : %d\n",amsys->user_idle_time,amsys->heartbeat);
vwrite_user(user,"Remote user maxlevel   : %-12s Remote user deflevel   : %s\n",user_level[amsys->rem_user_maxlevel].name,user_level[amsys->rem_user_deflevel].name);
vwrite_user(user,"Wizport min login level: %-12s Gatecrash level        : %s\n",user_level[amsys->wizport_level].name,user_level[amsys->gatecrash_level].name);
vwrite_user(user,"Time out maxlevel      : %-12s Private room min count : %d\n",user_level[amsys->time_out_maxlevel].name,amsys->min_private_users);
vwrite_user(user,"Message lifetime       : %-2d days      Message check time     : %02d:%02d\n",amsys->mesg_life,amsys->mesg_check_hour,amsys->mesg_check_min);
vwrite_user(user,"Net idle time out      : %-4d secs.   Number of rooms        : %d\n",amsys->net_idle_time,rms);
vwrite_user(user,"Num. accepting connects: %-2d           Total netlinks         : %d\n",inlinks,netlinks);
vwrite_user(user,"Number which are live  : %-2d           Number incoming        : %d\n",live,inc);
vwrite_user(user,"Number outgoing        : %-2d           Ignoring sigterm       : %s\n",outg,noyes2[amsys->ignore_sigterm]);
vwrite_user(user,"Echoing passwords      : %s          Swearing ban status    : %s\n",noyes2[amsys->password_echo],minmax[amsys->ban_swearing]);
vwrite_user(user,"Time out afks          : %s          Names recaps allowed   : %s\n",noyes2[amsys->time_out_afks],noyes2[amsys->allow_recaps]);
vwrite_user(user,"New user prompt default: %-3d          New user colour default: %s\n",amsys->prompt_def,offon[amsys->colour_def]);
vwrite_user(user,"New user charecho def. : %s          System logging         : %s\n",offon[amsys->charecho_def],offon[(amsys->logging)?1:0]);
vwrite_user(user,"Crash action           : %s       Object memory allocated: %d\n",ca[amsys->crash_action],mem);
vwrite_user(user,"User purge length      : %-3d days     Newbie purge length    : %-3d days\n",USER_EXPIRES,NEWBIE_EXPIRES);
vwrite_user(user,"Smail auto-forwarding  : %s          Auto purge on          : %s\n",offon[amsys->forwarding],noyes2[amsys->auto_purge]);
vwrite_user(user,"Flood protection       : %s          Resolving IP           : %s\n",offon[amsys->flood_protect],rip[amsys->resolve_ip]);
vwrite_user(user,"Next auto purge date   : %s",ctime((time_t *)&amsys->purge_date));
if (syspp->auto_save>0) sprintf(text, "%s > %ld min", noyes2[1], syspp->auto_save);
else sprintf(text, "%s", noyes2[0]);
vwrite_user(user,"Pueblo-Enhanced Mode   : %s          Auto saving user's det.: %s\n", noyes2[syspp->pueblo_enh], text);
vwrite_user(user,"Use hostsfile          : %s\n", noyes2[use_hostsfile]);
write_user(user,"+----------------------------------------------------------------------------+\n");
}


/*** Free a hung socket ***/
void clearline(UR_OBJECT user)
{
UR_OBJECT u;
int sock;

	set_crash();
if (word_count<2 || !is_number(word[1])) {
  write_usage(user,"%s <line>", command_table[CLEARLINE].name);
  return;
  }
sock=atoi(word[1]);
/* Find line amongst users */
for(u=user_first;u!=NULL;u=u->next) 
  if (u->type!=CLONE_TYPE && u->socket==sock) goto FOUND;
write_user(user,"That line is not currently active.\n");
return;
FOUND:
if (!u->login) {
  write_user(user,"You cannot clear the line of a logged in user.\n");
  return;
  }
write_user(u,"\n\nThis line is being cleared.\n\n");
disconnect_user(u);
write_syslog(SYSLOG,1,"%s cleared line %d.\n",user->name,sock);
vwrite_user(user,"Line %d cleared.\n",sock);
destructed=0;
no_prompt=0;
}


/*** View the system log ***/
void viewlog(UR_OBJECT user)
{
	FILE *fp;
	char logfile[500],c;
	char *usage="%s <log>|<level> [<DD[<MM[<RRRR>]>]>]";
	char str[10], *tmp;
	int lines=0,cnt=0,cnt2,type,level=-1;
	int rd, rm, ry, l=0;
	char ext[9];

	set_crash();
	if (word_count<2) {
		write_usage(user, usage, command_table[VIEWLOG].name);
		return;
		}
	logfile[0]='\0';
	level=-1;
	strtoupper(word[1]);
	if (!strcmp(word[1], "SYS")) type=SYSLOG;
	else if (!strcmp(word[1], "NET")) type=NETLOG;
	else if (!strcmp(word[1], "REQ")) type=REQLOG;
#ifdef DEBUG
	else if (!strcmp(word[1], "DEB")) type=DEBLOG;
#endif
	else if (!strcmp(word[1], "ERR")) type=ERRLOG;
	else if (!strcmp(word[1], "RETIRED")) type=-1;
	else {level=get_level(word[1]); type=-2;}

	if (type>-1) l=1;
	if (word_count>2 && l) {
		if ((strlen(word[2])<2 || strlen(word[2])>8) && type>0) {
			vwrite_user(user, "Chybny parameter '%s'\n", word[2]);
			return;
			}
		strncpy(str, word[2], 2);
		str[2]='\0';
		rd=atoi(str);
		if (strlen(word[2])>=4) {
			strncpy(str, tmp=(word[2]+2), 2);
			str[2]='\0';
			rm=atoi(str);
			if (strlen(word[2])==8) {
				strncpy(str, tmp=(word[2]+4), 4);
				str[4]='\0';
				ry=atoi(str);
				}
			else ry=tyear;
			}
		else {
			rm=tmonth+1;
			ry=tyear;
			}
		}
	else {
		rd=tmday;
		rm=tmonth+1;
		ry=tyear;
		}

	sprintf(ext, "%04d%02u%02u", ry, rm, rd);
	if (word_count<3+l) {
		switch (type) {
			case SYSLOG:
				sprintf(logfile,"%s/%s.%s", LOGFILES,MAINSYSLOG, ext);
				vwrite_user(user,"\n~BB~FG*** System log %d.%d.%d ***\n", rd, rm, ry);
				break;
			case NETLOG:
				sprintf(logfile,"%s/%s.%s", LOGFILES,NETSYSLOG, ext);
				vwrite_user(user,"\n~BB~FG*** Netlink log %d.%d.%d ***\n", rd, rm, ry);
				break;
			case REQLOG:
				sprintf(logfile,"%s/%s.%s", LOGFILES,REQSYSLOG, ext);
				vwrite_user(user,"\n~BB~FG*** Account Request log %d.%d.%d ***\n", rd, rm, ry);
				break;
#ifdef DEBUG
			case DEBLOG:
				sprintf(logfile,"%s/%s.%s", LOGFILES,DEBSYSLOG, ext);
				vwrite_user(user,"\n~BB~FG*** Debug log %d.%d.%d ***\n", rd, rm, ry);
				break;
#endif
			case ERRLOG:
				sprintf(logfile,"%s/%s.%s", LOGFILES,ERRSYSLOG, ext);
				vwrite_user(user,"\n~BB~FG*** Error log %d.%d.%d ***\n", rd, rm, ry);
				break;
			case -1:
				strcpy(logfile, RETIRE_LIST);
				write_user(user,"\n~BB~FG*** Retired Wiz log ***\n");
				break;
			case -2:
				if (level<0) goto BADP;
				vwrite_user(user,"\n~BB~FG*** User list for level '%s' ***\n",user_level[level].name);
				if (!amsys->level_count[level]) {
					write_user(user, empty_log);
					return;
					}
				user->user_page_lev=level;
				switch(more_users(user)) {
					case 0: write_user(user, empty_log);  return;
					case 1: user->misc_op=16; user->status='R';
					}
				return;
			default:
BADP:				write_usage(user, usage, command_table[VIEWLOG].name);
				return;
			}
		switch (more(user,user->socket,logfile)) {
			case 0: write_user(user, empty_log);  return;
			case 1: user->misc_op=2;
			}
		return;
		} /* word_count<3+l */

	if ((lines=atoi(word[2+l]))<1) {
		write_usage(user, usage, command_table[VIEWLOG].name);
		return;
		}
/* find out which log */
	switch (type) {
		case SYSLOG:
			sprintf(logfile,"%s/%s.%s", LOGFILES,MAINSYSLOG, ext);
			break;
		case NETLOG:
			sprintf(logfile,"%s/%s.%s", LOGFILES,NETSYSLOG, ext);
			break;
		case REQLOG:
			sprintf(logfile,"%s/%s.%s", LOGFILES,REQSYSLOG, ext);
			break;
#ifdef DEBUG
		case DEBLOG:
			sprintf(logfile,"%s/%s.%s", LOGFILES,DEBSYSLOG, ext);
			break;
#endif
		case ERRLOG:
			sprintf(logfile,"%s/%s.%s", LOGFILES,ERRSYSLOG, ext);
			break;
		case -1:
			strcpy(logfile, RETIRE_LIST);
			break;
		case -2:
			if (level<0) goto BADP;
			if (!amsys->level_count[level]) {
				write_user(user, empty_log);
				return;
				}
			if (lines>amsys->level_count[level]) {
				vwrite_user(user,"There %s only %d line%s in the log.\n",PLTEXT_IS(amsys->level_count[level]),amsys->level_count[level],PLTEXT_S(amsys->level_count[level]));
				return;
				}
			if (lines==amsys->level_count[level]) vwrite_user(user,"\n~BB~FG*** User list for level '%s' ***\n\n",user_level[level].name);
			else {
				user->user_page_pos=amsys->level_count[level]-lines;
				vwrite_user(user,"\n~BB~FG*** User list for level '%s' (last %d line%s) ***\n",user_level[level].name,lines,PLTEXT_S(lines));
				}
			user->user_page_lev=level;
			switch(more_users(user)) {
				case 0: write_user(user, empty_log);  return;
				case 1: user->misc_op=16; user->status='R';
				}
			return;
		default:
			write_usage(user, usage, command_table[VIEWLOG].name);
			return;
		}

/* count total lines */
	if (!(fp=fopen(logfile,"r"))) {
		write_user(user, empty_log);
		return;
		}

	cnt=0;
	c=getc(fp);
	while (!feof(fp)) {
		if (c=='\n') ++cnt;
		c=getc(fp);
		}
	if (cnt<lines) {
		vwrite_user(user,"There %s only %d line%s in the log.\n",PLTEXT_IS(cnt),cnt,PLTEXT_S(cnt));
		fclose(fp);
		return;
		}

/* Find line to start on */
	fseek(fp,0,0);
	cnt2=0;
	c=getc(fp);
	while (!feof(fp)) {
		if (c=='\n') ++cnt2;
		c=getc(fp);
		if (cnt2==cnt-lines) {
			switch (type) {
				case SYSLOG: vwrite_user(user,"\n~BB~FG*** System log %d.%d.%d (last %d line%s) ***\n", rd, rm, ry, lines,PLTEXT_S(lines));  break;
				case NETLOG: vwrite_user(user,"\n~BB~FG*** Netlink log %d.%d.%d (last %d line%s) ***\n", rd, rm, ry, lines,PLTEXT_S(lines));  break;
				case REQLOG: vwrite_user(user,"\n~BB~FG*** Account Request log %d.%d.%d (last %d line%s) ***\n", rd, rm, ry, lines,PLTEXT_S(lines));  break;
#ifdef DEBUG
				case DEBLOG: vwrite_user(user,"\n~BB~FG*** Debug log %d.%d.%d (last %d line%s) ***\n", rd, rm, ry, lines,PLTEXT_S(lines));  break;
#endif
				case ERRLOG: vwrite_user(user,"\n~BB~FG*** Error log %d.%d.%d (last %d line%s) ***\n", rd, rm, ry, lines,PLTEXT_S(lines));  break;
				case -1: vwrite_user(user,"\n~BB~FG*** Retired Wiz log (last %d line%s) ***\n",lines,PLTEXT_S(lines));  break;
				}
			user->filepos=ftell(fp)-1;
			fclose(fp);
			if (more(user,user->socket,logfile)!=1) user->filepos=0;
			else user->misc_op=2;
			return;
			}
		}
	fclose(fp);
	vwrite_user(user,"%s: Line count error.\n",syserror);
	write_syslog(ERRLOG, 1, "Line count error in viewlog().\n");
}


/*** Switch swearing ban on and off ***/
void toggle_swearban(UR_OBJECT user)
{
	set_crash();
switch(amsys->ban_swearing) {
  case SBOFF:
    write_user(user,"Swearing ban now set to ~FGminimum ban~RS.\n");
    amsys->ban_swearing=SBMIN;
    write_syslog(SYSLOG,1,"%s set swearing ban to MIN.\n",user->name);
    break;
  case SBMIN:
    write_user(user,"Swearing ban now set to ~FRmaximum ban~RS.\n");
    amsys->ban_swearing=SBMAX;
    write_syslog(SYSLOG,1,"%s set swearing ban to MAX.\n",user->name);
    break;
  case SBMAX:
    write_user(user,"Swearing ban now set to ~FYoff~RS.\n");
    amsys->ban_swearing=SBOFF;
    write_syslog(SYSLOG,1,"%s set swearing ban to OFF.\n",user->name);
    break;
  }
}

/*** Delete a user ***/
void delete_user(UR_OBJECT user, int this_user)
{
UR_OBJECT u;
char name[USER_NAME_LEN+1];
int level;

	set_crash();
if (this_user) {
  /* User structure gets destructed in disconnect_user(), need to keep a
     copy of the name */
  strcpy(name,user->name);
  level=user->level;
  write_user(user,"\n~FR~LI~OLACCOUNT DELETED!\n");
  vwrite_room_except(user->room,user,suicide_prompt,user->name, grm_gnd(4,user->gender));
  write_syslog(SYSLOG,1,"%s SUICIDED.\n",name);
  if (user->level>=WIZ) rem_wiz_node(user->name);
  disconnect_user(user);
  clean_retire_list(name);
  clean_files(name);
  rem_user_node(name,-1);
  amsys->level_count[level]--;
  return;
  }
if (word_count<2) {
  write_usage(user,"%s <user>", command_table[DELETE].name);
  return;
  }
word[1][0]=toupper(word[1][0]);
if (!strcmp(word[1],user->name)) {
  write_user(user,"Trying to delete yourself is the eleventh sign of madness.\n");
  return;
  }
if (get_user(word[1])!=NULL) {
  /* Safety measure just in case. Will have to .kill them first */
  write_user(user,"You cannot delete a user who is currently logged on.\n");
  return;
  }
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in delete_user().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot delete a user of an equal or higher level than yourself.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
clean_files(u->name);
clean_retire_list(u->name);
rem_user_node(u->name,-1);
if (u->level>=WIZ) rem_wiz_node(u->name);
amsys->level_count[u->level]--;
vwrite_user(user,"\07~FR~OL~LIUser %s deleted!\n",u->name);
write_syslog(SYSLOG,1,"%s DELETED %s.\n",user->name,u->name);
destruct_user(u);
destructed=0;
}


/*** Reboot talker interface func. ***/
void reboot_com(UR_OBJECT user)
{
	set_crash();
if (!amsys->rs_which) {
  write_user(user,"The shutdown countdown is currently active, you must cancel it first.\n");
  return;
  }
if (!strcmp(word[1],"-cancel")) {
  if (!amsys->rs_countdown) {
    write_user(user,"The reboot countdown is not currently active.\n");
    return;
    }
  if (amsys->rs_countdown && amsys->rs_user==NULL) {
    write_user(user,"Someone else is currently setting the reboot countdown.\n");
    return;
    }
  write_room(NULL,"~OLSYSTEM:~RS~FG Reboot cancelled.\n");
  write_syslog(SYSLOG,1,"%s cancelled the reboot countdown.\n",user->name);
  amsys->rs_countdown=0;
  amsys->rs_announce=0;
  amsys->rs_which=-1;
  amsys->rs_user=NULL;
  return;
  }
if (word_count>1 && !is_number(word[1])) {
  write_usage(user,"%s [<secs>]|[-cancel]", command_table[REBOOT].name);
  return;
  }
if (amsys->rs_countdown) {
  write_user(user,"The reboot countdown is currently active, you must cancel it first.\n");
  return;
  }
save_counters();
if (word_count<2) {
  amsys->rs_countdown=0;  
  amsys->rs_announce=0;
  amsys->rs_which=-1; 
  amsys->rs_user=NULL;
  }
else {
  amsys->rs_countdown=atoi(word[1]);
  amsys->rs_which=1;
  }
audioprompt(user, 4, 0);
write_user(user,"\n\07~FY~OL~LI*** POZOR - Toto rebootne talker ! ***\n\nNaozaj to chces (y/n)? ");
user->misc_op=7;  
no_prompt=1;  
}


/* allows the user to call the purge function */
void purge_users(UR_OBJECT user)
{
int exp=0;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s [-d] [-s <site>] [-t <days>]", command_table[PURGE].name);
  return;
  }
if (!strcmp(word[1],"-d")) {
  write_user(user,"~OL~FR***~RS Purging users with default settings ~OL~FR***\n");
  purge(1,NULL,0);
  }
else if (!strcmp(word[1],"-s")) {
  if (word_count<3) {
    write_usage(user,"%s [-d] [-s <site>] [-t <days>]", command_table[PURGE].name);
    return;
    }
  /* check for variations of wild card */
  if (!strcmp("*",word[2])) {
    write_user(user,"You cannot purge users from the site '*'.\n");
    return;
    }
  if (strstr(word[2],"**")) {
    write_user(user,"You cannot have ** in your site to purge.\n");
    return;
    }
  if (strstr(word[2],"?*")) {
    write_user(user,"You cannot have ?* in your site to purge.\n");
    return;
    }
  if (strstr(word[2],"*?")) {
    write_user(user,"You cannot have *? in your site to purge.\n");
    return;
    }
  vwrite_user(user,"~OL~FR***~RS Purging users with site '%s' ~OL~FR***\n",word[2]);
  purge(2,word[2],0);
  }
else if (!strcmp(word[1],"-t")) {
  if (word_count<3) {
    write_usage(user,"%s [-d] [-s <site>] [-t <days>]", command_table[PURGE].name);
    return;
    }
  exp=atoi(word[2]);
  if (exp<=USER_EXPIRES) {
    write_user(user,"You cannot purge users who last logged in less than the default time.\n");
    vwrite_user(user,"The current default is: %d days\n",USER_EXPIRES);
    return;
    }
  if (exp<0 || exp>999) {
    write_user(user,"You must enter the amount days from 0-999.\n");
    return;
    }
  vwrite_user(user,"~OL~FR***~RS Purging users who last logged in over '%d days' ago ~OL~FR***\n",exp);
  purge(3,NULL,exp);
  }
else {
  write_usage(user,"%s [-d] [-s <site>] [-t <days>]", command_table[PURGE].name);
  return;
  }
/* finished purging - give result */
vwrite_user(user,"Checked ~OL%d~RS user%s (~OL%d~RS skipped), ~OL%d~RS %s purged.  User count is now ~OL%d~RS.\n",
              amsys->purge_count,PLTEXT_S(amsys->purge_count),amsys->purge_skip,amsys->users_purged,PLTEXT_WAS(amsys->users_purged),amsys->user_count);
}


/* shows the history file of a given user */
void user_history(UR_OBJECT user)
{
char filename[500];

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user>", command_table[HISTORY].name);
  return;
  }
word[1][0]=toupper(word[1][0]);
if (!(find_user_listed(word[1]))) {
  write_user(user,nosuchuser);  return;
  }
sprintf(filename,"%s/%s.H", USERHISTORYS,word[1]);
vwrite_user(user,"~BB~FG*** The history of user ~OL%s~RS~BB~FG is as follows ***\n\n",word[1]);
switch(more(user,user->socket,filename)) {
  case 0: sprintf(text,"%s has no previously recorded history.\n\n",word[1]);
          write_user(user,text);  break;
  case 1: user->misc_op=2;  break;
  }
}


/* Set a user to either expire after a set time, or never expire */
void user_expires(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user>", command_table[EXPIRE].name);
  return;
  }
/* user logged on */
if ((u=get_user(word[1]))) {
  if (!u->expire) {
    u->expire=1;
    vwrite_user(user,"You have set it so %s will expire when a purge is run.\n",u->name);
    sprintf(text,"%s enables expiration with purge.\n",user->name);
    add_history(u->name,0,text);
    write_syslog(SYSLOG,1,"%s enabled expiration on %s.\n",user->name,u->name);
    return;
    }
  u->expire=0;
  vwrite_user(user,"You have set it so %s will no longer expire when a purge is run.\n",u->name);
  sprintf(text,"%s disables expiration with purge.\n",user->name);
  add_history(u->name,0,text);
  write_syslog(SYSLOG,1,"%s disabled expiration on %s.\n",user->name,u->name);
  return;
  }
/* user not logged on */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user session.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user session in user_expires().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);
  destruct_user(u);
  destructed=0;
  return;
  }
if (!u->expire) {
  u->expire=1;
  vwrite_user(user,"You have set it so %s will expire when a purge is run.\n",u->name);
  sprintf(text,"%s enables expiration with purge.\n",user->name);
  add_history(u->name,0,text);
  write_syslog(SYSLOG,1,"%s enabled expiration on %s.\n",user->name,u->name);
  save_user_details(u,0); destruct_user(u); destructed=0; return;
  }
u->expire=0;
vwrite_user(user,"You have set it so %s will no longer expire when a purge is run.\n",u->name);
sprintf(text,"%s disables expiration with purge.\n",user->name);
add_history(u->name,0,text);
write_syslog(SYSLOG,1,"%s disabled expiration on %s.\n",user->name,u->name);
save_user_details(u,0); destruct_user(u); destructed=0; return;
}


/* Unarrest a user who is currently under arrest/in jail */
void unarrest(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;

	set_crash();
word[1][0]=toupper(word[1][0]);
if ((u=get_user(word[1]))) {
  if (u==user) {
    write_user(user,"You cannot unarrest yourself.\n");
    return;
    }
  if (u->level!=JAILED) {
    vwrite_user(user,"%s~RS is not under arrest!\n",u->recap);
    return;
    }
  if (user->level<u->arrestby) {
    vwrite_user(user,"%s~RS can only be unarrested by a %s or higher.\n",u->recap,user_level[u->arrestby].name);
    return;
    }
  --amsys->level_count[u->level];
  u->level=u->unarrest;
  u->arrestby=0;
  user_list_level(u->name,u->level);
  strcpy(u->date,(long_date(1)));
  ++amsys->level_count[u->level];
  rm=get_room(default_warp);
  write_room(NULL,"The Hand of Justice reaches through the air...\n");
  write_user(u,"You have been unarrested...  Now try to behave!\n");
  if (rm==NULL) vwrite_user(user,"Cannot find a room for ex-cons, so %s~RS is still in the %s!\n",u->recap,u->room->name);
  else move_user(u,rm,2);
  write_syslog(SYSLOG,1,"%s UNARRESTED %s\n",user->name,u->name);
  sprintf(text,"Was ~FGunarrested~RS by %s.\n",user->name);
  add_history(u->name,1,text);
  return;
  }
/* Create a temp session, load details, alter , then save. This is inefficient
   but its simpler than the alternative */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in unarrest().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level!=JAILED) {
  vwrite_user(user,"%s~RS is not under arrest!\n",u->recap);
  destruct_user(u);
  destructed=0;
  return;
  }
if (user->level<u->arrestby) {
  vwrite_user(user,"%s~RS can only be unarrested by a %s or higher.\n",u->recap,user_level[u->arrestby].name);
  destruct_user(u);
  destructed=0;
  return;
  }
--amsys->level_count[u->level];
u->level=u->unarrest;
u->arrestby=0;
user_list_level(u->name,u->level);
strcpy(u->date,(long_date(1)));
++amsys->level_count[u->level];
u->socket=-2;
strcpy(u->site,u->last_site);
save_user_details(u,0);
vwrite_user(user,"You unarrest %s~RS.\n",u->recap);
send_mail(user,word[1],"~OLYou have been ~FGunarrested~RS~OL.  Please read the rules again and try to behave!\n",0);
write_syslog(SYSLOG,1,"%s UNARRESTED %s.\n",user->name,u->name);
sprintf(text,"Was ~FGunarrested~RS by %s.\n",user->name);
add_history(u->name,1,text);
destruct_user(u);
destructed=0;
}


/* Put annoying user in jail */
void arrest(UR_OBJECT user, int type)
{
UR_OBJECT u;
RM_OBJECT rm;

	set_crash();
if (type) {
	user->unarrest=ARCH;
	user->arrestby=GOD;
	--amsys->level_count[user->level];
	user->level=JAILED;
	user_list_level(user->name, user->level);
	strcpy(user->date, (long_date(1)));
	++amsys->level_count[user->level];
	rm=get_room(default_jail);
	write_room(NULL,"The Hand of Justice reaches through the air...\n");
	vwrite_user(user,"Bol%s si uvrhnut%s do vazenia.\n", grm_gnd(4, user->gender), grm_gnd(1, user->gender));
	if (rm!=NULL) move_user(user, rm, 2);
	vwrite_room_except(NULL, user, "%s~RS has been placed under arrest...\n",user->recap);
	write_syslog(SYSLOG,1,"Bank ARRESTED %s (at level %s)\n", user->name, user_level[user->arrestby].name);
	sprintf(text,"Was ~FRarrested~RS by Bank (at level ~OL%s~RS).\n", user_level[user->arrestby].name);
	add_history(user->name,1,text);
	save_user_details(user, 1);
	return;
	}

if (word_count<2) {
  write_usage(user,"%s <user> [-cancel]", command_table[ARREST].name);
  return;
  }
if (!strcmp(word[2], "-cancel")) {
	unarrest(user);
	return;
	}
word[1][0]=toupper(word[1][0]);
if ((u=get_user(word[1]))) {
  if (u==user) {
    write_user(user,"Nemozes uvaznit seba !\n");
    return;
    }
  if (u->level>=user->level) {
    write_user(user,"You cannot arrest anyone of the same or higher level than yourself.\n");
    return;
    }
  if (u->level==JAILED) {
    vwrite_user(user,"%s~RS uz je vo vazeni !\n",u->recap);
    return;
  }
  u->vis=1;
  u->unarrest=u->level;
  u->arrestby=user->level;
  --amsys->level_count[u->level];
  u->level=JAILED;
  user_list_level(u->name,u->level);
  strcpy(u->date,(long_date(1)));
  ++amsys->level_count[u->level];
  rm=get_room(default_jail);
  write_room(NULL,"The Hand of Justice reaches through the air...\n");
  write_user(u,"You have been placed in jail.\n");
  if (rm==NULL) vwrite_user(user,"Cannot find the jail, so %s~RS is arrested but still in the %s.\n",u->recap,u->room->name);
  else move_user(u,rm,2);
  vwrite_room_except(NULL,u,"%s~RS has been placed under arrest...\n",u->recap);
  write_syslog(SYSLOG,1,"%s ARRESTED %s (at level %s)\n",user->name,u->name,user_level[u->arrestby].name);
  sprintf(text,"Was ~FRarrested~RS by %s (at level ~OL%s~RS).\n",user->name,user_level[u->arrestby].name);
  add_history(u->name,1,text);
  save_user_details(u, 1);
  return;
  }
/* Create a temp session, load details, alter , then save. This is inefficient
   but its simpler than the alternative */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in arrest().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot arrest anyone of the same or higher level than yourself.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level==JAILED) {
  vwrite_user(user,"%s~RS uz je vo vazeni !\n",u->recap);
  destruct_user(u);
  destructed=0;
  return;
  }
u->vis=1;
u->unarrest=u->level;
u->arrestby=user->level;
--amsys->level_count[u->level];
u->level=JAILED;
user_list_level(u->name,u->level);
strcpy(u->date,(long_date(1)));
++amsys->level_count[u->level];
u->socket=-2;
strcpy(u->site,u->last_site);
save_user_details(u,0);
vwrite_user(user,"You place %s~RS under arrest.\n",u->recap);
send_mail(user,word[1],"~OLYou have been placed under ~FRarrest~RS~OL.\n",0);
write_syslog(SYSLOG,1,"%s ARRESTED %s (at level %s).\n",user->name,u->name,user_level[u->arrestby].name);
sprintf(text,"Was ~FRarrested~RS by %s (at level ~OL%s~RS).\n",user->name,user_level[u->arrestby].name);
add_history(u->name,1,text);
destruct_user(u);
destructed=0;
}


/* allows a user to add to another users history list */
void manual_history(UR_OBJECT user, char *inpstr)
{
	set_crash();
if (word_count<3) {
  write_usage(user,"%s <user> <text>", command_table[ADDHISTORY].name);
  return;
  }
word[1][0]=toupper(word[1][0]);
if (!strcmp(user->name,word[1])) {
  write_user(user,"You cannot add to your own history list.\n");
  return;
  }
if (!(find_user_listed(word[1]))) {
  write_user(user,nosuchuser);  return;
  }
inpstr=remove_first(inpstr);
sprintf(text,"%-*s : %s\n",USER_NAME_LEN,user->name,inpstr);
add_history(word[1],1,text);
vwrite_user(user,"You have added to %s's history list.\n",word[1]);
}


/* Display all the people logged on from the same site as user */
void samesite(UR_OBJECT user, int stage)
{
UR_OBJECT u,u_loop;
int found,cnt,same,on;
struct user_dir_struct *entry;

	set_crash();
on=0;
if (!stage) {
  if (word_count<2) {
    write_usage(user,"%s user/site [all]", command_table[SAMESITE].name);
    return;
    }
  strtolower(word[1]); strtolower(word[2]);
  if (word_count==3 && !strcmp(word[2],"all")) user->samesite_all_store=1;
  else user->samesite_all_store=0;
  if (!strcmp(word[1],"user")) {
    write_user(user,"Enter the name of the user to be checked against: ");
    user->misc_op=12;
    return;
    }
  if (!strcmp(word[1],"site")) {
    write_user(user,"~OL~FRNOTE:~RS Wildcards '*' and '?' can be used.\n");
    write_user(user,"Enter the site to be checked against: ");
    user->misc_op=13;
    return;
    }
  write_usage(user,"%s user/site [all]", command_table[SAMESITE].name);
  return;
  }
/* check for users of same site - user supplied */
if (stage==1) {
  /* check just those logged on */
  if (!user->samesite_all_store) {
    found=cnt=same=0;
    if ((u=get_user(user->samesite_check_store))==NULL) {
      write_user(user,notloggedon);
      return;
      }
    for (u_loop=user_first;u_loop!=NULL;u_loop=u_loop->next) {
      cnt++;
      if (u_loop==u) continue;
      if (!strcmp(u->site,u_loop->site)) {
	same++;
	if (++found==1) vwrite_user(user,"\n~BB~FG*** Users logged on from the same site as ~OL%s~RS~BB~FG ***\n\n",u->name);
	sprintf(text,"    %s %s\n",u_loop->name,u_loop->desc);
	if (u_loop->type==REMOTE_TYPE) text[2]='@';
	if (!u_loop->vis) text[3]='*';
	write_user(user,text);
        }
      }
    if (!found) vwrite_user(user,"No users currently logged on have the same site as %s.\n",u->name);
    else vwrite_user(user,"\nChecked ~FM~OL%d~RS users, ~FM~OL%d~RS had the site as ~FG~OL%s~RS ~FG(%s)\n\n",cnt,same,u->name,u->site);
    return;
    }
  /* check all the users..  First, load the name given */
  if (!(u=get_user(user->samesite_check_store))) {
    if ((u=create_user())==NULL) {
      vwrite_user(user,"%s: unable to create temporary user session.\n",syserror);
      write_syslog(ERRLOG,1,"Unable to create temporary user session in samesite() - stage 1/all.\n");
      return;
      }
    strcpy(u->name,user->samesite_check_store);
    if (!load_user_details(u)) {
      destruct_user(u); destructed=0;
      vwrite_user(user,"Sorry, unable to load user file for %s.\n",user->samesite_check_store);
      write_syslog(ERRLOG,1,"Unable to load user details in samesite() - stage 1/all.\n");
      return;
      }
    on=0;
    }
  else on=1;
  /* read userlist and check against all users */
  found=cnt=same=0;
  entry=first_dir_entry;
  while (entry!=NULL) {
    entry->name[0]=toupper(entry->name[0]); /* just incase */
    /* create a user object if user not already logged on */
    if ((u_loop=create_user())==NULL) {
      write_syslog(ERRLOG,1,"Unable to create temporary user session in samesite().\n");
      goto SAME_SKIP1;
      }
    strcpy(u_loop->name,entry->name);
    if (!load_user_details(u_loop)) {
      destruct_user(u_loop); destructed=0;
      goto SAME_SKIP1;
      }
    cnt++;
    if ((on && !strcmp(u->site,u_loop->last_site)) || (!on && !strcmp(u->last_site,u_loop->last_site))) {
      same++;
      if (++found==1) vwrite_user(user,"\n~BB~FG*** All users from the same site as ~OL%s~RS~BB~FG ***\n\n",u->name);
      vwrite_user(user,"    %s %s\n",u_loop->name,u_loop->desc);
      destruct_user(u_loop);
      destructed=0;
      goto SAME_SKIP1;
      }
    destruct_user(u_loop);
    destructed=0;
  SAME_SKIP1:
    entry=entry->next;
    }
  if (!found) vwrite_user(user,"No users have the same site as %s.\n",u->name);
  else {
    if (!on) vwrite_user(user,"\nChecked ~FM~OL%d~RS users, ~FM~OL%d~RS had the site as ~FG~OL%s~RS ~FG(%s)\n\n",cnt,same,u->name,u->last_site);
    else vwrite_user(user,"\nChecked ~FM~OL%d~RS users, ~FM~OL%d~RS had the site as ~FG~OL%s~RS ~FG(%s)\n\n",cnt,same,u->name,u->site);
    }
  if (!on) { destruct_user(u);  destructed=0; }
  return;
  } /* end of stage 1 */

/* check for users of same site - site supplied */
if (stage==2) {
  /* check any wildcards are correct */
  if (strstr(user->samesite_check_store,"**")) {
    write_user(user,"You cannot have ** in your site to check.\n");
    return;
    }
  if (strstr(user->samesite_check_store,"?*")) {
    write_user(user,"You cannot have ?* in your site to check.\n");
    return;
    }
  if (strstr(user->samesite_check_store,"*?")) {
    write_user(user,"You cannot have *? in your site to check.\n");
    return;
    }
  /* check just those logged on */
  if (!user->samesite_all_store) {
    found=cnt=same=0;
    for (u=user_first;u!=NULL;u=u->next) {
      cnt++;
      if (pattern_match(u->site,user->samesite_check_store)) continue;
      same++;
      if (++found==1) vwrite_user(user,"\n~BB~FG*** Users logged on from the same site as ~OL%s~RS~BB~FG ***\n\n",user->samesite_check_store);
      sprintf(text,"    %s %s\n",u->name,u->desc);
      if (u->type==REMOTE_TYPE) text[2]='@';
      if (!u->vis) text[3]='*';
      write_user(user,text);
      }
    if (!found) vwrite_user(user,"No users currently logged on have that same site as %s.\n",user->samesite_check_store);
    else vwrite_user(user,"\nChecked ~FM~OL%d~RS users, ~FM~OL%d~RS had the site as ~FG~OL%s\n\n",cnt,same,user->samesite_check_store);
    return;
    }
  /* check all the users.. */
  /* open userlist to check against all users */
  found=cnt=same=0;
  entry=first_dir_entry;
  while (entry!=NULL) {
    entry->name[0]=toupper(entry->name[0]);
    /* create a user object if user not already logged on */
    if ((u_loop=create_user())==NULL) {
      write_syslog(ERRLOG,1,"Unable to create temporary user session in samesite() - stage 2/all.\n");
      goto SAME_SKIP2;
      }
    strcpy(u_loop->name,entry->name);
    if (!load_user_details(u_loop)) {
      destruct_user(u_loop); destructed=0;
      goto SAME_SKIP2;
      }
    cnt++;
    if ((pattern_match(u_loop->last_site,user->samesite_check_store))) {
      same++;
      if (++found==1) vwrite_user(user,"\n~BB~FG*** All users that have the site ~OL%s~RS~BB~FG ***\n\n",user->samesite_check_store);
      vwrite_user(user,"    %s %s\n",u_loop->name,u_loop->desc);
      destruct_user(u_loop);
      destructed=0;
      goto SAME_SKIP2;
      }
    destruct_user(u_loop);
    destructed=0;
  SAME_SKIP2:
    entry=entry->next;
    }
  if (!found) vwrite_user(user,"No users have the same site as %s.\n",user->samesite_check_store);
  else {
    if (!on) vwrite_user(user,"\nChecked ~FM~OL%d~RS users, ~FM~OL%d~RS had the site as ~FG~OL%s~RS\n\n",cnt,same,user->samesite_check_store);
    else vwrite_user(user,"\nChecked ~FM~OL%d~RS users, ~FM~OL%d~RS had the site as ~FG~OL%s~RS\n\n",cnt,same,user->samesite_check_store);
    }
  return;
  } /* end of stage 2 */
}


/* Force a save of all the users who are logged on */
void force_save(UR_OBJECT user)
{
	UR_OBJECT u;
	int cnt;

	set_crash();
	cnt=0;
	for (u=user_first; u!=NULL; u=u->next) {
#ifdef NETLINKS
		if (u->type==REMOTE_TYPE) continue;
#endif
		if (u->type==CLONE_TYPE || u->login) continue;
		cnt++;
		save_user_details(u, 1);
		}
	if (user!=NULL) {
		write_syslog(SYSLOG, 1, "Manually saved %d user's details.\n",cnt);
		vwrite_user(user, "You have manually saved %d user's details.\n", cnt);
		}
	else write_syslog(SYSLOG, 1, "Automatic saved %d user's details.\n", cnt);
}


/* Allow a user to move between rooms again */
void unshackle(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (user==u) {
  write_user(user,"Nemozes odlepit seba !\n");
  return;
  }
if (u->lroom!=2) {
  vwrite_user(user,"%s~RS in not currently shackled.\n",u->recap);
  return;
  }
u->lroom=0;
write_user(u,"\n~FG~OLYou have been unshackled.\n");
write_user(u,"You can now use the ~FTset~RS command to alter the ~FBroom~RS attribute.\n");
vwrite_user(user,"~FG~OLYou unshackled %s from the %s room.\n",u->bw_recap,u->room->name);
sprintf(text,"~FGUnshackled~RS from the ~FB%s~RS room by ~FB~OL%s~RS.\n",u->room->name,user->name);
add_history(u->name,1,text);
write_syslog(SYSLOG,1,"%s UNSHACKLED %s from the room: %s\n",user->name,u->name,u->room->name);
}


/* Stop a user from using the go command and leaving the room they are currently in */
void shackle(UR_OBJECT user)
{
	UR_OBJECT u;

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user> [-cancel]", command_table[SHACKLE].name);
		return;
		}
	if (!strcmp(word[2], "-cancel")) {
		unshackle(user);
		return;
		}
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (user==u) {
  write_user(user,"Nemozes prilepit seba !\n");
  return;
  }
#ifdef NETLINKS
  if (u->room==NULL) {
    vwrite_user(user,"%s is currently off site and cannot be shackled there.\n",u->name);
    return;
    }
#endif
if (u->level>=user->level) {
  write_user(user,"You cannot shackle someone of the same or higher level as yourself.\n");
  return;
  }
if (u->lroom==2) {
  vwrite_user(user,"%s~RS has already been shackled.\n",u->recap);
  return;
  }
u->lroom=2;
vwrite_user(u,"\n~FR~OLYou have been shackled to the %s room.\n",u->room->name);
vwrite_user(user,"~FR~OLYou shackled %s to the %s room.\n",u->bw_recap,u->room->name);
sprintf(text,"~FRShackled~RS to the ~FB%s~RS room by ~FB~OL%s~RS.\n",u->room->name,user->name);
add_history(u->name,1,text);
write_syslog(SYSLOG,1,"%s SHACKLED %s to the room: %s\n",user->name,u->name,u->room->name);
}


/*** Unretire a user - ie, put them back on show on the wizlist ***/
void unretire_user(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
word[1][0]=toupper(word[1][0]);
if (!in_retire_list(word[1])) {
  vwrite_user(user,"%s has not been retired from the wizlist.\n",word[1]);
  return;
  }
if ((u=get_user_name(user,word[1]))) {
  if (u==user) {
    write_user(user,"You cannot unretire yourself.\n");
    return;
    }
  if (u->level<WIZ) {
    write_user(user,"You cannot retire anyone under the level WIZ.\n");  return;
    }
  clean_retire_list(u->name);
  vwrite_user(user,"You unretire %s and put them back on the wizlist.\n",u->name);
  write_user(u,"You have been unretired and placed back on the wizlist.\n");
  write_syslog(SYSLOG,1,"%s UNRETIRED %s\n",user->name,u->name);
  sprintf(text,"Was ~FGunretired~RS by %s.\n",user->name);
  add_history(u->name,1,text);
  return;
  }
/* Create a temp session, load details, alter , then save. This is inefficient
   but its simpler than the alternative */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in unretire_user().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
if (!in_retire_list(u->name)) {
  vwrite_user(user,"%s has not been retired.\n",word[1]);
  destruct_user(u);
  destructed=0;
  return;
  }
clean_retire_list(u->name);
vwrite_user(user,"You unretire %s and put them back on the wizlist.\n",u->name);
send_mail(user,u->name,"~OLYou have been ~FGunretired~RS~OL and put back on the wizlist.\n",0);
write_syslog(SYSLOG,1,"%s UNRETIRED %s.\n",user->name,u->name);
sprintf(text,"Was ~FGunretired~RS by %s.\n",user->name);
add_history(u->name,1,text);
destruct_user(u);
destructed=0;
}


/*** retire a member of the law - ie, remove from the wizlist but don't alter level ***/
void retire_user(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user> [-cancel]", command_table[RETIRE].name);
  return;
  }
if (!strcmp(word[2], "-cancel")) {
	unretire_user(user);
	return;
	}
word[1][0]=toupper(word[1][0]);
if (in_retire_list(word[1])) {
  vwrite_user(user,"%s has already been retired from the wizlist.\n",word[1]);
  return;
  }
if ((u=get_user_name(user,word[1]))) {
  if (u==user) {
    write_user(user,"You cannot retire yourself.\n");
    return;
    }
  if (u->level<WIZ) {
    write_user(user,"You cannot retire anyone under the level WIZ\n");  return;
    }
  add_retire_list(u->name);
  vwrite_user(user,"You retire %s from the wizlist.\n",u->name);
  write_user(u,"You have been retired from the wizlist but still retain your level.\n");
  write_syslog(SYSLOG,1,"%s RETIRED %s\n",user->name,u->name);
  sprintf(text,"Was ~FRretired~RS by %s.\n",user->name);
  add_history(u->name,1,text);
  return;
  }
/* Create a temp session, load details, alter , then save. This is inefficient
   but its simpler than the alternative */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in retire_user().\n");
  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);  
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level<WIZ) {
  write_user(user,"You cannot retire anyone under the level WIZ.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
add_retire_list(u->name);
vwrite_user(user,"You retire %s from the wizlist.\n",u->name);
write_syslog(SYSLOG,1,"%s RETIRED %s\n",user->name,u->name);
sprintf(text,"Was ~FRretired~RS by %s.\n",user->name);
add_history(u->name,1,text);
send_mail(user,u->name,"~OLYou have been ~FRretired~RS~OL from the wizlist but still retain your level.\n",0);
destruct_user(u);
destructed=0;
}


/**** Show the amount of memory that the objects are currently taking up ***/
void show_memory(UR_OBJECT user)
{
int ssize,usize,rsize,nsize,dsize,csize,lsize,wsize,plsize,tcmsize,total,i;
int tusize,trsize,tnsize,tdsize,tcsize,tlsize,twsize,tplsize,cmsize;
int sppsize;
float mb;
UR_OBJECT u;
RM_OBJECT r;
#ifdef NETLINKS
  NL_OBJECT n;
#endif
PL_OBJECT pl;
CM_OBJECT cm;
struct command_struct *c;
struct user_dir_struct *d;
struct wiz_list_struct *w;

	set_crash();
ssize=usize=rsize=nsize=dsize=csize=lsize=wsize=plsize=cmsize=0;
tusize=trsize=tnsize=tdsize=tcsize=tlsize=twsize=tplsize=tcmsize=0;
sppsize=0;

ssize=sizeof(struct system_struct);
sppsize=sizeof(struct syspp_struct);
usize=sizeof(struct user_struct);
for (u=user_first;u!=NULL;u=u->next) tusize+=sizeof(struct user_struct);
rsize=sizeof(struct room_struct);
for (r=room_first;r!=NULL;r=r->next) trsize+=sizeof(struct room_struct);
#ifdef NETLINKS
  nsize=sizeof(struct netlink_struct);
  for (n=nl_first;n!=NULL;n=n->next) tnsize+=sizeof(struct netlink_struct);
#endif
for (pl=plugin_first; pl!=NULL; pl=pl->next) tplsize+=sizeof(struct plugin_struct);
plsize=sizeof(struct plugin_struct);
for (cm=cmds_first; cm!=NULL; cm=cm->next) tcmsize+=sizeof(struct plugin_cmd);
cmsize=sizeof(struct plugin_cmd);
dsize=sizeof(struct user_dir_struct);
for (d=first_dir_entry;d!=NULL;d=d->next) tdsize+=sizeof(struct user_dir_struct);
csize=sizeof(struct command_struct);
for (c=first_command;c!=NULL;c=c->next) tcsize+=sizeof(struct command_struct);
lsize=sizeof(last_login_info[0]);
for (i=0;i<LASTLOGON_NUM;i++) tlsize+=sizeof(last_login_info[i]);
wsize=sizeof(struct wiz_list_struct);
for (w=first_wiz_entry;w!=NULL;w=w->next) twsize+=sizeof(struct wiz_list_struct);
total=tusize+trsize+tnsize+tdsize+tcsize+tlsize+twsize+ssize+tplsize+tcmsize+sppsize;
mb=(float)total/1048576;

write_user(user,"+----------------------------------------------------------------------------+\n");
write_user(user,"| ~OL~FTMemory Object Allocation~RS                                                   |\n");
write_user(user,"|----------------------------------------------------------------------------|\n");
vwrite_user(user,"|    user structure : %8d bytes    directory structure : %8d bytes |\n",usize,dsize);
vwrite_user(user,"|                   : ~OL%8d~RS bytes                        : ~OL%8d~RS bytes |\n",tusize,tdsize);
vwrite_user(user,"|    room structure : %8d bytes      command structure : %8d bytes |\n",rsize,csize);
vwrite_user(user,"|                   : ~OL%8d~RS bytes                        : ~OL%8d~RS bytes |\n",trsize,tcsize);
vwrite_user(user,"| wizlist structure : %8d bytes   last login structure : %8d bytes |\n",wsize,lsize);
vwrite_user(user,"|                   : ~OL%8d~RS bytes                        : ~OL%8d~RS bytes |\n",twsize,tlsize);
vwrite_user(user,"|  plugin structure : %8d bytes  plugin cmds structure : %8d bytes |\n",plsize,cmsize);
vwrite_user(user,"|                   : ~OL%8d~RS bytes                        : ~OL%8d~RS bytes |\n",tplsize,tcmsize);
#ifdef NETLINKS
  vwrite_user(user,"| netlink structure : %8d bytes                                         |\n",nsize);
  vwrite_user(user,"|                   : ~OL%8d~RS bytes                                         |\n",tnsize);
#endif
vwrite_user(user,"|  system structure : %8d bytes     systempp structure : %8d bytes |\n",ssize,sppsize);
vwrite_user(user,"|                   : ~OL%8d~RS bytes                        : ~OL%8d~RS bytes |\n",ssize,sppsize);
write_user(user,"+----------------------------------------------------------------------------+\n");
vwrite_user(user,   "| Total object memory allocated : ~OL%9d~RS bytes   (%02.3f Mb)               |\n",total,mb);
write_user(user,"+----------------------------------------------------------------------------+\n\n");
}

/*** Display how many times a command has been used, and its overall 
     percentage of showings compared to other commands
     ***/
void show_command_counts(UR_OBJECT user)
{
struct command_struct *cmd;
int total_hits,total_cmds,cmds_used,i,x;
char text2[ARR_SIZE];

	set_crash();
x=i=total_hits=total_cmds=cmds_used=0;
text2[0]='\0';
/* get totals of commands and hits */
for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
  total_hits+=cmd->count;
  total_cmds++;
  }
write_user(user,"\n+----------------------------------------------------------------------------+\n");
write_user(user,"| ~FT~OLStatistika pouzivania prikazov~RS                                             |\n");
write_user(user,"+----------------------------------------------------------------------------+\n");
for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
  /* skip if command has not been used so as not to cause crash by trying to / by 0 */
  if (!cmd->count) continue;
  ++cmds_used;
  /* skip if user cannot use that command anyway */
  if (cmd->min_lev>user->level) continue;
  i=((cmd->count*10000)/total_hits);
  /* build up first half of the string */
  if (!x) {
    sprintf(text,"| %11.11s %4d %3d%% ",cmd->name,cmd->count,i/100);
    ++x;
    }
  /* build up full line and print to user */
  else if (x==1) {
    sprintf(text2,"   %11.11s %4d %3d%%   ",cmd->name,cmd->count,i/100);
    strcat(text,text2);
    write_user(user,text);
    text[0]='\0';  text2[0]='\0';
    ++x;
    }
  else {
    sprintf(text2,"   %11.11s %4d %3d%%  |\n",cmd->name,cmd->count,i/100);
    strcat(text,text2);
    write_user(user,text);
    text[0]='\0';  text2[0]='\0';
    x=0;
    }
  } /* end for */
/* If you've only printed first half of the string */
if (x==1) {
  strcat(text,"                                                     |\n");
  write_user(user,text);
  }
if (x==2) {
  strcat(text,"                          |\n");
  write_user(user,text);
  }
write_user(user,"|                                                                            |\n");
write_user(user,"| Any other commands have not yet been used, or you cannot view them         |\n");
write_user(user,"+----------------------------------------------------------------------------+\n");
sprintf(text2,"Total of ~OL%d~RS commands.    ~OL%d~RS command%s used a total of ~OL%d~RS time%s.",
	total_cmds,cmds_used,PLTEXT_S(cmds_used),total_hits,PLTEXT_S(total_hits));
vwrite_user(user,"| %-92s |\n",text2);
write_user(user,"+----------------------------------------------------------------------------+\n");
}



/*** read all the user files to check if a user exists ***/
void recount_users(UR_OBJECT user, int ok)
{
int level,incorrect,correct,inlist,notin,added,removed;
char name[USER_NAME_LEN+3],filename[500];
DIR *dirp;
struct dirent *dp;
struct user_dir_struct *entry;
FILE *fp;
UR_OBJECT u;

	set_crash();
if (!ok) {
  write_user(user,"~OL~FRWARNING:~RS This process may take some time if you have a lot of user accounts.\n");
  write_user(user,"         This should only be done if there are no, or minimal, users currently\n         logged on.\n");
  write_user(user,"\nDo you wish to continue (y/n)? ");
  user->misc_op=17;
  return;
  }
write_user(user,"\n+----------------------------------------------------------------------------+\n");
level=-1;
incorrect=correct=added=removed=0;
write_user(user,"~OLRecounting all of the users...~RS\n");
/* First process the files to see if there are any to add to the directory listing */
write_user(user,"Processing users to add...");

/* open the directory file up */
dirp=opendir(USERFILES);
if (dirp==NULL) {
  write_user(user,"ERROR: Failed to open userfile directory.\n");
  write_syslog(ERRLOG,1,"Directory open failure in recount_users().\n");
  return;
  }
if ((u=create_user())==NULL) {
  write_user(user,"ERROR: Cannot create user object.\n");
  write_syslog(ERRLOG,1,"Cannot create user object in recount_users().\n");
  (void) closedir(dirp);
  return;
  }
/* count up how many files in the directory - this include . and .. */
while((dp=readdir(dirp))!=NULL) {
  if (!strcmp(dp->d_name,".") || !strcmp(dp->d_name,"..")) continue;
  if (strstr(dp->d_name,".D")) {
    strcpy(name,dp->d_name);
    name[strlen(name)-2]='\0';
    inlist=0;
    for (entry=first_dir_entry;entry!=NULL;entry=entry->next) {
      if (strcmp(name,entry->name)) continue;
      inlist=1;  break;
      }
    if (!inlist) {
      strcpy(u->name,name);
      if (load_user_details(u)) {
	add_user_node(u->name,u->level);
	amsys->level_count[u->level]++;
	write_syslog(SYSLOG,0,"Added new user node for existing user '%s'\n",name);
	++added;
	reset_user(u);
        }
      } /* end if inlist */
    else ++correct;
    }
  }
(void) closedir(dirp);
destruct_user(u);

/* Now process any nodes to remove the directory listing.  This may not be optimal to do one loop
   to add and then one to remove, but it's the best way I can think of doing it right now at 4:27am!
   */
write_user(user,"\nProcessing users to remove...");

/* Ok, now I know a lot of people think goto calls are the spawn of Satan, but it was, again, the
   best I could come up with, without having to copy the entire user_dir_struct - which could be
   pretty big if you have a lot of users.  If you don't like it, come up with something better and
   let me know...
   Done this way because you could destruct a node and then try to move onto the next one, but the
   link to do this, of course, has been destructed...  Let me know if I'm wrong!
   */
START_LIST:
notin=0;
entry=first_dir_entry;
while (entry!=NULL) {
  strcpy(name,entry->name);  level=entry->level;
  sprintf(filename,"%s/%s.D", USERFILES,name);
  if (!(fp=fopen(filename,"r"))) {
    notin=1;  break;
    }
  else fclose(fp);
  entry=entry->next;
  }
/* remove the node */
if (notin) {
  write_syslog(SYSLOG,0,"Removed user node for '%s' - user file does not exist.\n",name);
  ++removed;
  --correct;
  rem_user_node(name,level);
  amsys->level_count[level]--;
  goto START_LIST;
  }

/* now to make sure that the user level counts are correct as show in .version */
count_users();
write_user(user,"\n+----------------------------------------------------------------------------+\n");
vwrite_user(user,"Checked ~OL%d~RS user%s.  ~OL%d~RS node%s %s added, and ~OL%d~RS node%s %s removed.\n",
	added+removed+correct,PLTEXT_S(added+removed+correct),
        added,PLTEXT_S(added),PLTEXT_WAS(added),
        removed,PLTEXT_S(removed),PLTEXT_WAS(removed));
if (incorrect) write_user(user,"See the system log for further details.\n");
write_user(user,"+----------------------------------------------------------------------------+\n");
user->misc_op=0;
}


/*** Allows a user to alter the minimum level which can use the command given ***/
void set_command_level(UR_OBJECT user)
{
struct command_struct *cmd;
int new_lev,found;

	set_crash();
if (word_count<3) {
  write_usage(user,"%s <prikaz> <level>/norm", command_table[SETCMDLEV].name);
  return;
  }
found=0;
/* levels and 'norm' are checked in upper case */
strtoupper(word[2]);
if (!strcmp(word[2],"NORM")) {
  cmd=first_command;
  while (cmd!=NULL) {
    if (!strncmp(word[1],cmd->name,strlen(word[1]))) {
      if (cmd->min_lev==command_table[cmd->id].level) {
	write_user(user,"That command is already at its normal level.\n");
	return;
        }
      cmd->min_lev=command_table[cmd->id].level;
      found=1;
      break;
      }
    cmd=cmd->next;
    } /* end while */
  if (found) {
    write_syslog(SYSLOG,1,"%s has returned level to normal for cmd '%s'\n",user->name,cmd->name);
    write_monitor(user,NULL,0);
    vwrite_room(NULL,"~OL~FR--==<~RS The level for command ~OL%s~RS has been returned to %s ~OL~FR>==--\n",cmd->name,user_level[cmd->min_lev].name);
    return;
    }
  else {
    vwrite_user(user,"The command '~OL%s~RS' could not be found.\n",word[1]);
    return;
    }
  } /* end if 'norm' */
if ((new_lev=get_level(word[2]))==-1) {
  write_usage(user,"%s <command name> <level name>/norm", command_table[SETCMDLEV].name);
  return;
  }
if (new_lev>user->level) {
  write_user(user,"You cannot set a command's level to one greater than your own.\n");
  return;
  }
found=0;
cmd=first_command;
while (cmd!=NULL) {
  if (!strncmp(word[1],cmd->name,strlen(word[1]))) {
    if (command_table[cmd->id].level>user->level) {
      write_user(user,"You are not a high enough level to alter that command's level.\n");
      return;
      }
    cmd->min_lev=new_lev;
    found=1;
    break;
    }
  cmd=cmd->next;
  } /* end while */
if (found) {
  write_syslog(SYSLOG,1,"%s has set the level for cmd '%s' to %d (%s)\n",user->name,cmd->name,cmd->min_lev,user_level[cmd->min_lev].name);
  write_monitor(user,NULL,0);
  vwrite_room(NULL,"~OL~FR--==<~RS The level for command ~OL%s~RS has been set to %s ~OL~FR>==--\n",cmd->name,user_level[cmd->min_lev].name);
  }
else vwrite_user(user,"The command '~OL%s~RS' could not be found.\n",word[1]);
} /* end set_command_level */


/*** stop a user from using a certain command ***/
void user_xcom(UR_OBJECT user)
{
int i,x,cmd_id;
struct command_struct *cmd;
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user> [<command>]", command_table[XCOM].name);
  return;
  }
if (!(u=get_user(word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (u==user) {
  write_user(user,"You cannot ban any commands of your own.\n");
  return;
  }
/* if no command is given, then just view banned commands */
if (word_count<3) {
  x=0;
  write_user(user,"+----------------------------------------------------------------------------+\n");
  vwrite_user(user,"~OL~FTBanned commands for user '%s'\n",u->name);
  write_user(user,"+----------------------------------------------------------------------------+\n");
  for (i=0;i<MAX_XCOMS;i++) {
    if (u->xcoms[i]!=-1) {
      for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
	if (cmd->id==u->xcoms[i]) {
	  vwrite_user(user,"~OL%s~RS (level %d)\n",cmd->name,cmd->min_lev);
	  x=1;
	  }
        } /* end for */
      } /* end if */
    } /* end for */
  if (!x) write_user(user,"User has no banned commands.\n");
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  return;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot ban the commands of a user with the same or higher level as yourself.\n");
  return;
  }
cmd_id=-1;
for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
  if (!strncmp(word[2],cmd->name,strlen(word[2]))) {
    if (u->level<cmd->min_lev) {
      vwrite_user(user,"%s is not of a high enough level to use that command anyway.\n",u->name);
      return;
      }
    cmd_id=cmd->id;
    break;
    }
  } /* end for */
if (cmd_id==-1) {
  write_user(user,"That command does not exist.\n");
  return;
  }
/* check to see is the user has previously been given the command */
if (has_gcom(u,cmd_id)) {
  write_user(user,"You cannot ban a command that a user has been specifically given.\n");
  return;
  }
/* user already has the command, so unabn it */
if (has_xcom(u,cmd_id)) {
  if (set_xgcom(user,u,cmd_id,1,0)) {
    vwrite_user(user,"You have unbanned the '%s' command for %s\n",word[2],u->name);
    vwrite_user(u,"The command '%s' has been unbanned and you can use it again.\n",word[2]);
    sprintf(text,"%s ~FGUNXCOM'd~RS the command '%s'\n",user->name,word[2]);
    add_history(u->name,1,text);
    write_syslog(SYSLOG,1,"%s UNXCOM'd the command '%s' for %s\n",user->name,word[2],u->name);
    return;
    }
  else return;
  }
/* user doesn't have the command, so ban it */
if (set_xgcom(user,u,cmd_id,1,1)) {
  vwrite_user(user,"You have banned the '%s' command for %s\n",word[2],u->name);
  vwrite_user(u,"You have been banned from using the command '%s'.\n",word[2]);
  sprintf(text,"%s ~FRXCOM'd~RS the command '%s'\n",user->name,word[2]);
  add_history(u->name,1,text);
  write_syslog(SYSLOG,1,"%s XCOM'd the command '%s' for %s\n",user->name,word[2],u->name);
  }
}


/*** stop a user from using a certain command ***/
void user_gcom(UR_OBJECT user)
{
int i,x,cmd_id;
struct command_struct *cmd;
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user> [<command>]", command_table[GCOM].name);
  return;
  }
if (!(u=get_user(word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (u==user) {
  write_user(user,"You cannot give yourself any commands.\n");
  return;
  }
/* if no command is given, then just view given commands */
if (word_count<3) {
  x=0;
  write_user(user,"+----------------------------------------------------------------------------+\n");
  vwrite_user(user,"~OL~FTGiven commands for user '%s'\n",u->name);
  write_user(user,"+----------------------------------------------------------------------------+\n");
  for (i=0;i<MAX_GCOMS;i++) {
    if (u->gcoms[i]!=-1) {
      for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
	if (cmd->id==u->gcoms[i]) {
	  vwrite_user(user,"~OL%s~RS (level %d)\n",cmd->name,cmd->min_lev);
	  x=1;
	  }
        } /* end for */
      } /* end if */
    } /* end for */
  if (!x) write_user(user,"User has no given commands.\n");
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  return;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot give commands to a user with the same or higher level as yourself.\n");
  return;
  }
cmd_id=-1;
for (cmd=first_command;cmd!=NULL;cmd=cmd->next) {
  if (!strncmp(word[2],cmd->name,strlen(word[2]))) {
    if (u->level>cmd->min_lev) {
      vwrite_user(user,"%s can already use that command.\n",u->name);
      return;
      }
    cmd_id=cmd->id;
    break;
    }
  } /* end for */
if (cmd_id==-1) {
  write_user(user,"That command does not exist.\n");
  return;
  }
/* check to see if the user has previously been banned from using the command */
if (has_xcom(u,cmd_id)) {
  write_user(user,"You cannot give a command to a user that already has it banned.\n");
  return;
  }
/* user already has the command, so ungive it */
if (has_gcom(u,cmd_id)) {
  if (set_xgcom(user,u,cmd_id,0,0)) {
    vwrite_user(user,"You have removed the given command '%s' for %s\n",word[2],u->name);
    vwrite_user(u,"Access to the given command '%s' has now been taken away from you.\n",word[2]);
    sprintf(text,"%s ~FRUNGCOM'd~RS the command '%s'\n",user->name,word[2]);
    add_history(u->name,1,text);
    write_syslog(SYSLOG,1,"%s UNGCOM'd the command '%s' for %s\n",user->name,word[2],u->name);
    return;
    }
  else return;
  }
/* user doesn't have the command, so give it */
if (set_xgcom(user,u,cmd_id,0,1)) {
  vwrite_user(user,"You have given the '%s' command for %s\n",word[2],u->name);
  vwrite_user(u,"You have been given access to the command '%s'.\n",word[2]);
  sprintf(text,"%s ~FGGCOM'd~RS the command '%s'\n",user->name,word[2]);
  add_history(u->name,1,text);
  write_syslog(SYSLOG,1,"%s GCOM'd the command '%s' for %s\n",user->name,word[2],u->name);
  }
}


/*** Reloads the description for one or all rooms - incase you have edited the
     file and don't want to reboot the talker to to make the changes displayed
     ***/
void reload_room_description(UR_OBJECT user, int w)
{
int i,error;
RM_OBJECT rm;
char c,filename[500],pat[4];
FILE *fp;

	set_crash();
/* if reload all of the rooms */
if (!strcmp(word[1+w],"-a")) {
  error=0;
  for(rm=room_first;rm!=NULL;rm=rm->next) {
    if (rm->access==PERSONAL_UNLOCKED || rm->access==PERSONAL_LOCKED) continue;
    if (rm->transp==NULL) sprintf(filename,"%s/%s.R", ROOMFILES, rm->name);
    else sprintf(filename,"%s/%s.R", TRFILES, rm->name);
    if (!(fp=fopen(filename,"r"))) {
      vwrite_user(user,"Sorry, cannot reload the description file for the room '%s'.\n",rm->name);
      write_syslog(ERRLOG,1,"Couldn't reload the description file for room %s.\n",rm->name);
      ++error;
      continue;
      }
    i=0;
    c=getc(fp);
    while(!feof(fp)) {
      if (i==ROOM_DESC_LEN) {
	vwrite_user(user,"The description is too long for the room '%s'.\n",rm->name);
	write_syslog(ERRLOG,1,"Description too long when reloading for room %s.\n",rm->name);
	break;
        } /* end if */
      rm->desc[i]=c;
      c=getc(fp);
      ++i;
      } /* end while */
    rm->desc[i]='\0';
    fclose(fp);
    } /* end for */
  if (!error) write_user(user,"You have now reloaded all room descriptions.\n");
  else  write_user(user,"You have now reloaded all room descriptions that you can.\n");
  write_syslog(SYSLOG,1,"%s reloaded all of the room descriptions.\n",user->name);
  return;
  } /* end if */
/* if it's just one room to reload */
/* check first for personal room, and don't reload */
pat[0]='\0';
strcpy(pat,"(*)");
if (pattern_match(word[1+w],pat)) {
  write_user(user,"Sorry, but you cannot reload personal room descriptions.\n");
  return;
  }
if ((rm=get_room(word[1+w]))==NULL) {
  write_user(user,nosuchroom);
  return;
  }
if (rm->transp==NULL) sprintf(filename,"%s/%s.R", ROOMFILES, rm->name);
else sprintf(filename,"%s/%s.R", TRFILES, rm->name);
if (!(fp=fopen(filename,"r"))) {
  vwrite_user(user,"Sorry, cannot reload the description file for the room '%s'.\n",rm->name);
  write_syslog(ERRLOG,1,"Couldn't reload the description file for room %s.\n",rm->name);
  return;
  }
i=0;
c=getc(fp);
while(!feof(fp)) {
  if (i==ROOM_DESC_LEN) {
    vwrite_user(user,"The description is too long for the room '%s'.\n",rm->name);
    write_syslog(ERRLOG,1,"Description too long when reloading for room %s.\n",rm->name);
    break;
    }
  rm->desc[i]=c;
  c=getc(fp);
  ++i;
  }
rm->desc[i]='\0';
fclose(fp);
vwrite_user(user,"You have now reloaded the desctiption for the room '%s'.\n",rm->name);
write_syslog(SYSLOG,1,"%s reloaded the description for the room %s\n",user->name,rm->name);
}



/*** Force a user to do something ***/
/*** adapted from Ogham - Oh God Here's Another MUD - (c) Neil Robertson ***/
void force(UR_OBJECT user, char *inpstr) {
UR_OBJECT u;
int w;

if (word_count<3) {
  write_usage(user,"%s <user> <action>", command_table[FORCE].name);
  return;
  }
word[1][0]=toupper(word[1][0]);
if ((u=get_user_name(user,word[1]))==NULL) {
  write_user(user,notloggedon);
  return;
  }
if (u==user) {
  write_user(user,"There is an easier way to do something yourself.\n");
  return;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot force a user of the same or higher level as yourself.\n");
  return;
  }
if (u->afk) {
	write_user(user, "User momentalne nemoze vykonat ziadny rozkaz ...\n");
	return;
	}
inpstr=remove_first(inpstr);
write_syslog(SYSLOG,0,"%s FORCED %s to: '%s'\n",user->name,u->name,inpstr);

/* shift words down to pass to exec_com */
for (w=2;w<word_count;++w) strcpy(word[w-2],word[w]);
word[w][0]='\0';  
word[w+1][0]='\0';
word_count-=2;

vwrite_user(u,"%s forces you to: '%s'\n",user->name,inpstr);
vwrite_user(user,"You force %s to: '%s'\n",u->name,inpstr);
if (!exec_com(u,inpstr)) vwrite_user(user,"Unable to execute the command for %s.\n",u->name);
prompt(u);
}


/* this function allows admin to control personal rooms */
void personal_room_admin(UR_OBJECT user) {
RM_OBJECT rm;
int rsize,trsize,rmcnt,locked,unlocked,pcnt;
char usrname[USER_NAME_LEN+1],filename[100];

if (word_count<2) {
  write_usage(user,"%s -l / -m / -u <name> / -d <name>", command_table[RMADMIN].name);
  return;
  }
if (!amsys->personal_rooms) {
  write_user(user,"Personal room functions are currently disabled.\n");
  return;
  }
strtolower(word[1]);
/* just display the amount of memory used by personal rooms */
if (!strcmp(word[1],"-m")) {
  write_user(user,"+----------------------------------------------------------------------------+\n");
  write_user(user,"| ~FT~OLPersonal Room Memory Usage~RS                                                 |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  rsize=trsize=rmcnt=locked=unlocked=0;
  rsize=sizeof(struct room_struct);
  for (rm=room_first;rm!=NULL;rm=rm->next) {
    if (is_personal_room(rm)) {
      (rm->access==PERSONAL_LOCKED) ? locked++ : unlocked++;
      trsize+=sizeof(struct room_struct);
      rmcnt++;
      }
    }
  vwrite_user(user,"| room structure : ~OL%4d~RS bytes      total memory : ~OL%8d~RS bytes  (%02.3f Mb) |\n",rsize,trsize,(float)trsize/1000000);
  vwrite_user(user,"|    total rooms : ~OL%4d~RS                  status : ~OL%2d~RS locked, ~OL%2d~RS unlocked     |\n",rmcnt,locked,unlocked);
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  return;
  }
/* list all the personal rooms in memory together with status */
if (!strcmp(word[1],"-l")) {
  rmcnt=0;
  write_user(user,"+----------------------------------------------------------------------------+\n");
  write_user(user,"| ~OL~FTPersonal Room Listings~RS                                                     |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  for (rm=room_first;rm!=NULL;rm=rm->next) {
    if (is_personal_room(rm)) {
      pcnt=room_visitor_count(rm);
      midcpy(rm->name,usrname,1,strlen(rm->name)-2);
      usrname[0]=toupper(usrname[0]);
      vwrite_user(user,"| Owner : ~OL%-*s~RS       Status : ~OL%s~RS   Msg Count : ~OL%-2d~RS  People : ~OL%-2d~RS |\n",
	      USER_NAME_LEN,usrname,(rm->access==PERSONAL_LOCKED)?"~FRlocked  ":"~FGunlocked",rm->mesg_cnt,pcnt);
      rmcnt++;
      }
    }
  if (!rmcnt) write_user(user,"| ~FRNo personal rooms are currently in memory~RS                                  |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  if (rmcnt) {
    vwrite_user(user,"| Total personal rooms : ~OL~FM%2d~RS                                                  |\n",rmcnt);
    write_user(user,"+----------------------------------------------------------------------------+\n\n");
    }
  return;
  }
/* unload a room from memory or delete it totally - all rooms files */ 
if (!strcmp(word[1],"-u") || !strcmp(word[1],"-d")) {
  if (word_count<3) {
    write_usage(user,"%s -l / -m / -u <name> / -d <name>", command_table[RMADMIN].name);
    return;
    }
  sprintf(usrname,"(%s)",word[2]);
  strtolower(usrname);
  /* first do checks on the room */
  if ((rm=get_room_full(usrname))==NULL) {
    write_user(user,"That user does not have a personal room built.\n");
    return;
    }
  pcnt=0;
  pcnt=room_visitor_count(rm);
  if (pcnt) {
    write_user(user,"You cannot remove a room if people are in it.\n");
    return;
    }
  destruct_room(rm);
  strtolower(word[2]);
  word[2][0]=toupper(word[2][0]);
  /* delete all files */
  if (!strcmp(word[1],"-d")) {
    sprintf(filename,"%s/%s.R", USERROOMS,word[2]);
    unlink(filename);
    sprintf(filename,"%s/%s.B", USERROOMS,word[2]);
    unlink(filename);
    write_syslog(SYSLOG,1,"%s deleted the personal room of %s.\n",user->name,word[2]);
    vwrite_user(user,"You have now ~OL~FRdeleted~RS the room belonging to %s.\n",word[2]);
    }
  /* just unload from memory */
  else {
    write_syslog(SYSLOG,1,"%s unloaded the personal room of %s from memory.\n",user->name,word[2]);
    vwrite_user(user,"You have now ~OL~FGunloaded~RS the room belonging to %s from memory.\n",word[2]);
    }
  return;
  }
/* wrong input given */
write_usage(user,"%s -l / -m / -u <name> / -d <name>", command_table[RMADMIN].name);
}


/*** Allows you to dump certain things to files as a record ***/
void dump_to_file(UR_OBJECT user) {
char filename[500],bstr[40];
FILE *fp;
int ssize,usize,rsize,nsize,dsize,csize,lsize,wsize,total,i,j;
int tusize,trsize,tnsize,tdsize,tcsize,tlsize,twsize,lev,cnt;
int days,hours,mins,secs;
int sppsize, plsize, tplsize, cmsize, tcmsize;
float mb;
UR_OBJECT u;
RM_OBJECT r;
PL_OBJECT pl;
CM_OBJECT cm;
#ifdef NETLINKS
  NL_OBJECT n;
#endif
struct command_struct *c;
struct user_dir_struct *d,*entry;
struct wiz_list_struct *w;

u=NULL;
r=NULL;
#ifdef NETLINKS
  n=NULL;
#endif
pl=NULL;
cm=NULL;
ssize=usize=rsize=nsize=dsize=csize=lsize=wsize=i=j=cnt=0;
tusize=trsize=tnsize=tdsize=tcsize=tlsize=twsize=0;
days=hours=mins=secs=0;
sppsize=plsize=tplsize=cmsize=tcmsize=0;
lev=-1;

if (word_count<2) {
  write_usage(user,"%s -u/-r <rank>/-c/-m/-s", command_table[DUMPCMD].name);
  return;
  }
strtolower(word[1]);
/* see if -r switch was used : dump all users of given level */
if (!strcmp("-r",word[1])) {
  if (word_count<3) {
    write_usage(user,"%s -r <rank>", command_table[DUMPCMD].name);
    return;
    }
  strtoupper(word[2]);
  if ((lev=get_level(word[2]))==-1) {
    write_usage(user,"%s -r <rank>", command_table[DUMPCMD].name);
    return;
    }
  sprintf(filename,"%s/%s.dump", DUMPFILES,user_level[lev].name);
  if (!(fp=fopen(filename,"w"))) {
    write_user(user,"There was an error trying to open the file to dump to.\n");
    write_syslog(SYSLOG,0,"Unable to open dump file %s in dump_to_file().\n",filename);
    return;
    }
  cnt=0;
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"Users of level %s %s\n",user_level[lev].name,long_date(1));
  fprintf(fp,"------------------------------------------------------------------------------\n");
  for (entry=first_dir_entry;entry!=NULL;entry=entry->next)
    if (entry->level==lev) {
      fprintf(fp,"%s\n",entry->name);
      ++cnt;
      }
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"Total users at %s : %d\n",user_level[lev].name,cnt);
  fprintf(fp,"------------------------------------------------------------------------------\n\n");
  fclose(fp);
  sprintf(text,"Dumped rank ~OL%s~RS to file.  ~OL%d~RS user%s recorded.\n",user_level[lev].name,cnt,PLTEXT_S(cnt)); 
  write_user(user,text);
  return;
  }
/* check to see if -u switch was used : dump all users */
if (!strcmp("-u",word[1])) {
  sprintf(filename,"%s/users.dump", DUMPFILES);
  if (!(fp=fopen(filename,"w"))) {
    write_user(user,"There was an error trying to open the file to dump to.\n");
    write_syslog(SYSLOG,0,"Unable to open dump file %s in dump_to_file().\n",filename);
    return;
    }
  cnt=0;
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"All users %s\n",long_date(1));
  fprintf(fp,"------------------------------------------------------------------------------\n");
  for (entry=first_dir_entry;entry!=NULL;entry=entry->next) {
    fprintf(fp,"%s\n",entry->name);
    ++cnt;
    }
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"Total users : %d\n",cnt);
  fprintf(fp,"------------------------------------------------------------------------------\n\n");
  fclose(fp);
  sprintf(text,"Dumped all users to file.  ~OL%d~RS user%s recorded.\n",cnt,PLTEXT_S(cnt)); 
  write_user(user,text);
  return;
  }
/* check to see if -c switch was used : dump last few commands used */
if (!strcmp("-c",word[1])) {
  sprintf(filename,"%s/cmds.dump", DUMPFILES);
  if (!(fp=fopen(filename,"w"))) {
    write_user(user,"There was an error trying to open the file to dump to.\n");
    write_syslog(SYSLOG,0,"Unable to open dump file %s in dump_to_file().\n",filename);
    return;
    }
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"The last 16 commands %s\n",long_date(1));
  fprintf(fp,"------------------------------------------------------------------------------\n");
  for (i=((j=amsys->last_cmd_cnt-16)>0?j:0);i<amsys->last_cmd_cnt;i++) fprintf(fp,"%s\n",cmd_history[i&15]);
  fprintf(fp,"------------------------------------------------------------------------------\n\n");
  fclose(fp);
  write_user(user,"Dumped the last 16 commands that have been used.\n");
  return;
  }
/* check to see if -m was used : dump memory currently being used */
if (!strcmp("-m",word[1])) {
  sprintf(filename,"%s/memory.dump", DUMPFILES);
  if (!(fp=fopen(filename,"w"))) {
    write_user(user,"There was an error trying to open the file to dump to.\n");
    write_syslog(SYSLOG,0,"Unable to open dump file %s in dump_to_file().\n",filename);
    return;
    }
  ssize=sizeof(struct system_struct);
  sppsize=sizeof(struct syspp_struct);
  usize=sizeof(struct user_struct);
  for (u=user_first;u!=NULL;u=u->next) tusize+=sizeof(struct user_struct);
  rsize=sizeof(struct room_struct);
  for (r=room_first;r!=NULL;r=r->next) trsize+=sizeof(struct room_struct);
#ifdef NETLINKS
  nsize=sizeof(struct netlink_struct);
  for (n=nl_first;n!=NULL;n=n->next) tnsize+=sizeof(struct netlink_struct);
#endif
  dsize=sizeof(struct user_dir_struct);
  for (d=first_dir_entry;d!=NULL;d=d->next) tdsize+=sizeof(struct user_dir_struct);
  csize=sizeof(struct command_struct);
  for (c=first_command;c!=NULL;c=c->next) tcsize+=sizeof(struct command_struct);
  lsize=sizeof(last_login_info[0]);
  for (i=0;i<LASTLOGON_NUM;i++) tlsize+=sizeof(last_login_info[i]);
  wsize=sizeof(struct wiz_list_struct);
  for (w=first_wiz_entry;w!=NULL;w=w->next) twsize+=sizeof(struct wiz_list_struct);
  plsize=sizeof(struct plugin_struct);
  for (pl=plugin_first; pl!=NULL; pl=pl->next) tplsize+=sizeof(struct plugin_struct);
  cmsize=sizeof(struct plugin_cmd);
  for (cm=cmds_first; cm!=NULL; cm=cm->next) tcmsize+=sizeof(struct plugin_cmd);
  total=tusize+trsize+tnsize+tdsize+tcsize+tlsize+twsize+ssize+tplsize+tcmsize+sppsize;
  mb=(float)total/1048576;
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"Memory Object Allocation %s\n",long_date(1));
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"     user structure : %8d bytes    directory structure : %8d bytes\n",usize,dsize);
  fprintf(fp,"              total : %8d bytes                  total : %8d bytes\n",tusize,tdsize);
  fprintf(fp,"     room structure : %8d bytes      command structure : %8d bytes\n",rsize,csize);
  fprintf(fp,"              total : %8d bytes                  total : %8d bytes\n",trsize,tcsize);
  fprintf(fp,"  wizlist structure : %8d bytes   last login structure : %8d bytes\n",wsize,lsize);
  fprintf(fp,"              total : %8d bytes                  total : %8d bytes\n",twsize,tlsize);
#ifdef NETLINKS
  fprintf(fp,"  netlink structure : %8d bytes\n",nsize);
  fprintf(fp,"              total : %8d bytes\n",tnsize);
#endif
  fprintf(fp,"   system structure : %8d bytes     systempp structure : %8d bytes\n",ssize,sppsize);
  fprintf(fp,"              total : %8d bytes                  total : %8d bytes\n",ssize,sppsize);
  fprintf(fp,"   plugin structure : %8d bytes  plugin cmds structure : %8d bytes\n",plsize,cmsize);
  fprintf(fp,"              total : %8d bytes                  total : %8d bytes\n",tplsize,tcmsize);
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"Total object memory allocated : %9d bytes   (%02.3f Mb)\n",total,mb);
  fprintf(fp,"------------------------------------------------------------------------------\n\n");
  fclose(fp);
  write_user(user,"Dumped the memory currently being used by the talker.\n");
  return;
  }
/* check to see if -s switch was used : show system details */
if (!strcmp("-s",word[1])) {
  sprintf(filename,"%s/system.dump", DUMPFILES);
  if (!(fp=fopen(filename,"w"))) {
    write_user(user,"There was an error trying to open the file to dump to.\n");
    write_syslog(SYSLOG,0,"Unable to open dump file %s in dump_to_file().\n",filename);
    return;
    }
  strcpy(bstr,ctime(&amsys->boot_time));
  secs=(int)(time(0)-amsys->boot_time);
  days=secs/86400;
  hours=(secs%86400)/3600;
  mins=(secs%3600)/60;
  secs=secs%60;
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"System details %s\n",long_date(1));
  fprintf(fp,"------------------------------------------------------------------------------\n");
  fprintf(fp,"System name : %s, release %s, %s\n",amsys->sysname,amsys->sysrelease,amsys->sysversion);
  fprintf(fp,"Running on  : %s, %s\n",amsys->sysmachine,amsys->sysnodename);
  fprintf(fp,"Talker PID  : %u\n",amsys->pid);
  fprintf(fp,"Booted      : %s",bstr);
  fprintf(fp,"Uptime      : %d day%s, %d hour%s, %d minute%s, %d second%s\n",
	  days,PLTEXT_S(days),hours,PLTEXT_S(hours),mins,PLTEXT_S(mins),secs,PLTEXT_S(secs));
#ifdef NETLINKS
  fprintf(fp,"Netlinks    : Compiled and running\n");
#else
  fprintf(fp,"Netlinks    : Not currently compiled or running\n");
#endif
  switch(amsys->resolve_ip) {
    case 0:
      fprintf(fp,"IP Resolve  : Not currently in use\n");
      break;
    case 1:
      fprintf(fp,"IP Resolve  : Resolving automatically with gethostbyaddr()\n");
      break;
    case 2:
      fprintf(fp,"IP Resolve  : Resolving IPs without gethostbyaddr()\n");
      break;
    }
  fprintf(fp,"------------------------------------------------------------------------------\n\n");
  fclose(fp);
  write_user(user,"Dumped the system details.\n");
  return;
  }
write_usage(user,"%s -u/-r <rank>/-c/-m/-s", command_table[DUMPCMD].name);
return;
}


/*** do a promotion of a user that lasts only for the current session ***/
void temporary_promote(UR_OBJECT user)
{
UR_OBJECT u;
int lev;

	set_crash();
 if (word_count<2) {
   write_usage(user,"%s <user> [<level>]", command_table[TEMPRO].name);
   return;
   }
 if (!(u=get_user_name(user,word[1]))) {
   write_user(user,notloggedon);
   return;
   }
 if (u==user) {
   write_user(user,"You can't promote yourself, temporarily or otherwise.\n");
   return;
   }
 /* determine what level to promote them to */
 if (u->level>=user->level) {
   write_user(user,"You can't temporarily promote anyone of the same or greater level than you.\n");
   return;
   }
 if (word_count==3) {
   if ((lev=get_level(word[2]))==-1) {
     write_usage(user,"%s <user> [<level>]", command_table[TEMPRO].name);
     return;
     }
   if (lev<=u->level) {
     vwrite_user(user,"You must specify a level higher than %s currently is.\n",u->name);
     return;
     }
   }
 else lev=u->level+1;
 if (lev==GOD) {
   vwrite_user(user,"You can't temporarily promote anoyone to level %s.\n",user_level[lev].name);
   return;
   }
 /* if they have already been temp promoted this session then restore normal level first */
 if (u->level>u->real_level) u->level=u->real_level;
 u->real_level=u->level;
 u->level=lev;
 vwrite_user(user,"You temporarily promote %s to %s.\n",u->name,user_level[u->level].name);
 vwrite_room_except(u->room,u,"~OL~FG%s~RS~OL~FG starts to glow as their power increases...\n",u->bw_recap);
 vwrite_user(u,"~OL~FGYou have been promoted (temporarily) to level %s.\n",user_level[u->level].name);
 write_syslog(SYSLOG,1,"%s TEMPORARILY promote %s to %s.\n",user->name,u->name,user_level[u->level].name);
 sprintf(text,"Was temporarily to level %s.\n",user_level[u->level].name);
 add_history(u->name,1,text);
}


/*** Change a user's name from their existing one to whatever ***/
void change_user_name(UR_OBJECT user)
{
UR_OBJECT u,usr;
char *name,oldname[ARR_SIZE],newname[ARR_SIZE],oldfile[500],newfile[500];
int i,on=0;
struct wiz_list_struct *wiz;
struct user_dir_struct *uds;

	set_crash();
if (word_count<3) {
  write_usage(user,"%s <old user name> <new user name>", command_table[MORPH].name);
  return;
  }
/* do a bit of setting up */
strcpy(oldname,colour_com_strip(word[1]));
strcpy(newname,colour_com_strip(word[2]));
strtolower(oldname);
strtolower(newname);
oldname[0]=toupper(oldname[0]);
newname[0]=toupper(newname[0]);
/* first check the given attributes */
if (!(find_user_listed(oldname))) {
  write_user(user,nosuchuser);
  return;
  }
if (find_user_listed(newname)) {
  write_user(user,"You cannot change the name to that of an existing user.\n");
  return;
  }
if (strlen(newname)>USER_NAME_LEN) {
  write_user(user,"The new name given was too long.\n");
  return;
  }
if (strlen(newname)<3) {
  write_user(user,"The new name given was too short.\n");
  return;
  }
for (i=0;i<strlen(newname);++i) {
  if (!isalpha(newname[i])) {
    write_user(user,"You cannot have anything but letters in the new name.\n");
    return;
    }
  }
if (contains_swearing(newname)) {
  write_user(user,"You cannot use a name that contains swearing.\n");
  return;
  }
/* See if user is on atm */
if ((u=get_user(oldname))!=NULL) on=1;
else {
  /* User not logged on */
  if ((u=create_user())==NULL) {
    vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
    write_syslog(ERRLOG,1,"Unable to create temporary user object in change_user_name().\n");
    return;
    }
  strcpy(u->name,oldname);
  if (!load_user_details(u)) {
    write_user(user,nosuchuser);  
    destruct_user(u);
    destructed=0;
    return;
    }
  on=0;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot change the name of a user with the same of higher level to you.\n");
  if (!on) {
    destruct_user(u);
    destructed=0;
    }
  return;
  }
/* everything is ok, so go ahead with change */
strcpy(u->name,newname);
strcpy(u->recap,u->name);
strcpy(u->bw_recap,u->name);
/* if user was a WIZ+ */
if (u->level>=WIZ) {
  if (in_retire_list(oldname)) {
    clean_retire_list(oldname);
    add_retire_list(u->name);
    }
  for (wiz=first_wiz_entry;wiz!=NULL;wiz=wiz->next) {
    if (!strcmp(wiz->name,oldname)) {
      strcpy(wiz->name,u->name);
      break;
      }
    }
  }
/* online user list */
for(usr=user_first;usr!=NULL;usr=usr->next) {
  if (!strcmp(usr->name,oldname)) {
    strcpy(usr->name,u->name);
    break;
    }
  }
/* all users list */
for(uds=first_dir_entry;uds!=NULL;uds=uds->next) {
  if (!strcmp(uds->name,oldname)) {
    strcpy(uds->name,u->name);
    break;
    }
  }
/* all memory occurences should be done.  now do files */
sprintf(oldfile,"%s/%s.D", USERFILES,oldname);
sprintf(newfile,"%s/%s.D", USERFILES,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.M", USERMAILS,oldname);
sprintf(newfile,"%s/%s.M", USERMAILS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.P", USERPROFILES,oldname);
sprintf(newfile,"%s/%s.P", USERPROFILES,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.H", USERHISTORYS,oldname);
sprintf(newfile,"%s/%s.H", USERHISTORYS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.C", USERCOMMANDS,oldname);
sprintf(newfile,"%s/%s.C", USERCOMMANDS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.MAC", USERMACROS,oldname);
sprintf(newfile,"%s/%s.MAC", USERMACROS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.F", USERFRIENDS,oldname);
sprintf(newfile,"%s/%s.F", USERFRIENDS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.R", USERROOMS,oldname);
sprintf(newfile,"%s/%s.R", USERROOMS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.B", USERROOMS,oldname);
sprintf(newfile,"%s/%s.B", USERROOMS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.K", USERROOMS,oldname);
sprintf(newfile,"%s/%s.K", USERROOMS,u->name);
rename(oldfile,newfile);
sprintf(oldfile,"%s/%s.REM", USERREMINDERS,oldname);
sprintf(newfile,"%s/%s.REM", USERREMINDERS,u->name);
rename(oldfile,newfile);
/* give results of name change */
sprintf(text,"Had name changed from ~OL%s~RS by %s~RS.\n",oldname,user->recap);
add_history(u->name,1,text);
write_syslog(SYSLOG,1,"%s CHANGED NAME of %s to %s.\n",user->name,oldname,u->name);
if (user->vis) name=user->bw_recap; else name=invisname;
if (on) {
  vwrite_user(u,"\n~FR~OL%s has changed your name to '~RS~OL%s~FR'.\n\n",name,u->name);
  write_room_except(u->room,"~OL~FMThere is a shimmering in the air and something pops into existence...\n",u);
  }
else {
  u->socket=-2;
  strcpy(u->site,u->last_site);
  save_user_details(u,0);
  sprintf(text,"~FR~OLYou have had your name changed from ~FY%s~FR to ~FY%s~FR.\nDon't forget to reset your recap if you want.\n",oldname,u->name);
  send_mail(user,u->name,text,0);
  destruct_user(u);
  destructed=0;
  }
vwrite_user(user,"You have changed the name of ~OL%s~RS to ~OL%s~RS.\n\n",oldname,u->name);
}



/*****************************************************************************/
/* Doplnene funkcie                                                          */
/*****************************************************************************/

/*** Auth Checks - original: CryptV5 ***/
void auth_user(UR_OBJECT user)
{
	UR_OBJECT u;
	int w, buflen, rremote, rlocal, auth_sock, wait_time, all=0;
	char auth_string[81];
	char realbuf[200];
	char *buf;
	struct sockaddr_in sa;
	char ch;
	struct timeval timeout;
	fd_set auth_readmask;

	set_crash();
	if (word_count<2) {
		write_usage(user,"auth <user> [<cas>] [all]\n");
		return;
		}
	if (!(u=get_user(word[1]))) {
		write_user(user,notloggedon);
		return;
		}

	wait_time=atoi(word[2]);
	if (wait_time>9 || wait_time<0) {
		write_user(user,"~OLAuth:~RS cas moze byt maximalne 9 sekund\n");
		return;
		}

	if (wait_time==0) wait_time=1;
	if (!strcmp(word[3],"all")) all=1;

	if ((auth_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		write_user(user,"~OLAuth:~RS Cannot create socket for auth check\n");
		return;
		}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(113); /* Auth port */
	sa.sin_addr.s_addr = u->auth_addr; /* Address to connect to */

	if (connect(auth_sock, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
		close(auth_sock);
		write_user(user,"~OLAuth:~RS Cannot connect to site for auth check\n");
		return; /* Connect failure */
		}

	buf = realbuf;
	sprintf(buf, "%d , %d\r\n", u->site_port, u->port);

	buflen = strlen(buf);
	while ((w = write(auth_sock, buf, buflen)) < buflen)
		if (w == -1) {
			close(auth_sock);
			write_user(user,"~OLAuth: ~RSCannot write to auth port\n");
			return;
			}
		else {
			buf += w;
			buflen -= w;
			}

	sprintf(text,"~OLAuth:~RS Connected - timeout set at %d seconds.\n",wait_time);
	write_user(user,text);

	/* Sleep 5000us + wait_time to give server a chance to respond */
	FD_ZERO(&auth_readmask);
	FD_SET(auth_sock,&auth_readmask);
	timeout.tv_sec=wait_time;
	timeout.tv_usec=5000;
	select(FD_SETSIZE,&auth_readmask,0,0,&timeout);

	/* Read from server */
	buf = realbuf;
	while ((w = read(auth_sock, &ch, 1)) == 1) {
		*buf = ch;
		if ((ch != ' ') && (ch != '\t') && (ch != '\r')) ++buf;
		if ((buf - realbuf == sizeof(realbuf) - 1) || (ch == '\n')) break;
		}

	if (w == -1) {
		write_user(user,"~OLAuth:~RS Cannot read from auth port\n");
		close(auth_sock);
		return;
		}
	*buf = '\0';

	sscanf(realbuf, "%d,%d: USERID :%*[^:]:%s", &rremote, &rlocal, auth_string);
	close(auth_sock);

	if ((rremote != u->site_port) || (rlocal != u->port)) {
		write_user(user,"~OLAuth:~RS Incorrect ports returned from remote machine\n~OLAuth Diagnostic: ~RS");
		sprintf(text,"%s",realbuf);
		write_user(user,text);
		if (!all) return;
		}

	if (all) sprintf(text,"~OLAuth:~RS %s = %s",u->name,realbuf);
	else sprintf(text,"~OLAuth:~RS %s = %s\n",u->name,auth_string);
	write_user(user,text);
	return;
}


void finger_host(UR_OBJECT user)
{
	char fname[500];

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user@hostname>", command_table[FINGER].name);
		return;
		}
	
	switch (double_fork()) {
		case -1 : write_user(user,"Lotos: finger: fork failure\n");
			write_syslog(ERRLOG, 1, "finger: fork failure\n");
			return;
		case  0 :
			write_syslog(SYSLOG, 1, "%s fingered \"%s\"\n", user->name, word[1]);
			sprintf(fname,"%s/output.%s", TEMPFILES, user->name);	    
			unlink(fname);
			sprintf(text,"echo '\n~OL~FGFinger informacie o~FT: ~FB\"~FT%s~FB\"\n' > %s",word[1], fname);
			system(text);
			sprintf(text,"finger %s >> %s",word[1],fname);
			system(text);
			sprintf(text, "echo '\n\t\t~OLkoniec~RS' >> %s\n", fname);
			system(text);
			switch (more(user,user->socket,fname)) {
				case 0: write_user(user,"~OL~FRfinger_host(): Could Not Open Temporary Finger File...\n");
				case 1: user->misc_op=2;
				}
			_exit(1);
			break;
		}
}


void reloads(UR_OBJECT user)
{
	int i;
	char *rldstr[]={"all",  "rm",   "txt",  "pic",  "ur",
			"fnt",  "kill"
			};

	set_crash();
	if (user==NULL) {
		list_txt_files(user);
		list_pic_files(user);
		list_fnt_files(user);
		list_kill_msgs(user);
		return;
		}

	if (word_count<2 && user!=NULL) {
		write_user(user, "\n");
		write_usage(user, "%s <polozka> [<parametre>]", command_table[RLD].name);
		write_user(user, "\nMozne polozky:\n\t");
		for (i=0; i<SIZEOF(rldstr); i++) {
			vwrite_user(user, "%-9.9s ", rldstr[i]);
			if (((i+1)%5)==0 && i!=0) write_user(user, "\n\t");
			}
		write_user(user, "\n\n");
		return;
		}

	for (i=0; i<SIZEOF(rldstr); i++) {
		if (!strcmp(rldstr[i], word[1])) {
			switch (i) {
				case 0:
					write_user(user, "Obnovujem vsetky zoznamy ...\n");
					write_room_except(NULL, "~FR~OL~LISYSTEM: ~FWObnovujem vsetky zoznamy, moze to chvilu lagovat !!!\n\n", user);
					write_syslog(SYSLOG, 1, "%s obnovuje vsetky zoznamy\n", user->name);
					reload_room_description(user, 1);
					list_txt_files(user);
					list_pic_files(user);
					recount_users(user, 1);
					list_fnt_files(user);
					list_kill_msgs(user);
					write_user(user, "Zoznamy obnovene\n");
					break;
				case 1:
					reload_room_description(user, 1);
					break;
				case 2:
					list_txt_files(user);
					break;
				case 3:
					list_pic_files(user);
					break;
				case 4:
					recount_users(user, 0);
					break;
				case 5:
					list_fnt_files(user);
					break;
				case 6:
					list_kill_msgs(user);
					break;
				}
			return;
			} /* end if */
		} /* end for */
	write_user(user, "Chybny parameter\n");
}


/*** Searing file operations ***/
void swear_com(UR_OBJECT user)
{
	int i, sw_save;
	char fname[500], line[WORD_LEN+1];
	FILE *fp, *fpout;

	set_crash();
	if(word_count<2) {
		write_usage(user,"%s list/add/erase/rem/reload", command_table[SWEARS].name);
		return;
		}
	/* list swear words */
	if(word[1][0]=='?' || !strcmp(word[1],"list")) {
		i=0;
		while(swear_words[i][0]!='*') {
			if(!i) write_user(user,"\n~BB--->>> Swearing ban list <<<---\n\n");
			vwrite_user(user, "\t'%s~RS'\n",swear_words[i++]);
			}
		if(!i) write_user(user, ">>>No swear words found.\n");
		else vwrite_user(user, ">>>Total of ~FT%d~RS swear words found.\n",i);
		return;
		}        
	/* remove swear file */
	if(!strcmp(word[1],"rem")) {
		unlink(SWEARFILE);
		sprintf(text,"~OLSYSTEM:~RS %s zaps swear file.\n",user->name);
		write_syslog(SYSLOG, 1, text);
		write_level(GOD, 1, text, user);
		write_user(user,">>>Swear file deleted.\n");
		return;
		}
	/* add a word to swear file */
	if(!strcmp(word[1],"add")) {
		if(word_count<3) write_usage(user,"%s add <word>", command_table[SWEARS].name);
		else {
			/* change '_' in space */
			for(i=0; word[2][i]!='\0'; i++) {
				if(word[2][i]=='_' && word[2][i+1]!='_') word[2][i]=' ';
				else replace_string(word[2], "__", "_");
				}
			i=0;
			sw_save=amsys->ban_swearing;
			amsys->ban_swearing=0;
			while(swear_words[i][0]!='*') {
				if(!strcmp(swear_words[i],word[2])) {
					write_user(user,">>>This word already exists!\n");
					amsys->ban_swearing=sw_save;
					return;
					}
				i++;
				}
			if(i<MAX_SWEARS) {
				strcpy(swear_words[i],word[2]);
				strcpy(swear_words[i+1],"*");
				}
			else {
				write_user(user,">>>Sorry, swear word list is full!\n");
				amsys->ban_swearing=sw_save;
				return;
				} 
			if(!(fp=fopen(SWEARFILE,"a"))) {
				vwrite_user(user, ">>>%s: Unable to append file.\n",syserror);
				write_syslog(ERRLOG, 1, "Unable to append swearfile in swears().\n");
				amsys->ban_swearing=sw_save;
				return;
				}
			fprintf(fp,"%s\n",word[2]);
			fclose(fp);
			amsys->ban_swearing=sw_save;
			write_user(user,">>>Swear word added.\n");
			sprintf(text,"~OLSYSTEM:~RS %s added '%s' to swear file.\n",user->name,word[2]);
			write_syslog(SYSLOG, 1, text);
			write_level(GOD, 1, text, user);
			}
		return;
		}
	/* reload swear file */
	if(!strcmp(word[1],"reload")) {
		sw_save=amsys->ban_swearing;
		load_swear_file(user);
		amsys->ban_swearing=sw_save;
		sprintf(text,"~OLSYSTEM:~RS %s reload swear file.\n",user->name);
		write_syslog(SYSLOG, 1, text);
		write_level(GOD, 1, text, user);
		return;
		}  
	/* erase a word from swear file */
	if(!strcmp(word[1],"erase")) {
		if(word_count<3) {
			write_usage(user,"%s erase <word>", command_table[SWEARS].name);
			return;
			}
		if(!(fp=fopen(SWEARFILE,"r"))) {
			write_user(user,">>>Swear file not found.\n");
			return;
			}
		sprintf(fname, "%s/swears.tmp", TEMPFILES);
		if(!(fpout=fopen(fname, "w"))) {
			vwrite_user(user, "%s: Couldn't open tempfile.\n", syserror);
			write_syslog(ERRLOG, 1, "Couldn't open tempfile to write in swear_com().\n");
			fclose(fp);
			return;
			}
		i=0;
		fscanf(fp,"%s",line);
		while(!feof(fp)) {
			if(!strstr(word[2],line)) fprintf(fpout,"%s\n",line);
			else i=1;
			fscanf(fp,"%s",line);
			}
		fclose(fp);
		fclose(fpout);
		if(!i) {
			write_user(user,">>>Swear word not found.\n");
			unlink("tempfile");
			return;
			}
		rename(fname, SWEARFILE);
		write_user(user,">>>Swear word erased.\n>>>You must reload swear file to update internal list...\n");
		sprintf(text,"~OLSYSTEM:~RS %s erased '%s' from swear file.\n",user->name,word[2]);
		write_syslog(SYSLOG, 1, text);
		write_level(GOD, 1, text, user);
		return;
		}                            
	write_user(user,">>>Unknown option, yet...\n");
}


/*** modify user information ***/
void modify(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char *s, *tmp, *usage;
	int mins, hours, days, no_exists=0, level;

	set_crash();
	if (word_count<4) {
		write_usage(user,"%s <user> desc/inphr/outphr/time/level/gnd/credit <text>", command_table[MODIFY].name);
		return;
		}
	if ((u=get_user(word[1]))==NULL) {
		/* User not logged on */
		if ((u=create_user())==NULL) {
			vwrite_user(user, ">>>%s: unable to create temporary user object.\n", syserror);
			write_syslog(ERRLOG, 1, "Unable to create temporary user object in modify().\n");
			return;
			}
		strncpy(u->name, word[1], USER_NAME_LEN);
		u->name[USER_NAME_LEN]='\0';
		if (!load_user_details(u)) {
			write_user(user,nosuchuser);
			destruct_user(u);
			destructed=0;
			return;
			}
		no_exists=1;
		}
	if (u->level>=user->level && user->level<ROOT) {
		vwrite_user(user, ">>> Nemozes editovat t%s user%s !\n", grm_gnd(10, u->gender), grm_gnd(9, u->gender));
		if (no_exists) {
			destruct_user(u);
			destructed=0;
			}
		return;
		}
	s=remove_first(remove_first(remove_first(inpstr)));

	if (!strncmp(word[2],"desc",strlen(word[2]))) {
		if (strlen(s)>USER_DESC_LEN) s[USER_DESC_LEN]='\0';
		strcpy(u->desc,s);
		vwrite_user(user,">>> Menis %s desc ...\n", u->named);
		if (no_exists) {
			save_user_details(u, 0);
			destruct_user(u);
			destructed=0;
			}
		return;
		}

	if (!strncmp(word[2],"inphr",strlen(word[2]))) {
		if (strlen(s)>PHRASE_LEN) s[PHRASE_LEN]='\0';
		strcpy(u->in_phrase,s);
		vwrite_user(user, ">>> Menis %s vstupnu frazu ...\n", u->named);
		if (no_exists) {
			save_user_details(u,0);
			destruct_user(u);
			destructed=0;
			}
		return;
		}

	if (!strncmp(word[2],"outphr",strlen(word[2]))) {
		if (strlen(s)>PHRASE_LEN) s[PHRASE_LEN]='\0';
		strcpy(u->out_phrase,s);
		vwrite_user(user, ">>> Menis %s vystupnu frazu ...\n",u->named);
		if (no_exists) {
			save_user_details(u,0);
			destruct_user(u);
			destructed=0;
			}
		return;
		}

	if (!strncmp(word[2],"time",strlen(word[2]))) {
		if (word_count<6) {
			write_usage(user, "%s <user> time <days> <hours> <mins>", command_table[MODIFY].name);
			if (no_exists) {
				destruct_user(u);
				destructed=0;
				}
			return;
			}
		if (!is_number(word[3])
		    || !is_number(word[4])
		    || !is_number(word[5])
		    ) {
			write_user(user, "Chybny/e parameter/re\n");
			if (no_exists) {
				destruct_user(u);
				destructed=0;
				}
			return;
			}
		days=atoi(word[3]);
		hours=atoi(word[4]);
		mins=atoi(word[5]);
		u->total_login=days*86400+hours*3600+mins*60;
		vwrite_user(user,">>> Menis %s total login time ...\n", u->named);
		sprintf(text, "~OLSYSTEM:~RS %s changed %s's login time: %d d, %d h, %d min.\n",
			user->name, u->name, days, hours, mins);
		write_syslog(SYSLOG, 1, text);
		write_level(ARCH, 1, text, user);
		if (no_exists) {
			save_user_details(u,0);
			destruct_user(u);
			destructed=0;
			}
		return;
		}
	if (!strncmp(word[2],"level",strlen(word[2]))) {
		tmp=strdup(word[3]);
		strtoupper(tmp);
		if ((level=get_level(tmp))==-1) {
			free(tmp);
			write_user(user, "chybny nazov levelu !\n");
			if (no_exists) {
				destruct_user(u);
				destructed=0;
				}
			return;
			}
		free(tmp);
		u->real_level=u->level=level;
		vwrite_user(user, ">>> Menis %s level ...\n",u->named);
		sprintf(text, "~OLSYSTEM:~RS %s changed %s's level: %s.\n",
			user->name, u->name, user_level[u->level].name);
		write_syslog(SYSLOG, 1, text);
		write_level(ARCH, 1, text, user);
		if (no_exists) {
			save_user_details(u,0);
			destruct_user(u);
			destructed=0;
			}
		return;
		}
	if (!strncmp(word[2],"gnd",strlen(word[2]))) {
		switch (toupper(word[3][0])) {
			case 'N':
				u->gender=0;
				break;
			case 'M':
				u->gender=1;
				break;
			case 'Z':
			case 'F':
				u->gender=2;
				break;
			default :
				write_user(user, "Nezname pohlavie !\n");
				if (no_exists) {
					destruct_user(u);
					destructed=0;
					}
				return;
			}
		vwrite_user(user, ">>> Menis %s gender ...\n", u->named);
		sprintf(text, "~OLSYSTEM:~RS %s changed %s's gender: %s.\n",
			user->name, u->name, sex[u->gender]);
		write_syslog(SYSLOG, 1, text);
		write_level(ARCH, 1, text, user);
		nick_grm(u);
		if (no_exists) {
			save_user_details(u,0);
			destruct_user(u);
			destructed=0;
			}
		return;
		}
	if (!strncmp(word[2],"credit",strlen(word[2]))) {
		usage=strdup("modify <user> credit b/m <#credit>");
		if (word_count<4) {
			write_usage(user, usage);
			free(usage);
			return;
			}
		if (!strcmp(word[3], "b")) {
			if (!is_number(word[4])) {
				write_usage(user, usage);
				free(usage);
				return;
				}
			u->bank=atoi(word[4]);
			vwrite_user(user, ">>> Menis %s bankovy kredit na %d ...\n", u->named, u->bank);
		sprintf(text, "~OLSYSTEM:~RS %s changed %s's bank credit: %d.\n",
			user->name, u->name, u->bank);
			}
		if (!strcmp(word[3], "m")) {
			if (!is_number(word[4])) {
				write_usage(user, usage);
				free(usage);
				return;
				}
			u->money=atoi(word[4]);
			vwrite_user(user, ">>> Menis %s aktualny kredit na %d ...\n", u->named, u->money);
		sprintf(text, "~OLSYSTEM:~RS %s changed %s's credit: %d.\n",
			user->name, u->name, u->money);
			}
		write_syslog(SYSLOG, 1, text);
		write_level(ARCH, 1, text, user);
		if (no_exists) {
			save_user_details(u,0);
			destruct_user(u);
			destructed=0;
			}
		free(usage);
		return;
		}

	write_user(user,">>> Hmmm, asik xybny parameter ...\n");
	if (no_exists) {
		destruct_user(u);
		destructed=0;
		}
}


/*** set/reset restrictions ***/
void restrict(UR_OBJECT user)
{
	UR_OBJECT u;
	int i,no_exists=0;
	char *restrict_name[]={"go","move","promote","demote","muzzle",
				"unmuzzle","kill","help","suicide","who",
				"run","clone","review","execmds"
				};

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user> [<restrict_string>]", command_table[RESTRICT].name);
		write_usage(user,"%s <user> +/- <restriction>", command_table[RESTRICT].name);
		return;
		}
	if ((u=get_user(word[1]))==NULL) {
		/* User not logged on */
		if ((u=create_user())==NULL) {
			vwrite_user(user, ">>>%s: unable to create temporary user object.\n",syserror);
			write_syslog(ERRLOG, 1,"Unable to create temporary user object in restrict().\n");
			return;
			}
		strcpy(u->name,word[1]);
		if (!load_user_details(u)) {
			write_user(user,nosuchuser);
			destruct_user(u);
			destructed=0;
			return;
			}
		no_exists=1;
		}

	/* list user's restrictions */
	if(word_count==2) {
		vwrite_user(user, "\n~BB--->>> Obmedzenia pre %s <<<---\n\n", u->namea);
		for (i=0;i<MAX_RESTRICT;i++)
			vwrite_user(user,">>>%-8s (%c) restriction... %s\n",
				restrict_name[i],restrict_string[i],
				u->restrict[i]==restrict_string[i]?"~FGset":"~FRreset");
		if (no_exists) {
			destruct_user(u);
			destructed=0;
			}
		return;
		}

	if (u->level>=user->level && user->level<ROOT) {
		write_user(user,">>> Nemozes pouzit tento prikaz pre tohoto usera !\n");
		if (no_exists) {
			destruct_user(u);
			destructed=0;
			}
		return;
		}

	/* set only a single restriction */
	if (word_count>=4) {
		if (word[2][0]!='+' && word[2][0]!='-') {
			write_usage(user,"%s <user> +/- <restriction>", command_table[RESTRICT].name);
			if (no_exists) {
				destruct_user(u);
				destructed=0;
				}
			return;
			}
		strtoupper(word[3]);
		for (i=0;i<MAX_RESTRICT;i++) {
			if (restrict_string[i]==word[3][0]) {
				u->restrict[i]=(word[2][0]=='+')?restrict_string[i]:'.';
				vwrite_user(user,">>>%s's %s (%c) obmedzenie nastavene na %s.\n",
					u->name,restrict_name[i],restrict_string[i],
					offon[word[2][0]=='+']);
				write_syslog(SYSLOG, 1, "\n~OLSYSTEM:~RS %s turn %s restriction %s (%c) for %s.\n",
					user->name,offon[word[2][0]=='+'],restrict_name[i],
					restrict_string[i],u->name);
				write_level(ARCH, 1, text, user);
				if (no_exists) {
					save_user_details(u,0);
					destruct_user(u);
					destructed=0;
					}
				return;
				}
			}
		write_user(user,">>> Hmm, asik nezname obmedzenie ...\n");
		return;
		}
	/* global settings */
	if (strlen(word[2])!=MAX_RESTRICT) {
		vwrite_user(user, ">>> Obmedzovaci retazec musi mat presne ~FT%d~RS znakov !\n",MAX_RESTRICT);
		if (no_exists) {
			destruct_user(u);
			destructed=0;
			}
		return;
		}
	for (i=0;i<MAX_RESTRICT;i++)
		if (word[2][i]==restrict_string[i]) {
			u->restrict[i]=restrict_string[i];
			vwrite_user(user, ">>>Restriction %-8s (%c) ~FGset~RS for %s...\n",
				restrict_name[i],restrict_string[i],u->name);
			}
		else {
			if(word[2][i]=='?') continue;
			else {
				u->restrict[i]='.';
				vwrite_user(user, ">>>Restriction %-8s (%c) ~FRreset~RS for %s...\n",
					restrict_name[i],restrict_string[i],u->name);
				}
			}
	sprintf(text,"\n~OLSYSTEM:~RS Restrictions for user %s changed by %s: %s\n",
		u->name,user->name,u->restrict);
	write_syslog(SYSLOG, 1,text);
	write_level(ARCH, 1, text, user);
	if (no_exists) {
		save_user_details(u,0);
		destruct_user(u);
		destructed=0;
		}
}


/*** set system access to allow or disallow further logins ***/
void system_access(UR_OBJECT user, char *inpstr, int co)
{
	char line[132];

	set_crash();
	if (word_count<2) {
		vwrite_user(user,"Main port je     %s\n",opcl[syspp->sys_access]);
		vwrite_user(user,"Wiz port je      %s\n",opcl[syspp->wiz_access]);
		return;
		}

	strtolower(inpstr);

	if (!co) {
		if(!strcmp(word[1],"all")) {
			sprintf(line,"ALL PORTS CLOSED BY %s\n",user->name);
			write_level(ARCH, 1, line, user);
			write_syslog(SYSLOG, 1, "SYSTEM: ALL PORTS CLOSED BY %s\n", user->name);
			write_user(NULL, "\07~FR~OL~LI*** Talker je odteraz uzavrety pre vsetky prihlasenia ***~RS\n");
			syspp->sys_access=0;
			syspp->wiz_access=0;
			return;
			}
		if(!strcmp(word[1], "main")) {
			sprintf(line,"MAIN PORT CLOSED BY %s\n",user->name);
			write_level(ARCH, 1, line, user);
			write_syslog(SYSLOG, 1,"SYSTEM: MAIN PORT CLOSED BY %s\n", user->name);
			syspp->sys_access=0;
			return;
			}
		if(!strcmp(word[1], "wiz")) {
			sprintf(line,"WIZ PORT CLOSED BY %s\n",user->name);
			write_level(ARCH, 1, line, user);
			write_syslog(SYSLOG, 1, "SYSTEM: WIZ PORT CLOSED BY %s\n", user->name);
			syspp->wiz_access=0;
			return;
			}
		else {
			write_user(user,"Neznamy parameter.\n");
			return;
			}
		}  /* end of co if */
	else {	
		if(!strcmp(word[1], "all")) {
			sprintf(line,"ALL PORTS OPENED BY %s\n", user->name);
			write_level(ARCH, 1, line, user);
			write_syslog(SYSLOG, 1, "SYSTEM: ALL PORTS OPENED BY %s\n", user->name);
			write_user(NULL, "\07~FR~OL*** Talker je teraz otvoreny pre vsetky nahlasenia ***\n");
			syspp->sys_access=1;
			syspp->wiz_access=1;
			return;
			}
		if(!strcmp(word[1], "main")) {
			sprintf(line,"MAIN PORT OPENED BY %s\n", user->name);
			write_level(ARCH, 1, line, user);
			write_syslog(SYSLOG, 1, "SYSTEM: MAIN PORT OPENED BY %s\n", user->name);
			syspp->sys_access=1;
			return;
			}
		if(!strcmp(word[1], "wiz")) {
			sprintf(line,"WIZ PORT OPENED BY %s\n", user->name);
			write_level(ARCH, 1, line, user);
			write_syslog(SYSLOG, 1, "SYSTEM: WIZ PORT OPENED BY %s\n", user->name);
			syspp->wiz_access=1;
			return;
			}
		else {
			write_user(user,"Neznamy parameter.\n");
			return;
			}
		}
}


void force_backup(UR_OBJECT user)
{
	set_crash();
	if (!backup_talker()) {
		write_user(user,"~FR@> ~FTUnable To Start Backup Process,\n~FR@> ~FTBackup_talker() returned an error...\n");
		return;
		}
	else {
		write_room(NULL,"\n~OL~FGSYSTEM: ~FMA Backup Process To Backup all talker files is being\n");
		write_room(NULL,"        ~OL~FMstarted.  This ~FR*may*~FM lag the talker a little...\n\n");
		write_user(user,"     ~FTStarting Backup Of Talker Files...\n");
		vwrite_user(user,"     ~FTBacking Up Talker Files To: ~FM%s/%s.zip\n",BACKUPDIR,BACKUPFILE);
		vwrite_user(user,"     ~FTFor progress info, Read File: ~FM%s/%s.log\n\n",LOGFILES,BACKUPFILE);
		}
}


void restart_com(UR_OBJECT user)
{
	set_crash();
	if (amsys->rs_countdown) {
		vwrite_user(user, "~OL%s ~FWodpocitavanie je momentalne aktivne, najprv ho vypni !\n", amsys->rs_which?"~FYreboot":"~FRshutdown");
		return;
		}
	audioprompt(user, 4, 0);
	write_user(user, "\n\007~FM~OL~LI*** Pozor, toto restartne talker ! ***\nNaozaj to chces ? (y/n)\n");
	user->misc_op=101;
	no_prompt=1;
}


/*** Get identification info about an user 
     (mainly written by Mituc <BlackBlue@DeathsDoor.Com>) ***/  
void identify(UR_OBJECT user)
{
	UR_OBJECT u;
	char accname[ID_BUFFLEN+1],email[2*ID_BUFFLEN+1],username[USER_NAME_LEN+1];
	struct hostent *rhost;
	int remoteport,localport,line,found=0,retval;

	set_crash();
	if (word_count<2) {
		write_usage(user, "identify <user>|<line>");
		return;
		}
	if (!is_number(word[1])) { /* user name */
		if((u=get_user(word[1]))==NULL) {
			write_user(user, nosuchuser);
			return;
			}
		if (u->type==REMOTE_TYPE) {
			write_user(user,">>>You cannot use this command to identify remote users.\n");
			return;
			}
		}
	else { /* socket line */
		line=atoi(word[1]);
		for (u=user_first; u!=NULL; u=u->next)
			if(u->type!=CLONE_TYPE && u->socket==line) {
				found=1;
				break;
				}
		if (!found) {
			write_user(user,">>>That line is not currently active.\n");
			return;
			}
		}
	remoteport=u->site_port;
	localport=u->port;
	if (found || strlen(u->name)<USER_MIN_LEN) strcpy(username,"Login user");
	else strcpy(username,u->name);
	if ((rhost=fgethostbyname(u->site))==NULL) {
		if (use_hostsfile) {
			write_syslog(ERRLOG, 1, "Warning: Host %s not found by fgethostbyname() for %s.\n", u->site, username);
			write_user(user,">>>Warning! Cannot found user site in hostsfile...\n");
			}
		if ((rhost=gethostbyname(u->site))==NULL) { /* try directly */
			write_syslog(ERRLOG, 1, ">>>Host %s not found by gethostbyname() for %s in identify().\n",u->site,username);
			vwrite_user(user, "%s: %s not found.\n",syserror,u->site);
			return;
			}
		}
	retval=ident_request(rhost,remoteport,localport,accname);
	switch (retval) {
		case ID_OK: write_syslog(SYSLOG, 1, "%s gets account name of %s: %s.\n",user->name,username,accname); break;
		case ID_CONNERR:
	      	write_syslog(ERRLOG, 1, "LOTOS: Cannot open local connection in ident_request().\n");
			sprintf(text,">>>Cannot open local connection error occured while getting %s's account name.\n",username);
			break;
		case ID_NOFOUND:
			write_syslog(ERRLOG, 1,"LOTOS: Ident not running error on %s while getting %s's account name.\n",u->site,username);
			sprintf(text,">>>Ident daemon is not running on %s.\n",u->site);
			break;
		case ID_CLOSED:
			write_syslog(ERRLOG, 1, "LOTOS: Closed connection error to %s in ident_request().\n",u->site);
			sprintf(text,">>>Closed connection error occured while getting %s's account name.\n",username);
			break;
		case ID_READERR:
			write_syslog(ERRLOG, 1, "LOTOS: Read error from %s in ident_request() while getting %s's account name.\n",u->site,username);
			sprintf(text,">>>A read error occured while getting %s's account name.\n",username);
			break;
		case ID_WRITEERR:
			write_syslog(ERRLOG, 1, "LOTOS: Write error to %s in ident_request() while getting %s's account name.\n",u->site,username);
			sprintf(text,">>>A write error occured while getting %s's account name.\n",username);
			break;
		case ID_NOMEM:
			write_syslog(ERRLOG, 1, "LOTOS: No memory error in ident_request().\n");
			sprintf(text,">>>Not enough memory error occured while getting %s's account name.\n",username);
			break;
		case ID_NOUSER:
			write_syslog(ERRLOG, 1, "LOTOS: NO-USER error occured while getting %s's account name. This means that %s's ident daemon cannot (doesn't want to ) associate connected sockets with users or the port pair (%d,%d) is invalid.\n",username,u->site,remoteport,localport);
			sprintf(text,">>>NO-USER error occured while getting %s's account name.\n",username);
			break;
		case ID_INVPORT:
			write_syslog(ERRLOG, 1, "LOTOS: INVALID-PORT error occured while getting %s's account name. This means that the port pair (%d,%d) wasn't received properly by %s\'s ident daemon or it's a value out of range.\n",username,remoteport,localport,u->site);
			sprintf(text,">>>INVALID-PORT error occured while getting %s's account name.\n",username);
			break;
		case ID_UNKNOWNERR:
			write_syslog(ERRLOG, 1, "LOTOS: UNKNOWN-ERROR error for %s's ident daemon on request (%d,%d).\n",u->site,remoteport,localport);
			sprintf(text,">>>UNKNOWN-ERROR error occured while getting %s's account name.\n",username);
			break;
		case ID_HIDDENUSER:
			write_syslog(ERRLOG, 1, "LOTOS: HIDDEN-USER error for %s's ident daemon on request (%d,%d). This means that %'s administrator allows users to hidde themselfs on ident requests.\n",u->site,remoteport,localport,u->site);
			sprintf(text,">>>HIDDEN-USER error occured while getting %s's account name.\n",username);
			break;
		case ID_CRAP:
			write_syslog(ERRLOG, 1, "LOTOS: %s returned crap while receiving data for request (%d,%d).\n",u->site,remoteport,localport);
			sprintf(text,">>>An undefined type error occured while getting %s's account name.\n",username);
			break;
		case ID_TIMEOUT:
			write_syslog(ERRLOG, 1, "LOTOS: Read timed out from %s while getting %s's account name.\n",u->site,username);
			sprintf(text,">>>Read timed out from %s's site.\n",username);
			break;
		}

	if (retval!=ID_OK) { /* it's unknown, so exit */
		write_user(user,text);
		return;
		}

	switch (mail_id_request(rhost,accname,email)) {
		case ID_OK:
			write_syslog(SYSLOG, 1, "%s gets e-mail address of %s: %s.\n",username,username,email);
			sprintf(text,">>>%s's e-mail address is: ~OL%s\n",username,email);
			break;
		case ID_CONNERR:
			write_syslog(ERRLOG, 1, "LOTOS: Cannot open local connection in mail_id_request().\n");
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_NOFOUND:
			write_syslog(ERRLOG, 1, "LOTOS: Mail daemon not running error on %s while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_CLOSED:
			write_syslog(ERRLOG, 1, "LOTOS: Connection closed error by %s in mail_id_request() while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_READERR:
			write_syslog(ERRLOG, 1, "LOTOS: Read error from %s in mail_id_request() while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_WRITEERR:
			write_syslog(ERRLOG, 1, "LOTOS: Write error to %s in mail_id_request() while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_NOMEM:
			write_syslog(ERRLOG, 1, "LOTOS: No memory error in mail_id_request().\n");
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_COMERR:
			write_syslog(ERRLOG, 1, "LOTOS: Received invalid command error from %s's mail daemon while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_UNKNOWN:
			write_syslog(ERRLOG, 1, "LOTOS: Could not determine an exact e-mail address for %s. This means that %s is a mail alias to an account from another server or %s is running ip_masquerade.\n",username,accname,u->site);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_CRAP:
			write_syslog(ERRLOG, 1, "LOTOS: %s returned crap while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		case ID_TIMEOUT:
			write_syslog(ERRLOG, 1, "LOTOS: Read timed out from %s while getting %s's e-mail address.\n",u->site,username);
			sprintf(text,">>>E-mail address: ~OL<%s@%s>\n",accname,u->site);
			break;
		}
	write_user(user, text);
}



#endif /* ct_admin.c */
