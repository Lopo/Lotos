/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                   Funkcie Lotos v1.2.0 pre pracu s fontami
original: figlets system from CryptV5 
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __FONTS_C__
#define __FONTS_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_sys.h"
#include "fonts.h"


#ifdef __STDC__
char *myalloc(size_t size)
#else
char *myalloc(int size)
#endif
{
	char *ptr;
#ifndef __STDC__
	extern void *malloc();
#endif

	set_crash();
	if ((ptr = (char*)malloc(size))==NULL) {
		write_room(NULL,"~FR~OLSYSTEM: Malloc failed in figlet().\n");
		return NULL;
		}

	return ptr;

}


/****************************************************************************
  readfontchar

  Reads a font character from the font file, and places it in a
  newly-allocated entry in the list.
****************************************************************************/
void readfontchar(FILE *file, long theord, char *line, int maxlen)
{
	int row,k;
	char endchar;
	fcharnode *fclsave;

	set_crash();
	fclsave = fcharlist;

	fcharlist = (fcharnode*)myalloc(sizeof(fcharnode));
	fcharlist->ord = theord;
	fcharlist->thechar = (char**)myalloc(sizeof(char*)*charheight);
	fcharlist->next = fclsave;

	for (row=0;row<charheight;row++) {
		if (fgets(line,maxlen+1,file)==NULL) line[0] = '\0';
		k = MYSTRLEN(line)-1;
		while (k>=0 && isspace(line[k]))
			k--;
		if (k>=0) {
			endchar = line[k];
			while (k>=0 ? line[k]==endchar : 0)
				k--;
			}
		line[k+1] = '\0';
		fcharlist->thechar[row] = (char*)myalloc(sizeof(char)*(k+2));
		strcpy(fcharlist->thechar[row],line);
		}
}


/****************************************************************************
	skiptoeol

	Skips to the end of a line, given a stream.
****************************************************************************/
void skiptoeol(FILE *fp)
{
	int dummy;

	set_crash();
	while (dummy=getc(fp),dummy!='\n'&&dummy!=EOF) ;
}


/****************************************************************************
  readfont

  Allocates memory, initializes variables, and reads in the font.
  Called near beginning of main().
****************************************************************************/
int readfont(char *fontname)
{
	int i,row,numsread;
	long theord;
	int maxlen,cmtlines,ffright2left;
	char *fileline,magicnum[5];
	FILE *fontfile;
	char fontpath[500];

	set_crash();
	sprintf(fontpath, "%s/%s.flf", FIGLET_FONTS, fontname);

	fontfile=fopen(fontpath,"r");

	if (!fontfile) return -1;

	fscanf(fontfile,"%4s",magicnum);
	fileline = (char*)myalloc(sizeof(char)*(MAXFIRSTLINELEN+1));
	if (fgets(fileline,MAXFIRSTLINELEN+1,fontfile)==NULL) fileline[0] = '\0';

	if (MYSTRLEN(fileline)>0 ? fileline[MYSTRLEN(fileline)-1]!='\n' : 0)
		skiptoeol(stdin);

	numsread = sscanf(fileline,"%*c%c %d %*d %d %d %d%*[ \t]%d",
		&hardblank, &charheight, &maxlen,
		&defaultmode, &cmtlines, &ffright2left
		);
	free(fileline);

	for (i=1;i<=cmtlines;i++)
		skiptoeol(fontfile);

	if (numsread<6) ffright2left = 0;
	if (charheight<1) charheight = 1;
	if (maxlen<1) maxlen = 1;
	maxlen += 100; /* Give ourselves some extra room */

	if (right2left<0) right2left = ffright2left;
	if (justification<0) justification = 2*right2left;

	fileline = (char*)myalloc(sizeof(char)*(maxlen+1));
	/* Allocate "missing" character */
	fcharlist = (fcharnode*)myalloc(sizeof(fcharnode));
	fcharlist->ord = 0;
	fcharlist->thechar = (char**)myalloc(sizeof(char*)*charheight);
	fcharlist->next = NULL;
	for (row=0;row<charheight;row++) {
		fcharlist->thechar[row] = (char*)myalloc(sizeof(char));
		fcharlist->thechar[row][0] = '\0';
		}
	for (theord=' ';theord<='~';theord++)
		readfontchar(fontfile,theord,fileline,maxlen);
	for (theord= -255;theord<= -249;theord++)
		readfontchar(fontfile,theord,fileline,maxlen);
	while (fgets(fileline,maxlen+1,fontfile)==NULL?0:sscanf(fileline,"%li",&theord)==1)
		readfontchar(fontfile,theord,fileline,maxlen);
	fclose(fontfile);
	free(fileline);
	return 0;
}


