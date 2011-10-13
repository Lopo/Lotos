/*****************************************************************************
         Funkcie pre OS Star v1.0.0 nezaradene do specifickej skupiny
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
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


void oss_versionVerify(void);
UR_OBJECT get_user(char *name);
RM_OBJECT get_room(char *name);
UR_OBJECT create_user(void);


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
void write_usage(UR_OBJECT user, char *str)
{
	vwrite_user(user, "~OL>Pouzitie: ~FT%s\n", str);
}


void vwrite_usage(UR_OBJECT user, char *str, ...)
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
}


int show_file(UR_OBJECT user, char *filename)
{
	FILE *fp;
	int cl, i;
	char line[ARR_SIZE];

	if ((fp=fopen(filename, "r"))==NULL) return 1;
	while(!feof(fp)) {
		line[0]='\0';
		fgets(line, ARR_SIZE-1, fp);
		line[ARR_SIZE-1]='\0';
		line[ARR_SIZE-2]='\n';
		write_user(user, line);
		}
	fclose(fp);
	return 0;
		
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
	char filename[280],line[WORD_LEN+1];
	int i;

	for (i=0; i<MAX_SWEARS; i++)
		swear_words[i][0]='\0';
	i=0;
	sprintf(filename,"%s/%s/%s/%s", ROOTDIR, DATAFILES, MISCFILES, SWEARFILE);
	if(user==NULL) printf("Loading swear words file ... ");
	else write_user(user,">>>Loading swear words file ... ");
	if(!(fp=fopen(filename,"r"))) {
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
	char fname[6][300];
	FILE *fp[6];
	int i;

	switch(double_fork()) {
	  case -1 :
		sprintf(text,"~OLSYSTEM: backup_talker(): Failed to fork backup process...\n");
	        write_level(ARCH, 1, text);
		write_syslog(SYSLOG, 1, "backup_talker(): Failed to fork process...\n");
		return 0; /* double_fork() failed */
	  case  0 : /* Start Backup Of Files */
	  	sprintf(fname[0], "%s/%s/%s.tgz", ROOTDIR, BACKUPDIR, BACKUPFILE);
	  	sprintf(fname[1], "%s/%s/%s.log1", ROOTDIR, LOGFILES, BACKUPFILE);
	  	sprintf(fname[2], "%s/%s/%s.log2", ROOTDIR, LOGFILES, BACKUPFILE);
	  	sprintf(fname[3], "/tmp/%s.tgz", reg_sysinfo[TALKERNAME]);
	  	sprintf(fname[4], "/tmp/%s.log1", reg_sysinfo[TALKERNAME]);
	  	sprintf(fname[5], "/tmp/%s.log2", reg_sysinfo[TALKERNAME]);
	  	for (i=0; i<6; i++) unlink(fname[i]);
		write_syslog(SYSLOG, 1, "Backing Up Talker Files To : %s/%s.zip\n",BACKUPDIR,BACKUPFILE);
		write_syslog(SYSLOG, 1, "For Zip Progress, Read File: %s/%s.log\n",LOGFILES,BACKUPFILE);
//		sprintf(text,"zip -v -9 -r %s/%s/%s.zip * > %s/%s/%s.log",
//			ROOTDIR, BACKUPDIR, BACKUPFILE, ROOTDIR, LOGFILES, BACKUPFILE);
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


void restart(UR_OBJECT user)
{
	UR_OBJECT u,wu;
#ifdef NETLINKS
	NL_OBJECT nl;
#endif
	FILE *fp;
	char *argy[]={progname, confile, "-reinit", NULL };
	char filename[200];

	save_counters();
	write_syslog(SYSLOG, 1, "%s robi ~OL~FTrestart~RS talkra\n", user->name);

	sprintf(filename, "%s/%s", ROOTDIR, RESTARTFILE);
	if ((fp=fopen(filename, "w"))==NULL) {
		write_user(user, "Nemozem otvorit subor pre zoznam, nerestartujem ...\n");
		write_syslog(ERRLOG, 1, "Nemozem otvorit subor v restart()\n");
		write_syslog(SYSLOG, 1, "Restart zruseny - pozri errlog\n");
		return;
		}

	oss_plugin_dump();
#ifdef NETLINKS
	for (nl=nl_first; nl!=NULL; nl=nl->next) shutdown_netlink(nl);
#endif
	fprintf(fp,"%d %d %d %d\n",port[0], port[1], listen_sock[0],listen_sock[1]);

	u=user_first;
	while (u!=NULL) {
		wu=u->next;
		if (u->editing) u->ignall=u->ignall_store;
		fprintf(fp,"%s %d %d %d\n",u->name,u->port,u->socket,u->site_port);
		save_user_details(u,1);
		write_user(u, "~OLRestartujem, chvilu nic nerob... ");
		destruct_user(u);
		u=wu;
		}
	fclose(fp);

#ifdef NETLINKS
	close(listen_sock[2]);
#endif
	execvp(progname,argy);
	exit(12);
}


void reinit_sockets(void)
{
	FILE *fp;
	struct sockaddr_in bind_addr;
	int i,on,size;

	sprintf(text, "%s/%s", ROOTDIR, RESTARTFILE);
	fp=fopen(text, "r");
	fscanf(fp, "%d %d %d %d", &port[0], &port[1], &listen_sock[0], &listen_sock[1]);
	fclose(fp);

	printf("reInitialising sockets on ports: %d a %d\n",port[0],port[1]);
	on=1;
	size=sizeof(struct sockaddr_in);
	bind_addr.sin_family=AF_INET;
	bind_addr.sin_addr.s_addr=INADDR_ANY;

	for(i=0;i<2;++i) {
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

void reconnect_users(void)
{
	UR_OBJECT u;
	FILE *fp;
	char meno[USER_NAME_LEN+1], line[200], inps[REVIEW_LEN];
	int sock=0, sport, siz, jeholen, cl, mport;
	char filename[200];

	siz=sizeof(struct sockaddr_in);
	sprintf(filename, "%s/%s", ROOTDIR, RESTARTFILE);
	fp=fopen(filename, "r");
	fgets(line, 199, fp); /* prvy riadok s portami */

	fgets(line, 199, fp);
	while (!feof(fp)) {
		sscanf(line, "%s %d %d %d %s\n", meno, &mport, &sock, &sport, inps);
		u=create_user();
		strcpy(u->name,meno);
		load_user_details(u);
		if (u->level==L_0) {
			u->room=get_room(default_jail);
			if (u->room==NULL) u->room=room_first;
			}
		else check_start_room(u);
		strcpy(u->site, u->last_site);
		strcpy(u->inpstr_old, inps);

		u->port=mport;
		jeholen=1;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&jeholen, sizeof(jeholen));
		u->socket=sock;
		u->site_port=sport;
		syspp->acounter[3]++;
		syspp->acounter[u->gender]++;
		write_user(u, "\n\t~OL...hotovo, cest zawislosti !!!\n");
		prompt(u);
		record_last_login(u->name);
		fgets(line, 199, fp);
		}
	fclose(fp);
	unlink(filename);
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


