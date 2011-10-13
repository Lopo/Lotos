/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
                 Funkcie pre Lotos v1.2.0 na pracu so sietou
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/
#ifndef __S_NET_C__
#define __S_NET_C__ 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "define.h"
#include "prototypes.h"
#include "obj_sys.h"
#include "s_net.h"


/*** Get host structure, using hostsfile info 
     ( written by Mituc <BlackBlue@DeathsDoor.Com>) ***/
struct hostent *fgethostbyname(char *hostname)
{
	FILE *fp;
	char siteaddr[SITE_NAME_LEN+1],sitename[SITE_NAME_LEN+1];
	char int_h_addr_seq,*temp;
	struct hostent *host;
	int i,found=0;

	set_crash();
	if (!use_hostsfile) return NULL;
	if ((temp=(char *)malloc(SITE_NAME_LEN))==NULL) return NULL;

	if(!(fp=fopen(HOSTSFILE, "r"))) return NULL;

	fscanf(fp, "%s %s", siteaddr, sitename);

	while (!feof(fp)) {
		if(!strcmp(hostname,sitename)) { /* found */
			found=1;
			break;
			}
		fscanf(fp,"%s %s",siteaddr,sitename);          
		}
	fclose(fp);

	if (!found) return NULL;

	if ((host=(struct hostent *)malloc(sizeof(struct hostent)))==NULL) return NULL;

	if ((host->h_addr_list=(char **)malloc(2))==NULL) return NULL;      
	if ((host->h_addr_list[0]=(char *)malloc(10))==NULL) return NULL;  
	if ((host->h_name=(char *)malloc(strlen(sitename)+1))==NULL) return NULL;  

	strcpy(temp,siteaddr);
	temp=(char *)strtok(temp,".");

	i=0; /* get IP components */
	while (temp!=NULL && strlen(temp)>0) {
		int_h_addr_seq=atoi(temp);
		host->h_addr_list[0][i]=int_h_addr_seq;
		i++;
		temp=(char *)strtok(NULL,".");
		}

	host->h_addrtype=AF_INET;
	host->h_length=strlen(host->h_addr_list[0]);
	return host;
}


/*** Get user account, using ident daemon of remote site 
     (written by Mituc <BlackBlue@DeathsDoor.Com>) ***/
