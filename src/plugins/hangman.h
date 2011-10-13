/* OS Star         Hangman Plugin - hlavickovy fajl
  -----------------------------------------------*/

#define HANGDICT "hangdata"
#define DEF_WORD "hangman"

int plugin_02x100_init(int cm);
int plugin_02x100_main(UR_OBJECT user, char *str, int comid);
struct plugin_02x100_player *plugin_02x100_create_player(void);
void plugin_02x100_vloz(UR_OBJECT user);
void plugin_02x100_switch(UR_OBJECT user, char *str);
void plugin_02x100_signon(UR_OBJECT user);
int plugin_02x100_load_user_details(UR_OBJECT user);
void plugin_02x100_leave(UR_OBJECT user);
void plugin_02x100_destruct_player(UR_OBJECT user);
int plugin_02x100_save_user_details(UR_OBJECT user);
char *plugin_02x100_get_word(char *aword);
void plugin_02x100_start(UR_OBJECT user);
void plugin_02x100_stop(UR_OBJECT user);
void plugin_02x100_show(UR_OBJECT user);
void plugin_02x100_status(UR_OBJECT user);
int plugin_02x100_reinit_save(UR_OBJECT user);
int plugin_02x100_reinit_load(UR_OBJECT user);


struct plugin_02x100_player {
	struct user_struct *user;
	int win, lose;
	int stage;
	char word[WORD_LEN+1], word_show[WORD_LEN+1], guess[WORD_LEN+1];
	};

/* hangman picture for the hangman game */
char *hanged[8]={
	"~FY~OL+~RS~FY---~OL+  \n~FY|      \n~FY|~RS           ~OLWord:~RS %s\n~FY|~RS           ~OLLetters guessed:~RS %s\n~FY|~RS      \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS           ~OLWord:~RS %s\n~FY|~RS           ~OLLetters guessed:~RS %s\n~FY|~RS      \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS   O       ~OLWord:~RS %s\n~FY|~RS           ~OLLetters guessed:~RS %s\n~FY|~RS      \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS   O       ~OLWord:~RS %s\n~FY|~RS   ~FG#~RS       ~OLLetters guessed:~RS %s\n~FY|~RS      \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS   O       ~OLWord:~RS %s\n~FY|~RS  /~FG#~RS       ~OLLetters guessed:~RS %s\n~FY|~RS      \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS   O       ~OLWord:~RS %s\n~FY|~RS  /~FG#~RS\\      ~OLLetters guessed:~RS %s\n~FY|~RS      \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS   O       ~OLWord:~RS %s\n~FY|~RS  /~FG#~RS\\      ~OLLetters guessed:~RS %s\n~FY|~RS  /   \n~FY|______\n",
	"~FY~OL+~RS~FY---~OL+  \n~FY|   |  \n~FY|~RS   O       ~OLWord:~RS %s\n~FY|~RS  /~FG#~RS\\      ~OLLetters guessed:~RS %s\n~FY|~RS  / \\ \n~FY|______\n"
  };
