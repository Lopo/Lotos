/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                 Funkcie pre Lotos v1.2.0 na pracu s retazcami
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __S_STRING_C__
#define __S_STRING_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "define.h"
#include "prototypes.h"
#include "obj_sys.h"
#include "s_string.h"


/*** Attempt to get '\n' terminated line of input from a character
     mode client else store data read so far in user buffer. ***/
int get_charclient_line(UR_OBJECT user, char *inpstr, int len)
{
	int l;

	set_crash();
	for(l=0;l<len;++l) {
  /* see if delete entered */
		if (inpstr[l]==8 || inpstr[l]==127) {
			if (user->buffpos) {
				user->buffpos--;
				if (user->terminal.checho) write_user(user,"\b \b");
				}
			continue;
			}
		user->buff[user->buffpos]=inpstr[l];
  /* See if end of line */
		if (inpstr[l]<32 || user->buffpos+2==ARR_SIZE) {
			terminate(user->buff);
			strcpy(inpstr,user->buff);
			if (user->terminal.checho) write_user(user,"\n");
			return 1;
			}
		user->buffpos++;
		}
	if (user->terminal.checho
	    && ((user->login!=LOGIN_PASSWD && user->login!=LOGIN_CONFIRM)
		    || (amsys->password_echo || user->show_pass)
			)
		)
		write(user->socket,inpstr,len);
	return 0;
}


/*** Put string terminate char. at first char < 32 ***/
void terminate(char *str)
{
	int i;

	set_crash();
	for (i=0;i<ARR_SIZE;++i) if (*(str+i)<32) {  *(str+i)=0;  return;  } 
	str[i-1]=0;
}


/*** Get words from sentence. This function prevents the words in the 
     sentence from writing off the end of a word array element. This is
     difficult to do with sscanf() hence I use this function instead. ***/
int wordfind(char *inpstr)
{
	int wn,wpos;

	set_crash();
	wn=0; wpos=0;
	do {
		while(*inpstr<33) if (!*inpstr++) return wn;
		while(*inpstr>32 && wpos<WORD_LEN-1) {
			word[wn][wpos]=*inpstr++; 
			wpos++;
			}
		word[wn][wpos]='\0';
		wn++;
		wpos=0;
		} while (wn<MAX_WORDS);
	return wn-1;
}


/*** clear word array etc. ***/
void clear_words()
{
	int w;

	set_crash();
	for(w=0;w<MAX_WORDS;++w) word[w][0]='\0';
	word_count=0;
}


/** check to see if string given is YES or NO **/
int yn_check(char *wd)
{
	set_crash();
	if (!strcmp(wd,"YES")) return 1;
	if (!strcmp(wd,"NO")) return 0;
	if (!strcmp(wd, "ANO")) return 1;
	if (!strcmp(wd, "NIE")) return 0;
	return -1;
}


/** check to see if string given is ON or OFF **/
int onoff_check(char *wd)
{
	set_crash();
	if (!strcmp(wd,"ON")) return 1;
	if (!strcmp(wd,"OFF")) return 0;
	if (!strcmp(wd, "ZAP")) return 1;
	if (!strcmp(wd, "VYP")) return 0;
	return -1;
}


/** check to see if string given is OFF, MIN or MAX **/
int minmax_check(char *wd)
{
	set_crash();
	if (!strcmp(wd,"OFF")) return SBOFF;
	if (!strcmp(wd,"MIN")) return SBMIN;
	if (!strcmp(wd,"MAX")) return SBMAX;
	if (!strcmp(wd, "VYP")) return SBOFF;
	return -1;
}


/** check to see if string given is OFF, AUTO, MANUAL **/
int resolve_check(char *wd)
{
	set_crash();
	if (!strcmp(wd,"OFF")) return 0;
	if (!strcmp(wd,"AUTO")) return 1;
	if (!strcmp(wd,"MANUAL")) return 2;
	if (!strcmp(wd, "VYP")) return 0;
	return -1;
}


/*** Tell telnet not to echo characters - for password entry ***/
void echo_off(UR_OBJECT user)
{
	set_crash();
	if (amsys->password_echo || user->show_pass) return;
	vwrite_user(user,"%c%c%c",255,251,1);
}


/*** Tell telnet to echo characters ***/
void echo_on(UR_OBJECT user)
{
	set_crash();
	if (amsys->password_echo || user->show_pass) return;
	vwrite_user(user,"%c%c%c",255,252,1);
}


