/*****************************************************************************
         Funkcie pre OS Star v1.1.0 nezaradene do specifickej skupiny
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>
#include <arpa/inet.h>

#include "define.h"
#include "ur_obj.h"
#include "rm_obj.h"
#ifdef NETLINKS
	#include "nl_obj.h"
#endif
#include "pl_obj.h"
#include "sys_obj.h"
#include "syspp_obj.h"
#include "adds.h"
#include "comvals.h"
#include "setval.h"


void oss_versionVerify(void);
UR_OBJECT get_user(char *name);
RM_OBJECT get_room_full(char *name);
UR_OBJECT create_user(void);
RM_OBJECT create_room(void);
char * colour_com_strip(char *str);
char * remove_first(char *str);


char * gnd_grm(int typ, int gnd)
{
	char *ret;

	switch (typ) {
		case  1: return ((gnd==0)?"e":((gnd==1)?"y":"a"));
		case  2: return ((gnd==0)?"ca":((gnd==1)?"":"ka"));
		case  3: return ((gnd==0)?"e":((gnd==1)?"":"a"));
		case  4: return ((gnd==0)?"o":((gnd==1)?"":"a"));
		case  5: return ((gnd==0)?"lo":((gnd==1)?"ol":"la"));
		case  6: return ((gnd==0)?"catu":((gnd==1)?"ovi":"ke"));
		case  7: return ((gnd==0)?"lo":((gnd==1)?"iel":"la"));
		case  8: return ((gnd==0)?"to":((gnd==1)?"ho":"ju"));
		default: return NULL;
		}
}




/* Writes STR to users of LEVEL and higher who are in ROOM except for USER */
void write_duty(int level, char *str, RM_OBJECT room, UR_OBJECT user, int cr)
{
	UR_OBJECT u;

	if (level<WIZ) level=WIZ;
	for (u=user_first;u!=NULL;u=u->next) {
		if (u->login
		    || u->type==CLONE_TYPE
		    || u->type==BOT_TYPE
		    || u->level<level
		    || u==user
		    || u->room==NULL
		    || (u->room!=room && room!=NULL)
		    || ((user!=NULL)
		        && (check_igusers(u,user)!=-1)
		        && (user->level>GOD)
		        )
		    || u->ignall
		    ) continue;
		write_user(u,str);
		}
}


/*** Write usage of a command ***/
void write_usage(UR_OBJECT user, char *str, ...)
{
	va_list args;

	vtext[0]='\0';
	va_start(args, str);
	vsprintf(vtext, str, args);
	va_end(args);
	strcpy(text, vtext);
	vwrite_user(user, "~OL>Pouzitie: ~FT%s\n", text);
}


int osstar_load(void)
{
int cmd,tmp;
cmd=-1;
if (tmp=init_ossmain()) cmd=0;  /* Initialize main system */
if (cmd==-1) { printf("OS Star:  Main system did not initialize in osstar_load()!!\n           BOOT ABORTED.\n\n"); exit(0); }
/* --------------------------------------------------
   Place third-party plugin initialization calls HERE
   Ex:  if (tmp=plugin_00x000_init(cmd)) cmd=cmd+tmp;
   -------------------------------------------------- */
if (tmp=plugin_00x000_init(cmd)) cmd=cmd+tmp;
if (tmp=plugin_02x100_init(cmd)) cmd=cmd+tmp;
/* ------------------------------------------------
   End third-party plugin initialization calls HERE
   ------------------------------------------------ */
printf("Verifikujem pluginy ");
oss_versionVerify();
printf("System plugin registry initialized.\n");
return 1;
}

int init_ossmain(void)
{
printf("\nVerifikujem systemove komponenty a premenne. . .\n");
/* check modular colorcode index checksum */
if ((CDEFAULT+CHIGHLIGHT+CTEXT+CBOLD+CSYSTEM+CSYSBOLD+CWARNING+CWHOUSER+CWHOINFO+
        CPEOPLEHI+CPEOPLE+CUSER+CSELF+CEMOTE+CSEMOTE+CPEMOTE+CTHINK+CTELLUSER+
        CTELLSELF+CTELL+CSHOUT+CMAILHEAD+CMAILDATE+CBOARDHEAD+CBOARDDATE)!=300)
        { printf("OS Star:  Chybny sucet modularnych indexov color-kodov\n"); return 0; }

/* check system registry index checksum */
if ((TALKERNAME+SERIALNUM+REGUSER+SERVERDNS+SERVERIP+TALKERMAIL+TALKERHTTP+SYSOPNAME+SYSOPUNAME+PUEBLOWEB+PUEBLOPIC)!=66)
        {printf("OS Star:  System information registry index checksum FAILED.\n"); return 0; }

/* check system registry master entry */
if (reg_sysinfo[0][0]!='*') { printf("OS Star:  System registry master entry (0) must be '*'.\n   -- Temporarily fixed.");
        reg_sysinfo[0][0]='*';  reg_sysinfo[0][1]='\0'; }

printf("OS Star verzia %s inicializovana.\n\n", OSSVERSION);
return 1;
}

