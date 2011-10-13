/*****************************************************************************
                     Struktura miestnost v OS Star v1.0.0b
            Copyright (C) Pavol Hluchy - posledny update: 28.3.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

/* room information structure */
struct room_struct {
  char name[ROOM_NAME_LEN+1],label[ROOM_LABEL_LEN+1],desc[ROOM_DESC_LEN+1];
  char topic[TOPIC_LEN+1],revbuff[REVIEW_LINES][REVIEW_LEN+2],map[ROOM_NAME_LEN+1];
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
  int tr;
  int place, route;
  int out, go, smer;
  int at_place, at_route;
  };
typedef struct room_struct *RM_OBJECT;
