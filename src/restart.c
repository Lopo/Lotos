/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                  Funkcie pre Lotos v1.2.0 - restart talkra
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __RESTART_C__
#define __RESTART_C__ 1

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "obj_syspp.h"
#include "restart.h"


int reinit_save_user_malloc(UR_OBJECT user)
{
	FILE *fp;
	char fname[500], *p=user->malloc_start;

	set_crash();
	sprintf(fname, "%s/%s.ri_urm", TEMPFILES, user->name);
	if ((fp=fopen(fname, "w"))==NULL) return 0;
	while (p!=user->malloc_end) {
		fputc(p[0], fp);
		p++;
		}
	fclose(fp);
	return 1;
}


int reinit_save_user(UR_OBJECT user)
{
	UR_OBJECT u;
	PL_OBJECT pl;
	FILE *fp;
	char fname[500];
	int i;

	set_crash();
	if (user->type==REMOTE_TYPE || user->type==CLONE_TYPE) return 0;
	sprintf(fname, "%s/%s.ri_ur", TEMPFILES, user->name);
	if ((fp=fopen(fname, "w"))==NULL) {
		vwrite_user(user, "%s: chyba pri ukladani tvojich detailov\n", syserror);
		write_syslog(ERRLOG, 1, "REINIT_SAVE_USER: chyba pri ukladani detailov pre %s\n", user->name);
		return 0;
		}
	fprintf(fp, "level      %d %d\n", user->level, user->real_level);
	fprintf(fp, "site       %s %s %s\n", user->last_site, user->site, user->ipsite);
	fprintf(fp, "afk        %d %s\n", user->afk, user->afk_mesg);
	fprintf(fp, "call       %s\n", user->call);
	fprintf(fp, "gcoms ");
	for (i=0; i<MAX_GCOMS; i++) fprintf(fp, "%d ", user->gcoms[i]);
	fprintf(fp, "\n");
	fprintf(fp, "xcoms ");
	for (i=0; i<MAX_XCOMS; i++) fprintf(fp, "%d ", user->xcoms[i]);
	fprintf(fp, "\n");
	fprintf(fp, "pueblo     %d %d\n", user->pueblo, user->pblodetect);
	fprintf(fp, "alarm      %d %d\n", user->atime, user->alarm);
	fprintf(fp, "ltell      %s\n", user->ltell);
	fprintf(fp, "count      %ld %ld\n", user->tcount, user->bcount);
	fprintf(fp, "auth       %lu\n", user->auth_addr);
	fprintf(fp, "afkbuff\n");
		for (i=0; i<REVTELL_LINES; i++) {
			if (user->afkbuff[i][0]!='\0')
				fprintf(fp, "%s", user->afkbuff[i]);
			}
	fprintf(fp, "/afkbuff\n");
	fprintf(fp, "buff       %d\n%s\n/buff\n", strlen(user->buff), user->buff);
	fprintf(fp, "ops        %d %d %d %d\n", user->misc_op, user->edit_op, user->set_op, user->set_mode);
	fprintf(fp, "hwrap      %d %d %d %d %d\n", user->hwrap_lev, user->hwrap_id, user->hwrap_same, user->hwrap_func, user->hwrap_pl);
	fprintf(fp, "page       %d %d %d %s\n", user->pagecnt, user->user_page_pos, user->user_page_lev, user->page_file);
	fprintf(fp, "pages\n");
		for (i=0; i<MAX_PAGES; i++) fprintf(fp, "%d ", user->pages[i]);
	fprintf(fp, "\n/pages\n");
	fprintf(fp, "pos        %d %d\n", user->filepos, user->buffpos);
	fprintf(fp, "revbuff\n");
		for (i=0; i<REVTELL_LINES; i++) {
			if (user->revbuff[i][0]!='\0')
				fprintf(fp, "%s", user->revbuff[i]);
			}
	fprintf(fp, "/revbuff\n");
	fprintf(fp, "inpstr     %d %s\n", strlen(user->inpstr_old), user->inpstr_old);
	fprintf(fp, "copyto\n");
		for (i=0; i<MAX_COPIES; i++) {
			if (user->copyto[i][0]!='\0')
				fprintf(fp, "%s", user->copyto[i]);
			}
	fprintf(fp, "/copyto\n");
	fprintf(fp, "invite     %s\n", user->invite_by);
	fprintf(fp, "ign_ur\n");
		for (i=0; i<MAX_IGNORES; i++) {
			if (user->ignoreuser[i][0]!='\0')
				fprintf(fp, "%s\n", user->ignoreuser[i]);
			}
	fprintf(fp, "/ign_ur\n");
	fprintf(fp, "editbuff\n");
		for (i=0; i<REVTELL_LINES; i++) {
			if (user->editbuff[i][0]!='\0')
				fprintf(fp, "%s", user->editbuff[i]);
			}
	fprintf(fp, "/editbuff\n");
	fprintf(fp, "samesite   %d %d %s\n", user->samesite_all_store, strlen(user->samesite_check_store), user->samesite_check_store);
	fprintf(fp, "ignall     %d %d\n", user->ignore.all, user->ignore.all_store);
	fprintf(fp, "lines      %d %d %d %d\n", user->edit_line, user->editline, user->revline, user->afkline);
	fprintf(fp, "others     %d %d %d %d %d %ld %d %c %d %d\n",
		user->warned, user->editing, user->charcnt, user->lmail_lev, user->remote_com,
		(long)user->last_input, user->tmp_int, user->status, user->clone_hear, user->kradnutie);
	fprintf(fp, "wipe       %d %d\n", user->wipe_from, user->wipe_to);
	fprintf(fp, "restrict   %s\n", user->restrict);
	fprintf(fp, "room       %s\n", user->room!=NULL?user->room->name:"");
	fprintf(fp, "invite_rm  %s\n", user->invite_room!=NULL?user->invite_room->name:"");
	fprintf(fp, "wrap_rm    %s\n", user->wrap_room!=NULL?user->wrap_room->name:"");
	fprintf(fp, "remind     %d %d %d %d %s\n", user->temp_remind.day, user->temp_remind.month, user->temp_remind.year, strlen(user->temp_remind.msg), user->temp_remind.msg!='\0'?user->temp_remind.msg:"");
	fprintf(fp, "p_tmp_ch   %s\n", user->p_tmp_ch!=NULL?user->p_tmp_ch:"");
	fprintf(fp, "follow     %s\n", user->follow!=NULL?user->follow->name:"");
	fprintf(fp, "clones\n");
		for (u=user_first; u!=NULL; u=u->next) {
			if (u->type!=CLONE_TYPE) continue;
			if (u->owner==user) fprintf(fp, "%s\n", u->room->name);
			}
	fprintf(fp, "/clones\n");
	if (user->malloc_start!=NULL) {
		fprintf(fp, "malloc\n");
		reinit_save_user_malloc(user);
		}
	fclose(fp);
	for (pl=plugin_first; pl!=NULL; pl=pl->next) {
		call_plugin_exec(user, "", pl, -1);
		}

	return 1;
}


