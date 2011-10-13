/* vi: set ts=4 sw=4 ai: */
/*****************************************************************************
              Zakladne frazy, hlasky a retazce pre Lotos v1.2.0
            Copyright (C) Pavol Hluchy - posledny update: 23.4.2001
          lotos@losys.net           |          http://lotos.losys.net
 *****************************************************************************/
/***************************************************************************
   Please be CAREFUL when editing some of the prompts, especially the Style
   prompts because they MUST be in a specific order otherwise they will not
   work properly.  Especially mind the %s substitution variables as their
   order IS important!   And DO NOT change the prompt name!
                       original: Moenuts
 ***************************************************************************/

#ifndef __PROMPTS_C__
#define __PROMPTS_C__ 1

/*   Prompt Name:           Value:                                       */

/* Style Prompts ** BE CAREFUL WITH THESE ** They can be touchy!         */
char *say_style            ="~FT%s ~RS~FG%s~FT: ~RS%s\n";
char *help_levelname_style ="~OL[~FR%s ~FW- ~FG%d ~FW- ~FT%d~FW]\n    ";
char *all_clone_style      =" %-20.20s : %s\n";
char *site_style_dns_ip    ="%s is logged in from %s (%s) : %d.\n";
char *site_style_dns       ="%s is remotely connected from %s.\n";
char *site_style_offline   ="~OL%s~RS was last logged in from ~OL~FT%s~RS.\n";
char *wizshout_style       ="~OL~FW[~FMWiZ~FW]~RS~FT %s ~RS~OL~FG: %s\n";
char *wizshout_style_lev   ="~OL~FW[~FMWiZ~FW] ~FT%s>~RS~FT %s ~RS~OL~FG: %s\n";

/* Miscelaneous Prompt Configuration */
char *syserror               ="~OLPOZOR:~FR ** Vyskytla sa sytemova chyba **~RS\n";
char *nosuchroom             ="~OLCo si koki? Taku ruumu mas asi doma ...~RS\n";
char *nosuchuser             ="~OLToho si vymyslas, co ?~RS\n";
char *notloggedon            ="~OLTy jepka krepa, sag tu neni !~RS\n";
char *invisenter             ="~OL~FBdagdo dosjel ...~RS\n";
char *invisleave             ="~OL~FBFuha, dagdo odisjel ...~RS\n";
char *invisname              ="DaGdo";
char *noswearing             ="\nXces po xrapie ?~RS\n";
char *enterprompt            ="~FB-=+ ~RSstlac ~OL[~RS~FGENTER~OL~FW] ~RSpre nalogovanie ~FB+=-~RS";
char *session_swap           ="~OL~FB[ ~FTswap session ~FB]~FG:";
char *more_prompt            ="~BB~FG-=[~OL%d%%~RS~BB~FG]=- (~OLR~RS~BB~FG)-znovu, (~OLB~RS~BB~FG)-spat, (~OLE~RS~BB~FG)-koniec, <RETURN>-pokracuj:~RS ";
char *unknown_command        ="~FTNeznamy prikaz. Ak xces vytvorit novy napis na talker@losys.net~RS\n";
char *no_message_prompt      ="\nNa nastenke neni su spravy.\n";
char *single_message_prompt  ="\nNa nastenke je len jedna sprava.\n";
char *message_prompt         =" a pocet sprav na nastenke: ~OL~FM%d~RS.\n";
char *topic_prompt           ="~FG~OLAktualny topic:~RS %s\n";
char *notopic_prompt         ="~FGTopic este nebol nastaveny.\n";
char *no_exits               ="\n~FTNeni su tu vychody.~RS";
char *people_here_prompt     ="~FG~OLMozes tu videt:\n";
char *no_people_here_prompt  ="\n~FROkrem teba tu nikdo neni.\n";
char *already_in_room        ="~FTVed uz si v %s !\n";
char *help_header            ="~FT~OLPrikazy dostupne pre level ~RS~OL%s~RS";
char *help_footer1           ="\n~OL~FTSpolu ~FG%d ~FTprikazov v systeme. ~FTMas ~FG%d~FT prikazov dostupnych pre teba.\n";
char *help_footer2           ="~FGPre help k specifickemu prikazu napis:   ~OL.help <prikaz>~RS~FG.\n\n";
char *muzzle_victim_prompt   ="~FR~OLOdteraz mas zalepenu tlamu !\n";
char *muzzle_user_prompt     ="~FR~OL%s ma odteraz muzzle levelu: ~RS~OL%s.\n";
char *unmuzzle_victim_prompt ="~FG~OLTvoja tlama bola ocistena od lepidla !\n";
char *unmuzzle_user_prompt   ="~FG~OL%s moze osteraz znovu kecat.\n";
char *suicide_prompt         ="~OL~LI%s sa zasamovrazdil%s!\n";

