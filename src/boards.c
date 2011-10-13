/*****************************************************************************
              Funkcie pre OS Star v1.0.0b pracujuce s nastenkami
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

#include <stdio.h>
#include <time.h>
#include <netinet/in.h>

#include "define.h"
#include "ur_obj.h"
#include "rm_obj.h"
#ifdef NETLINKS
	#include "nl_obj.h"
#endif
#include "sys_obj.h"
#include "boards.h"
#include "comvals.h"

/* */
RM_OBJECT get_room(char *name);
char * colour_com_strip(char *str);
void read_board_specific(UR_OBJECT user, RM_OBJECT rm, int msg_number);
char * remove_first(char *inpstr);




/*** Read the message board ***/
void read_board(UR_OBJECT user)
{
	RM_OBJECT rm;
	char filename[180],*name,rmname[USER_NAME_LEN];
	int ret;

	rm=NULL;
	if (word_count<2)
		rm=user->room;
	else {
		if (word_count>=3) {
			if ((rm=get_room(word[1]))==NULL) {
				write_user(user,nosuchroom);
				return;
				}
			read_board_specific(user,rm,atoi(word[2]));
			return;
			}
		if (word_count==2) {
			if (atoi(word[1])) {
				read_board_specific(user,user->room,atoi(word[1]));
				return;
				}
			else {
				if ((rm=get_room(word[1]))==NULL) {
					write_user(user,nosuchroom);
					return;
					}
				}
			}
		if (!has_room_access(user,rm)) {
			write_user(user,"That room is currently private, you cannot read the board.\n");
			return;
			}
		}
	vwrite_user(user, message_board_header, rm->name);
	if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
		midcpy(rm->name,rmname,1,strlen(rm->name)-2);
		rmname[0]=toupper(rmname[0]);
		sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, USERFILES,USERROOMS,rmname);
		}
	else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES,rm->name);
	if (!(ret=more(user,user->socket,filename)))
		vwrite_user(user, read_no_messages, rm->name);
	else if (ret==1) user->misc_op=2;
	if (user->vis) name=user->recap;
	else name=invisname;
	if (rm==user->room)
		vwrite_room_except(user->room, user, user_read_board_prompt, name);
}


/*** Write on the message board ***/
void write_board(UR_OBJECT user, char *inpstr, int done_editing)
{
	FILE *fp;
	int cnt,inp;
	char *ptr,*name,filename[180],rmname[USER_NAME_LEN];

	if (user->muzzled) {
		write_user(user,"You are muzzled, you cannot write on the board.\n");  
		return;
		}
	if (!done_editing) {
		if (word_count<2) {
#ifdef NETLINKS
			if (user->type==REMOTE_TYPE) {
      /* Editor won't work over netlink cos all the prompts will go
	 wrong, I'll address this in a later version. */
				write_user(user,"Sorry, due to software limitations remote users cannot use the line editor.\nUse the '.write <mesg>' method instead.\n");
				return;
				}
#endif
			write_user(user, write_edit_header);
			user->misc_op=3;
			editor(user,NULL);
			return;
			}
		ptr=inpstr;
		inp=1;
		}
	else {
		ptr=user->malloc_start;
		inp=0;
		}
	if (user->room->access==PERSONAL_LOCKED || user->room->access==PERSONAL_UNLOCKED) {
		midcpy(user->room->name,rmname,1,strlen(user->room->name)-2);
		rmname[0]=toupper(rmname[0]);
		sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, USERFILES,USERROOMS,rmname);
		}
	else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES, user->room->name);
	if (!(fp=fopen(filename,"a"))) {
		vwrite_user(user,"%s: cannot write to file.\n",syserror);
		write_syslog(SYSLOG,0,"ERROR: Couldn't open file %s to append in write_board().\n",filename);
		return;
		}
	if (user->vis) name=user->bw_recap;
	else name=invisname;
/* The posting time (PT) is the time its written in machine readable form, this 
   makes it easy for this program to check the age of each message and delete 
   as appropriate in check_messages() */
#ifdef NETLINKS
	if (user->type==REMOTE_TYPE) 
		sprintf(text,"PT: %d\r~OLFrom: %s@%s  %s\n",(int)(time(0)),name,user->netlink->service,long_date(0));
	else 
