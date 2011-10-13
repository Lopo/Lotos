/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                    Funkcie Lotos v1.2.0 suvisiace s klonmi
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_CLONE_C__
#define __CT_CLONE_C__ 1

#include <time.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_sys.h"
#include "ct_clone.h"


/*** Clone a user in another room ***/
void create_clone(UR_OBJECT user)
{
	UR_OBJECT u;
	RM_OBJECT rm;
	char *name;
	int cnt;

	set_crash();
	if (user->restrict[RESTRICT_CLON]==restrict_string[RESTRICT_CLON]) {
		write_user(user,">>>You have no right to use this command! Sorry...\n");
		return;
		}
/* Check room */
if (word_count<2) rm=user->room;
else {
  if ((rm=get_room(word[1]))==NULL) {
    write_user(user,nosuchroom);
    return;
    }
  }	
/* If room is private then nocando */
if (!has_room_access(user,rm)) {
  write_user(user,"That room is currently private, you cannot create a clone there.\n");  
  return;
  }
if (rm->transp!=NULL && user->level<ARCH) {
	write_user(user, "Nemozes si robit klony v transportoch !\n");
	return;
	}
/* Count clones and see if user already has a copy there , no point having 
   2 in the same room */
cnt=0;
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE && u->owner==user) {
    if (u->room==rm) {
      vwrite_user(user,"You already have a clone in the %s.\n",rm->name);
      return;
      }	
    if ((++cnt==amsys->max_clones) && (user->level<GOD)) {
      write_user(user,"You already have the maximum number of clones allowed.\n");
      return;
      }
    }
  }
/* Create clone */
if ((u=create_user())==NULL) {		
  vwrite_user(user,"%s: Unable to create copy.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create user copy in clone().\n");
  return;
  }
u->type=CLONE_TYPE;
u->socket=user->socket;
u->room=rm;
u->owner=user;
u->vis=1;
strcpy(u->name,user->name);
strcpy(u->recap,u->name);
strcpy(u->bw_recap,colour_com_strip(u->recap));
strcpy(u->desc, clone_desc);
if (rm==user->room)
	write_user(user, clone_here_prompt);
else
	vwrite_user(user, clone_prompt, rm->name);
if (user->vis) name=user->name; else name=invisname;
vwrite_room_except(user->room, user, "~FB~OL%s waves their hands...\n",name);
vwrite_room_except(rm, user, "~FB~OLA clone of %s appears in a swirling magical mist!\n",user->bw_recap);
}


/*** Destroy user clone ***/
void destroy_clone(UR_OBJECT user)
{
UR_OBJECT u,u2;
RM_OBJECT rm;
char *name;

	set_crash();
/* Check room and user */
if (word_count<2) rm=user->room;
else {
  if ((rm=get_room(word[1]))==NULL) {
    write_user(user,nosuchroom);  return;
    }
  }
if (word_count>2) {
  if ((u2=get_user_name(user,word[2]))==NULL) {
    write_user(user,notloggedon);  return;
    }
  if (u2->level>=user->level) {
    write_user(user,"You cannot destroy the clone of a user of an equal or higher level.\n");
    return;
    }
  }
else u2=user;
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE && u->room==rm && u->owner==u2) {
    destruct_user(u);
    reset_access(rm);
    write_user(user, clone_user_destroy);
    if (user->vis) name=user->bw_recap; else name=invisname;
    vwrite_room_except(user->room,user,clone_room_destroy1, name);
    vwrite_room(rm, clone_room_destroy2, u2->bw_recap);
    if (u2!=user) vwrite_user(u2,"~OLSYSTEM: ~FR%s has destroyed your clone in the %s.\n",user->bw_recap,rm->name);
    destructed=0;
    return;
    }
  }
if (u2==user) vwrite_user(user,"You do not have a clone in the %s.\n",rm->name);
else vwrite_user(user,"%s~RS does not have a clone the %s.\n",u2->recap,rm->name);
}


/*** Show users own clones ***/
void myclones(UR_OBJECT user)
{
	UR_OBJECT u;
	int cnt;

	set_crash();
	if (user->restrict[RESTRICT_CLON]==restrict_string[RESTRICT_CLON]) {
		write_user(user,">>>You have no right to use this command! Sorry...\n");
		return;
		}
	cnt=0;
	for(u=user_first;u!=NULL;u=u->next) {
		if (u->type!=CLONE_TYPE || u->owner!=user) continue;
		if (++cnt==1) write_user(user,"\n~BB*** Rooms you have clones in ***\n\n");
		vwrite_user(user,"  %s\n",u->room->name);
		}
	if (!cnt) write_user(user,"You have no clones.\n");
	else vwrite_user(user,"\nTotal of ~OL%d~RS clone%s.\n\n",cnt,PLTEXT_S(cnt));
}


