/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
              Funkcie Lotos v1.2.0 na pracu so spravami typu mail
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __MAIL_C__
#define __MAIL_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "mail.h"
#include "comvals.h"


#ifdef NETLINKS
/*** Spool mail file and ask for confirmation of users existence on remote site ***/
void send_external_mail(NL_OBJECT nl, UR_OBJECT user, char *to, char *ptr)
{
FILE *fp;
char filename[500];

	set_crash();
/* Write out to spool file first */
sprintf(filename,"%s/OUT_%s_%s@%s",MAILSPOOL,user->name,to,nl->service);
if (!(fp=fopen(filename,"a"))) {
  sprintf(text,"%s: unable to spool mail.\n",syserror);
  write_user(user,text);
  write_syslog(ERRLOG,1,"Couldn't open file %s to append in send_external_mail().\n",filename);
  return;
  }
putc('\n',fp);
fputs(ptr,fp);
fclose(fp);
/* Ask for verification of users existence */
sprintf(text,"EXISTS? %s %s\n",to,user->name);
write_sock(nl->socket,text);
/* Rest of delivery process now up to netlink functions */
write_user(user,"Mail sent to external talker.\n");
}
#endif


/*** This is function that sends mail to other users ***/
int send_mail(UR_OBJECT user, char *to, char *ptr, int iscopy)
{
#ifdef NETLINKS
  NL_OBJECT nl;
#endif
FILE *infp,*outfp;
char *c,d,*service,filename[500],cc[4],header[ARR_SIZE];
int amount,size,tmp1,tmp2;
struct stat stbuf;

	set_crash();
/* See if remote mail */
c=to;  service=NULL;
while(*c) {
  if (*c=='@') {  
    service=c+1;  *c='\0'; 
#ifdef NETLINKS
    for(nl=nl_first;nl!=NULL;nl=nl->next) {
      if (!strcmp(nl->service,service) && nl->stage==UP) {
	send_external_mail(nl,user,to,ptr);
	return 1;
        }
      }
#endif
    vwrite_user(user,"Service %s unavailable.\n",service);
    return 0;
    }
  ++c;
  }
/* Local mail */
if (!(outfp=fopen("tempfile","w"))) {
  write_user(user,"Error in mail delivery.\n");
  write_syslog(ERRLOG,1,"Couldn't open tempfile in send_mail().\n");
  return 0;
  }
/* Copy current mail file into tempfile if it exists */
sprintf(filename,"%s/%s.M", USERMAILS,to);
/* but first get the original sizes and write those to the temp file */
amount=mail_sizes(to,1); /* amount of new mail */
if (!amount) {
  if (stat(filename,&stbuf)==-1) size=0;
  else size=stbuf.st_size;
  }
else size=mail_sizes(to,2);
fprintf(outfp,"%d %d\r",++amount,size);

if ((infp=fopen(filename,"r"))) {
  /* Discard first line of mail file. */
  fscanf(infp,"%d %d\r",&tmp1,&tmp2);
  /* Copy rest of file */
  d=getc(infp);  
  while(!feof(infp)) {  putc(d,outfp);  d=getc(infp);  }
  fclose(infp);
  }
switch(iscopy) {
  case 0:
  case 2: cc[0]='\0'; break;
  case 1: strcpy(cc,"(CC)"); break;
  }
header[0]='\0';
/* Put new mail in tempfile */
if (user!=NULL) {
#ifdef NETLINKS
  if (user->type==REMOTE_TYPE)
    sprintf(header,"~OLFrom: %s@%s  %s %s\n",user->name,user->netlink->service,long_date(0),cc);
  else 
#endif
    sprintf(header,"~OLFrom: %s  %s %s\n",user->bw_recap,long_date(0),cc);
  }
else sprintf(header,"~OLFrom: MAILER  %s %s\n",long_date(0),cc);
fputs(header,outfp);
fputs(ptr,outfp);
fputs("\n",outfp);
fclose(outfp);
rename("tempfile",filename);
switch(iscopy) {
  case 0: vwrite_user(user,"Mail is delivered to %s\n",to); break;
  case 1: vwrite_user(user,"Mail is copied to %s\n",to); break;
  case 2: break;
  }
if (!iscopy) write_syslog(SYSLOG,1,"%s sent mail to %s\n",user->name,to);
write_user(get_user(to),"\07~FT~OL~LI** YOU HAVE NEW MAIL **\n");
forward_email(to,header,ptr);
return 1;
}