/* Promotion Prompts */
char *promote_user_prompt ="~FTPromotujes %s~RS na level ~FM%s\n";
char *demote_user_prompt  ="~FRDemotujes %s~RS na level ~FM%s.\n";

/* Visibility Prompts */
char *appear_prompt        ="~FB~OLPocujes nejake divne brblanie a %s sa tu zjavuje !\n";
char *appear_user_prompt   ="~FB~OLNieco divneho si zabrbles a uz ta vsetci zase vidia.\n";
char *disapear_prompt      ="~OL~FB%s ti mizne pred ocami ...\n";
char *disapear_user_prompt ="~FB~OLNieco divneho si brbles a uz ta takmer nikto nevidi !\n";

/* Kill Prompts */
char *kill_user_chant  ="~OL~FMNieco otrasne si mrmles ...\n";
char *kill_room_chant  ="~OL~FM%s si mrmle nieco otrasne ...\n";

/* Clone Prompts */
char *clone_here_prompt   ="~FB~OLNieco si brbles popod nos a zrazu sa tu objavuje tvoj klon.\n";
char *clone_prompt        ="~FB~OLNieco si brbles popod nos a zrazu sa tvoj klon objavuje v ruume %s.\n";
char *clone_user_destroy  ="~FM~OLCosik mrmles a klon je zniceny.\n";
char *clone_room_destroy1 ="~FM~OL%s whispers a sharp spell...\n";
char *clone_room_destroy2 ="~FM~OLKlon menom %s sa rozplynul.\n";
char *clone_switch_prompt ="\n~FB~OLCosik si pomyslis ...\n";

/* Smail Prompts */
char *smail_edit_header ="~CS\n~FM-~OL=~FR[ ~FTPises mail pre ~FG%s ~FR]~FM=~RS~FM-\n\n";

/* Dmail Prompts */
char *dmail_nomail        ="~FRNemas mail na zmazanie.\n";
char *dmail_too_many      ="~OL~FRLen ~RS~OL%d ~FRsprav%s v tvojej schranke, vsetky teraz zmazane.\n";

/* Message Board Read Prompts */
char *message_board_header   ="\n~FM-~OL=~FR[ ~OL~FTNastenka v ruume ~FG%s~FT ~FR]~FM=~RS~FM-\n\n";
char *read_no_messages       ="~FTJak cumis, tak cumis, ale nastenka v %s je prazdna !\n\n";
char *user_read_board_prompt ="~FG%s~RS~FT cita nastenku.\n";

/* Message Board Write Prompts */
char *write_edit_header ="~CS\n~FM-~OL=~FR[ ~FGPises spravu na nastenku ~FR]~FM=~RS~FM-\n\n";
char *user_write_end    ="~OL~FMPricapujes spravu na nastenku ...\n";
char *room_write_end    ="~FM%s ~RS~FTpricapuje spravu na nastenku.\n";

/* Message Wipe Prompts */
char *wipe_empty_board       ="~OL~FRNastenka je prazdna.\n";
char *wipe_user_all_deleted  ="~OL~FRVsetky spravy zmazane\n";
char *wipe_room_all_deleted  ="~FG%s ~RS~FMmaze nastenku\n";
char *wipe_too_many          ="~OL~FRLen ~FM%d ~FRsprav%s na nastenke, odteraz vsetky zmazane !\n";
char *wipe_user_delete_range ="~OL%d ~FMsprav%s odtrhnut%s z nastenky.\n";

/* Profile Prompts */
char *profile_edit_header ="~CS\n~FG-~OL~FG=~FM]~FT Pises si vlastny profil ~FM[~FG=~RS~FG-\n\n";
char *no_profile_prompt   ="~FMTento user nema napisany profil\n\n";

