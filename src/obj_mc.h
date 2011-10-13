/* vi: set ts=4 sw=4 ai: */
/*
 * obj_mc.h
 *
 *   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
 *   last update   : 16.5.2002
 *   email         : lopo@losys.sk
 *   homepage      : lopo.losys.sk
 *   Lotos homepage: lotos.losys.sk
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