int ident_request(struct hostent *rhost, int rport, int lport, char *accname)
{
	int sockid,i,n_msgs,nbread,partial_nbread,read_count;
	struct sockaddr_in forident;
	char *inbuf,outbuf[ID_BUFFLEN+1],*temp,*msgs[4],mask;
	struct timeval read_timeout;
	fd_set readfds;
  
	set_crash();
	if ((temp=(char *)malloc(ID_BUFFLEN+1))==NULL) return ID_NOMEM;
	if ((inbuf=(char *)malloc(ID_BUFFLEN+1))==NULL) return ID_NOMEM;
	if ((sockid=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1) /* open a socket for ident */
		return ID_CONNERR;

	forident.sin_family=rhost->h_addrtype;
	forident.sin_addr.s_addr=*((long *)rhost->h_addr_list[0]);
	forident.sin_port=htons(113);

	if((connect(sockid,(struct sockaddr *)&forident,sizeof(forident)))!=0) { /* ident is not running */
		close(sockid);
		return ID_NOFOUND;
		}

	/* begin interogation... */
	sprintf(outbuf,"%d,%d\n",rport,lport);
	if (write(sockid,outbuf,strlen(outbuf))==-1) {
		close(sockid);
		return ID_WRITEERR;      
		}

	resetbuff(inbuf);
	read_timeout.tv_sec=ID_READTIMEOUT;
	read_timeout.tv_usec=0;
	FD_ZERO(&readfds);
	FD_SET(sockid,&readfds);
	nbread=0;
	read_count=0;
	partial_nbread=0;
	while (partial_nbread<2 && read_count<3) {
		if(select(FD_SETSIZE,&readfds,NULL,NULL,&read_timeout)==-1) return ID_TIMEOUT;
      /* There is no need to see if FD_ISSET(sockid,&readfds) is true because
         we have only one file descriptor in readfds, and a select error will
         refere only this file descriptor (sockid).
       */
		read_count++;
		if ((partial_nbread=read(sockid,&inbuf[nbread],ID_BUFFLEN-nbread))==-1) {
			close(sockid);
			return ID_CLOSED;
			}
		nbread+=partial_nbread;
		}
	close(sockid); /* end interogation */

	temp=strtok(inbuf,":");
	n_msgs=0;
  /* We'll restrict the numbers of items read here to 4 just in case
    a lasy ident daemon runs on a given host... and who can give us
    gigabytes of auth infos...
  */
	while (temp!=NULL && n_msgs<4) {
		if((msgs[n_msgs]=(char *)malloc(ID_BUFFLEN+1))==NULL) return ID_NOMEM;
		strcpy(msgs[n_msgs],temp);
      /* We'll clean the return ident messages...
         We wasted a lot of CPU time already...:-|
       */
		while (strlen(msgs[n_msgs])>0 && !isalnum(msgs[n_msgs][0])) (msgs[n_msgs])++;
		i=strlen(msgs[n_msgs])-1;
		while (i>0 && !isalnum(msgs[n_msgs][i])) msgs[n_msgs][i--]='\0';
		if (msgs[n_msgs]!=NULL && strlen(msgs[n_msgs])>2) n_msgs++;
		temp=strtok(NULL,":");
		}
  /* The returned value should be like: "item1 : item2 : item3" in
     case of an error or "item1 : item2 : item3 :item4" on success.
   */
	if (n_msgs<2) return ID_READERR; /* Or this should be ID_CRAP...? */
  /* Conforming to the RFC 1413 document, the second item (n_msgs=1)
     should contain the keyword "ERROR" or "USERID"...
  */
	if (!stricmp(msgs[1],"ERROR")) {
		mask=	 2*(!stricmp(msgs[2],"NO-USER"))+
				 4*(!stricmp(msgs[2],"INVALID-PORT"))+
				 8*(!stricmp(msgs[2],"HIDDEN-USER"))+
				16*(!stricmp(msgs[2],"UNKNOWN-ERROR"));
		switch (mask) {
			case 2: return ID_NOUSER;
			case 4: return ID_INVPORT;
			case 8: return ID_HIDDENUSER;
			case 16: return ID_UNKNOWNERR;
			default: return ID_CRAP;
			}
		}
	else if(stricmp(msgs[1],"USERID")) return ID_READERR;
  /* strcmp() can be used succesfuly too, but a case
     insensitive comparation seems better to me... */
  /* It's ok now... all tests where succesfuly passed. */
	strcpy(accname,msgs[n_msgs-1]);
	return ID_OK; /* Here we go... */
}


/*** Get user's Real name via mail daemon
     (written by Mituc <BlackBlue@DeathsDoor.Com>) ***/
int mail_id_request(struct hostent *rhost, char *accname, char *email)
{
	struct hostent *lhost;
	struct sockaddr_in forident;
	char *inbuf,outbuf[ID_BUFFLEN+1],localhostname[SITE_NAME_LEN+1];
	char *temp,*msgs[10];
	int sockmail,i;
	int nbread,read_count,partial_nbread,n_msgs;
	struct timeval read_timeout;
	fd_set readfds;

	set_crash();
	if ((inbuf=(char *)malloc(ID_BUFFLEN+1))==NULL) return ID_NOMEM;
	if((sockmail=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==-1) /* open a socket for mail ident */
		return ID_CONNERR;

	forident.sin_family=rhost->h_addrtype;
	forident.sin_addr.s_addr=*((long *)rhost->h_addr_list[0]);
	forident.sin_port=htons(25);

	if ((connect(sockmail,(struct sockaddr *)&forident,sizeof(forident)))==-1) {
      /* mail daemon is not running or another error occured */
		close(sockmail);
		return ID_NOFOUND;
		}

  /* preparing to interogate mail daemon... */
	strncpy(localhostname,(char *)getenv("HOSTNAME"),SITE_NAME_LEN);

  /* On missconfigurated systems we may have the unpleasant surprise to 
     see that the HOSTNAME variable is not set or there is no host 
     entry for the value it is set to.
   */
	if ((lhost=gethostbyname(localhostname))==NULL)
		strcpy(localhostname,"localhost");
        /* Some problems with "helo" may occure now...!:-/ */
	else strncpy(localhostname,lhost->h_name,SITE_NAME_LEN);

  /* begin interogation... */
	resetbuff(inbuf);
	read_timeout.tv_sec=ID_READTIMEOUT;
	read_timeout.tv_usec=0;
	FD_ZERO(&readfds);
	FD_SET(sockmail,&readfds);
	nbread=0;
	read_count=0;
	partial_nbread=0;
	while (partial_nbread<2 && read_count<3) {
		if (select(FD_SETSIZE,&readfds,NULL,NULL,&read_timeout)==-1) return ID_TIMEOUT;
      /* There is no need to see if FD_ISSET(sockmail,&readfds) is true because
         we have only one file descriptor in readfds, and a select error will
         refere only this file descriptor (sockmail). This comment is for the
         select()s below, too.
       */
		read_count++;
		if ((partial_nbread=read(sockmail,&inbuf[nbread],ID_BUFFLEN-nbread))==-1) {
			close(sockmail);
			return ID_CLOSED;
			}
		nbread+=partial_nbread;  
		}

  /* send 'questions'... */
	resetbuff(inbuf);
	resetbuff(outbuf);
	sprintf(outbuf,"helo %s\n",localhostname);
	if (write(sockmail,outbuf,strlen(outbuf))==-1) {
		close(sockmail);
		return ID_WRITEERR;
		}

	read_timeout.tv_sec=ID_READTIMEOUT;
	read_timeout.tv_usec=0;
	FD_ZERO(&readfds);
	FD_SET(sockmail,&readfds);
	nbread=0;
	read_count=0;
	partial_nbread=0;
	while (partial_nbread<2 && read_count<3) {
		if(select(FD_SETSIZE,&readfds,NULL,NULL,&read_timeout)==-1) return ID_TIMEOUT;
		read_count++;
		if ((partial_nbread=read(sockmail,&inbuf[nbread],ID_BUFFLEN-nbread))==-1) {
			close(sockmail);
			return ID_CLOSED;
			}
		nbread+=partial_nbread;
		}

	resetbuff(inbuf);
	resetbuff(outbuf);       
	sprintf(outbuf,"vrfy %s\n",accname);
	if (write(sockmail,outbuf,strlen(outbuf))==-1) {
		close(sockmail);
		return ID_WRITEERR;
		}

	read_timeout.tv_sec=ID_READTIMEOUT;
	read_timeout.tv_usec=0;
	FD_ZERO(&readfds);
	FD_SET(sockmail,&readfds);

	nbread=0;
	read_count=0;
	partial_nbread=0;
	while (partial_nbread<2 && read_count<3) {
		if(select(FD_SETSIZE,&readfds,NULL,NULL,&read_timeout)==-1) return ID_TIMEOUT;
		read_count++;
		if ((partial_nbread=read(sockmail,&inbuf[nbread],ID_BUFFLEN-nbread))==-1) {
			close(sockmail);
			return ID_READERR;
			}
		nbread+=partial_nbread;
		}

	sprintf(outbuf,"quit\n");
	write(sockmail,outbuf,strlen(outbuf)); /* it isn't interesting here if write fails...*/
	close(sockmail);
	/* end interogation */

	temp=strtok(inbuf," ");
	n_msgs=0;
  /* We will restrict the number of items read here just like in
    ident_request(), not to 4, but 10, 'cause there are some lasy people
    over the world ( Like me, yeah !:) ) whom name can contain a lot of
    initials, father's initial(s), mother's ( why not ?), and another
    one bilion of names. */

	while (temp!=NULL && n_msgs<10) {
		if((msgs[n_msgs]=(char *)malloc(ID_BUFFLEN+1))==NULL) return ID_NOMEM;
		strcpy(msgs[n_msgs],temp);
	/* We'll clean this messages right now! We wasted
	   a lot of CPU time already and this is not all...:-| */
		while (strlen(msgs[n_msgs]) >0 && !isalnum(msgs[n_msgs][0])) (msgs[n_msgs])++;
		i=strlen(msgs[n_msgs])-1;
		while (i>0 && !isalnum(msgs[n_msgs][i])) msgs[n_msgs][i--]='\0';
		if (msgs[n_msgs]!=NULL && strlen(msgs[n_msgs])>0) n_msgs++;
		temp=strtok(NULL," ");
		}
                                                                                              
	if (n_msgs<1) return ID_READERR;
	i=atoi(msgs[0]);
	switch (i) {
		case 500: return ID_COMERR;
		case 550: return ID_UNKNOWN;
      /* I should add more values here, so the return messages will
         be mode comprehensible...
      */
		case 250:
			if (!strchr(msgs[n_msgs-1],'@')) return ID_CRAP;
			strcpy(email,"");
			for (i=1;i<n_msgs-1;i++) sprintf(email,"%s %s",email,msgs[i]);
			sprintf(email,"%s <%s>",email,msgs[n_msgs-1]);
			return ID_OK;
		default: return ID_CRAP;
		}
}


/*** Check if a host is in hosts file ***/
int check_host(char *ip_site, char *named_site)
{
	FILE *fp;
	char siteaddr[82], sitename[82];

	set_crash();
	if (!use_hostsfile) return 0;
	if ((fp=fopen(HOSTSFILE, "r"))==NULL) return 0;
	fscanf(fp, "%s %s", siteaddr, sitename);
	while (!feof(fp)) {
		if (!strcmp(ip_site, siteaddr)) {
			fclose(fp);
			strcpy(named_site, sitename);
			return 1;
			}
		fscanf(fp, "%s %s", siteaddr, sitename);
		}
	fclose(fp);
	return 0;
}


/*** Init the hosts file ***/
void init_hostsfile(void)
{
	FILE *fp;

	set_crash();
	if (!use_hostsfile) return;
	if ((fp=fopen(HOSTSFILE, "r"))==NULL) {
		if((fp=fopen(HOSTSFILE, "w"))==NULL) {
			write_syslog(ERRLOG, 1, "Cannot read file in add_host().\n");
			return;
			}
		fprintf(fp, "127.0.0.1 localhost\n");
		fclose(fp);
		return;
		}
}

/*** Add a site in hosts file ***/
void add_host(char *siteaddr, char *sitename)
{
	FILE *fp;
	char asite[80], aname[80];

	set_crash();
	if (!use_hostsfile) return;
	if ((fp=fopen(HOSTSFILE,"r"))==NULL) {
		write_syslog(ERRLOG, 1, "Cannot read file in add_host().\n");
		return;
		}
	fscanf(fp, "%s %s", asite, aname);
	while (!feof(fp)) {
		if (!strcmp(asite, siteaddr)) {
			fclose(fp);  /* found, so don't want to write in file */
			return;
			}
		fscanf(fp, "%s %s", asite, aname);
		}
	fclose(fp);
	if ((fp=fopen(HOSTSFILE,"a"))==NULL) {
		write_syslog(ERRLOG, 1, "Cannot append file in add_host().\n");
		return;
		}
	fprintf(fp, "%s %s\n", siteaddr, sitename);
	fclose(fp);
}    


/*** Get net address of accepted connection ***/
void get_net_addresses(struct sockaddr_in acc_addr, char *ip_site, char *named_site)
{
	struct hostent *host;

	set_crash();
	/* Get number addr */
	strcpy(ip_site,(char *)inet_ntoa(acc_addr.sin_addr));
	/* Get named site
	   Hanging usually happens on BSD systems when using gethostbyaddr.  If this happens
	   to you then alter the resolve_ip setting in the config file.
	*/
	switch (amsys->resolve_ip) {
		case 0: /* don't resolve */
			strcpy(named_site,ip_site);
			return;
		case 1: /* resolve automatically */
			if (!check_host(ip_site, named_site)) {
				if ((host=gethostbyaddr((char *)&acc_addr.sin_addr,sizeof(acc_addr.sin_addr),AF_INET))!=NULL) {
					strcpy(named_site, host->h_name);
					strtolower(named_site);
					add_host(ip_site, named_site);
					}
				else strcpy(named_site,ip_site);
				}
			return;
		case 2: /* resolve with function by tref */
			strcpy(named_site,((char *)resolve_ip(ip_site)));
			return;
		}
}


#endif /* s_net.c */

