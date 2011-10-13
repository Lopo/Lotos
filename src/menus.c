/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                   Funkcie pre Lotos v1.2.0 pre pracu s menu
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __MENUS_C__
#define __MENUS_C__ 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "val_set_main.h"
#include "val_set_term.h"
#include "val_set_bank.h"
#include "obj_sys.h"
#include "obj_syspp.h"
#include "prototypes.h"
#include "menus.h"


/* Prints out the various menus */
void print_menu(UR_OBJECT user)
{
	char fname[500];

	set_crash();
	cls(user);
	sprintf(fname, "%s/%s.%d", SCRFILES, menu_tab[user->set_mode].fname, user->set_op);
	if (!show_file(user, fname)) {
		vwrite_user(user, "~FRNemozem najst %s menu..\n", menu_tab[user->set_mode].name);
		write_syslog(ERRLOG, 1, "Nemozem najst subor %s\n", fname);
		return;
		}
	if (user->set_op) write_user(user, menu_tab[user->set_mode].prompt1);
	else write_user(user, menu_tab[user->set_mode].prompt0);
}

/* main set_ops switch */
int setops(UR_OBJECT user, char *inpstr)
{
	switch (user->set_mode) {
		case SET_MAIN: return setmain_ops(user, inpstr);
		case SET_TERM: return setops_term(user, inpstr);
		case SET_BANK: return setops_bank(user, inpstr);
		default: return 0;
		}
}



// SET
void show_attributes(UR_OBJECT user)
{
	char *shide[]={"Zobrazuje","Skryty"};
	char *rm[]={"Hlavna ruuma","odhlasovacia"};
	int i=1,cnt=0;

	set_crash();
	write_user(user," ~FT~OLStatus tvojich '~FRset~FT' atributov~RS\n");
	while (set_tab[i].type[0]!='*') {
		text[0]='\0';
		vwrite_user(user," %-10.10s : ~OL", set_tab[i].type);
		switch (i) {
			case SET_GEND:
				vwrite_user(user,"%-61.61s", sex[user->gender]);
				break;
			case SET_AGE:
				if (!user->age) sprintf(text,"~FRnenastaveny");
				else sprintf(text,"%d",user->age);
				vwrite_user(user,"%-61.61s",text);
				break;
			case SET_EMAIL:
				if (!strcmp(user->email,"#UNSET")) sprintf(text,"~CRnenastaveny");
				else {
					if (user->mail_verified) sprintf(text,"%s~RS - ~FGovereny",user->email);
					else sprintf(text,"%s~RS - ~FRneovereny",user->email);
					}
				vwrite_user(user,"%-66.66s",text);
				break;
			case SET_HOMEP:
				if (!strcmp(user->homepage,"#UNSET")) sprintf(text, "~FRnenastavena");
				else sprintf(text,"%s",user->homepage);
				vwrite_user(user,"%-61.61s",text);
				break;
			case SET_HIDEEMAIL:
				vwrite_user(user,"%-61.61s",shide[user->hideemail]);
				break;
			case SET_ROOM:
				vwrite_user(user,"%-61.61s",rm[user->lroom]);
				break;
			case SET_FWD:
				vwrite_user(user,"%-61.61s",offon[user->autofwd]);
				break;
			case SET_PASSWD:
				vwrite_user(user,"%-61.61s",offon[user->show_pass]);
				break;
			case SET_RDESC:
				vwrite_user(user,"%-61.61s",offon[user->show_rdesc]);
				break;
			case SET_COMMAND:
				vwrite_user(user,"%-61.61s",help_style[user->cmd_type]);
				break;
			case SET_RECAP:
				cnt=colour_com_count(user->recap);
				vwrite_user(user,"%-*.*s",61+cnt*3,61+cnt*3,user->recap);
				break;
			case SET_ICQ:
				if (!strcmp(user->icq,"#UNSET")) sprintf(text,"~FRnenastavene");
				else sprintf(text,"%s",user->icq);
				vwrite_user(user,"%-61.61s",text);
				break;
			case SET_ALERT:
				vwrite_user(user,"%-61.61s",offon[user->alert]);
				break;
			case SET_AUDIO:
				vwrite_user(user,"%-61.61s",offon[user->pueblo_mm]);
				break;
			case SET_PPA:
				vwrite_user(user,"%-61.61s",offon[user->pueblo_pg]);
				break;
			case SET_VOICE:
				vwrite_user(user,"%-61.61s", sex[(!(user->voiceprompt-1))+1]);
				break;
			case SET_MODE:
				vwrite_user(user,"%-61.61s", user->command_mode?"PRIKAZOVY":"KECACI");
				break;
			case SET_PROMPT:
				vwrite_user(user,"%-61.61s", (user->prompt==-1)?"vlastny":prompt_tab[user->prompt].name);
				break;
			case SET_WHO:
				vwrite_user(user,"%2.2d - %-56.56s", user->who_type, who_style[user->who_type]);
				break;
			} /* end main switch */
		write_user(user, "~RS\n");
		i++;
		}
	return;
}


