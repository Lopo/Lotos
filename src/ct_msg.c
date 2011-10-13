/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                  Funkcie Lotos v1.2.0 suvisiace so spravami
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __CT_MSG_C__
#define __CT_MSG_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_sys.h"
#include "ct_msg.h"


/* verify that mail has been sent to the address supplied */
void verify_email(UR_OBJECT user)
{
	set_crash();
if (word_count<2) {
  write_usage(user,"verify <verification code>");
  return;
  }
if (!user->email[0] || !strcmp(user->email,"#UNSET")) {
  write_user(user,"You have not yet set your email address.  You must do this first.\n");
  return;
  }
if (!strcmp(user->verify_code,"#EMAILSET")) {
  write_user(user,"You have already verified your current email address.\n");
  return;
  }
if (strcmp(user->verify_code,word[1]) || !strcmp(user->verify_code,"#NONE")) {
  write_user(user,"That does not match your verification code.  Check your code and try again.\n");
  return;
  }
strcpy(user->verify_code,"#EMAILSET");
user->mail_verified=1;
write_user(user,"\nThe verification code you gave was correct.\nYou may now use the auto-forward functions.\n\n");
if (amsys->auto_promote) check_autopromote(user, 3);
}


/*** Display the reminders a user has to them
     login is used to show information at the login prompt, otherwise user
     is just using the .reminder command.  stage is used for inputting of a new reminder
     ***/
