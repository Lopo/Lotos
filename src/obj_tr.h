/* vi: set ts=4 sw=4 ai: */
/*
 * obj_tr.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __OBJ_TR_H__
#define __OBJ_TR_H__ 1

#include "obj_rm.h"

/* transport information structure */
struct transport_struct {
	int place, route;
	int out, go, smer;
	int time;
	struct room_struct *room;
	struct transport_struct *next, *prev;
	};
typedef struct transport_struct *TR_OBJECT;

#endif /* __OBJ_TR_H__ */

