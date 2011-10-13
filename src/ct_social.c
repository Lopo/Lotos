/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
            Funkcie Lotos v1.2.0 na rozne medzipouzivatelske akcie
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_SOCIAL_C__
#define __CT_SOCIAL_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "ct_social.h"
#include "comvals.h"


/*** Say user speech. ***/
void say(UR_OBJECT user, char *inpstr)
{
	char type[15], *name;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kecat");  return;
		}
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
                break;
    case SBMAX: write_user(user,noswearing);
                return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
#ifdef NETLINKS
if (user->room==NULL) {
  sprintf(text,"ACT %s say %s\n",user->name,inpstr);
  write_sock(user->netlink->socket,text);
  no_prompt=1;
  return;
  }
#endif
if (word_count<2 && user->command_mode) {
  write_user(user,"Say what?\n");  return;
  }
smiley_type(inpstr,type);
if (user->type==CLONE_TYPE) {
  vwrite_room(user->room,"Clone of %s~RS %ss: %s\n",user->recap,type,inpstr);
  record(user->room,text);
  return;
  }
vwrite_user(user,"~FGYou %s:~RS %s\n",type,inpstr);
if (user->vis) name=user->bw_recap; else name=invisname;
if (!user->vis) write_monitor(user,user->room,0);
sprintf(text, say_style, name,type,inpstr);
write_room_except(user->room,text,user);
record(user->room,text);
plugin_triggers(user, inpstr);
}


/*** Shout something ***/
void shout(UR_OBJECT user, char *inpstr)
{
	char *name;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kricat");
		return;
		}
	if (word_count<2) {
		write_user(user,"Kricat co ?\n");
		return;
		}
	if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
		switch(amsys->ban_swearing) {
			case SBMIN: inpstr=censor_swear_words(inpstr); break;
			case SBMAX: write_user(user,noswearing); return;
			default: break; /* do nothing as ban_swearing is off */
			}
		}
	vwrite_user(user, "%sKricis:~RS %s\n", colors[CSHOUT], inpstr);
	if (user->vis) name=user->bw_recap;
	else name=invisname;
	if (!user->vis) write_monitor(user,NULL,0);
	sprintf(text,"%s~RS %skrici:~RS %s\n", name, colors[CSHOUT], inpstr);
	write_room_except(NULL,text,user);
	record_shout(text);
}


/*** Tell another user something ***/
void tell(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u=user;
	char type[15],*name;
	int mur=0, i, c=0;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kecat");
		return;
		}
	/* check for qcall */
	if (inpstr[0]==',') c=1;
	if (word_count<2) {
		if (!c) write_user(user, "Povedat komu co ?\n");
		else write_user(user, "Povedat co ?\n");
		return;
		}
	if (word_count<3 && !c) {
		write_user(user,"Povedat co ?\n");
		return;
		}
	/* test na multi */
	if (!c) mur=count_musers(user, word[1]);
	else mur=1;
	inpstr=remove_first(inpstr);
	if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
		switch (amsys->ban_swearing) {
			case SBMIN: inpstr=censor_swear_words(inpstr); break;
			case SBMAX: write_user(user,noswearing); return;
			default : break; /* do nothing as ban_swearing is off */
			}
		}
	if (mur>1) {
		for (i=0; i<mur; i++) {
			u=get_user_name(user, user->murlist[i]);
			if (!u) {
				vwrite_user(user, "~OL~FR%s~RS : %s",
					user->murlist[i], notloggedon);
				continue;
				}
			if (u==user) {
				vwrite_user(user, "~OL~FR%s~RS : ~OLsebe ? hmm ...\n",
					user->murlist[i]);
				continue;
				}
			if ((check_igusers(u, user))!=-1 && (user->level<GOD || user->level<u->level)) {
				vwrite_user(user,"~OL~FR%s~RS : ignoruje tvoje telly.\n",
					user->murlist[i]);
				continue;
				}
			if (u->ignore.tells && (user->level<WIZ || u->level>user->level)) {
				vwrite_user(user,"~OL~FR%s~RS : ignoruje momentalne telly.\n",
					user->murlist[i]);
				continue;
				}
			if (u->afk) {
				if (u->afk_mesg[0]) vwrite_user(user,"~OL~FR%s~RS : ~FRAFK~RS, odkaz: %s\n",u->recap,u->afk_mesg);
				else vwrite_user(user,"~OL~FR%s~RS : ~FRAFK~RS momentalne.\n",u->recap);
				if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
				else strcpy(type,"tell");
				if (user->vis || u->level>=user->level) name=user->bw_recap;
				else name=invisname;
				sprintf(text,"~FG~OL%s %s%ss you:~RS %s\n",name, colors[CTELLUSER], type,inpstr);
				record_afk(u,text);
				sprintf(u->ltell, user->name);
				continue;
				}
			if (u->editing) {
				vwrite_user(user,"~FR~OL%s~RS : momentalne nieco pise.\n",u->recap);
				if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
				else strcpy(type,"tell");
				if (user->vis || u->level>=user->level) name=user->bw_recap;
				else name=invisname;
				sprintf(text,"~FG~OL%s %s%ss you:~RS %s\n", name, colors[CTELLUSER], type, inpstr);
				record_edit(u,text);
				sprintf(u->ltell, user->name);
				continue;
				}
			if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
				if (u->malloc_start!=NULL) vwrite_user(user,"~FR~OL%s~RS : momentalne nieco pise.\n",u->recap);
				else vwrite_user(user,"%s~RS is ignoring everyone at the moment.\n",u->recap);
				continue;
				}
		#ifdef NETLINKS
			if (u->room==NULL) {
				vwrite_user(user,"~FR~OL%s~RS : offsite and would not be able to reply to you.\n",u->recap);
				continue;
				}
		#endif
			if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
			else strcpy(type,"tell");
			sprintf(text,"~FTYou %s %s:~RS %s\n",type,u->bw_recap,inpstr);
			write_user(user,text);
			record_tell(user,text);
			if (user->vis || u->level>=user->level) name=user->bw_recap;
			else name=invisname;
			sprintf(text,"~OL~FT%s %s%ss you:~RS %s\n", name, colors[CTELLUSER], type, inpstr);
			write_user(u,text);
			record_tell(u,text);
			sprintf(u->ltell, user->name);
			}
		}
	else {
		u=get_user_name(user, c?user->call:word[1]);
		if (!u) {
			write_user(user,notloggedon);
			return;
			}
		if (u==user) {
			write_user(user,"Sebe ??? hmmmmm...... :(\n");
			return;
			}
		if ((check_igusers(u,user))!=-1
		    && (user->level<GOD
			|| user->level<u->level
			)
		    ) {
			vwrite_user(user,"%s~RS is ignoring tells from you.\n",u->recap);
			return;
			}
		if (u->ignore.tells && (user->level<WIZ || u->level>user->level)) {
			vwrite_user(user,"%s~RS is ignoring tells at the moment.\n",u->recap);
			return;
			}
		if (u->afk) {
			if (u->afk_mesg[0]) vwrite_user(user,"%s~RS je ~FRAFK~RS, odkaz: %s\n",u->recap,u->afk_mesg);
			else vwrite_user(user,"%s~RS je momentalne ~FRAFK~RS.\n",u->recap);
			write_user(user,"Sending message to their afk review buffer.\n");
			if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
			else strcpy(type,"tell");
			if (user->vis || u->level>=user->level) name=user->bw_recap; else name=invisname;
			sprintf(text,"~FT~OL%s %s%ss you:~RS %s\n", name, colors[CTELLUSER], type, inpstr);
			record_afk(u,text);
			sprintf(u->ltell, user->name);
			return;
			}
		if (u->editing) {
			vwrite_user(user,"%s~RS momentalne nieco pise.\n",u->recap);
			write_user(user,"Sending message to their edit review buffer.\n");
			if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
			else strcpy(type,"tell");
			if (user->vis || u->level>=user->level) name=user->bw_recap; else name=invisname;
			sprintf(text,"~FT~OL%s %s%ss you:~RS %s\n", name, colors[CTELLUSER], type, inpstr);
			record_edit(u,text);
			sprintf(u->ltell, user->name);
			return;
			}
		if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
			if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS momentalne nieco pise.\n",u->recap);
			else vwrite_user(user,"%s~RS is ignoring everyone at the moment.\n",u->recap);
			return;
			}
	#ifdef NETLINKS
		if (u->room==NULL) {
			vwrite_user(user,"%s~RS is offsite and would not be able to reply to you.\n",u->recap);
			return;
			}
	#endif
		if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
		else strcpy(type,"tell");
		sprintf(text,"%sYou %s %s:~RS %s\n", colors[CTELLUSER], type, u->bw_recap,inpstr);
		write_user(user,text);
		record_tell(user,text);
		if (user->vis || u->level>=user->level) name=user->bw_recap;
		else name=invisname;
		sprintf(text,"%s %s%ss you:~RS %s\n",name, colors[CTELLUSER], type, inpstr);
		write_user(u,text);
		record_tell(u,text);
		sprintf(u->ltell, user->name);
		}
}


