/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                Funkcie Lotos v1.2.0 na pracu s plugin modulmi
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __PLUGIN_C__
#define __PLUGIN_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#include "obj_sys.h"
#include "obj_pl.h"
#include "obj_syspp.h"

#include "plugin.h"

/* ---------------------------------------
    Begin PLUGIN INCLUDE code lines here. 
   --------------------------------------- */
#include "./plugins/eightball.c"
#include "./plugins/hangman.c"
/* -------------------------------------
    End PLUGIN INCLUDE code lines here. 
   ------------------------------------- */

void destroy_pl_cmd(CM_OBJECT c)
{
	set_crash();
/* Remove from linked list */
if (c==cmds_first) {
        cmds_first=c->next;
        if (c==cmds_last) cmds_last=NULL;
        else cmds_first->prev=NULL;
        }
else {
        c->prev->next=c->next;
        if (c==cmds_last) { 
                cmds_last=c->prev;  cmds_last->next=NULL; 
                }
        else c->next->prev=c->prev;
        }
free(c);
}

void destroy_plugin(PL_OBJECT p)
{
	set_crash();
/* Remove from linked list */
if (p==plugin_first) {
        plugin_first=p->next;
        if (p==plugin_last) plugin_last=NULL;
        else plugin_first->prev=NULL;
        }
else {
        p->prev->next=p->next;
        if (p==plugin_last) { 
                plugin_last=p->prev;  plugin_last->next=NULL; 
                }
        else p->next->prev=p->prev;
        }
free(p);
}

void save_plugin_data(UR_OBJECT user)
{
	PL_OBJECT plugin;
	int i;
	i=0;

	set_crash();
	for (plugin=plugin_first; plugin!=NULL; plugin=plugin->next) {
		if (!i && plugin->req_userfile) { call_plugin_exec(user,"",plugin,-7); i++; }
		else if (plugin->req_userfile) { call_plugin_exec(user,"",plugin,-8); i++; }
		}
}

int call_plugin_exec(UR_OBJECT user, char *str, PL_OBJECT plugin, int comnum)
{
	set_crash();
/* ---------------------------------------------------
   Put third-party plugin command calls here!
   example:  if (!strcmp(plugin->registration,"00-000")) { plugin_00x000_main(user,str,comnum); return 1; }
   --------------------------------------------------- */
	if (!strcmp(plugin->registration,"00-000")) { plugin_00x000_main(user,str,comnum); return 1; }
	if (!strcmp(plugin->registration,"02-100")) { plugin_02x100_main(user,str,comnum); return 1; }
/* ---------------------------------------------------
   End third-party plugin comand calls here.
   --------------------------------------------------- */
	return 0;
}

int oss_run_plugins(UR_OBJECT user, char *str, char *comword, int len)
{
	CM_OBJECT com;

	set_crash();
	for (com=cmds_first; com!=NULL; com=com->next) {
		if (!(!strncmp(comword,com->command,len))) continue;
		if (user->level < com->req_lev) return 0;
		if (!call_plugin_exec(user,str,com->plugin,com->comnum)) return 0;
		return 1;
		}
	return 0;
}

int oss_plugin_dump(void)
{
	PL_OBJECT p,p2;
	CM_OBJECT c,c2;

	set_crash();
	for (c=cmds_first; c!=NULL; c=c2) { c2=c->next; destroy_pl_cmd(c); }
	for (p=plugin_first; p!=NULL; p=p2) { p2=p->next; call_plugin_exec(NULL,"",p,-6); destroy_plugin(p); }
	return 1;
}

void plugin_triggers(UR_OBJECT user, char *str)
{
	PL_OBJECT plugin;

	set_crash();
	for (plugin=plugin_first; plugin!=NULL; plugin=plugin->next) {
	if (plugin->triggerable) call_plugin_exec(user,str,plugin,-5);
	}
}

void load_plugin_data(UR_OBJECT user)
{
	PL_OBJECT plugin;

	set_crash();
	for (plugin=plugin_first; plugin!=NULL; plugin=plugin->next)
		if (plugin->req_userfile) call_plugin_exec(user,"",plugin,-9);
}

void disp_plugin_registry(UR_OBJECT user)
{
PL_OBJECT plugin;
CM_OBJECT com;
int cm,total;

	set_crash();
cm=0;  total=0;
/* write data for each loaded plugin */
for (plugin=plugin_first; plugin!=NULL; plugin=plugin->next) {
        if (!total) {
                sprintf(text,"\n%s%s                      Lotos  Plugin  Module  Registry                   \n\n",colors[CHIGHLIGHT],colors[CTEXT]);
                write_user(user,text);
                write_user(user,"~FTPos: ID-Auth : Name                       : Cmds : Ver : Author\n");
                }
        /* search for associated commands */
        cm=0; for (com=cmds_first; com!=NULL; com=com->next) if (com->plugin==plugin) cm++;
        /* write data to user */
        sprintf(text,"%3d: %-7s : %-26s :  %2d  : %-3s : %-21s\n",total,plugin->registration,plugin->name,cm,plugin->ver,plugin->author);
        write_user(user,text);
        total++;
        }
if (!total) write_user(user,"No plugins are loaded on this system.\n");
else write_user(user,"\n");
}