void oss_versionVerify(void)
{
	PL_OBJECT plugin,p;
	CM_OBJECT com;

	for (plugin=plugin_first; plugin!=NULL; plugin=p) {
		p=plugin->next;
		if (atoi(RUN_VER) < atoi(plugin->req_ver)) {
			printf("\nOSS: Plugin '%s' pozaduje vyssiu verziu OS Star.\n",plugin->name);
			write_syslog(SYSLOG, 0, "OS Star: Plugin '%s' pozaduje OS Star verzie %s.\n",plugin->name,plugin->req_ver);
			for (com=cmds_first; com!=NULL; com=com->next) if (com->plugin==plugin) destroy_pl_cmd(com);
			destroy_plugin(plugin);
			}
		printf(".");
		}
	printf("  OK\n");
}


void create_systempp(void)
{
	int i;
	
	if ((syspp=(SYSPP_OBJECT)malloc(sizeof(struct syspp_struct)))==NULL) {
		fprintf(stderr,"OS Star: Failed to create systempp object in create_systempp().\n");
		boot_exit(21);
		}

	syspp->oss_highlev_debug=0;
	syspp->debug_input=0; // POZOR !!! Nikdy nenastavovat na 1 !!!
	syspp->highlev_debug_on=0;
	syspp->pueblo_enh=0;
	syspp->pblo_usr_mm_def=0;
	syspp->pblo_usr_pg_def=0;
	syspp->kill_msgs=0;
	syspp->sys_access=1;
	syspp->wiz_access=1;
	syspp->auto_afk=0;
	syspp->auto_afk_time=0;
}


int show_file(UR_OBJECT user, char *filename)
{
	FILE *fp;
	int cl, i;
	char line[ARR_SIZE+1];

	if ((fp=fopen(filename, "r"))==NULL) return 0;
	fgets(line, ARR_SIZE, fp);
	while(!feof(fp)) {
		line[strlen(line)-1]='\0';
		strcat(line, "\n");
		write_user(user, line);
		fgets(line, ARR_SIZE, fp);
		}
	fclose(fp);
	return 1;
		
}


void split_com_str_num(char *inpstr, int num)
{
	char tmp[ARR_SIZE*2];

	strcpy(tmp, &inpstr[num]);
	inpstr[num]=' ';
	strcpy(&inpstr[num+1], tmp);
	word_count=wordfind(inpstr);
}




void write_room_except2(RM_OBJECT rm, char *str, UR_OBJECT user, UR_OBJECT user2)
{
	UR_OBJECT u;

for(u=user_first;u!=NULL;u=u->next) {
  if (u->login 
      || u->room==NULL 
      || (u->room!=rm && rm!=NULL) 
      || (u->ignall && !force_listen)
      || (u->ignshouts && (com_num==SHOUT || com_num==SEMOTE || com_num==SHOUTTO))
      || (u->ignlogons && logon_flag)
      || (u->igngreets && com_num==GREET)
      || u==user
      || u==user2) continue;
  if ((check_igusers(u,user))!=-1 && user->level<ARCH) continue;
  if (u->type==CLONE_TYPE) {
    if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignall) continue;
    /* Ignore anything not in clones room, eg shouts, system messages
       and semotes since the clones owner will hear them anyway. */
    if (rm!=u->room) continue;
    if (u->clone_hear==CLONE_HEAR_SWEARS) {
      if (!contains_swearing(str)) continue;
      }
    sprintf(text, "~FT[ %s ]:~RS %s",u->room->name,str);
    write_user(u->owner, text);
    }
  else write_user(u,str);
  } /* end for */
}

