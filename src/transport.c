/*****************************************************************************
               Funkcie OS Star v1.1.0 suvisiace s transportami
            Copyright (C) Pavol Hluchy - posledny update: 15.8.2000
          osstar@star.sjf.stuba.sk  |  http://star.sjf.stuba.sk/osstar
 *****************************************************************************/

#include <stdio.h>
#include <time.h>

#include "define.h"
#include "ur_obj.h"
#include "rm_obj.h"
#ifdef NETLINKS
	#include "nl_obj.h"
#endif
#include "sys_obj.h"
#include "transport.h"
#include "comvals.h"

/* prototypy */
RM_OBJECT get_room(char *name);


void transport_plane(UR_OBJECT user)
{
	RM_OBJECT tr;
	int i;

	tr=user->room;
	if (word_count<2){
		if (!tr->tr) {
			write_usage(user,"tplan <name>/all");
			return;
			}
		vwrite_user(user, "Transport name: %s\n", tr->name);
		vwrite_user(user, "na zastavke stoji: %d sek.\n", tr->place);
		vwrite_user(user, "cas trvania cesty: %d sek.\n", tr->route);
		write_user(user, "akt. stav:\n");
		if (tr->go) {
			write_user(user, "~FRna ceste\n");
			vwrite_user(user, "zastavi za %d sek. v roome %s\n",
				((tr->route-tr->time)>=0) ? (tr->route-tr->time) : 0,
				((tr->link[tr->out+tr->smer])!=NULL) ? tr->link[tr->out+tr->smer]->name : tr->link[tr->out-tr->smer]->name
				);
			}
		else {
			vwrite_user(user, "stoji na zastavke %s este %d sek.\n",
				tr->link[tr->out]->name,
				((tr->place-tr->time)>=0) ? (tr->place-tr->time) : 0
				);
			}
		return;
		}
	
	if (strcmp(word[1], "all")) {
		if ((tr=get_room(word[1]))==NULL) {
			write_user(user, nosuchtr);
			return;
			}
		if (!tr->tr) {
			write_user(user, nosuchtr);
			return;
			}
		vwrite_user(user, "Transport name: %s\n", tr->name);
		vwrite_user(user, "na zastavke stoji: %d sek.\n", tr->place);
		vwrite_user(user, "cas trvania cesty: %d sek.\n", tr->route);
		write_user(user, "akt. stav:\n");
		if (tr->go) {
			write_user(user, "~FRna ceste\n");
			vwrite_user(user, "zastavi za %d sek. v roome %s\n",
				((tr->route-tr->time)>=0) ? (tr->route-tr->time) : 0,
				((tr->link[tr->out+tr->smer])!=NULL) ? tr->link[tr->out+tr->smer]->name : tr->link[tr->out-tr->smer]->name
				);
			}
		else {
			vwrite_user(user, "stoji na zastavke %s este %d sek.\n",
				tr->link[tr->out]->name,
				((tr->place-tr->time)>=0) ? (tr->place-tr->time) : 0
				);
			}
		}
	else {
		write_user(user, "Transport name       : place route zastavky\n");
		for (tr=room_first; tr!=NULL; tr=tr->next) {
			if (!tr->tr) continue;
			vwrite_user(user, "%-20s :  %3d   %3d ", tr->name, tr->place, tr->route);
			for (i=0; tr->link[i]!=NULL; i++) {
				if (tr->link[i]==NULL) break;
				if (!tr->go && tr->out==i)
					vwrite_user(user, " ~OL%s~RS", tr->link[i]->label);
				else
					vwrite_user(user, " %s", tr->link[i]->label);
				}
			write_user(user, "\n");
			}
		write_user(user, "\n");
		}
}


void write_transport_except(RM_OBJECT tr, char *str, UR_OBJECT user)
{
	UR_OBJECT u;
	char text2[ARR_SIZE];

	if (tr==NULL) return;
	for(u=user_first; u!=NULL; u=u->next) {
		if (u->room) if (!u->room->tr) continue;
		if (u->login
		    || u->room!=tr
		    || (u->ignall && !force_listen)
		    || u->igntr
		    || u->misc_op
		    || u->set_op
		    || u->type==CLONE_TYPE
		    || u==user)
			continue;
		if (u->type==CLONE_TYPE) {
			if (u->clone_hear==CLONE_HEAR_NOTHING || u->owner->ignall) continue;
			/* Ignore anything not in clones room, eg shouts,
			   system messages and semotes since the clones owner
			   will hear them anyway. */
			if (u->clone_hear==CLONE_HEAR_SWEARS && !contains_swearing(str)) continue;
			vwrite_user(user->owner, "~FT[ %s ]:~RS %s",u->room->name,str);
			}
		else write_user(u,str); 
		} /* end for */
}


void write_transport(RM_OBJECT tr, char *str)
{
	write_transport_except(tr, str, NULL);
}


void transport(void)
{
	UR_OBJECT u;
	RM_OBJECT tr;
	int tmp;

	tr=room_first;
	for (tr=room_first; tr!=NULL; tr=tr->next) {
		if (!tr->tr) continue;
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
			else if (tr->link[tr->out+tr->smer]==NULL)
				tr->smer*=(-1);
			tr->out+=tr->smer;
			for (u=user_first; u!=NULL; u=u->next) {
				if (u->ignall
				    || u->igntr
				    || u->misc_op
				    || u->set_op
				    || u->login
				    || u->type==CLONE_TYPE
				    || u->room!=tr->link[tr->out]) continue;
				 else vwrite_user(u, "~OLPrisiel transport '%s', mozes nastupit.\n", tr->name);
				}
			sprintf(text, "~OLTransport sa zastavil na zastavke '%s', mozes vystupit\n", tr->link[tr->out]->name);
			write_transport(tr, text);
			}
		else {
			for (u=user_first; u!=NULL; u=u->next) {
				if (!u->ignall
				    && !u->igntr
					&& !u->login
					&& !u->misc_op
					&& !u->set_op
					&& u->type!=CLONE_TYPE
				    && u->room==tr->link[tr->out]
				    ) {
					vwrite_user(u, "~OLTransport '%s' odisiel\n", tr->name);
					}
				}
			if (tr->out+tr->smer>MAX_LINKS
			    || tr->out+tr->smer<0)
				tmp=tr->out-tr->smer;
			else if (tr->link[tmp=tr->out+tr->smer]==NULL)
				tmp=tr->out-tr->smer;
			else tmp=tr->out+tr->smer;
			sprintf(text, "~OLTransport sa pohol, nasledujuca zastavka je '%s'\n", tr->link[tmp]);
			write_transport(tr, text);
			} /* end else go */
		} /* end for tr */
}

