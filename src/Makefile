#*
#*   Lotos v1.2.1  : (c) 1999-2001 Pavol Hluchy (Lopo)
#*   last update   : 26.12.2001
#*   email         : lopo@losys.sk
#*   homepage      : lopo.losys.sk
#*   Lotos homepage: lotos.losys.sk
#*

#ak sa pouziju netlinky, treba odkomentovat netlinks.o
USE_NETLINKS		= n
DEBUG			= n

#umiestnenie vysledneho spustitelneho suboru
BIN 			= ../bin
#nazov spustitelneho suboru
BINARY			= lotos121
TARGET			= $(BIN)/$(BINARY)
#adresar so skompilovanymi castami
OBJECT-DIR		= ../objs
#hlavny adresar
MAIN-DIR		= ..
SRC-DIR			= .
#programy
CC      		= gcc
SED			= sed
#
DEPFILE			= .depend
TAGFILE			= tags

ifeq ($(DEBUG),y)
	CFLAGS  	= -O0 -m486 -Wall
else
	CFLAGS		= -O3 -m486 -Wall
endif
HDRS          		=
#definicie
ifeq ($(DEBUG),y)
	DEFS		= -DDEBUG
else
	DEFS		= 
endif
ifeq ($(USE_NETLINKS),y)
	DEFS		+= -DNETLINKS
endif
#kniznice
LIBS    		= -lcrypt
#
# You'll need these libs for a Solaris build
#
#LIBS    		= -lmalloc -lsocket -lnsl

OBJS          = $(OBJECT-DIR)/main.o \
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
#		$(OBJECT-DIR)/netlinks.o
# predchadzajuci riadok odkomentovat pri pouziti NETLINKS


all:   			$(TARGET)
		@echo -e "\a\033[1;32mHotovo\033[0m\n"

$(TARGET):		$(OBJS) Makefile
		$(CC) $(CFLAGS) $(DEFS) $(HDRS) -c build.c -o $(OBJECT-DIR)/build.o
		$(CC) $(CFLAGS) -o $(TARGET) $(DEFS) $(DMALLOC) $(OBJS) $(LIBS) $(HDRS)
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
		@rm -f $(TAGFILE)
		@echo -e "\033[1mzmazane\033[0m\n"

distclean: clean
		@echo -e "\033[1;35mMaking distclean\033[0m\n"
		@rm -f $(MAIN-DIR)/backups/*
		@rm -f $(BIN)/*
		@rm -f $(MAIN-DIR)/dumpfiles/*
		@rm -f $(MAIN-DIR)/logfiles/*
		@rm -f $(OBJECT-DIR)/*
		@rm -f $(MAIN-DIR)/tempfiles/*
		@rm -f $(SRC-DIR)/$(DEPFILE)
		@rm -f $(SRC-DIR)/$(TAGFILE)

reall:
		@echo "kompilujem vsetko od zaciatku\n"
		@make clean
		@make dep
		@make ctags
		@make all

ctags:
		@echo -e "\033[1mMaking tags file\033[0m (useful for vim)\033[0m"
		@ctags --sort=yes --language-force=c *.[h,c] ./plugins/*.[h,c]
		@echo -e "Done ...\n"

final:
		@make reall
		@rm -vf $(OBJECT-DIR)/*

dep:
		@rm -f $(DEPFILE)
		make $(DEPFILE)

dist:
		@echo -e "\033[1;32mMaking distribution\033[0m\n"
		@make distclean
		@( cd ../.. ;\
			echo -e "\033[1;37mcompressing\n\033[0m\n" ;\
			tar zcfp $(DISTFILE) lotos121 ;\
			chmod 600 $(DISTFILE) ;\
			ls -la $(DISTFILE))

###
include $(DEPFILE)

