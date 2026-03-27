CC=sdcc
AR=sdar

# Windows-Kompatibilität: 'del' statt 'rm'
ifeq ($(OS),Windows_NT)
    RM=del /Q /F
else
    RM=rm -f
endif

########### BUILD CONFIGURATION ##############

############# END OF BUILD CONFIG OPTIONS #############
OPT=
CFLAGS=-mz80 $(OPT) $(CONFIG)
PEEP_OPTIONS=

OUTPUT_LIBS=SGlib.lib

OBJS=SGlib.rel

ALL: $(OUTPUT_LIBS)

# Library output
SGlib.lib: $(OBJS)
	sdar r $@ $^

# Compilation rule
SGlib.rel: SGlib.c SGlib.h
	$(CC) $(CFLAGS) $(PEEP_OPTIONS) -c -o $@ $<

clean:
	$(RM) $(OUTPUT_LIBS) $(OBJS) *.lst *.sym *.asm