/*** Show all clones on the system ***/
void allclones(UR_OBJECT user)
{
	UR_OBJECT u;
	int cnt;

	set_crash();
	if (user->restrict[RESTRICT_CLON]==restrict_string[RESTRICT_CLON]) {
		write_user(user,">>>You have no right to use this command! Sorry...\n");
		return;
		}
	cnt=0;
	for(u=user_first;u!=NULL;u=u->next) {
		if (u->type!=CLONE_TYPE) continue;
		if (++cnt==1) vwrite_user(user,"\n~BB*** Current clones %s ***\n\n",long_date(1));
		vwrite_user(user, all_clone_style,u->name,u->room->name);
		}
	if (!cnt) write_user(user,"There are no clones on the system.\n");
	else vwrite_user(user,"\nTotal of ~OL%d~RS clone%s.\n\n",cnt,PLTEXT_S(cnt));
}


/*** User swaps places with his own clone. All we do is swap the rooms the
	objects are in. ***/
void clone_switch(UR_OBJECT user)
{
UR_OBJECT u,tu;
RM_OBJECT rm;
int cnt=0;

	set_crash();
/* if no room was given then check to see how many clones user has.  If 1, then
   move the user to that clone, else give an error
*/
tu=NULL;
rm=NULL;
if (word_count<2) {
  for(u=user_first;u!=NULL;u=u->next) {
    if (u->type==CLONE_TYPE && u->owner==user) {
      if (++cnt>1) {
	write_user(user,"You have more than one clone active.  Please specify a room to switch to.\n");
	return;
        }
      rm=u->room;
      tu=u;
      }
    }
  if (!cnt) {
    write_user(user,"You do not currently have any active clones.\n");
    return;
    }
  write_user(user, clone_switch_prompt);
  tu->room=user->room;
  user->room=rm;
  vwrite_room_except(user->room,user,"The clone of %s~RS comes alive!\n",user->recap);
  vwrite_room_except(tu->room,tu,"%s~RS turns into a clone!\n",tu->recap);
  look(user);
  return;
  }

/* if a room name was given then try to switch to a clone there */

if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
 }
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) {
    write_user(user, clone_switch_prompt);
    u->room=user->room;
    user->room=rm;
    vwrite_room_except(user->room,user,"The clone of %s comes alive!\n",u->name);
    vwrite_room_except(u->room,u,"%s~RS turns into a clone!\n",u->recap);
    look(user);
    return;
    }
  }
write_user(user,"You do not have a clone in that room.\n");
}


/*** Make a clone speak ***/
void clone_say(UR_OBJECT user, char *inpstr)
{
	RM_OBJECT rm;
	UR_OBJECT u;

	set_crash();
	if (user->muzzled) {
		write_user(user,"You are muzzled, your clone cannot speak.\n");
		return;
		}
	if (word_count<3) {
		write_usage(user,"csay <room clone is in> <message>");
		return;
		}
	if ((rm=get_room(word[1]))==NULL) {
		write_user(user,nosuchroom);
		return;
		}
	for(u=user_first;u!=NULL;u=u->next) {
		if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) {
			say(u,remove_first(inpstr));
			return;
			}
		}
	write_user(user,"You do not have a clone in that room.\n");
}


/*** Set what a clone will hear, either all speach , just bad language
	or nothing. ***/
void clone_hear(UR_OBJECT user)
{
RM_OBJECT rm;
UR_OBJECT u;

	set_crash();
if (word_count<3  
    || (strcmp(word[2],"all") 
    && strcmp(word[2],"swears") 
    && strcmp(word[2],"nothing"))) {
  write_usage(user,"chear <room clone is in> all/swears/nothing");
  return;
  }
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) break;
  }
if (u==NULL) {
  write_user(user,"You do not have a clone in that room.\n");
  return;
  }
strtolower(word[2]);
if (!strcmp(word[2],"all")) {
  u->clone_hear=CLONE_HEAR_ALL;
  write_user(user,"Clone will now hear everything.\n");
  return;
  }
if (!strcmp(word[2],"swears")) {
  u->clone_hear=CLONE_HEAR_SWEARS;
  write_user(user,"Clone will now only hear swearing.\n");
  return;
  }
u->clone_hear=CLONE_HEAR_NOTHING;
write_user(user,"Clone will now hear nothing.\n");
}


/*** Make a clone emote ***/
void clone_emote(UR_OBJECT user,char *inpstr)
{
RM_OBJECT rm;
UR_OBJECT u;

	set_crash();
if (user->muzzled>1) {
  write_user(user,"You are muzzled, your clone cannot emote.\n");
  return;
  }
if (word_count<3) {
  write_usage(user,"cemote <room clone is in> <message>");
  return;
  }
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
for(u=user_first;u!=NULL;u=u->next) {
  if (u->type==CLONE_TYPE && u->room==rm && u->owner==user) {
    emote(u,remove_first(inpstr));  return;
    }
  }
write_user(user,"You do not have a clone in that room.\n");
}

#endif /* ct_clone.c */