/*** Read just the new mail, taking the fseek size from the stat st_buf saved in the
     mail file first line - along with how many new mail messages there are
     ***/
void read_new_mail(UR_OBJECT user) {
char filename[100];
int total,new;

/* Get total number of mail */
if (!(total=mail_sizes(user->name,0))) {
  write_user(user,"You do not have any mail.\n");  return;
  }
/* Get the amount of new mail */
if (!(new=mail_sizes(user->name,1))) {
  write_user(user,"You do not have any new mail.\n");  return;
  }
if (new==total) {
  /* Update last read / new mail received time at head of file */
  if (!(reset_mail_counts(user))) {
    write_user(user,"You don't have any mail.\n");
    return;
    }
  sprintf(filename,"%s/%s.M", USERMAILS,user->name);
  write_user(user,"\n~BB*** These are the new mail messages you have in your mailbox ***\n\n");
  more(user,user->socket,filename);
  return;
  }
/* Get where new mail starts */
user->filepos=mail_sizes(user->name,2);
/* Update last read / new mail received time at head of file */
if (!(reset_mail_counts(user))) {
  write_user(user,"You don't have any mail.\n");
  return;
  }
sprintf(filename,"%s/%s.M", USERMAILS,user->name);
write_user(user,"\n~BB*** These are the new mail messages you have in your mailbox ***\n\n");
if (more(user,user->socket,filename)!=1) user->filepos=0;
else user->misc_op=2;
return;
}


/* allows a user to choose what message to read */
void read_specific_mail(UR_OBJECT user) {
FILE *fp;
int valid,cnt,total,smail_number,tmp1,tmp2;
char w1[ARR_SIZE],line[ARR_SIZE],filename[500];

if (word_count>2) {
  write_usage(user,"rmail [new/<message #>]");
  return;
  }
if (!(total=mail_sizes(user->name,0))) {
  write_user(user,"You currently have no mail.\n");
  return;
  }
smail_number=atoi(word[1]);
if (!smail_number) {
  write_usage(user,"rmail [new/<message #>]");
  return;
  }
if (smail_number>total) {
  vwrite_user(user,"You only have %d message%s in your mailbox.\n",total,PLTEXT_S(total));
  return;
  }
/* Update last read / new mail received time at head of file */
if (!(reset_mail_counts(user))) {
  write_user(user,"You don't have any mail.\n");
  return;
  }
sprintf(filename,"%s/%s.M", USERMAILS,user->name);
if (!(fp=fopen(filename,"r"))) {
  write_user(user,"There was an error trying to read your mailbox.\n");
  write_syslog(SYSLOG,0,"Unable to open %s's mailbox in read_mail_specific.\n",user->name);
  return;
  }
valid=1;  cnt=1;
fscanf(fp,"%d %d\r",&tmp1,&tmp2);
fgets(line,ARR_SIZE-1,fp);
while(!feof(fp)) {
  if (*line=='\n') valid=1;
  sscanf(line,"%s",w1);
  if (valid && (!strcmp(w1,"~OLFrom:") || !strcmp(w1,"From:"))) {
    if (smail_number==cnt) {
      write_user(user,"\n");
      while(*line!='\n') {
	write_user(user,line);
	fgets(line,ARR_SIZE-1,fp);
        }
      }
    valid=0;  cnt++;
    if (cnt>smail_number) goto SKIP; /* no point carrying on if read already */
    }
  fgets(line,ARR_SIZE-1,fp);
  }
SKIP:
fclose(fp);
vwrite_user(user,"\nMail message number ~FM~OL%d~RS out of ~FM~OL%d~RS.\n\n",smail_number,total);
}