#endif
		sprintf(text,"PT: %d\r~OLFrom: %s  %s\n",(int)(time(0)),name,long_date(0));
	fputs(text,fp);
	cnt=0;
	while(*ptr!='\0') {
		putc(*ptr,fp);
		if (*ptr=='\n') cnt=0;
		else ++cnt;
		if (cnt==80) {
			putc('\n',fp);
			cnt=0;
			}
		++ptr;
		}
	if (inp) fputs("\n\n",fp); else putc('\n',fp);
	fclose(fp);
	write_user(user, user_write_end);
	vwrite_room_except(user->room, user, room_write_end, name);
	user->room->mesg_cnt++;
}


/*** Wipe some messages off the board ***/
void wipe_board(UR_OBJECT user)
{
	int cnt;
	char filename[80],*name,rmname[USER_NAME_LEN];
	RM_OBJECT rm;

	rm=user->room;
	if (word_count<2 && ((user->level>=WIZ && !is_personal_room(rm))
	    || (is_personal_room(rm) && (is_my_room(user,rm) || user->level>=GOD)))) {
		write_usage(user,"wipe all\n");
		write_usage(user,"wipe <#>\n");
		write_usage(user,"wipe to <#>\n");
		write_usage(user,"wipe from <#> to <#>\n");
		return;
		}
	else if (word_count<2
		 && ((user->level<WIZ && !is_personal_room(rm))
		     || (is_personal_room(rm)
		         && !is_my_room(user,rm)
		         && user->level<GOD
		         )
		     )
		 ) {
		write_usage(user,"wipe <#>");
		return;
		}
	switch(is_personal_room(rm)) {
		case 0:
			if (user->level<WIZ && !(check_board_wipe(user))) return;
			else if (get_wipe_parameters(user)==-1) return;
			break;
		case 1:
			if (!is_my_room(user,rm) && user->level<GOD && !check_board_wipe(user)) return;
			else if (get_wipe_parameters(user)==-1) return;
			break;
		}
	if (user->vis) name=user->recap;
	else name=invisname;
	if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
		midcpy(rm->name,rmname,1,strlen(rm->name)-2);
		rmname[0]=toupper(rmname[0]);
		sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, USERFILES,USERROOMS,rmname);
		}
	else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES, rm->name);
	if (!rm->mesg_cnt) {
		write_user(user, wipe_empty_board);
		return;
		}
	if (user->wipe_from==-1) {
		unlink(filename);
		write_user(user, wipe_user_all_deleted);
		if (user->level<GOD || user->vis)
			vwrite_room_except(rm, user, wipe_room_all_deleted, name);
		write_syslog(SYSLOG,1,"%s wiped all messages from the board in the %s.\n",user->name,rm->name);
		rm->mesg_cnt=0;
		return;
		}
	if (user->wipe_from>rm->mesg_cnt) {
		vwrite_user(user,"There %s only %d message%s on the board.\n",PLTEXT_IS(rm->mesg_cnt),rm->mesg_cnt,PLTEXT_S(rm->mesg_cnt));
		return;
		}
	cnt=wipe_messages(filename,user->wipe_from,user->wipe_to,0);
	if (cnt==rm->mesg_cnt) {
		unlink(filename);
		vwrite_user(user, wipe_too_many,PLTEXT_WAS(rm->mesg_cnt),rm->mesg_cnt,PLTEXT_S(rm->mesg_cnt));
		if (user->level<GOD || user->vis) vwrite_room_except(rm,user,"%s wipes the message board.\n",name);
		write_syslog(SYSLOG,1,"%s wiped all messages from the board in the %s.\n",user->name,rm->name);
		rm->mesg_cnt=0;
		return;
		}
	rm->mesg_cnt-=cnt;
	vwrite_user(user, wipe_user_delete_range, cnt, PLTEXT_S(cnt));
	if (user->level<GOD || user->vis) vwrite_room_except(rm,user,"%s wipes some messages from the board.\n",name);
	write_syslog(SYSLOG,1,"%s wiped %d message%s from the board in the %s.\n",user->name,cnt,PLTEXT_S(cnt),rm->name);
}


/*** Remove any expired messages from boards unless force = 2 in which case
     just do a recount. ***/
