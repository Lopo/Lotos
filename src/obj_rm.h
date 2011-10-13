/* vi: set ts=4 sw=4 ai: */
/*
 * obj_rm.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __OBJ_RM_H__
#define __OBJ_RM_H__ 1

#include "define.h"

typedef struct {
	time_t time;
	char buff[REVIEW_LEN+2];
	} review_buffer;

/* room information structure */
struct room_struct {
	char name[ROOM_NAME_LEN+1],label[ROOM_LABEL_LEN+1],desc[ROOM_DESC_LEN+1];
	char real_name[PERSONAL_ROOMNAME_LEN];
	char topic[TOPIC_LEN+1], topicowner[1+USER_NAME_LEN*4];
	int topiclock;
	char map[ROOM_NAME_LEN+1];
	int access; /* public , private etc */
	int revline; /* line number for review */
	int mesg_cnt;
	char link_label[MAX_LINKS][ROOM_LABEL_LEN+1]; /* temp store for parse */
	struct room_struct *link[MAX_LINKS];
	struct room_struct *next,*prev;
#ifdef NETLINKS
	int inlink; /* 1 if room accepts incoming net links */
	char netlink_name[SERV_NAME_LEN+1]; /* temp store for config parse */
	struct netlink_struct *netlink; /* for net links, 1 per room */
#endif
	/* rozsirenie pre transporty */
	struct transport_struct *transp;
	review_buffer revbuff[REVIEW_LINES];
	};
typedef struct room_struct *RM_OBJECT;

#endif /* __OBJ_RM_H__ */

