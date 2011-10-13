/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                     Struktura pre makra v Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __OBJ_MC_H__
#define __OBJ_MC_H__ 1

struct macro_struct {
	char name[MC_NAME_LEN+1], comstr[MC_COM_LEN+1];
	struct macro_struct *prev, *next;
	};
typedef struct macro_struct *MC_OBJECT;

#endif /* obj_mc.h */