/*** Emote something ***/
void emote(UR_OBJECT user, char *inpstr)
{
char *name;

	set_crash();
name="";
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "kecat");  return;
  }
if (word_count<2) {
  write_user(user,"Emote what?\n");  return;
  }
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch (amsys->ban_swearing) {
    case SBMIN: if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (user->type==CLONE_TYPE) {
  if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) vwrite_room(user->room,"The clone of %s%s\n",name,inpstr);
  else vwrite_room(user->room,"The clone of %s~RS %s\n",user->recap,inpstr);
  record(user->room,text);
  plugin_triggers(user, inpstr);
  return;
  }
if (user->vis) name=user->recap;
else name=invisname;
if (!user->vis) write_monitor(user,user->room,0);
if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"%s~RS%s\n",name,inpstr);
else sprintf(text,"%s~RS %s\n",name,inpstr);
write_room(user->room,text);
record(user->room,text);
plugin_triggers(user, inpstr);
}


/*** Do a shout emote ***/
void semote(UR_OBJECT user, char *inpstr)
{
char *name;

	set_crash();
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "kricat");  return;
  }
if (word_count<2) {
  write_user(user,"Shout emote what?\n");  return;
  }
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (user->vis) name=user->recap;
else name=invisname;
if (!user->vis) write_monitor(user,NULL,0);
if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"~OL!!!~RS %s~RS%s\n",name,inpstr);
else sprintf(text,"~OL!!~RS %s~RS %s\n",name,inpstr);
write_room(NULL,text);
record_shout(text);
}


/*** Do a private emote ***/
void pemote(UR_OBJECT user, char *inpstr)
{
	char *name, *pp=inpstr;
	UR_OBJECT u;
	int i, mur=0;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kecat");
		return;
		}
	if ((i=strlen(command_table[PEMOTE].alias))
	    && strncmp(inpstr, command_table[PEMOTE].alias, i)
	    && word_count<3
	    ) {
		write_user(user,"pemote co ?\n");
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
	if ((mur=count_musers(user, word[1]))<2) {
		if (!(u=get_user_name(user,word[1]))) {
			write_user(user,notloggedon);
			return;
			}
		if (u==user) {
			write_user(user,"Kecat so sebou je troxu uxylne, nemyslis ?\n");
			return;
			}
		if ((check_igusers(u,user))!=-1 && user->level<GOD) {
			vwrite_user(user,"%s~RS is ignoring private emotes from you.\n",u->recap);
			return;
			}
		if (u->ignore.tells && (user->level<WIZ || u->level>user->level)) {
			vwrite_user(user,"%s~RS is ignoring private emotes at the moment.\n",u->recap);
			return;
			}
		if (u->afk) {
			if (u->afk_mesg[0]) vwrite_user(user,"%s~RS is ~FRAFK~RS, message is: %s\n",u->recap,u->afk_mesg);
			else vwrite_user(user,"%s~RS is ~FRAFK~RS at the moment.\n",u->recap);
			write_user(user,"Sending message to their afk review buffer.\n");
			if (user->vis || u->level>=user->level) name=user->recap;
			else name=invisname;
			inpstr=remove_first(inpstr);
			sprintf(text, "%s(=>)~RS %s~RS %s\n", colors[CPEMOTE], name, inpstr);
			record_afk(u,text);
			return;
			}
		if (u->editing) {
			vwrite_user(user,"%s~RS is in ~FTEDIT~RS mode at the moment (using the line editor).\n",u->recap);
			write_user(user,"Sending message to their edit review buffer.\n");
			if (user->vis || u->level>=user->level) name=user->recap;
			else name=invisname;
			inpstr=remove_first(inpstr);
			sprintf(text, "%s(=>)~RS %s~RS %s\n", colors[CPEMOTE], name, inpstr);
			record_edit(u,text);
			return;
			}
		if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
			if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS is using the editor at the moment.\n",u->recap);
			else vwrite_user(user,"%s~RS is ignoring everyone at the moment.\n",u->recap);
			return;
			}
	#ifdef NETLINKS
		if (u->room==NULL) {
			vwrite_user(user,"%s~RS is offsite and would not be able to reply to you.\n",u->recap);
			return;
			}
	#endif
		if (user->vis || u->level>=user->level) name=user->recap;
		else name=invisname;
		inpstr=remove_first(inpstr);
		if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"%s(%s =>)~RS %s~RS%s\n", colors[CPEMOTE], u->bw_recap, name, inpstr);
		else sprintf(text, "%s(%s =>)~RS %s~RS %s\n", colors[CPEMOTE], u->bw_recap, name, inpstr);
		write_user(user,text);
		record_tell(user,text);
		if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"%s(=>)~RS %s~RS%s\n", colors[CPEMOTE], name, inpstr);
		else sprintf(text,"%s(=>)~RS %s~RS %s\n", colors[CPEMOTE], name, inpstr);
		write_user(u,text);
		record_tell(u,text);
		return;
		}

	for (i=0; i<mur; i++) {
		inpstr=pp;
		if (!(u=get_user_name(user, user->murlist[i]))) {
			vwrite_user(user, "~FR~OL%s~RS : %s",
				user->murlist[i], notloggedon);
			continue;
			}
		if (u==user) {
			vwrite_user(user,"~FR~OL%s~RS : sebe ? hmm...\n",
				user->murlist[i]);
			continue;
			}
		if ((check_igusers(u,user))!=-1 && user->level<GOD) {
			vwrite_user(user,"~FR~OL%s~RS : ignoring private emotes from you.\n",
				user->murlist[i]);
			continue;
			}
		if (u->ignore.tells && (user->level<WIZ || u->level>user->level)) {
			vwrite_user(user,"~FR~OL%s~RS : ignoring private emotes at the moment.\n",
				user->murlist[i]);
			continue;
			}
		if (u->afk) {
			if (u->afk_mesg[0]) vwrite_user(user,"%s~RS : ~FRAFK~RS, message is: %s\n",user->murlist[i],u->afk_mesg);
			else vwrite_user(user,"~FR~OL%s~RS : ~FRAFK~RS at the moment.\n",user->murlist[i]);
			if (user->vis || u->level>=user->level) name=user->recap;
			else name=invisname;
			inpstr=remove_first(inpstr);
			sprintf(text,"%s(=>)~RS %s~RS %s\n", colors[CPEMOTE], name, inpstr);
			record_afk(u,text);
			continue;
			}
		if (u->editing) {
			vwrite_user(user,"~FR~OL%s~RS : in ~FTEDIT~RS mode at the moment (using the line editor).\n",user->murlist[i]);
			if (user->vis || u->level>=user->level) name=user->recap;
			else name=invisname;
			inpstr=remove_first(inpstr);
			sprintf(text,"%s(=>)~RS %s~RS %s\n", colors[CPEMOTE], name, inpstr);
			record_edit(u,text);
			continue;
			}
		if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
			if (u->malloc_start!=NULL) vwrite_user(user,"~FR~OL%s~RS : using the editor at the moment.\n",user->murlist[i]);
			else vwrite_user(user,"~FR~OL%s~RS : ignoring everyone at the moment.\n",user->murlist[i]);
			continue;
			}
	#ifdef NETLINKS
		if (u->room==NULL) {
			vwrite_user(user,"~FR~OL%s~RS : offsite and would not be able to reply to you.\n",user->murlist[i]);
			continue;
			}
	#endif
		if (user->vis || u->level>=user->level) name=user->recap;
		else name=invisname;
		inpstr=remove_first(inpstr);
		if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"%s(%s =>)~RS %s~RS%s\n", colors[CPEMOTE], u->bw_recap, name, inpstr);
		else sprintf(text,"%s(%s =>)~RS %s~RS %s\n", colors[CPEMOTE], u->bw_recap, name, inpstr);
		write_user(user,text);
		record_tell(user,text);
		if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"%s(=>)~RS %s~RS%s\n", colors[CPEMOTE], name,inpstr);
		else sprintf(text,"%s(=>)~RS %s~RS %s\n", colors[CPEMOTE], name, inpstr);
		write_user(u,text);
		record_tell(u,text);
		}
}


/*** Echo something to screen ***/
void s_echo(UR_OBJECT user, char *inpstr)
{
	set_crash();
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "echovat");  return;
  }
if (word_count<2) {
  write_user(user,"Echo what?\n");  return;
  }
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
write_monitor(user,user->room,0);
sprintf(text,"%s\n",inpstr);
write_room(user->room,text);
record(user->room,text);
}


