/* vi: set ts=4 sw=4 ai: */
/*
 * s_net.h
 *
 *   Lotos v1.2.3  : (c) 1999-2003 Pavol Hluchy (Lopo)
 *   last update   : 30.1.2003
 *   email         : lotos@losys.sk
 *   homepage      : lotos.losys.sk
 */

#ifndef __S_NET_H__
#define __S_NET_H__ 1

extern SYS_OBJECT amsys;

extern int use_hostsfile;
extern struct {
	char *text;
	int lag;
	} speeds[];

#endif /* __S_NET_H__ */

