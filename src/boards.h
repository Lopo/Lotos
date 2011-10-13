/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                      Hlavickovy subor Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __BORADS_H__
#define __BORADS_H__ 1

extern RM_OBJECT room_first;
extern SYS_OBJECT amsys;

extern char text[];
extern char word[MAX_WORDS][WORD_LEN+1];
extern int thour, tmin;
extern int word_count;

extern char *invisname, *nosuchroom, *syserror;
extern char *message_board_header, *read_no_messages, *user_read_board_prompt;
extern char *write_edit_header, *user_write_end, *room_write_end;
extern char *wipe_empty_board, *wipe_too_many, *wipe_user_delete_range;
extern char *wipe_user_all_deleted, *wipe_room_all_deleted;
extern char *no_message_prompt;
extern char *muzzled_cannot;

#endif