/*** Set the room topic ***/
void set_topic(UR_OBJECT user, char *inpstr)
{
RM_OBJECT rm;
char *name;

	set_crash();
rm=user->room;
if (word_count<2) {
  if (!strlen(rm->topic)) {
    write_user(user,"No topic has been set yet.\n");  return;
    }
  vwrite_user(user,"The current topic is: %s\n",rm->topic);
  return;
  }
if (strlen(inpstr)>TOPIC_LEN) {
  write_user(user,"Topic too long.\n");  return;
  }
vwrite_user(user,"Topic set to: %s\n",inpstr);
if (user->vis) name=user->recap;
else name=invisname;
vwrite_room_except(rm,user,"%s~RS has set the topic to: %s\n",name,inpstr);
strcpy(rm->topic,inpstr);
}


/*** Broadcast an important message ***/
void bcast(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <message>", command_table[BCAST].name);
		return;
		}
	/* wizzes should be trusted... But they ain't! */
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
	force_listen=1;
	write_monitor(user,NULL,0);
	vwrite_room(NULL,"\007~FR~OL--==<~RS %s ~RS~FR~OL>==--\n", inpstr);
	for (u=user_first; u!=NULL; u=u->next)
		if (u->pueblo && !audioprompt(u,2,1))
			write_user(u,"</xch_mudtext><img xch_alert><xch_mudtext>");
}


/*** Wake up some idle sod ***/
void wake(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char *name,*b="\007",null[1],*bp;
	char text1[WAKEMSG_LEN+1];

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user>", command_table[WAKE].name);
		return;
		}
	strncpy(text1, remove_first(inpstr), WAKEMSG_LEN);
	text1[WAKEMSG_LEN]='\0';
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "budit");
		return;
		}
	if (!(u=get_user_name(user,word[1]))) {
		write_user(user,notloggedon);
		return;
		}
	if (u==user) {
		write_user(user,"Trying to wake yourself up is the eighth sign of madness.\n");
		return;
		}
	if ((check_igusers(u,user))!=-1 && user->level<GOD) {
		vwrite_user(user,"%s~RS is ignoring wakes from you.\n",u->recap);
		return;
		}
	if (u->afk) {
		write_user(user,"You cannot wake someone who is AFK.\n");
		return;
		}
	if (u->level>=GOD && user->level<L_9) {
		write_user(user, "Nemozes budit GODov !\n");
		return;
		}
	if ((check_igusers(u, user))!=-1
	    && (user->level<GOD
	        ||user->level<u->level
	        )
	    ) {
		vwrite_user(user, ">>>%s is ignoring you. You must ask him to forgive you first.\n", u->name);
		return;
		}
	if (!u->ignore.funs) show_file(u, WAKEFILE);
	if (user->vis) name=user->bw_recap;
	else name=invisname;
	null[0]='\0';
	if (u->ignore.beeps) bp=null;
	else bp=b;
	vwrite_user(u,"\n%s~BR*** %s krici: ~OL~LIZOBUD SA ~RS~BR(~RS%s~RS~BR) !!! ***\n\n",
		bp, name, text1);
	if (u->pueblo && !audioprompt(u,2,1)) write_user(u,"</xch_mudtext><img xch_alert><xch_mudtext>");
	vwrite_user(user,"Budicek poslany pre %s (%s~RS)\n",
		u->name, text1);
}


/*** Shout something to other wizes and gods. If the level isnt given it
     defaults to WIZ level. ***/
void wizshout(UR_OBJECT user, char *inpstr)
{
int lev;

	set_crash();
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "kricat");  return;
  }
if (word_count<2) {
  write_usage(user,"%s [<superuser level>] <text>", command_table[WIZSHOUT].name); 
  return;
  }
/* Even wizzes cannot escapde the swear ban!  MWHAHahaha.... ahem.  */
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
strtoupper(word[1]);
if ((lev=get_level(word[1]))==-1) lev=WIZ;
else {
  if (lev<WIZ || word_count<3) {
    write_usage(user,"%s [<superuser level>] <text>", command_table[WIZSHOUT].name);
    return;
    }
  if (lev>user->level) {
    write_user(user,"You cannot specifically shout to users of a higher level than yourself.\n");
    return;
    }
  inpstr=remove_first(inpstr);
  sprintf(text,"~FY<WIZ: ~OL[%s]~RS~FY>~RS %s\n",user_level[lev].name,inpstr);
  write_user(user,text);
  record_tell(user,text);
  sprintf(text,wizshout_style_lev, user_level[lev].name, user->bw_recap, inpstr);
  write_level(lev,1,text,user);
  return;
  }
sprintf(text,"~FY<WIZ>~RS %s\n",inpstr);
write_user(user,text);
record_tell(user,text);
sprintf(text, wizshout_style, user->bw_recap, inpstr);
write_level(WIZ,1,text,user);
}


/*** Clear the review buffer ***/
void revclr(UR_OBJECT user)
{
	char *name;

	set_crash();
	clear_revbuff(user->room); 
	write_user(user,"Review buffer cleared.\n");
	if (user->vis) name=user->name;
	else name=invisname;
	vwrite_room_except(user->room,user, cbuff_prompt, name);
}


/*** Show recorded tells and pemotes ***/
void revtell(UR_OBJECT user)
{
	int i,cnt,line;

	set_crash();
	if (user->restrict[RESTRICT_VIEW]==restrict_string[RESTRICT_VIEW]) {
		write_user(user,">>>Sorry, you have no right to access the revtell buffer.\n");
		return;
		}
cnt=0;
for(i=0;i<REVTELL_LINES;++i) {
  line=(user->revline+i)%REVTELL_LINES;
  if (user->revbuff[line][0]) {
    cnt++;
    if (cnt==1) write_user(user, tell_review_header);
    write_user(user,user->revbuff[line]); 
    }
  }
if (!cnt) write_user(user, no_tell_review_prompt);
else write_user(user,"\n~BB~FG*** End ***\n\n");
}


/* clears a room topic */
void clear_topic(UR_OBJECT user)
{
RM_OBJECT rm;
char *name;

	set_crash();
strtolower(word[1]);
if (word_count<2) {
  rm=user->room;
  rm->topic[0]='\0';
  write_user(user,"Topic has been cleared\n");
  if (user->vis) name=user->name; else name=invisname;
  vwrite_room_except(rm,user,"~FY~OL%s has cleared the topic.\n",name);
  return;
  }
if (!strcmp(word[1],"all")) {
  if (user->level>command_table[CTOPIC].level || user->level>=ARCH) {
    for(rm=room_first;rm!=NULL;rm=rm->next) {
      rm->topic[0]='\0';
      write_room_except(rm,"\n~FY~OLThe topic has been cleared.\n",user);
      }
    write_user(user,"All room topics have now been cleared\n");
    return;
    }
  write_user(user,"You can only clear the topic of the room you are in.\n");
  return;
  }
write_usage(user,"%s [all]", command_table[CTOPIC].name);
}


/** Tell something to everyone but one person **/
void mutter(UR_OBJECT user, char *inpstr)
{
UR_OBJECT user2;
char *name;

	set_crash();
if (word_count<3) {
  write_usage(user,"%s <user> <text>", command_table[MUTTER].name);
  return;
  }
if (!(user2=get_user_name(user,word[1]))) {
  write_user(user,notloggedon); return;
  }
inpstr=remove_first(inpstr);
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
	      return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (user->room!=user2->room) {
  vwrite_user(user,"%s~RS is not in the same room, so speak freely of them.\n",user2->recap);
  return;
  }
if (user2==user) {
  write_user(user,"Talking about yourself is a sign of madness!\n");
  return;
  }
if (user->vis) name=user->bw_recap;
else {
  name=invisname;
  write_monitor(user,user->room,0);
  }
vwrite_room_except(user->room,user2,"~FT%s mutters:~RS %s ~FY~OL(to all but %s)\n",name,inpstr,user2->bw_recap);
}


/** ask all the law, (sos), no muzzle restrictions **/
void plead(UR_OBJECT user, char *inpstr)
{
int found=0;
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <text>", command_table[SOS].name);
  return;
  }
if (user->level>=WIZ) {
  write_user(user,"You are already a wizard!\n");
  return;
  }
for (u=user_first;u!=NULL;u=u->next)  if (u->level>=WIZ && !u->login) found=1;
if (!found) {
  write_user(user, no_wizs_logged);
  return;
  }
sprintf(text,"~OL[ SOS from %s ]~RS %s\n",user->bw_recap,inpstr);
write_level(WIZ,1,text,NULL);
sprintf(text,"~OLYou sos to the wizzes:~RS %s\n",inpstr);
write_user(user,text);
record_tell(user,text);
}


/* Displays a picture to a person */
void picture_tell(UR_OBJECT user)
{
	UR_OBJECT u;
	char fname[500], *name, *c;

	set_crash();
if (word_count<3) {
  write_usage(user,"%s <user> <obrazok>", command_table[PTELL].name);
  return;
  }
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "nic posielat");
  return;
  }
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);  return;
  }