/* Set the user attributes */
void set_attributes(UR_OBJECT user, char *inpstr)
{
	int i=0,tmp=0,setattrval=-1;
	char name[USER_NAME_LEN+1],*recname;

	set_crash();
	if (word_count<2) {
		write_user(user, use_menu_prompt);
		user->set_mode=SET_MAIN;
		user->misc_op=102;
		no_prompt=1;
		return;
		}
	i=0;
	strtolower(word[1]);
	while (set_tab[i].type[0]!='*') {
		if (!strcmp(set_tab[i].type,word[1])) {
			setattrval=i;
			break;
			}
		++i;
		}
	if (setattrval==-1) {
		i=0;
		write_user(user,"Nastavenia ktore si mozes zmenit:\n");
		while (set_tab[i].type[0]!='*') {
			text[0]='\0';
			vwrite_user(user,"~FT%s~RS : %s\n",set_tab[i].type,set_tab[i].desc);
			i++;
			}
		user->set_mode=SET_NONE;
		user->set_op=0;
		return;
		}
	write_user(user,"\n");
	switch (setattrval) {
		case SET_SHOW: show_attributes(user); return;
		case SET_GEND:
			if (word_count<3) {
				write_usage(user,"set %s m/z", set_tab[SET_GEND].type);
				return;
				}
			if (user->gender && user->level<ARCH) {
				write_user(user, "Ved uz mas nastaveny gender !\n");
				return;
				}
			inpstr=remove_first(inpstr);
			inpstr=colour_com_strip(inpstr);
			inpstr[0]=tolower(inpstr[0]);
			tmp=user->gender;
			switch (inpstr[0]) {
				case 'm' :
					user->gender=MALE;
					break;
				case 'f' :
				case 'z' :
					user->gender=FEMALE;
					break;
				case 'n' :
				case 'd' :
					if (user->level >= GOD) {
						user->gender=NEUTER;
						break;
						}
				default  :
					write_user(user, "Take pohlavie nepoznam, ty hej ?\n");
					return;
				} /* end switch */
			vwrite_user(user,"Pohlavie nastavene na ~OL%s~RS\n", sex[user->gender]);
			syspp->acounter[tmp]--;
			syspp->acounter[user->gender]++;
			if (syspp->acounter[user->gender]>syspp->mcounter[user->gender]) {
				syspp->mcounter[user->gender]++;
				save_counters();
				}
			if (amsys->auto_promote && user->gender) check_autopromote(user,1);
			if (user->gender!=tmp) {
				nick_grm(user);
				write_user(user, "Tvoje sklonovanie mena bolo nastavene nasledovne:\n");
				show_nick_grm(user, user);
				write_user(user, "Ak mas proti tomu nejake vyhrady, kontaktuj adminov\n\n");
				}
			return;
		case SET_AGE:
			if (word_count<3) {
				write_usage(user,"set %s <age>", set_tab[SET_AGE].type);
				return;
				}
			tmp=atoi(word[2]);
			if (tmp<1 || tmp>120) {
				write_user(user,"You can only set your age range between 1 and 120.\n");
				return;
				}
			user->age=tmp;
			vwrite_user(user,"Age now set to: %d\n",user->age);
			return;
		case SET_EMAIL:
			inpstr=remove_first(inpstr);
			inpstr=colour_com_strip(inpstr);
			if (!inpstr[0]) strcpy(user->email,"#UNSET");
			else if (strlen(inpstr)>80) {
				write_user(user,"The maximum email length you can have is 80 characters.\n");
				return;
				}
			else {
				if (!validate_email(inpstr)) {
					write_user(user,"That email address format is incorrect.  Correct format: user@network.net\n");
					return;
					}
				strcpy(user->email,inpstr);
				} /* end else */
			if (!strcmp(user->email,"#UNSET")) write_user(user,"Email set to : ~FRunset\n");
			else vwrite_user(user,"Email set to : ~FT%s\n",user->email);
			set_forward_email(user);
			return;
		case SET_HOMEP:
			inpstr=remove_first(inpstr);
			inpstr=colour_com_strip(inpstr);
			if (!inpstr[0]) strcpy(user->homepage,"#UNSET");
			else if (strlen(inpstr)>80) {
				write_user(user,"The maximum homepage length you can have is 80 characters.\n");
				return;
				}
			else strcpy(user->homepage,inpstr);
			if (!strcmp(user->homepage,"#UNSET")) write_user(user,"Homepage set to : ~FRunset\n");
			else vwrite_user(user,"Homepage set to : ~FT%s\n",user->homepage);
			return;
		case SET_HIDEEMAIL:
			user->hideemail=!user->hideemail;
			vwrite_user(user,"Email showing now %s.\n", offon[user->hideemail]);
			return;
		case SET_ROOM:
			switch (user->lroom) {
				case 0:
					user->lroom=1;
					write_user(user,"You will log on into the room you left from.\n");
					break;
				case 1:
					user->lroom=0;
					write_user(user,"You will log on into the main room.\n");
					break;
				}
			return;
		case SET_FWD:
			if (!user->email[0] || !strcmp(user->email,"#UNSET")) {
				write_user(user,"You have not yet set your email address - autofwd cannot be used until you do.\n");
				return;
				}
			if (!user->mail_verified) {
				write_user(user,"You have not yet verified your email - autofwd cannot be used until you do.\n");
				return;
				}
			switch (user->autofwd) {
				case 0:
					user->autofwd=1;
					write_user(user,"You will also receive smails via email.\n");
					break;
				case 1:
					user->autofwd=0;
					write_user(user,"You will no longer receive smails via email.\n");
					break;
				}
			return;
		case SET_PASSWD:
			switch (user->show_pass) {
				case 0:
					user->show_pass=1;
					write_user(user,"You will now see your password when entering it at login.\n");
					break;
				case 1:
					user->show_pass=0;
					write_user(user,"You will no longer see your password when entering it at login.\n");
					break;
				}
			return;
		case SET_RDESC:
			switch (user->show_rdesc) {
				case 0:
					user->show_rdesc=1;
					write_user(user,"You will now see the room descriptions.\n");
					break;
				case 1:
					user->show_rdesc=0;
					write_user(user,"You will no longer see the room descriptions.\n");
					break;
				}
			return;
		case SET_COMMAND:
			if (word_count<3) {
				write_usage(user,"set %s <#typ>", set_tab[SET_COMMAND].type);
				return;
				}
			tmp=atoi(word[2]);
			if (tmp<1 || tmp>NUM_HELP) {
				vwrite_user(user,"Help type can only be set between 1 & %d - setting to default\n", NUM_HELP);
				user->cmd_type=1;
				}
			else user->cmd_type=tmp;
			vwrite_user(user,"Help type now set to: %s\n", help_style[user->cmd_type]);
			return;
		case SET_RECAP:
			if (!amsys->allow_recaps) {
				write_user(user,"Sorry, names cannot be recapped at this present time.\n");
				return;
				}
			if (word_count<3) {
				write_usage(user,"set %s <meno ako ho chces mat>", set_tab[SET_RECAP].type);
				return;
				}
			if (strstr(word[2], "~FK") && user->level<ROOT) {
				write_user(user, "You can't have black in your name!\n");
				return;
				}
			if (strlen(word[2])>(USER_NAME_LEN+USER_NAME_LEN*3-1)) {
				write_user(user, "That recapped name is too long!\n");
				return;
				}
			recname=colour_com_strip(word[2]);
			strcpy(name,recname);
			strtolower(name);
			name[0]=toupper(name[0]);
			if (strcmp(user->name,name)) {
				write_user(user,"The recapped name still has to match your proper name.\n");
				return;
				}
			strcpy(user->recap,word[2]);
			strcpy(user->bw_recap,recname);
			vwrite_user(user,"Your name will now appear as \"%s~RS\" on the 'who', 'examine', tells, etc.\n",user->recap);
			return;
		case SET_ICQ:
			strcpy(word[2],colour_com_strip(word[2]));
			if (!word[2][0]) strcpy(user->icq,"#UNSET");
			else if (strlen(word[2])>ICQ_LEN) {
				vwrite_user(user,"The maximum ICQ UIN length you can have is %d characters.\n", ICQ_LEN);
				return;
				}
			else strcpy(user->icq,word[2]);
			if (!strcmp(user->icq,"#UNSET")) write_user(user,"ICQ number set to : ~FRunset\n");
			else vwrite_user(user,"ICQ number set to : ~FT%s\n",user->icq);
			return;
		case SET_ALERT:
			switch (user->alert) {
				case 0:
					user->alert=1;
					write_user(user,"You will now be alerted if anyone on your friends list logs on.\n");
					break;
				case 1:
					user->alert=0;
					write_user(user,"You will no longer be alerted if anyone on your friends list logs on.\n");
					break;
				}
			return;
		case SET_AUDIO:
			user->pueblo_mm=!user->pueblo_mm;
			vwrite_user(user, "'Pueblo Audio Prompt' now %s\n", offon[user->pueblo_mm]);
			return;
		case SET_PPA:
			user->pueblo_pg=!user->pueblo_pg;
			vwrite_user(user, "'Pueblo Pager Audio' now %s\n", offon[user->pueblo_pg]);
			return;
		case SET_VOICE:
			user->voiceprompt=!user->voiceprompt;
			vwrite_user(user, "'audio prompt voice gender' nastavene na %s\n",
				sex[(!(user->voiceprompt-1))+1]);
			if (!user->pueblo) write_user(user,"This function only works when connected using the Pueblo telnet client.\n");
			return;
		case SET_MODE:
#ifdef NETLINKS
			if (user->room==NULL) {
				vwrite_user(user, "Sorrac, momentalne ako vzdialen%s user%s nemozes pouzit tento prikaz\n", grm_gnd(1, user->gender), grm_gnd(2, user->gender));
				return;
				}
#endif
			user->command_mode=!user->command_mode;
			vwrite_user(user, "odteraz si v %s mode\n", user->command_mode?"prikazovom":"kecacom");
			return;
		case SET_PROMPT:
#ifdef NETLINKS
			if (user->room==NULL) {
				vwrite_user(user, "Sorrac, momentalne ako vzdialen%s user%s nemozes pouzit tento prikaz\n", grm_gnd(1, user->gender), grm_gnd(2, user->gender));
				return;
				}
#endif
			if (word_count<3) {
				if (user->prompt>0) vwrite_user(user, "Aktualny prompt: %s\n", prompt_tab[user->prompt].name);
				else write_user(user, "Aktualny prompt: vlastny\n");
				write_usage(user, "set %s <typ>|<str>", set_tab[SET_PROMPT].type);
				return;
				}
			if (is_number(word[2])) {
				tmp=atoi(word[2]);
				if (tmp>(NUM_PROMPT-1)) {
					vwrite_user(user, "Prompt moze byt 0 - %d alebo vlastny retazec\n", NUM_PROMPT);
					return;
					}
				user->prompt=tmp;
				strcpy(user->prompt_str, prompt_tab[user->prompt].str);
				vwrite_user(user, "Prompt nastaveny na: %s\n", prompt_tab[user->prompt].name);
				return;
				}
			if (strlen(word[2])>PROMPT_LEN) {
				vwrite_user(user, "Pridlhy prompt, max. dlzka: %d\n", PROMPT_LEN);
				return;
				}
			user->prompt=-1;
			sprintf(user->prompt_str, "%.*s", PROMPT_LEN, word[2]);
			user->prompt_str[PROMPT_LEN]='\0';
			write_user(user, "Prompt nastaveny\n");
			return;
		case SET_WHO:
			if (word_count<3) {
				write_usage(user,"set %s <#typ>", set_tab[SET_WHO].type);
				vwrite_user(user, "aktualne nastaveny typ: %d - %s\n", user->who_type, who_style[user->who_type]);
				write_user(user, "mozne typy:\n");
				for (i=1; i<=NUM_WHO; i++)
					vwrite_user(user, "\t%d - %s\n", i, who_style[i]);
				return;
				}
			tmp=atoi(word[2]);
			if (tmp<1 || tmp>NUM_WHO) {
				vwrite_user(user,"Who type can only be set between 1 & %d - setting to default\n", NUM_WHO);
				user->who_type=1;
				}
			else user->who_type=tmp;
			vwrite_user(user,"Who type now set to: %s\n", who_style[user->who_type]);
			return;
		} /* end main switch */
}