/*** Read your mail ***/
void rmail(UR_OBJECT user) {
int ret,size;
char filename[200];
struct stat stbuf;

sprintf(filename,"%s/%s.M", USERMAILS,user->name);
/* get file size */
if (stat(filename,&stbuf)==-1) size=0;
else size=stbuf.st_size;

/* Just reading the one message or all new mail */
if (word_count>1) {
  strtolower(word[1]);
  if (!strcmp(word[1],"new")) read_new_mail(user);
  else read_specific_mail(user);
  return;
  }
/* Update last read / new mail received time at head of file */
if (!(reset_mail_counts(user))) {
  write_user(user,"You don't have any mail.\n");
  return;
  }
/* Reading the whole mail box */
write_user(user,"\n~BB*** You mailbox has the following messages ***\n\n");
ret=more(user,user->socket,filename);
if (ret==1) user->misc_op=2;
}


/*** send out copy of smail to anyone that is in user->copyto ***/
void send_copies(UR_OBJECT user, char *ptr)
{
	int i,found=0;

	set_crash();
	for (i=0; i<MAX_COPIES; i++) {
		if (!user->copyto[i][0]) continue;
		if (++found==1)
			write_user(user,"Attempting to send copies of smail...\n");
		if (send_mail(user,user->copyto[i],ptr,1))
			write_syslog(SYSLOG,1,"%s sent a copy of mail to %s.\n",user->name,user->copyto[i]);
		}
	for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
}


/*** Send mail message ***/
void smail(UR_OBJECT user, char *inpstr, int done_editing)
{
UR_OBJECT u;
int remote,has_account;
char *c;

	set_crash();
if (user->muzzled) {
  write_user(user,"You are muzzled, you cannot mail anyone.\n");  return;
  }
if (done_editing) {
  if (*user->malloc_end--!='\n') *user->malloc_end--='\n';
  send_mail(user,user->mail_to,user->malloc_start,0);
  send_copies(user,user->malloc_start);
  user->mail_to[0]='\0';
  return;
  }
if (word_count<2) {
  write_user(user,"Smail who?\n");  return;
  }
/* See if its to another site */
remote=0;
has_account=0;
c=word[1];
while(*c) {
  if (*c=='@') {  
    if (c==word[1]) {
      write_user(user,"Users name missing before @ sign.\n");  
      return;
      }
    remote=1;  break;  
    }
  ++c;
  }
word[1][0]=toupper(word[1][0]);
/* See if user exists */
if (!remote) {
  u=NULL;
  if (!(u=get_user(word[1]))) {
    if (!(find_user_listed(word[1]))) {
      write_user(user,nosuchuser);  return;
      }
    has_account=1;
    }
  if (u==user && user->level<ARCH) {
    write_user(user,"Trying to mail yourself is the fifth sign of madness.\n");
    return;
    }
  if ((check_igusers(u,user))!=-1 && user->level<GOD) {
    vwrite_user(user,"%s~RS is ignoring smails from you.\n",u->recap);
    return;
    }
  if (u!=NULL) strcpy(word[1],u->name); 
  if (!has_account) {
    /* See if user has local account */
    if (!(find_user_listed(word[1]))) {
      vwrite_user(user,"%s is a remote user and does not have a local account.\n",u->name);
      return;
      }
    }
  }
if (word_count>2) {
  /* One line mail */
  strcat(inpstr,"\n"); 
  send_mail(user,word[1],remove_first(inpstr),0);
  send_copies(user,remove_first(inpstr));
  return;
  }
#ifdef NETLINKS
  if (user->type==REMOTE_TYPE) {
    write_user(user,"Sorry, due to software limitations remote users cannot use the line editor.\nUse the '.smail <user> <mesg>' method instead.\n");
    return;
    }
#endif
vwrite_user(user, smail_edit_header, word[1]);
user->misc_op=4;
strcpy(user->mail_to,word[1]);
editor(user,NULL);
}