/*** Write text figlets & intros ***/
void write_text_figlet(UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm, char *fig_text, char *name, char *font)
{
	char fig1[ARR_SIZE];
	char fig2[ARR_SIZE];

	set_crash();
	if (strcmp(font,"standard"))
		sprintf(fig1,"~FRBanner od ~OL%s~RS~FR (%s font): ~RS%s\n",
			name, font, fig_text
			);
	else
		sprintf(fig1,"~FRBanner od ~OL%s~RS~FR: ~RS%s\n",
			name, fig_text);
	sprintf(fig2,"~FRBanner od ~OL%s~RS~FR:\n",name);

	if (rm) {
		if (!user->vis) write_monitor(user, rm, 0);
		write_room(rm, fig2);
		record(rm, fig1);
		return;
		}
	if (u) {
		if (u==user) return;
		if (u->afk) record_afk(u, fig1);
		else if (u->editing) record_edit(u, fig1);
		record_tell(u, fig1);
		sprintf(u->ltell, user->name);
		if (strcmp(font,"standard"))
			sprintf(text, "~FRBanner pre ~OL%s~RS~FR (%s font): ~RS%s\n",
				u->name, font, fig_text
				);
		else 
			sprintf(text, "~FRBanner pre ~OL%s~RS~FR: ~RS%s\n",
				u->name, fig_text
				);
		write_user(user, text);
		record_tell(user, text);
		if (u->afk || u->editing) return;
		write_user(u, fig2);
		return;
		}

	for (rm=room_first; rm!=NULL; rm=rm->next)
		record(rm, fig1);
	if (!user->vis) write_monitor(user, NULL, 0);
	write_room_except(NULL, fig2, user);
	write_user(user, fig2);
}


/****************************************************************************
	clearline

  Clears both the input (inchrline) and output (outline) storage.
****************************************************************************/
void fclearline(void)
{
	int i;

	set_crash();
	for (i=0;i<charheight;i++)
		outline[i][0] = '\0';
	outlinelen = 0;
	inchrlinelen = 0;
}


/*** Write figlet lines to users that want them ***/
void write_broadcast_figlet(UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm, char *fig_text)
{
	set_crash();
	if (u) {
		write_user(u, fig_text);
		return;
		}
	if (rm) {
		write_room(rm, fig_text);
		return;
		}
	write_room_except(NULL, fig_text, user);
	write_user(user, fig_text);
}


/****************************************************************************
  putstring

  Prints out the given null-terminated string, substituting blanks
  for hardblanks.  If outputwidth is 1, prints the entire string;
  otherwise prints at most outputwidth-1 characters.  Prints a newline
  at the end of the string.  The string is left-justified, centered or
  right-justified (taking outputwidth as the screen width) if
  justification is 0, 1 or 2, respectively.
****************************************************************************/
void putstring(UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm, char *string)
{
	int i,j=0,len;
	char t;

	set_crash();
	len = MYSTRLEN(string);
	if (outputwidth>1) {
		if (len>outputwidth-1) len = outputwidth-1;
		if (justification>0) {
			for (i=1;(3-justification)*i+len+justification-2<outputwidth;i++) {
				text[j]=' ';
				j++;
				}
			}
		}
	for (i=0;i<len;i++) {
		t=string[i]==hardblank?' ':string[i];
		text[j]=t;
		j++;
		}
	text[j]='\n';
	text[j+1]='\0';
	write_broadcast_figlet(user, u, rm, text);
}


/****************************************************************************
  printline

  Prints outline using putstring, then clears the current line.
****************************************************************************/
void printline(UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm)
{
	int i;

	set_crash();
	for (i=0;i<charheight;i++)
		putstring(user, u, rm, outline[i]);
	fclearline();
}


/****************************************************************************
  getletter

  Sets currchar to point to the font entry for the given character.
  Sets currcharwidth to the width of this character.
****************************************************************************/
void getletter(long c)
{
	fcharnode *charptr;

	set_crash();
	for (charptr=fcharlist; charptr==NULL?0:charptr->ord!=c; charptr=charptr->next) ;
	if (charptr!=NULL) currchar = charptr->thechar;
	else {
		for (charptr=fcharlist;charptr==NULL?0:charptr->ord!=0;charptr=charptr->next) ;
		currchar = charptr->thechar;
		}
	currcharwidth = MYSTRLEN(currchar[0]);
}