void check_messages(UR_OBJECT user, int chforce)
{
	RM_OBJECT rm;
	FILE *infp,*outfp;
	char id[182],filename[180],line[82],rmname[USER_NAME_LEN];
	int valid,pt,write_rest;
	int board_cnt,old_cnt,bad_cnt,tmp;
	static int done=0;

	infp=outfp=NULL;
	switch(chforce) {
		case 0:
			if (amsys->mesg_check_hour==thour && amsys->mesg_check_min==tmin) {
				if (done) return;
				}
			else {
				done=0;
				return;
				}
			break;
		case 1:
			printf("Checking boards...\n");
		case 2:
			if (word_count>=2) {
				strtolower(word[1]);
				if (strcmp(word[1],"motds")) {
					write_usage(user,"recount [motds]");
					return;
					}
				if (!count_motds(1)) {
					write_user(user,"Sorry, could not recount the motds at this time.\n");
					write_syslog(SYSLOG,1,"ERROR: Could not recount motds in check_messages().\n");
					return;
					}
				vwrite_user(user,"There %s %d login motd%s and %d post-login motd%s\n",PLTEXT_WAS(amsys->motd1_cnt),amsys->motd1_cnt,PLTEXT_S(amsys->motd1_cnt),amsys->motd2_cnt,PLTEXT_S(amsys->motd2_cnt));
				write_syslog(SYSLOG,1,"%s recounted the MOTDS.\n",user->name);
				return;
				}
		}
	done=1;
	board_cnt=0;
	old_cnt=0;
	bad_cnt=0;

	for(rm=room_first;rm!=NULL;rm=rm->next) {
		tmp=rm->mesg_cnt;  
		rm->mesg_cnt=0;
		if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
			midcpy(rm->name,rmname,1,strlen(rm->name)-2);
			rmname[0]=toupper(rmname[0]);
			sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, USERFILES,USERROOMS,rmname);
			}
		else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES, rm->name);
		if (!(infp=fopen(filename,"r"))) continue;
		if (chforce<2) {
			if (!(outfp=fopen("tempfile","w"))) {
				if (chforce)
					fprintf(stderr,"OS Star: Couldn't open tempfile.\n");
				write_syslog(SYSLOG,0,"ERROR: Couldn't open tempfile in check_messages().\n");
				fclose(infp);
				return;
				}
			}
		board_cnt++;
		/* We assume that once 1 in date message is encountered all the others
		   will be in date too , hence write_rest once set to 1 is never set to
		   0 again */
		valid=1;
		write_rest=0;
		fgets(line,82,infp); /* max of 80+newline+terminator = 82 */
		while(!feof(infp)) {
			if (*line=='\n') valid=1;
			sscanf(line,"%s %d",id,&pt);
			if (!write_rest) {
				if (valid && !strcmp(id,"PT:")) {
					if (chforce==2) rm->mesg_cnt++;
					else {
						/* 86400 = num. of secs in a day */
						if ((int)time(0) - pt < amsys->mesg_life*86400) {
							fputs(line,outfp);
							rm->mesg_cnt++;
							write_rest=1;
							}
						else old_cnt++;
						}
					valid=0;
					}
				}
			else {
				fputs(line,outfp);
				if (valid && !strcmp(id,"PT:")) {
					rm->mesg_cnt++;
					valid=0;
					}
				}
			fgets(line,82,infp);
			}
		fclose(infp);
		if (chforce<2) {
			fclose(outfp);
			unlink(filename);
			if (!write_rest) unlink("tempfile");
			else rename("tempfile",filename);
			}
		if (rm->mesg_cnt!=tmp) bad_cnt++;
		}
	switch(chforce) {
		case 0:
			if (bad_cnt)
				write_syslog(SYSLOG,1,"CHECK_MESSAGES: %d file%s checked, %d had an incorrect message count, %d message%s deleted.\n",
					board_cnt,PLTEXT_S(board_cnt),bad_cnt,old_cnt,PLTEXT_S(old_cnt));
			else
				write_syslog(SYSLOG,1,"CHECK_MESSAGES: %d file%s checked, %d message%s deleted.\n",board_cnt,PLTEXT_S(board_cnt),old_cnt,PLTEXT_S(old_cnt));
			break;
		case 1:
			printf("  %d board file%s checked, %d out of date message%s found.\n",board_cnt,PLTEXT_S(board_cnt),old_cnt,PLTEXT_S(old_cnt));
			break;
		case 2:
			vwrite_user(user,"%d board file%s checked, %d had an incorrect message count.\n",board_cnt,PLTEXT_S(board_cnt),bad_cnt);
			write_syslog(SYSLOG,1,"%s forced a recount of the message boards.\n",user->name);
		}
}


