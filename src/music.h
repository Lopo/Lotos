/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                       Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/
/*****************************************************************************
    POZOR !!! Tento subor je sucastou rozsirenia pre Pueblo klienta
    zatial experimentalne - nerucim za funkcnost
 *****************************************************************************/
/*==========================================================================*/
/*======================  Lotos  Jukebox File Setup ========================*/
/*==========================================================================*/

#ifndef __MUSIC_H__
#define __MUSIC_H__ 1

/* Enter the titles of your music files here.  Be SURE that they correspond
   to the filenames farther down.  There MUST be '*' as the last entry. */
char *jb_titles[]={
"Znelka :)",	"Get it?",
"*"};

/* Enter the filenames of the songs above.  Make sure you match the song
   title with the filename correctly. */
char *jb_files[]={
"overture.wav",	"got_it.mid"
};

#endif /* music.h */