/****************************************************************************
  addchar

	Attempts to add the given character onto the end of the current line.
  Returns 1 if this can be done, 0 otherwise.
****************************************************************************/
int addchar(long c)
{
  int smushamount,row;
  char *templine;

	set_crash();
  getletter(c);
  smushamount=0;
  if (outlinelen+currcharwidth>outlinelenlimit
      ||inchrlinelen+1>inchrlinelenlimit) {
    return 0;
  }
  
  templine = (char*)myalloc(sizeof(char)*(outlinelenlimit+1));
  for (row=0;row<charheight;row++) {
    if (right2left) {
      strcpy(templine,currchar[row]);
      strcat(templine,outline[row]+smushamount);
      strcpy(outline[row],templine);
    }
    else
      strcat(outline[row],currchar[row]+smushamount);
    
  }
  free(templine);
  outlinelen = MYSTRLEN(outline[0]);
  inchrline[inchrlinelen++] = c;
  return 1;
}


/****************************************************************************
  splitline

  Splits inchrline at the last word break (bunch of consecutive blanks).
  Makes a new line out of the first part and prints it using
	printline.  Makes a new line out of the second part and returns.
****************************************************************************/
void splitline(UR_OBJECT user, UR_OBJECT u, RM_OBJECT rm)
{
	int i,gotspace,lastspace=0,len1,len2;
	long *part1,*part2;

	set_crash();
	part1 = (long*)myalloc(sizeof(long)*(inchrlinelen+1));
	part2 = (long*)myalloc(sizeof(long)*(inchrlinelen+1));
	gotspace = 0;
	for (i=inchrlinelen-1;i>=0;i--) {
		if (!gotspace && inchrline[i]==' ') {
			gotspace = 1;
			lastspace = i;
			}
		if (gotspace && inchrline[i]!=' ') break;
		}
	len1 = i+1;
	len2 = inchrlinelen-lastspace-1;
	for (i=0;i<len1;i++)
		part1[i] = inchrline[i];
	for (i=0;i<len2;i++)
		part2[i] = inchrline[lastspace+1+i];
	fclearline();
	for (i=0;i<len1;i++)
		addchar(part1[i]);
	printline(user, u, rm);
	for (i=0;i<len2;i++)
		addchar(part2[i]);
	free(part1);
	free(part2);
}



