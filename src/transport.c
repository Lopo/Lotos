/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                 Funkcie Lotos v1.2.0 suvisiace s transportami
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/

#ifndef __TRANSPORT_C__
#define __TRANSPORT_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "define.h"
#include "prototypes.h"
#include "obj_ur.h"
#include "obj_rm.h"
#include "obj_tr.h"
#ifdef NETLINKS
	#include "obj_nl.h"
#endif
#include "obj_sys.h"
#include "transport.h"
#include "comvals.h"


TR_OBJECT create_transport(void)
{
	TR_OBJECT tr;

	set_crash();
	if ((tr=(TR_OBJECT)malloc(sizeof(struct transport_struct)))==NULL) {
		fprintf(stderr, "Lotos: Memory allocation failure in create_transport().\n");
		boot_exit(1);
		}
	if (transport_first==NULL) {
		transport_first=tr;
		tr->prev=NULL;
		}
	else {
		transport_last->next=tr;
		tr->prev=transport_last;
		}
	tr->next=NULL;
	transport_last=tr;

	tr->place=tr->route=0;
	tr->out=tr->go=tr->smer=0;
	tr->time=0;
	tr->room=NULL;
	return tr;
}

void destruct_transport(TR_OBJECT tr)
{
	set_crash();
	if (tr==transport_first) {
		transport_first=tr->next;
		if (tr==transport_last) transport_last=NULL;
		else transport_first->prev=NULL;
		}
	else {
		tr->prev->next=tr->next;
		if (tr==transport_last) {
			transport_last=tr->prev;
			transport_last->next=NULL;
			}
		else tr->next->prev=tr->prev;
		}
	free(tr);
}


void transport_plane(UR_OBJECT user)
{
	RM_OBJECT rm;
	TR_OBJECT tr;
	int i;

	set_crash();
	tr=user->room->transp;
	if (word_count<2){
		if (tr==NULL) {
			write_usage(user,"tplan <name>/all");
			return;
			}
		vwrite_user(user, "Transport name: %s\n", tr->room->name);
		vwrite_user(user, "na zastavke stoji: %d sek.\n", tr->place);
		vwrite_user(user, "cas trvania cesty: %d sek.\n", tr->route);
		write_user(user, "akt. stav:\n");
		if (tr->go) {
			write_user(user, "~FRna ceste\n");
			vwrite_user(user, "zastavi za %d sek. v roome %s\n",
				((tr->route-tr->time)>=0) ? (tr->route-tr->time) : 0,
				((tr->room->link[tr->out+tr->smer])!=NULL) ? tr->room->link[tr->out+tr->smer]->name : tr->room->link[tr->out-tr->smer]->name
				);
			}
		else {
			vwrite_user(user, "stoji na zastavke %s este %d sek.\n",
				tr->room->link[tr->out]->name,
				((tr->place-tr->time)>=0) ? (tr->place-tr->time) : 0
				);
			}
		return;
		}
	
	if (strcmp(word[1], "all")) {
		if ((rm=get_room(word[1]))==NULL) {
			write_user(user, nosuchtr);
			return;
			}
		tr=rm->transp;
		if (tr==NULL) {
			write_user(user, nosuchtr);
			return;
			}
		vwrite_user(user, "Transport name: %s\n", tr->room->name);
		vwrite_user(user, "na zastavke stoji: %d sek.\n", tr->place);
		vwrite_user(user, "cas trvania cesty: %d sek.\n", tr->route);
		write_user(user, "akt. stav:\n");
		if (tr->go) {
			write_user(user, "~FRna ceste\n");
			vwrite_user(user, "zastavi za %d sek. v roome %s\n",
				((tr->route-tr->time)>=0) ? (tr->route-tr->time) : 0,
				((tr->room->link[tr->out+tr->smer])!=NULL) ? tr->room->link[tr->out+tr->smer]->name : tr->room->link[tr->out-tr->smer]->name
				);
			}
		else {
			vwrite_user(user, "stoji na zastavke %s este %d sek.\n",
				tr->room->link[tr->out]->name,
				((tr->place-tr->time)>=0) ? (tr->place-tr->time) : 0
				);
			}
		}
	else {
		write_user(user, "Transport name       : place route zastavky\n");
		for (tr=transport_first; tr!=NULL; tr=tr->next) {
			vwrite_user(user, "%-20s :  %3d   %3d ", tr->room->name, tr->place, tr->route);
			for (i=0; tr->room->link[i]!=NULL; i++) {
				if (tr->room->link[i]==NULL) break;
				if (!tr->go && tr->out==i)
					vwrite_user(user, " ~OL~FG%s~RS", tr->room->link[i]->label);
				else
					vwrite_user(user, " %s", tr->room->link[i]->label);
				}
			write_user(user, "\n");
			}
		write_user(user, "\n");
		}
}