/* sklonovanie mena podla padov */
void nick_grm(UR_OBJECT user)
{
	char c1,c2,c3;

	c1=user->name[strlen(user->name)-1];
	c2=user->name[strlen(user->name)-2];
	c3=user->name[strlen(user->name)-3];

/* 2. pad -> GENITIV */
	strcpy(user->nameg, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'o' :
			case 'e' : user->nameg[strlen(user->nameg)-1]='u'; break;
			default  : strcat(user->nameg, "tu"); break;
			}
		}

	else if (user->gender==1) {
		switch (c1) {
			case 'o' : user->nameg[strlen(user->nameg)-1]='a'; break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 't' :
								user->nameg[strlen(user->nameg)-2]='\0';
								strcat(user->nameg, "ra");
								break;
							default  : strcat(user->nameg, "a"); break;
							}
						break;
					default  : strcat(user->nameg, "a"); break;
					}
				break;
			case 'y' : strcat(user->nameg, "ho"); break;
			case 'a' : user->nameg[strlen(user->nameg)-1]='u'; break;
			default  : strcat(user->nameg, "a"); break;
			}
			
		}

	else if (user->gender==2) {
		switch (c1) {
			case 'a' : user->nameg[strlen(user->nameg)-1]='u'; break;
			case 'o' : user->nameg[strlen(user->nameg)-1]='a'; break;
			default  : break;
			}
		}

/* 3. pad - DATIV */
	strcpy(user->named, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'a' : strcat(user->named, "tu"); break;
			default  :
				user->named[strlen(user->named)-1]='u';
				break;
			}
		}
	
	if (user->gender==1) {
		switch (c1) {
			case 'o' : strcat(user->named, "vi"); break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 't' :
								user->named[strlen(user->named)-2]='\0';
								strcat(user->named, "rovi");
								break;
							default  : strcat(user->named, "ovi"); break;
							}
						break;
					default  : strcat(user->named, "ovi"); break;
					}
				break;
			case 'y' : strcat(user->named, "mu"); break;
			case 'a' :
				user->named[strlen(user->named)-1]='o';
				strcat(user->named, "vi");
				break;
			default  : strcat(user->named, "ovi"); break;
			}
		}
	
	if (user->gender==2) {
		switch (c1) {
			case 'o' : strcat(user->named, "vi"); break;
			case 'a' :
				switch (c2) {
					case 'c' : user->named[strlen(user->named)-1]='i'; break;
					default  : user->named[strlen(user->named)-1]='e'; break;
					}
				break;
			default  : strcat(user->named, "i"); break;
			}
		}

/* 4. pad - AKUZATIV */
	strcpy(user->namea, user->nameg);

/* 6. pad - LOKAL */
	strcpy(user->namel, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'e' : user->namel[strlen(user->namel)-1]='i'; break;
			case 'o' : user->namel[strlen(user->namel)-1]='e'; break;
			default  : strcat(user->namel, "ti"); break;
			}
		}
	else if (user->gender==1) {
		switch (c1) {
			case 'o' : strcat(user->namel, "vi"); break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 't' :
								user->namel[strlen(user->namel)-2]='\0';
								strcat(user->namel, "rovi");
								break;
							default  : strcat(user->namel, "ovi"); break;
							}
						break;
					default  : strcat(user->namel, "ovi"); break;
					}
				break;
			case 'y' : strcat(user->namel, "m"); break;
			case 'a' :
				user->namel[strlen(user->namel)-1]='o';
				strcat(user->namel, "vi");
				break;
			default  : strcat(user->namel, "ovi"); break;
			}
		}
	else if (user->gender==2) {
		switch (c1) {
			case 'o' : strcat(user->namel, "vi"); break;
			case 'a' :
				switch (c2) {
					case 'k' :
					case 'n' : user->namel[strlen(user->namel)-1]='e'; break;
					default  : user->namel[strlen(user->namel)-1]='i'; break;
					}
				break;
			default  : strcat(user->namel, "i"); break;
			}
		}

