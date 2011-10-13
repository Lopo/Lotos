/* vi: set ts=4 sw=4: ai*/
/*****************************************************************************
                Funkcie Lotos v1.2.0 suvisiace s ignoraciami
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_IGNORE_C__
#define __CT_IGNORE_C__ 1

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "ct_ignore.h"
#include "comvals.h"
#include "ignval.h"



/*** Switch ignoring all on and off ***/
void toggle_ignall(UR_OBJECT user)
{
	set_crash();
	if (!user->ignore.all) {
		write_user(user,"You are now ignoring everyone.\n");
		if (user->vis) vwrite_room_except(user->room,user,"%s~RS is now ignoring everyone.\n",user->recap);
		else vwrite_room_except(user->room,user,"%s~RS is now ignoring everyone.\n",invisname);
		user->ignore.all=1;
		return;
		}
	write_user(user,"You will now hear everyone again.\n");
	if (user->vis) vwrite_room_except(user->room,user,"%s~RS is listening again.\n",user->recap);
	else vwrite_room_except(user->room,user,"%s~RS is listening again.\n",invisname);
	user->ignore.all=0;
}


/* displays what the user is currently listening to/ignoring */
void show_ignlist(UR_OBJECT user)
{
	set_crash();
write_user(user,"+----------------------------------------------------------------------------+\n");
if (user->ignore.all) {
  write_user(user,"| Momentalne ignorujes ~OL~FRvsetko~RS                                                |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  return;
  }
vwrite_user(user,"| Ignoring shouts   : ~OL%-3s~RS    Ignoring tells  : ~OL%-3s~RS    Ignoring logons : ~OL%-3s~RS  |\n",
	noyes2[user->ignore.shouts],noyes2[user->ignore.tells],noyes2[user->ignore.logons]);
vwrite_user(user,"| Ignoring pictures : ~OL%-3s~RS    Ignoring greets : ~OL%-3s~RS    Ignoring beeps  : ~OL%-3s~RS  |\n",
	noyes2[user->ignore.pics],noyes2[user->ignore.greets],noyes2[user->ignore.beeps]);
if (user->level>=WIZ) {
	vwrite_user(user,"| Ignoring wiztells : ~OL%-3s~RS    Ign. transports : ~OL%-3s~RS    Ignoring funs   : ~OL%-3s~RS  |\n",
  		noyes2[user->ignore.wiz], noyes2[user->ignore.transp], noyes2[user->ignore.funs]);
  }
else {
	vwrite_user(user,"| Ign. transports   : ~OL%-3s~RS    Ignoring funs   : ~OL%-3s~RS                           |\n",
  		noyes2[user->ignore.transp], noyes2[user->ignore.funs]);
  }
write_user(user,"+----------------------------------------------------------------------------+\n\n");
}


/*** set to ignore/listen to a user ***/
void set_igusers(UR_OBJECT user)
{
	UR_OBJECT u;
	int i=0;

	set_crash();
	if (word_count<3) {
		show_igusers(user);
		return;
		}
	if (!(u=get_user_name(user,word[2]))) {
		write_user(user,notloggedon);
		return;
		}
	if (user==u) {
		write_user(user,"Nemozes ignorovat seba !\n");
		return;
		}
	for (i=0; i<MAX_IGNORES; ++i) {
		if (!strcmp(user->ignoreuser[i],u->name)) {
			vwrite_user(user,"You once again listen to %s\n",user->ignoreuser[i]);
			user->ignoreuser[i][0]='\0';
			return;
			}
		if (!user->ignoreuser[i][0]) {
			strcpy(user->ignoreuser[i],u->name);
			vwrite_user(user,"You will now ignore tells from %s\n",user->ignoreuser[i]);
			return;
			}
		}
	write_user(user,"You have ignored the maximum amount of users already.\n");
}


/*** Allows a user to listen to everything again ***/
void user_listen(UR_OBJECT user)
{
	int yes;

	set_crash();
	yes=0;
	if (user->ignore.all) {
		user->ignore.all=0;
		yes=1;
		}
	if (user->ignore.tells) {
		user->ignore.tells=0;
		yes=1;
		}
	if (user->ignore.shouts) {
		user->ignore.shouts=0;
		yes=1;
		}
	if (user->ignore.pics) {
		user->ignore.pics=0;
		yes=1;
		}
	if (user->ignore.logons) {
		user->ignore.logons=0;
		yes=1;
		}
	if (user->ignore.wiz) {
		user->ignore.wiz=0;
		yes=1;
		}
	if (user->ignore.greets) {
		user->ignore.greets=0;
		yes=1;
		}
	if (user->ignore.beeps) {
		user->ignore.beeps=0;
		yes=1;
		}
	if (user->ignore.transp) {
		user->ignore.transp=0;
		yes=1;
		}
	if (user->ignore.funs) {
		user->ignore.funs=0;
		yes=1;
		}
	if (yes) {
		write_user(user,"You listen to everything again.\n");
		if (user->vis) {
			vwrite_room_except(user->room,user,"%s is now listening to you all again.\n",user->name);
			}
		return;
		}
	write_user(user,"You are already listening to everything.\n");
}


void set_ign_word(UR_OBJECT user)
{
	char *pp;
	set_crash();
	if (word_count<3) {
		if (user->ign_word!=NULL)
			vwrite_user(user, "Teraz ignorujes slovo '%s'.\n", user->ign_word);
		else vwrite_user(user, "Teraz neignorujes ziadne slovo.\n");
		return;
		}
	if (user->ign_word) {
		if (!strcmp(word[2], "-cancel")) {
			free(user->ign_word);
			user->ign_word=NULL;
			write_user(user, "Odteraz neignorujes ziadne slovo.\n");
			return;
			}
		pp=(char *)realloc(user->ign_word, strlen(word[2])+1);
		if (pp!=NULL) {
			user->ign_word=NULL;
			vwrite_user(user, "Odteraz ignorujes slovo '%s'\n", word[2]);
			user->ign_word=pp;
			strcpy(user->ign_word, word[2]);
			}
		else {
			write_user(user, "~FR~OLCHYBA~RS: nemozem realokovat pamat !\n");
			write_user(user, "Doterajsie slovo nebolo zmenene !\n");
			write_syslog(ERRLOG, 1, "pri alokacii pamate v set_ign_word() pre %s, %s\n",
				user->name, word[3]);
			}
		return;
		}
	if (!strcmp(word[2], "off")) {
		write_user(user, "Ved neignorujes ziadne slovo !\n");
		return;
		}
	user->ign_word=strdup(word[2]);
	vwrite_user(user, "Odteraz ignorujes slovo '%s'.\n", user->ign_word);
}


void set_ignores(UR_OBJECT user)
{
	int i, ignattrval=-1;

	set_crash();
	if (word_count<2) goto IGN_JUMP;
	i=0;
	strtolower(word[1]);
	while (ignstr[i].type[0]!='*') {
		if (!strcmp(ignstr[i].type, word[1])) {
			ignattrval=i;
			break;
			}
		++i;
		}
IGN_JUMP:
	if (ignattrval==-1) {
		i=0;
		write_user(user, "Polozky, ktore mozes ignorovat:\n");
		while (ignstr[i].type[0]!='*') {
			if (i!=IGN_WIZ || user->level>=WIZ)
				vwrite_user(user, "~FT%s~RS : %s\n", ignstr[i].type, ignstr[i].desc);
			i++;
			}
		}
	write_user(user, "\n");
	switch (ignattrval) {
		case IGN_SHOW:
			show_ignlist(user);
			return;
		case IGN_ALL:
			toggle_ignall(user);
			return;
		case IGN_TELLS:
	 		switch(user->ignore.tells) {
				case 0: user->ignore.tells=1;
					write_user(user,"You will now ignore tells.\n");
					break; 
				case 1: user->ignore.tells=0;
					write_user(user,"You will now hear tells.\n");
					break;
				}
			return;
		case IGN_SHOUTS:
			switch(user->ignore.shouts) {
				case 0: user->ignore.shouts=1;
					write_user(user,"You will now ignore shouts.\n");
					break; 
				case 1: user->ignore.shouts=0;
					write_user(user,"You will now hear shouts.\n");
					break;
				}
			return;
		case IGN_PICS:
			switch(user->ignore.pics) {
				case 0: user->ignore.pics=1;
					write_user(user,"You will now ignore pictures.\n");
					break; 
				case 1: user->ignore.pics=0;
					write_user(user,"You will now see pictures.\n");
					break;
				}
			return;
		case IGN_LOGONS:
			switch(user->ignore.logons) {
				case 0: user->ignore.logons=1;
					write_user(user,"You will now ignore all logons and logoffs.\n");
					break;
				case 1: user->ignore.logons=0;
					write_user(user,"You will now see all logons and logoffs.\n");
					break;
				}
			return;
		case IGN_WIZ:
			if (user->level<WIZ) {
				write_user(user, "Nemas na to level !\n");
				return;
				}
			switch(user->ignore.wiz) {
				case 0: user->ignore.wiz=1;
					write_user(user,"You will now ignore all wiztells and wizemotes.\n");
					break;
				case 1: user->ignore.wiz=0;
					write_user(user,"You will now listen to all wiztells and wizemotes.\n");
					break;
				}
			return;
		case IGN_GREETS:
			switch(user->ignore.greets) {
				case 0: user->ignore.greets=1;
					write_user(user,"You will now ignore all greets.\n");
					break;
				case 1: user->ignore.greets=0;
					write_user(user,"You will now see all greets.\n");
					break;
				}
			return;
		case IGN_BEEPS:
			switch(user->ignore.beeps) {
				case 0: user->ignore.beeps=1;
					write_user(user,"You will now ignore all beeps from users.\n");
					break;
				case 1: user->ignore.beeps=0;
					write_user(user,"You will now hear all beeps from users.\n");
					break;
				}
			return;
		case IGN_TRANSP:
			switch(user->ignore.transp) {
				case 0: user->ignore.transp=1;
					write_user(user,"You will now ignore all transports.\n");
					break;
				case 1: user->ignore.transp=0;
					write_user(user,"You will now hear all transports.\n");
					break;
				}
			return;
		case IGN_USER:
			set_igusers(user);
			return;
		case IGN_WORD:
			set_ign_word(user);
			return;
			
		case IGN_FUNS:
	 		switch(user->ignore.funs) {
				case 0: user->ignore.funs=1;
					write_user(user,"You will now ignore funs.\n");
					break; 
				case 1: user->ignore.funs=0;
					write_user(user,"You will now hear funs.\n");
					break;
				}
			return;
		}
}

#endif /* ct_ignore.c */