void write_transport_except(TR_OBJECT tr, char *str, UR_OBJECT user)
{
	UR_OBJECT u;

	set_crash();
	for (u=user_first; u!=NULL; u=u->next) {
		if (u->room) if (u->room->transp==NULL) continue;
		if (u->login
		    || u->room!=tr->room
		    || (u->ignore.all && !force_listen)
		    || u->ignore.transp
		    || u->misc_op
		    || u->set_op
		    || u->type==CLONE_TYPE
		    || u==user)
			continue;
		if (u->type==CLONE_TYPE) {
			if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignore.all) continue;
			/* Ignore anything not in clones room, eg shouts,
			   system messages and semotes since the clones owner
			   will hear them anyway. */
			if (u->clone_hear==CLONE_HEAR_SWEARS && !contains_swearing(str)) continue;
			vwrite_user(user->owner, "~FT[ %s ]:~RS %s",u->room->name,str);
			}
		else write_user(u,str); 
		} /* end for */
}


void write_transport(TR_OBJECT tr, char *str)
{
	set_crash();
	write_transport_except(tr, str, NULL);
}


void transport(void)
{
	UR_OBJECT u;
	TR_OBJECT tr;
	int tmp;

	set_crash();
	tr=transport_first;
	for (tr=transport_first; tr!=NULL; tr=tr->next) {
		tr->time+=amsys->heartbeat;
		if ((tr->time>=tr->route && tr->go) || (tr->time>=tr->place && !tr->go)) {
			tr->go=!tr->go;
			tr->time=0;
			}
		else continue;
		if (!tr->go) {
			if (tr->out+tr->smer>MAX_LINKS
			    || tr->out+tr->smer<0)
				tr->smer*=(-1);
			else if (tr->room->link[tr->out+tr->smer]==NULL)
				tr->smer*=(-1);
			tr->out+=tr->smer;
			for (u=user_first; u!=NULL; u=u->next) {
				if (u->ignore.all
				    || u->ignore.transp
				    || u->misc_op
				    || u->set_op
				    || u->login
				    || u->type==CLONE_TYPE
				    || u->room!=tr->room->link[tr->out]) continue;
				 else vwrite_user(u, "~OLPrisiel transport '%s', mozes nastupit.\n", tr->room->name);
				}
			sprintf(text, "~OLTransport sa zastavil na zastavke '%s', mozes vystupit\n", tr->room->link[tr->out]->name);
			write_transport(tr, text);
			}
		else {
			for (u=user_first; u!=NULL; u=u->next) {
				if (!u->ignore.all
				    && !u->ignore.transp
					&& !u->login
					&& !u->misc_op
					&& !u->set_op
					&& u->type!=CLONE_TYPE
				    && u->room==tr->room->link[tr->out]
				    ) {
					vwrite_user(u, "~OLTransport '%s' odisiel\n", tr->room->name);
					}
				}
			if (tr->out+tr->smer>MAX_LINKS
			    || tr->out+tr->smer<0)
				tmp=tr->out-tr->smer;
			else if (tr->room->link[tmp=tr->out+tr->smer]==NULL)
				tmp=tr->out-tr->smer;
			else tmp=tr->out+tr->smer;
			sprintf(text, "~OLTransport sa pohol, nasledujuca zastavka je '%s'\n", tr->room->link[tmp]->name);
			write_transport(tr, text);
			} /* end else go */
		} /* end for tr */
}

#endif /* transport.c */
