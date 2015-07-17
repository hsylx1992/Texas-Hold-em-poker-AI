CC		= gcc
LINKER	= gcc -o
#CLIBS	= -lcurl -lm -lpthread
CLIBS	= -lm -lpthread
rm		= rm -f

all: 	CFLAGS = -Wall -Werror -pedantic -std=gnu99 -Wno-unused-result -O3
debug: 	CFLAGS = -Wall -Werror -pedantic -std=gnu99 -g

SRCDIR	= ../source
OBJDIR	= ./
BINDIR	= ../target

COMMONDIR 		= $(SRCDIR)/common
COMMUNICATIONDIR= $(SRCDIR)/communication
AIPLAYERDIR 	= $(SRCDIR)/client/AI_player
#AIALLINDIR 		= $(SRCDIR)/client/AI_all_in
#AIFLOPDIR	 	= $(SRCDIR)/client/AI_flop
	
AIPLAYER_INCSRC = $(COMMONDIR) $(COMMUNICATIONDIR) $(AIPLAYERDIR)
#AIALLIN_INCSRC = $(COMMONDIR) $(COMMUNICATIONDIR) $(AIALLINDIR)
#AIFLOP_INCSRC 	= $(COMMONDIR) $(COMMUNICATIONDIR) $(AIFLOPDIR)

AIPLAYER_INC 	= $(foreach d, $(AIPLAYER_INCSRC), -I$d)
#AIALLIN_INC 	= $(foreach d, $(AIALLIN_INCSRC), -I$d)
#AIFLOP_INC 	= $(foreach d, $(AIFLOP_INCSRC), -I$d)

COMMON_SOURCES 		= $(wildcard $(COMMONDIR)/*.c)
COMMUNICATION_SOURCES = $(wildcard $(COMMUNICATIONDIR)/*.c)
AIPLAYER_SOURCES 	= $(wildcard $(AIPLAYERDIR)/*.c)
#AIALLIN_SOURCES 	= $(wildcard $(AIALLINDIR)/*.c)
#AIFLOP_SOURCES 	= $(wildcard $(AIFLOPDIR)/*.c)

COMMON_OBJECTS 		:= $(patsubst $(COMMONDIR)/%.c, $(OBJDIR)/%.o, $(COMMON_SOURCES))
COMMUNICATION_OBJECTS := $(patsubst $(COMMUNICATIONDIR)/%.c, $(OBJDIR)/%.o, $(COMMUNICATION_SOURCES))
AIPLAYER_OBJECTS 	:= $(patsubst $(AIPLAYERDIR)/%.c, $(OBJDIR)/%.o, $(AIPLAYER_SOURCES))
#AIALLIN_OBJECTS 	:= $(patsubst $(AIALLINsDIR)/%.c, $(OBJDIR)/%.o, $(AIALLIN_SOURCES))
#AIFLOP_OBJECTS 	:= $(patsubst $(AIFLOPDIR)/%.c, $(OBJDIR)/%.o, $(AIFLOP_SOURCES))
OBJECTS 			:= $(wildcard $(OBJDIR)/*.o)

#TARGETS 			:= game AI_all_in AI_flop
TARGETS				:= game
TARGETS 			:= $(foreach t, $(TARGETS), $(BINDIR)/$t)

all: $(TARGETS)

debug: $(TARGETS)
	ctags -R *

$(BINDIR)/game: $(COMMON_OBJECTS) $(COMMUNICATION_OBJECTS) $(AIPLAYER_OBJECTS)
	@echo "\t[link] "$@
	@$(LINKER) $@ $(CFLAGS) $(AIPLAYER_INC) $(COMMON_OBJECTS) $(COMMUNICATION_OBJECTS) $(AIPLAYER_OBJECTS) $(CLIBS)
	@echo "\n"$@" built successfully.\n"
	
#$(BINDIR)/AI_all_in: $(COMMON_OBJECTS) $(COMMUNICATION_OBJECTS) $(AIALLIN_OBJECTS)
#	@echo "\t[link] "$@
#	@$(LINKER) $@ $(CFLAGS) $(AIALLIN_INC) $(COMMON_OBJECTS) $(COMMUNICATION_OBJECTS) $(AIALLIN_OBJECTS) $(CLIBS)
#	@echo "\n"$@" built successfully.\n"
	
#$(BINDIR)/AI_flop: $(COMMON_OBJECTS) $(COMMUNICATION_OBJECTS) $(AIFLOP_OBJECTS)
#	@echo "\t[link] "$@
#	@$(LINKER) $@ $(CFLAGS) $(AIFLOP_INC) $(COMMON_OBJECTS) $(COMMUNICATION_OBJECTS) $(AIFLOP_OBJECTS) $(CLIBS)
#	@echo "\n"$@" built successfully.\n"
		
$(COMMON_OBJECTS): $(OBJDIR)/%.o : $(COMMONDIR)/%.c
	@echo "\t[compile] "$<
	@$(CC) $(CFLAGS) -c $< -o $@ $(CLIBS)

$(COMMUNICATION_OBJECTS): $(OBJDIR)/%.o : $(COMMUNICATIONDIR)/%.c
	@echo "\t[compile] "$<
	@$(CC) $(CFLAGS) -c $< -o $@ $(CLIBS)
	
$(AIPLAYER_OBJECTS): $(OBJDIR)/%.o : $(AIPLAYERDIR)/%.c
	@echo "\t[compile] "$<
	@$(CC) $(CFLAGS) $(AIPLAYER_INC) -c $< -o $@ $(CLIBS)

#$(AIALLIN_OBJECTS): $(OBJDIR)/%.o : $(AIALLINDIR)/%.c
#	@echo "\t[compile] "$<
#	@$(CC) $(CFLAGS) $(AIALLIN_INC) -c $< -o $@ $(CLIBS)

#$(AIFLOP_OBJECTS): $(OBJDIR)/%.o : $(AIFLOPDIR)/%.c
#	@echo "\t[compile] "$<
#	@$(CC) $(CFLAGS) $(AIFLOP_INC) -c $< -o $@ $(CLIBS)
		
clean:
	@echo "Removing object files"
	@$(rm) $(OBJECTS)

remove: clean
	@echo "Removing object files and binaries
	@$(rm) $(TARGETS)
	
