/* vi: set ts=4 sw=4 ai: */
/*
 * music.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */
/*==========================================================================*/
/*======================  Lotos  Jukebox File Setup ========================*/
/*==========================================================================*/

#ifndef __MUSIC_H__
#define __MUSIC_H__ 1

/* Enter the titles of your music files here.  Be SURE that they correspond
   to the filenames farther down.  There MUST be '*' as the last entry. */
char *jb_titles[]={
	"Znelka :)",	"Get it?",
	"*"
	};

/* Enter the filenames of the songs above.  Make sure you match the song
   title with the filename correctly. */
char *jb_files[]={
	"overture.wav",	"got_it.mid"
	};

#endif /* __MUSIC_H__ */

