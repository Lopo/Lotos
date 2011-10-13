/*****************************************************************************
                Funkcie OS Star v1.1.0 suvisiace s ignoraciami
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

#include <time.h>
#include <string.h>

#include "define.h"
#include "ur_obj.h"
#include "ct_ignore.h"
#include "comvals.h"
#include "ignval.h"

/* */
UR_OBJECT get_user_name(UR_OBJECT user, char *i_name);


/*** Switch ignoring all on and off ***/
void toggle_ignall(UR_OBJECT user)
{
	if (!user->ignall) {
		write_user(user,"You are now ignoring everyone.\n");
		if (user->vis) vwrite_room_except(user->room,user,"%s~RS is now ignoring everyone.\n",user->recap);
		else vwrite_room_except(user->room,user,"%s~RS is now ignoring everyone.\n",invisname);
		user->ignall=1;
		return;
		}
	write_user(user,"You will now hear everyone again.\n");
	if (user->vis) vwrite_room_except(user->room,user,"%s~RS is listening again.\n",user->recap);
	else vwrite_room_except(user->room,user,"%s~RS is listening again.\n",invisname);
	user->ignall=0;
}


/* displays what the user is currently listening to/ignoring */
void show_ignlist(UR_OBJECT user)
{
write_user(user,"+----------------------------------------------------------------------------+\n");
if (user->ignall) {
  write_user(user,"| Momentalne ignorujes ~OL~FRvsetko~RS                                                |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  return;
  }
vwrite_user(user,"| Ignoring shouts   : ~OL%-3s~RS    Ignoring tells  : ~OL%-3s~RS    Ignoring logons : ~OL%-3s~RS  |\n",
	noyes2[user->ignshouts],noyes2[user->igntells],noyes2[user->ignlogons]);
vwrite_user(user,"| Ignoring pictures : ~OL%-3s~RS    Ignoring greets : ~OL%-3s~RS    Ignoring beeps  : ~OL%-3s~RS  |\n",
	noyes2[user->ignpics],noyes2[user->igngreets],noyes2[user->ignbeeps]);
if (user->level>=WIZ) {
	vwrite_user(user,"| Ignoring wiztells : ~OL%-3s~RS    Ign. transports : ~OL%-3s~RS    Ignoring funs   : ~OL%-3s~RS  |\n",
  		noyes2[user->ignwiz], noyes2[user->igntr], noyes2[user->ignfuns]);
  }
else {
	vwrite_user(user,"| Ign. transports   : ~OL%-3s~RS    Ignoring funs   : ~OL%-3s~RS                           |\n",
  		noyes2[user->igntr], noyes2[user->ignfuns]);
  }
write_user(user,"+----------------------------------------------------------------------------+\n\n");
}


/*** set to ignore/listen to a user ***/
void set_igusers(UR_OBJECT user)
{
	UR_OBJECT u;
	int i=0;

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

	yes=0;
	if (user->ignall) {
		user->ignall=0;
		yes=1;
		}
	if (user->igntells) {
		user->igntells=0;
		yes=1;
		}
	if (user->ignshouts) {
		user->ignshouts=0;
		yes=1;
		}
	if (user->ignpics) {
		user->ignpics=0;
		yes=1;
		}
	if (user->ignlogons) {
		user->ignlogons=0;
		yes=1;
		}
	if (user->ignwiz) {
		user->ignwiz=0;
		yes=1;
		}
	if (user->igngreets) {
		user->igngreets=0;
		yes=1;
		}
	if (user->ignbeeps) {
		user->ignbeeps=0;
		yes=1;
		}
	if (user->igntr) {
		user->igntr=0;
		yes=1;
		}
	if (user->ignfuns) {
		user->ignfuns=0;
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
	 		switch(user->igntells) {
				case 0: user->igntells=1;
					write_user(user,"You will now ignore tells.\n");
					break; 
				case 1: user->igntells=0;
					write_user(user,"You will now hear tells.\n");
					break;
				}
			return;
		case IGN_SHOUTS:
			switch(user->ignshouts) {
				case 0: user->ignshouts=1;
					write_user(user,"You will now ignore shouts.\n");
					break; 
				case 1: user->ignshouts=0;
					write_user(user,"You will now hear shouts.\n");
					break;
				}
			return;
		case IGN_PICS:
			switch(user->ignpics) {
				case 0: user->ignpics=1;
					write_user(user,"You will now ignore pictures.\n");
					break; 
				case 1: user->ignpics=0;
					write_user(user,"You will now see pictures.\n");
					break;
				}
			return;
		case IGN_LOGONS:
			switch(user->ignlogons) {
				case 0: user->ignlogons=1;
					write_user(user,"You will now ignore all logons and logoffs.\n");
					break;
				case 1: user->ignlogons=0;
					write_user(user,"You will now see all logons and logoffs.\n");
					break;
				}
			return;
		case IGN_WIZ:
			if (user->level<WIZ) {
				write_user(user, "Nemas na to level !\n");
				return;
				}
			switch(user->ignwiz) {
				case 0: user->ignwiz=1;
					write_user(user,"You will now ignore all wiztells and wizemotes.\n");
					break;
				case 1: user->ignwiz=0;
					write_user(user,"You will now listen to all wiztells and wizemotes.\n");
					break;
				}
			return;
		case IGN_GREETS:
			switch(user->igngreets) {
				case 0: user->igngreets=1;
					write_user(user,"You will now ignore all greets.\n");
					break;
				case 1: user->igngreets=0;
					write_user(user,"You will now see all greets.\n");
					break;
				}
			return;
		case IGN_BEEPS:
			switch(user->ignbeeps) {
				case 0: user->ignbeeps=1;
					write_user(user,"You will now ignore all beeps from users.\n");
					break;
				case 1: user->ignbeeps=0;
					write_user(user,"You will now hear all beeps from users.\n");
					break;
				}
			return;
		case IGN_TRANSP:
			switch(user->igntr) {
				case 0: user->igntr=1;
					write_user(user,"You will now ignore all transports.\n");
					break;
				case 1: user->igntr=0;
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
	 		switch(user->ignfuns) {
				case 0: user->ignfuns=1;
					write_user(user,"You will now ignore funs.\n");
					break; 
				case 1: user->ignfuns=0;
					write_user(user,"You will now hear funs.\n");
					break;
				}
			return;
		}
}