/*** Return pos. of second word in inpstr ***/
char *remove_first(char *inpstr)
{
	char *pos=inpstr;
	set_crash();
	while(*pos<33 && *pos) ++pos;
	while(*pos>32) ++pos;
	while(*pos<33 && *pos) ++pos;
	return pos;
}


/*** See if string contains any swearing ***/
int contains_swearing(char *str)
{
	char *s;
	int i;

	set_crash();
	if ((s=(char *)malloc(strlen(str)+1))==NULL) {
		write_syslog(ERRLOG,0,"CHYBA: Chyba pri alokacii pamate v contains_swearing().\n");
		return 0;
		}
	strcpy(s,str);
	strtolower(s); 
	i=0;
	while(swear_words[i][0]!='*') {
		if (strstr(s,swear_words[i])) {
			free(s);
			return 1;
			}
		++i;
		}
	/* check to see if it contains a fake pueblo command from another user. */
	if (strstr(s,"</xch_mudtext>")) {
		free(s);
		return 1;
		}
	if (strstr(s,"this world is pueblo") && strstr(s,"enhanced")) {
		free(s);
		return 1;
		}
	free(s);
	return 0;
}


/* go through the given string and replace any of the words found in the
   swear_words array with the default swear censor, *swear_censor
   */
char *censor_swear_words(char *has_swears)
{
	int i;
	char *clean;

	set_crash();
	clean='\0';

	i=0;
	while (swear_words[i][0]!='*') {
		while(has_swears!=NULL) {
			clean=has_swears;
			has_swears=replace_swear(clean, swear_words[i]);
			}
		++i;
		has_swears=clean;
		}
	return clean;
}


/*** Convert string to upper case ***/
void strtoupper(char *str)
{
	set_crash();
	while(*str) {  *str=toupper(*str);  str++; }
}


/*** Convert string to lower case ***/
void strtolower(char *str)
{
	set_crash();
	while(*str) {  *str=tolower(*str);  str++; }
}


/*** Returns 1 if string is a positive number ***/
int is_number(char *str)
{
	set_crash();
	while (*str) if (!isdigit(*str++)) return 0;
	return 1;
}


/*** Peforms the same as strstr, in that it returns a pointer to the first occurence
     of pat in str - except that this is performed case insensitive
     ***/
char *istrstr(char *str, char *pat)
{
	char *pptr, *sptr, *start;
	int  slen, plen;

	set_crash();
	slen=strlen(str);
	plen=strlen(pat);
	for (start=(char *)str,pptr=(char *)pat;slen>=plen;start++,slen--) {
  /* find start of pattern in string */
		while (toupper(*start)!=toupper(*pat)) {
			start++;  slen--;
    /* if pattern longer than string */
			if (slen<plen) return(NULL);
			}
		sptr=start;
		pptr=(char *)pat;
		while (toupper(*sptr)==toupper(*pptr)) {
			sptr++;  pptr++;
    /* if end of pattern then pattern was found */
			if (*pptr=='\0') return (start);
			} /* end while */
		} /* end for */
	return(NULL);
}


/*** Take the string 'inpstr' and replace any occurence of 'old' with
     the string 'new'
     ***/
char *replace_string(char *inpstr, char *old, char *new)
{
	int old_len,new_len;
	char *x,*y;

	set_crash();
	if (NULL==(x=(char *)istrstr(inpstr,old))) return x;
	old_len=strlen(old);
	new_len=strlen(new);
	memmove(y=x+new_len,x+old_len,strlen(x+old_len)+1);
	memcpy(x,new,new_len);
	return inpstr;
}


/*** Searches string s1 for string s2 ***/
int s_instr(char *s1, char *s2)
{
	int f,g;

	set_crash();
	for (f=0;*(s1+f);++f) {
		for (g=0;;++g) {
			if (*(s2+g)=='\0' && g>0) return f;
			if (*(s2+g)!=*(s1+f+g)) break;
			}
		}
	return -1;
}


/* used to copy out a chuck of text in macros */
void midcpy(char *strf, char *strt, int fr, int to)
{
	int f;

	set_crash();
	for (f=fr;f<=to;++f) {
		if (!strf[f]) { strt[f-fr]='\0';  return; }
		strt[f-fr]=strf[f];
		}
	strt[f-fr]='\0';
}