/* 7. pad - INSTRUMENTAL */
	strcpy(user->namei, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'o' : strcat(user->namei, "m"); break;
			case 'e' :
				switch (c2) {
					case 'c' :
						user->namei[strlen(user->namei)-1]='o';
						strcat(user->namei, "m");
						break;
					case 'i' : user->namei[strlen(user->namei)-1]='m'; break;
					}
				break;
			default  : strcat(user->namei, "tom"); break;
			}
		}
	else if (user->gender==1) {
		switch (c1) {
			case 'y' :
			case 'o' : strcat(user->namei, "m"); break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 't' :
								user->namei[strlen(user->namei)-2]='\0';
								strcat(user->namei, "rom");
								break;
							default  : strcat(user->namei, "om"); break;
							}
						break;
					default  : strcat(user->namei, "om"); break;
					}
				break;
			case 'a' :
				user->namei[strlen(user->namei)-1]='o';
				strcat(user->namei, "m");
				break;
			default  : strcat(user->namei, "om"); break;
			}
		}
	else if (user->gender==2) {
		switch (c1) {
			case 'o' : strcat(user->namei, "m"); break;
			case 'a' :
				user->namei[strlen(user->namei)-1]='\0';
				strcat(user->namei, "ou");
				break;
			default  :
				strcat(user->namei, "ou");
				break;
			}
		}
}

/* zobrazenie sklonovania nicku */
void show_nick_grm(UR_OBJECT user, UR_OBJECT u)
{
	vwrite_user(user, "2. p - G: %s\n", u->nameg);
	vwrite_user(user, "3. p - D: %s\n", u->named);
	vwrite_user(user, "4. p - A: %s\n", u->namea);
	vwrite_user(user, "6. p - L: %s\n", u->namel);
	vwrite_user(user, "7. p - I: %s\n", u->namei);
}


/* koncovky podla cisla */
char * grm(int typ, int n)
{
	switch (typ) {
		case  1 : return (((n)>4 || (n)==0)?"":(((n)==1)?"u":"y"));
		case  2 : return (((n)>4 || (n)==0)?"ok":(((n)==1)?"ku":"ky"));
		case  3 : return (((n)>4 || (n)==0)?"ych":(((n)==1)?"u":"e"));
		case  4 : return (((n)>4 || (n)==0)?"kov":(((n)==1)?"ok":"ky"));
		case  5 : return (((n)>4 || (n)==0)?"ov":(((n)==1)?"":"i"));
		case  6 : return (((n)>4 || (n)==0)?"ych":(((n)==1)?"y":"i"));
		case  7 : return (((n)>4 || (n)==0)?"ov":(((n)==1)?"":"y"));
		case  8 : return (((n)>4 || (n)==0)?"":(((n)==1)?"a":"y"));
		default : return NULL;
		}
}

