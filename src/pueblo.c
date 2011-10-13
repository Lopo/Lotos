/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
            Funkcie Lotos v1.2.0 suvisiace s podporou Pueblo klienta
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/
/*****************************************************************************
        POZOR !!! Zatial iba experimentalne - nerucim za funkcnost !!!!!
 *****************************************************************************/

#ifndef __PUEBLO_C__
#define __PUEBLO_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#ifdef NETLINKS
#	include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "pueblo.h"
#include "music.h"
#include "comvals.h"


/* Online init. of pueblo.  (In case the login doesn't do it.) */
int chck_pblo(UR_OBJECT user, char *str)
{
	set_crash();
if (user->pueblo==1) {
        user->pblodetect=0;
        if (!strncmp(str,"PUEBLOCLIENT",12)) return 1;
                else return 0;
        }
if (!user->pblodetect) {
        if (!strncmp(str,"PUEBLOCLIENT",12)) return 1;
                else return 0;
        }
if (!strncmp(str,"PUEBLOCLIENT",12)) {
                user->pueblo=1;
                user->pblodetect=0;
                write_user(user,"</xch_mudtext><font size=+2 color=\"#FFFFFF\">Pueblo has been detected!</font><xch_mudtext>\n");
                return 1;
                }
user->pueblo=0;
user->pblodetect=0;
return 0;
}


int contains_pueblo(char *str)
{
	char *s;

	set_crash();
	if ((s=(char *)malloc(strlen(str)+1))==NULL) {
		write_syslog(ERRLOG, 1, "Failed to allocate memory in contains_pueblo().\n");
		return 0;
		}
	strcpy(s,str);
	strtolower(s); 
/* check to see if it contains a pueblo flag or a http:// so we don't
   convert "/~username/file.ext" into thinking its a color command. */
	if (strstr(s,"</xch_mudtext>")) { free(s); return 1; }
	if (strstr(s,"http://")) { free(s); return 1; }
	free(s);
	return 0;
}


void click_rm_access(UR_OBJECT user)
{
UR_OBJECT u;
RM_OBJECT rm;
char *name;
int cnt;

	set_crash();
        if ((rm=get_room(word[3]))==NULL) {
                write_user(user,nosuchroom);  return;
                }
        if (user->level<amsys->gatecrash_level && rm!=user->room) {
                write_user(user,"You have left that room.  Cannot perform access change.\n");
                return;
                }
        if (rm->access==PERSONAL_UNLOCKED || rm->access==PERSONAL_UNLOCKED) {
                write_user(user,"You cannot change the access of a personal room.\n");
                return;
                }
if (user->vis) name=user->name; else name=invisname;
if (rm->access>PRIVATE) {
        if (rm==user->room) 
                write_user(user,"This room's access is fixed.\n"); 
        else write_user(user,"That room's access is fixed.\n");
        return;
        }
if (!strcmp(word[2],"PUBLIC") && rm->access==PUBLIC) {
        if (rm==user->room) 
                write_user(user,"This room is already public.\n");  
        else write_user(user,"That room is already public.\n"); 
        return;
        }
if (user->vis) name=user->name; else name=invisname;
if (!strcmp(word[2],"PRIVATE")) {
        if (rm->access==PRIVATE) {
                if (rm==user->room) 
                        write_user(user,"This room is already private.\n");  
                else write_user(user,"That room is already private.\n"); 
                return;
                }
        cnt=0;
        for(u=user_first;u!=NULL;u=u->next) if (u->room==rm) ++cnt;
        if (cnt<amsys->min_private_users && user->level<amsys->ignore_mp_level) {
                sprintf(text,"You need at least %d users/clones in a room before it can be made private.\n", amsys->min_private_users);
                write_user(user,text);
                return;
                }
        write_user(user,"Room set to ~FRPRIVATE.\n");
        if (rm==user->room) {
                sprintf(text,"%s has set the room to ~FRPRIVATE.\n",name);
                write_room_except(rm,text,user);
                }
        else write_room(rm,"This room has been set to ~FRPRIVATE.\n");
        rm->access=PRIVATE;
        return;
        }
write_user(user,"Room set to ~FGPUBLIC.\n");
if (rm==user->room) {
        sprintf(text,"%s has set the room to ~FGPUBLIC.\n",name);
        write_room_except(rm,text,user);
        }
else write_room(rm,"This room has been set to ~FGPUBLIC.\n");
rm->access=PUBLIC;

/* Reset any invites into the room & clear review buffer */
for(u=user_first;u!=NULL;u=u->next) {
        if (u->invite_room==rm) u->invite_room=NULL;
        }
clear_revbuff(rm);
}