if (u==user) {
  write_user(user,"There is an easier way to see pictures...\n");
  return;
  }
if ((check_igusers(u,user))!=-1 && user->level<GOD) {
  vwrite_user(user,"%s~RS is ignoring pictures from you.\n",u->recap);
  return;
  }
if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
  if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS is writing a message at the moment.\n",u->recap);
  else vwrite_user(user,"%s~RS is not listening at the moment.\n",u->recap);
  return;
  }
#ifdef NETLINKS
  if (u->room==NULL) {
    vwrite_user(user,"%s~RS is offsite and would not be able to reply to you.\n",u->recap);
    return;
    }
#endif
if (u->afk) {
	if (u->afk_mesg[0])
		sprintf(text,"~OL~FG%s ~FWje STAND BY, odkaz je:~RS %s\n",u->name,u->afk_mesg);
	else
		sprintf(text,"~OL~FG%s ~FWje momentalne STAND BY.\n",u->name);
	write_user(user,text);
	return;
	}
c=word[2];
while(*c) {
  if (*c=='.' || *c++=='/') {
    write_user(user,"Sorry, there is no picture with that name.\n");
    return;
    }
  }
if (!user->vis && u->level<user->level) name=invisname;
else name=user->recap;
sprintf(fname,"%s/%s.pic", PICTFILES, word[2]);
if (u->ignore.funs || u->ignore.pics) {
	if (file_exists(fname))
		vwrite_user(u, "~OLObrazok ~FG%s~FW ti posiela ~FR%s\n", word[2], name);
	else {
		write_user(user, "Taky obrazok neexistuje !\n");
		return;
		}
	}
else {
	if (!show_file(u, fname)) {
		write_user(user,"Taky obrazok neexistuje !\n");
		return;
		}
	vwrite_user(user, "\n%s ti posiela tento obrazok\n", name);
	}
vwrite_user(user, "~OLobrazok ~FG%s~FW pre ~FR%s\n",word[2],u->name);
}


/* see list of pictures availiable - file dictated in 'go' script */
void preview(UR_OBJECT user)
{
	char fname[500],*c;

	set_crash();
	if (word_count<2) {
		write_user(user,"The following pictures can be viewed...\n\n");
		switch (more(user,user->socket, PICTLIST)) {
			case 0:
				write_user(user,"No list of the picture files is availiable.\n");
				break;
			case 1: user->misc_op=2;
			}
		return;
		}
	c=word[1];
	while(*c) {
		if (*c=='.' || *c++=='/') {
			write_user(user,"Sorry, there is no picture with that name.\n");
			return;
			}
		}
	sprintf(fname,"%s/%s.pic", PICTFILES, word[1]);
	write_user(user, "\n");
	if (!show_file(user, fname)) {
		write_user(user,"Sorry, there is no picture with that name.\n");
		return;
		}
	write_user(user,"\n");
}


/*** Show a picture to the whole room that the user is in ***/
void picture_all(UR_OBJECT user)
{
UR_OBJECT u;
char filename[500],*name,*c;
FILE *fp;

	set_crash();
if (word_count<2) {
  write_usage(user, "%s <obrazok>", command_table[PICTURE].name);
  preview(user); 
  return;
  }
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "nic posielat");
  return;
  }
c=word[1];
while(*c) {
  if (*c=='.' || *c++=='/') {
    write_user(user,"Sorry, there is no picture with that name.\n");
    return;
    }
  }
sprintf(filename,"%s/%s.pic", PICTFILES,word[1]);
if (!(fp=fopen(filename,"r"))) {
  write_user(user,"Sorry, there is no picture with that name.\n");
  return;
  }
fclose(fp);
for (u=user_first;u!=NULL;u=u->next) {
  if (u->login
      || u->room==NULL
      || (u->room!=user->room && user->room!=NULL)
      || (u->ignore.all && !force_listen)
      || u->ignore.pics
      || u==user) continue;
  if ((check_igusers(u,user))!=-1 && user->level<GOD) continue;
  if (!user->vis && u->level<=user->level) name=invisname;  else  name=user->bw_recap;
  if (u->type==CLONE_TYPE) {
    if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignore.all
	|| u->clone_hear==CLONE_HEAR_SWEARS) continue;
    /* Ignore anything not in clones room, eg shouts, system messages
       and shemotes since the clones owner will hear them anyway. */
    if (user->room!=u->room) continue;
    vwrite_user(u->owner,"~BG~FK[ %s ]:~RS~FG~OL %s shows the following picture...\n\n",u->room->name,name);
    show_file(u, filename);
/*    while(!feof(fp)) {
	fgets(text,ARR_SIZE,fp);
	write_user(u,text);
	}*/
    write_user(u, "\n");
    }
  else {
    vwrite_user(u,"~FG~OL%s shows the following picture...\n\n",name);
    show_file(u, filename);
/*    while(!feof(fp)) {
	fgets(text,ARR_SIZE,fp);
	write_user(u,text);
	}*/
    write_user(u, "\n");
    } /* end if else */
  } /* end for */
write_user(user,"~FG~OLObrazok bol poslany do ruumy.\n\n");
fclose(fp);
}


/*** print out greeting in large letters ***/
void greet(UR_OBJECT user, char *inpstr)
{
	char pbuff[ARR_SIZE],temp[8];
	int slen,lc,c,i,j;
	char *clr[]={"~OL~FR","~OL~FG","~OL~FT","~OL~FM","~OL~FY"};

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "pouzit tento prikaz");
		return;
		}
	if (word_count<2) {
		write_usage(user,"%s <text>", command_table[GREET].name);
		return;
		}
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (strlen(inpstr)>11) {
  write_user(user,"You can only have up to 11 letters in the greet.\n");
  return;
  }
write_monitor(user,user->room,0);
write_room(user->room,"\n");
slen=strlen(inpstr);
if (slen>11) slen=11;
for (i=0; i<5; ++i) {
  pbuff[0] = '\0';
  temp[0]='\0';
  for (c=0; c<slen; ++c) {
    /* check to see if it's a character a-z */
    if (isupper(inpstr[c]) || islower(inpstr[c])) {
      lc = tolower(inpstr[c]) - 'a';
      if ((lc >= 0) && (lc < 27)) { 
	for (j=0;j<5;++j) {
	  if(biglet[lc][i][j]) {
	    sprintf(temp,"%s#",clr[rand()%5]);
	    strcat(pbuff,temp);
	    }
	  else strcat(pbuff," ");
	  }
	strcat(pbuff,"  ");
        }
      }
    /* check if it's a character from ! to @ */
    if (isprint(inpstr[c])) {
      lc = inpstr[c] - '!';
      if ((lc >= 0) && (lc < 32)) { 
	for (j=0;j<5;++j) {
	  if(bigsym[lc][i][j]) {
	    sprintf(temp,"%s#",clr[rand()%5]);
	    strcat(pbuff,temp);
	    }
	  else strcat(pbuff," ");
	  }
	strcat(pbuff,"  ");
        }
      }
    }
  vwrite_room(user->room,"%s\n",pbuff);
  }
}


/** put speech in a think bubbles **/
void think_it(UR_OBJECT user, char *inpstr)
{
	char *name;

	set_crash();
	if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
		switch(amsys->ban_swearing) {
			case SBMIN:
				if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
				break;
			case SBMAX: write_user(user,noswearing); return;
			default : break; /* do nothing as ban_swearing is off */
			}
		}
	if (user->vis) name=user->recap;
	else {
		name=invisname;
		write_monitor(user,user->room,0);
		}
	if (word_count<2) sprintf(text,"%s~RS%s si mysli ... aaale nist :)\n", name, colors[CTHINK]);
	else sprintf(text, "%s~RS%s . o O (~RS %s~RS %s)\n", name, colors[CTHINK], inpstr, colors[CTHINK]);
	write_room(user->room,text);
	record(user->room,text);
	plugin_triggers(user, inpstr);
}


/** put speech in a music notes **/
void sing_it(UR_OBJECT user, char *inpstr)
{
char *name;

	set_crash();
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (user->vis) name=user->recap;
else {
  name=invisname;
  write_monitor(user,user->room,0);
  }
if (word_count<2) sprintf(text,"%s~RS sings a tune... BADLY!\n",name);
else sprintf(text,"%s~RS sings o/~ %s~RS o/~\n",name,inpstr);
write_room(user->room,text);
record(user->room,text);
plugin_triggers(user, inpstr);
}


/*** Emote something to other wizes and gods. If the level isnt given it
     defaults to WIZ level. ***/
void wizemote(UR_OBJECT user, char *inpstr)
{
int lev;

	set_crash();
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "kecat");  return;
  }
