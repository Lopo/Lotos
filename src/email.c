/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
               Funkcie Lotos v1.2.0 pre pracu s email spravami
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __EMAIL_C__
#define __EMAIL_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_sys.h"
#include "email.h"


/*** check to see if a given email has the correct format ***/
int validate_email(char *email)
{
	int dots;
	char *p;

	set_crash();
	dots=0;
	p=email;
	if (!*p) return 0;
	while (*p!='@' && *p) {
		if (!isalnum(*p) && *p!='.' && *p!='_' && *p!='-')
			return 0;
		p++;
		}
	if (*p!='@') return 0;
	p++;
	if (*p==' ' || *p=='.' || *p=='@' || !*p)
		return 0;
	while (*p) {
		while (*p!='.') {
			if (!*p) {
				if (!(dots)) return 0;
				else return 1;
				}
			p++;
			} /* end while */
		dots++;
		p++;
		if (*p==' ' || *p=='.' || !*p) return 0;
		if (!*p) return 1;
		} /* end while */
	return 1;
}

/*** verify user's email when it is set specified ***/
void set_forward_email(UR_OBJECT user)
{
	FILE *fp;
	char filename[500];

	set_crash();
	if (!user->email[0] || !strcmp(user->email,"#UNSET")) {
		write_user(user,"Your email address is currently ~FRunset~RS.  If you wish to use the\nauto-forwarding function then you must set your email address.\n\n");
		user->autofwd=0;
		return;
		}
	if (!amsys->forwarding) {
		write_user(user,"Even though you have set your email, the auto-forwarding function is currently unavaliable.\n");
		user->mail_verified=0;
		user->autofwd=0;
		return;
		}
	user->mail_verified=0;
	user->autofwd=0;
	/* Let them know by email */
	sprintf(filename,"%s/%s.FWD",MAILSPOOL,user->name);
	if (!(fp=fopen(filename,"w"))) {
		write_syslog(ERRLOG, 1, "Unable to open forward mail file in set_forward_email()\n");
		return;
		}
	sprintf(user->verify_code,"osstar%d",rand()%999);
	/* email header */
	fprintf(fp,"From: %s\n",reg_sysinfo[TALKERNAME]);
	fprintf(fp,"To: %s <%s>\n",user->name,user->email);
	fprintf(fp,"Subject: Verification of auto-mail.\n");
	fprintf(fp,"\n");
	/* email body */
	fprintf(fp,"Hello, %s.\n\n",user->name);
	fprintf(fp, vrf_fwd_email, user->verify_code);
	fputs(talker_signature,fp);
	fclose(fp);
	/* send the mail */
	send_forward_email(user->email,filename);
	write_syslog(SYSLOG,1,"%s had mail sent to them by set_forward_email().\n",user->name);
	/* Inform them online */
	write_user(user,"Now that you have set your email you can use the auto-forward functions.\n");
	write_user(user,"You must verify your address with the code you will receive shortly, via email.\n");
	write_user(user,"If you do not receive any email, then use ~FTset email <email>~RS again, making\nsure you have the correct address.\n\n");
}


/*** send smail to the email ccount ***/
void forward_email(char *name, char *from, char *message)
{
FILE *fp;
UR_OBJECT u;
char filename[500];
int on=0;

	set_crash();
if (!amsys->forwarding) return;
if ((u=get_user(name))) {
  on=1;
  goto SKIP;
  }
/* Have to create temp user if not logged on to check if email verified, etc */
if ((u=create_user())==NULL) {
  write_syslog(ERRLOG,1,"Unable to create temporary user object in forward_email().\n");
  return;
  }
strcpy(u->name,name);
if (!load_user_details(u)) {
  destruct_user(u);
  destructed=0;
  return;
  }
on=0;
SKIP:
	if (!u->mail_verified) {
		if (!on) {
			destruct_user(u);
			destructed=0;
			}
		return;
		}
	if (!u->autofwd){
		if (!on) {
			destruct_user(u);
			destructed=0;
			}
		return;
		} 

sprintf(filename,"%s/%s.FWD",MAILSPOOL,u->name);
if (!(fp=fopen(filename,"w"))) {
  write_syslog(SYSLOG,0,"Unable to open forward mail file in set_forward_email()\n");
  return;
  }
fprintf(fp,"From: %s\n",reg_sysinfo[TALKERNAME]);
fprintf(fp,"To: %s <%s>\n",u->name,u->email);
fprintf(fp,"Subject: Auto-forward of smail.\n");
fprintf(fp,"\n");
from=colour_com_strip(from);
fputs(from,fp);
fputs("\n",fp);
message=colour_com_strip(message);
fputs(message,fp);
fputs("\n\n",fp);
fputs(talker_signature,fp);
fclose(fp);
send_forward_email(u->email,filename);
write_syslog(SYSLOG,1,"%s had mail sent to their email address.\n",u->name);
if (!on) {
  destruct_user(u);
  destructed=0;
  }
return;
}


/*** stop zombie processes ***/
int send_forward_email(char *send_to, char *mail_file)
{
	set_crash();
  switch(double_fork()) {
    case -1 : unlink(mail_file); return -1; /* double_fork() failed */
    case  0 : sprintf(text,"mail %s < %s",send_to,mail_file);
              system(text);
	      unlink(mail_file);
	      _exit(1);
	      break; /* should never get here */
    default: break;
    }
return 1;
}

#endif /* email.c */