int reinit_save_room(RM_OBJECT room)
{
	FILE *fp;
	char fname[500];
	int i;

	set_crash();
	sprintf(fname, "%s/%s.ri_rm", TEMPFILES, room->name);
	if ((fp=fopen(fname, "w"))==NULL) {
		write_syslog(ERRLOG, 1, "REINIT_SAVE_ROOM: chyba pri ukladani detailov pre %s\n", room->name);
		return 0;
		}
	fprintf(fp, "topic      %s\n", room->topic);
	fprintf(fp, "revbuff\n");
		for (i=0; i<REVTELL_LINES; i++) {
			if (room->revbuff[i][0]!='\0')
				fprintf(fp, "%s", room->revbuff[i]);
			}
	fprintf(fp, "/revbuff\n");
	fprintf(fp, "access     %d\n", room->access);
	fprintf(fp, "revline    %d\n", room->revline);
	fprintf(fp, "mesg_cnt   %d\n", room->mesg_cnt);
	if (room->transp)
		fprintf(fp, "transport  %d %d %d %d %d %d\n",
			room->transp->place, room->transp->route, room->transp->out,
			room->transp->go, room->transp->smer, room->transp->time);
	fclose(fp);
	return 1;
}


void restart(UR_OBJECT user)
{
	UR_OBJECT u,wu;
	RM_OBJECT rm;
#ifdef NETLINKS
	NL_OBJECT nl;
#endif
	FILE *fp;
	char name[ROOM_NAME_LEN+1];
	char *argy[]={progname, confile, "-reinit", NULL };
	int p;

	set_crash();
	clear_temps();
	write_room_except(NULL, restart_prompt, user);
	save_counters();
	write_syslog(SYSLOG, 1, "%s robi ~OL~FTrestart~RS talkra\n", user->name);

	if ((fp=fopen(RESTARTFILE, "w"))==NULL) {
		write_user(user, "Nemozem otvorit subor pre zoznam, nerestartujem ...\n");
		write_syslog(ERRLOG, 1, "Nemozem otvorit RESTARTFILE na zapis v restart()\n");
		write_syslog(SYSLOG, 1, "Restart zruseny - pozri errlog\n");
		return;
		}

#ifdef NETLINKS
	for (nl=nl_first; nl!=NULL; nl=nl->next) shutdown_netlink(nl);
#endif
	fprintf(fp,"%d %d %d %d\n",port[0], port[1], listen_sock[0],listen_sock[1]);

	for (rm=room_first; rm!=NULL; rm=rm->next) {
		p=is_personal_room(rm);
		if (p) {
			sscanf(rm->name, "(%s", name);
			name[strlen(name)-1]='\0';
			personal_room_store(name, 1, rm);
			}
		fprintf(fp, "%s %d\n", rm->name, p);
		reinit_save_room(rm);
		}

	fprintf(fp, "_users\n");
	u=user_first;
	while (u!=NULL) {
		wu=u->next;
		if (u->type==CLONE_TYPE || u->login) {
			u=wu;
			continue;
			}
		fprintf(fp,"%s %d %d %d\n",u->name,u->port,u->socket,u->site_port);
		reinit_save_user(u);
		save_user_details(u,1);
		u=wu;
		}
	fclose(fp);
	u=user_first;
	while (u!=NULL) {
		wu=u->next;
		if (u->login) disconnect_user(u);
		else destruct_user(u);
		u=wu;
		}
	oss_plugin_dump();
	

#ifdef NETLINKS
	close(listen_sock[2]);
#endif
	execvp(progname,argy);
	exit(12);
}