/* Executes all the main features. */
void pblo_exec(UR_OBJECT user, char *inpstr)
{
	set_crash();
	inpstr=remove_first(inpstr);
	if (!user->pueblo) return;
	if (!strcmp(word[1],"audioSTOP")) {
		write_user(user,"</xch_mudtext><img xch_sound=stop><xch_mudtext>");
		return;
		}
	if (!strcmp(word[1],"rmAccess")) {
		click_rm_access(user);
		return;
		}
if (!strcmp(word[1],"RoomConfig_setOpt")) {
        if (user->room->access==PERSONAL_UNLOCKED || user->room->access==PERSONAL_LOCKED) {
        	write_user(user,"Personal rooms do not have access controls.\n");
        	return;
        	}
		if (user->room->access==ROOT_CONSOLE) {
			write_user(user, "Root console do not have access controls.\n");
			return;
			}
        if ( user->level < command_table[FIX].level
                && user->level < command_table[UNFIX].level
                && user->level < command_table[CTOPIC].level
                && user->level < command_table[REVCLR].level
                && user->level < command_table[MYPAINT].level) {
                        if ( user->level >= command_table[PUBCOM].level
                             && user->level >= command_table[PRIVCOM].level) {
                                clear_words();
                                if (user->room->access==PUBLIC) { com_num=PRIVCOM; set_room_access(user); return; }
                                if (user->room->access==PRIVATE) { com_num=PUBCOM;  set_room_access(user); return; }
                                }
                        return;
                        }
        sprintf(text,"\n ---- Room Configuration Options : %s ----\n\n",user->room->name);  write_user(user,text);
        if (user->level >= command_table[FIX].level && user->level >= command_table[UNFIX].level) {
		vwrite_user(user, "        Room Type:  </xch_mudtext><b><a xch_cmd=\".%s %s\" xch_hint=\"Set this room's type to FIXED access.\">FIXED</a> / <a xch_cmd=\".%s %s\" xch_hint=\"Set this room's type to VARIABLE access.\">VARIABLE</a></b><xch_mudtext>\n", command_table[FIX].name, command_table[UNFIX].name, user->room->name,user->room->name);
		}
        if (user->level >= command_table[PUBCOM].level && user->level >= command_table[PRIVCOM].level) {
		vwrite_user(user, "           Access:  </xch_mudtext><b><a xch_cmd=\".pbloenh rmAccess PUBLIC %s\" xch_hint=\"Set this room's access to PUBLIC.\">PUBLIC</a> / <a xch_cmd=\".pbloenh rmAccess PRIVATE %s\" xch_hint=\"Set this room's access to PRIVATE.\">PRIVATE</a></b><xch_mudtext>\n", user->room->name,user->room->name);
                }
        if (user->level >= command_table[REVCLR].level) {
		vwrite_user(user, "    Review Buffer:  </xch_mudtext><b><a xch_cmd=\".%s %s\" xch_hint=\"Clear this room's speech review buffer.\">CLEAR REVIEW BUFFER NOW</a></b><xch_mudtext>\n", command_table[REVCLR].name, user->room->name);
                }                
        if (user->level >= command_table[CTOPIC].level) {
		vwrite_user(user, "       Room Topic:  </xch_mudtext><b><a xch_cmd=\".%s %s\" xch_hint=\"Clear this room's topic.\">CLEAR TOPIC NOW</a></b><xch_mudtext>\n", command_table[CTOPIC].name, user->room->name);
                }
/*        if (user->level >= command_table[MYPAINT].level
	      && user->can_edit_rooms
	      && user->room->access!=PERSONAL_UNLOCKED
	      && user->room->access!=PERSONAL_LOCKED) {
                sprintf(text," Room Description:  </xch_mudtext><b><a xch_cmd=\".rmdesc %s\" xch_hint=\"EDIT this room's description.\">EDIT DESCRIPTION</a></b><xch_mudtext>\n",user->room->name);
                write_user(user,text);
                }
*/	sprintf(text, "(%s)", user->name);
	strtolower(text);
	if (user->level >= command_table[MYPAINT].level
            && (user->room->access==PERSONAL_UNLOCKED
                || user->room->access==PERSONAL_LOCKED)
            && !strcmp(user->room->name, text)) {
                vwrite_user(user, " Room Description:  </xch_mudtext><b><a xch_cmd=\".%s\" xch_hint=\"Edit your PERSONAL ROOM description.\">EDIT DESCRIPTION</a></b><xch_mudtext>\n", command_table[MYPAINT].name);
                }
        write_user(user,"\n");
        return;
        }
if (word_count<3) return;
if (!strcmp(word[1],"vpic")) {
        vwrite_user(user, "</xch_mudtext><br><img src=\"%s\"><br><xch_mudtext>",inpstr);
        return;
        }
if (!strcmp(word[1],"audioPLAY")) {
	vwrite_user(user, "</xch_mudtext><img xch_sound=play href=\"%s\"><xch_mudtext>",inpstr);
        return;
        }
if (!strcmp(word[1],"audioVOL")) {
        if (!strcmp(word[2],"muteVOL")) write_user(user,"</xch_mudtext><img xch_volume=0><xch_mudtext>");
        if (!strcmp(word[2],"minVOL")) write_user(user,"</xch_mudtext><img xch_volume=33><xch_mudtext>");
        if (!strcmp(word[2],"medVOL")) write_user(user,"</xch_mudtext><img xch_volume=66><xch_mudtext>");
        if (!strcmp(word[2],"maxVOL")) write_user(user,"</xch_mudtext><img xch_volume=100><xch_mudtext>");
        return;
        }
}


