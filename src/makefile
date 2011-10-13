###############################################################################
#              Makefile subor na kompilaciu OS Star po castiach               #
#        Copyright (C) 2000 Pavol Hluchy - posledny update: 15.8.2000         #
#         osstar@star.sjf.stuba.sk | http://star.sjf.stuba.sk/osstar          #
################################################################################

#umiestnenie vysledneho spustitelneho suboru
BIN 			= ../bin
#nazov spustitelneho suboru
BINARY			= star
#adresar so skompilovanymi castami
OBJECT-DIR		= ../objs
#hlavny adresar
MAIN-DIR		= ..
#kompilacny program
CC      		= gcc
#optimalizacia, maximalne 2
CFLAGS  		= -O2
HDRS          		= 
#definicie
DEFS			=
#kniznice
LIBS    		= -lcrypt
#
# You'll need these libs for a Solaris build
#
#LIBS    		= -lmalloc -lsocket -lnsl



OBJS          = $(OBJECT-DIR)/star.o \
		$(OBJECT-DIR)/globals.o \
		$(OBJECT-DIR)/prompts.o \
		$(OBJECT-DIR)/plugin.o \
		$(OBJECT-DIR)/adds.o \
		$(OBJECT-DIR)/boots.o \
		$(OBJECT-DIR)/ct_general.o \
		$(OBJECT-DIR)/ct_social.o \
		$(OBJECT-DIR)/ct_msg.o \
		$(OBJECT-DIR)/ct_user.o \
		$(OBJECT-DIR)/ct_ignore.o \
		$(OBJECT-DIR)/ct_move.o \
		$(OBJECT-DIR)/ct_clone.o \
		$(OBJECT-DIR)/ct_admin.o \
		$(OBJECT-DIR)/mail.o \
		$(OBJECT-DIR)/email.o \
		$(OBJECT-DIR)/boards.o \
		$(OBJECT-DIR)/transport.o \
		$(OBJECT-DIR)/macros.o \
		$(OBJECT-DIR)/pueblo.o \
		$(OBJECT-DIR)/fonts.o \
		$(OBJECT-DIR)/poc_zoz.o \
		$(OBJECT-DIR)/who.o \
#		$(OBJECT-DIR)/netlinks.o
# predchadzajuci riadok odkomentovat pri pouziti NETLINKS


all:   			$(BIN)/$(BINARY)
		@echo 'Hotovo'

$(BIN)/$(BINARY):   	$(OBJS) makefile
		$(CC) $(CFLAGS) -o $(BIN)/$(BINARY) $(DEFS) $(OBJS) $(LIBS)
		@strip $(BIN)/$(BINARY)

$(OBJECT-DIR)/%.o:	prototypes.h define.h *_obj.h commands.h
		$(CC) $(CFLAGS) $(DEFS) -c $*.c -o $(OBJECT-DIR)/$*.o

clean:
		@echo 'Mazem binarky'
		@rm $(OBJECT-DIR)/*
		@echo zmazane

reall:
		@kompilujem vsetko od zaciatku
		@make clean
		@make all

###
$(OBJECT-DIR)/star.o: star.c star.h
$(OBJECT-DIR)/globals.o: globals.c commands.h
$(OBJECT-DIR)/prompts.o: prompts.c
$(OBJECT-DIR)/plugin.o: plugin.c plugin.h ./plugins/*
$(OBJECT-DIR)/adds.o: adds.c adds.h
$(OBJECT-DIR)/boots.o: boots.c boots.h
$(OBJECT-DIR)/ct_general.o: ct_general.c ct_general.h
$(OBJECT-DIR)/ct_social.o: ct_social.c ct_social.h
$(OBJECT-DIR)/ct_msg.o: ct_msg.c ct_msg.h
$(OBJECT-DIR)/ct_user.o: ct_user.c ct_user.h
$(OBJECT-DIR)/ct_ignore.o: ct_ignore.c ct_ignore.h
$(OBJECT-DIR)/ct_move.o: ct_move.c ct_move.h
$(OBJECT-DIR)/ct_clone.o: ct_clone.c ct_clone.h
$(OBJECT-DIR)/ct_admin.o: ct_admin.c ct_admin.h
$(OBJECT-DIR)/mail.o: mail.c mail.h
$(OBJECT-DIR)/email.o: email.c email.h
$(OBJECT-DIR)/boards.o: boards.c boards.h
$(OBJECT-DIR)/transport.o: transport.c transport.h
$(OBJECT-DIR)/macros.o: macros.c macros.h
$(OBJECT-DIR)/pueblo.o: pueblo.c pueblo.h music.h
$(OBJECT-DIR)/fonts.o: fonts.c fonts.h
$(OBJECT-DIR)/poc_zoz.o: poc_zoz.c poc_zoz.h
$(OBJECT-DIR)/who.o: who.c who.h
#$(OBJECT-DIR)/netlinks.o: netlinks.c netlinks.h
# predchadzajuci riadok odkomentovat pri pouziti NETLINKS