int reinit_load_user_malloc(UR_OBJECT user)
{
	FILE *fp;
	char fname[500], c;
	int i=0;

	set_crash();
	sprintf(fname, "%s/%s.ri_urm", TEMPFILES, user->name);
	if ((fp=fopen(fname, "r"))==NULL) return 0;
	if ((user->malloc_start=(char *)malloc(MAX_LINES*81))==NULL) return 0;
	while ((c=fgetc(fp))!=EOF) {
		user->malloc_start[i]=c;
		i++;
		}
	user->malloc_end=user->malloc_start+i;
	fclose(fp);
	return 1;
}


/* stage: 1-main, 2-plugins */
int reinit_load_user(UR_OBJECT user, int stage)
{
	RM_OBJECT rm;
	UR_OBJECT ur;
	PL_OBJECT pl;
	FILE *fp;
	char fname[500], line[ARR_SIZE*5], *str, s[50];
	char ur_words[10][ARR_SIZE];
	int i, wn, wpos, wcnt, op, found, c, damaged=0;
	char *options[]={
		"level", "site", "afk", "call",	"gcoms",
		"xcoms", "pueblo", "alarm", "ltell", "count",
		"auth", "afkbuff", "buff", "ops", "hwrap",
		"page", "pages", "pos", "revbuff", "inpstr",
		"copyto", "invite", "ign_ur", "editbuff", "samesite",
		"ignall", "lines", "others", "wipe", "restrict",
		"room", "invite_rm", "wrap_rm", "remind", "p_tmp_ch",
		"follow", "clones", "malloc",
		"*"};

	set_crash();
if (stage==1) {
	sprintf(fname, "%s/%s.ri_ur", TEMPFILES, user->name);
	if ((fp=fopen(fname, "r"))==NULL) return 0;
	fgets(line, (ARR_SIZE*5)-1, fp);
	line[strlen(line)-1]='\0';

	while (!feof(fp)) {
		wn=0; wpos=0;
		str=line;
		do {
			while (*str<33) if (!*str++) goto RLUOUT;
			while (*str>32 && wpos<ARR_SIZE) ur_words[wn][wpos++]=*str++;
			ur_words[wn++][wpos]='\0';
			wpos=0;
			} while (wn<1010);
		wn--;
RLUOUT:
	wcnt=wn;
	op=0; found=1;
	while (strcmp(options[op], ur_words[0])) {
		if (options[op][0]=='*') {
			found=0;
			break;
			}
		op++;
		}
	if (found) {
		switch (op) {
			case  0: /* level */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->level=atoi(ur_words[i]); break;
						case 2: user->real_level=atoi(ur_words[i]); break;
						}
					}
				break;
			case  1: /* site */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: strcpy(user->last_site, ur_words[i]); break;
						case 2: strcpy(user->site, ur_words[i]); break;
						case 3: strcpy(user->ipsite, ur_words[i]); break;
						}
					}
				break;
			case  2: /* afk */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->afk=atoi(ur_words[i]); break;
						case 2: strncpy(user->afk_mesg, remove_first(remove_first(line)), AFK_MESG_LEN); break;
						}
					}
				break;
			case  3: /* call */
				if (wcnt>=2) strncpy(user->call, remove_first(line), USER_NAME_LEN);
				break;
			case  4: /* gcoms */
				for (i=1; i<wcnt; i++)
					user->gcoms[i-1]=atoi(ur_words[i]);
				break;
			case  5: /* xcoms */
				for (i=1; i<wcnt; i++)
					user->xcoms[i-1]=atoi(ur_words[i]);
				break;
			case  6: /* pueblo */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->pueblo=atoi(ur_words[i]); break;
						case 2: user->pblodetect=atoi(ur_words[i]); break;
						}
					}
				break;
			case  7: /* alarm */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->atime=atoi(ur_words[i]); break;
						case 2: user->alarm=atoi(ur_words[i]); break;
						}
					}
				break;
			case  8: /* ltell */
				if (wcnt>=2) strncpy(user->ltell, remove_first(line), USER_NAME_LEN);
				break;
			case  9: /* count */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->tcount=atol(ur_words[i]); break;
						case 2: user->bcount=atol(ur_words[i]); break;
						}
					}
				break;
			case 10: /* auth */
				if (wcnt>=2) user->auth_addr=strtoul(remove_first(line), NULL, 10);
				break;
			case 11: /* afkbuff */
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/afkbuff\n")) {
					if (c<REVTELL_LINES) strncpy(user->afkbuff[c], line, REVIEW_LEN+1);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 12: /* buff */
				c=atoi(ur_words[2]);
				fgets(line, ARR_SIZE*5-1, fp);
				i=0;
				while (strncmp(line, "/buff", 5)) {
					if (i<BUFSIZE) strncat(user->buff, line, BUFSIZE-strlen(user->buff)-1);
					i+=strlen(line);
					fgets(line, ARR_SIZE*5-1, fp);
					}
				user->buff[c-1]='\0';
				break;
			case 13: /* ops */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->misc_op=atoi(ur_words[i]); break;
						case 2: user->edit_op=atoi(ur_words[i]); break;
						case 3: user->set_op=atoi(ur_words[i]); break;
						case 4: user->set_mode=atoi(ur_words[i]); break;
						}
					}
				break;
			case 14: /* hwrap */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->hwrap_lev=atoi(ur_words[i]); break;
						case 2: user->hwrap_id=atoi(ur_words[i]); break;
						case 3: user->hwrap_same=atoi(ur_words[i]); break;
						case 4: user->hwrap_func=atoi(ur_words[i]); break;
						case 5: user->hwrap_pl=atoi(ur_words[i]); break;
						}
					}
				break;
			case 15: /* page */
				strcpy(line, remove_first(line));
				for (i=1; i<(wcnt-1); i++) {
					strcpy(line, remove_first(line));
					switch (i) {
						case 1: user->pagecnt=atoi(ur_words[i]); break;
						case 2: user->user_page_pos=atoi(ur_words[i]); break;
						case 3: user->user_page_lev=atoi(ur_words[i]); break;
						}
					}
				if (wcnt>4) strcpy(user->page_file, line);
				break;
			case 16: /* pages */
				fscanf(fp, "%s ", s);
				i=0;
				while (strcmp(s, "/pages\n") && i<MAX_PAGES) {
					user->pages[i]=atoi(s);
					i++;
					fscanf(fp, "%s ", s);
					}
				break;
			case 17: /* pos */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->filepos=atoi(ur_words[i]); break;
						case 2: user->buffpos=atoi(ur_words[i]); break;
						}
					}
				break;
			case 18: /* revbuff */
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/revbuff\n")) {
					if (c<REVTELL_LINES) strncpy(user->revbuff[c], line, REVIEW_LEN+1);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 19: /* inpstr */
				strncpy(user->inpstr_old, remove_first(remove_first(line)), atoi(ur_words[1]));
				break;
			case 20: /* copyto */
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/copyto\n")) {
					if (c<MAX_COPIES) strncpy(user->revbuff[c], line, USER_NAME_LEN);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 21: /* invite */
				if (wcnt>=2) strncpy(user->invite_by, remove_first(line), USER_NAME_LEN);
				break;
			case 22: /* ign_ur */
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/ign_ur\n")) {
					if (c<MAX_IGNORES) {
						strncpy(user->ignoreuser[c], line, USER_NAME_LEN);
						user->ignoreuser[c][strlen(user->ignoreuser[c])-1]='\0';
						}
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 23: /* editbuff */
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/editbuff\n")) {
					if (c<REVTELL_LINES) strncpy(user->editbuff[c], line, REVIEW_LEN+1);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 24: /* samesite */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->samesite_all_store=atoi(ur_words[i]); break;
						case 3: strncpy(user->samesite_check_store, ur_words[i], atoi(ur_words[i-1])); break;
						}
					}
				break;
			case 25: /* ignall */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->ignore.all=atoi(ur_words[i]); break;
						case 2: user->ignore.all_store=atoi(ur_words[i]); break;
						}
					}
				break;
			case 26: /* lines */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->edit_line=atoi(ur_words[i]); break;
						case 2: user->editline=atoi(ur_words[i]); break;
						case 3: user->revline=atoi(ur_words[i]); break;
						case 4: user->afkline=atoi(ur_words[i]); break;
						}
					}
				break;
			case 27: /* others */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case  1: user->warned=atoi(ur_words[i]); break;
						case  2: user->editing=atoi(ur_words[i]); break;
						case  3: user->charcnt=atoi(ur_words[i]); break;
						case  4: user->lmail_lev=atoi(ur_words[i]); break;
						case  5: user->remote_com=atoi(ur_words[i]); break;
						case  6: user->last_input=(time_t)strtoul(ur_words[i], NULL, 10); break;
						case  7: user->tmp_int=atoi(ur_words[i]); break;
						case  8: user->status=ur_words[i][0]; break;
						case  9: user->clone_hear=atoi(ur_words[i]); break;
						case 10: user->kradnutie=atoi(ur_words[i]); break;
						}
					}
				break;
			case 28: /* wipe */
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->wipe_from=atoi(ur_words[i]); break;
						case 2: user->wipe_to=atoi(ur_words[i]); break;
						}
					}
				break;
			case 29: /* restrict */
				if (wcnt>=2) strncpy(user->restrict, remove_first(line), MAX_RESTRICT);
				break;
			case 30: /* room */
				if (wcnt>=2) {
						rm=get_room_full(ur_words[1]);
						if (rm!=NULL) user->room=rm;
						}
					break;
				case 31: /* invite_rm */
					if (wcnt>=2) {
						rm=get_room_full(ur_words[2]);
						if (rm!=NULL) user->invite_room=rm;
						}
					break;
				case 32: /* wrap_rm */
					if (wcnt>=2) {
						rm=get_room_full(ur_words[2]);
						if (rm!=NULL) user->wrap_room=rm;
						}
					break;
				case 33: /* remind */
					for (i=1; i<(wcnt-1); i++) {
						strcpy(line, remove_first(line));
						switch (i) {
							case 1: user->temp_remind.day=atoi(ur_words[i]); break;
							case 2: user->temp_remind.month=atoi(ur_words[i]); break;
							case 3: user->temp_remind.year=atoi(ur_words[i]); break;
							case 4: strncpy(user->temp_remind.msg, remove_first(line), atoi(ur_words[i])); break;
							}
						strcpy(line, remove_first(line));
						}
					break;
				case 34: /* p_tmp_ch */
					if (wcnt>=2)
						user->p_tmp_ch=strdup(ur_words[2]);
					break;
				case 35: /* follow */
					if (wcnt>=2) {
						ur=get_user(ur_words[2]);
						if (ur!=NULL) user->follow=ur;
						}
					break;
				case 36: /* clones */
					fgets(line, ARR_SIZE*5-1, fp);
					while (strcmp(line, "/clones\n")) {
						line[strlen(line)-1]='\0';
						rm=get_room_full(line);
						if (rm!=NULL) {
							if ((ur=create_user())!=NULL) {
								ur->type=CLONE_TYPE;
								ur->socket=user->socket;
								ur->room=rm;
								ur->owner=user;
								ur->vis=1;
								strcpy(ur->name, user->name);
								strcpy(ur->recap, user->name);
								strcpy(ur->bw_recap, colour_com_strip(ur->recap));
								strcpy(ur->desc, clone_desc);
								}
							}
						fgets(line, ARR_SIZE*5-1, fp);
						}
					break;
				case 37: /* malloc */
					reinit_load_user_malloc(user);
				break;
			}
		}
	else damaged++;
	fgets(line, ARR_SIZE*5-1, fp);
	line[strlen(line)-1]='\0';
	}
	fclose(fp);
	return 1;
	} /* stage 1 */

