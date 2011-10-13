/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
               Funkcie Lotos v1.2.0 pre manipulaciu s kreditmi
original:  Money system version 1.0.1 - Copyright (C) Andrew Collington
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __MONEY_C__
#define __MONEY_C__ 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "obj_ur.h"
#include "obj_sys.h"
#include "prototypes.h"
#include "money.h"


/* give some cash to another user */
void donate_cash(UR_OBJECT user)
{
UR_OBJECT u;
int cash;
char *name;

	set_crash();
if (word_count<3) {
  write_usage(user,"donate <user> <amount>");
  return;
  }
word[1][0]=toupper(word[1][0]);
if (!(u=get_user_name(user,word[1]))) {
  write_user(user,notloggedon);
  return;
  }
if (u==user) {
  write_user(user,"You cannot donate money to yourself.\n");
  return;
  }
cash=atoi(word[2]);
if (cash>MAX_DONATION) {
  write_user(user,"You cannot donate more than $5000.\n");
  return;
  }
if (cash<0) {
  write_user(user,"Now don't be trying to steal money from them!\n");
  return;
  }
if (!cash) {
  write_user(user,"If you're going to donate money, at least donate something!\n");
  return;
  }
if (user->money<cash) {
  write_user(user,"You have not got that much money on you right now.\n");
  return;
  }
u->money+=cash;
user->money-=cash;
if (user->vis || u->level<WIZ) name=user->recap;  else name=invisname;
vwrite_user(user,"You donate $%d out of your own pocket to %s~RS.\n",cash,u->recap);
vwrite_user(u,"%s~RS donates $%d to you out of their own pocket.\n",name,cash);
sprintf(text,"%s donates $%d.\n",user->name,cash);
add_history(u->name,1,text);
}


/* show the user how much money they have */
void show_money(UR_OBJECT user)
{
	set_crash();
if (!user->money) {
  write_user(user,"You do not have any money on your right now.\n");
  return;
  }
vwrite_user(user,"You currently have ~OL~FT$%d~RS on you.\n",user->money);
}


/** Add in the credits system **/
void check_credit_updates(void)
{
UR_OBJECT u,next;

	set_crash();
u=user_first;
while(u!=NULL) {
  next=u->next;  
  /* only update credits for users who qualify */
  if (u->level<MIN_CREDIT_UPDATE_LEVEL || u->afk || u->login || (int)(time(0)-u->last_input)>=amsys->user_idle_time) return;
  u->inctime+=amsys->heartbeat;
  /* work out how many credits per hour */
  if (!(u->inctime%(int)(3600/CREDITS_PER_HOUR))) {
	u->inctime=0;
	u->money++;
    }
  u=next;
  }
}


/* give, take and view money of users currently logged on */
void global_money(UR_OBJECT user)
{
UR_OBJECT u;
int cash,i,x,user_cnt;
char *name,text2[ARR_SIZE];

	set_crash();
x=i=user_cnt=0;
text2[0]='\0';

if (word_count<2) {
  write_usage(user,"money -l/-g/-t [<user> <amount>]");
  return;
  }
/* list all users online and the amount of cash they have */
if (!strcasecmp(word[1],"-l")) {
  write_user(user,"\n+----------------------------------------------------------------------------+\n");
  write_user(user,"| ~FT~OLUser money listings~RS                                                        |\n");
  write_user(user,"+----------------------------------------------------------------------------+\n");
  for (u=user_first;u!=NULL;u=u->next) {
    ++user_cnt;
    /* build up first half of the string */
    if (!x) {
      sprintf(text,"| %-13.13s $%6d ",u->name,u->money);
      ++x;
      }
    /* build up full line and print to user */
    else if (x==1) {
      sprintf(text2,"   %-13.13s $%6d   ",u->name,u->money);
      strcat(text,text2);
      write_user(user,text);
      text[0]='\0';  text2[0]='\0';
      ++x;
      }
    else {
      sprintf(text2,"   %-13.13s $%6d  |\n",u->name,u->money);
      strcat(text,text2);
      write_user(user,text);
      text[0]='\0';  text2[0]='\0';
      x=0;
      }
    } /* end for */
  /* If you've only printed first half of the string */
  if (x==1) {
    strcat(text,"                                                     |\n");
    write_user(user,text);
    }
  if (x==2) {
    strcat(text,"                          |\n");
    write_user(user,text);
    }
  write_user(user,"+----------------------------------------------------------------------------+\n");
  sprintf(text,"Total of ~OL%d~RS user%s",user_cnt,PLTEXT_S(user_cnt));
  vwrite_user(user,"| %-80s |\n",text);
  write_user(user,"+----------------------------------------------------------------------------+\n\n");
  return;
  }
/* give money to users */
if (!strcasecmp(word[1],"-g")) {
  if (word_count<4) {
    write_usage(user,"money -l/-g/-t [<user> <amount>]");
    return;
    }
  strtolower(word[2]);
  word[2][0]=toupper(word[2][0]);
  if (!(u=get_user_name(user,word[2]))) {
    write_user(user,notloggedon);
    return;
    }
  if (u==user && user->level<GOD) {
    write_user(user,"You cannot give money to yourself.\n");
    return;
    }
  cash=atoi(word[3]);
  if (!cash || cash<0) {
    write_user(user,"You must supply an amount to give.\n");
    return;
    }
  u->money+=cash;
  if (user->vis || u->level<WIZ) name=user->name;  else name=invisname;
  vwrite_user(user,"You give $%d to %s.\n",cash,u->name);
  vwrite_user(u,"%s kindly gives $%d.\n",name,cash);
  sprintf(text,"%s gives $%d.\n",user->name,cash);
  add_history(u->name,1,text);
  return;
  }
/* take money from users */
if (!strcasecmp(word[1],"-t")) {
  if (word_count<4) {
    write_usage(user,"money -l/-g/-t [<user> <amount>]");
    return;
    }
  strtolower(word[2]);
  word[2][0]=toupper(word[2][0]);
  if (!(u=get_user_name(user,word[2]))) {
    write_user(user,notloggedon);
    return;
    }
  if (u==user) {
    write_user(user,"You cannot take money away from yourself.\n");
    return;
    }
  cash=atoi(word[3]);
  if (!cash || cash<0) {
    write_user(user,"You must supply an amount to take.\n");
    return;
    }
  if (u->money<cash) {
    vwrite_user(user,"%s has not got that much money.\n");
    return;
    }
  u->money-=cash;
  if (user->vis || u->level<WIZ) name=user->name;  else name=invisname;
  vwrite_user(user,"You take $%d from %s.\n",cash,u->name);
  vwrite_user(u,"%s takes $%d from you.\n",name,cash);
  sprintf(text,"%s takes $%d.\n",user->name,cash);
  add_history(u->name,1,text);
  return;
  }
write_usage(user,"money -l/-g/-t [<user> <amount>]");
}

#endif /* money.c */
