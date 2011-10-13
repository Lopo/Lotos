/*****************************************************************************
                      Hlavickovy subor OS Star v1.0.0
            Copyright (C) Pavol Hluchy - posledny update: 2.5.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

/* Structure for net links, ie server initiates them */
struct netlink_struct {
  char service[SERV_NAME_LEN+1];
  char site[SITE_NAME_LEN+1];
  char verification[VERIFY_LEN+1];
  char buffer[ARR_SIZE*2];
  char mail_to[WORD_LEN+1];
  char mail_from[WORD_LEN+1];
  FILE *mailfile;
  time_t last_recvd; 
  int port,socket,type,connected;
  int stage,lastcom,allow,warned,keepalive_cnt;
  int ver_major,ver_minor,ver_patch;
  struct user_struct *mesg_user;
  struct room_struct *connect_room;
  struct netlink_struct *prev,*next;
};
typedef struct netlink_struct *NL_OBJECT;