if (word_count<2) {
  write_usage(user,"%s [<Wiz level>] <text>", command_table[WIZEMOTE].name); 
  return;
  }
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
strtoupper(word[1]);
if ((lev=get_level(word[1]))==-1) lev=WIZ;
else {
  if (lev<WIZ || word_count<3) {
    write_usage(user,"%s [<Wiz level>] <text>", command_table[WIZEMOTE].name);
    return;
    }
  if (lev>user->level) {
    write_user(user,"You cannot specifically emote to users of a higher level than yourself.\n");
    return;
    }
  inpstr=remove_first(inpstr);
  sprintf(text,"~FY<WIZ: ~OL[%s]~RS~FY=>~RS %s~RS %s\n",user_level[lev].name,user->recap,inpstr);
  write_level(lev,1,text,NULL);
  return;
  }
sprintf(text,"~FY<WIZ: =>~RS %s~RS %s\n",user->recap,inpstr);
write_user(user,text);
record_tell(user,text);
sprintf(text,"~FY<WIZ: =>~RS %s~RS %s\n",user->recap,inpstr);
write_level(WIZ,1,text,user);
}


/*** See review of shouts ***/
void revshout(UR_OBJECT user)
{
	int i,line,cnt;

	set_crash();
	cnt=0;
	for(i=0;i<REVIEW_LINES;++i) {
		line=(amsys->sbuffline+i)%REVIEW_LINES;
		if (amsys->shoutbuff[line][0]) {
			cnt++;    
			if (cnt==1) write_user(user, shout_review_header);
			write_user(user,amsys->shoutbuff[line]); 
			}
		}
	if (!cnt) write_user(user, no_shout_review_prompt);
	else write_user(user,"\n~BB~FG*** End ***\n\n");
}


/*** Clear the shout buffer of the talker ***/
void clear_shouts(void)
{
	int c;

	set_crash();
	for(c=0;c<REVIEW_LINES;++c)
		amsys->shoutbuff[c][0]='\0';
	amsys->sbuffline=0;
}


/*** Clear the tell buffer of the user ***/
void clear_tells(UR_OBJECT user)
{
	int c;

	set_crash();
	for(c=0;c<REVTELL_LINES;++c)
		user->revbuff[c][0]='\0';
	user->revline=0;
}


/* set a name for a quick call */
void quick_call(UR_OBJECT user)
{
UR_OBJECT u;

	set_crash();
if (word_count<2) {
  if (!user->call[0]) {
    write_user(user,"Quick call nenastaveny.\n");
    return;
    }
  vwrite_user(user,"~OLQuick call: ~FG%s~RS\n",user->call);
  return;
  }
if (!strcmp(word[1], "-cancel")) {
	user->call[0]='\0';
	write_user(user, "Quick call zruseny\n");
	return;
	}
if (strlen(word[1])>USER_NAME_LEN) {
  write_user(user,"Pridlhe meno\n");
  return;
  }
if ((u=get_user_name(user,word[1]))==NULL) {
  write_user(user,"User musi byt zaweseny !\n");
  return;
  }
if (u==user) {
  write_user(user,"Qcall si nemozes nastavit na seba !\n");
  return;
  }
strcpy(user->call,u->name);
user->call[0]=toupper(user->call[0]);
vwrite_user(user,"Qcall nastaveny na: ~OL%s\n", user->call);
}


/*** Show recorded tells and pemotes ***/
void revafk(UR_OBJECT user)
{
int i,cnt,line;
	set_crash();
cnt=0;
for(i=0;i<REVTELL_LINES;++i) {
  line=(user->afkline+i)%REVTELL_LINES;
  if (user->afkbuff[line][0]) {
    cnt++;
    if (cnt==1) write_user(user,"\n~BB~FG*** Your AFK review buffer ***\n\n");
    write_user(user,user->afkbuff[line]); 
    }
  }
if (!cnt) write_user(user,"AFK review buffer is empty.\n");
else write_user(user,"\n~BB~FG*** End ***\n\n");
}


/*** Clear the tell buffer of the user ***/
void clear_afk(UR_OBJECT user)
{
	int c;

	set_crash();
	for(c=0;c<REVTELL_LINES;++c)
		user->afkbuff[c][0]='\0';
	user->afkline=0;
}


/*** Show recorded tells and pemotes ***/
void revedit(UR_OBJECT user)
{
int i,cnt,line;
	set_crash();
cnt=0;
for(i=0;i<REVTELL_LINES;++i) {
  line=(user->editline+i)%REVTELL_LINES;
  if (user->editbuff[line][0]) {
    cnt++;
    if (cnt==1) write_user(user,"\n~BB~FG*** Your edit review  buffer ***\n\n");
    write_user(user,user->editbuff[line]); 
    }
  }
if (!cnt) write_user(user,"EDIT review buffer is empty.\n");
else write_user(user,"\n~BB~FG*** End ***\n\n");
}


/*** Clear the tell buffer of the user ***/
void clear_edit(UR_OBJECT user)
{
	int c;

	set_crash();
	for(c=0;c<REVTELL_LINES;++c)
		user->editbuff[c][0]='\0';
	user->editline=0;
}


/*** Direct a say to someone, even though the whole room can hear it ***/
void say_to(UR_OBJECT user, char *inpstr)
{
	char type[15],*name1,*name2, *pp=inpstr;
	UR_OBJECT u;
	int mur=0, i;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kecat");
		return;
		}
	if ((i=strlen(command_table[SAYTO].alias))
	    && strncmp(inpstr, command_table[SAYTO].alias, i)
	    && word_count<3
	    ) {
		write_user(user,"Say what to who?\n");
		return;
		}
	if ((mur=count_musers(user, word[1]))<2) {
		if ((u=get_user_name(user,word[1]))==NULL) {
			write_user(user,notloggedon);
			return;
			}
		}
	if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
		switch (amsys->ban_swearing) {
			case SBMIN:
				if (!(in_private_room(user))) inpstr=censor_swear_words(inpstr);
				break;
			case SBMAX:
				write_user(user,noswearing);
				return;
			default : break; /* do nothing as ban_swearing is off */
			}
		}
	if (mur==1) {
		if (u==user) {
			write_user(user,"Talking to yourself is the first sign of madness!\n");
			return;
			}
		if ((check_igusers(u, user))!=-1
		    && (user->level<GOD
			|| user->level<u->level
			)
		    ) {
			vwrite_user(user,"~OL~FR%s~RS : ignoring tells from you.\n",
				user->murlist[i]);
			return;
			}
	#ifdef NETLINKS
		/* can send over netlink as normal say */
		if (user->room==NULL) {
			sprintf(text,"ACT %s say %s\n",user->name,inpstr);
			write_sock(user->netlink->socket,text);
			no_prompt=1;
			return;
			}
	#endif
		smiley_type(inpstr,type);
		inpstr=remove_first(inpstr);
		vwrite_user(user,"~FTYou %s to %s:~RS %s\n",type,u->bw_recap,inpstr);
		if (user->vis) name1=user->bw_recap;
		else name1=invisname;
		if (u->vis) name2=u->bw_recap;
		else name2=invisname;
		if (!user->vis) write_monitor(user,user->room,0);
		sprintf(text,"~FT%s %ss (to %s):~RS %s\n",name1,type,name2,inpstr);
		write_room_except(user->room,text,user);
		record(user->room,text);
		plugin_triggers(user, inpstr);
		return;
		}
	for (i=0; i<mur; i++) {
		inpstr=pp;
		u=get_user_name(user, user->murlist[i]);
		if (!u) {
			vwrite_user(user, "~OL~FR%s~RS : %s",
				user->murlist[i], notloggedon);
			continue;
			}
		if (u==user) {
			vwrite_user(user,"~OL~FR%s~RS : ~OLsebe ? hmmm...\n",
				user->murlist[i]);
			continue;
			}
		if ((check_igusers(u, user))!=-1
		    && (user->level<GOD
			|| user->level<u->level
			)
		    ) {
			vwrite_user(user,"~OL~FR%s~RS : ignoruje ta.\n",user->murlist[i]);
			continue;
			}
	#ifdef NETLINKS
		/* can send over netlink as normal say */
		if (user->room==NULL) {
			sprintf(text,"ACT %s say %s\n",user->name,inpstr);
			write_sock(user->netlink->socket,text);
			no_prompt=1;
			continue;
			}
	#endif
		smiley_type(inpstr,type);
		inpstr=remove_first(inpstr);
		vwrite_user(user,"~FTYou %s to %s:~RS %s\n",type,u->bw_recap,inpstr);
		if (user->vis) name1=user->bw_recap;
		else name1=invisname;
		if (u->vis) name2=u->bw_recap;
		else name2=invisname;
		if (!user->vis) write_monitor(user,user->room,0);
		sprintf(text,"~FT%s %ss (to %s):~RS %s\n",name1,type,name2,inpstr);
		write_room_except(user->room,text,user);
		record(user->room,text);
		plugin_triggers(user, inpstr);
		continue;
		}
}