/*** This is function that sends mail to other users ***/
int send_broadcast_mail(UR_OBJECT user, char *ptr, int lvl, int type)
{
FILE *infp,*outfp;
char d,*cc,header[ARR_SIZE],filename[500];
int tmp1,tmp2,amount,size,cnt=0;
struct user_dir_struct *entry;
struct stat stbuf;

	set_crash();
if ((entry=first_dir_entry)==NULL) return 0;
while (entry!=NULL) {
  /*    just wizzes                            specific level    */
  if (((type==-1) && (entry->level!=lvl)) || ((type>=0) && (entry->level!=lvl))) {
    entry=entry->next;
    continue;
    }
  /* if type == -2 then do everyone */
  entry->name[0]=toupper(entry->name[0]);
  if (!(outfp=fopen("tempfile","w"))) {
    write_user(user,"Error in mail delivery.\n");
    write_syslog(ERRLOG,1,"Couldn't open tempfile in send_broadcast_mail().\n");
    entry=entry->next;
    continue;
    }
  /* Write current time on first line of tempfile */
  sprintf(filename,"%s/%s.M", USERMAILS,entry->name);
  /* first get old file size if any new mail, and also new mail count */
  amount=mail_sizes(entry->name,1);
  if (!amount) {
    if (stat(filename,&stbuf)==-1) size=0;
    else size=stbuf.st_size;
    }
  else size=mail_sizes(entry->name,2);
  fprintf(outfp,"%d %d\r",++amount,size);
  if ((infp=fopen(filename,"r"))) {
    /* Discard first line of mail file. */
    fscanf(infp,"%d %d\r",&tmp1,&tmp2);
    /* Copy rest of file */
    d=getc(infp);
    while(!feof(infp)) {  putc(d,outfp);  d=getc(infp);  }
    fclose(infp);
    }
  header[0]='\0';   cc='\0';
  switch(type) {
  case -1: cc="(BCLM Wizzes)";  break;
  case -2: cc="(BCLM All users)";  break;
  default: sprintf(text,"(BCLM %s lvl)",user_level[lvl].name);
    cc=text;
    break;
    }
  if (user!=NULL) {
#ifdef NETLINKS
    if (user->type==REMOTE_TYPE)
      sprintf(header,"~OLFrom: %s@%s  %s %s\n",user->name,user->netlink->service,long_date(0),cc);
    else 
#endif
      sprintf(header,"~OLFrom: %s  %s %s\n",user->bw_recap,long_date(0),cc);
    }
  else sprintf(header,"~OLFrom: MAILER  %s %s\n",long_date(0),cc);
  fprintf(outfp,header);
  fputs(ptr,outfp);
  fputs("\n",outfp);
  fclose(outfp);
  rename("tempfile",filename);
  forward_email(entry->name,header,ptr);
  write_user(get_user(entry->name),"\07~FT~OL~LI*** YOU HAVE NEW MAIL ***\n");
  ++cnt;
  entry=entry->next;
  } /* end while */
return 1;
}


/*** Delete some or all of your mail. A problem here is once we have deleted
     some mail from the file do we mark the file as read? If not we could
     have a situation where the user deletes all his mail but still gets
     the YOU HAVE UNREAD MAIL message on logging on if the idiot forgot to 
     read it first. ***/
void dmail(UR_OBJECT user)
{
int num,cnt;
char filename[500];

	set_crash();
if (word_count<2) {
  write_user(user,"Pouzitie: dmail all\n");
  write_user(user,"Pouzitie: dmail <#>\n");
  write_user(user,"Pouzitie: dmail to <#>\n");
  write_user(user,"Pouzitie: dmail from <#> to <#>\n");
  return;
  }
if (get_wipe_parameters(user)==-1) return;
num=mail_sizes(user->name,0);
if (!num) {
  write_user(user, dmail_nomail);  return;
  }
sprintf(filename,"%s/%s.M", USERMAILS,user->name);
if (user->wipe_from==-1) {
  write_user(user,"\07~OL~FR~LIDelete all of your mail?~RS (y/n): ");
  user->misc_op=18;
  return;
  }
if (user->wipe_from>num) {
  vwrite_user(user,"You only have %d mail message%s.\n",num,PLTEXT_S(num));
  return;
  }
cnt=wipe_messages(filename,user->wipe_from,user->wipe_to,1);
reset_mail_counts(user);
if (cnt==num) {
  unlink(filename);
  vwrite_user(user, dmail_too_many, cnt, grm_gnd(8, cnt));
  return;
  }
vwrite_user(user,"%d mail message%s deleted.\n",cnt,PLTEXT_S(cnt));
user->read_mail=time(0)+1;
}


