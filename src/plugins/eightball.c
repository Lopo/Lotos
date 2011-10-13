/* OS Star plugin                      pre verziu 1.1.0 a vyssiu
   -------------------------------------------------------------
   Inicializacny riadok pre tento plugin:
   	if (tmp=plugin_00x000_init(cmd)) cmd=cmd+tmp;
   je v subore adds.c

   Volanie prikazu pre tento plugin:
   	if (!strcmp(plugin->registration, "00-000")) {plugin_00x000_main(user,str,comnum); return 1; }
   je v subore plugin.c
   ------------------------------------------------------------- */

extern CM_OBJECT create_cmd(void);
void plugin_00x000_respond(UR_OBJECT user, char *str);


int plugin_00x000_init(int cm)
{
	PL_OBJECT plugin;
	CM_OBJECT com;
	int i=0;

/* create plugin */
	if ((plugin=create_plugin())==NULL) {
		write_syslog(SYSLOG, 0, "ERROR: Unable to create new registry entry!\n");
		return 0;
		}
	strcpy(plugin->name,"TalkerMagicEightBall");    /* Plugin Description   */
	strcpy(plugin->author,"orig. William Price");   /* Author's name        */
	/* re-write Lopo */
	strcpy(plugin->registration,"00-000");          /* Plugin/Author ID     */
	strcpy(plugin->ver,"1.1");                      /* Plugin version       */
	strcpy(plugin->req_ver,"110");                  /* Runtime ver required */
	plugin->id = cm;                                /* ID used as reference */
	plugin->req_userfile = 0;                       /* Requires user data?  */
	plugin->triggerable = 0;                        /* Can be triggered by
	                                                   regular speech? */

/* create associated command */
	if ((com=create_cmd())==NULL) {
		write_syslog(SYSLOG, 0, "ERROR: Unable to add command to registry!\n");
		return 0;
		}
	i++;                                            /* Keep track of number created */
	strcpy(com->command,"8ball");                   /* Name of command */
	com->id = plugin->id;                           /* Command reference ID */
	com->req_lev = USER;                            /* Required level for cmd. */
	com->comnum = i;                                /* Per-plugin command ID */
	com->plugin = plugin;                           /* Link to parent plugin */
/* end creating command - repeat as needed for more commands */
	return i;
}


int plugin_00x000_main(UR_OBJECT user, char *str, int comid)
{
        /* put case calls here for each command needed.
           remember that the commands will be in order, with the first
           command always being 1
        */
	switch (comid) {
		case  1: plugin_00x000_respond(user,str); return 1;
		case -5: /* Trigger Heartbeat */
		case -6: /* Talker shutdown */
		case -7: /* 1st to save */
		case -8: /* save normal */
		case -9: /* load user data */
		default: return 0;
		}
}


void plugin_00x000_respond(UR_OBJECT user, char *str)
{
	char filename[280],line[150],*name;
	int i,num,total;
	FILE *fp;

	i=0;  num=0;  total=0;
	filename[0]='\0';  line[0]='\0';

	if (word_count<2) {
		write_user(user,"The Magic EightBall will only respond to a question.\n");
		return;
		}
	if (user->vis) name=user->name;
	else name=invisname;

        sprintf(filename,"%s/eightball.8", PLFILES);
        if (!(fp=fopen(filename,"r"))) {
                write_user(user,"EightBall:  Sorry!  Response file was not found.\n");
                return;
                }
        fscanf(fp,"%d\n",&total); /* total file entries */
        num = rand() % total;
        num++;  /* Have to increase because the rand() might leave it 0. */

        /* get the line from the file that contains the random response */
        for(i=0; (i<num && !feof(fp)); i++) fgets(line,161,fp);
        fclose(fp);
        line[strlen(line)-1]='\0';
        terminate(line);

        /* write question to users */
        vwrite_user(user,"%s[You ask the EightBall]%s: %s\n",colors[CSELF],colors[CTEXT],str);
        vwrite_room_except(user->room,user,"%s[%s asks the EightBall]%s: %s\n",colors[CUSER],name,colors[CTEXT],str);
        record(user->room,text);

        /* write response to room */
        vwrite_room(user->room,"~FG[The Magic EightBall]%s: %s\n",colors[CTEXT],line);
        record(user->room,text);
}