/*** Construct plugin object ***/
PL_OBJECT create_plugin(void)
{
PL_OBJECT plugin;

	set_crash();
if ((plugin=(PL_OBJECT)malloc(sizeof(struct plugin_struct)))==NULL) {
        write_syslog(ERRLOG, 1, "Memory allocation failure in create_plugin().\n");
        return NULL;
        }

/* Append object into linked list. */
if (plugin_first==NULL) {  
        plugin_first=plugin;  plugin->prev=NULL;  
        }
else {  
        plugin_last->next=plugin;  plugin->prev=plugin_last;  
        }
plugin->next=NULL;
plugin_last=plugin;

/* initialise plugin location structure */
plugin->name[0]='\0';
plugin->author[0]='\0';
plugin->ver[0]='\0';
plugin->req_ver[0]='\0';
plugin->registration[0]='\0';
plugin->id=-1;
return plugin;
}

/* -------------------
    Plug-in Debuger
   ------------------- */
void oss_debugger(UR_OBJECT user)
{
	set_crash();
	if (word_count<2) {
		write_user(user,"Lotos DEBUGGER:  Specifikuj debug modul/parametre.\n");
		return;
		}
	if (!strncmp(word[1],"com",3)) {
		oss_debug_commands(user);
		return;
		}
	if (syspp->oss_highlev_debug && !strncmp(word[1],"inp",3)) {
		oss_debug_allinput(user);
		return;
		}
	if (!strncmp(word[1],"plug",4)) {
		oss_debug_plugindata(user);
		return;
		}
}

void oss_debug_commands(UR_OBJECT user)
{
PL_OBJECT plugin;
CM_OBJECT com;
int i,pos,num,sysN,plc,pl,total;

	set_crash();
i=0; pos=0; num=0; sysN=0; plc=0; pl=0; total=0;

for (i=0; command_table[i].name[0]!='*'; i++) {
        if (!total) write_user(user,"\n          Lotos DEBUGGER:  Command Listing by Memory Address\n\n");
        if (!pos) write_user(user,"  ");
        total++;  sysN++;  pos++;
        sprintf(text,"%10s - %3d   ",command_table[i].name,sysN);
        write_user(user,text);
        if (pos==3) { pos=0; write_user(user,"\n"); }
        }

for (plugin=plugin_first; plugin!=NULL; plugin=plugin->next) {
        if (!total) write_user(user,"\n          Lotos DEBUGGER:  Command Listing by Addressable Location\n\n");
        for (com=cmds_first; com!=NULL; com=com->next) {
                if (com->plugin != plugin) continue;
                if (!pos) write_user(user,"  ");
                total++;  plc++;  pos++;
                sprintf(text,"%10s - %3dx%-4d   ",com->command,com->comnum,plugin->id);
                write_user(user,text);
                if (pos==3) { pos=0; write_user(user,"\n"); }
                }
        pl++;
        }
if (pos!=4) write_user(user,"\n");
sprintf(text,"\nDebuger:  Addressable command search finished.\n           Najdenych spolu %d prikazov.\n           %d Lotos  /  %d z %d plugin-ov.\n\n",total,sysN,plc,pl);
write_user(user,text);
}

void oss_debug_allinput(UR_OBJECT user)
{
	set_crash();
if (syspp->debug_input) {
        syspp->debug_input=0;  syspp->highlev_debug_on--;
        write_syslog(SYSLOG, 1, "%s deaktivoval Debuger systemoveho vstupu.\n",user->name);
        sprintf(text,"\n%s%sSYSTEM Debuger:  Logovanie vstupu bolo deaktivovane.\n\n",colors[CBOLD],colors[CSYSBOLD]);
        write_room_except(NULL,text,user);
        write_user(user,"Debuger:  Logovanie vstupu bolo deaktivovane.\n");
        }
else {
        syspp->debug_input=1;  syspp->highlev_debug_on++;
        write_syslog(SYSLOG, 1, "%s activated the System Input Debugger.\n",user->name);
        sprintf(text,"\n%s%sSYSTEM Debugger:  Input logging has been activated!\n                  NOTE:  *ALL* commands, speech, and text are being\n                         logged for debugging and testing purposes.\n\n",colors[CBOLD],colors[CSYSBOLD]);
        write_room_except(NULL,text,user);
        write_user(user,"Debugger:  Input logging has been activated!\n           ~OLNOTE: If left on, this command will *rapidly* fill up the system log.\n");
        }
}