/*** Show list of people your mail is from without seeing the whole lot ***/
void mail_from(UR_OBJECT user)
{
FILE *fp;
int valid,cnt,tmp1,tmp2,nmail;
char w1[ARR_SIZE],line[ARR_SIZE],filename[500];

	set_crash();
sprintf(filename,"%s/%s.M", USERMAILS,user->name);
if (!(fp=fopen(filename,"r"))) {
  write_user(user,"You have no mail.\n");
  return;
  }
write_user(user,"\n~BB*** Mail from ***\n\n");
valid=1;  cnt=0;
fscanf(fp,"%d %d\r",&tmp1,&tmp2); 
fgets(line,ARR_SIZE-1,fp);
while(!feof(fp)) {
  if (*line=='\n') valid=1;
  sscanf(line,"%s",w1);
  if (valid && (!strcmp(w1,"~OLFrom:") || !strcmp(w1,"From:"))) {
    cnt++;  valid=0;
    vwrite_user(user,"~FT%2d)~RS %s",cnt,remove_first(line));
    }
  fgets(line,ARR_SIZE-1,fp);
  }
fclose(fp);
nmail=mail_sizes(user->name,1);
vwrite_user(user,"\nTotal of ~OL%d~RS message%s, ~OL%d~RS of which %s new.\n\n",cnt,PLTEXT_S(cnt),nmail,PLTEXT_IS(nmail));
}


/*** get users which to send copies of smail to ***/
void copies_to(UR_OBJECT user)
{
int remote,i=0,docopy,found,cnt;
char *c;

	set_crash();
if (com_num==NOCOPIES) {
  for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
  write_user(user,"Sending no copies of your next smail.\n");  return;
  }
if (word_count<2) {
  text[0]='\0';  found=0;
  for (i=0; i<MAX_COPIES; i++) {
    if (!user->copyto[i][0]) continue;
    if (++found==1) write_user(user,"Sending copies of your next smail to...\n");
    strcat(text,"   ");  strcat(text,user->copyto[i]);
    }
  strcat(text,"\n\n");
  if (!found) write_user(user,"You are not sending a copy to anyone.\n");
  else write_user(user,text);
  return;
  }
if (word_count>MAX_COPIES+1) {      /* +1 because 1 count for command */
  vwrite_user(user,"You can only copy to a maximum of %d people.\n",MAX_COPIES);
  return;
  }
write_user(user,"\n");
cnt=0;
for (i=0; i<MAX_COPIES; i++) user->copyto[i][0]='\0';
for (i=1; i<word_count; i++) {
  remote=0;  docopy=1;
  /* See if its to another site */
  c=word[i];
  while(*c) {
    if (*c=='@') {
      if (c==word[i]) {
	vwrite_user(user,"Name missing before @ sign for copy to name '%s'.\n",word[i]);
        docopy=0; goto SKIP;
        }
      remote=1;  docopy=1;  goto SKIP;
      }
    ++c;
    }
  /* See if user exists */
  if (get_user(word[i])==user && user->level<ARCH) {
    write_user(user,"You cannot send yourself a copy.\n");
    docopy=0;  goto SKIP;
    }
  word[i][0]=toupper(word[i][0]);
  if (!remote) {
    if (!(find_user_listed(word[i]))) {
      vwrite_user(user,"There is no such user with the name '%s' to copy to.\n",word[i]);
      docopy=0;
      }
    else docopy=1;
    }
SKIP:
  if (docopy) {
    strcpy(user->copyto[cnt],word[i]);  cnt++;
    }
  }
text[0]='\0';  i=0;  found=0;
for (i=0; i<MAX_COPIES; i++) {
  if (!user->copyto[i][0]) continue;
  if (++found==1) write_user(user,"Sending copies of your next smail to...\n");
  strcat(text,"   ");  strcat(text,user->copyto[i]);
  }
strcat(text,"\n\n");
if (!found) write_user(user,"You are not sending a copy to anyone.\n");
else write_user(user,text);
}


