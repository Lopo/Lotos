/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                   Funkcie Lotos v1.2.0 na pracu s makrami
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __MACROS_C__
#define __MACROS_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_mc.h"
#include "macros.h"


MC_OBJECT create_macro(void)
{
	MC_OBJECT mc;
	
	set_crash();
	if ((mc=(MC_OBJECT)malloc(sizeof(struct macro_struct)))==NULL) {
		write_syslog(ERRLOG, 1, "Chyba alokacie pamate v create_macro().\n");
		return NULL;
		}
	mc->name[0]='\0';
	mc->comstr[0]='\0';
	mc->next=NULL;
	mc->prev=NULL;
	return mc;
}


void save_macros(UR_OBJECT user)
{
	MC_OBJECT mc;
	FILE *fp;
	char filename[ARR_SIZE];

	set_crash();
	if (user->first_macro==NULL) return;

	sprintf(filename, "%s/%s.MAC", USERMACROS, user->name);

	if ((fp=fopen(filename, "w"))==NULL) {
		write_user(user, "SYSTEM: Chyba pri zapise makier do suboru\n");
		write_syslog(ERRLOG, 1, "Chyba pri zapise makier do suboru %s\n", filename);
		return;
		}
	for (mc=user->first_macro; mc!=NULL; mc=mc->next) {
		fprintf(fp, "%s %s\n", mc->name, mc->comstr);
		}
	fclose(fp);
}


/* Get user's macros */
void get_macros(UR_OBJECT user)
{
	FILE *fp;
	int l_len=(MC_NAME_LEN+MC_COM_LEN+2);
	MC_OBJECT mc;
	char filename[500], line[ARR_SIZE*2];
	
	set_crash();
	if (user->type==REMOTE_TYPE) return;
	sprintf(filename,"%s/%s.MAC",USERMACROS,user->name);

	if ((fp=fopen(filename, "r"))==NULL) {
		return;
		}
	fgets(line, l_len, fp);
	
	while(!feof(fp)) {
		line[strlen(line)-1]='\0';
		if ((mc=create_macro())==NULL) {
			fclose(fp);
			return;
			}
		if (user->first_macro==NULL) {
			user->first_macro=mc;
			mc->prev=NULL;
			}
		else {
			user->last_macro->next=mc;
			mc->prev=user->last_macro;
			}
		mc->next=NULL;
		user->last_macro=mc;

		sscanf(line, "%s ", mc->name);
		strcpy(mc->comstr, remove_first(line));
		fgets(line, l_len, fp);
		}
	fclose(fp);
}


void show_macros(UR_OBJECT user)
{
	int cnt;
	MC_OBJECT mc;

	set_crash();
	write_user(user,"~OLTvoje aktualne makra:\n");
	if (user->first_macro==NULL) {
		write_user(user, "\nNemas ziadne makro\n");
		return;
		}
	cnt=0;
	for (mc=user->first_macro; mc!=NULL; mc=mc->next) {
		cnt++;
		vwrite_user(user, "%*.*s~RS  %s~RS\n", MC_NAME_LEN, MC_NAME_LEN, mc->name, mc->comstr);
		}
	vwrite_user(user, "~OLCelkovy pocet makier: %d\n", cnt);
}


void delete_macro(UR_OBJECT user, MC_OBJECT mc)
{
	set_crash();
	if (mc==user->first_macro) {
		user->first_macro=mc->next;
		if (mc==user->last_macro) user->last_macro=NULL;
		else user->first_macro->prev=NULL;
		}
	else {
		mc->prev->next=mc->next;
		if (mc==user->last_macro) {
			user->last_macro=mc->prev;
			user->last_macro->next=NULL;
			}
		else mc->next->prev=mc->prev;
		}
	free(mc);
}


