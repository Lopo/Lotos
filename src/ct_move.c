/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                  Funkcie Lotos v1.2.0 suvisiace s pohybom
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_MOVE_C__
#define __CT_MOVE_C__ 1

#include <stdio.h>
#include <unistd.h>
#include <time.h>
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
#include "obj_syspp.h"
#include "ct_move.h"
#include "comvals.h"


/*** Move to another room ***/
void go(UR_OBJECT user)
{
RM_OBJECT rm;
#ifdef NETLINKS
  NL_OBJECT nl;
#endif
int i;

	set_crash();
if (user->lroom==2) {
  write_user(user,"You have been shackled and cannot move.\n");
  return;
  }
if (user->restrict[RESTRICT_GO]==restrict_string[RESTRICT_GO]) {
	write_user(user,">>>You cannot have access to another sky...\n");
	return;
	}
if (word_count<2) {
  if (!(rm=get_room(default_warp))) {
    write_user(user,"You cannot warp to the main room at this time.\n");
    return;
    }
  if (user->room==rm) {
    vwrite_user(user,"You are already in the %s!\n",default_warp);
    return;
    }
  move_user(user,rm,1);
  follow(user);
  return;
  }
#ifdef NETLINKS
  nl=user->room->netlink;
  if (nl!=NULL && !strncmp(nl->service,word[1],strlen(word[1]))) {
    if (user->pot_netlink==nl) {
      write_user(user,"The remote service may be lagged, please be patient...\n");
      return;
      }
    rm=user->room;
    if (nl->stage<2) {
      write_user(user,"The netlink is inactive.\n");
      return;
      }
    if (nl->allow==IN && user->netlink!=nl) {
      /* Link for incoming users only */
      write_user(user,"Sorry, link is for incoming users only.\n");
      return;
      }
    /* If site is users home site then tell home system that we have removed
       him. */
    if (user->netlink==nl) {
      write_user(user,"~FB~OLYou traverse cyberspace...\n");
      sprintf(text,"REMVD %s\n",user->name);
      write_sock(nl->socket,text);
      if (user->vis) {
	sprintf(text,"%s~RS goes to the %s\n",user->recap,nl->service);
	write_room_except(rm,text,user);
        }
      else write_room_except(rm,invisleave,user);
      write_syslog(NETLOG,1,"NETLINK: Remote user %s removed.\n",user->name);
      destroy_user_clones(user);
      syspp->acounter[user->gender]--;
      syspp->acounter[3]--;
      destruct_user(user);
      reset_access(rm);
      no_prompt=1;
      return;
      }
    /* Can't let remote user jump to yet another remote site because this will 
       reset his user->netlink value and so we will lose his original link.
       2 netlink pointers are needed in the user structure to allow this
       but it means way too much rehacking of the code and I don't have the 
       time or inclination to do it */
    if (user->type==REMOTE_TYPE) {
      write_user(user,"Sorry, due to software limitations you can only traverse one netlink.\n");
      return;
      }
    if (nl->ver_major<=3 && nl->ver_minor<=3 && nl->ver_patch<1) {
      if (!word[2][0]) 
	sprintf(text,"TRANS %s %s %s\n",user->name,user->pass,user->desc);
      else sprintf(text,"TRANS %s %s %s\n",user->name,(char *)crypt(word[2],crypt_salt),user->desc);
      }
    else {
      if (!word[2][0]) 
	sprintf(text,"TRANS %s %s %d %s\n",user->name,user->pass,user->level,user->desc);
      else sprintf(text,"TRANS %s %s %d %s\n",user->name,(char *)crypt(word[2],crypt_salt),user->level,user->desc);
      }
    write_sock(nl->socket,text);
    user->remote_com=GO;
    user->pot_netlink=nl;  /* potential netlink */
    no_prompt=1;
    return;
    }
  /* If someone tries to go somewhere else while waiting to go to a talker
     send the other site a release message */
  if (user->remote_com==GO) {
    sprintf(text,"REL %s\n",user->name);
    write_sock(user->pot_netlink->socket,text);
    user->remote_com=-1;
    user->pot_netlink=NULL;
    }
#endif
if ((rm=get_room(word[1]))==NULL) {
  write_user(user,nosuchroom);  return;
  }
if (rm==user->room) {
  vwrite_user(user,"Ved uz si v %s!\n",rm->name);
  return;
  }
/* See if link from current room */
if (rm->transp!=NULL) {
	if ((!rm->transp->go) && rm->link[rm->transp->out]==user->room) {
		move_user(user, rm, 0);
		follow(user);
		return;
		}
	}
else
	for(i=0;i<MAX_LINKS;++i) {
		if (user->room->link[i]==rm) {
			move_user(user,rm,0);
			follow(user);
			return;
			}
		}
if (user->level<WIZ) {
  vwrite_user(user,"The %s is not adjoined to here.\n",rm->name);
  return;
  }
if (rm->access==PERSONAL_UNLOCKED || rm->access==PERSONAL_LOCKED) {
  write_user(user,"To go to another user's home you must .visit them.\n");
  return;
  }
move_user(user,rm,1);
follow(user);
}



