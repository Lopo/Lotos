#*
#*   Lotos v1.2.2  : (c) 1999-2002 Pavol Hluchy (Lopo)
#*   last update   : 16.5.2002
#*   email         : lopo@losys.sk
#*   homepage      : lopo.losys.sk
#*   Lotos homepage: lotos.losys.sk
#*

USE_NETLINKS		= n
USE_PUEBLO		= n
USE_DEBUG		= n

#adresare
DIR_MAIN		= ..
DIR_SRC			= $(DIR_MAIN)/src
DIR_BIN			= $(DIR_MAIN)/bin
DIR_OBJ			= $(DIR_MAIN)/objs
#parametre vyslednej binarky
BINARY			= lotos122
TARGET			= $(DIR_BIN)/$(BINARY)
PERMS			= 700
#programy
CC      		= gcc
SED			= sed
#pomocne subory
FILE_DEP		= .depend
FILE_TAG		= tags
FILE_DIST		= lotos122.tgz

ifeq ($(USE_DEBUG), y)
	CFLAGS  	= -O0 -m486 -Wall -g
else
	CFLAGS		= -O3 -m486 -Wall
endif
#definicie
ifeq ($(USE_DEBUG), y)
	DEFS		= -DDEBUG
else
	DEFS		= 
endif
ifeq ($(USE_NETLINKS), y)
	DEFS		+= -DNETLINKS
endif
ifeq ($(USE_PUEBLO), y)
	DEFS		+= -DPUEBLO
endif
#kniznice
LIBS    		= -lcrypt
#
# You'll need these libs for a Solaris build
#
#LIBS    		= -lmalloc -lsocket -lnsl

OBJS          = $(DIR_OBJ)/main.o \
		$(DIR_OBJ)/globals.o \
		$(DIR_OBJ)/prompts.o \
		$(DIR_OBJ)/plugin.o \
		$(DIR_OBJ)/adds.o \
		$(DIR_OBJ)/boots.o \
		$(DIR_OBJ)/ct_general.o \
		$(DIR_OBJ)/ct_social.o \
		$(DIR_OBJ)/ct_msg.o \
		$(DIR_OBJ)/ct_user.o \
		$(DIR_OBJ)/ct_ignore.o \
		$(DIR_OBJ)/ct_move.o \
		$(DIR_OBJ)/ct_clone.o \
		$(DIR_OBJ)/ct_admin.o \
		$(DIR_OBJ)/mail.o \
		$(DIR_OBJ)/email.o \
		$(DIR_OBJ)/boards.o \
		$(DIR_OBJ)/transport.o \
		$(DIR_OBJ)/macros.o \
		$(DIR_OBJ)/fonts.o \
		$(DIR_OBJ)/poc_zoz.o \
		$(DIR_OBJ)/who.o \
		$(DIR_OBJ)/s_string.o \
		$(DIR_OBJ)/s_events.o \
		$(DIR_OBJ)/s_net.o \
		$(DIR_OBJ)/money.o \
		$(DIR_OBJ)/menus.o \
		$(DIR_OBJ)/restart.o \
		$(DIR_OBJ)/build.o
ifeq ($(USE_NETLINKS), y)
	OBJS	+= $(DIR_OBJ)/netlinks.o
endif
ifeq ($(USE_PUEBLO), y)
	OBJS	+= $(DIR_OBJ)/pueblo.o
endif

all:   			$(TARGET)
		@printf "\a\033[1;32mHotovo\033[0m\n"

$(TARGET):		$(OBJS) Makefile
		$(CC) $(CFLAGS) $(DEFS) -c build.c -o $(DIR_OBJ)/build.o
		$(CC) $(CFLAGS) -o $(TARGET) $(DEFS) $(OBJS) $(LIBS)
ifeq ($(USE_DEBUG), n)
		@strip $(TARGET)
endif
		@chmod $(PERMS) $(TARGET)

$(FILE_DEP):
		@printf "\033[1;32mMaking $(FILE_DEP)\033[0m ..."
		@touch $(FILE_DEP)
		@$(CC) -MM *.c -MM ./plugins/*.c > .tmdepend
		@$(SED) -e '1,$$s/^[a-z]/$$(DIR_OBJ)\/&/' .tmdepend > $(FILE_DEP)
		@rm -f .tmdepend
		@printf " \033[1;32mdone\033[0m\n"

$(DIR_OBJ)/%.o:	prototypes.h define.h obj_*.h commands.h
		$(CC) $(CFLAGS) $(DEFS) $(NETLINKS) -c $*.c -o $(DIR_OBJ)/$*.o

clean:
		@printf "\033[1;31mDeleting binaries\033[0m ..."
		@rm -f $(OBJS)
		@rm -f $(TARGET)
		@rm -f $(FILE_TAG)
		@printf " \033[1;32mdone\033[0m\n"

distclean: clean
		@printf "\033[1;31mMaking distclean\033[0m ... "
		@rm -f $(DIR_MAIN)/backups/*
		@rm -f $(DIR_MAIN)/dumpfiles/*
		@rm -f $(DIR_MAIN)/logfiles/*
		@rm -f $(DIR_MAIN)/tempfiles/*
		@rm -f $(DIR_SRC)/$(FILE_DEP)
		@rm -f $(DIR_SRC)/$(FILE_TAG)
		@printf "\033[1;32mdone\033[0m\n"

reall:
		@printf "kompilujem vsetko od zaciatku\n"
		@make clean
		@make dep
		@make ctags
		@make all

ctags:
		@printf "\033[1;32mMaking tags file\033[0m (useful for vim) ... "
		@ctags --sort=yes --language-force=c *.[h,c] ./plugins/*.[h,c]
		@printf "\033[1;32mdone\033[0m\n"

final:
		@make reall
		@rm -vf $(DIR_OBJ)/*

dep:
		@rm -f $(FILE_DEP)
		@make $(FILE_DEP)

dist:
		@printf "\033[1;32mMaking distribution\033[0m ... "
		@make distclean
		@( cd ../..;\
			printf "\033[1;37mcompressing\n\033[0m\n";\
			tar zcfp $(FILE_DIST) lotos122;\
			chmod 600 $(FILE_DIST);\
			pwd;\
			ls -la $(FILE_DIST))
		@printf " \033[1;32mdone\033[0m\n"

###
include $(FILE_DEP)

