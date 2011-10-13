/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                Funkcie Lotos v1.2.0 pre userovu osobnu potrebu
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_USER_C__
#define __CT_USER_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "obj_syspp.h"
#include "ct_user.h"
#include "comvals.h"
#include "ignval.h"


/*** Set user description ***/
void set_desc(UR_OBJECT user, char *inpstr)
{
	set_crash();
if (word_count<2) {
  vwrite_user(user,"Your current description is: %s\n",user->desc);
  return;
  }
if (strstr(word[1],"(CLONE)")) {
  write_user(user,"You cannot have that description.\n");
  return;
  }
if (strlen(inpstr)>USER_DESC_LEN) {
  write_user(user,"Description too long.\n");
  return;
  }
strcpy(user->desc,inpstr);
write_user(user,"Description set.\n");
/* check to see if user should be promoted */
if (amsys->auto_promote) check_autopromote(user,2);
}


/*** Set in and out phrases ***/
void set_iophrase(UR_OBJECT user, char *inpstr)
{
	set_crash();
if (strlen(inpstr)>PHRASE_LEN) {
  write_user(user,"Phrase too long.\n");  return;
  }
if (com_num==INPHRASE) {
  if (word_count<2) {
    vwrite_user(user,"Your current in phrase is: %s\n",user->in_phrase);
    return;
    }
  strcpy(user->in_phrase,inpstr);
  write_user(user,"In phrase set.\n");
  return;
  }
if (word_count<2) {
  vwrite_user(user,"Your current out phrase is: %s\n",user->out_phrase);
  return;
  }
strcpy(user->out_phrase,inpstr);
write_user(user,"Out phrase set.\n");
}