else if (stage==2) {
		for (pl=plugin_first; pl!=NULL; pl=pl->next)
			for (ur=user_first; ur!=NULL; ur=ur->next)
				call_plugin_exec(ur, "", pl, -2);
		return 1;
		} /* stage 2 */
else return 0;
}


int reinit_load_room(RM_OBJECT room)
{
	FILE *fp;
	char fname[500], line[ARR_SIZE*5], *str;
	char rm_words[10][ARR_SIZE];
	int i, found, wn, wpos, wcnt, c, op, damaged=0;
	char *options[]={
		"topic", "revbuff", "access", "revline", "mesg_cnt",
		"transport",
		"*"
		};

	set_crash();
	sprintf(fname, "%s/%s.ri_rm", TEMPFILES, room->name);
	if ((fp=fopen(fname, "r"))==NULL) return 0;
	fgets(line, (ARR_SIZE*5)-1, fp);
	line[strlen(line)-1]='\0';

	while (!feof(fp)) {
		wn=0; wpos=0;
		str=line;
		do {
			while (*str<33) if (!*str++) goto RLROUT;
			while (*str>32 && wpos<ARR_SIZE) rm_words[wn][wpos++]=*str++;
			rm_words[wn++][wpos]='\0';
			wpos=0;
			} while (wn<1010);
		wn--;
RLROUT:
		wcnt=wn;
		op=0; found=1;
		while (strcmp(options[op], rm_words[0])) {
			if (options[op][0]=='*') {
				found=0;
				break;
				}
			op++;
			}
		if (found) {
			switch (op) {
				case  0:
					strcpy(room->topic, remove_first(line));
					break;
				case  1:
					fgets(line, ARR_SIZE*5-1, fp);
					c=0;
					while (strcmp(line, "/revbuff\n")) {
						if (c<REVTELL_LINES) strncpy(room->revbuff[c], line, REVIEW_LEN+1);
						c++;
						fgets(line, ARR_SIZE*5-1, fp);
						}
					break;
				case  2:
					if (wcnt>=2) room->access=atoi(rm_words[1]);
					else room->access=PERSONAL_UNLOCKED;
					break;
				case  3:
					if (wcnt>=2) room->revline=atoi(rm_words[1]);
					break;
				case  4:
					if (wcnt>=2) room->mesg_cnt=atoi(rm_words[1]);
					break;
				case  5:
					for (i=1; i<wcnt; i++) {
						switch (i) {
							case 1: room->transp->place=atoi(rm_words[i]); break;
							case 2: room->transp->route=atoi(rm_words[i]); break;
							case 3: room->transp->out=atoi(rm_words[i]); break;
							case 4: room->transp->go=atoi(rm_words[i]); break;
							case 5: room->transp->smer=atoi(rm_words[i]); break;
							case 6: room->transp->time=atoi(rm_words[i]); break;
							}
						}
					break;
				}
			}
		else damaged++;
		fgets(line, ARR_SIZE*5-1, fp);
		line[strlen(line)-1]='\0';
		}
	fclose(fp);
	return 1;
}