/*** Send mail message to everyone ***/
void level_mail(UR_OBJECT user, char *inpstr, int done_editing)
{
int level,i;

	set_crash();
if (user->muzzled) {
  write_user(user,"You are muzzled, you cannot mail anyone.\n");  return;
  }
strtoupper(word[1]);
if (done_editing) {
  switch(user->lmail_lev) {
    case -1:
      for (i=WIZ;i<=GOD;i++) {
	if (send_broadcast_mail(user,user->malloc_start,i,-1))
	  vwrite_user(user,"You have sent mail to all the %ss.\n",user_level[i].name);
        }
      write_syslog(SYSLOG,1,"%s sent mail to all the Wizzes.\n",user->name);
      return;
    case -2:
      if (send_broadcast_mail(user,user->malloc_start,-1,-2))
	write_user(user,"You have sent mail to all the users.\n");
      write_syslog(SYSLOG,1,"%s sent mail to all the users.\n",user->name);
      return;
    } /* end switch */
  if (send_broadcast_mail(user,user->malloc_start,user->lmail_lev,user->lmail_lev))
    vwrite_user(user,"You have sent mail to all the %ss.\n",user_level[user->lmail_lev].name);
  write_syslog(SYSLOG,1,"%s sent mail to all the %ss.\n",user->name,user_level[user->lmail_lev].name);
  user->lmail_lev=-3;
  return;
  }
if (word_count>2) {
  if ((level=get_level(word[1]))==-1) {
    if (!strcmp(word[1],"WIZZES")) level=-1;
    else if (!strcmp(word[1],"ALL")) level=-2;
    else {
      write_usage(user,"lmail <level name>/wizzes/all [<message>]");
      return;
      }
    }
  strcat(inpstr,"\n"); /* risky but hopefully it'll be ok */
  switch(level) {
    case -1:
      for (i=WIZ;i<=GOD;i++) {
  	  if (send_broadcast_mail(user,remove_first(inpstr),i,-1))
	    vwrite_user(user,"You have sent mail to all the %ss.\n",user_level[i].name);
        }
      write_syslog(SYSLOG,1,"%s sent mail to all the Wizzes.\n",user->name);
      return;
    case -2:
      if (send_broadcast_mail(user,remove_first(inpstr),-1,-2))
	write_user(user,"You have sent mail to all the users.\n");
      write_syslog(SYSLOG,1,"%s sent mail to all the users.\n",user->name);
      return;
    } /* end switch */
  if (send_broadcast_mail(user,remove_first(inpstr),level,level))
    vwrite_user(user,"You have sent mail to all the %ss.\n",user_level[level].name);
  write_syslog(SYSLOG,1,"%s sent mail to all the %ss.\n",user->name,user_level[level].name);
  return;
  }
if (user->type==REMOTE_TYPE) {
  write_user(user,"Sorry, due to software limitations remote users cannot use the line editor.\nUse the '.lmail <level>/wizzes/all <mesg>' method instead.\n");
  return;
  }
if ((level=get_level(word[1]))==-1) {
  if (!strcmp(word[1],"WIZZES")) level=-1;
  else if (!strcmp(word[1],"ALL")) level=-2;
  else {
    write_usage(user,"lmail <level name>/wizzes/all [<message>]");
    return;
    }
  }
user->lmail_lev=level;
write_user(user,"~FR~LICaution~RS:  This will hang the talker until the process has finished!\n");
write_user(user,"          Use only when there are no, or a nominal amount of users logged on.\n");
if (user->lmail_lev==-1) sprintf(text,"\n~FG*** Writing broadcast level mail message to all the Wizzes ***\n\n");
else if (user->lmail_lev==-2) sprintf(text,"\n~FG*** Writing broadcast level mail message to everyone ***\n\n");
else sprintf(text,"\n~FG*** Writing broadcast level mail message to all the %ss ***\n\n",user_level[user->lmail_lev].name);
write_user(user,text);
user->misc_op=9;
editor(user,NULL);
}