/** Write a suggestion to the board, or read if if you can **/
void suggestions(UR_OBJECT user, int done_editing)
{
FILE *fp;
char filename[30],*c;
int cnt=0;

if (com_num==RSUG) {
  sprintf(filename,"%s/%s/%s", ROOTDIR,MISCFILES,SUGBOARD);
  write_user(user,"~BB~FG*** The Suggestions board has the following ideas ***\n\n");
  switch(more(user,user->socket,filename)) {
    case 0: write_user(user,"There are no suggestions.\n\n");  break;
    case 1: user->misc_op=2;
    }
  return;
  }
if (user->type==REMOTE_TYPE) {
  write_user(user,"Remote users cannot use this command, sorry!\n");
  return;
  }
if (!done_editing) {
  write_user(user,"~BB~FG*** Writing a suggestion ***\n\n");
  user->misc_op=8;
  editor(user,NULL);
  return;
  }
sprintf(filename,"%s/%s/%s", ROOTDIR,MISCFILES,SUGBOARD);
if (!(fp=fopen(filename,"a"))) {
  vwrite_user(user,"%s: couldn't add suggestion.\n",syserror);
  write_syslog(SYSLOG,0,"ERROR: Couldn't open file %s to write in suggestions().\n",filename);
  return;
  }
sprintf(text,"~OLFrom: %s  %s\n",user->bw_recap,long_date(0));
fputs(text,fp);
c=user->malloc_start;
while(c!=user->malloc_end) {
  putc(*c++,fp);
  if (*c=='\n') cnt=0; else ++cnt;
  if (cnt==80) { putc('\n',fp); cnt=0; }
  }
fprintf(fp,"\n");
fclose(fp);
write_user(user,"Suggestion written.  Thank you for your contribution.\n");
amsys->suggestion_count++;
}


/** delete suggestions from the board **/
void delete_suggestions(UR_OBJECT user)
{
int cnt;
char filename[100];

if (word_count<2) {
  write_user(user,"Pouzitie: dsug all\n");
  write_user(user,"          dsug <#>\n");
  write_user(user,"          dsug to <#>\n");
  write_user(user,"          dsug from <#> to <#>\n");
  return;
  }
if (get_wipe_parameters(user)==-1) return;
sprintf(filename,"%s/%s/%s", ROOTDIR,MISCFILES,SUGBOARD);
if (!amsys->suggestion_count) {
  write_user(user,"There are no suggestions to delete.\n");  return;
  }
if (user->wipe_from==-1) {
  unlink(filename);
  write_user(user,"All suggestions deleted.\n");
  write_syslog(SYSLOG,1,"%s wiped all suggestions from the %s board\n",user->name,SUGBOARD);
  amsys->suggestion_count=0;
  return;
  }
if (user->wipe_from>amsys->suggestion_count) {
  vwrite_user(user,"There %s only %d suggestion%s on the board.\n",PLTEXT_IS(amsys->suggestion_count),amsys->suggestion_count,PLTEXT_S(amsys->suggestion_count));
  return;
  }
cnt=wipe_messages(filename,user->wipe_from,user->wipe_to,0);
if (cnt==amsys->suggestion_count) {
  unlink(filename);
  vwrite_user(user,"There %s only %d suggestion%s on the board, all now deleted.\n",PLTEXT_WAS(cnt),cnt,PLTEXT_S(cnt));
  write_syslog(SYSLOG,1,"%s wiped all suggestions from the %s board\n",user->name,SUGBOARD);
  amsys->suggestion_count=0;
  return;
  }
amsys->suggestion_count-=cnt;
vwrite_user(user,"%d suggestion%s deleted.\n",cnt,PLTEXT_S(cnt));
write_syslog(SYSLOG,1,"%s wiped %d suggestion%s from the %s board\n",user->name,cnt,PLTEXT_S(cnt),SUGBOARD);
}