void disp_song(UR_OBJECT user, int num)
{
	UR_OBJECT u;
	char *name;

	set_crash();
	if (user->vis) name=user->name;  else name=invisname;
	if (!user->vis) { sprintf(text,"%s[%s] ",colors[CSYSTEM],user->name); write_duty(user->level,text,user->room,user,0); }
	sprintf(text,"o/~ %s plays \"%s\" on the Jukebox. o/~\n",name,jb_titles[num]);
	write_room(user->room,text);
	sprintf(text,"~FMo/~ ~FYAudioPlayer~FM o/~~RS </xch_mudtext><b>[ <a xch_cmd=\".pbloenh audioSTOP\" xch_hint=\"Stop all audio.\">STOP</a> | <a xch_cmd=\".pbloenh audioPLAY %s%s%s\" xch_hint=\"Play this file.\">PLAY</a> |</b> Volume <b><a xch_cmd=\".pbloenh audioVOL muteVOL\" xch_hint=\"Mute\">Mute</a> : <a xch_cmd=\".pbloenh audioVOL minVOL\" xch_hint=\"Low Volume\">Soft</a> : <a xch_cmd=\".pbloenh audioVOL medVOL\" xch_hint=\"Normal Volume\">Normal</a> : <a xch_cmd=\".pbloenh audioVOL maxVOL\" xch_hint=\"High Volume\">Loud</a> ]</b><br><xch_mudtext>",reg_sysinfo[TALKERHTTP],reg_sysinfo[PUEBLOWEB],jb_files[num]);
	for (u=user_first; u!=NULL; u=u->next) {
		if (u->login || u->room!=user->room) continue;
		if (u->pueblo) write_user(u,text);
		}
}


