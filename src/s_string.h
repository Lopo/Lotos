/* vi: set ts=4 sw=4 ai: */
/*
 * s_string.h
 *
 *   Lotos v1.2.1  : (c) 1999-2001 Pavol Hluchy (Lopo)
 *   last update   : 26.12.2001
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

#ifndef __S_STRING_H__
#define __S_STRING_H__ 1

extern SYS_OBJECT amsys;

extern char *month[];
extern char *day[];

extern char swear_words[MAX_SWEARS+1][WORD_LEN+1];

extern char vtext[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int tyear, tmonth, tmday, twday, thour, tmin;
extern int word_count;

extern struct {
	char *esc_code;
	char *txt_code;
	} colour_codes[];

#endif /* s_string.h */