/*** Show list of people board posts are from without seeing the whole lot ***/
void board_from(UR_OBJECT user)
{
	FILE *fp;
	int cnt;
	char id[ARR_SIZE],line[ARR_SIZE],filename[80],rmname[USER_NAME_LEN];
	RM_OBJECT rm;

	if (word_count<2)
		rm=user->room;
	else {
		if ((rm=get_room(word[1]))==NULL) {
			write_user(user,nosuchroom);
			return;
			}
		if (!has_room_access(user,rm)) {
			write_user(user,"That room is currently private, you cannot read the board.\n");
			return;
			}
		}
	if (!rm->mesg_cnt) {
		write_user(user,"That room has no messages on it's board.\n");
		return;
		}
	if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
		midcpy(rm->name,rmname,1,strlen(rm->name)-2);
		rmname[0]=toupper(rmname[0]);
		sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, USERFILES, USERROOMS, rmname);
		}
	else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES, rm->name);
	if (!(fp=fopen(filename,"r"))) {
		write_user(user,"There was an error trying to read message board.\n");
		write_syslog(SYSLOG,0,"Unable to open message board for %s in board_from().\n",rm->name);
		return;
		}
	vwrite_user(user,"\n~FG~BB*** Posts on the %s message board from ***\n\n",rm->name);
	cnt=0;
	line[0]='\0';
	fgets(line,ARR_SIZE-1,fp);
	while(!feof(fp)) {
		sscanf(line,"%s",id);
		if (!strcmp(id,"PT:")) {
			cnt++;
			vwrite_user(user,"~FT%2d)~RS %s",
				cnt,remove_first(remove_first(remove_first(line))));
			}
		line[0]='\0';
		fgets(line,ARR_SIZE-1,fp);
		}
	fclose(fp);
	vwrite_user(user,"\nTotal of ~OL%d~RS messages.\n\n",rm->mesg_cnt);
}


/*** Show list of people suggestions are from without seeing the whole lot ***/
void suggestions_from(UR_OBJECT user)
{
FILE *fp;
int cnt;
char id[ARR_SIZE],line[ARR_SIZE],filename[100],*str;

if (!amsys->suggestion_count) {
  write_user(user,"There are currently no suggestions.\n");
  return;
  }
sprintf(filename,"%s/%s/%s", ROOTDIR,MISCFILES,SUGBOARD);
if (!(fp=fopen(filename,"r"))) {
  write_user(user,"There was an error trying to read the suggestion board.\n");
  write_syslog(SYSLOG,0,"Unable to open suggestion board in suggestions_from().\n");
  return;
  }
vwrite_user(user,"\n~BB*** Suggestions on the %s board from ***\n\n",SUGBOARD);
cnt=0;  line[0]='\0';
fgets(line,ARR_SIZE-1,fp);
while(!feof(fp)) {
  sscanf(line,"%s",id);
  str=colour_com_strip(id);
  if (!strcmp(str,"From:")) {
    cnt++;
    vwrite_user(user,"~FT%2d)~RS %s",cnt,remove_first(line));
    }
  line[0]='\0';
  fgets(line,ARR_SIZE-1,fp);
  }
fclose(fp);
vwrite_user(user,"\nTotal of ~OL%d~RS suggestions.\n\n",amsys->suggestion_count);
}


/* Allows a user to read a specific message number */
void read_board_specific(UR_OBJECT user, RM_OBJECT rm, int msg_number)
{
FILE *fp;
int valid,cnt,pt;
char id[ARR_SIZE],line[ARR_SIZE],filename[100],*name,rmname[USER_NAME_LEN];

if (!rm->mesg_cnt) {
  vwrite_user(user, read_no_messages, rm->name);
  return;
  }
if (!msg_number) {
  write_usage(user,"read [<room>] [<message #>]");
  return;
  }
if (msg_number>rm->mesg_cnt) {
  vwrite_user(user,"There %s only %d message%s posted on the %s board.\n",PLTEXT_IS(rm->mesg_cnt),rm->mesg_cnt,PLTEXT_S(rm->mesg_cnt),rm->name);
  return;
  }
if (rm!=user->room && !has_room_access(user,rm)) {
  write_user(user,"That room is currently private, you cannot read the board.\n");
  return;
  }
if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
  midcpy(rm->name,rmname,1,strlen(rm->name)-2);
  rmname[0]=toupper(rmname[0]);
  sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR,USERFILES,USERROOMS,rmname);
  }