/*** Send mail message to all people on your friends list ***/
void friend_smail(UR_OBJECT user, char *inpstr, int done_editing)
{
int i,fcnt;

	set_crash();
if (user->muzzled) {
  write_user(user,"You are muzzled, you cannot mail anyone.\n");  return;
  }
/* check to see if any friends listed */
fcnt=0;
for (i=0;i<MAX_FRIENDS;++i) {
  if (user->friend[i][0]) fcnt++;
  }
if (!fcnt) {
  write_user(user,"You have no friends listed, so you have noone to smail.\n");
  return;
  }
if (done_editing) {
  if (*user->malloc_end--!='\n') *user->malloc_end--='\n';
  for (i=0;i<MAX_FRIENDS;++i) {
    if (!user->friend[i][0]) continue;
    send_mail(user,user->friend[i],user->malloc_start,2);
    }
  write_user(user,"Mail sent to all people on your friends list.\n");
  write_syslog(SYSLOG,1,"%s sent mail to all people on their friends list.\n",user->name);
  return;
  }
if (word_count>1) {
  /* One line mail */
  strcat(inpstr,"\n");
  for (i=0;i<MAX_FRIENDS;++i) {
    if (!user->friend[i][0]) continue;
    send_mail(user,user->friend[i],inpstr,2);
    }
  write_user(user,"Mail sent to all people on your friends list.\n");
  write_syslog(SYSLOG,1,"%s sent mail to all people on their friends list.\n",user->name);
  return;
  }
#ifdef NETLINKS
if (user->type==REMOTE_TYPE) {
  write_user(user,"Sorry, due to software limitations remote users cannot use the line editor.\nUse the '.smail <user> <mesg>' method instead.\n");
  return;
  }
#endif
write_user(user,"\n~BB*** Writing mail message to all your friends ***\n\n");
user->misc_op=24;
editor(user,NULL);
}


/* allows a user to email specific messages to themselves */
void forward_specific_mail(UR_OBJECT user) {
FILE *fpi,*fpo;
int valid,cnt,total,smail_number,tmp1,tmp2;
char w1[ARR_SIZE],line[ARR_SIZE],filenamei[500],filenameo[500];

if (word_count<2) {
  write_usage(user,"fmail all/<mail number>");
  return;
  }
if (!(total=mail_sizes(user->name,0))) {
  write_user(user,"You currently have no mail.\n");
  return;
  }
if (!user->mail_verified) {
  write_user(user,"You have not yet verified your email address.\n");
  return;
  }
/* send all smail */ 
if (!strcasecmp(word[1],"all")) {
  sprintf(filenameo,"%s/%s.FWD",MAILSPOOL,user->name);
  if (!(fpo=fopen(filenameo,"w"))) {
    write_syslog(SYSLOG,0,"Unable to open forward mail file in forward_specific_mail()\n");
    write_user(user,"Sorry, could not forward any mail to you.\n");
    return;
    }
  sprintf(filenamei,"%s/%s.M", USERMAILS,user->name);
  if (!(fpi=fopen(filenamei,"r"))) {
    write_user(user,"Sorry, could not forward any mail to you.\n");
    write_syslog(SYSLOG,0,"Unable to open %s's mailbox in forward_specific_mail()\n",user->name);
    fclose(fpo);
    return;
    }
  fprintf(fpo,"From: %s\n",reg_sysinfo[TALKERNAME]);
  fprintf(fpo,"To: %s <%s>\n",user->name,user->email);
  fprintf(fpo,"Subject: Manual forwarding of smail.\n\n\n");
  fscanf(fpi,"%d %d\r",&tmp1,&tmp2);
  fgets(line,ARR_SIZE-1,fpi);
  while (!feof(fpi)) {
    fprintf(fpo,"%s",colour_com_strip(line));
    fgets(line,ARR_SIZE-1,fpi);
    }
  fputs(talker_signature,fpo);
  fclose(fpi);
  fclose(fpo);
  send_forward_email(user->email,filenameo);
  write_user(user,"You have now sent ~OL~FRall~RS your smails to your email account.\n");
  return;
  }
/* send just a specific smail */
smail_number=atoi(word[1]);
if (!smail_number) {
  write_usage(user,"fmail all/<mail number>");
  return;
  }
if (smail_number>total) {
  vwrite_user(user,"You only have %d message%s in your mailbox.\n",total,PLTEXT_S(total));
  return;
  }
sprintf(filenameo,"%s/%s.FWD",MAILSPOOL,user->name);
if (!(fpo=fopen(filenameo,"w"))) {
  write_syslog(SYSLOG,0,"Unable to open forward mail file in forward_specific_mail()\n");
  write_user(user,"Sorry, could not forward any mail to you.\n");
  return;
  }
sprintf(filenamei,"%s/%s.M", USERMAILS,user->name);
if (!(fpi=fopen(filenamei,"r"))) {
  write_user(user,"Sorry, could not forward any mail to you.\n");
  write_syslog(SYSLOG,0,"Unable to open %s's mailbox in forward_specific_mail()\n",user->name);
  fclose(fpo);
  return;
  }
fprintf(fpo,"From: %s\n",reg_sysinfo[TALKERNAME]);
fprintf(fpo,"To: %s <%s>\n",user->name,user->email);
fprintf(fpo,"Subject: Manual forwarding of smail.\n\n\n");
valid=1;  cnt=1;
fscanf(fpi,"%d %d\r",&tmp1,&tmp2);
fgets(line,ARR_SIZE-1,fpi);
while(!feof(fpi)) {
  if (*line=='\n') valid=1;
  sscanf(line,"%s",w1);
  if (valid && (!strcmp(w1,"~OLFrom:") || !strcmp(w1,"From:"))) {
    if (smail_number==cnt) {
      while(*line!='\n') {
	fprintf(fpo,"%s",colour_com_strip(line));
	fgets(line,ARR_SIZE-1,fpi);
        }
      }
    valid=0;  cnt++;
    if (cnt>smail_number) goto SKIP; /* no point carrying on if read already */
    }
  fgets(line,ARR_SIZE-1,fpi);
  }
SKIP:
fputs(talker_signature,fpo);
fclose(fpi);
fclose(fpo);
send_forward_email(user->email,filenameo);
vwrite_user(user,"You have now sent smail number ~FM~OL%d~RS to your email account.\n",smail_number);
}