void com_nick_grm(UR_OBJECT user)
{
	UR_OBJECT u;
	int on;

	if ((word_count<2) || (user->level<GOD)) {
		u=user;
		on=1;
		}
	else {
		if (!(u=get_user(word[1]))) {
			if ((u=create_user())==NULL) {
				vwrite_user(user,"%s: nemozem vytvorit docasny user objekt.\n",syserror);
				write_syslog(SYSLOG,0,"ERROR: Unable to create temporary user object in examine().\n");
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
	vwrite_user(user, "2. p - G: %s\n", u->nameg);
	vwrite_user(user, "3. p - D: %s\n", u->named);
	vwrite_user(user, "4. p - A: %s\n", u->namea);
	vwrite_user(user, "6. p - L: %s\n", u->namel);
	vwrite_user(user, "7. p - I: %s\n", u->namei);
	if (!on) {
		destruct_user(u);
		destructed=0;
		}
	return;
}


int contains_extension(char *str, int type)
{
	char *s;
	int ok;
	if ((s=(char *)malloc(strlen(str)+1))==NULL) {
		write_syslog("ERROR: Failed to allocate memory in contains_extention().\n",0,SYSLOG);
		return 0;
		}
	strcpy(s,str);
	strtolower(s); 
	ok=0;
	if (type==0) {  /* Images  (.gif/.jpg) */
		if (strstr(s,".gif")) ok=1;
		if (strstr(s,".jpg")) ok=1;
		if (strstr(s,".jpeg")) ok=1;
		if (ok) { free(s); return 1; }
		}
	if (type==1) {  /* Audio   (.wav/.mid) */
		if (strstr(s,".wav")) ok=1;
		if (strstr(s,".mid")) ok=1;
		if (strstr(s,".midi")) ok=1;
		if (ok) { free(s); return 1; }
		}
	free(s);
	return 0;
}


int port_connect(char *host, int port)
{
	int portsocket;
	struct sockaddr_in sin;
	struct hostent *he;

	he=gethostbyname(host);
	if(he==NULL) return -1;

	bzero((char *)&sin,sizeof(sin));
	bcopy(he->h_addr,(char *)&sin.sin_addr,he->h_length);
	sin.sin_family=he->h_addrtype;
	sin.sin_port=htons(port);
	portsocket=socket(AF_INET, SOCK_STREAM, 0);
	if(portsocket==-1) return -1;
	if(connect(portsocket,(struct sockaddr *)&sin,sizeof(sin))==-1) return -1;
	return portsocket;   
}

char *getanswer(FILE *popfp, char *buff, int eol)
{
	int ch;
	char *in=buff;

	for(;;) {
		ch=getc(popfp);
		if((eol==1) || (ch == '\n')) {
			*in='\0';
			eol=0;
			return buff;
			}
		else {
			*in=(char)ch;
			in++;
			}
		}
}


void check_alarm(void)
{
	UR_OBJECT user,next;
	int med;

	user=user_first;
	while(user) {
		next=user->next;
		if (user->alarm) {
			if (user->atime) user->atime-=amsys->heartbeat;
			if (user->atime<=0)
				vwrite_user(user, "%s~FR~OLBudik ti zvoni !!!~RS (vypnes ho '.alarm stop')\n",
					(user->ignbeeps)?"":"\07");
			}
		user=next;
		}
}

void check_autosave(void)
{
	syspp->autosave+=amsys->heartbeat;
	if (syspp->autosave>=(syspp->auto_save*60)) {
		force_save(NULL);
		write_duty(GOD, "Automatic saved user's details\n", NULL, NULL, 0);
		syspp->autosave=0;
		}
}


int is_fnumber(char *str)
{
	int d=0;

	if (str[0]=='-') *str++;
	while(*str) {
		if (!isdigit(*str++)) {
			if (*str=='.') {
				if (!d) d=1;
				else return 0;
				}
			else return 0;
			}
		}
	return 1;
}


int is_inumber(char *str)
{
	int d=0;

	if (str[0]=='-') *str++;
	while(*str) if (!isdigit(*str++)) return 0;
	return 1;
}


char *replace_swear(char *inpstr, char *old)
{
	int i;
	char *x, *y;

	if (NULL==(x=(char *)istrstr(inpstr, old))) return x;
	for (i=0; i<(strlen(old)-2); i++) memcpy(y=x+1+i, ".", 1);
	return inpstr;
}


void reset_murlist(UR_OBJECT user)
{
	int i;

	for (i=0; i<MAX_MUSERS; i++) user->murlist[i][0]='\0';
}


/*** Load swear words list from file ***/
void load_swear_file(UR_OBJECT user)
{
	FILE *fp;
	char line[WORD_LEN+1];
	int i;

	for (i=0; i<MAX_SWEARS; i++)
		swear_words[i][0]='\0';
	i=0;
	if(user==NULL) printf("Loading swear words file ... ");
	else write_user(user,">>>Loading swear words file ... ");
	if(!(fp=fopen(SWEARFILE, "r"))) {
		strcpy(swear_words[0],"*");
		if(user==NULL) printf(" not found.\n");
		else write_user(user," not found.\n");
		return;
		}
	fgets(line,WORD_LEN+2,fp);
	while(!feof(fp)) {
		line[strlen(line)-1]='\0';
		strcpy(swear_words[i],line);
		i++;
		if(i>=MAX_SWEARS) break;
		fgets(line,WORD_LEN+2,fp);
		}
	fclose(fp);
	strcpy(swear_words[i],"*");
	if(user==NULL) printf(" done (%d words).\n",i);
	else vwrite_user(user," done ( ~FT%d~RS words ).\n",i);
}


int backup_talker(void)
{
	char fname[6][500];
	FILE *fp[6];
	int i;

	switch(double_fork()) {
	  case -1 :
		sprintf(text,"~OLSYSTEM: backup_talker(): Failed to fork backup process...\n");
	        write_level(ARCH, 1, text);
		write_syslog(SYSLOG, 1, "backup_talker(): Failed to fork process...\n");
		return 0; /* double_fork() failed */
	  case  0 : /* Start Backup Of Files */
	  	sprintf(fname[0], "%s/%s.tgz", BACKUPDIR, BACKUPFILE);
	  	sprintf(fname[1], "%s/%s.log1", LOGFILES, BACKUPFILE);
	  	sprintf(fname[2], "%s/%s.log2", LOGFILES, BACKUPFILE);
	  	sprintf(fname[3], "%s/%s.tgz", TEMPFILES, BACKUPFILE);
	  	sprintf(fname[4], "%s/%s.log1", TEMPFILES, BACKUPFILE);
	  	sprintf(fname[5], "%s/%s.log2", TEMPFILES, BACKUPFILE);
	  	for (i=0; i<6; i++) unlink(fname[i]);
		write_syslog(SYSLOG, 1, "Backing Up Talker Files To : %s/%s.tgz\n",BACKUPDIR,BACKUPFILE);
		write_syslog(SYSLOG, 1, "For Zip Progress, Read File: %s/%s.log\n",LOGFILES,BACKUPFILE);
//		sprintf(text,"zip -v -9 -r %s/%s.zip * > %s/%s/%s.log",
//			BACKUPDIR, BACKUPFILE, ROOTDIR, LOGFILES, BACKUPFILE);
		sprintf(text, "tar zcfp '%s' '%s' 1> '%s' 2> '%s'",
			fname[3], ROOTDIR, fname[4], fname[5]
			);

	        system(text);
		for (i=0; i<3; i++) rename(fname[i+3], fname[i]);
	        _exit(1);
		return 1;
		}
}


void follow(UR_OBJECT user)
{
	UR_OBJECT ur;
	int i;

	for (ur=user_first; ur!=NULL; ur=ur->next) {
		if (ur->follow!=user) continue;
		if (ur->room==user->room) continue; /* inac by (asi) vznikol nekonecny cyklus */
		if (ur->level<command_table[FOLLOW].level) { /* kvoli demote */
			ur->follow=NULL;
			continue;
			}
		if (!has_room_access(ur, user->room)) {
			vwrite_user(ur, "Smola, ale %s is%s do miestnosti, kde ty nemas pristup,\nnemozes %s uz dalej sledovat ...\n",
				user->name, gnd_grm(7, user->gender), gnd_grm(8, user->gender));
			ur->follow=NULL;
			continue;
			}
		if (ur->level<WIZ) { /* len ak je do roomy linka */
			if (user->room->tr) {
				if ((user->room->go) || user->room->link[user->room->out]!=ur->room) {
					vwrite_user(ur, "Smola, ale %s is%s do miestnosti, kde sa teraz odtialto nedostanes,\nnemozes %s dalej sledovat ...\n",
						user->name, gnd_grm(7, user->gender), gnd_grm(8, user->gender));
					continue;
					}
				vwrite_user(ur, "Sledujes %s do miestnosti %s\n",
					user->name, user->room->name);
				move_user();
				follow(ur);
				continue;
				}
			else {
				for (i=0; i<MAX_LINKS; ++i) {
					if (ur->room->link[i]==user->room) {
						vwrite_user(ur, "Sledujes %s do miestnosti %s\n",
							user->name, user->room->name);
						move_user(ur, user->room, 0);
						follow(ur);
						continue;
						}
					}
				}
			vwrite_user(ur, "Smola, ale %s is%s do miestnosti, kde sa teraz odtialto nedostanes,\nnemozes %s dalej sledovat ...\n",
				user->name, gnd_grm(7, user->gender), gnd_grm(8, user->gender));
			continue;
			}
		vwrite_user(ur, "Sledujes %s do miestnosti %s\n",
			user->name, user->room->name);
		move_user(ur, user->room, 0);
		follow(ur); /* ak aj jeho niekto sleduje */
		continue;
		}
}


int reinit_save_user_malloc(UR_OBJECT user)
{
	FILE *fp;
	char fname[500], *p=user->malloc_start;
	int i;

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
	fprintf(fp, "ignall     %d %d\n", user->ignall, user->ignall_store);
	fprintf(fp, "lines      %d %d %d %d\n", user->edit_line, user->editline, user->revline, user->afkline);
	fprintf(fp, "others     %d %d %d %d %d %ld %d %c %d\n",
		user->warned, user->editing, user->charcnt, user->lmail_lev, user->remote_com,
		(long)user->last_input, user->tmp_int, user->status, user->clone_hear);
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
			if (u->owner==user) fprintf(fp, "%s\n", u->room);
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
	char fname[500], name[ROOM_NAME_LEN+1];
	int i;

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
	fprintf(fp, "transport  %d %d %d %d %d %d\n",
		room->place, room->route, room->out,
		room->go, room->smer, room->time);
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


int reinit_load_user(UR_OBJECT user)
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
			case  0:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->level=atoi(ur_words[i]); break;
						case 2: user->real_level=atoi(ur_words[i]); break;
						}
					}
				break;
			case  1:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: strcpy(user->last_site, ur_words[i]); break;
						case 2: strcpy(user->site, ur_words[i]); break;
						case 3: strcpy(user->ipsite, ur_words[i]); break;
						}
					}
				break;
			case  2:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->afk=atoi(ur_words[i]); break;
						case 2: strncpy(user->afk_mesg, remove_first(remove_first(line)), AFK_MESG_LEN); break;
						}
					}
				break;
			case  3:
				if (wcnt>=2) strncpy(user->call, remove_first(line), USER_NAME_LEN);
				break;
			case  4:
				for (i=1; i<wcnt; i++)
					user->gcoms[i-1]=atoi(ur_words[i]);
				break;
			case  5:
				for (i=1; i<wcnt; i++)
					user->xcoms[i-1]=atoi(ur_words[i]);
				break;
			case  6:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->pueblo=atoi(ur_words[i]); break;
						case 2: user->pblodetect=atoi(ur_words[i]); break;
						}
					}
				break;
			case  7:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->atime=atoi(ur_words[i]); break;
						case 2: user->alarm=atoi(ur_words[i]); break;
						}
					}
				break;
			case  8:
				if (wcnt>=2) strncpy(user->ltell, remove_first(line), USER_NAME_LEN);
				break;
			case  9:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->tcount=atol(ur_words[i]); break;
						case 2: user->bcount=atol(ur_words[i]); break;
						}
					}
				break;
			case 10:
				if (wcnt>=2) user->auth_addr=strtoul(remove_first(line), NULL, 10);
				break;
			case 11:
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/afkbuff\n")) {
					if (c<REVTELL_LINES) strncpy(user->afkbuff[c], line, REVIEW_LEN+1);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 12:
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
			case 13:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->misc_op=atoi(ur_words[i]); break;
						case 2: user->edit_op=atoi(ur_words[i]); break;
						case 3: user->set_op=atoi(ur_words[i]); break;
						case 4: user->set_mode=atoi(ur_words[i]); break;
						}
					}
				break;
			case 14:
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
			case 15:
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
			case 16:
				fscanf(fp, "%s ", s);
				i=0;
				while (strcmp(s, "/pages\n") && i<MAX_PAGES) {
					user->pages[i]=atoi(s);
					i++;
					fscanf(fp, "%s ", s);
					}
				break;
			case 17:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->filepos=atoi(ur_words[i]); break;
						case 2: user->buffpos=atoi(ur_words[i]); break;
						}
					}
				break;
			case 18:
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/revbuff\n")) {
					if (c<REVTELL_LINES) strncpy(user->revbuff[c], line, REVIEW_LEN+1);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 19:
				strncpy(user->inpstr_old, remove_first(remove_first(line)), atoi(ur_words[1]));
				break;
			case 20:
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/copyto\n")) {
					if (c<MAX_COPIES) strncpy(user->revbuff[c], line, USER_NAME_LEN);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 21:
				if (wcnt>=2) strncpy(user->invite_by, remove_first(line), USER_NAME_LEN);
				break;
			case 22:
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
			case 23:
				fgets(line, ARR_SIZE*5-1, fp);
				c=0;
				while (strcmp(line, "/editbuff\n")) {
					if (c<REVTELL_LINES) strncpy(user->editbuff[c], line, REVIEW_LEN+1);
					c++;
					fgets(line, ARR_SIZE*5-1, fp);
					}
				break;
			case 24:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->samesite_all_store=atoi(ur_words[i]); break;
						case 3: strncpy(user->samesite_check_store, ur_words[i], atoi(ur_words[i-1])); break;
						}
					}
				break;
			case 25:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->ignall=atoi(ur_words[i]); break;
						case 2: user->ignall_store=atoi(ur_words[i]); break;
						}
					}
				break;
			case 26:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->edit_line=atoi(ur_words[i]); break;
						case 2: user->editline=atoi(ur_words[i]); break;
						case 3: user->revline=atoi(ur_words[i]); break;
						case 4: user->afkline=atoi(ur_words[i]); break;
						}
					}
				break;
			case 27:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->warned=atoi(ur_words[i]); break;
						case 2: user->editing=atoi(ur_words[i]); break;
						case 3: user->charcnt=atoi(ur_words[i]); break;
						case 4: user->lmail_lev=atoi(ur_words[i]); break;
						case 5: user->remote_com=atoi(ur_words[i]); break;
						case 6: user->last_input=(time_t)strtoul(ur_words[i], NULL, 10); break;
						case 7: user->tmp_int=atoi(ur_words[i]); break;
						case 8: user->status=ur_words[i][0]; break;
						case 9: user->clone_hear=atoi(ur_words[i]); break;
						}
					}
				break;
			case 28:
				for (i=1; i<wcnt; i++) {
					switch (i) {
						case 1: user->wipe_from=atoi(ur_words[i]); break;
						case 2: user->wipe_to=atoi(ur_words[i]); break;
						}
					}
				break;
			case 29:
				if (wcnt>=2) strncpy(user->restrict, remove_first(line), MAX_RESTRICT);
				break;
			case 30:
				if (wcnt>=2) {
					rm=get_room_full(ur_words[2]);
					if (rm!=NULL) user->room=rm;
					}
				break;
			case 31:
				if (wcnt>=2) {
					rm=get_room_full(ur_words[2]);
					if (rm!=NULL) user->invite_room=rm;
					}
				break;
			case 32:
				if (wcnt>=2) {
					rm=get_room_full(ur_words[2]);
					if (rm!=NULL) user->wrap_room=rm;
					}
				break;
			case 33:
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
			case 34:
				if (wcnt>=2)
					user->p_tmp_ch=strdup(ur_words[2]);
				break;
			case 35:
				if (wcnt>=2) {
					ur=get_user(ur_words[2]);
					if (ur!=NULL) user->follow=ur;
					}
				break;
			case 36:
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
			case 37:
				reinit_load_user_malloc(user);
				break;
			}
		}
	else damaged++;
	fgets(line, ARR_SIZE*5-1, fp);
	line[strlen(line)-1]='\0';
	}
	fclose(fp);
	for (pl=plugin_first; pl!=NULL; pl=pl->next)
		call_plugin_exec(user, "", pl, -2);

	return 1;
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
							case 1: room->place=atoi(rm_words[i]); break;
							case 2: room->route=atoi(rm_words[i]); break;
							case 3: room->out=atoi(rm_words[i]); break;
							case 4: room->go=atoi(rm_words[i]); break;
							case 5: room->smer=atoi(rm_words[i]); break;
							case 6: room->time=atoi(rm_words[i]); break;
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

	if ((fp=fopen(RESTARTFILE, "r"))==NULL) {
		write_syslog(SYSLOG, 0, "nemozem otvorit RESTARTFILE na citanie v restore_structs()\n");
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
		reinit_load_user(u);
		prompt(u);
		record_last_login(u->name);
		fgets(line, 199, fp);
		}

	fclose(fp);
	unlink(RESTARTFILE);
	write_room(NULL, restart_ok);
}