/*** Get ordinal value of a date and return the string ***/
char *ordinal_text(int num)
{
	char *ords[]={"th","st","nd","rd"};

	set_crash();
	if (((num%=100)>9 && num<20) || (num%=10)>3) num=0;
	return ords[num];
}


/*** Date string for board messages, mail, .who and .allclones, etc ***/
char *long_date(int which)
{
static char dstr[80];
int ap,hour;

	set_crash();
if (thour>=12) {
  (thour>12) ? (hour=(int)thour-12) : (hour=12);
  ap=1;
  }
else {
  (!thour) ? (hour=12) : (hour=(int)thour);
  ap=0;
  }
if (which) sprintf(dstr,"v %s %d %s %d o %02d:%02d%s",day[twday],tmday,month[tmonth],(int)tyear,hour,(int)tmin,!ap?"":"pm");
else sprintf(dstr,"[ %s %d %s %d o %02d:%02d%s ]",day[twday],tmday,month[tmonth],(int)tyear,hour,(int)tmin,!ap?"":"pm");
return dstr;
}


/* takes string str and determines what smiley type it should have.  The type is then
   stored in 'type'.  The smiley type is determind by the last 2 characters in str.
*/
void smiley_type(char *str, char *type)
{
	set_crash();
	switch(str[strlen(str)-1]) {
		case '?':
			strcpy(type,"ask");
			break;
		case '!':
			strcpy(type,"exclaim");
			break;
		case ')':
			if (str[strlen(str)-2]==':') strcpy(type,"smile");
			else if (str[strlen(str)-2]=='=') strcpy(type,"smile");
			else if (str[strlen(str)-2]==';') strcpy(type,"wink");
			else if (str[strlen(str)-2]=='8') strcpy(type,"glaze");
			else strcpy(type,"say");
			break;
		case '(':
			if (str[strlen(str)-2]==':' || str[strlen(str)-2]=='=') strcpy(type,"frown");
			else strcpy(type,"say");
			break;
		case ':':
			if (str[strlen(str)-2]=='(') strcpy(type,"smile");
			else if (str[strlen(str)-2]==')') strcpy(type,"frown");
			else strcpy(type,"say");
			break;
		case '=':
			if (str[strlen(str)-2]=='(') strcpy(type,"smile");
			else if (str[strlen(str)-2]==')') strcpy(type,"frown");
			else strcpy(type,"say");
			break;
		case ';':
			if (str[strlen(str)-2]=='(') strcpy(type,"wink");
			else if (str[strlen(str)-2]==')') strcpy(type,"frown");
			else strcpy(type,"say");
			break;
		case '8':
			if (str[strlen(str)-2]=='(') strcpy(type,"gaze");
			else strcpy(type,"say");
			break;
		default :
			strcpy(type,"say");
			break;
  }
}


/* This allows you to center text to a given size.  It will also allow you
   to have markers on the edges of the text, if mark = 1, and the marker must
   be passed if mark = 1. marker is an int - one symbol only.
   Based on code by Mike Irving (Moe - MoeNUTS)
*/
char *center_string(int cstrlen, int mark, char *marker, char *str, ...)
{
va_list args;
char text2[ARR_SIZE*2];
int len=0,spc=0,odd=0,cnt=0;

	set_crash();
/* first build up the string */
vtext[0]='\0';
va_start(args,str);
vsprintf(vtext,str,args);
va_end(args);
/* get size */
cnt=colour_com_count(vtext);
len=strlen(colour_com_strip(vtext));
spc=(int)((cstrlen/2)-(len/2));
odd=((spc+spc+len)-cstrlen);
/* if greater than size given then don't do anything except return */
if (len>cstrlen) return(vtext);
sprintf(text2,"%*.*s%s",spc,spc," ",vtext);
/* if marked, then add spaces on the other side too */
if (mark) {
  /* if markers can't be placed without over-writing text then return */
  if (len>(cstrlen-2)) return(vtext);
  /* if they forgot to pass a marker, use a default one */
  if (!marker) marker="|";
  /* compensate for uneven spaces */
  sprintf(vtext,"%s%*.*s\n",text2,spc-odd,spc-odd," ");
  vtext[0]=marker[0];
  vtext[strlen(vtext)-2]=marker[0];
  }
else {
  strcpy(vtext,text2);
  strcat(vtext,"\n");
  }
return(vtext);
}


