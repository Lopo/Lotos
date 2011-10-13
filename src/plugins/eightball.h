/* vi: set ts=4 sw=4 ai: */
#ifndef __PL00x100_H__
#define __PL00x100_H__ 1

extern char *colors[];

int pl00x100_init(int cm);
int pl00x100_main(UR_OBJECT user, char *str, int comid);
void pl00x100_respond(UR_OBJECT user, char *str);

#endif /* __PL00x100_H__ */