void myxterm(UR_OBJECT user, char *inpstr)
{
	if (!user->xterm) {
		write_user(user, "Ved nemas povoleny xterm !!!\n");
		return;
		}
	vwrite_user(user, "\033]0;%s\007", inpstr);
}


void allxterm(UR_OBJECT user, char *inpstr)
{
	UR_OBJECT u;

	for (u=user_first; u!=NULL; u=u->next) {
		if (u->login || u->type==CLONE_TYPE) continue;
		if (u->xterm) vwrite_user(u, "\033]0;%s\007", inpstr);
		else vwrite_user(user, "Window titlebar & icon changed to ~OL%s\n", inpstr);
		}
}


/* Prints out the various menus */
void print_menu(UR_OBJECT user, int type)
{
	char filename[500];

	cls(user);
	sprintf(filename, "%s/%s", SCRFILES, menu_tab[type].fname);
	if (!show_file(user, filename)) {
		vwrite_user(user, "~FRCouldn't find the %s menu..\n", menu_tab[type].name);
		return;
		}
	write_user(user, menu_tab[type].prompt);
}
	
	
/* Centers the text */
char *center(char *string, int clen)
{
	static char text2[ARR_SIZE*2];
	char text3[ARR_SIZE*2];
	int len=0,spc=0;

	strcpy(text3,string);
	len=strlen(colour_com_strip(text3));
	if (len>clen) {
		strcpy(text2,text3);
		return text2;
		}
	spc=(clen/2)-(len/2);
	sprintf(text2,"%*.*s",spc,spc," ");
	strcat(text2,text3);
	return text2;
}


int inroom(RM_OBJECT rm)
{
	UR_OBJECT u;
	int users=0;

	for (u=user_first; u!=NULL; u=u->next) {
		if (u->login) continue;
		if (u->room==rm) users++;
		}
	return users;
}

#ifdef DEBUG
void test(UR_OBJECT user, char *inpstr)
{
}
#endif
