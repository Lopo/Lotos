/* OS Star plugin                      pre verziu 1.1.0 a vyssiu
   -------------------------------------------------------------
   Original: MoeNUTS
   Modifikoval na plug-in pre OS Star: Lopo

   inicializacny riadok pre tento plugin:
   	if (tmp=plugin_02x100_init(cmd)) cmd=cmd+tmp;
   je v subore adds.c

   Volaci prikazu pre tento plugin:
   	if (!strcmp(plugin->registration,"02-100")) { plugin_02x100_main(user,str,comnum); return 1; }
   je v subore plugin.c

   Nasledujuci riadok MUSI byt vlozeny na koniec struktury
   v subore ur_obj.h:
   	struct plugin_02x100_player *plugin_02x100;
   ------------------------------------------------------------- */
 
#include "hangman.h"
 
	extern CM_OBJECT create_cmd(void);
/* ------------------------------------------------------------- */

int plugin_02x100_init(int cm)
{
	PL_OBJECT plugin;
	CM_OBJECT com;
	int i=0,verFail=0;

/* create plugin */
	if ((plugin=create_plugin())==NULL) {
		write_syslog(ERRLOG, 0, "Nemozem vytvorit novu polozku v registroch pre plugin 'hangman'!\n");
		return 0;
		}
	strcpy(plugin->name,"Hangman");                 /* Plugin Description   */
	strcpy(plugin->author,"Lopo");                  /* Author's name        */
	strcpy(plugin->registration,"02-100");          /* Plugin/Author ID     */
	strcpy(plugin->ver,"1.2");                      /* Plugin version       */
	strcpy(plugin->req_ver,"110");                  /* OSS version required */
	plugin->id = cm;                                /* ID used as reference */
	plugin->req_userfile = 1;                       /* Requires user data?  */
	                                                /* (no separate file required
	                                                    since it keeps its data
	                                                    in a central file, but
	                                                    we need to do housekeeping
	                                                    procedures when the user
	                                                    leaves, so we set this to
	                                                    1 so that we are notified
	                                                    when a user leaves.) */
	plugin->triggerable = 0;                        /* This plugin is triggered
	                                                   by the system timer, and
	                                                   it will automatically
	                                                   save the current poker
	                                                   data when the boards
	                                                   are automatically
	                                                   checked. */
/* create associated command */
	if ((com=create_cmd())==NULL) {
		write_syslog(ERRLOG, 0, "Nemozem pridat prikaz do registrov pre plugin %s !\n", plugin->registration);
		return 0;
		}
	i++;                                            /* Keep track of number created */
	strcpy(com->command,"hangman");                 /* Name of command */
	com->id = plugin->id;                           /* Command reference ID */
	com->req_lev = USER;                            /* Required level for cmd. */
	com->comnum = i;
	com->plugin = plugin;
/* end creating command - repeat as needed for more commands */

	return i;
}

int plugin_02x100_main(UR_OBJECT user, char *str, int comid)
{
switch (comid) {
        /* put case calls here for each command needed.
           remember that the commands will be in order, with the first
           command always being 1;
        */
	case -1: return plugin_02x100_reinit_save(user);
	case -2: return plugin_02x100_reinit_load(user);
        case -8: if (user->plugin_02x100!=NULL) plugin_02x100_leave(user);
                 return 1;
        case -9: plugin_02x100_signon(user); return 1;
        case  1: plugin_02x100_switch(user,str); return 1;
        default: return 0;
        }
}


/*** Create a hang player ***/
struct plugin_02x100_player *plugin_02x100_create_player(void)
{
	struct plugin_02x100_player *player;
	int i;

if ((player=(struct plugin_02x100_player *)malloc(sizeof(struct plugin_02x100_player)))==NULL) {
        write_syslog(ERRLOG, 1, "chyba alokacie pamate v 02x100_create_player().\n");
        return NULL;
        }

/* initialise the player */
	player->user = NULL;
	player->win=player->lose=0;
	player->stage=-1;

	return player;
}