void oss_debug_plugindata(UR_OBJECT user)
{
PL_OBJECT plugin;
CM_OBJECT cmd;
int num,cnt,found;

	set_crash();
num = -1;       cnt = 0;       found = 0;       plugin = plugin_first;
if (word_count<3) {
	write_user(user,"DEBUG PluginData requires the plugin position to retrieve data.\n");
	return;
	}
if ((num=atoi(word[2]))==-1) {
	write_user(user,"DEBUG PluginData requires the NUMERICAL position for data retrieval.\n");
	return;
	}
write_user(user,"Lotos DEBUGGER:  Retrieving addressable plugin information by location...\n\n");
if (plugin_first!=NULL) {
        for (plugin=plugin_first; plugin!=NULL; plugin=plugin->next) {
                if (cnt==num) { found++; break; }
                cnt++;
                }
        }        
if (!found) { write_user(user,"No addressable plugins found in system memory.  Check .plugreg for details.\n"); return; }
sprintf(text,"Plugin and Associated Commands data for registry position: %d\n\n",cnt);
write_user(user,text);
sprintf(text,"Registered ID: %-7s                            Local ID      : %3d\n",plugin->registration,plugin->id);
write_user(user,text);
sprintf(text,"Author Name  : %-22s             Plugin Version: %3s\n",plugin->author,plugin->ver);
write_user(user,text);
sprintf(text,"Description  : %-27s        Int Ver. Req'd: %3s\n",plugin->name,plugin->req_ver);
write_user(user,text);
sprintf(text,"Uses Userdata: %-3s                                Triggerable   : %-3s\n\n",noyes2[plugin->req_userfile],noyes2[plugin->triggerable]);
write_user(user,text);
cnt=0;
for (cmd=cmds_first; cmd!=NULL; cmd=cmd->next) {
        if (cmd->plugin != plugin) continue;
        cnt++;
        write_user(user,"   Associated Command\n   ------------------\n");
        sprintf(text,"   Command ID: %3d                        Full Address: %3dx%d\n",cmd->comnum,cmd->comnum,cmd->id);
        write_user(user,text);
        sprintf(text,"   Name      : %-20s       Level Req'd : %s\n\n",cmd->command,user_level[cmd->req_lev].name);
        write_user(user,text);
        }
if (!cnt) write_user(user,"   (No commands associated with this plugin.)\n");
write_user(user,"Debugger:  Search finished.\n");
}

void oss_debug_alert(UR_OBJECT user)
{
	set_crash();
if (syspp->highlev_debug_on) {
        write_user(user,"~OL~FYATTENTION!!!     ATTENTION!!!   ATTENTION!!!\n\n");
        write_user(user,"Higher-level debug features are currently active on this\n");
        write_user(user,"system.  You are being informed for privacy reasons.  Do\n");
        write_user(user,"NOT send private information or change your password until\n");
        write_user(user,"this message no longer appears when you log in.  See the\n");
        write_user(user,"~OL.tinfo~RS command for status information.\n\n");
        }
}

CM_OBJECT create_cmd(void)
{
CM_OBJECT cmd;

	set_crash();
if ((cmd=(CM_OBJECT)malloc(sizeof(struct plugin_cmd)))==NULL) {
        write_syslog(ERRLOG, 1, "Memory allocation failure in create_cmd().\n");
        return NULL;
        }

/* Append object into linked list. */
if (cmds_first==NULL) {  
        cmds_first=cmd;  cmd->prev=NULL;  
        }
else {  
        cmds_last->next=cmd;  cmd->prev=cmds_last;  
        }
cmd->next=NULL;
cmds_last=cmd;

/* initialise plugin command structure */
cmd->command[0]='\0';
cmd->id=-1;
cmd->comnum=-1;
cmd->req_lev=99;
cmd->plugin=NULL;
return cmd;
}


void load_plugins(void)
{
	int tmp=0, cmd=0;
	set_crash();
/* --------------------------------------------------
   Place third-party plugin initialization calls HERE
   Ex: if ((tmp=plugin_00x000_init(cmd))) cmd=cmd+tmp;
   -------------------------------------------------- */
	if ((tmp=plugin_00x000_init(cmd))) cmd=cmd+tmp;
	if ((tmp=plugin_02x100_init(cmd))) cmd=cmd+tmp;
/* ------------------------------------------------
   End third-party plugin initialization calls HERE
   ------------------------------------------------ */
}


#endif /* plugin.c */
