/* OS Star plugin                      pre verziu 1.0.0b a vyssiu
   -------------------------------------------------------------
   Original: MoeNUTS talker
   Modifikoval na plug-in pre OS Star: Lopo
   
   inicializacny riadok pre tento plugin:
   	if (tmp=plugin_02x100_init(cmd)) cmd=cmd+tmp;
   je v subore adds.c
   
   Volaci prikazu pre tento plugin:
   	if (!strcmp(plugin->registration,"02-100")) { plugin_02x100_main(user,str,comnum); return 1; }
   je v subore plugin.c
   	
   Nasledujuci riadok MUSI byt vlozeny na koniec struktury
   v subore ur_obj.h:
   	struct plugin_02x100_tic_player *plugin_02x100_tictac;
   ------------------------------------------------------------- */
 
 #include "hangman.h"
 
	extern CM_OBJECT create_cmd();
	extern PL_OBJECT create_plugin();
	void plugin_02x100_vloz_hgame(UR_OBJECT user);
	struct plugin_02x100_hang_player *plugin_02x100_create_hang_player(void);

/* ------------------------------------------------------------- */

plugin_02x100_init(int cm)
{
PL_OBJECT plugin;
CM_OBJECT com;
int i,verFail;
i=0; verFail=0;
/* create plugin */
if ((plugin=create_plugin())==NULL) {
        write_syslog(ERRLOG, 0, "Nemozem vytvorit novu polozku v registroch pre plugin 'hangman'!\n");
        return 0;
	}
strcpy(plugin->name,"Hangman");                 /* Plugin Description   */
strcpy(plugin->author,"Lopo");                  /* Author's name        */
strcpy(plugin->registration,"02-100");          /* Plugin/Author ID     */
strcpy(plugin->ver,"1.1");                      /* Plugin version       */
strcpy(plugin->req_ver,"100");                  /* OSS version required */
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

plugin_02x100_main(user,str,comid)
UR_OBJECT user;
char *str;
int comid;
{
switch (comid) {
        /* put case calls here for each command needed.
           remember that the commands will be in order, with the first
           command always being 1;
        */
        case -8: if (user->plugin_02x100_hangman!=NULL) plugin_02x100_leave_hang(user);
                 return 1;
        case -9: plugin_02x100_signon(user); return 1;
        case  1: plugin_02x100_switch(user,str); return 1;
        default: return 0;
        }
}

/* Main Hangman Command Switch.. combines individual commands into one or
   more instead. */
plugin_02x100_switch(user,str)
UR_OBJECT user;
char *str;
{
int i,hcmd;
char options[5][20]={"start","stop","ukaz","vloz","status"};

if (user->plugin_02x100_hangman==NULL) {
	if ((user->plugin_02x100_hangman=plugin_02x100_create_hang_player())==NULL) {
		write_syslog(ERRLOG, 1, "chyba alokacie pamate v plugin_02x100_switch()\n");
		write_user(user, "HANGMAN: vyskytla sa chyba pri alokacii pamate\n");
		return;
		}
	user->plugin_02x100_hangman->user=user;
	plugin_02x100_load_user_details(user);
	}

hcmd=-1;
for (i=0; i<5; i++) {
	if (!strncmp(options[i],word[1], strlen(word[1]))) {
		hcmd=i;
		break;
		}
	}

if (hcmd==-1 || word_count<2) {
	write_user(user,"Pouzitie: hangman [start][stop][ukaz][vloz <#>]\n");
	write_user(user,"Priklady: hangman start    = Zacne hru hangman\n");
	write_user(user,"          hangman stop     = Vzda rozohratu hru\n");
	write_user(user,"          hangman ukaz     = Znovu ukaze rozohratu hru\n");
	write_user(user,"          hangman vloz <#> = Vlozi pismeno '#'\n");
	return;
        return;
        }
switch (hcmd) {
        case  0: plugin_02x100_start_hgame(user); break;
        case  1: plugin_02x100_stop_hgame(user); break;
        case  2: plugin_02x100_show_hgame(user); break;
        case  3: plugin_02x100_vloz_hgame(user); break;
        case  4: plugin_02x100_status_hgame(user); break;
        default: write_user(user,"Error.  Hangman command not executed.\n");
        }
}

plugin_02x100_signon(user)
UR_OBJECT user;
{
	user->plugin_02x100_hangman=NULL;
}

/*** Create a hang player ***/
struct plugin_02x100_hang_player *plugin_02x100_create_hang_player()
{
struct plugin_02x100_hang_player *player;
int i;

if ((player=(struct plugin_02x100_hang_player *)malloc(sizeof(struct plugin_02x100_hang_player)))==NULL) {
        write_syslog(ERRLOG, 1, "chyba alokacie pamate v create_02x100_hang_player().\n");
        return NULL;
        }


/* initialise the player */
player->user = NULL;
player->hwin=player->hlose=0;
player->hstage=-1;

return player;
}

plugin_02x100_load_user_details(UR_OBJECT user)
{
	FILE *fp;
	char filename[120];

	user->plugin_02x100_hangman->hwin=user->plugin_02x100_hangman->hlose=0;
	sprintf(filename, "%s/%s/%s/%s.02x100", ROOTDIR, USERFILES, USERPLDATAS, user->name);
	if (!(fp=fopen(filename, "r"))) {
		write_user(user, "HANGMAN: Chyba pri nahravani user dat\n");
		return 0;
		}
	fscanf(fp, "%d %d", &user->plugin_02x100_hangman->hwin, &user->plugin_02x100_hangman->hlose);
	fclose(fp);
}

plugin_02x100_leave_hang(UR_OBJECT user)
{
	if (user->plugin_02x100_hangman!=NULL) {
		plugin_02x100_destruct_hang_player(user);
		}
	else {
		write_user(user, "HANGMAN: Nemas hru.\n");
		}
}

plugin_02x100_destruct_hang_player(UR_OBJECT user)
{
	plugin_02x100_save_user_details(user);
	free(user->plugin_02x100_hangman);
	user->plugin_02x100_hangman=NULL;
}

plugin_02x100_save_user_details(UR_OBJECT user)
{
	FILE *fp;
	char filename[120];
	
	sprintf(filename, "%s/%s/%s/%s.02x100", ROOTDIR, USERFILES, USERPLDATAS, user->name);
	if (!(fp=fopen(filename, "w"))) {
		write_user(user, "HANGMAN: chyba pri ukladani user dat\n");
		write_syslog(ERRLOG, 1, "Chyba pri ukladani dat v plugin_02x100_save_user_details()\n");
		return;
		}
	fprintf(fp, "%d %d", user->plugin_02x100_hangman->hwin, user->plugin_02x100_hangman->hlose);
	fclose(fp);
}


char *get_hang_word(char *aword)
{
char filename[120];
FILE *fp;
int lines,cnt,i;

lines=cnt=i=0;
sprintf(filename,"%s/%s/%s/%s", ROOTDIR, DATAFILES, PLFILES, HANGDICT);
lines=count_lines(filename);
srand(time(0));
cnt=rand()%lines;
if (!(fp=fopen(filename,"r"))) return("hangman");
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
return("hangman");
}

plugin_02x100_start_hgame(UR_OBJECT user)
{
int i;

  if (user->plugin_02x100_hangman->hstage>-1) {
    write_user(user,"Ved uz mas rozohratu hru.\n");
    return;
    }
  get_hang_word(user->plugin_02x100_hangman->hword);
  strcpy(user->plugin_02x100_hangman->hword_show,user->plugin_02x100_hangman->hword);
  for (i=0;i<strlen(user->plugin_02x100_hangman->hword_show);++i) user->plugin_02x100_hangman->hword_show[i]='-';
  user->plugin_02x100_hangman->hstage=0;
  write_user(user,"Your current hangman game status is:\n\n");
  sprintf(text,hanged[user->plugin_02x100_hangman->hstage],user->plugin_02x100_hangman->hword_show,"None yet!");
  write_user(user,text);
  return;
}

plugin_02x100_stop_hgame(UR_OBJECT user)
{
	if (user->plugin_02x100_hangman->hstage==-1) {
		write_user(user,"nemas rozohrateho hangmana.\n");
		return;
		}
	user->plugin_02x100_hangman->hstage=-1;
	user->plugin_02x100_hangman->hword[0]='\0';
	user->plugin_02x100_hangman->hword_show[0]='\0';
	user->plugin_02x100_hangman->hguess[0]='\0';
	write_user(user,"Vzdal si hru hangmana.\n");
	return;
}

plugin_02x100_show_hgame(UR_OBJECT user)
{
	if (user->plugin_02x100_hangman->hstage==-1) {
		write_user(user,"You haven't started a game of hangman yet.\n");
		return;
		}
	write_user(user,"Your current hangman game status is:\n");
	if (strlen(user->plugin_02x100_hangman->hguess)<1) sprintf(text,hanged[user->plugin_02x100_hangman->hstage],user->plugin_02x100_hangman->hword_show,"None yet!");
	else sprintf(text,hanged[user->plugin_02x100_hangman->hstage],user->plugin_02x100_hangman->hword_show,user->plugin_02x100_hangman->hguess);
	write_user(user,text);
	write_user(user,"\n");
	return;
}

plugin_02x100_status_hgame(UR_OBJECT user)
{
	write_user(user, "~OL~FM---------------~FT[ ~FYTvoja Hangman statistika ~FT]~FM----------------\n\n");
	sprintf(text,"~FGvyhier ~FY%d ~FG, prehier ~FY%d\n\n",user->plugin_02x100_hangman->hwin,user->plugin_02x100_hangman->hlose);
	write_user(user,text);
	return;
}

void plugin_02x100_vloz_hgame(UR_OBJECT user)
{
int count,i,blanks;

count=blanks=i=0;
if (strlen(word[2])>1) {
  write_user(user,"Naraz mozes vlozit len jedno pismeno !\n");
  return;
  }
if (user->plugin_02x100_hangman->hstage==-1) {
  write_user(user,"Nemas zacatu hru.\n");
  return;
  }
strtolower(word[2]);
if (strstr(user->plugin_02x100_hangman->hguess,word[2])) {
  user->plugin_02x100_hangman->hstage++;
  write_user(user,"You have already guessed that letter!  And you know what that means...\n\n");
  sprintf(text,hanged[user->plugin_02x100_hangman->hstage],user->plugin_02x100_hangman->hword_show,user->plugin_02x100_hangman->hguess);
  write_user(user,text);
  if (user->plugin_02x100_hangman->hstage>=7) {
    write_user(user,"~FR~OLUh-oh!~RS  You couldn't guess the word and died!\n");
    user->plugin_02x100_hangman->hstage=-1;
    user->plugin_02x100_hangman->hword[0]='\0';
    user->plugin_02x100_hangman->hword_show[0]='\0';
    user->plugin_02x100_hangman->hguess[0]='\0';
    user->plugin_02x100_hangman->hlose+=1;
    plugin_02x100_save_user_details(user);
    }
  write_user(user,"\n");
  return;
  }
for (i=0;i<strlen(user->plugin_02x100_hangman->hword);++i) {
  if (user->plugin_02x100_hangman->hword[i]==word[2][0]) {
    user->plugin_02x100_hangman->hword_show[i]=user->plugin_02x100_hangman->hword[i];
    ++count;
    }
  if (user->plugin_02x100_hangman->hword_show[i]=='-') ++blanks;
  }
strcat(user->plugin_02x100_hangman->hguess,word[2]);
if (!count) {
  user->plugin_02x100_hangman->hstage++;
  write_user(user,"That letter isn't in the word!  And you know what that means...\n");
  sprintf(text,hanged[user->plugin_02x100_hangman->hstage],user->plugin_02x100_hangman->hword_show,user->plugin_02x100_hangman->hguess);
  write_user(user,text);
  if (user->plugin_02x100_hangman->hstage>=7) {
    write_user(user,"~FR~OLUh-oh!~RS  You couldn't guess the word and died!\n");
    user->plugin_02x100_hangman->hstage=-1;
    user->plugin_02x100_hangman->hword[0]='\0';
    user->plugin_02x100_hangman->hword_show[0]='\0';
    user->plugin_02x100_hangman->hguess[0]='\0';
    user->plugin_02x100_hangman->hlose+=1;
    plugin_02x100_save_user_details(user);
    }
  write_user(user,"\n");
  return;
  }
if (count==1) sprintf(text,"Well done!  There was 1 occurrence of the letter %s\n",word[2]);
else sprintf(text,"Well done!  There were %d occurrences of the letter %s\n",count,word[2]);
write_user(user,text);
sprintf(text,hanged[user->plugin_02x100_hangman->hstage],user->plugin_02x100_hangman->hword_show,user->plugin_02x100_hangman->hguess);
write_user(user,text);
if (!blanks) {
  write_user(user,"~FY~OLCongratz!~RS  You guessed the word without dying!\n");
  user->plugin_02x100_hangman->hwin+=1;
  user->plugin_02x100_hangman->hstage=-1;
  user->plugin_02x100_hangman->hword[0]='\0';
  user->plugin_02x100_hangman->hword_show[0]='\0';
  user->plugin_02x100_hangman->hguess[0]='\0';
  plugin_02x100_save_user_details(user);
  }
}