void plugin_02x100_vloz(UR_OBJECT user)
{
	int count,i,blanks;

	count=blanks=i=0;
	if (user->plugin_02x100->stage==-1) {
		write_user(user,"Nemas zacatu hru.\n");
		return;
		}
	if (strlen(word[1])>1) {
		write_user(user,"Naraz mozes vlozit len jedno pismeno !\n");
		return;
		}
	strtolower(word[1]);
	if (strstr(user->plugin_02x100->guess,word[1])) {
		user->plugin_02x100->stage++;
		write_user(user,"You have already guessed that letter!  And you know what that means...\n\n");
		vwrite_user(user, hanged[user->plugin_02x100->stage],user->plugin_02x100->word_show, user->plugin_02x100->guess);
		if (user->plugin_02x100->stage>=7) {
			write_user(user,"~FR~OLUh-oh!~RS  You couldn't guess the word and died!\n");
			user->plugin_02x100->stage=-1;
			user->plugin_02x100->word[0]='\0';
			user->plugin_02x100->word_show[0]='\0';
			user->plugin_02x100->guess[0]='\0';
			user->plugin_02x100->lose+=1;
			plugin_02x100_save_user_details(user);
			}
		write_user(user,"\n");
		return;
		}
	for (i=0; i<strlen(user->plugin_02x100->word); ++i) {
		if (user->plugin_02x100->word[i]==word[1][0]) {
			user->plugin_02x100->word_show[i]=user->plugin_02x100->word[i];
			++count;
			}
		if (user->plugin_02x100->word_show[i]=='-') ++blanks;
		}
	strcat(user->plugin_02x100->guess,word[1]);
	if (!count) {
		user->plugin_02x100->stage++;
		write_user(user, "That letter isn't in the word!  And you know what that means...\n");
		vwrite_user(user, hanged[user->plugin_02x100->stage],user->plugin_02x100->word_show,user->plugin_02x100->guess);
		if (user->plugin_02x100->stage>=7) {
			write_user(user,"~FR~OLUh-oh!~RS  You couldn't guess the word and died!\n");
			user->plugin_02x100->stage=-1;
			user->plugin_02x100->word[0]='\0';
			user->plugin_02x100->word_show[0]='\0';
			user->plugin_02x100->guess[0]='\0';
			user->plugin_02x100->lose++;
			plugin_02x100_save_user_details(user);
			}
		write_user(user,"\n");
		return;
		}
	if (count==1) vwrite_user(user, "Well done!  There was 1 occurrence of the letter %s\n",word[1]);
	else vwrite_user(user, "Well done!  There were %d occurrences of the letter %s\n",count,word[1]);
	vwrite_user(user, hanged[user->plugin_02x100->stage], user->plugin_02x100->word_show, user->plugin_02x100->guess);
	if (!blanks) {
		write_user(user,"~FY~OLCongratz!~RS  You guessed the word without dying!\n");
		user->plugin_02x100->win++;
		user->plugin_02x100->stage=-1;
		user->plugin_02x100->word[0]='\0';
		user->plugin_02x100->word_show[0]='\0';
		user->plugin_02x100->guess[0]='\0';
		plugin_02x100_save_user_details(user);
		}
}

/* Main Hangman Command Switch.. combines individual commands into one or
   more instead. */
void plugin_02x100_switch(UR_OBJECT user, char *str)
{
	int i,hcmd;
	char options[5][20]={"start","stop","ukaz","status", "*"};

	if (word_count<2) {
		write_user(user, "Pouzitie: hangman [start][stop][ukaz][status]<#>\n");
		write_user(user, "Priklady: hangman start    = Zacne hru hangman\n");
		write_user(user, "          hangman stop     = Vzda rozohratu hru\n");
		write_user(user, "          hangman ukaz     = Znovu ukaze rozohratu hru\n");
		write_user(user, "          hangman status   = Ukaze hernu statistiku\n");
		write_user(user, "          hangman <#>      = Vlozi zadane pismeno\n");
		return;
		}
	if (user->plugin_02x100==NULL) {
		if ((user->plugin_02x100=plugin_02x100_create_player())==NULL) {
			write_syslog(ERRLOG, 1, "chyba alokacie pamate v 02x100_switch()\n");
			write_user(user, "HANGMAN: vyskytla sa chyba pri alokacii pamate\n");
			return;
			}
		user->plugin_02x100->user=user;
		plugin_02x100_load_user_details(user);
		}

	hcmd=-1;
	for (i=0; options[i][0]!='*'; i++)
		if (!strcmp(options[i], word[1])) {
			hcmd=i;
			break;
			}
	switch (hcmd) {
		case  0: plugin_02x100_start(user); break;
		case  1: plugin_02x100_stop(user); break;
		case  2: plugin_02x100_show(user); break;
		case  3: plugin_02x100_status(user); break;
		default: plugin_02x100_vloz(user); break;
		}
}


void plugin_02x100_signon(UR_OBJECT user)
{
	user->plugin_02x100=NULL;
}


int plugin_02x100_load_user_details(UR_OBJECT user)
{
	FILE *fp;
	char filename[250];

	sprintf(filename, "%s/%s.02x100", USERPLDATAS, user->name);
	if (!(fp=fopen(filename, "r"))) {
		write_user(user, "HANGMAN: Chyba pri nahravani user dat\n");
		return 0;
		}
	fscanf(fp, "%d %d", &user->plugin_02x100->win, &user->plugin_02x100->lose);
	fclose(fp);
	return 1;
}

void plugin_02x100_leave(UR_OBJECT user)
{
	if (user->plugin_02x100!=NULL) {
		plugin_02x100_destruct_player(user);
		}
	else {
		write_user(user, "HANGMAN: Nemas hru.\n");
		}
}

void plugin_02x100_destruct_player(UR_OBJECT user)
{
	if (user->plugin_02x100==NULL) return;

	plugin_02x100_save_user_details(user);
	free(user->plugin_02x100);
	user->plugin_02x100=NULL;
}

