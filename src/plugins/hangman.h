/* vi: set ts=4 sw=4: */
/* Lotos         Hangman Plugin - hlavickovy fajl
  -----------------------------------------------*/

#ifndef __PL02x100_H__
#define __PL02x100_H__ 1

#define PL02x100_DICT "hangdata"
#define PL02x100_DEFWORD "hangman"

int pl02x100_init(int cm);
int pl02x100_main(UR_OBJECT user, char *str, int comid);
struct pl02x100_player *pl02x100_create_player(void);
void pl02x100_vloz(UR_OBJECT user);
void pl02x100_switch(UR_OBJECT user, char *str);
void pl02x100_signon(UR_OBJECT user);
int pl02x100_load_user_details(UR_OBJECT user);
void pl02x100_leave(UR_OBJECT user);
void pl02x100_destruct_player(UR_OBJECT user);
int pl02x100_save_user_details(UR_OBJECT user);
char *pl02x100_get_word(char *aword);
void pl02x100_start(UR_OBJECT user);
void pl02x100_stop(UR_OBJECT user);
void pl02x100_show(UR_OBJECT user);
void pl02x100_status(UR_OBJECT user);
int pl02x100_reinit_save(UR_OBJECT user);
int pl02x100_reinit_load(UR_OBJECT user);


struct pl02x100_player {
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

#endif /* __PL02x100_H__ */