/* Centers the text */
char *center(char *string, int clen)
{
	static char text2[ARR_SIZE*2];
	char text3[ARR_SIZE*2];
	int len=0,spc=0;

	set_crash();
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


int is_fnumber(char *str)
{
	int d=0;
	char *tmp=str;

	set_crash();
	if (str[0]=='-') str++;
	while (*str) {
		if (!isdigit(*str++)) {
			if (*(--str)=='.') {
				if (str==tmp) return 0;
				if (*(str-1)=='-') return 0;
				if (!d) d=1;
				else return 0;
				}
			else return 0;
			str++;
			}
		}
	return 1;
}


int is_inumber(char *str)
{
	set_crash();
	if (str[0]=='-') str++;
	while(*str) if (!isdigit(*str++)) return 0;
	return 1;
}


char *replace_swear(char *inpstr, char *old)
{
	int i;
	char *x, *y;

	set_crash();
	if (NULL==(x=(char *)istrstr(inpstr, old))) return x;
	for (i=0; i<(strlen(old)-2); i++) memcpy(y=x+1+i, ".", 1);
	return inpstr;
}


void reset_murlist(UR_OBJECT user)
{
	int i;

	set_crash();
	for (i=0; i<MAX_MUSERS; i++) user->murlist[i][0]='\0';
}


char * grm_gnd(int typ, int gnd)
{
	set_crash();
	switch (typ) {
		case  1: return ((gnd==0)?"e":((gnd==1)?"y":"a"));
		case  2: return ((gnd==0)?"ca":((gnd==1)?"":"ka"));
		case  3: return ((gnd==0)?"e":((gnd==1)?"":"a"));
		case  4: return ((gnd==0)?"o":((gnd==1)?"":"a"));
		case  5: return ((gnd==0)?"lo":((gnd==1)?"ol":"la"));
		case  6: return ((gnd==0)?"catu":((gnd==1)?"ovi":"ke"));
		case  7: return ((gnd==0)?"lo":((gnd==1)?"iel":"la"));
		case  8: return ((gnd==0)?"to":((gnd==1)?"ho":"ju"));
		case  9: return ((gnd==0)?"ca":((gnd==1)?"a":"ku"));
		case 10: return ((gnd==0)?"oto":((gnd==1)?"ohoto":"uto"));
		default: return NULL;
		}
}


/* koncovky podla cisla */
char * grm_num(int typ, int n)
{
	set_crash();
	switch (typ) {
		case  1 : return (((n)>4 || (n)==0)?"":(((n)==1)?"u":"y"));
		case  2 : return (((n)>4 || (n)==0)?"ok":(((n)==1)?"ku":"ky"));
		case  3 : return (((n)>4 || (n)==0)?"ych":(((n)==1)?"u":"e"));
		case  4 : return (((n)>4 || (n)==0)?"kov":(((n)==1)?"ok":"ky"));
		case  5 : return (((n)>4 || (n)==0)?"ov":(((n)==1)?"":"i"));
		case  6 : return (((n)>4 || (n)==0)?"ych":(((n)==1)?"y":"i"));
		case  7 : return (((n)>4 || (n)==0)?"ov":(((n)==1)?"":"y"));
		case  8 : return (((n)>4 || (n)==0)?"":(((n)==1)?"a":"y"));
		case  9 : return (((n)>4 || (n)==0)?"ych":(((n)==1)?"a":"e"));
		case 10 : return (((n)>4 || (n)==0)?"rov":(((n)==1)?"er":"ri"));
		default : return NULL;
		}
}


void split_com_str_num(char *inpstr, int num)
{
	char tmp[ARR_SIZE*2];

	set_crash();
	strcpy(tmp, &inpstr[num]);
	inpstr[num]=' ';
	strcpy(&inpstr[num+1], tmp);
	word_count=wordfind(inpstr);
}


/* sklonovanie mena podla padov */
void nick_grm(UR_OBJECT user)
{
	char c1,c2,c3;

	set_crash();
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
							case 'd' :
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
			default  : strcat(user->named, "u"); break;
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
							case 'd' :
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
							case 'd' :
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
							case 'd' :
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
/* privlastnovaci tvar pre gnd=1 */
	strcpy(user->namex, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'o' : strcat(user->namex, "v"); break;
			case 'e' :
				user->namex[strlen(user->namex)-1]='\0';
				strcat(user->namex, "ov");
				break;
			case 'a' : strcat(user->namex, "tov"); break;
			}
		}
	else if (user->gender==1) {
		switch (c1) {
			case 'y' : strcat(user->namex, "ho"); break;
			case 'o' : strcat(user->namex, "v"); break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 'd' :
							case 't' :
								user->namex[strlen(user->namex)-2]='\0';
								strcat(user->namex, "rov");
								break;
							default : strcat(user->namex, "ov"); break;
							};
						break;
					default : strcat(user->namex, "ov"); break;
					};
				break;
			case 'a' :
				user->namex[strlen(user->namex)-1]='\0';
				strcat(user->namex, "ov");
				break;
			default : strcat(user->namex, "ov"); break;
			}
		}
	else if (user->gender==2) {
		switch (c1) {
			case 'a' : user->namex[strlen(user->namex)-1]='\0';
			default :
				strcat(user->namex, "in");
				break;
			}
		}