void reinit_sockets(void)
{
	FILE *fp;
	struct sockaddr_in bind_addr;
	int i,on,size;

	set_crash();
	fp=fopen(RESTARTFILE, "r");
	fscanf(fp, "%d %d %d %d", &port[0], &port[1], &listen_sock[0], &listen_sock[1]);
	fclose(fp);

	printf("reInitialising sockets on ports: %d a %d\n",port[0],port[1]);
	on=1;
	size=sizeof(struct sockaddr_in);
	bind_addr.sin_family=AF_INET;
	bind_addr.sin_addr.s_addr=INADDR_ANY;

	for (i=0; i<2; ++i) {
		setsockopt(listen_sock[i],SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
		bind_addr.sin_port=htons(port[i]);
		if (listen(listen_sock[i],10)==-1) boot_exit(i+8);
		fcntl(listen_sock[i],F_SETFL,O_NDELAY);
		}

#ifdef NETLINKS
	printf("Initialising socket on port: %d\n", port[2]);
	if ((listen_sock[2]=socket(AF_INET,SOCK_STREAM,0))==-1) boot_exit(4);
	/* allow reboots on port even with TIME_WAITS */
	setsockopt(listen_sock[2],SOL_SOCKET,SO_REUSEADDR,(char *)&on,sizeof(on));
	/* bind sockets and set up listen queues */
	bind_addr.sin_port=htons(port[2]);
	if (bind(listen_sock[2],(struct sockaddr *)&bind_addr,size)==-1) boot_exit(7);
	if (listen(listen_sock[2],10)==-1) boot_exit(10);
	/* Set to non-blocking */
	fcntl(listen_sock[2],F_SETFL,O_NDELAY);
#endif
}

void restore_structs(void)
{
	UR_OBJECT u;
	RM_OBJECT rm;
	FILE *fp;
	char meno[USER_NAME_LEN+1], line[200], inps[REVIEW_LEN];
	int sock=0, sport, siz, jeholen, mport;

	set_crash();
	if ((fp=fopen(RESTARTFILE, "r"))==NULL) {
		write_syslog(ERRLOG, 1, "nemozem otvorit RESTARTFILE na citanie v restore_structs()\n");
		exit(0);
		}
	fgets(line, 199, fp); /* prvy riadok s portami */

	fgets(line, 199, fp);
	while (!feof(fp)) {
		if (!strcmp(line, "_users\n")) break;
		sscanf(line, "%s %d\n", meno, &siz);
		if (siz) {
			if (amsys->personal_rooms) {
				if ((rm=create_room())==NULL)
					write_syslog(ERRLOG, 1, "nemozem vytvorit roomu v restore_structs()\n");
				else {
					strcpy(rm->name, meno);
					sscanf(meno, "(%s", meno);
					meno[strlen(meno)-1]='\0';
					personal_room_store(meno, 0, rm);
					reinit_load_room(rm);
					}
				}
			}
		else {
			for (rm=room_first; rm!=NULL; rm=rm->next) {
				if (!strcmp(rm->name, meno)) break;
				}
			reinit_load_room(rm);
			}
		fgets(line, 199, fp);
		}

	siz=sizeof(struct sockaddr_in);
	fgets(line, 199, fp);
	while (!feof(fp)) {
		sscanf(line, "%s %d %d %d %s\n", meno, &mport, &sock, &sport, inps);
		u=create_user();
		strcpy(u->name,meno);
		load_user_details(u);
		if (u->level==L_0) {
			u->room=get_room_full(default_jail);
			if (u->room==NULL) u->room=room_first;
			}
		else check_start_room(u);

		u->port=mport;
		jeholen=1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&jeholen, sizeof(jeholen));
		u->socket=sock;
		u->site_port=sport;
		syspp->acounter[3]++;
		syspp->acounter[u->gender]++;
		reinit_load_user(u, 1);
		prompt(u);
		record_last_login(u->name);
		fgets(line, 199, fp);
		}

	fclose(fp);
	unlink(RESTARTFILE);
/* reload plugins */
	for (u=user_first; u!=NULL; u=u->next)
		reinit_load_user(u, 2);
	write_room(NULL, restart_ok);
}


#endif /* restart.c */