void figlet(UR_OBJECT user, char *inpstr, int typ)
{
	UR_OBJECT u=NULL;
	RM_OBJECT rm=NULL;
	long c;
	int i=0, row, wordbreakmode, char_not_added;
	char *p=inpstr, *name;
	fcharnode *fclsave;
	char fontname[256]="standard";

	set_crash();
	if (user->muzzled) {
		write_user(user, "Si muzzled, nemozes bannerovat\n");
		return;
		}
	if (word_count<2) {
		write_user(user,"Pozri si najprv help ...\n");
		return;
		}
	if (!strcmp(inpstr, "/l")) {
		show_file(user, FONTLIST);
		return;
		}
	if (typ==0) rm=user->room;
	if (typ==1) {
		if (word_count<3) {
			write_usage(user, "tbanner /<user> [-<font>] text");
			return;
			}
		/* Check to see if a username is specified */
		i=0;
		if (*p=='/') {
			/* Get size of font name */
			while (*(p+i)!=' ') {
				i++;
				if (i==100) break;
				}
			strncpy(text, p+1, i);
			*(text+i-1)='\0';
			name=text;
			p=p+i+1;
			}
		else {
			write_usage(user, "tbanner /<user> [-<font>] text");
			return;
			}
		if ((u=get_user_name(user, name))==NULL) {
			write_user(user, notloggedon);
			return;
			}
		if ((check_igusers(u,user))!=-1
		     && (user->level<GOD
			 || user->level<u->level)) {
			vwrite_user(user,"%s~RS is ignoring tells from you.\n",u->recap);
			return;
			}
		if (u->ignore.tells && (user->level<WIZ || u->level>user->level)) {
			vwrite_user(user,"%s~RS is ignoring tells at the moment.\n",u->recap);
			return;
			}
		if (u->ignore.all && (user->level<WIZ || u->level>user->level)) {
			if (u->malloc_start!=NULL) vwrite_user(user,"%s~RS is using the editor at the moment.\n",u->recap);
			else vwrite_user(user,"%s~RS is ignoring everyone at the moment.\n",u->recap);
			return;
			}
#ifdef NETLINKS
		if (u->room==NULL) {
			vwrite_user(user,"%s~RS is offsite and would not be able to reply to you.\n",u->recap);
			return;
			}
#endif
		if (u->afk) {
			if (u->afk_mesg[0]) vwrite_user(user,"%s~RS is ~FRAFK~RS, message is: %s\n",u->recap,u->afk_mesg);
			else vwrite_user(user,"%s~RS is ~FRAFK~RS at the moment.\n",u->recap);
			write_user(user,"Sending message to their afk review buffer.\n");
			}
		if (u->editing) {
			vwrite_user(user,"%s~RS is in ~FTEDIT~RS mode at the moment (using the line editor).\n",u->recap);
			write_user(user,"Sending message to their edit review buffer.\n");
			}
		}

	/* Check to see if a font is specified */
	i=0;
	if (*p=='-') {
		/* Get size of font name */
		while (*(p+i)!=' ') {
			i++;
			if (i==100) break;
			}
		strncpy(fontname, p+1, i);
		*(fontname+i-1)='\0';
		p=p+i+1;

		if ((word_count<3 && typ!=1)
		    || (word_count<4 && typ==1)
		    ) {
			write_user(user,"Co zabannerovat ?\n");
			return;
			}
		}

	if (amsys->ban_swearing && contains_swearing(p) && user->level<MIN_LEV_NOSWR) {
		switch(amsys->ban_swearing) {
			case SBMIN:
				p=censor_swear_words(p);
				break;
			case SBMAX:
				write_user(user,noswearing);
				return;
			default : break; /* do nothing as ban_swearing is off */
			}
		}

	justification = 0;
	right2left = -1;

	outputwidth = 80;

	outlinelenlimit = outputwidth-1;

	if ((i=readfont(fontname))==-1) {
		sprintf(text,"Nemozem nahrat font %s\n",fontname);
		write_user(user,text);
		return;
		}

	name=invisname;
	if ((typ==1 && u->level>=user->level) || user->vis) name=user->name;

	write_text_figlet(user, u, rm, p, name, fontname);

	/* Line alloc... */
	outline = (char**)myalloc(sizeof(char*)*charheight);
	for (row=0;row<charheight;row++) {
		outline[row] = (char*)myalloc(sizeof(char)*(outlinelenlimit+1));
		}
	inchrlinelenlimit = outputwidth*4+100;
	inchrline = (long*)myalloc(sizeof(long)*(inchrlinelenlimit+1));
	fclearline();
	wordbreakmode = 0;

	while (*p) { 
		c=*p;
		p=p+1;
  
		if (isascii(c) && isspace(c)) {
			c = (c=='\t' || c==' ') ? ' ' : '\n';
			}

		if ( (c>'\0' && c<' ' && c!='\n' ) || c==127) continue;

		/*
		Note: The following code is complex and thoroughly tested.
		Be careful when modifying!
		*/

		do {
			char_not_added = 0;

			if (wordbreakmode== -1) {
				if (c==' ') break;
				else if (c=='\n') {
					wordbreakmode = 0;
					break;
					}
				wordbreakmode = 0;
				}

			if (c=='\n') {
				printline(user, u, rm);
				wordbreakmode = 0;
				}
			else if (addchar(c)) {
				if (c!=' ') wordbreakmode = (wordbreakmode>=2)?3:1;
				else wordbreakmode = (wordbreakmode>0)?2:0;
				}
			else if (outlinelen==0) {
				for (i=0;i<charheight;i++) {
					if (right2left && outputwidth>1)
						putstring(user, u, rm, currchar[i]+MYSTRLEN(currchar[i])-outlinelenlimit);
					else
						putstring(user, u, rm, currchar[i]);
					}
				wordbreakmode = -1;
				}
    
			else if (c==' ') {
				if (wordbreakmode==2) splitline(user, u, rm);
				else printline(user, u, rm);
				wordbreakmode = -1;
				}
			else {
				if (wordbreakmode>=2) splitline(user, u, rm);
				else printline(user, u, rm);
				wordbreakmode = (wordbreakmode==3)?1:0;
				char_not_added = 1;
				}
			} while (char_not_added);
		}
	if (outlinelen!=0) printline(user, u, rm);

	/* Free up memory... */
	free(inchrline);
	for (row=0;row<charheight;row++)
		free(outline[row]);
	free(outline);
	/* Free up font memory... */
	do {
		/* Save pointer to next node */
		fclsave=fcharlist->next;
		/* Free memory used by this node */
		for (row=0;row<charheight;row++)
			free(fcharlist->thechar[row]);
		free(fcharlist->thechar);
		free(fcharlist);
		fcharlist=fclsave;
		} while (fclsave!=NULL);
	return;
}

#endif /* fonts.c */
