/* vi: set ts=4 sw=4 ai: */
/*
 * music.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
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