void show_reminders(UR_OBJECT user, int stage)
{
char temp[ARR_SIZE];
int i,j,d,m,y,cnt_total,cnt_today,del,done;

	set_crash();
cnt_total=cnt_today=0;

/* display manually */
if (!stage) {
  if (word_count<2) {
    write_user(user, "~OLNa prezretie:\n");
    write_user(user, "       reminder all\n       reminder today\n       reminder <d> [<m> [<y>]]\n");
    write_user(user, "~OLNa upravu:\n");
    write_user(user, "       reminder set\n       reminder del <number>\n");
    return;
    }
  /* display all the reminders a user has set */
  cnt_total=0;
  if (!strcasecmp("all",word[1])) {
    write_user(user,"+----------------------------------------------------------------------------+\n");
    write_user(user,"| ~OL~FTAll your reminders~RS                                                         |\n");
    write_user(user,"+----------------------------------------------------------------------------+\n\n");
    for (i=0;i<MAX_REMINDERS;i++) {
      /* no msg set, then no reminder */
      if (!user->reminder[i].msg[0]) continue;
      vwrite_user(user,"~OL%2d) ~FT%d%s %s, %d~RS - (%d/%d/%d)\n",++cnt_total,user->reminder[i].day,ordinal_text(user->reminder[i].day),
	      month[user->reminder[i].month-1],user->reminder[i].year,user->reminder[i].day,user->reminder[i].month,user->reminder[i].year);
      vwrite_user(user,"    %s\n",user->reminder[i].msg);
      }
    if (!cnt_total) write_user(user,"You do not have reminders set.\n");
    write_user(user,"\n+----------------------------------------------------------------------------+\n\n");
    return;
    }
  /* display all the reminders a user has for today */
  if (!strcasecmp("today",word[1])) {
    d=tmday;
    m=tmonth+1;
    y=tyear;
    cnt_today=0;
    write_user(user,"+----------------------------------------------------------------------------+\n");
    write_user(user,"| ~OL~FTYour reminders for today are~RS                                               |\n");
    write_user(user,"+----------------------------------------------------------------------------+\n\n");
    for (i=0,j=0;i<MAX_REMINDERS;i++) {
      if (user->reminder[i].day==d && user->reminder[i].month==m && user->reminder[i].year==y) {
	vwrite_user(user,"~OL%2d)~RS %s\n",++j,user->reminder[i].msg);
	cnt_today++;
        }
      }
    if (!cnt_today) write_user(user,"You do not have reminders set for today.\n");
    write_user(user,"\n+----------------------------------------------------------------------------+\n\n");
    return;
    }
  /* allow a user to set a reminder */
  if (!strcasecmp("set",word[1])) {
    cnt_total=0;
    /* check to see if there is enough space to add another reminder */
    for (i=0;i<MAX_REMINDERS;i++) if (!user->reminder[i].msg[0]) cnt_total++;
    if (!cnt_total) {
      write_user(user,"You already have the maximum amount of reminders set.\n");
      return;
      }
    user->temp_remind.day=0;
    user->temp_remind.month=0;
    user->temp_remind.year=0;
    user->temp_remind.msg[0]='\0';
    write_user(user,"Please enter a date for the reminder (1-31): ");
    user->misc_op=20;
    return;
    }
  /* allow a user to delete one of their reminders */
  if (!strcasecmp("del",word[1])) {
    if (word_count<3) {
      write_usage(user,"reminder del <cislo>");
      write_user(user,"   kde <cislo> moze byt prebrate z 'reminder all'");
      return;
      }
    del=atoi(word[2]);
    cnt_total=0;  done=0;
    for (i=0;i<MAX_REMINDERS;i++) {
      if (!user->reminder[i].msg[0]) continue;
      cnt_total++;
      if (cnt_total==del) {
	user->reminder[i].day=0;
	user->reminder[i].month=0;
	user->reminder[i].year=0;
	user->reminder[i].msg[0]='\0';
	done=1;  break;
        }
      }
    if (!done) {
      vwrite_user(user,"Sorry, could not delete reminder number ~OL%d~RS.\n",del);
      return;
      }
    vwrite_user(user,"You have now deleted reminder number ~OL%d~RS.\n",del);
    write_user_reminders(user);
    return;
    }
  /* view reminders for a particular day */
  if (word_count>4) {
    write_usage(user,"reminder <d> [<m> [<y>]]");
    write_user(user,"   kde <d> = den od 1 po 31\n");
    write_user(user,"       <m> = mesiac od 1 po 12\n");
    write_user(user,"       <y> = rok od 1 po 99, alebo 1800 az 3000\n");
    return;
    }
  /* full date given */
  if (word_count==4) {
    y=atoi(word[3]);
    m=atoi(word[2]);
    d=atoi(word[1]);
    /* assume that year give xx is y2k if xx!=99 */
    if (y==99) y+=1900;
    else if (y<99) y+=2000;
    if ((y>3000) || (y<1800) || !m || (m>12) || !d || (d>31)) {
      write_usage(user,"reminder <d> [<m> [<y>]]");
      write_user(user,"   kde <d> = den od 1 po 31\n");
      write_user(user,"       <m> = mesiac od 1 po 12\n");
      write_user(user,"       <y> = rok od 1 po 99, alebo 1800 az 3000\n");
      return;
      }
    }
  /* only date and month given, so show for this year */
  else if (word_count==3) {
    y=tyear;
    m=atoi(word[2]);
    d=atoi(word[1]);
    if (!m || (m>12) || !d || (d>31)) {
      write_usage(user,"reminder <d> [<m> [<y>]]");
      write_user(user,"   kde <d> = den od 1 po 31\n");
      write_user(user,"       <m> = mesiac od 1 po 12\n");
      write_user(user,"       <y> = rok od 1 po 99, alebo 1800 az 3000\n");
      return;
      }
    }
  /* only date given, so show for this month and year */
  else {
    y=tyear;
    m=tmonth+1;
    d=atoi(word[1]);
    if (!d || (d>31)) {
      write_usage(user,"reminder <d> [<m> [<y>]]");
      write_user(user,"   kde <d> = den od 1 po 31\n");
      write_user(user,"       <m> = mesiac od 1 po 12\n");
      write_user(user,"       <y> = rok od 1 po 99, alebo 1800 az 3000\n");
      return;
      }
    }
  write_user(user,"+----------------------------------------------------------------------------+\n");
  sprintf(temp, "Your reminders for %d%s %s, %d", d, ordinal_text(d), month[m-1], y);
  vwrite_user(user,"| ~OL~FT%-74.74s~RS |\n", temp);
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  cnt_today=0;
  for (i=0;i<MAX_REMINDERS;i++) {
    if (user->reminder[i].day==d && user->reminder[i].month==m && user->reminder[i].year==y) {
      vwrite_user(user,"~OL%2d)~RS %s\n",i+1,user->reminder[i].msg);
      cnt_today++;
      }
    }
  if (!cnt_today) vwrite_user(user,"You have no reminders set for %d%s %s, %d\n",d,ordinal_text(d),month[m-1],y);
  write_user(user,"\n+----------------------------------------------------------------------------+\n\n");
  return;
  }

/* next stages of asking for a new reminder */
switch(stage) {
  case 1:
    if (!user->temp_remind.day || user->temp_remind.day>31) {
      write_user(user,"The day for the reminder must be between 1 and 31.\n");
      user->temp_remind.day=0;
      user->misc_op=0;
      return;
      }
    write_user(user,"Please enter a month (1-12): ");
    user->misc_op=21;
    return;
  case 2:
    if (!user->temp_remind.month || user->temp_remind.month>12) {
      write_user(user,"The month for the reminder must be between 1 and 12.\n");
      user->temp_remind.day=0;
      user->temp_remind.month=0;
      user->misc_op=0;
      return;
      }
    write_user(user,"Please enter a year (xx or 19xx or 20xx, etc, <return> for this year): ");
    user->misc_op=22;
    return;
  case 3:
    /* assume that year give xx is y2k if xx!=99 */
    if (user->temp_remind.year==99) user->temp_remind.year+=1900;
    else if (user->temp_remind.year<99) user->temp_remind.year+=2000;
    if ((user->temp_remind.year>3000) || (user->temp_remind.year<1800)) {
      write_user(user,"The year for the reminder must be between 1-99 or 1800-3000.\n");
      user->temp_remind.day=0;
      user->temp_remind.month=0;
      user->temp_remind.year=0;
      user->misc_op=0;
      return;
      }
    /* check to see if date has past - why put in a remind for a date that has? */
    if ((int)(ymd_to_scalar(user->temp_remind.year,user->temp_remind.month,user->temp_remind.day)) < (int)(ymd_to_scalar(tyear,tmonth+1,tmday))) {
      write_user(user,"That date has already passed so there's no point setting a reminder for it.\n");
      user->temp_remind.day=0;
      user->temp_remind.month=0;
      user->temp_remind.year=0;
      user->misc_op=0;
      return;
      }
    write_user(user,"Please enter reminder message:\n~FG>>~RS");
    user->misc_op=23;
    return;
  case 4:
    /* tell them they MUST enter a message */
    if (!user->temp_remind.msg[0]) {
      write_user(user,"Please enter reminder message:\n~FG>>~RS");
      user->misc_op=23;
      return;
      }
    /* add in first available slot */
    for (i=0;i<MAX_REMINDERS;i++) {
      if (!user->reminder[i].msg[0]) {
	user->reminder[i].day=user->temp_remind.day;
	user->reminder[i].month=user->temp_remind.month;
	user->reminder[i].year=user->temp_remind.year;
	strcpy(user->reminder[i].msg,user->temp_remind.msg);
	break;
        }
      }
    write_user(user,"You have set the following reminder:\n");
    vwrite_user(user,"(%d/%d/%d) %s\n\n",user->reminder[i].day,user->reminder[i].month,user->reminder[i].year,user->reminder[i].msg);
    write_user_reminders(user);
    user->misc_op=0;
    return;
  }
}

#endif /* ct_msg.c */
