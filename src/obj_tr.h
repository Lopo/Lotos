/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                   Struktura transportu pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __OBJ_TR_H__
#define __OBJ_TR_H__ 1

/* transport information structure */
struct transport_struct {
	int place, route;
	int out, go, smer;
	int time;
	struct room_struct *room;
	struct transport_struct *next, *prev;
	};
typedef struct transport_struct *TR_OBJECT;

#endif /* obj_tr.h */