/*** returns stats on the mail file.  If type=0 then return total amount of mail message.
     if type=1 then return the number of new mail messages.  Else return size of mail file.
     ***/
int mail_sizes(char *name, int type)
{
FILE *fp;
int valid,cnt,new,size;
char w1[ARR_SIZE],line[ARR_SIZE],filename[500],*str;

	set_crash();
cnt=new=size=0;
name[0]=toupper(name[0]);
sprintf(filename,"%s/%s.M", USERMAILS,name);
if (!(fp=fopen(filename,"r"))) return cnt;
valid=1;
fscanf(fp,"%d %d\r",&new,&size);
/* return amount of new mail or size of mail file */
if (type) {
  fclose(fp);
  return type==1?new:size;
  }
/* count up total mail */
fgets(line,ARR_SIZE-1,fp);
while(!feof(fp)) {
  if (*line=='\n') valid=1;
  sscanf(line,"%s",w1);
  str=colour_com_strip(w1);
  if (valid && !strcmp(str,"From:")) {
    cnt++;  valid=0;
    }
  fgets(line,ARR_SIZE-1,fp);
  }
fclose(fp);
return cnt;
}


/*** Reset the new mail count ans file size at the top of a user's mail file ***/
int reset_mail_counts(UR_OBJECT user)
{
	FILE *infp,*outfp;
	int size,tmp1,tmp2;
	char c,filename[500];
	struct stat stbuf;

	set_crash();
	sprintf(filename,"%s/%s.M", USERMAILS,user->name);
	/* get file size */
	if (stat(filename,&stbuf)==-1) size=0;
	else size=stbuf.st_size;

	if (!(infp=fopen(filename,"r"))) return 0;
	/* Update last read / new mail received time at head of file */
	if ((outfp=fopen("tempfile","w"))) {
		fprintf(outfp,"0 %d\r",size);
		/* skip first line of mail file */
		fscanf(infp,"%d %d\r",&tmp1,&tmp2);
		/* Copy rest of file */
		c=getc(infp);
		while(!feof(infp)) {
			putc(c,outfp);
			c=getc(infp);
			}
		fclose(outfp);
		rename("tempfile",filename);
		}
	user->read_mail=time(0);
	fclose(infp);
	return 1;
}

#endif /* mail.c */
