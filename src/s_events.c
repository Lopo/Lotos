/* vi: set ts=4 sw=4 ai: */
/******************************************************************************
                   Funkcie pre Lotos v1.2.0 - event funkcie
			Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __S_EVENTS_C__
#define __S_EVENTS_C__ 1

#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "define.h"
#include "prototypes.h"
#include "obj_sys.h"
#include "obj_syspp.h"
#include "s_events.h"


void do_events(int sig)
{
	set_crash();
	set_date_time();
	check_reboot_shutdown();
	check_idle_and_timeout();
#ifdef NETLINKS
	check_nethangs_send_keepalives(); 
#endif
	check_messages(NULL,0);
	reset_alarm();
	transport();
	plugin_triggers(NULL, "");
	check_alarm();
	if (syspp->auto_save!=(-1))
		check_autosave();
	check_credit_updates();
}


/*** Set global vars. hours,minutes,seconds,date,day,month,year ***/
void set_date_time(void)
{
struct tm *tm_struct; /* structure is defined in time.h */
time_t tm_num;

	set_crash();
/* Set up the structure */
time(&tm_num);
tm_struct=localtime(&tm_num);

/* Get the values */
tday=tm_struct->tm_yday;
tyear=1900+tm_struct->tm_year; /* Will this work past the year 2000? Hmm... */
tmonth=tm_struct->tm_mon;
tmday=tm_struct->tm_mday;
twday=tm_struct->tm_wday;
thour=tm_struct->tm_hour;
tmin=tm_struct->tm_min;
tsec=tm_struct->tm_sec; 
}


/*** See if timed reboot or shutdown is underway ***/
void check_reboot_shutdown(void)
{
int secs;
char *w[]={ "~FRVypnutie","~FYRestart" };

	set_crash();
if (amsys->rs_user==NULL) return;
amsys->rs_countdown-=amsys->heartbeat;
if (amsys->rs_countdown<=0) talker_shutdown(amsys->rs_user,NULL,amsys->rs_which);

/* Print countdown message every minute unless we have less than 1 minute
   to go when we print every 10 secs */
secs=(int)(time(0)-amsys->rs_announce);
if (amsys->rs_countdown>=60 && (secs>=60 || amsys->rs_countdown%60==0)) {
  vwrite_room(NULL, "~OLSYSTEM: %s za %d minut%s, %d sekund%s.\n",
	  w[amsys->rs_which], amsys->rs_countdown/60, grm_num(1, amsys->rs_countdown/60),
	  amsys->rs_countdown%60, grm_num(1, amsys->rs_countdown%60));
  amsys->rs_announce=time(0);
  }
if (amsys->rs_countdown<60 && (secs>=10 || amsys->rs_countdown<=10)) {
  vwrite_room(NULL,"~OLSYSTEM: %s za %d sekund%s.\n",w[amsys->rs_which],amsys->rs_countdown,grm_num(1, amsys->rs_countdown));
  amsys->rs_announce=time(0);
  }
}


/*** login_time_out is the length of time someone can idle at login, 
     user_idle_time is the length of time they can idle once logged in. 
     Also ups users total login time. ***/
void check_idle_and_timeout(void)
{
	UR_OBJECT user,next;
	int tm;

	set_crash();
/* Use while loop here instead of for loop for when user structure gets
   destructed, we may lose ->next link and crash the program */
	user=user_first;
	while (user) {
		next=user->next;
		if (user->type==CLONE_TYPE) {  user=next;  continue;  }
		user->total_login+=amsys->heartbeat; 
		if (user->level>amsys->time_out_maxlevel) {  user=next;  continue;  }
		tm=(int)(time(0) - user->last_input);
		if (user->login && tm>=amsys->login_idle_time) {
			write_user(user, login_timeout);
			disconnect_user(user);
			user=next;
			continue;
			}
		if (syspp->auto_afk
		    && tm>=syspp->auto_afk_time
		    && !user->afk
		    && !user->login) {
			afk(user, auto_afk_mesg);
			user=next;
			continue;
			}
		if (user->warned) {
			if (tm<amsys->user_idle_time-60) {  user->warned=0;  continue;  }
			if (tm>=amsys->user_idle_time) {
				write_user(user,"\n\n\07~FR~OL~LI*** You have been timed out. ***\n\n");
				disconnect_user(user);
				user=next;
				continue;
				}
			}
		if ((!user->afk || (user->afk && amsys->time_out_afks)) 
		    && !user->login 
		    && !user->warned
		    && tm>=amsys->user_idle_time-60) {
			audioprompt(user, 4, 0);
			vwrite_user(user,"\n\07~FY~OL~LI*** POZOR - Mas 1 minutu aby si nieco napisal%s lebo ta vydrbkam !***\n\n", grm_gnd(4, user->gender));
			user->warned=1;
			}
		user=next;
		}
}


void reset_alarm(void)
{
	set_crash();
  SIGNAL(SIGALRM,do_events);
  alarm(amsys->heartbeat);
}


void check_alarm(void)
{
	UR_OBJECT user,next;

	set_crash();
	user=user_first;
	while(user) {
		next=user->next;
		if (user->alarm) {
			if (user->atime) user->atime-=amsys->heartbeat;
			if (user->atime<=0)
				vwrite_user(user, "%s~FR~OLBudik ti zvoni !!!~RS (vypnes ho '.alarm stop')\n",
					(user->ignore.beeps)?"":"\07");
			}
		user=next;
		}
}


void check_autosave(void)
{
	set_crash();
	syspp->autosave+=amsys->heartbeat;
	if (syspp->autosave>=(syspp->auto_save*60)) {
		force_save(NULL);
		write_duty(GOD, "Automatic saved user's details\n", NULL, NULL, 0);
		syspp->autosave=0;
		}
}


#endif /* s_events.c */