/*** Wizard moves a user to another room ***/
void s_move(UR_OBJECT user)
{
	UR_OBJECT u;
	RM_OBJECT rm;
	char *name;
	int i, mur=0;

	set_crash();
	if (word_count<2) {
		write_usage(user,"%s <user> [<room>]", command_table[MOVE].name);  return;
		}
	if (word_count<3) rm=user->room;
	else {
		if ((rm=get_room(word[2]))==NULL) {
			write_user(user,nosuchroom);
			return;
			}
		}

	if ((mur=count_musers(user, word[1]))<2) {
		if (!(u=get_user_name(user,word[1]))) {
			write_user(user,notloggedon);
			return;
			}
		if (user==u) {
			write_user(user,"Trying to move yourself this way is the fourth sign of madness.\n");
			return;
			}
		if (u->level>=user->level) {
			write_user(user,"You cannot move a user of equal or higher level than yourself.\n");
			return;
			}
		if (rm==u->room) {
			vwrite_user(user,"%s~RS is already in the %s.\n",u->recap,rm->name);
			return;
			};
		if (u->restrict[RESTRICT_MOVE]==restrict_string[RESTRICT_MOVE]) {
			write_user(user,">>>This user is unmoveable...\n");
			return;
			}
		if (!has_room_access(user,rm)) {
			vwrite_user(user,"The %s is currently private, %s~RS cannot be moved there.\n",rm->name,u->recap);
			return;
			}
		write_user(user, move_prompt_user);
		if (user->vis) name=user->bw_recap;
		else name=invisname;
		vwrite_room_except(user->room,user, user_room_move_prompt, name);
		move_user(u,rm,2);
		prompt(u);
		}

	for (i=0; i<mur; i++) {
		if (!(u=get_user_name(user, user->murlist[i]))) {
			vwrite_user(user, "~FR~OL%s~RS : %s",
				user->murlist[i], notloggedon);
			continue;
			}
		if (user==u) {
			vwrite_user(user, "~FR~OL%s~RS : seba ? hmm...\n", user->murlist[i]);
			continue;
			}
		if (u->level>=user->level) {
			vwrite_user(user,"~FR~OL%s~RS : You cannot move a user of equal or higher level than yourself.\n", user->murlist[i]);
			continue;
			}
		if (rm==u->room) {
			vwrite_user(user,"~FR~OL%s~RS : already in the %s.\n",
				user->murlist[i], rm->name);
			continue;
			};
		if (u->restrict[RESTRICT_MOVE]==restrict_string[RESTRICT_MOVE]) {
			vwrite_user(user,"~FR~OL%s~RS : >>>This user is unmoveable...\n");
			continue;
			}
		if (!has_room_access(user,rm)) {
			vwrite_user(user,"The %s is currently private, %s~RS cannot be moved there.\n",rm->name,u->recap);
			return;
			}
		vwrite_user(user, "~FR~OL%s~RS : %s",
			user->murlist[i], move_prompt_user);
		if (user->vis) name=user->bw_recap;
		else name=invisname;
		if (!i) vwrite_room_except(user->room,user, user_room_move_prompt, name);
		move_user(u,rm,2);
		prompt(u);
		}
}


/*** Join a user in another room ***/
void join(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user>", command_table[JOIN].name);
  return;
  }
if (user->lroom==2) {
  write_user(user,"You have been shackled and cannot move.\n");
  return;
  }
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (user==u) {
  write_user(user,"You really want join yourself?  What would the neighbours think?\n");
  return;
  }
rm=u->room;
#ifdef NETLINKS
  if (rm==NULL) {
    vwrite_user(user,"%s is currently off site so you cannot join them.\n",u->name);
    return;
    }
#endif
if (rm==user->room) {
  vwrite_user(user,"You are already with %s~RS in the %s.\n",u->recap,rm->name);
  return;
  };
if (!has_room_access(user,rm)) {
  vwrite_user(user,"That room is currently private, you cannot join %s~RS there.\n",u->recap);
  return;
  }
if (user->vis) name=user->recap;
else name=invisname;
vwrite_user(user,"~FT~OLYou join %s in the %s.~RS\n",u->bw_recap,u->room->name);
if (user->level<GOD || user->vis) {
  vwrite_room_except(user->room,user,"%s~RS %s\n",name,user->out_phrase);
  vwrite_room_except(rm,user,"%s~RS %s\n",name,user->in_phrase);
  }
user->room=rm;
look(user);
}


