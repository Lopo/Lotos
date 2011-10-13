/* vi: set ts=4 sw=4 ai: */
/*
 * obj_tr.h
 *   Lotos v1.2.1  : (c) 1999-2001 Pavol Hluchy (Lopo)
 *   last update   : 26.12.2001
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
 */

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