else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES, rm->name);
if (!(fp=fopen(filename,"r"))) {
  write_user(user,"There was an error trying to read the message board.\n");
  write_syslog(SYSLOG,0,"Unable to open message board for %s in read_board_specific().\n",rm->name);
  return;
  }
vwrite_user(user, message_board_header, rm->name);
valid=1;  cnt=1;  id[0]='\0';
fgets(line,ARR_SIZE-1,fp);
while(!feof(fp)) {
  if (*line=='\n') valid=1;
  sscanf(line,"%s %d",id,&pt);
  if (valid && !strcmp(id,"PT:")) {
    if (msg_number==cnt) {
      while(*line!='\n') {
	write_user(user,line);
	fgets(line,ARR_SIZE-1,fp);
        }
      }
    valid=0;  cnt++;
    if (cnt>msg_number) goto SKIP; /* no point carrying on if read already */
    }
  fgets(line,ARR_SIZE-1,fp);
  }
SKIP:
fclose(fp);
vwrite_user(user,"\nMessage number ~FM~OL%d~RS out of ~FM~OL%d~RS.\n\n",msg_number,rm->mesg_cnt);
if (user->vis) name=user->recap;  else name=invisname;
if (rm==user->room)
  if (user->level<GOD || user->vis) vwrite_room_except(user->room,user,"%s~RS reads the message board.\n",name);
}


/* Check if a normal user can remove a message */
int check_board_wipe(UR_OBJECT user)
{
	FILE *fp;
	int valid,cnt,msg_number,yes,pt;
	char w1[ARR_SIZE],w2[ARR_SIZE],line[ARR_SIZE],line2[ARR_SIZE],filename[80],id[ARR_SIZE],rmname[USER_NAME_LEN];
	RM_OBJECT rm;

	if (word_count<2) {
		write_usage(user,"wipe <message #>");
		return 0;
		}
	rm=user->room;
	if (!rm->mesg_cnt) {
		write_user(user, no_message_prompt);
		return 0;
		}
	msg_number=atoi(word[1]);
	if (!msg_number) {
		write_usage(user,"wipe <#>");
		return 0;
		}
	if (msg_number>rm->mesg_cnt) {
		vwrite_user(user,"There %s only %d message%s on the board.\n",
			PLTEXT_IS(rm->mesg_cnt),rm->mesg_cnt,PLTEXT_S(rm->mesg_cnt));
		return 0;
		}
	if (rm->access==PERSONAL_LOCKED || rm->access==PERSONAL_UNLOCKED) {
		midcpy(rm->name,rmname,1,strlen(rm->name)-2);
		rmname[0]=toupper(rmname[0]);
		sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, USERFILES,USERROOMS,rmname);
		}
	else sprintf(filename,"%s/%s/%s/%s.B", ROOTDIR, DATAFILES, ROOMFILES, rm->name);
	if (!(fp=fopen(filename,"r"))) {
		write_user(user,"There was an error trying to read message board.\n");
		write_syslog(SYSLOG,0,"Unable to open message board for %s in check_board_wipe().\n",rm->name);
		return 0;
		}
	valid=1;
	cnt=1;
	yes=0;
	id[0]='\0';
	w1[0]='\0';
	w2[0]='\0';
	fgets(line,ARR_SIZE-1,fp);
	while(!feof(fp)) {
		if (*line=='\n') valid=1;
		sscanf(line,"%s %d",id,&pt);
		if (valid && !strcmp(id,"PT:")) {
			line2[0]='\0';
			strcpy(line2,remove_first(remove_first(line)));
			sscanf(line2,"%s %s",w1,w2);
			if (msg_number==cnt) {
				/* lower case the name incase of recapping */
				strtolower(w2);
				w2[0]=toupper(w2[0]);
				if (!strcmp(w2,user->name)) {
					yes=1;
					goto SKIP; /* found result, no need to go through rest of file */
					}
				}
			valid=0;
			cnt++;
			if (cnt>msg_number) goto SKIP; /* no point carrying on if checked already */
			}
		fgets(line,ARR_SIZE-1,fp);
		}
SKIP:
	fclose(fp);
	if (!yes) {
		write_user(user,"You did not post that message.  Use ~FTbfrom~RS to check the number again.\n");
		return 0;
		}
	user->wipe_from=msg_number;
	user->wipe_to=msg_number;
	return 1;
}