/*** Manipulacia s makrami ***/
void macros(UR_OBJECT user, char *inpstr)
{
	MC_OBJECT mc;
	char *p, *n, *c;
	int i;
	set_crash();
#ifdef NETLINKS
	if (user->type==REMOTE_TYPE) {
		write_user(user,"Due to software limitations, remote users cannot have macros.\n");
		return;
		}
#endif
	if (word_count<2) {
		show_macros(user);
		return;
		}

	p=inpstr;
	i=0;
	while (*p!='=' && *p) {
		if (!isalpha(*p)) {
			if (i!=0 || (i==0 && *p!='.')) {
				vwrite_user(user, "V nazve mozes mat len pismena ! - %c\n", *p);
				return;
				}
			}
		p++;
		i++;
		}
	if (*p!='=') {
		write_user(user, "Pozri si radsej najprv help ...\n");
		return;
		}
	c=(p+1);
	if (strlen(c)>MC_COM_LEN) {
		write_user(user, "Pridlhe rozvinutie makra\n");
		return;
		}
	if (i>MC_NAME_LEN) {
		write_user(user, "Pridlhy nazov makra\n");
		return;
		}
	if (c[0]=='\0') {
		for (mc=user->first_macro; mc!=NULL; mc=mc->next) {
			if (!strncmp(mc->name, word[1], i)) {
				delete_macro(user, mc);
				write_user(user, "Makro bolo vymazane\n");
				save_macros(user);
				return;
				}
			}
		write_user(user, "Nezadane rozvinutie makra !\n");
		return;
		}
	if ((n=(char *)malloc(MC_NAME_LEN+1))==NULL) {
		write_syslog(ERRLOG, 1, "chyba alokacie pamate v macros()\n");
		write_user(user, "~FRSYSTEM: chyba alokacie pamate\n");
		return;
		}

	strncpy(n, word[1], i);
	n[i]='\0';
	for (mc=user->first_macro; mc!=NULL; mc=mc->next) {
		if (!strcmp(mc->name, n)) break;
		}
	if (mc!=NULL) {
		strcpy(mc->comstr, c);
		write_user(user, "Makro bolo zmenene\n");
		save_macros(user);
		free(n);
		return;
		}
	if ((mc=create_macro())==NULL) {
		write_syslog(ERRLOG, 1, "chyba alokacie pamate v macros()\n");
		write_user(user, "~FRSYSTEM: ~RSchyba alokacie pamate\n");
		free(n);
		return;
		}
	strcpy(mc->name, n);
	strcpy(mc->comstr, c);
	if (user->first_macro==NULL) {
		user->first_macro=mc;
		mc->prev=NULL;
		}
	else {
		user->last_macro->next=mc;
		mc->prev=user->last_macro;
		}
	mc->next=NULL;
	user->last_macro=mc;
	save_macros(user);
	write_user(user, "Makro bolo pridane\n");
	free(n);
}


/*** See if command just executed by the user was a macro ***/
int check_macros(UR_OBJECT user, char *inpstr)
{
	MC_OBJECT mc;
	char com[MC_NAME_LEN+1], tmp[MC_NAME_LEN+1], outstr[ARR_SIZE*2];
	int found, zn=0, max;
	char ch[2], *zost=NULL;

	set_crash();
	if (user->type==REMOTE_TYPE) return 1;
	if (user->first_macro==NULL) return 1;

	word_count=wordfind(inpstr);
	strncpy(tmp, inpstr, 10);
	tmp[MC_NAME_LEN]='\0';
	sscanf(tmp, "%s ", com);
	found=strlen(com);
	if (found>MC_NAME_LEN) {
		return (1);
	}
	found=0;
	for (mc=user->first_macro; mc!=NULL; mc=mc->next) {
		if (!strcmp(mc->name, com)) {
			found=1;
			break;
			}
		}
	if (!found) return 1;
	outstr[0]='\0';
	if (strstr(mc->comstr, "$*")) {
		zn=1;
		sprintf(text, "$%d", zn);
		while (strstr(mc->comstr, text)) {
			zn++;
			sprintf(text, "$%d", zn);
			}
		max=zn-1;
		zost=inpstr;
		for (zn=0; zn<=max; zn++) zost=remove_first(zost);
		}
	for (zn=0; mc->comstr[zn]!='\0'; zn++) {
		if (mc->comstr[zn]=='$') {
			if (mc->comstr[zn+1]=='+') {
				if ((strlen(outstr)+strlen(remove_first(inpstr)))<(ARR_SIZE*2))
					strcat(outstr, remove_first(inpstr));
				zn++;
				continue;
				}
			if (mc->comstr[zn+1]=='*') {
				if ((strlen(outstr)+strlen(zost))<(ARR_SIZE*2))
					strcat(outstr, zost);
				zn++;
				continue;
				}
			if (isdigit(mc->comstr[zn+1])) {
				if (word[(mc->comstr[zn+1]-'0')][0]=='\0') {
					write_user(user, "Malo zadanych argumentov.\n");
					return 0;
					}
				else {
					if ((strlen(outstr)+strlen(word[(mc->comstr[zn+1]-'0')]))<ARR_SIZE)
					strcat(outstr, word[(mc->comstr[zn+1]-'0')]);
					zn++;
					continue;
					}
				}
			else {
				ch[0]=mc->comstr[zn];
				ch[1]='\0';
				if ((strlen(outstr)+2)<=ARR_SIZE) strcat(outstr, ch);
				continue;
				}
			}
		else {
			ch[0]=mc->comstr[zn];
			ch[1]='\0';
			if ((strlen(outstr)+2)<=(ARR_SIZE*2)) strcat(outstr, ch);
			}
		}
	strcpy(inpstr, outstr);
	return 1;
}

#endif /* macros.c */