/* Room Decsription Editor Prompts */
char *entroom_edit_header ="\n~CS~FG-~OL=~FM] ~FWPises desc - popis ruumy ~FM[~FG=~RS~FG-\n\n";

/* Room Entry Request Prompts */
char *user_knock_prompt      ="Trieskas na dvere do ruumy %s.\n";
char *room_knock_prompt      ="%s~RS sa sem dobyja.\n";
char *user_room_knock_prompt ="%s~RS sa dobyja do %s.\n";

/* Various Review Buffer Prompts */
char *cbuff_prompt           ="~FT%s maze zaznam kecov ...\n";
char *shout_cbuff_prompt     ="~FGMazes zaznam celotalkrovych skrekov\n";
char *no_review_prompt       ="~FTNevidim tu nic na zobrazenie.\n";
char *private_review_prompt  ="~OL~FRTa ruuma je privat, nemozes si pozret zaznam kecov.\n";
char *review_header          ="~FM-~OL=~FR[ ~FTZaznam kecov pre: ~FG%s ~FR]~FM=~RS~FM-\n";

/* Tell Review */
char *tell_review_header    ="\n~FM-~OL=~FR[ ~FTZaznam tellov ~FR]~FM=~RS~FM-\n";
char *no_tell_review_prompt ="~FMNemas telly na prezretie !\n";

/* Shout Review */
char *shout_review_header    ="~FM-~OL=~FR[ ~FTZaznam skrekov ~FR]~FM=~RS~FM-\n";
char *no_shout_review_prompt ="~OL~FMNeni su tu skreky na zobrazenie.\n";

/* Move Prompts */
char *move_prompt_user      ="~FT~OLMrmles si nejake stare kuzlo ...\n";
char *user_room_move_prompt ="~OL~FT%s si mrmle nejake stare kuzlo ...\n";

/*  Default Prompt Config */
char *default_inphr       ="je odteraz tu";
char *default_outphr      ="odchadza";

/* Login / Connection Prompts */
char *login_timeout      ="\n\n~OL~FRHmmm, smolka. Pridlho rozmyslas ...\nodpajam\n\n";
char *login_prompt       ="\n~FT~OLzadaj meno: ";
char *login_quit         ="\n~FY~OLHmmm, smolka. Je to tvoja volba\n~OL~FMAle o vela prixadzas...\n\n";
char *login_shortname    ="\n~OLPrikratke meno\n\n";
char *login_longname     ="\n~OLPridlhe meno\n\n";
char *login_swname       ="\nSorac, nemozes mat meno podobne tomuto !\n\n";
char *login_lettersonly  ="\n~OLV mene mozes mat len pismena\n\n";
char *login_pbloname     ="\n~OL~FR-=- Chybne meno. Skus stastie znova ...-=-~RS\n\n";
char *login_welcome      ="\n~OL~FYWitay v ~FT%s ~FM!\n";
char *login_attempts     ="\n~OL~FRSorry, uz ma to prestalo bavit ...\n";
char *user_banned_prompt ="\nSi vybanovany(a) z tohoto talkra\n\n";
char *login_rules_prompt ="\nZnovu napisanim hesla suhlasis s podmienkami (niekde si ix potom precitas).\n"
				"Ak nesuhlasis tak sa odpoj :)\n";
char *login_nonewatwiz   ="\nSorac, nove konta nemozu byt vytvorene na tomto porte\n\n";
char *login_nonewacc     ="\nSorac, momentalne nemozu byt vytvorene nove konta\n\n";
char *login_nonewatbanned="\nSorac, nove konta nemozu byt momentalne vytvorene z tvojej sajty\n\n";
char *login_new_user     =".oO Vytvaram noveho zawislaka Oo.\n";
char *login_minlev       ="\nSorac, talker je prave zavrety pre zawislakof tvojho levelu\n\n";
char *login_minwizlev    ="\nSorac, len zawislaci s levelom %s a vyssim sa mozu zavesit na tento port\n\n";

