################################################################################
#               Makefile subor na kompilaciu Lotos po castiach                 #
#        Copyright (C) 2001 Pavol Hluchy - posledny update: 23.4.2001          #
#                    lotos@losys.net | http://lotos.losys.net                  #
################################################################################

VERSION			= 1.2.0
MAJORVER		= 1
MINORVER		= 2
PATCHVER		= 0

#ak sa pouziju netlinky, treba odkomentovat netlinks.o
USE_NETLINKS		= false
DEBUG			= true

#umiestnenie vysledneho spustitelneho suboru
BIN 			= ../bin
#nazov spustitelneho suboru
BINARY			= star
TARGET			= $(BIN)/$(BINARY)
#adresar so skompilovanymi castami
OBJECT-DIR		= ../objs
MAIN-DIR		= ..
SRC-DIR			= .
#programy
CC      		= gcc
SED			= sed
#
DISTFILE		= lotos_$(MAJORVER)$(MINORVER)$(PATCHVER).tar.gz
DEPFILE			= .depend

ifeq ($(DEBUG), true)
	CFLAGS  	= -g -mcpu=i586 -Wall
else
	CFLAGS		= -O0 -mcpu=i586 -Wall
endif
HDRS          		=
#definicie
ifeq ($(DEBUG), true)
	DEFS		= -DDEBUG
else
	DEFS		= 
endif
ifeq ($(USE_NETLINKS), true)
	DEFS		+= -DNETLINKS
endif
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
		$(OBJECT-DIR)/s_string.o \
		$(OBJECT-DIR)/s_events.o \
		$(OBJECT-DIR)/s_net.o \
		$(OBJECT-DIR)/money.o \
		$(OBJECT-DIR)/menus.o \
		$(OBJECT-DIR)/restart.o \
		$(OBJECT-DIR)/build.o \
		$(OBJECT-DIR)/netlinks.o


all:   			$(TARGET)
		@echo -e "\a\033[1;32mHotovo\033[0m\n"

$(TARGET):		$(OBJS) Makefile
		$(CC) $(CFLAGS) $(DEFS) $(HDRS) -c build.c -o $(OBJECT-DIR)/build.o
		$(CC) $(CFLAGS) -o $(TARGET) $(DEFS) $(NETLINKS) $(OBJS) $(LIBS) $(HDRS)
		@strip $(TARGET)
		@chmod 700 $(TARGET)

$(DEPFILE):
		@echo -e "\033[1;32mMaking $(DEPFILE)\033[0m\n"
		@touch $(DEPFILE)
		@$(CC) -MM *.c -MM ./plugins/*.c > .tmdepend
		@$(SED) -e '1,$$s/^[a-z]/$$(OBJECT-DIR)\/&/' .tmdepend > $(DEPFILE)
		@rm -vf .tmdepend

$(OBJECT-DIR)/%.o:	prototypes.h define.h obj_*.h commands.h
		$(CC) $(CFLAGS) $(DEFS) $(NETLINKS) $(HDRS) -c $*.c -o $(OBJECT-DIR)/$*.o

clean:
		@echo -e "\033[1;31mMazem binarky\033[0m\n"
		@rm -f $(OBJECT-DIR)/*
		@rm -f $(TARGET)
		@rm -f tags

distclean: clean
		@echo -e "\033[1;35mMaking distclean\033[0m\n"
		@rm -f $(MAIN-DIR)/backups/*
		@rm -f $(BIN)/*
		@rm -f $(MAIN-DIR)/dumpfiles/*
		@rm -f $(MAIN-DIR)/logfiles/*
		@rm -f $(OBJECT-DIR)/*
		@rm -f $(MAIN-DIR)/tempfiles/*
		@rm -f $(SRC-DIR)/$(DEPFILE)

reall:
		@echo "kompilujem vsetko od zaciatku\n"
		@make clean
		@make dep
		@make ctags
		@make all

ctags:
		@echo -e "\033[1mMaking tags file\033[0m (useful for vim)\033[0m\n"
		@ctags --sort=yes --lang=c *.[h,c] ./plugins/*.[h,c]
		@echo -e "Done ...\n"

dep:
		@make $(DEPFILE)

dist:
		@echo -e "\033[1;32mMaking distribution\033[0m\n"
		@make distclean
		@( cd ../.. ;\
			echo -e "\033[1;37mcompressing\n\033[0m\n" ;\
			tar zcfp $(DISTFILE) lotos120 ;\
			chmod 600 $(DISTFILE) ;\
			ls -la $(DISTFILE))


###
include $(DEPFILE)
#Old style dependencies if the automatic ones don't work for you.
#$(OBJECT-DIR)/star.o: star.c star.h
#$(OBJECT-DIR)/globals.o: globals.c commands.h
#$(OBJECT-DIR)/prompts.o: prompts.c
#$(OBJECT-DIR)/plugin.o: plugin.c plugin.h ./plugins/*
#$(OBJECT-DIR)/adds.o: adds.c adds.h
#$(OBJECT-DIR)/boots.o: boots.c boots.h
#$(OBJECT-DIR)/ct_general.o: ct_general.c ct_general.h
#$(OBJECT-DIR)/ct_social.o: ct_social.c ct_social.h
#$(OBJECT-DIR)/ct_msg.o: ct_msg.c ct_msg.h
#$(OBJECT-DIR)/ct_user.o: ct_user.c ct_user.h
#$(OBJECT-DIR)/ct_ignore.o: ct_ignore.c ct_ignore.h
#$(OBJECT-DIR)/ct_move.o: ct_move.c ct_move.h
#$(OBJECT-DIR)/ct_clone.o: ct_clone.c ct_clone.h
#$(OBJECT-DIR)/ct_admin.o: ct_admin.c ct_admin.h
#$(OBJECT-DIR)/mail.o: mail.c mail.h
#$(OBJECT-DIR)/email.o: email.c email.h
#$(OBJECT-DIR)/boards.o: boards.c boards.h
#$(OBJECT-DIR)/transport.o: transport.c transport.h
#$(OBJECT-DIR)/macros.o: macros.c macros.h
#$(OBJECT-DIR)/pueblo.o: pueblo.c pueblo.h music.h
#$(OBJECT-DIR)/fonts.o: fonts.c fonts.h
#$(OBJECT-DIR)/poc_zoz.o: poc_zoz.c poc_zoz.h
#$(OBJECT-DIR)/who.o: who.c who.h
#$(OBJECT-DIR)/s_string.o: s_string.c s_string.h
#$(OBJECT-DIR)/s_events.o: s_events.c s_events.h
#$(OBJECT-DIR)/s_net.o: s_net.c s_net.h
#$(OBJECT-DIR)/money.o: money.c money.h
#$(OBJECT-DIR)/menus.o: menus.c menus.h
#$(OBJECT-DIR)/restart.o: restart.c restart.h
#$(OBJECT-DIR)/build.o: build.c build.h
#$(OBJECT-DIR)/netlinks.o: netlinks.c netlinks.h