int plugin_02x100_save_user_details(UR_OBJECT user)
{
	FILE *fp;
	char filename[250];
	
	sprintf(filename, "%s/%s.02x100", USERPLDATAS, user->name);
	if (!(fp=fopen(filename, "w"))) {
		write_user(user, "HANGMAN: chyba pri ukladani user dat\n");
		write_syslog(ERRLOG, 1, "Chyba pri ukladani dat v plugin_02x100_save_user_details()\n");
		return 0;
		}
	fprintf(fp, "%d %d", user->plugin_02x100->win, user->plugin_02x100->lose);
	fclose(fp);
	return 1;
}


char *plugin_02x100_get_word(char *aword)
{
	FILE *fp;
	char filename[250];
	int lines,cnt,i;

	lines=cnt=i=0;
	sprintf(filename,"%s/%s", PLFILES, HANGDICT);
	lines=count_lines(filename);
	srand(time(0));
	cnt=rand()%lines;
	if (!(fp=fopen(filename,"r"))) return(DEF_WORD);
	fscanf(fp,"%s\n",aword);
	while (!feof(fp)) {
		if (i==cnt) {
			fclose(fp);
			return aword;
			}
		++i;
		fscanf(fp,"%s\n",aword);
		}
	fclose(fp);
/* if no word was found, just return a generic word */
	return(DEF_WORD);
}

void plugin_02x100_start(UR_OBJECT user)
{
	int i;

	if (user->plugin_02x100->stage>-1) {
		write_user(user,"Ved uz mas rozohratu hru.\n");
		return;
		}
	plugin_02x100_get_word(user->plugin_02x100->word);
	strcpy(user->plugin_02x100->word_show, user->plugin_02x100->word);
	for (i=0; i<strlen(user->plugin_02x100->word_show); ++i) user->plugin_02x100->word_show[i]='-';
	user->plugin_02x100->stage=0;
	write_user(user,"Your current hangman game status is:\n\n");
	vwrite_user(user, hanged[user->plugin_02x100->stage], user->plugin_02x100->word_show, "None yet!");
}

void plugin_02x100_stop(UR_OBJECT user)
{
	if (user->plugin_02x100->stage==-1) {
		write_user(user,"nemas rozohrateho hangmana.\n");
		return;
		}
	user->plugin_02x100->stage=-1;
	user->plugin_02x100->word[0]='\0';
	user->plugin_02x100->word_show[0]='\0';
	user->plugin_02x100->guess[0]='\0';
	write_user(user,"Vzdal si hru hangman.\n");
}


void plugin_02x100_show(UR_OBJECT user)
{
	if (user->plugin_02x100->stage==-1) {
		write_user(user,"You haven't started a game of hangman yet.\n");
		return;
		}
	write_user(user,"Your current hangman game status is:\n");
	if (strlen(user->plugin_02x100->guess)<1) vwrite_user(user, hanged[user->plugin_02x100->stage], user->plugin_02x100->word_show, "None yet!");
	else vwrite_user(user, hanged[user->plugin_02x100->stage], user->plugin_02x100->word_show,user->plugin_02x100->guess);
	write_user(user,"\n");
}


void plugin_02x100_status(UR_OBJECT user)
{
	write_user(user, "~OL~FM---------------~FT[ ~FYTvoja Hangman statistika ~FT]~FM----------------\n\n");
	vwrite_user(user, "~FGvyhier ~FY%d ~FG, prehier ~FY%d\n\n", user->plugin_02x100->win, user->plugin_02x100->lose);
}


int plugin_02x100_reinit_save(UR_OBJECT user)
{
	FILE *fp;
	char fname[250];
	struct plugin_02x100_player *pl;

	if ((pl=user->plugin_02x100)==NULL) return 1;
	if (!plugin_02x100_save_user_details(user)) return 0;
	sprintf(fname, "%s/%s.02x100", TEMPFILES, user->name);
	if ((fp=fopen(fname, "w"))==NULL) return 0;
	fprintf(fp, "%d\n", pl->stage);
	fprintf(fp, "%s\n", pl->word);
	fprintf(fp, "%s\n", pl->word_show);
	fprintf(fp, "%s\n", pl->guess);
	fclose(fp);
	return 1;
}


int plugin_02x100_reinit_load(UR_OBJECT user)
{
	FILE *fp;
	char fname[250];
	struct plugin_02x100_player *pl;

	sprintf(fname, "%s/%s.02x100", TEMPFILES, user->name);
	if ((fp=fopen(fname, "r"))==NULL) return 0;
	user->plugin_02x100=plugin_02x100_create_player();
	if ((pl=user->plugin_02x100)==NULL) {
		fclose(fp);
		return 0;
		}
	pl->user=user;
	plugin_02x100_load_user_details(user);
	fscanf(fp, "%d\n", &pl->stage);
	fscanf(fp, "%s\n", pl->word);
	fscanf(fp, "%s\n", pl->word_show);
	fscanf(fp, "%s\n", pl->guess);
	fclose(fp);
	return 1;
}