/* Password Prompts */
char *password_prompt  ="\n\n~FT~OLzadaj heslo:~RS ";
char *password_again   ="\n\n~FT~OLpotvrd heslo:~RS ";
char *password_wrong   ="\n\n~FRKua nevies sa poriadne zavesit ?~RS\n\n";
char *password_nomatch ="\n\n~FRKua nevies napisat dva krat to iste ?~RS\n\n";
char *password_short   ="\n\n~FRPrikratke hesielko !~RS\n\n";
char *password_long    ="\n\n~FRPridlhe heslisko !~RS\n\n";
char *password_bad     ="Chybne heslo\n";

/* added */
char *default_desc     ="este nepouzilo ~FR.desc";
char *edit_markers     ="[---------------- Prosim snaz sa zmestit medzi tieto dve znacky ---------------]\n\n~FG%d>~RS";
char *edit_prompt      ="\n~FG~OLU~RS~FGloz~RS, ~FTu~FT~OLK~RS~FTaz~RS, ~FY~OLP~RS~FYrerob~RS, ~FR~OLZ~RS~FRrus~RS: ";
char *eq_hi_lev_prompt ="Nemozes %s usera s rovnakym alebo vyssim levelom ako mas ty.\n";
char *this_m_ban_prompt="Nemozes zabanovat masinu na ktorej je spusteny tento talker.\n";
char *autopromo_prompt ="\n%s~OL~FY****************************************************************************\n"
			"~OL~FY*               ~FR   PODMIENKY   NA   PRVE     AUTO-PROMOTE~FY                  *\n"
			"~OL~FY*           Nastaveny (.desc), nastaveny gender-pohlavie (.set gnd)        *\n"
			"~OL~FY*           a nastaveny a overeny e-mail pomocou .accreq & .verify         *\n"
			"~OL~FY****************************************************************************\n\n";
char *flood_prompt     ="\nYou have attempted to flood this talker's ports.  We view this as\n\r"
			"a sign that you are trying to hack this system.  Therefor you have now\n\r"
			"had logins from your site/domain banned.\n\n";
char *flood_prompt_r   ="\n\rYou have attempted to flood this talker's ports.  We view this as\n\r"
			"a sign that you are trying to hack this system.  Therefor you have now\n\r"
			"had logins from your site/domain banned.\n\n\r";
char *sys_port_closed  ="Sorrac, hlavny port je momentalne uzavrety pre dalsie loginy~RS\n";
char *wiz_port_closed  ="Sorrac, wizardovsky port je momentalne uzavrety pre dalsie loginy~RS\n";
char *continue1        ="~CB-=[ ~CRStlac enter na pokracovanie. ~CB]=-\n";
char *continue2        ="~CB-=[ ~CW[~CYENTER~CW] = ~CYpokracovat, ~CW[~CYE~CW] + [~CYENTER~CW] = ~CYKonec ~CB]=-\n";
char *auto_afk_mesg    ="~OL[~FYAUTO~FW-~FRAFK~FW]";
char *clone_desc       ="~BR~OL(KLON)";
char *no_wizs_logged   ="Sorac, momentalne tu neni ziadny wizard.\n";
char *empty_log        ="Tento log je prazdny.\n\n";
char *nosuchtr         ="taky transport neni\n";
char *default_personal_room_desc="The walls are stark and the floors are bare. Either the owneris new\n"
			"or just plain lazy. Perhaps they just don't know how to use the .mypain\n"
			"command yet ?\n";
char *default_personal_room_topic="Witay v mojej roome !";
char *restart_prompt   ="~OL~FR~LIRestartujem, chvilu nic nerob ... \n";
char *restart_ok       ="~OL~FGhotovo, cest zawislosti !!!\n";
char *use_menu_prompt  ="~CGXces pouzit menu ? ~FW<~FRano~FW/~FRnie~FW>~FB :~FR";
char *prooms_disabled  ="Osobne ruumky momentalne zakazane.\n";

/* nastavovanie */
char *room_setup_enter ="%s~RS ~FRsi zacina nieco nastavovat ...\n";
char *room_leave_setup ="%s~RS ~FGkonci svoje nastavovanie ...\n";
char *user_bch_setup   ="~CRTak volba neexistuje ...\n";

/* netriedene */
char *muzzled_cannot   ="~FRMas zalepenu tlamu, nemozes %s !\n";

#endif /* prompts.c */