/* privlastnovaci tvar pre gnd=2 */
	strcpy(user->namey, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'o' : strcat(user->namey, "va"); break;
			case 'a' : strcat(user->namey, "tova"); break;
			case 'e' :
				user->namex[strlen(user->namey)-2]='\0';
				strcat(user->namey, "ova");
				break;
			}
		}
	else if (user->gender==1) {
		switch (c1) {
			case 'y' : strcat(user->namey, "ho"); break;
			case 'o' : strcat(user->namey, "va"); break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 'd' :
							case 't' :
								user->namey[strlen(user->namey)-2]='\0';
								strcat(user->namey, "rova");
								break;
							default : strcat(user->namey, "ova"); break;
							};
						break;
					default : strcat(user->namey, "ova"); break;
					};
				break;
			case 'a' :
				user->namey[strlen(user->namey)-1]='\0';
				strcat(user->namey, "ova");
				break;
			default : strcat(user->namey, "ova"); break;
			}
		}
	else if (user->gender==2) {
		switch (c1) {
			case 'a' : user->namey[strlen(user->namey)-1]='\0';
			default :
				strcat(user->namey, "ina");
				break;
			}
		}
/* privlastnovaci tvar pre gnd=0 */
	strcpy(user->namez, user->name);
	if (user->gender==0) {
		switch (c1) {
			case 'o' : strcat(user->namez, "vo"); break;
			case 'a' : strcat(user->namez, "tovo"); break;
			case 'e' :
				user->namez[strlen(user->namez)-2]='\0';
				strcat(user->namez, "ovo");
				break;
			}
		}
	else if (user->gender==1) {
		switch (c1) {
			case 'y' : strcat(user->namez, "ho"); break;
			case 'o' : strcat(user->namez, "ve"); break;
			case 'r' :
				switch (c2) {
					case 'e' :
						switch (c3) {
							case 'd' :
							case 't' :
								user->namez[strlen(user->namez)-2]='\0';
								strcat(user->namez, "rovo");
								break;
							default : strcat(user->namez, "ovo"); break;
							};
						break;
					default : strcat(user->namez, "ovo"); break;
					};
				break;
			case 'a' :
				user->namez[strlen(user->namez)-1]='\0';
				strcat(user->namez, "ovo");
				break;
			default : strcat(user->namez, "ovo"); break;
			}
		}
	else if (user->gender==2) {
		switch (c1) {
			case 'a' : user->namez[strlen(user->namez)-1]='\0';
			default :
				strcat(user->namez, "ino");
				break;
			}
		}
}


int contains_extension(char *str, int type)
{
	char *s;
	int ok;
	set_crash();
	if ((s=(char *)malloc(strlen(str)+1))==NULL) {
		write_syslog(ERRLOG, 1, "Failed to allocate memory in contains_extention().\n");
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


/*** Fill buff with '\0' ***/
void resetbuff(char *buff) 
{
	int i;
	set_crash();
	for (i=strlen(buff)-1; i>=0; buff[i--]='\0')
	;
} 


/*** Compare two strings (case insensitive) ***/
char stricmp(char *str1, char *str2)
{
	set_crash();
	for ( ; toupper(*str1)==toupper(*str2); str1++,str2++)
		if(*str1=='\0') return 0;
	return (toupper(*str1)-toupper(*str2));
}

      
#endif /* s_string.c */

