/* vi: set ts=4 sw=4 ai: */
/*
 * email.c
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

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
/*** patched by Ardant on October 26th, 2001 ***/

#ifndef NO_EMAIL_PATCH
int validate_email(char *email)
{
  int dots;
  char *p;
  
  dots=0;
  p=email;
  if (!*p || strchr(" ;/<\"&`'>",*p)) return 0;
  while (*p!='@' && *p)
	{
	  if (!isalnum(*p) && *p!='.' && *p!='_' && *p!='-')
		return 0;
	  p++;
	}
  if (*p!='@') return 0;
  p++;
  if (*p==' ' || *p=='.' || *p=='@' || !*p) return 0;
  while (*p)
	{
	  while (*p!='.')
		{
		  if (*p==' ' || *p==';' || *p=='/') return 0;
		  if (!*p)
			{
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
#else
int validate_email(char *email)
{
	int dots=0;
	char *p=email;

	set_crash();
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
#endif

/*** verify user's email when it is set specified ***/
void set_forward_email(UR_OBJECT user)
{
	FILE *fp;
	char fname[FNAME_LEN];

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
	sprintf(fname,"%s/%s.FWD",MAILSPOOL,user->name);
	if (!(fp=fopen(fname,"w"))) {
		write_syslog(ERRLOG, 1, "Unable to open forward mail file in set_forward_email()\n");
		return;
		}
	sprintf(user->verify_code,"lotos%d",rand()%999);
	/* email header */
	fprintf(fp,"From: %s\n",reg_sysinfo[TALKERNAME]);
	fprintf(fp,"To: %s <%s>\n\n",user->name,user->email);
	/* email body */
	fprintf(fp,"Hello, %s.\n\n",user->name);
	fprintf(fp, vrf_fwd_email, user->verify_code);
	fputs(talker_signature,fp);
	fclose(fp);
	/* send the mail */
	send_email(user->email, "Verification of auto-mail", fname);
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
	char fname[FNAME_LEN];
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

sprintf(fname,"%s/%s.FWD",MAILSPOOL,u->name);
if (!(fp=fopen(fname,"w"))) {
  write_syslog(SYSLOG,0,"Unable to open forward mail file in set_forward_email()\n");
  return;
  }
fprintf(fp,"From: %s\n",reg_sysinfo[TALKERNAME]);
fprintf(fp,"To: %s <%s>\n\n",u->name,u->email);
from=colour_com_strip(from);
fputs(from,fp);
fputs("\n",fp);
message=colour_com_strip(message);
fputs(message,fp);
fputs("\n\n",fp);
fputs(talker_signature,fp);
fclose(fp);
send_email(u->email, "Auto-forward of smail", fname);
write_syslog(SYSLOG,1,"%s had mail sent to their email address.\n",u->name);
if (!on) {
  destruct_user(u);
  destructed=0;
  }
return;
}

/*** stop zombie processes ***/
int send_email(char *addr, char *subj, char *fname)
{
	set_crash();
	switch (double_fork()) {
		case -1: unlink(fname); return -1; /* double_fork() failed */
		case  0:
				 if (subj) sprintf(text, "sendmail -s \"%s\" %s < %s", subj, addr, fname);
				 else sprintf(text, "sendmail %s < %s", addr, fname);
				 system(text);
				 unlink(fname);
				 _exit(1);
				 break; /* should never get here */
		default: break;
		}
	return 1;
}

#endif /* __EMAIL_C__ */