/* take a users name and add it to friends list */
void friends(UR_OBJECT user)
{
int i,cnt,found;
char filename[500];
FILE *fp;
struct user_dir_struct *entry;

	set_crash();
cnt=0;
if (word_count<2) {
  for (i=0;i<MAX_FRIENDS;++i) {
    if (!user->friend[i][0]) continue;
    if (++cnt==1) {
      write_user(user,"+----------------------------------------------------------------------------+\n");
      write_user(user,"| ~FT~OLNames on your friends list are as follows~RS                                  |\n");
      write_user(user,"+----------------------------------------------------------------------------+\n");
      }
    vwrite_user(user,"| %-74s |\n",user->friend[i]);
    }
  if (!cnt) write_user(user,"You have no names on your friends list.\n");
  else {
    write_user(user,"+----------------------------------------------------------------------------+\n");
    if (!user->alert) write_user(user,"| ~FTYou are currently not being alerted~RS                                        |\n");
    else write_user(user,"| ~OL~FTYou are currently being alerted~RS                                            |\n");
    write_user(user,"+----------------------------------------------------------------------------+\n");
    }
  return;
  }
if (strlen(word[1])>USER_NAME_LEN) {
  write_user(user,"Your friend doesn't have a name that long!\n");
  return;
  }
if (strlen(word[1])<3) {
  write_user(user,"Your friend doesn't have a name that short!\n");
  return;
  }
word[1][0]=toupper(word[1][0]);
if (!strcmp(word[1],user->name)) {
  write_user(user,"You should know when you log on!\n");
  return;
  }
for (i=0;i<MAX_FRIENDS;++i) {
  if (!strcmp(user->friend[i],word[1])) {
    vwrite_user(user,"You have removed %s from your friend list.\n",user->friend[i]);
    user->friend[i][0]='\0';
    goto SKIP;
    }
  }
found=0;
for (entry=first_dir_entry;entry!=NULL;entry=entry->next) {
  if (!strcmp(entry->name,word[1])) {
    found=1;  break;
    }
  }
if (!found) {
  write_user(user,nosuchuser);
  return;
  }
for (i=0;i<MAX_FRIENDS;++i) {
  if (!user->friend[i][0]) {
    strcpy(user->friend[i],word[1]);
    vwrite_user(user,"You have added %s to your list of friends.\n",user->friend[i]);
    goto SKIP;
    }
  }
write_user(user,"You have the maximum amount of friends listed already.\n");
return;
SKIP:
sprintf(filename,"%s/%s.F", USERFRIENDS,user->name);
if (!(fp=fopen(filename,"w"))) {
  write_user(user,"ERROR: Unable to open to friend list file.\n");
  write_syslog(ERRLOG,1,"Unable to open %s's friend list in friends().\n",user->name);
  return;
  }
cnt=0;
for (i=0;i<MAX_FRIENDS;++i) {
  if (!user->friend[i][0]) continue;
  fprintf(fp,"%s\n",user->friend[i]);
  cnt++;
  }
fclose(fp);
if (!cnt) unlink(filename);
}


/*** Say user speech to all people listed on users friends list ***/
void friend_say(UR_OBJECT user, char *inpstr)
{
char type[15],*name;
int i,cnt;

	set_crash();
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "kecat");
  return;
  }
/* check to see if use has friends listed */
cnt=0;
for (i=0;i<MAX_FRIENDS;++i) if (user->friend[i][0]) ++cnt;
if (!cnt) {
  write_user(user,"You have no friends listed.\n");
  return;
  }
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (word_count<2) {
  write_user(user,"Say what to your friends?\n");  return;
  }
smiley_type(inpstr,type);
sprintf(text,"~FGYou %s to your friends:~RS %s\n",type,inpstr);
write_user(user,text);
record_tell(user,text);
if (user->vis) name=user->bw_recap; else name=invisname;
sprintf(text,"~FGFriend %s %ss:~RS %s\n",name,type,inpstr);
write_friends(user,text,1);
}


/*** Emote something to all the people on the suers friends list ***/
void friend_emote(UR_OBJECT user, char *inpstr)
{
char *name;
int i,cnt;

	set_crash();
if (user->muzzled) {
  vwrite_user(user, muzzled_cannot, "kecat");  return;
  }
if (word_count<2) {
  write_user(user,"Emote what to your friends?\n");
  return;
  }
/* check to see if use has friends listed */
cnt=0;
for (i=0;i<MAX_FRIENDS;++i) if (user->friend[i][0]) ++cnt;
if (!cnt) {
  write_user(user,"You have no friends listed.\n");
  return;
  }
if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
  switch(amsys->ban_swearing) {
    case SBMIN: inpstr=censor_swear_words(inpstr);
              break;
    case SBMAX: write_user(user,noswearing);
              return;
    default : break; /* do nothing as ban_swearing is off */
    }
  }
if (user->vis) name=user->recap;
else name=invisname;
if (inpstr[0]=='\'' && (inpstr[1]=='s' || inpstr[1]=='S')) sprintf(text,"~OL~FGFriend~RS %s~RS%s\n",name,inpstr);
else sprintf(text,"~OL~FGFriend~RS %s~RS %s\n",name,inpstr);
write_user(user,text);
record_tell(user,text);
write_friends(user,text,1);
}


/* Beep a user - as tell but with audio warning */
void s_beep(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "nikoho prezvanat");
		return;
		}
	if (word_count<2) {
		write_usage(user,"%s <user> [<text>]", command_table[BEEP].name);
		return;
		}
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (u==user) {
  write_user(user,"Beeping yourself is yet another sign of madness!\n");
  return;
  }
if (u->ignore.beeps) {
  vwrite_user(user,"%s~RS is ignoring beeps at the moment.\n",u->recap);
  return;
  }
if ((check_igusers(u,user))!=-1 && user->level<GOD) {
  vwrite_user(user,"%s~RS is ignoring tells from you.\n",u->recap);
  return;
  }
if (u->ignore.all && (user->level<GOD || u->level>user->level)) {
  if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS is writing a message at the moment.\n",u->recap);
  else vwrite_user(user,"%s~RS is not listening at the moment.\n",u->recap);
  return;
  }
inpstr=remove_first(inpstr);
if (!u->vis && user->level<u->level) {
  write_user(user,"You cannot see that person here!\n");
  return;
  }
if (word_count<3) {
  if (!user->vis && u->level<user->level) write_user(u,"\007~FR~OLSomeone beeps to you:~RS~FR -=[*] BEEP [*]=-\n");
  else vwrite_user(u,"\007~FR~OL%s beeps to you:~RS~FR -=[*] BEEP [*]=-\n",user->bw_recap);
  vwrite_user(user,"\007~FR~OLYou beep to %s:~RS~FR -=[*] BEEP [*]=-\n",u->bw_recap);
  return;
  }
if (!user->vis && u->level<user->level) vwrite_user(u,"\007~FR~OLSomeone beeps to you:~RS %s\n",inpstr);
else vwrite_user(u,"\007~FR~OL%s beeps to you:~RS %s\n",user->bw_recap,inpstr);
vwrite_user(user,"\007~FR~OLYou beep to %s:~RS %s\n",u->bw_recap,inpstr);
}

/*****************************************************************************/
/* Doplnene funkcie                                                          */
/*****************************************************************************/