/*** bring a user to the same room ***/
void bring(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;

	set_crash();
if (word_count<2) {
  write_usage(user,"%s <user>", command_table[BRING].name);
  return;
  }
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
rm=user->room;
if (user==u) {
  write_user(user,"You ~OLreally~RS want to bring yourself?!  What would others think?!\n");
  return;
  }
if (rm==u->room) {
  vwrite_user(user,"%s~RS is already here!\n",u->recap);
  return;
  }
if (u->level>=user->level && user->level<GOD) {
  write_user(user,"You cannot move a user of equal or higher level that yourself.\n");
  return;
  }
write_user(user,"You chant a mystic spell...\n");
if (user->vis) vwrite_room_except(user->room,user,"%s~RS chants a mystic spell...\n",user->recap);
else {
  write_monitor(user,user->room,0);
  write_room_except(user->room,"Someone chants a mystic spell...\n",user);
  }
move_user(u,rm,2);
prompt(u);
}


/* lets a user enter their own room.  It creates the room if !exists */
void personal_room(UR_OBJECT user) {
char name[ROOM_NAME_LEN+1],filename[500];
RM_OBJECT rm;
int pcnt=0;

if (!amsys->personal_rooms) {
  write_user(user,"Personal room functions are currently disabled.\n");
  return;
  }
sprintf(name,"(%s)",user->name);
strtolower(name);
/* if the user wants to delete their room */
if (word_count>=2) {
  if (strcmp(word[1],"-d")) {
    write_usage(user,"%s [-d]", command_table[MYROOM].name);
    return;
    }
  /* move to the user out of the room if they are in it */
  if ((rm=get_room_full(name))==NULL) {
    write_user(user,"You do not have a personal room built.\n");
    return;
    }
  pcnt=room_visitor_count(rm);
  if (pcnt) {
    write_user(user,"You cannot destroy your room if any people are in it.\n");
    return;
    }
  write_user(user,"~OL~FRYou whistle a sharp spell and watch your room crumble into dust.~RS\n");
  destruct_room(rm);
  /* delete the files */
  sprintf(filename,"%s/%s.R", USERROOMS,user->name);
  unlink(filename);
  sprintf(filename,"%s/%s.B", USERROOMS,user->name);
  unlink(filename);
  sprintf(filename,"%s/%s.K", USERROOMS,user->name);
  unlink(filename);
  write_syslog(SYSLOG,1,"%s destructed their personal room.\n",user->name);
  return;
  }
/* if the user is moving to their room */
if (user->lroom==2) {
  write_user(user,"You have been shackled and cannot move.\n");
  return;
  }
/* if room doesn't exist then create it */
if ((rm=get_room_full(name))==NULL) {
  if ((rm=create_room())==NULL) {
    write_user(user,"Sorry, but your room could not be created at this time.\n");
    write_syslog(ERRLOG,1,"Could not create room for in personal_room()\n");
    return;
    }
  write_user(user,"\nYour room doesn't exists.  Building it now...\n\n");
  /* set up the new rooms attributes.  We presume that one room has already been parsed
     and that is the room everyone logs onto, and so we link to that */
  strcpy(rm->name,name);
  rm->access=PERSONAL_UNLOCKED;
  rm->link[0]=room_first;
  /* check to see if the room was just unloaded from memory first */
  if (!(personal_room_store(user->name,0,rm))) {
    strcpy(rm->desc,default_personal_room_desc);
    strcpy(rm->topic,"Welcome to my room!");
    write_syslog(SYSLOG,1,"%s creates their own room.\n",user->name);
    if (!personal_room_store(user->name,1,rm)) {
      write_syslog(ERRLOG,1,"Unable to save personal room status in personal_room_decorate()\n");
      }
    }
  }
/* if room just created then shouldn't go in his block */
if (user->room==rm) {
  write_user(user,"You are already in your own room!\n");
  return;
  }
move_user(user,rm,1);
}


/* lets a user go into another user's personal room if it's unlocked */
void personal_room_visit(UR_OBJECT user) {
char name[ROOM_NAME_LEN+1];
RM_OBJECT rm;

if (word_count<2) {
  write_usage(user,"%s <name>", command_table[VISIT].name);
  return;
  }
if (!amsys->personal_rooms) {
  write_user(user,"Personal room functions are currently disabled.\n");
  return;
  }
/* check if not same user */
word[1][0]=toupper(word[1][0]);
if (!strcmp(user->name,word[1])) {
  vwrite_user(user,"To go to your own room use the '%s' command.\n",command_table[MYROOM].name);
  return;
  }
/* see if there is such a user */
if (!find_user_listed(word[1])) {
  write_user(user,nosuchuser);
  return;
  }
/* get room to go to */
sprintf(name,"(%s)",word[1]);
strtolower(name);
if ((rm=get_room_full(name))==NULL) {
  write_user(user,nosuchroom);
  return;
  }
/* can they go there? */
if (!has_room_access(user,rm)) {
  write_user(user,"That room is currently private, you cannot enter.\n");  
  return;
  }
move_user(user,rm,1);
}

#endif /* ct_move.c */