void pblo_jukebox(UR_OBJECT user)
{
int i,cnt,pos;
	set_crash();
i=0; cnt=0; pos=0;
if (word_count<2) {
        for (i=0; jb_titles[i][0]!='*'; i++) {
                cnt++;
                if (i==0) write_user(user,"Jukebox Song List\n\n");
                sprintf(text,"%3d: %-33s ",cnt,jb_titles[i]);
                write_user(user,text);
                if (pos) { write_user(user,"\n"); pos=0; }
                else pos=1;
                }
        if (pos) write_user(user,"\n");
        if (cnt) write_user(user,"\nUsage:  .jukebox <songnumber>\n");
        else write_user(user,"There are no song titles loaded in the jukebox.\n");
        return;
        }
if (is_number(word[1])) cnt=atoi(word[1]);
else { write_usage(user,"jukebox <songnumber>\n"); return; }
for (i=0; jb_titles[i][0]!='*'; i++) {
        if (i==cnt-1) { disp_song(user,i); break; }
        }
if (i!=cnt-1) write_user(user,"Song not found.\n");
}


/* Relist EXITS ... This will work for anyone, but users with Pueblo can
                    click on the non-private rooms to go there.  Netlinks
                    are not clickable, because they sometimes require pwds. */
void pblo_listexits(UR_OBJECT user)
{
	RM_OBJECT rm;
	char temp[125];
	int i,exits=0;

	set_crash();
	rm=user->room;

if (rm->access!=PERSONAL_LOCKED && rm->access!=PERSONAL_UNLOCKED) sprintf(text,"The exits for this room (%s) are:",rm->name);
              else sprintf(text,"The exits for this personal room %s are:",rm->name);
write_user(user,text);
strcpy(text,"\n");
for(i=0;i<MAX_LINKS;++i) {
        if (rm->link[i]==NULL) break;
        if (rm->link[i]->access & PRIVATE)
                if (user->pueblo) sprintf(temp,"  ~FR%s",rm->link[i]->name);
                else sprintf(temp,"  ~FR%s",rm->link[i]->name);
        else {
                if (user->pueblo) sprintf(temp,"  </xch_mudtext><b><a xch_cmd=\".go %s\" xch_hint=\"Go to this room.\">%s</a></b><xch_mudtext>",rm->link[i]->name,rm->link[i]->name);
                else sprintf(temp,"  ~FG%s",rm->link[i]->name);
                }
        strcat(text,temp);
        ++exits;
        }
#ifdef NETLINKS
if (rm->netlink!=NULL && rm->netlink->stage==UP) {
        if (rm->netlink->allow==IN) sprintf(temp,"  ~FR%s*",rm->netlink->service);
        else sprintf(temp,"  ~FG%s*",rm->netlink->service);
        strcat(text,temp);
        }
else
#endif
	if (!exits) strcpy(text,"\n~FY(!) ~FRThere are no exits. ~FY(!)");
strcat(text,"\n");
write_user(user,text);
#ifdef NETLINKS
write_user(user," * indicates a netlink.\n\n");
#endif
}


/** ----------------------------------------------------------------------
     TalkerOS  AUDIO PROMPTER for Pueblo
    ---------------------------------------------------------------------- **/
int audioprompt(UR_OBJECT user, int prmpt, int pager)
{
UR_OBJECT u;
char audiofiles[8][30]={
        "ap_f-welcome.wav",     /* 00: Login greeting        (female)  */
        "ap_m-welcome.wav",     /* 01: Login greeting        (male)    */
        "ap_f-pager.wav",       /* 02: Pager sound           (female)  */
        "ap_m-pager.wav",       /* 03: Pager sound           (male)    */
        "ap_f-warning.wav",     /* 04: Warning sound         (female)  */
        "ap_m-warning.wav",     /* 05: Warning sound         (male)    */
        "ap_f-shutdown.wav",    /* 06: Shutdown/Reboot alert (female)  */
        "ap_m-shutdown.wav"     /* 07: Shutdown/Reboot alert (male)    */
        };

	set_crash();
if (user!=NULL) {
        /* Check user prefs. */
        if (!user->pueblo) return 0;
        if (!pager && !user->pueblo_mm) return 0;
        if (pager  && !user->pueblo_pg) return 0;

        /* Set to male voice if that is the user's preference. */
        if (user->voiceprompt) prmpt++;
        
        /* Send playback command */
        sprintf(text,"</xch_mudtext><img xch_sound=play xch_device=wav href=\"%s%s%s\"><xch_mudtext>",reg_sysinfo[TALKERHTTP],reg_sysinfo[PUEBLOWEB],audiofiles[prmpt]);
        write_user(user,text);
        return 1;
        }
/* If we get here then the announcement is to everyone. */
for (u=user_first; u!=NULL; u=u->next) {
        /* Check user prefs. */
        if (!u->pueblo || !u->pueblo_mm) continue;

        /* Send playback command */
        sprintf(text,"</xch_mudtext><img xch_sound=play xch_device=wav href=\"%s%s%s\"><xch_mudtext>",reg_sysinfo[TALKERHTTP],reg_sysinfo[PUEBLOWEB],audiofiles[prmpt+u->voiceprompt]);
        write_user(u,text);
        }
	return 0;
}