void hug(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	int mur, i;

	set_crash();
	if (word_count<2) {
		write_usage(user, "%s <user> [<text>]", command_table[HUG].name);
		return;
		}
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "zdravit");
		return;
		}
	mur=count_musers(user, word[1]);
	inpstr=remove_first(inpstr);
	if (mur>1) {
		for (i=0; i<mur; i++) {
			if ((u=get_user_name(user, user->murlist[i]))==NULL) {
				vwrite_user(user, "~OL~FR%s~RS: %s",
					user->murlist[i], notloggedon);
				continue;
				}
			if (u==user) {
				vwrite_user(user, "~OL~FR%s~RS: ~OLseba ? hmm ...\n",
					user->murlist[i]);
				continue;
				}
			if ((check_igusers(u, user))!=-1
			    && (user->level<GOD
				|| user->level<u->level
				)
			    ) {
				vwrite_user(user,"~OL~FR%s~RS : momentalne ta ignoruje.\n",
					user->murlist[i]);
				continue;
				}
			if ((u->ignore.all || u->ignore.tells) && (user->level<ARCH || u->level>user->level)) {
				vwrite_user(user,"~OL~FR%s~RS : momentalne ignoruje.\n",
					user->murlist[i]);
				continue;
				}
		#ifdef NETLINKS
			if (u->room==NULL) {
				vwrite_user(user,"~FR~OL%s~RS : je momentalne mimo sajty, nemoze odpovedat.\n", u->name);
				continue;
				}
		#endif
			if (!u->ignore.funs) {
				show_file(u, HUGFILE);
				write_user(u, "\n");
				}
			if (u->afk) {
				if (word_count==2) sprintf(text, "~OL~FG%s~FW ta pozdravuje:~RS %s\n", user->name, inpstr);
				else sprintf(text, "~OL~FG%s~FW ta pozdravuje !\n", user->name);
				record_afk(u, text);
				}
			else vwrite_user(u, "~OL~FG%s~FW ta pozdravuje !\n", user->name);
			if (word_count>2) {
					if (!u->afk) vwrite_user(u, "A sepka ti:~RS %s\n", inpstr);
				sprintf(text, "~FG~OLHUG~RS od %s:~RS %s\n", user->name, inpstr);
				}
			else sprintf(text, "~FG~OLHUG~RS od %s\n", user->name);
			record_tell(u, text);
			if (word_count==2) vwrite_user(user, "~FG%s ~OLOK\n", u->name);
			else vwrite_user(user, "~FG%s:~RS %s\n", u->name, inpstr);
			}
		}
	else {
		if ((u=get_user_name(user, word[1]))==NULL) {
			write_user(user, notloggedon);
			return;
			}
		if (u==user) {
			write_user(user, "Posielat HUG sebe ? Salies ?\n");
			return;
			}
		if ((check_igusers(u, user)!=-1)
		    && (user->level<GOD
			|| user->level<u->level
			)
		    ) {
			vwrite_user(user,"~FG~OL%s~FW ta momentalne ignoruje.\n", u->name);
			return;
			}
		if (u->ignore.all && (user->level<ARCH || u->level>user->level)) {
			if (u->malloc_start)
				vwrite_user(user, "~FG~OL%s~FW momentalne nieco pise\n", u->name);
			else
				vwrite_user(user, "~FG~OL%s~FW momentalne vsetko ignoruje\n", u->name);
			return;
			}
	#ifdef NETLINKS
		if (u->room==NULL) {
			vwrite_user(user, "~FG~OL%s~FW je momentalne mimo sajty, nemoze ti odpovedat\n", u->name);
			return;
			}
	#endif
		if (!u->ignore.funs && !u->afk) {
			show_file(u, HUGFILE);
			write_user(u, "\n");
			}
		if (u->afk) {
			if (word_count==2) sprintf(text, "~OL~FG%s~FW ta pozdravuje !\n", user->name);
			else sprintf(text, "~OL~FG%s~FW ta pozdravuje:~RS %s\n", user->name, inpstr);
			record_afk(u, text);
			}
		else vwrite_user(u, "~OL~FG%s~FW ta pozdravuje !\n", user->name);
		if (word_count>2) {
				if (!u->afk) vwrite_user(u, "A sepka ti: %s\n", inpstr);
			sprintf(text, "~FG~OLHUG~RS od %s:~RS %s\n", user->name, inpstr);
			}
		else sprintf(text, "~FG~OLHUG~RS od %s\n", user->name);
		record_tell(u, text);
		if (word_count==2) vwrite_user(user, "~FG~OLHUG~FW %s OK\n", u->name);
		else vwrite_user(user, "~OL~FGHUG~FW %s:~RS %s\n", u->name, inpstr);
		}
}


void kiss(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	int mur, i;

	set_crash();
	if (word_count<2) {
		write_usage(user, "%s <user> [<text>]", command_table[KISS].name);
		return;
		}
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "pusinkovat");
		return;
		}
	mur=count_musers(user, word[1]);
	inpstr=remove_first(inpstr);
	if (mur>1) {
		for (i=0; i<mur; i++) {
			if ((u=get_user_name(user, user->murlist[i]))==NULL) {
				vwrite_user(user, "~OL~FR%s~RS: %s",
					user->murlist[i], notloggedon);
				continue;
				}
			if (u==user) {
				vwrite_user(user, "~OL~FR%s~RS: ~OLseba ? hmm ...\n",
					user->murlist[i]);
				continue;
				}
			if ((check_igusers(u, user))!=-1
			    && (user->level<GOD
				|| user->level<u->level
				)
			    ) {
				vwrite_user(user,"~OL~FR%s~RS : momentalne ta ignoruje.\n",
					user->murlist[i]);
				continue;
				}
			if ((u->ignore.all || u->ignore.tells) && (user->level<ARCH || u->level>user->level)) {
				vwrite_user(user,"~OL~FR%s~RS : momentalne ignoruje.\n",
					user->murlist[i]);
				continue;
				}
		#ifdef NETLINKS
			if (u->room==NULL) {
				vwrite_user(user,"~FR~OL%s~RS : je momentalne mimo sajty, nemoze odpovedat.\n", u->name);
				continue;
				}
		#endif
			if (!u->ignore.funs) {
				show_file(u, KISSFILE);
				write_user(u, "\n");
				}
			if (u->afk) {
				if (word_count==2) sprintf(text, "~OL~FG%s~FR ta bozkava:~RS %s\n", user->name, inpstr);
				else sprintf(text, "~OL~FG%s~FR ta bozkava !\n", user->name);
				record_afk(u, text);
				}
			else vwrite_user(u, "~OL~FG%s~FW ta bozkava !\n", user->name);
			if (word_count>2) {
					if (!u->afk) vwrite_user(u, "A sepka ti:~RS %s\n", inpstr);
				sprintf(text, "~FR~OLKISS~RS od %s:~RS %s\n", user->name, inpstr);
				}
			else sprintf(text, "~FR~OLKISS~RS od %s\n", user->name);
			record_tell(u, text);
			if (word_count==2) vwrite_user(user, "~FG%s ~OLOK\n", u->name);
			else vwrite_user(user, "~FG%s:~RS %s\n", u->name, inpstr);
			}
		}
	else {
		if ((u=get_user_name(user, word[1]))==NULL) {
			write_user(user, notloggedon);
			return;
			}
		if (u==user) {
			write_user(user, "Posielat KISS sebe ? Salies ?\n");
			return;
			}
		if ((check_igusers(u, user)!=-1)
		    && (user->level<GOD
			|| user->level<u->level
			)
		    ) {
			vwrite_user(user,"~FG~OL%s~FW ta momentalne ignoruje.\n", u->name);
			return;
			}
		if (u->ignore.all && (user->level<ARCH || u->level>user->level)) {
			if (u->malloc_start)
				vwrite_user(user, "~FG~OL%s~FW momentalne nieco pise\n", u->name);
			else
				vwrite_user(user, "~FG~OL%s~FW momentalne vsetko ignoruje\n", u->name);
			return;
			}
	#ifdef NETLINKS
		if (u->room==NULL) {
			vwrite_user(user, "~FG~OL%s~FW je momentalne mimo sajty, nemoze ti odpovedat\n", u->name);
			return;
			}
	#endif
		if (!u->ignore.funs && !u->afk) {
			show_file(u, KISSFILE);
			write_user(u, "\n");
			}
		if (u->afk) {
			if (word_count==2) sprintf(text, "~OL~FR%s~FW ta bozkava !\n", user->name);
			else sprintf(text, "~OL~FR%s~FW ta bozkava:~RS %s\n", user->name, inpstr);
			record_afk(u, text);
			}
		else vwrite_user(u, "~OL~FR%s~FW ta bozkava !\n", user->name);
		if (word_count>2) {
				if (!u->afk) vwrite_user(u, "A sepka ti: %s\n", inpstr);
			sprintf(text, "~FR~OLKISS~RS od %s:~RS %s\n", user->name, inpstr);
			}
		else sprintf(text, "~FR~OLKISS~RS od %s\n", user->name);
		record_tell(u, text);
		if (word_count==2) vwrite_user(user, "~FR~OLKISS~FW %s OK\n", u->name);
		else vwrite_user(user, "~OL~FRKISS~FW %s:~RS %s\n", u->name, inpstr);
		}
}


