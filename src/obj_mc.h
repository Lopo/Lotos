/* vi: set ts=4 sw=4 ai: */
/*
 * obj_mc.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __OBJ_MC_H__
#define __OBJ_MC_H__ 1

#include "define.h"

struct macro_struct {
	char name[MC_NAME_LEN+1], comstr[MC_COM_LEN+1];
	struct macro_struct *prev, *next;
	};
typedef struct macro_struct *MC_OBJECT;

#endif /* __OBJ_MC_H__ */