void query_img(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char *name;

	set_crash();
	if (word_count<2) {
		write_usage(user,"ppic <picture URL>");
		return;
		}
	if (amsys->ban_swearing && contains_swearing(inpstr) && user->level<MIN_LEV_NOSWR) {
		switch(amsys->ban_swearing) {
			case SBMIN:
				inpstr=censor_swear_words(inpstr);
				break;
			case SBMAX:
				write_user(user, noswearing);
				return;
			default : break; /* do nothing as ban_swearing is off */
			}
		}
	if (!strncmp(word[1],"http://",7)) {
		if (!(contains_extension(inpstr,0))) {
			write_user(user,"URL must end in either .jpg or .gif for a picture.\n");
			return;
			}
		if (user->vis) name=user->name;
		else name=invisname;
		if (!user->vis) {
			sprintf(text,"%s[%s] ",colors[CSYSTEM],user->name);
			write_duty(user->level,text,user->room,user,0);
			}
		sprintf(text,"%s suggests picture: %s\n",name,inpstr);
		write_room(user->room,text);
		sprintf(text,"-->  [ </xch_mudtext><a xch_cmd=\".pbloenh vpic %s\" xch_hint=\"View picture.\"><b>CLICK to view picture</b></a><xch_mudtext> ]\n",
			inpstr
			);
		for (u=user_first; u!=NULL; u=u->next) {
			if (!u->pueblo || u->login || u->room!=user->room)
				continue;
			write_user(u,text);
			}
		}
	else {
		write_user(user,"URL must begin with:  http://\n");
		return;
		}
}


void query_aud(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;
	char *name;

	set_crash();
	if (word_count<2) {
		write_usage(user,"paudio <soundfile URL>");
		return;
		}
	if (contains_swearing(inpstr)) {
		write_user(user,noswearing);
		return;
		}
	if (!strncmp(word[1],"http://",7)) {
		if (!(contains_extension(inpstr,1))) {
			write_user(user,"URL must end in either .wav or .mid for an audio clip.\n");
			return;
			} 
		if (user->vis) name=user->name;
		else name=invisname;
		if (!user->vis) {
			sprintf(text,"%s[%s] ",colors[CSYSTEM],user->name);
			write_duty(user->level,text,user->room,user,0);
			}
		sprintf(text,"%s suggests audio: %s\n",name,inpstr);
		write_room(user->room,text);
		sprintf(text,"~FMo/~ ~FYAudioPlayer~FM o/~~RS </xch_mudtext><b>[ <a xch_cmd=\".pbloenh audioSTOP\" xch_hint=\"Stop all audio.\">STOP</a> | <a xch_cmd=\".pbloenh audioPLAY %s\" xch_hint=\"Play this file.\">PLAY</a> |</b> Volume <b><a xch_cmd=\".pbloenh audioVOL muteVOL\" xch_hint=\"Mute\">Mute</a> : <a xch_cmd=\".pbloenh audioVOL minVOL\" xch_hint=\"Low Volume\">Soft</a> : <a xch_cmd=\".pbloenh audioVOL medVOL\" xch_hint=\"Normal Volume\">Normal</a> : <a xch_cmd=\".pbloenh audioVOL maxVOL\" xch_hint=\"High Volume\">Loud</a> ]</b><br><xch_mudtext>",
			inpstr
			);
		for (u=user_first; u!=NULL; u=u->next) {
			if (!u->pueblo || u->login || u->room!=user->room)
				continue;
			write_user(u,text);
			}
		}
	else {
		write_user(user,"URL must begin with:  http://\n");
		return;
		}
}

#endif /* pueblo.c */