void reply(UR_OBJECT user, char *inpstr)
{
	
	UR_OBJECT u;
	char type[15],*name;

	set_crash();
	if (user->ltell[0]=='\0') {
		write_user(user, "Este ti nikto netelloval\n");
		return;
		}
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kecat");
		return;
		}
	if (word_count<2) {	
		vwrite_user(user, "~OLPosledny tell od: ~FG%s~RS\n", user->ltell);
		return;
		}
	u=get_user_name(user,user->ltell);

	if (!u) {
		write_user(user,notloggedon);
		return;
		}
	if ((check_igusers(u,user))!=-1 && user->level<GOD) {
		vwrite_user(user,"%s~RS is ignoring tells from you.\n",u->recap);
		return;
		}
	if (u->ignore.tells && (user->level<ARCH || u->level>user->level)) {
		vwrite_user(user,"%s~RS is ignoring tells at the moment.\n",u->recap);
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
if (u->afk) {
	if (u->afk_mesg[0])
		vwrite_user(user,"%s~RS is ~FRAFK~RS, message is: %s\n",
			u->recap,u->afk_mesg);
	else vwrite_user(user,"%s~RS is ~FRAFK~RS at the moment.\n",u->recap);
  write_user(user,"Sending message to their afk review buffer.\n");
  if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
  else strcpy(type,"tell");
  if (user->vis || u->level>=user->level) name=user->bw_recap;
  else name=invisname;
  sprintf(text,"~FG~OL%s %ss you:~RS %s\n",name,type,inpstr);
  record_afk(u,text);
  sprintf(u->ltell, user->name);
  return;
  }
if (u->editing) {
  vwrite_user(user,"%s~RS is in ~FTEDIT~RS mode at the moment (using the line editor).\n",u->recap);
  write_user(user,"Sending message to their edit review buffer.\n");
  if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
  else strcpy(type,"tell");
  if (user->vis || u->level>=user->level) name=user->bw_recap;
  else name=invisname;
  sprintf(text,"~FG~OL%s %ss you:~RS %s\n",name,type,inpstr);
  record_edit(u,text);
  sprintf(u->ltell, user->name);
  return;
  }
if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
  if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS is using the editor at the moment.\n",u->recap);
  else vwrite_user(user,"%s~RS is ignoring everyone at the moment.\n",u->recap);
  return;
  }
#ifdef NETLINKS
  if (u->room==NULL) {
    vwrite_user(user,"%s~RS is offsite and would not be able to reply to you.\n",u->recap);
    return;
    }
#endif
if (inpstr[strlen(inpstr)-1]=='?') strcpy(type,"ask");
else strcpy(type,"tell");
sprintf(text,"~OL~FTYou %s %s:~RS %s\n",type,u->bw_recap,inpstr);
write_user(user,text);
record_tell(user,text);
if (user->vis || u->level>=user->level) name=user->bw_recap; else name=invisname;
sprintf(text,"~FT%s %ss you:~RS %s\n",name,type,inpstr);
write_user(u,text);
record_tell(u,text);
sprintf(u->ltell, user->name);
}


void shoutto(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char *name;

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kricat");
		return;
		}
	if (word_count<2) {
		write_user(user,"Kricat komu co ?\n");
		return;
		}
	if (word_count<3) {
		write_user(user, "Kricat co ?\n");
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
	u=get_user_name(user, word[1]);
	inpstr=remove_first(inpstr);
	if (!u) {
		write_user(user, notloggedon);
		return;
		}
	if (u==user) {
		vwrite_user(user,"Salen%s ? Kricat sam%s na seba ?\n",
			grm_gnd(1, user->gender), grm_gnd(4, user->gender)
			);
		return;
		}
	if ((check_igusers(u,user))!=-1 && user->level<GOD) {
		vwrite_user(user,"%s~RS ignoruje hlasky od teba.\n", u->recap);
		return;
		}
	if (u->ignore.shouts && (user->level<WIZ || u->level>user->level)) {
		vwrite_user(user,"%s~RS momentalne ignoruje vykriky.\n", u->recap);
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
	if (u->afk) {
		if (u->afk_mesg[0]) vwrite_user(user,"%s~RS je ~FRAFK~RS, odkaz: %s\n", u->recap, u->afk_mesg);
		else vwrite_user(user,"%s~RS je momentalne ~FRAFK~RS.\n", u->recap);
		return;
		}
	if (u->editing) {
		vwrite_user(user,"%s~RS je v ~FTEDIT~RS mode.\n", u->recap);
		write_user(user,"Sending message to their edit review buffer.\n");
		return;
		}
	if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
		if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS momentalne pracuje v editore.\n", u->recap);
		else vwrite_user(user,"%s~RS momentalne vsetko ignoruje.\n", u->recap);
		return;
		}
#ifdef NETLINKS
	if (u->room==NULL) {
		vwrite_user(user,"%s~RS je momentalne mimo sajty a preto ti nemoze odpovedat.\n",u->recap);
		return;
		}
#endif
	vwrite_user(user,"~OL~FYKricis na %s:~RS %s\n", u->nameg, inpstr);
	if (user->vis) name=user->bw_recap; else name=invisname;
	if (!user->vis) write_monitor(user,NULL,0);
	sprintf(text,"~OL~FY%s krici na %s:~RS %s\n", name, u->nameg, inpstr);
	write_room_except2(NULL, text, user, u);
	vwrite_user(u, "~OL~FY%s ti krici:~RS %s\n", name, inpstr);
	record_shout(text);
}


void tellall(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char type[15], *name, tmp[200];

	set_crash();
	if (user->muzzled) {
		vwrite_user(user, muzzled_cannot, "kecat");
		return;
		}
	if (word_count<2) {
		write_user(user, "Co tellnut ?\n");
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
	for (u=user_first; u!=NULL; u=u->next) {
		if (u==user) continue;
		if (u->type==CLONE_TYPE) continue;
		if ((check_igusers(u,user))!=-1 && user->level<GOD) continue;
		if (u->ignore.tells && (user->level<WIZ || u->level>user->level)) continue;
		if (u->afk) {
			if (inpstr[strlen(inpstr)-1]=='?')
				strcpy(type,"sa ta pyta");
			else strcpy(type,"ti telluje");
			if (user->vis || u->level>=user->level)
				name=user->bw_recap;
			else name=invisname;
			if (u->level>=user->level)
				sprintf(text, "~OLFT>> ");
			sprintf(tmp, "~FG~OL%s %s:~RS %s\n", name, type, inpstr);
			strcat(text, tmp);
			record_afk(u,text);
			sprintf(u->ltell, user->name);
			continue;
			}
		if (u->editing) {
			if (inpstr[strlen(inpstr)-1]=='?')
				strcpy(type,"sa ta pyta");
			else strcpy(type,"ti telluje");
			if (user->vis || u->level>=user->level)
				name=user->bw_recap;
			else name=invisname;
			if (u->level>=user->level)
				sprintf(text,"~OL~FT>> ");
			sprintf(tmp, "~OL~FG%s %s:~RS %s\n",name, type, inpstr);
			strcat(text, tmp);
			record_edit(u,text);
			sprintf(u->ltell, user->name);
			}
		if (u->ignore.all && (user->level<WIZ || u->level>user->level)) continue;
		#ifdef NETLINKS
		if (u->room==NULL) continue;
		#endif
		if (inpstr[strlen(inpstr)-1]=='?')
			strcpy(type,"sa ta pyta");
		else strcpy(type,"ti telluje");
		if (user->vis || u->level>=user->level)
			name=user->bw_recap;
		else name=invisname;
		if (u->level>=user->level)
			sprintf(text,"~OL~FT>> ");
		sprintf(tmp, "~FT%s %s:~RS %s\n", name, type, inpstr);
		strcat(text,tmp);
		write_user(u,text);
		record_tell(u,text);
		sprintf(u->ltell, user->name);
		} /* koniec for */
	sprintf(text,"~OL~FT>> tellall:~RS %s\n", inpstr);
	write_user(user,text);
	record_tell(user, text);
	
}


void kick(UR_OBJECT user)
{
	UR_OBJECT ur;
	RM_OBJECT rm=NULL;
	int i;

	set_crash();
	for (i=0; i<MAX_LINKS; ++i) {
		if (user->room->link[i]) {
			if (user->room->link[i]->access==PUBLIC
			    || user->room->link[i]->access==FIXED_PUBLIC
			    ) {
				rm=user->room->link[i];
				break;
				}
			}
		}
	if (!rm) {
		if (!(rm=get_room(default_warp))) {
			write_user(user, "Smola, ale nemas odtialto kde vykopnut ...\n");
			return;
			}
		}
	if ((ur=get_user_name(user, word[1]))==NULL) {
		write_user(user, notloggedon);
		return;
		}
	if (ur->level>=user->level) {
		write_user(user, "Nemozes vykopnut usera s rovnakym alebo vyssim levelom ako mas ty !\n");
		return;
		}
	if (ur->room!=user->room) {
		vwrite_user(user, "Ved %s nie je v tejto miestnosti !\n", ur->name);
		return;
		}
	move_user(ur, rm, 0);
	if (user->vis) vwrite_user(ur, "~OL~FG%s~FW ta vykop%s z miestnosti ~FY%s~FW !\n",
		user->name, grm_gnd(5, user->gender), user->room->name);
	else vwrite_user(ur, "~OL~FG%s~FW ta vykopol z miestnosti ~FY%s~FW !\n",
		invisname, user->room->name);
	vwrite_user(user, "~OLVykop%s si ~FG%s~FW odtialto ...\n", grm_gnd(5, user->gender), ur->name);
}

#endif /* ct_social.c */