/*** Show some user stats ***/
void status(UR_OBJECT user)
{
	UR_OBJECT u;
	int on, days, hours, mins, hs, newmail;
	char nm[5], *pp;

	set_crash();
	if (word_count<2) {
		u=user;
		on=1;
		}
	else {
		if (!(u=get_user(word[1]))) {
			if ((u=create_user())==NULL) {
				vwrite_user(user, "%s: unable to create temporary user session.\n",syserror);
				write_syslog(ERRLOG,0,"Unable to create temporary user session in status().\n");
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

	if (user->level<u->level) {
		write_user(user, "Nemozes si pozret status usera s vyssim levelom ako mas ty\n");
		if (!on) {
			destruct_user(u);
			destructed=0;
			}
		return;
		}

	write_user(user, "\n");
	vwrite_user(user, "Status pre %s~RS %s\n",
		u->recap, u->desc);
	if (on) write_user(user, "~OL.-- ~FTSystem info~FW ----~FY(on line)~FW----------------------------------- ~FTUser info ~FW----.\n");
	else write_user(user, "~OL.-- ~FTSystem info~FW ----~FB(off line)~FW---------------------------------- ~FTUser info ~FW----.\n");
	if (on) {
		sprintf(text, "site~RS       : %s:%d", u->site, u->site_port);
		vwrite_user(user, "~OL|~RS ~FG%-79.79s ~OL|\n", text);
		}
	vwrite_user(user, "~OL|~RS ~FGlast site~RS  : %-46.46s~OL|~RS ~FGlevel~RS %s         ~OL|\n",
		u->last_site, user_level[u->level].alias);
	days=u->total_login/86400;
	hours=(u->total_login%86400)/3600;
	mins=(u->total_login%3600)/60;
	hs=(int)(time(0)-u->last_login)/60;
	sprintf(text, "%dd %dh %dm", days, hours, mins);
	if (on)	vwrite_user(user, "~OL|~RS ~FGtotal login~RS: %-20.20s                          ~OL|~RS ~FGonline~RS %-5d min~OL|\n",
		text, hs);
	else vwrite_user(user, "~OL|~RS ~FGtotal login~RS: %-20.20s                          ~OL|~RS                 ~OL|\n",
		text);
	vwrite_user(user, "~OL|~RS ~FGhomepage~RS   : %-46.46s~OL|~RS ~FGgender~RS %-8.8s ~OL|\n",
		(u->homepage[0]!='#')?u->homepage:"nenastavena", sex[u->gender]);
	if (!strcmp(u->email, "#UNSET")) sprintf(text, "~FR~OLnenastaveny~RS");
	else {
		sprintf(text, "%s", u->email);
		if (u->hideemail) strcat(text, " ~FB~OL(SKRYTY)~RS");
		}
	if (u==user
	    || !u->hideemail
	    || (user->level>=ARCH && user->level>u->level)
	    ) {
		hs=colour_com_count(text);
	vwrite_user(user, "~OL|~RS ~FGe-mail~RS     : %-*.*s ~OL|\n", 63+hs*3, 63+hs*3, text);
	}

	if (u==user || user->level>=GOD) {
		write_user(user, "~OL|-- ~FTPrivat info~FW ---------------------------------------------------------------|\n");
		newmail=mail_sizes(u->name, 1);
		if (newmail) sprintf(nm, "%d", newmail);
		else sprintf(nm, "NIE");
		write_user(user, "~OL|~RS ~FGNewMail monitor FWD CharEcho wrap Pueblo riadky farby AudioPrompt xterm blind~RS~OL|\n");
		vwrite_user(user, "~OL|~RS   %-3.3s     %-3.3s   %-3.3s   %-3.3s    %-3.3s   %-3.3s     %-2.2d     %-1.1d       %-3.3s      %-3.3s   %-3.3s ~OL|\n",
			nm, offon[u->monitor], noyes2[u->autofwd], noyes2[u->terminal.checho],
			noyes2[u->terminal.wrap], noyes2[u->pueblo], u->terminal.pager, /*u->colour*/9,
			noyes2[(u->pueblo_mm && u->pueblo)], offon[u->terminal.xterm],
			offon[u->terminal.blind]);
		sprintf(text, "Killed %d people, and died %d times.  Energy : %d, Bullets : %d",
			u->kills, u->deaths, u->hps, u->bullets);
		vwrite_user(user,"~OL|~RS %-76.76s ~OL|\n", text);
		}

	if (user->level>=GOD || u==user) {
		write_user(user, "~OL|-- ~FTPhrases~FW -------------------------------------------------------------------|\n");
		vwrite_user(user, "~OL|~RS ~FGin~RS : %-71.71s~RS ~OL|\n", u->in_phrase);
		vwrite_user(user, "~OL|~RS ~FGout~RS: %-71.71s~RS ~OL|\n", u->out_phrase);
		}

	write_user(user, "~OL|-- ~FTIgnore~FW --------------------------------------------------------------------|\n");
	write_user(user, "~OL|~RS   ~FGAll    Shout    Tell    Greet    Fun    Wiz    Pic    Beep    Transport~RS    ~OL|\n");
	vwrite_user(user, "~OL|~RS   %-3.3s     %-3.3s     %-3.3s      %-3.3s     %-3.3s    %-3.3s    %-3.3s    %-3.3s       %-3.3s        ~OL|\n",
		noyes2[u->ignore.all], noyes2[u->ignore.shouts], noyes2[u->ignore.tells],
		noyes2[u->ignore.greets], noyes2[u->ignore.funs], noyes2[u->ignore.wiz],
		noyes2[u->ignore.pics], noyes2[u->ignore.beeps], noyes2[u->ignore.transp]
		);

	if (user->level>=WIZ) {
		write_user(user, "~OL|-- ~FTWiz info~FW ------------------------------------------------------------------|\n");
		vwrite_user(user, "~OL|~RS ~FGICQ~RS     : %-10.10s       ~FGvek~RS     : %-3d      ~FGtot. logins~RS: %-5d      ~FGmoneys ~FW~OL|\n",
			(u->icq[0]!='#')?u->icq:"unset", u->age, u->logons);
		vwrite_user(user, "~OL|~RS ~FGrestrict~RS: %-14.14s   ~FGbcounter~RS: %-6d   ~FGtcounter~RS   : %-5d   %-4d/%-4d ~OL|\n",
			u->restrict, u->bcount, u->tcount, u->bank, u->money);
		strcpy(text, ctime((time_t *)&u->t_expire));
		if ((pp=strchr(text, '\n'))) pp[0]='\0';
		vwrite_user(user, "~OL|~RS ~FGexpire~RS  : %-3.3s  o %-26.26s   ~FGverified~RS   : %-3.3s               ~OL|\n",
			noyes2[u->expire], text, noyes2[u->mail_verified]);
		}

	write_user(user, "~OL|-- ~FTOther info~FW ----------------------------------------------------------------|\n");
	vwrite_user(user, "~OL|~RS   ~FGJailLev~RS: %-3.3s      ~FGUnJailLev~RS: %-3.3s      ~FGUnMuzzLev~RS: %-3.3s      ~FGprilep~RS %-3.3s       ~OL|\n",
		(u->arrestby)?user_level[u->arrestby].alias:"not",
		user_level[u->unarrest].alias,
		(u->muzzled)?user_level[u->muzzled].alias:"not",
		noyes2[(u->lroom==2)]
		);
	write_user(user, "~OL`------------------------------------------------------------------------------'\n");

	if (!on) {
		destruct_user(u);
		destructed=0;
		}
}


/*** Enter user profile ***/
void enter_profile(UR_OBJECT user, int done_editing)
{
FILE *fp;
char *c,filename[500];

	set_crash();
if (!done_editing) {
  write_user(user, profile_edit_header);
  user->misc_op=5;
  editor(user,NULL);
  return;
  }
sprintf(filename,"%s/%s.P", USERPROFILES,user->name);
if (!(fp=fopen(filename,"w"))) {
  vwrite_user(user,"%s: couldn't save your profile.\n",syserror);
  write_syslog(ERRLOG,1,"Couldn't open file %s to write in enter_profile().\n",filename);
  return;
  }
c=user->malloc_start;
while(c!=user->malloc_end) putc(*c++,fp);
fclose(fp);
write_user(user,"Profile stored.\n");
}


/*** Examine a user ***/
void examine(UR_OBJECT user)
{
	UR_OBJECT u;
	FILE *fp;
	char filename[500], text2[ARR_SIZE], line[182];
	int on,days,hours,mins,timelen,days2,hours2,mins2,idle,cnt,newmail;
	int prf;

	set_crash();
	if (word_count<2) {
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

	days=u->total_login/86400;
	hours=(u->total_login%86400)/3600;
	mins=(u->total_login%3600)/60;
	timelen=(int)(time(0) - u->last_login);
	days2=timelen/86400;
	hours2=(timelen%86400)/3600;
	mins2=(timelen%3600)/60;
	prf=strncmp("-noprofile", word[2], strlen(word[2]));
	if (word_count<3) prf=1;

	write_user(user,"\n+----------------------------------------------------------------------------+\n");
	sprintf(text2,"%s~RS %s",u->recap,u->desc);
	cnt=colour_com_count(text2);
	vwrite_user(user,"Name   : %-*.*s~RS Level : %s\n",45+cnt*3,45+cnt*3,text2,user_level[u->level].name);
	if (!on) {
		vwrite_user(user,"Last login : %s",ctime((time_t *)&(u->last_login)));
		vwrite_user(user,"Which was  : %d day%s, %d hour%s, %d minute%s ago\n",days2,PLTEXT_S(days2),hours2,PLTEXT_S(hours2),mins2,PLTEXT_S(mins2));
		vwrite_user(user,"Was on for : %d hours, %d minutes\nTotal login: %d day%s, %d hour%s, %d minute%s\n",
		u->last_login_len/3600,(u->last_login_len%3600)/60,days,PLTEXT_S(days),hours,PLTEXT_S(hours),mins,PLTEXT_S(mins));
		if (user->level>=WIZ) vwrite_user(user,"Last site  : %s\n",u->last_site);
		if ((newmail=mail_sizes(u->name,1))) vwrite_user(user,"%s~RS has unread mail (%d).\n",u->recap,newmail);
		write_user(user,"+----- ~OL~FTProfile~RS --------------------------------------------------------------+\n\n");
		if (prf) {
			sprintf(filename,"%s/%s.P", USERPROFILES,u->name);
			if (!(fp=fopen(filename,"r"))) write_user(user, no_profile_prompt);
			else {
				fgets(line, 81, fp);
				while (!feof(fp)) {
					replace_string(line, "{name}", user->name);
					replace_string(line, "{nameg}", user->nameg);
					replace_string(line, "{named}", user->named);
					replace_string(line, "{namea}", user->namea);
					replace_string(line, "{namel}", user->namel);
					replace_string(line, "{namei}", user->namei);
					replace_string(line, "{namex}", user->namex);
					replace_string(line, "{namey}", user->namey);
					replace_string(line, "{namez}", user->namez);
					replace_string(line, "{level}", user_level[user->level].name);
					write_user(user, line);
					fgets(line, 81, fp);
					}
				fclose(fp);
				}
			}
		else write_user(user, "Preskakujem profil\n");
		write_user(user,"+----------------------------------------------------------------------------+\n\n");
		destruct_user(u);
		destructed=0;
		return;
		}
	idle=(int)(time(0) - u->last_input)/60;
	if (u->editing)
		vwrite_user(user,"Ignoring all: ~FTUsing Line Editor~RS\n");
	else
		vwrite_user(user,"Ignoring all: %s\n",noyes2[u->ignore.all]);
	vwrite_user(user,"On since    : %sOn for      : %d hour%s, %d minute%s\n",ctime((time_t *)&u->last_login),hours2,PLTEXT_S(hours2),mins2,PLTEXT_S(mins2));
	if (u->afk) {
		vwrite_user(user,"Idle for    : %d minute%s ~BR(AFK)\n",idle,PLTEXT_S(idle));
		if (u->afk_mesg[0]) vwrite_user(user,"AFK message : %s\n",u->afk_mesg);
		}
	else vwrite_user(user,"Idle for    : %d minute%s\n",idle,PLTEXT_S(idle));
	vwrite_user(user,"Total login : %d day%s, %d hour%s, %d minute%s\n",days,PLTEXT_S(days),hours,PLTEXT_S(hours),mins,PLTEXT_S(mins));
	if (u->socket>=1) {
		if (user->level>=WIZ)
			vwrite_user(user,"Site        : %-40.40s  Port : %d\n",u->site,u->site_port);
#ifdef NETLINKS
		else vwrite_user(user,"Home service: %s\n",u->netlink->service);
#endif
		}
	if ((newmail=mail_sizes(u->name,1))) vwrite_user(user,"%s~RS has unread mail (%d).\n",u->recap,newmail);
	write_user(user,"+----- ~OL~FTProfile~RS --------------------------------------------------------------+\n\n");
	if (prf) {
		sprintf(filename,"%s/%s.P", USERPROFILES,u->name);
		if (!(fp=fopen(filename,"r"))) write_user(user, no_profile_prompt);
		else {
			fgets(line, 81, fp);
			while(!feof(fp)) {
				replace_string(line, "{name}", user->name);
				replace_string(line, "{nameg}", user->nameg);
				replace_string(line, "{named}", user->named);
				replace_string(line, "{namea}", user->namea);
				replace_string(line, "{namel}", user->namel);
				replace_string(line, "{namei}", user->namei);
				replace_string(line, "{namex}", user->namex);
				replace_string(line, "{namey}", user->namey);
				replace_string(line, "{namez}", user->namez);
				replace_string(line, "{level}", user_level[user->level].name);
				write_user(user, line);
				fgets(line, 81, fp);
				}
			fclose(fp);
			}
		}
	else write_user(user, "Preskakujem profil\n");
	write_user(user,"+----------------------------------------------------------------------------+\n\n");
	if (user->level<=u->level && user!=u)
		vwrite_user(u, "%s si ta oscannoval%s\n", user->name, grm_gnd(4, user->gender));
}


/*** Change users password. Only GODs and above can change another users 
     password and they do this by specifying the user at the end. When this is 
     done the old password given can be anything, the wiz doesnt have to know it
     in advance. ***/
void change_pass(UR_OBJECT user)
{
UR_OBJECT u;
char *name;

	set_crash();
if (word_count<3) {
  if (user->level<GOD) write_usage(user,"passwd <stare heslo> <nove heslo>");
  else write_usage(user,"passwd <stare heslo> <nove heslo> [<user>]");
  return;
  }
if (strlen(word[2])<PASS_MIN_LEN) {
  write_user(user,"New password too short.\n");  return;
  }
if (strlen(word[2])>PASS_LEN) {
  write_user(user,"New password too long.\n");  return;
  }
/* Change own password */
if (word_count==3) {
  if (strcmp((char *)crypt(word[1],crypt_salt),user->pass)) {
    write_user(user,"Old password incorrect.\n");  return;
    }
  if (!strcmp(word[1],word[2])) {
    write_user(user,"Old and new passwords are the same.\n");  return;
    }
  strcpy(user->pass,(char *)crypt(word[2],crypt_salt));
  save_user_details(user,0);
  cls(user);
  write_user(user,"Password changed.\n");
  add_history(user->name,1,"Changed passwords.\n");
  return;
  }
/* Change someone elses */
if (user->level<GOD) {
  write_user(user,"You are not a high enough level to use the user option.\n");  
  return;
  }
word[3][0]=toupper(word[3][0]);
if (!strcmp(word[3],user->name)) {
  /* security feature  - prevents someone coming to a wizes terminal and 
     changing his password since he wont have to know the old one */
  write_user(user,"You cannot change your own password using the <user> option.\n");
  return;
  }
if ((u=get_user(word[3]))) {
#ifdef NETLINKS
  if (u->type==REMOTE_TYPE) {
    write_user(user,"You cannot change the password of a user logged on remotely.\n");
    return;
    }
#endif
  if (u->level>=user->level) {
    write_user(user,"You cannot change the password of a user of equal or higher level than yourself.\n");
    return;
    }
  strcpy(u->pass,(char *)crypt(word[2],crypt_salt));
  cls(user);
  vwrite_user(user,"%s's password has been changed.\n",u->name);
  if (user->vis) name=user->name; else name=invisname;
  vwrite_user(u,"~FR~OLYour password has been changed by %s!\n",name);
  write_syslog(SYSLOG,1,"%s changed %s's password.\n",user->name,u->name);
  sprintf(text,"Forced password change by %s.\n",user->name);
  add_history(u->name,0,text);
  return;
  }
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user object in change_pass().\n");
  return;
  }
strcpy(u->name,word[3]);
if (!load_user_details(u)) {
  write_user(user,nosuchuser);   
  destruct_user(u);
  destructed=0;
  return;
  }
if (u->level>=user->level) {
  write_user(user,"You cannot change the password of a user of equal or higher level than yourself.\n");
  destruct_user(u);
  destructed=0;
  return;
  }
strcpy(u->pass,(char *)crypt(word[2],crypt_salt));
save_user_details(u,0);
destruct_user(u);
destructed=0;
cls(user);
vwrite_user(user,"%s's password changed to \"%s\".\n",word[3],word[2]);
sprintf(text,"Forced password change by %s.\n",user->name);
add_history(u->name,0,text);
write_syslog(SYSLOG,1,"%s changed %s's password.\n",user->name,u->name);
}


/*** Set user visible or invisible ***/
void visibility(UR_OBJECT user, int vis)
{
	set_crash();
if (vis) {
  if (user->vis) {
    write_user(user,"You are already visible.\n");  return;
    }
  write_user(user, appear_user_prompt);
  vwrite_room_except(user->room, user,appear_prompt, user->bw_recap);
  user->vis=1;
  return;
  }
if (!user->vis) {
  write_user(user,"You are already invisible.\n");  return;
  }
write_user(user, disapear_user_prompt);
vwrite_room_except(user->room,user, disapear_prompt, user->bw_recap);
user->vis=0;
}


/*** Set the character mode echo on or off. This is only for users logging in
     via a character mode client, those using a line mode client (eg unix
     telnet) will see no effect. ***/
void toggle_charecho(UR_OBJECT user)
{
	set_crash();
	if (!user->terminal.checho) {
		write_user(user,"Echoing for character mode clients ~FGON~RS.\n");
		user->terminal.checho=1;
		}
	else {
		write_user(user,"Echoing for character mode clients ~FROFF~RS.\n");
		user->terminal.checho=0;
		}
	if (user->room==NULL) prompt(user);
}



/*** A newbie is requesting an account. Get his email address off him so we
     can validate who he is before we promote him and let him loose as a 
     proper user. ***/
void account_request(UR_OBJECT user, char *inpstr)
{
	set_crash();
if (user->level>NEW) {
  write_user(user,"Tento prikaz je len pre novych juzerov.\n");
  return;
  }
/* stop them from requesting an account twice */
if (BIT_TEST(user->accreq,3)) {
  write_user(user,"Uz mas podany request.\n");
  return;
  }
if (word_count<2) {
  write_usage(user,"accreq <email + nejake info>");
  return;
  }
if (!validate_email(inpstr)) {
  write_user(user,"That email address format is incorrect.  Correct format: user@network.net\n");
  return;
  }
write_syslog(REQLOG,1,"%-*s : %s\n",USER_NAME_LEN,user->name,inpstr);
sprintf(text,"~OLSYSTEM:~RS %s poziadal%s o konto.\n",
	user->name, grm_gnd(4, user->gender));
write_level(WIZ,1,text,NULL);
write_user(user,"Account request logged.\n");
add_history(user->name,1,"Made a request for an account.\n");
if (strlen(word[1])>80) write_user(user, "Pre nastavenie adresy pre autoforwrd pouzi ~FG.set email\n");
else (strcpy(user->email, word[1]));
set_forward_email(user);
}


/*** Do AFK ***/
void afk(UR_OBJECT user, char *inpstr)
{
	int blind=0;

	set_crash();
if (word_count>1 || inpstr!=NULL) {
  if (!strcmp(word[1],"lock")) {
#ifdef NETLINKS
    if (user->type==REMOTE_TYPE) {
      /* This is because they might not have a local account and hence
	 they have no password to use. */
      write_user(user,"Sorry, due to software limitations remote users cannot use the lock option.\n");
      return;
      }
#endif
    inpstr=remove_first(inpstr);
			if (!strcmp(word[2], "-b")) {
				inpstr=remove_first(inpstr);
				blind=1;
				cls(user);
				}
    if (strlen(inpstr)>AFK_MESG_LEN) {
      write_user(user,"AFK message too long.\n");
      return;
      }
    write_user(user,"You are now AFK with the session locked, enter your password to unlock it.\n");
    if (inpstr[0]) {
      strcpy(user->afk_mesg,inpstr);
      write_user(user,"AFK message set.\n");
      }
    user->afk=2;
    user->status='A';
    echo_off(user);
    if (blind) user->terminal.blind=1;
    }
  else {
    if (strlen(inpstr)>AFK_MESG_LEN) {
      write_user(user,"AFK message too long.\n");  return;
      }
    write_user(user,"You are now AFK, press <return> to reset.\n");
    if (inpstr[0]) {
      strcpy(user->afk_mesg,inpstr);
      write_user(user,"AFK message set.\n");
      }
    user->afk=1;
    user->status='A';
    }
  }
else {
  write_user(user,"Odteraz si STAND BY, stlac <return> na reset.\n");
  user->afk=1;
  user->status='A';
  }
if (user->vis) {
  if (user->afk_mesg[0]) vwrite_room_except(user->room,user,"%s~RS goes AFK: %s\n",user->recap,user->afk_mesg);
  else vwrite_room_except(user->room,user,"%s~RS goes AFK...\n",user->recap);
  }
clear_afk(user);
}


/**** Allow a user to delete their own account ***/
void suicide(UR_OBJECT user)
{
	set_crash();
	if (word_count<2) {
		write_usage(user,"suicide <heslo>");
		return;
		}
	if (user->restrict[RESTRICT_SUIC]==restrict_string[RESTRICT_SUIC]) {
		write_user(user,">>>You cannot use this feature...\n");
		return;
		}
	if (strcmp((char *)crypt(word[1],crypt_salt),user->pass)) {
		write_user(user,"Password incorrect.\n");
		return;
		}
	audioprompt(user, 4, 0);
	write_user(user,"\n\07~FR~OL~LI*** WARNING - This will delete your account! ***\n\nAre you sure about this (y/n)? ");
	user->misc_op=6;  
	no_prompt=1;
}


/** Force a user to become visible **/
void make_vis(UR_OBJECT user)
{
UR_OBJECT user2;

	set_crash();
if (word_count<2) {
  write_usage(user,"makevis <user>");
  return;
  }
if (!(user2=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);  return;
  }
if (user==user2) {
  write_user(user,"There is an easier way to make yourself visible!\n");
  return;
  }
if (user2->vis) {
  vwrite_user(user,"%s~RS is already visible.\n",user2->recap);
  return;
  }
if (user2->level>user->level) {
  vwrite_user(user,"%s~RS cannot be forced visible.\n",user2->recap);
  return;
  }
user2->vis=1;
vwrite_user(user,"You force %s~RS to become visible.\n",user2->recap);
write_user(user2,"You have been forced to become visible.\n");
vwrite_room_except(user2->room,user2,"You see %s~RS mysteriously emerge from the shadows!\n",user2->recap);
}


/** Force a user to become invisible **/
void make_invis(UR_OBJECT user)
{
UR_OBJECT user2;

	set_crash();
if (word_count<2) {
  write_usage(user,"makeinvis <user>");
  return;
  }
if (!(user2=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);  return;
  }
if (user==user2) {
  write_user(user,"There is an easier way to make yourself invisible!\n");
  return;
  }
if (!user2->vis) {
  vwrite_user(user,"%s~RS is already invisible.\n",user2->recap);
  return;
  }
if (user2->level>user->level) {
  vwrite_user(user,"%s~RS cannot be forced invisible.\n",user2->recap);
  return;
  }
user2->vis=0;
vwrite_user(user,"You force %s~RS to become invisible.\n",user2->recap);
write_user(user2,"You have been forced to become invisible.\n");
vwrite_room_except(user2->room,user2,"You see %s~RS mysteriously disappear into the shadows!\n",user2->recap);
}


/* Shows when a user was last logged on */
void show_last_login(UR_OBJECT user)
{
UR_OBJECT u;
int timelen,days,hours,mins,i;
char line[ARR_SIZE],tmp[ARR_SIZE];

	set_crash();
if (word_count>2) {
  write_usage(user,"last [<user>]");
  return;
  }
/* if checking last on a user */
if (word_count==2) {
  word[1][0]=toupper(word[1][0]);
  if (!strcmp(word[1],user->name)) {
    write_user(user,"You are already logged on!\n");
    return;
    }
  if ((u=get_user(word[1]))) {
    vwrite_user(user,"%s is currently logged on.\n",u->name);
    return;
    }
  /* User not logged in */
  if ((u=create_user())==NULL) {
    vwrite_user(user,"%s: unable to create temporary user object.\n",syserror);
    write_syslog(ERRLOG,1,"Unable to create temporary user object in show_last().\n");
    return;
    }
  strcpy(u->name,word[1]);
  if (!load_user_details(u)) {
    write_user(user,nosuchuser);
    destruct_user(u);
    destructed=0;
    return;
    }
  timelen=(int)(time(0)-u->last_login);
  days=timelen/86400;
  hours=(timelen%86400)/3600;
  mins=(timelen%3600)/60;
  write_user(user,"\n+----------------------------------------------------------------------------+\n");
  vwrite_user(user,"| ~FT~OLLast login details of %-52s~RS |\n",u->name);
  write_user(user,"+----------------------------------------------------------------------------+\n");
  strcpy(tmp,ctime((time_t *)&(u->last_login)));
  tmp[strlen(tmp)-1]='\0';
  vwrite_user(user,"| Was last logged in %-55s |\n",tmp);
  sprintf(tmp,"Which was %d day%s, %d hour%s and %d minute%s ago",days,PLTEXT_S(days),hours,PLTEXT_S(hours),mins,PLTEXT_S(mins));
  vwrite_user(user,"| %-74s |\n",tmp);
  sprintf(tmp,"Was on for %d hour%s and %d minute%s",
	  u->last_login_len/3600,PLTEXT_S(u->last_login_len/3600),(u->last_login_len%3600)/60,PLTEXT_S((u->last_login_len%3600)/60));
  vwrite_user(user,"| %-74s |\n",tmp);
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  destruct_user(u);
  destructed=0;
  return;
  } /* end if */
/* if checking all of the last users to log on */
/* get each line of the logins and check if that user is still on & print out the result. */
write_user(user,"\n+----------------------------------------------------------------------------+\n");
write_user(user,"| ~FT~OLThe last users to have logged in~RS                                           |\n");
write_user(user,"+----------------------------------------------------------------------------+\n");
for (i=0;i<LASTLOGON_NUM;i++) {
  if (!last_login_info[i].name[0]) continue;
  if (last_login_info[i].on && (!(u=get_user(last_login_info[i].name))->vis && user->level<WIZ)) continue;
  sprintf(line,"%s %s",last_login_info[i].name,last_login_info[i].time);
  if (last_login_info[i].on) sprintf(text,"| %-67s ~OL~FYONLINE~RS |\n",line);
  else sprintf(text,"| %-74s |\n",line);
  write_user(user,text);
  } /* end for */
write_user(user,"+----------------------------------------------------------------------------+\n\n");
return;
}


/* Create an account for a user if new users from their site have been
   banned and they want to log on - you know they aint a trouble maker, etc */
void create_account(UR_OBJECT user)
{
UR_OBJECT u;
int i;

	set_crash();
if (word_count<3) {
  write_usage(user,"create <meno> <heslo>");
  return;
  }
/* user logged on */
if ((u=get_user(word[1]))!=NULL) {
  write_user(user,"You cannot create with the name of an existing user!\n");
  return;
  }
/* user not logged on */
if ((u=create_user())==NULL) {
  vwrite_user(user,"%s: unable to create temporary user session.\n",syserror);
  write_syslog(ERRLOG,1,"Unable to create temporary user session in create_account().\n");
  return;
  }
if (strlen(word[1])>USER_NAME_LEN) {
  write_user(user,"Name was too long - account not created.\n");
  destruct_user(u);  destructed=0;  return;
  }
if (strlen(word[1])<USER_MIN_LEN) {
  write_user(user,"Name was too short - account not created.\n");
  destruct_user(u);  destructed=0;  return;
  }
for (i=0;i<strlen(word[1]);++i) {
  if (!isalpha(word[1][i])) {
    write_user(user,"You can't have anything but letters in the name - account not created.\n\n");
    destruct_user(u);  destructed=0;  return;
    }
  }
if (contains_swearing(word[1])) {
  write_user(user,"You cannot use a name like that - account not created.\n\n");
  destruct_user(u);  destructed=0;  return;
  }
if (strlen(word[2])>PASS_LEN) {
  write_user(user,"Password was too long - account not created.\n");
  destruct_user(u);  destructed=0;  return;
  }
if (strlen(word[2])<PASS_MIN_LEN) {
  write_user(user,"Password was too short - account not created.\n");
  destruct_user(u);  destructed=0;  return;
  }
strcpy(u->name,word[1]);
if (!load_user_details(u)) {
  strcpy(u->pass,(char *)crypt(word[2],crypt_salt));
  strcpy(u->recap,u->name);
  strcpy(u->desc,"is a newbie needing a desc.");
  strcpy(u->in_phrase,"wanders in.");
  strcpy(u->out_phrase,"wanders out");
  strcpy(u->last_site,"created_account");
  strcpy(u->site,u->last_site);
  strcpy(u->logout_room,"<none>");
  strcpy(u->verify_code,"#NONE");
  strcpy(u->email,"#UNSET");
  strcpy(u->homepage,"#UNSET");
  strcpy(u->icq,"#UNSET");
  u->prompt=amsys->prompt_def;
  u->terminal.checho=0;
  u->room=room_first;
  u->level=NEW;
  u->unarrest=NEW;
  strcpy(u->restrict, RESTRICT_MASK);
  amsys->level_count[u->level]++;
  amsys->user_count++;
  save_user_details(u,0);
  add_user_node(u->name,u->level);
  add_user_date_node(user->name,(long_date(1)));
  sprintf(text,"Was manually created by %s.\n",user->name);
  add_history(u->name,1,text);
  vwrite_user(user,"You have created an account with the name \"~FT%s~RS\" and password \"~FG%s~RS\".\n",u->name,word[2]);
  write_syslog(SYSLOG,1,"%s created a new account with the name '%s'\n",user->name,u->name);
  destruct_user(u);
  destructed=0;
  return;
  }
write_user(user,"You cannot create an account with the name of an existing user!\n");
destruct_user(u);
destructed=0;
}



/*****************************************************************************/
/* Doplnene funkcie                                                          */
/*****************************************************************************/

void set_ualarm(UR_OBJECT user)
{
	set_crash();
	if (word_count<2) {
		write_usage(user,"alarm stop/<cas v min>");
		return;
		}
	if (!strcmp(word[1],"stop")) {
		if (user->alarm) {
			write_user(user,"Budik je odteraz vypnuty\n");
			user->alarm=0;
			user->atime=0;
			return;
			}
		else {
			write_user(user,"Ved nemas zapnuty budik !\n");
			return;
			}
		}
	if ((is_number(word[1])) && (user->alarm)) {
		write_user(user,"Ved uz mas nastaveny budik\n");
		vwrite_user(user, "Bude ti zvonit za %d s\n", user->atime);
		return;
		}
	if (!(is_number(word[1]))) {
		write_user(user,"Nekorektny parameter\n");
		return;
		}
	user->alarm=1;
	user->atime=60*(atoi(word[1]));
	vwrite_user(user, "Budik nastaveny na %d min\n", user->atime/60);
}


void set_follow(UR_OBJECT user)
{

	set_crash();
	if (word_count<2) {
		write_usage(user, "follow [<user>]|[-cancel]");
		if (!user->follow) write_user(user, "Momentalne nikoho nesledujes\n");
		else vwrite_user(user, "Momentalne sledujes %s\n", user->follow->name);
		return;
		}
	if (!strcmp(word[1], "-cancel")) {
		if (!user->follow) {
			write_user(user, "Ved nikoho nesledujes !\n");
			return;
			}
		vwrite_user(user, "Odteraz uz nesledujes %s\n", user->follow->name);
		user->follow=NULL;
		return;
		}
	if (user->lroom==2) {
		vwrite_user(user, "Si prilepen%s - nemozes nikoho sledovat 1\n", grm_gnd(1, user->gender));
		return;
		}
	if (user->restrict[RESTRICT_GO]==restrict_string[RESTRICT_GO]) {
		write_user(user, ">>>You cannot have acces to another sky...\n");
		return;
		}
	if ((user->follow=get_user_name(user, word[1]))==NULL) {
		write_user(user, notloggedon);
		return;
		}
	if (user->follow==user) {
		write_user(user, "Sledovat seba ? Salies ?\n");
		return;
		}
	vwrite_user(user, "Odteraz sledujes %s\n", user->follow->name);
}

#endif /* ct_user.c */