/* The set menu... very very very handy... for newbies */
int setmain_ops(UR_OBJECT user, char *inpstr)
{
	char temp[ARR_SIZE], *name;
	int which, val, hlp;

	set_crash();
	if (user->set_mode!=SET_MAIN) return 0;
	if (!user->vis) name=invisname;
	else name=user->recap;
	temp[0]='\0';
	val=0;
	if (inpstr[0]=='0') {
		hlp=-1;
		val=1;
		}
	else for (hlp=0; set_tab[hlp].cmenu!='*'; hlp++)
		if (toupper(inpstr[0])==set_tab[hlp].cmenu) {
			val=1;
			break;
		}
	switch (user->set_op) {
		case 1 :
			switch (hlp) {
				case SET_GEND:
					if (user->gender && user->level<ARCH) {
						write_user(user, "Ved uz mas nastaveny gender !\n");
						write_user(user, center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					vwrite_user(user, "Aktualny gender: %s\n", sex[user->gender]);
					write_user(user,"~CTEnter new gender~CB: ");
					user->set_op=2;
					return 1;
				case SET_AGE:
					if (user->age) {
						vwrite_user(user,"~FGYour age currently is~CB: [~RS%d~CB]\n", user->age);
						write_user(user,"~CTEnter new age~CB: ");
						user->set_op=3;
						return 1;
						}
					write_user(user,"~CTHow old are you?~CB: ");
					user->set_op=3;
					return 1;
				case SET_EMAIL:
					if (!strcmp(user->email, "#UNSET")) write_user(user,"~FGEmail currently set at~CB: [~RSunset~CB]\n");
					else vwrite_user(user,"~FGEmail currently set at~CB: [~RS%s~CB]\n", user->email);
					write_user(user,"~CTEnter new email address~CB: ");
					user->set_op=4;
					return 1;
				case SET_HOMEP:
					if (strcmp(user->homepage, "#UNSET")) {
						vwrite_user(user,"~FGWebpage currently set at~CB: [~RS%s~CB]\n",user->homepage);
						write_user(user,"~CTEnter a new homepage address~CB =>~RS http://");
						user->set_op=5;
						return 1;
						}
					write_user(user,"~CTEnter a homepage address~CB:~RS http://");
					user->set_op=5;
					return 1;
				case SET_HIDEEMAIL:
					if (user->hideemail) {
						write_user(user,"~FGYou are no longer hiding your email address...\n");
						user->hideemail=0;
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1; 
						}
					write_user(user,"~FGYou hide your email from other users...\n");
					user->hideemail=1;
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_ROOM:
					user->lroom=!user->lroom;
					vwrite_user(user, "You will log on into the %s\n",
						user->lroom?"room you left from":"main room");
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_FWD:
					if (!user->email[0] || !strcmp(user->email, "#UNSET")) {
						write_user(user, "You have no yet set your email address - autofwd cannot be used until you do.\n");
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					if (!user->mail_verified) {
						write_user(user, "You have no yet verified your email - autofwd cannot be used until you do.\n");
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					user->autofwd=!user->autofwd;
					vwrite_user(user, "Autofwd now %s\n", offon[user->autofwd]);
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_PASSWD:
					if (user->show_pass) {
						write_user(user, "You will now see your password when entering it at login\n");
						write_user(user,center(continue1,81));
						user->show_pass=0;
						user->set_op=-1;
						return 1;
						}
					write_user(user, "You will no longer see your password when entering it at login\n");
					write_user(user,center(continue1,81));
					user->show_pass=1;
					user->set_op=-1;
					return 1;
				case SET_RDESC:
					if (user->show_rdesc) {
						write_user(user, "You will now see the room descriptions\n");
						write_user(user,center(continue1,81));
						user->show_rdesc=0;
						user->set_op=-1;
						return 1;
						}
					write_user(user, "You will no longer see the room descriptions\n");
					write_user(user,center(continue1,81));
					user->show_rdesc=1;
					user->set_op=-1;
					return 1;
				case SET_COMMAND:
					vwrite_user(user,"~FGYour help type is currently~CB: [~RS%d - %s~CB]\n", user->cmd_type, help_style[user->cmd_type]);
					write_user(user,"~FMPossible help types are~CB:\n");
					for (which=1; which<=NUM_HELP; ++which) {
						vwrite_user(user,"~CG%d~CB = ~CT%s\n", which, help_style[which]);
						}
					write_user(user,"~CTYour choice~CB: ");
					user->set_op=8;
					return 1;
				case SET_RECAP:
					if (!amsys->allow_recaps) {
						write_user(user, "Sorry, names cannot be recapped at this present time");
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					vwrite_user(user,"~FGName capitalization currently set at~CB: [~RS%s~RS~CB]\n", user->recap);
					write_user(user,"~CTEnter new capitalization~CB: ");
					user->set_op=9;
					return 1;
				case SET_ICQ:
					if (user->icq) {
						if (strcmp(user->icq, "#UNSET")) vwrite_user(user,"~FGYour ICQ number is set at~CB: [~RS%s~CB]\n", user->icq);
						else write_user(user,"~FGYour ICQ number is set at~CB: [~RSunset~CB]\n");
						write_user(user,"~CTEnter new ICQ number~CB: ");
						user->set_op=10;
						return 1;
						}
					write_user(user,"~CTEnter an ICQ number~CB: ");
					user->set_op=10;
					return 1;
				case SET_ALERT:
					if (user->alert) {
						write_user(user, "You will now be alerted if anyone on your notify list logs on\n");
						user->alert=0;
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					write_user(user, "You will no longer be alerted if anyone on your notify list logs on\n");
					user->alert=1;
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_AUDIO:
					user->pueblo_mm=!user->pueblo_mm;
					vwrite_user(user, "'Pueblo Audio Prompt' now %s\n", offon[user->pueblo_mm]);
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_PPA:
					user->pueblo_pg=!user->pueblo_pg;
					vwrite_user(user, "'Pueblo Pager Audio' now %s\n", offon[user->pueblo_pg]);
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_VOICE:
					user->voiceprompt=!user->voiceprompt;
					vwrite_user(user, "Audio Prompt Voice Gender' nastavene na %s\n",
						sex[(!(user->voiceprompt-1))+1]);
					if (!user->pueblo) write_user(user, "This function only works when connected using the Pueblo telnet client.\n");
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1;
				case SET_MODE:
					if (user->room==NULL) {
						write_user(user, "teraz nemozes zmenit tento parameter\n");
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					user->command_mode=!user->command_mode;
					vwrite_user(user,"~FGOdteraz si v %s mode!\n", user->command_mode?"prikazovom":"kecacom");
					write_user(user,center(continue1,81));
					user->set_op=-1;
					return 1; 
				case SET_PROMPT:
					vwrite_user(user, "~FGAktualny prompt~CB: [~RS%d ~CW-~RS %s~CB].\n", user->prompt, prompt_tab[user->prompt].name);
					vwrite_user(user, "~CTZadaj novy prompt~CB [~FR0~FW-~FR%d~FW|~FR<str>~CB]: ~RS", NUM_PROMPT-1);
					user->set_op=11;
					return 1;
				case SET_WHO:
					vwrite_user(user,"~FGYour who type is currently~CB: [~RS%d - %s~CB]\n", user->who_type, who_style[user->who_type]);
					write_user(user,"~CTYour choice~CB: ");
					user->set_op=12;
					return 1;
				case -1:
					vwrite_room_except(user->room, user, room_leave_setup, name);
					user->ignore.all=user->ignore.all_store;
					user->set_mode=SET_NONE;
					user->set_op=0;
					user->status='a';
					prompt(user);
					return 1;
				default  :
					write_user(user, user_bch_setup);
					write_user(user,center(continue1,81));
					user->set_op=-1;
				} /* end of switch (hlp) */
			return 1;

		case 2: /* SET_GEND */
			hlp=user->gender;
			inpstr=colour_com_strip(inpstr);
			switch (toupper(inpstr[0])) {
				case 'M':
					if (user->gender & FEMALE) {
						write_user(user,"~FGGender switched to ~CRMALE\n");
						user->gender=MALE;
						syspp->acounter[hlp]--;
						syspp->acounter[user->gender]++;
						user->set_op=-1;
						goto CH_GEND;
						}
					if (user->gender & MALE) {
						write_user(user,"~FGYes, we know that your male...\n");
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					user->gender=MALE;
					syspp->acounter[hlp]--;
					syspp->acounter[user->gender]++;
					write_user(user,"~FGGender set to ~CRMALE!\n");
					user->set_op=-1;
					goto CH_GEND;
				case 'Z':
					if (user->gender & MALE) {
						write_user(user,"~FGGender switched to ~CRFEMALE\n");
						user->gender=FEMALE;
						syspp->acounter[hlp]--;
						syspp->acounter[user->gender]++;
						user->set_op=-1;
						goto CH_GEND;
						}
					if (user->gender & FEMALE) {
						write_user(user,"~FGYes, we know that your female...\n");
						write_user(user,center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					user->gender=FEMALE;
					syspp->acounter[hlp]--;
					syspp->acounter[user->gender]++;
					write_user(user,"~FGGender set to ~CRFEMALE!\n");
					user->set_op=-1;
					goto CH_GEND;
				case 'N':
				case 'D':
					if (user->level>=GOD) {
						if (user->gender & NEUTER) {
							write_user(user,"~FGYes, we know that your female...\n");
							write_user(user,center(continue1,81));
							user->set_op=-1;
							return 1;
							}
						user->gender=NEUTER;
						syspp->acounter[hlp]--;
						syspp->acounter[user->gender]++;
						write_user(user,"~FGGender set to ~CRNEUTER!\n");
						user->set_op=-1;
						goto CH_GEND;
						}
				default :
					write_user(user,"~FGInvalid gender, please choose either male or female!\n");
					write_user(user,"~CB-=> ");
					user->set_op=2;
					return 1;
				}
CH_GEND:
			if (syspp->acounter[user->gender]>syspp->mcounter[user->gender]) {
				syspp->mcounter[user->gender]++;
				save_counters();
				}
			if (amsys->auto_promote && user->gender) check_autopromote(user, 1);
			if (user->gender!=hlp) {
				nick_grm(user);
				write_user(user, "Tvoje sklonovanie nicku bolo nastavene nasledovne:\n");
				show_nick_grm(user, user);
				write_user(user, "Ak mas proti tomu nejake vyhrady, kontaktuj adminov\n");
				}
			write_user(user,center(continue1,81));
			return 1;

		case 3: /* SET_AGE */
			val=atoi(word[0]);
			if (!val) {
				write_user(user,"~FGInvalid age...\n");
				write_user(user,"~CTRe-enter your age~CB: ");
				user->set_op=3;
				return 1;
				}
			if (val<1) {
				write_user(user,"~FGI don't think so...\n");
				write_user(user,"~CTRe-enter your age~CB: ");
				user->set_op=3;
				return 1;
				}
			if (val>120) {
				write_user(user,"~FGThats a tad bit to old to believe!\n");
				write_user(user,"~CTRe-enter your age~CB: ");
				user->set_op=3;
				return 1;
				}
			user->age=val;
			vwrite_user(user,"~FGAge is now set to~CB: [~RS%d~CB]\n", user->age);
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 4: /* SET_EMAIL */
			if (!strlen(inpstr)) {
				write_user(user,"~FGWould help if you entered something ;)\n");
				write_user(user,"~CTRe-enter email address~CB: ");
				user->set_op=4;
				return 1;
				}
			inpstr=colour_com_strip(inpstr);
			if (!inpstr[0]) strcpy(user->email, "#UNSET");
			else if (strlen(inpstr)>80) {
				write_user(user, "The maximum email length you can have is 80 characters.\n");
				write_user(user,"~CTRe-enter email address~CB: ");
				user->set_op=4;
				return 1;
				}
			else {
				if (!validate_email(inpstr)) {
					write_user(user,"~FGYou have entered an invalid email address!\n");
					write_user(user,"~CTRe-enter email address~CB: ");
					user->set_op=4;
					return 1;
					}
				strcpy(user->email,inpstr);
				}
			if (!strcmp(user->email, "#UNSET")) write_user(user,"~FGEmail address now set to~CB: [~RSunset~CB]\n");
			else vwrite_user(user,"~FGEmail address now set to~CB: [~RS%s~CB]\n", user->email);
			set_forward_email(user);
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 5: /* SET_HOMEP */
			inpstr=colour_com_strip(inpstr);
			if (!inpstr[0]) strcpy(user->homepage, "#UNSET");
			else if (strlen(inpstr)>73) {
				write_user(user, "The maximum homepage length you can have is 73 characters\n");
				write_user(user,"~CTRe-enter a new homepage address~CB =>~RS http://");
				user->set_op=5;
				return 1;
				}
			else {
				sprintf(text,"http://%s",inpstr);
				strcpy(user->homepage,text);
				}
			if (strcmp(user->homepage, "#UNSET")) vwrite_user(user,"~FGHomepage now set to~CB: [~RS%s~CB]\n",user->homepage);
			else write_user(user,"~FGHomepage now set to~CB: [~RSunset~CB]\n");
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 6: /* SET_PAGER */
			user->terminal.pager=atoi(word[0]);
			if (user->terminal.pager<MAX_LINES || user->terminal.pager>99) {
				vwrite_user(user, "Pager can only be set between %d and 99 - setting to default\n");
				user->terminal.pager=23;
				}
			vwrite_user(user, "~FGPager length now set to: ~RS%d\n", user->terminal.pager);
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 7: /* SET_COLOUR */
/*			if (!strlen(inpstr)) {
				write_user(user,"~FGWould help if you entered something ;)\n");
				vwrite_user(user,"~FTRe-enter colour choice ~CW<~CR0~CW - ~CR%d~CW|~CRtest~CW> ~CB-=> ", NUM_COLMODS);
				user->set_op=7;
				return 1;
				}
			if (!strcasecmp(inpstr, "test")) {
				display_colour(user);
				vwrite_user(user,"~FTEnter colour choice ~CW<~CR0~CW - ~CR%d~CW|~CRtest~CW> ~CB-=> ", NUM_COLMODS);
				user->set_op=7;
				return 1;
				}
			hlp=atoi(word[0]);
			if (hlp<0 || hlp>NUM_COLMODS) {
				vwrite_user(user,"~FTInvalid choice ~CW<~CR1~CW - ~CR%d~CW/~CRtest~CW> ~CB-=> ", NUM_COLMODS);
				user->set_op=7;
				return 1;
				}
			user->colour=hlp;
			vwrite_user(user, "~FGColour mode now set to: ~RS%d\n", user->colour);
			write_user(user,center(continue1,81));
		        user->set_op=-1;
*/			return 1;

		case 8: /* SET_COMMAND */
			val=atoi(word[0]);
			if (!val) {
				write_user(user,"~FGInvalid help style...\n");
				vwrite_user(user,"~CTRe-enter help choice 1-%d~CB: ",NUM_HELP);
				user->set_op=8;
				val=0;
				return 1;
				}
			if (val<1 || val>NUM_HELP) {
				write_user(user,"~FGInvalid help style...\n");
				vwrite_user(user,"~CTRe-enter help choice 1-%d~CB: ",NUM_HELP);
				user->set_op=8;
				val=0;
				return 1;
				}
			user->cmd_type=val;
			vwrite_user(user,"~FGHelp style now set to~CB: [~CY%s~CB]\n", help_style[user->cmd_type]);
			write_user(user,center(continue1,81));
			user->set_op=-1;
			return 1;

		case 9: /* SET_RECAP */
			if (!strlen(inpstr)) {
				write_user(user,"~FGWould help if you entered something ;)\n");
				write_user(user,"~CTRe-enter recapped name~CB: ");
				user->set_op=9;
				return 1;
				}
			if (strstr(inpstr,"~FK") && user->level<ROOT) {
				write_user(user,"~FGYou can't have black in your name!\n");
				write_user(user,"~CTRe-enter recapped name~CB: ");
				user->set_op=9;
				return 1;
				}
			if (strlen(inpstr)>(USER_NAME_LEN+USER_NAME_LEN*3-1)) {
				write_user(user,"~FGThat recapped name is too long!\n");
				write_user(user,"~CTRe-enter recapped name~CB: ");
				user->set_op=9;
				return 1;
				}
			strcpy(temp,colour_com_strip(inpstr));
			if (strcasecmp(temp,user->name) && user->level<ROOT) {
				write_user(user,"~FGThats not your name...\n");
				write_user(user,"~CTRe-enter recapped name~CB: ");
				user->set_op=9;
				return 1;
				}
			strcpy(user->recap, inpstr);
			strcpy(user->bw_recap, temp);
			vwrite_user(user,"~FGYou will now be known as~CB: [~RS%s~RS~CB]\n", user->recap);
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 10: /* SET_ICQ */
			inpstr=colour_com_strip(inpstr);
			if (!inpstr[0]) strcpy(user->icq, "#UNSET");
			else if (strlen(inpstr)>ICQ_LEN) {
				vwrite_user(user, "The Maximnum ICQ UIN length you can have is %d characters.\n", ICQ_LEN);
				write_user(user,"~CTRe-enter an ICQ number~CB: ");
				user->set_op=10;
				return 1;
				}
			else strcpy(user->icq, word[0]);
			if (strcmp(user->icq, "#UNSET")) vwrite_user(user,"~FGICQ number now set to~CB: [~RS%s~CB]\n",user->icq);
			else write_user(user,"~FGICQ number now set to~CB: [~RSunset~CB]\n");
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 11: /* SET_PROMPT */
			if (is_number(inpstr)) {
				val=atoi(inpstr);
				if (val>(NUM_PROMPT-1)) {
					vwrite_user(user, "Maximalne cislo promptu je %d.\n", NUM_PROMPT-1);
					write_user(user,"~CTRe-enter prompt~CB: ");
					user->set_op=11;
					return 1;
					}
				user->prompt=val;
				vwrite_user(user, "Prompt nastaveny na typ %s\n", prompt_tab[user->prompt].name);
				strcpy(user->prompt_str, prompt_tab[user->prompt].str);
				write_user(user,center(continue1,81));
				user->set_op=-1;
				return 1;
				}
			if (strlen(inpstr)>PROMPT_LEN) {
				vwrite_user(user, "Maximalna dlzka promptu je %d.\n", PROMPT_LEN);
				write_user(user,"~CTRe-enter prompt~CB: ");
				user->set_op=11;
				return 1;
				}
			user->prompt=-1;
			strcpy(user->prompt_str, inpstr);
			write_user(user, "Prompt nastaveny\n");
			write_user(user,center(continue1,81));
		        user->set_op=-1;
			return 1;

		case 12: /* SET_WHO */
			if (!strcmp(word[0], "list")) {
				write_user(user,"~FMPossible who types are~CB:\n");
				for (which=1; which<=NUM_WHO; ++which) {
					vwrite_user(user,"~CG%d~CB = ~CT%s\n", which, who_style[which]);
					}
				write_user(user,"~CTYour choice~CB: ");
				user->set_op=12;
				return 1;
				}
			val=atoi(word[0]);
			if (!val) {
				write_user(user,"~FGInvalid who style...\n");
				vwrite_user(user,"~CTRe-enter who choice 1-%d|list~CB: ",NUM_WHO);
				user->set_op=12;
				return 1;
				}
			if (val<1 || val>NUM_WHO) {
				write_user(user,"~FGInvalid who style...\n");
				vwrite_user(user,"~CTRe-enter who choice 1-%d|list~CB: ",NUM_WHO);
				user->set_op=12;
				return 1;
				}
			user->who_type=val;
			vwrite_user(user,"~FGWho style now set to~CB: [~CY%s~CB]\n", who_style[user->who_type]);
			write_user(user,center(continue1,81));
			user->set_op=-1;
			return 1;

		case -1:
			temp[0]='\0';
			which=0;
			val=0;
			user->set_mode=SET_MAIN;
			user->set_op=1;
			print_menu(user);
			return 1;
		}
	return 0;
}



// TERM
void showattribs_term(UR_OBJECT user)
{
	int i=1;

	set_crash();
	write_user(user, "Tvoje aktualne nastavenia terminalu:\n");
	while (setterm_tab[i].type[0]!='*') {
		text[0]='\0';
		vwrite_user(user, " %-10.10s : ~OL", setterm_tab[i].type);
		switch (i) {
			case SETTERM_BCKG:
				vwrite_user(user, "%-61.61s", offon[user->terminal.bckg]);
				break;
			case SETTERM_TXT:
				vwrite_user(user, "%-61.61s", offon[user->terminal.txt]);
				break;
			case SETTERM_REVERS:
				vwrite_user(user, "%-61.61s", offon[user->terminal.revers]);
				break;
			case SETTERM_BLINK:
				vwrite_user(user, "%-61.61s", offon[user->terminal.blink]);
				break;
			case SETTERM_BOLD:
				vwrite_user(user, "%-61.61s", offon[user->terminal.bold]);
				break;
			case SETTERM_UNDERLINE:
				vwrite_user(user, "%-61.61s", offon[user->terminal.underline]);
				break;
			case SETTERM_CLEAR:
				vwrite_user(user, "%-61.61s", offon[user->terminal.clear]);
				break;
			case SETTERM_MUSIC:
				vwrite_user(user, "%-61.61s", offon[user->terminal.music]);
				break;
			case SETTERM_XTERM:
				vwrite_user(user, "%-61.61s", offon[user->terminal.xterm]);
				break;
			case SETTERM_CHECHO:
				vwrite_user(user, "%-61.61s", offon[user->terminal.checho]);
				break;
			case SETTERM_WRAP:
				vwrite_user(user, "%-61.61s", offon[user->terminal.wrap]);
				break;
			case SETTERM_BLIND:
				vwrite_user(user, "%-61.61s", offon[user->terminal.blind]);
				break;
			case SETTERM_PAGER:
				sprintf(text, "%d", user->terminal.pager);
				vwrite_user(user, "%-61.61s", text);
				break;
			}
		write_user(user, "~RS\n");
		++i;
		}
	return;
}


void set_terminal(UR_OBJECT user, char *inpstr)
{
	int i=0, setattrval=-1;

	set_crash();
	if (word_count<2) {
		write_user(user, use_menu_prompt);
		user->set_mode=SET_TERM;
		user->misc_op=102;
		no_prompt=1;
		return;
		}
	strtolower(word[1]);
	while (setterm_tab[i].type[0]!='*') {
		if (!strcmp(setterm_tab[i].type, word[1])) {
			setattrval=i;
			break;
			}
		++i;
		}
	if (setattrval==-1) {
		i=0;
		write_user(user, "nastavenia ktore si mozes zmenit:\n");
		while (setterm_tab[i].type[0]!='*') {
			text[0]='\0';
			vwrite_user(user, "~FT%s~RS : %s\n", setterm_tab[i].type, setterm_tab[i].desc);
			i++;
			}
		user->set_mode=SET_NONE;
		user->set_op=0;
		return;
		}
	write_user(user, "\n");
	switch (setattrval) {
		case SETTERM_SHOW: showattribs_term(user); return;
		case SETTERM_BCKG:
			user->terminal.bckg=!user->terminal.bckg;
			vwrite_user(user, "Pouzivanie pozadia odteraz %s%s\n", user->terminal.bckg?"~CG":"~CR", offon[user->terminal.bckg]);
			return;
		case SETTERM_TXT:
			user->terminal.txt=!user->terminal.txt;
			vwrite_user(user, "Pouzivanie farieb odteraz %s%s\n", user->terminal.txt?"~CG":"~CR", offon[user->terminal.txt]);
			return;
		case SETTERM_REVERS:
			user->terminal.revers=!user->terminal.revers;
			vwrite_user(user, "Pouzivanie reverznych farieb odteraz %s%s\n", user->terminal.revers?"~CG":"~CR", offon[user->terminal.revers]);
			return;
		case SETTERM_BLINK:
			user->terminal.blink=!user->terminal.blink;
			vwrite_user(user, "Blikanie odteraz %s%s\n", user->terminal.blink?"~CG":"~CR", offon[user->terminal.blink]);
			return;
		case SETTERM_BOLD:
			user->terminal.bold=!user->terminal.bold;
			vwrite_user(user, "Zvyrazneny text odteraz %s%s\n", user->terminal.bold?"~CG":"~CR", offon[user->terminal.bold]);
			return;
		case SETTERM_UNDERLINE:
			user->terminal.underline=!user->terminal.underline;
			vwrite_user(user, "Podciarkovanie odteraz %s%s\n", user->terminal.underline?"~CG":"~CR", offon[user->terminal.underline]);
			return;
		case SETTERM_CLEAR:
			user->terminal.clear=!user->terminal.clear;
			vwrite_user(user, "Mazanie odteraz %s\n", user->terminal.clear?"kodom":"odriadkovanim");
			return;
		case SETTERM_MUSIC:
			user->terminal.music=!user->terminal.music;
			vwrite_user(user, "Ansi hudba odteraz %s%s\n", user->terminal.music?"~CG":"~CR", offon[user->terminal.music]);
			return;
		case SETTERM_XTERM:
			user->terminal.xterm=!user->terminal.xterm;
			vwrite_user(user, "Xterm kompatibilita odteraz %s%s\n", user->terminal.xterm?"~CG":"~CR", offon[user->terminal.xterm]);
			return;
		case SETTERM_CHECHO:
			user->terminal.checho=!user->terminal.checho;
			vwrite_user(user, "Charecho odteraz %s%s\n", user->terminal.checho?"~CG":"~CR", offon[user->terminal.checho]);
			return;
		case SETTERM_WRAP:
			user->terminal.wrap=!user->terminal.wrap;
			vwrite_user(user, "Zalamovanie slov odteraz %s%s\n", user->terminal.wrap?"~CG":"~CR", offon[user->terminal.wrap]);
			return;
		case SETTERM_BLIND:
			if (user->level<MIN_LEV_BLIND) {
				vwrite_user(user, "Tento parameter mozes pouzit az od levelu %s\n", user_level[MIN_LEV_BLIND].name);
				return;
				}
			if (user->terminal.blind) {
				user->terminal.blind=0;
				vwrite_user(user, "Zobrazovanie na terminal odteraz ~CG%s\n",
					offon[!user->terminal.blind]);
				}
			else {
				cls(user);
				vwrite_user(user, "Zobrazovanie na terminal odteraz ~CR%s\n",
					offon[user->terminal.blind]);
				user->terminal.blind=1;
				}
			return;
		case SETTERM_PAGER:
			if (word_count<3) {
				write_usage(user, "set %s <#pocet>", setterm_tab[SETTERM_PAGER].type);
				vwrite_user(user, "aktualna hodnota : %d\n", user->terminal.pager);
				return;
				}
			user->terminal.pager=atoi(word[2]);
			if (user->terminal.pager<MAX_LINES || user->terminal.pager>99) {
				vwrite_user(user, "Pocet riadkov musi byt v rozmedzi %d ~ 99 - nastavujem def. hodnotu\n", MAX_LINES);
				user->terminal.pager=23;
				return;
				}
			vwrite_user(user, "Pocet riadkov odteraz nastaveny na %d\n", user->terminal.pager);
			return;
		}
}


int setops_term(UR_OBJECT user, char *inpstr)
{
	char *name, temp[ARR_SIZE];
	int val, hlp;

	set_crash();
	if (user->set_mode!=SET_TERM) return 0;
	if (!user->vis) name=invisname;
	else name=user->recap;
	temp[0]='\0';
	val=0;
	if (inpstr[0]=='0') {
		hlp=-1;
		val=1;
		}
	else for (hlp=0; setterm_tab[hlp].cmenu!='*'; hlp++)
		if (toupper(inpstr[0])==setterm_tab[hlp].cmenu) {
			val=1;
			break;
			}
	switch (user->set_op) {
		case 1:
			switch (hlp) {
				case SETTERM_SHOW:
					showattribs_term(user);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_BCKG:
					user->terminal.bckg=!user->terminal.bckg;
					vwrite_user(user, "Pouzivanie farebneho pozadia odteraz %s%s\n", user->terminal.bckg?"~CG":"~CR", offon[user->terminal.bckg]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_TXT:
					user->terminal.txt=!user->terminal.txt;
					vwrite_user(user, "Pouzivanie farebneho textu odteraz %s%s\n", user->terminal.txt?"~CG":"~CR", offon[user->terminal.txt]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_REVERS:
					user->terminal.revers=!user->terminal.revers;
					vwrite_user(user, "Pouzivanie reverznych farieb odteraz %s%s\n", user->terminal.revers?"~CG":"~CR", offon[user->terminal.revers]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_BLINK:
					user->terminal.blink=!user->terminal.blink;
					vwrite_user(user, "Pouzivanie blikania odteraz %s%s\n", user->terminal.blink?"~CG":"~CR", offon[user->terminal.blink]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_BOLD:
					user->terminal.bold=!user->terminal.bold;
					vwrite_user(user, "Pouzivanie zvyraznenych farieb odteraz %s%s\n", user->terminal.bold?"~CG":"~CR", offon[user->terminal.bold]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_UNDERLINE:
					user->terminal.underline=!user->terminal.underline;
					vwrite_user(user, "Pouzivanie podciarkovania odteraz %s%s\n", user->terminal.underline?"~CG":"~CR", offon[user->terminal.underline]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_CLEAR:
					user->terminal.clear=!user->terminal.clear;
					vwrite_user(user, "Mazanie obrazovky odteraz %s\n", user->terminal.clear?"kodom":"odriadkovanim");
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_MUSIC:
					user->terminal.music=!user->terminal.music;
					vwrite_user(user, "Pouzivanie ANSII hudby odteraz %s%s\n", user->terminal.music?"~CG":"~CR", offon[user->terminal.music]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_XTERM:
					user->terminal.xterm=!user->terminal.xterm;
					vwrite_user(user, "Kompatibilita klienta s Xterm odteraz %s%s\n", user->terminal.xterm?"~CG":"~CR", offon[user->terminal.xterm]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_CHECHO: 
					user->terminal.checho=!user->terminal.checho;
					vwrite_user(user, "Echovanie znakov odteraz %s%s\n",
						user->terminal.checho?"~CG":"~CR", offon[user->terminal.checho]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_WRAP:
					user->terminal.wrap=!user->terminal.wrap;
					vwrite_user(user, "Zalamovanie slov odteraz %s%s\n",
						user->terminal.wrap?"~CG":"~CR", offon[user->terminal.wrap]);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_BLIND:
					if (user->level<MIN_LEV_BLIND) {
						vwrite_user(user, "Tento parameter si mozes nastavit az od levela %s\n", user_level[MIN_LEV_BLIND].name);
						write_user(user, center(continue1, 81));
						user->set_op=-1;
						return 1;
						}
					write_user(user, "Tento parameter nie je radsej cez menu pristupny\n");
					write_user(user, center(continue1, 81));
					user->set_op=-1;
					return 1;
				case SETTERM_PAGER:
					vwrite_user(user, "Tvoje aktualne nastavenie ~CB: [~RS%d~CB]\n", user->terminal.pager);
					write_user(user, "~CTNapis novu hodnotu~CB: ");
					user->set_op=2;
					return 1;
				case -1:
					vwrite_room_except(user->room, user, room_leave_setup, name);
					user->ignore.all=user->ignore.all_store;
					user->set_mode=SET_NONE;
					user->set_op=0;
					user->status='a';
					prompt(user);
					return 1;
				default:
					write_user(user, user_bch_setup);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
				}
			return 1;
		case 2: /* setterm_pager */
			user->terminal.pager=atoi(word[0]);
			if (user->terminal.pager<MAX_LINES || user->terminal.pager>99) {
				vwrite_user(user, "Pocet riadkov moze byt len v rozmedzi %d ~ 99 - nastavujem def. hodnotu\n", MAX_LINES);
				user->terminal.pager=23;
				}
			vwrite_user(user, "~FGPocet riadkov na zobrazenie odteraz: ~RS%d\n", user->terminal.pager);
			write_user(user, center(continue1, 81));
			user->set_op=-1;
			return 1;
		case -1:
			temp[0]='\0';
			val=0;
			user->set_mode=SET_TERM;
			user->set_op=1;
			print_menu(user);
			return 1;
		}
	return 0;
}


// BANK
void showattribs_bank(UR_OBJECT user)
{
	set_crash();
	write_user(user, "Tvoje aktualne financie:\n");
	vwrite_user(user, "Na ucte  : %d\n", user->bank);
	vwrite_user(user, "Hotovost : %d\n", user->money);
	return;
}


void set_bank(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	int i=0, setattrval=-1, dep, on;

	set_crash();
	if (strcmp(user->room->name, default_bank)) {
		vwrite_user(user, "najprv chod do ruumy ~CR%s~RS, aby na teba pokladnik videl !\n", default_bank);
		return;
		}
	if (word_count<2) {
		write_user(user, use_menu_prompt);
		user->set_mode=SET_BANK;
		user->misc_op=102;
		no_prompt=1;
		return;
		}
	i=0;
	strtolower(word[1]);
	while (settab_bank[i].type[0]!='*') {
		if (!strcmp(settab_bank[i].type, word[1])) {
			setattrval=i;
			break;
			}
		++i;
		}
	if (setattrval==-1) {
		i=0;
		write_user(user, "mozne operacie:\n");
		while (settab_bank[i].type[0]!='*') {
			text[0]='\0';
			vwrite_user(user, "~FT%s~RS : %s\n", settab_bank[i].type, settab_bank[i].desc);
			i++;
			}
		user->set_mode=SET_NONE;
		user->set_op=0;
		return;
		}
	write_user(user, "\n");
	switch (setattrval) {
		case SETBANK_SHOW: showattribs_bank(user); return;
		case SETBANK_DEP:
			if (word_count<3) {
				write_usage(user, ".bank %s <#suma>", settab_bank[SETBANK_DEP].type);
				vwrite_user(user, "aktualna hotovost : %d\n", user->money);
				return;
				}
			if (!is_number(word[2])) {
				vwrite_user(user, "`%s' je chybny parameter !\n", word[2]);
				return;
				}
			dep=atoi(word[2]);
			if (dep<1) {
				write_user(user, "Minimalny vklad je 1\n");
				return;
				}
			if (dep>user->money) {
				write_user(user, "Nemas pri sebe taku velku hotovost !\n");
				return;
				}
			user->money-=dep;
			user->bank+=dep;
			vwrite_user(user, "OK, vklad na ucet: %d\n", dep);
			return;
		case SETBANK_WITH:
			if (word_count<3) {
				write_usage(user, ".bank %s <#suma>", settab_bank[SETBANK_WITH].type);
				vwrite_user(user, "aktualny stav uctu: %d\n", user->bank);
				return;
				}
			if (!is_number(word[2])) {
				vwrite_user(user, "`%s' je chybny parameter !\n", word[2]);
				return;
				}
			dep=atoi(word[2]);
			if (dep<1) {
				write_user(user, "Minimalny vyber je 1\n");
				return;
				}
			if (dep>user->bank) {
				write_user(user, "Nemas tolko na ucte !\n");
				return;
				}
			user->money+=dep;
			user->bank-=dep;
			vwrite_user(user, "OK, vyber z uctu: %d\n", dep);
			return;
		case SETBANK_SEND:
			if (word_count<3) {	
				write_usage(user, ".bank %s <user> <#suma>", settab_bank[SETBANK_SEND].type);
				vwrite_user(user, "aktualny stav uctu: %d\n", user->bank);
				return;
				}
			if (!is_inumber(word[3])) {
				vwrite_user(user, "`%s' je chybny parameter !\n", word[2]);
				return;
				}
			dep=atoi(word[3]);
			if (dep<0) {
				if (!user->kradnutie) {
					write_user(user, "~CR~OLVidel som ta !!! Este raz a uvidis !!!\n");
					user->kradnutie=1;
					}
				else {
					arrest(user, 1);
					write_user(user, "~CTVaroval som ta !!!\n");
					}
				return;
				}
			if (dep<1) {
				write_user(user, "Minimalny obnos je 1\n");
				return;
				}
			if (dep>user->bank) {
				write_user(user, "Nemas tolko na ucte !\n");
				return;
				}
			if ((u=get_user(word[2]))==NULL) {
				if ((u=create_user())==NULL) {
					vwrite_user(user, "%s: nemozem poslat peniaze\n", syserror);
					write_syslog(ERRLOG, 1, "Unable to create temporary user object in set_bank()\n");
					return;
					}
				strcpy(u->name, word[2]);
				if (!load_user_details(u)) {
					write_user(user, nosuchuser);
					destruct_user(u);
					destructed=0;
					return;
					}
				on=0;
				}
			else on=1;
			if (u==user) {
				write_user(user, "Posielat sebe ???\n");
				return;
				}
			u->bank+=dep;
			user->bank-=dep;
			vwrite_user(user, "OK, vyber z uctu: %d\n", dep);
			vwrite_user(user, "OK, poslanie na ucet: %s\n", u->name);
			if (!on) {
				destruct_user(u);
				destructed=0;
				}
			return;
		}
}


int setops_bank(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char *name, temp[ARR_SIZE];
	int val, hlp, dep, on;

	set_crash();
	if (user->set_mode!=SET_BANK) return 0;
	if (!user->vis) name=invisname;
	else name=user->recap;
	temp[0]='\0';
	val=0;
	if (inpstr[0]=='0') {
		hlp=-1;
		val=1;
		}
	else for (hlp=0; settab_bank[hlp].cmenu!='*'; hlp++)
		if (toupper(inpstr[0])==settab_bank[hlp].cmenu) {
			val=1;
			break;
			}
	switch (user->set_op) {
		case 1:
			switch (hlp) {
				case SETBANK_SHOW:
						showattribs_bank(user);
						write_user(user, center(continue1,81));
						user->set_op=-1;
						return 1;
				case SETBANK_DEP:
					if (user->money<1) {
						write_user(user, "Ved nemas ziadnu hotovost !\n");
						write_user(user, center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					vwrite_user(user, "Tvoja aktualna hotovost ~CB: [~RS%d~CB]\n", user->money);
					write_user(user, "~CTNapis vklad~CB: ");
					user->set_op=2;
					return 1;
				case SETBANK_WITH:
					if (user->bank<1) {
						write_user(user, "Ved nemas nic na ucte !\n");
						write_user(user, center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					vwrite_user(user, "Tvoj aktualny stav uctu ~CB: [~RS%d~CB]\n", user->bank);
					write_user(user, "~CTNapis vyber~CB: ");
					user->set_op=3;
					return 1;
				case SETBANK_SEND:
					if (user->bank<1) {
						write_user(user, "Ved nemas nic na ucte !\n");
						write_user(user, center(continue1,81));
						user->set_op=-1;
						return 1;
						}
					vwrite_user(user, "Tvoj aktualny stav uctu ~CB: [~RS%d~CB]\n", user->bank);
					write_user(user, "~CTNapis kolko poslat~CB: ");
					user->set_op=4;
					return 1;
				case -1:
					vwrite_room_except(user->room, user, room_leave_setup, name);
					user->ignore.all=user->ignore.all_store;
					user->set_mode=SET_NONE;
					user->set_op=0;
					user->status='a';
					prompt(user);
					return 1;
				default:
					write_user(user, user_bch_setup);
					write_user(user, center(continue1, 81));
					user->set_op=-1;
				} /* end of switch (hlp) */
			return 1;

		case 2: /* SETBANK_DEP */
			if (!is_number(word[0])) {
				vwrite_user(user, "`%s' je chybny parameter !\n", word[0]);
				write_user(user, "~CToprav svoj vklad~CB:~RS ");
				user->set_op=2;
				return 1;
				}
			dep=atoi(word[0]);
			if (dep>user->money) {
				write_user(user, "Nemas taku velku hotovost !\n");
				write_user(user, "~CToprav svoj vklad~CB:~RS ");
				user->set_op=2;
				return 1;
				}
			if (dep<1) {
				write_user(user, "Vkladanie na ucet zrusene.\n");
				write_user(user, center(continue1,81));
				user->set_op=-1;
				return 1;
				}
			user->bank+=dep;
			user->money-=dep;
			vwrite_user(user, "OK, uskutocneny vklad: %d\n", dep);
			write_user(user, center(continue1, 81));
			user->set_op=-1;
			return 1;
		case 3: /* setbank_with */
			if (!is_number(word[0])) {
				vwrite_user(user, "`%s' je chybny parameter !\n", word[0]);
				write_user(user, "~CToprav svoj vyber~CB:~RS ");
				user->set_op=3;
				return 1;
				}
			if (user->bank<1) {
				write_user(user, "ved nemas nic na ucte !\n");
				write_user(user, center(continue1,81));
				user->set_op=-1;
				return 1;
				}
			dep=atoi(word[0]);
			if (dep>user->bank) {
				write_user(user, "Nemas tolko na ucte !\n");
				write_user(user, "~CToprav svoj vyber~CB:~RS ");
				user->set_op=3;
				return 1;
				}
			if (dep<1) {
				write_user(user,"Vyber z uctu zruseny.\n");
				write_user(user, center(continue1,81));
				user->set_op=-1;
				return 1;
				}
			user->bank-=dep;
			user->money+=dep;
			vwrite_user(user, "OK, uskutocneny vyber: %d\n", dep);
			write_user(user, center(continue1, 81));
			user->set_op=-1;
			return 1;
		case  4: /* setbank_send - ammount */
			if (!is_number(word[0])) {
				vwrite_user(user, "`%s' je chybny parameter !\n", word[0]);
				write_user(user, "~CToprav obnos~CB:~RS ");
				user->set_op=4;
				return 1;
				}
			if (user->bank<1) {
				write_user(user, "ved nemas nic na ucte !\n");
				write_user(user, center(continue1,81));
				user->set_op=-1;
				return 1;
				}
			dep=atoi(word[0]);
			if (dep>user->bank) {
				write_user(user, "Nemas tolko na ucte !\n");
				write_user(user, "~CToprav obnos~CB:~RS ");
				user->set_op=4;
				return 1;
				}
			if (dep<1) {
				write_user(user,"Vyber z uctu zruseny.\n");
				write_user(user, center(continue1,81));
				user->set_op=-1;
				return 1;
				}
			user->tmp_int=dep;
			vwrite_user(user, "OK, zadaj komu poslat: ");
			user->set_op=5;
			return 1;
		case  5: /* setbank_send - user */
			if ((u=get_user(word[0]))==NULL) {
				if ((u=create_user())==NULL) {
					vwrite_user(user, "%s: nemozem poslat peniaze\n", syserror);
					write_syslog(ERRLOG, 1, "Unable to create temporary user object in set_bank()\n");
					user->tmp_int=0;
					user->set_op=-1;
					write_user(user, center(continue1,81));
					return 1;
					}
				strcpy(u->name, word[0]);
				if (!load_user_details(u)) {
					write_user(user, nosuchuser);
					destruct_user(u);
					destructed=0;
					user->set_op=-1;
					user->tmp_int=0;
					write_user(user, center(continue1,81));
					return 1;
					}
				on=1;
				}
			else on=1;
			u->bank+=user->tmp_int;
			user->bank-=user->tmp_int;
			vwrite_user(user, "OK, vyber z uctu: %d\n", user->tmp_int);
			vwrite_user(user, "OK, poslanie na ucet: %s\n", u->name);
			if (!on) {
				destruct_user(u);
				destructed=0;
				}
			user->tmp_int=0;
			user->set_op=-1;
			write_user(user, center(continue1, 81));
			return 1;
		case -1:
			temp[0]='\0';
			val=0;
			user->set_mode=SET_BANK;
			user->set_op=1;
			print_menu(user);
			return 1;
		}
	return 0;
}


#endif /* menus.c */
