OUTPUT := Akalabeth
DEMOFLAG :=
TARGET_PLATFORM := sg
DEBUGFLAG := --debug
#--debug
#--verbose

CODE_START := 0x0000
#DATA_START := 0xF000
DATA_START := 0xC000


# Entferne -j4, das kann bei SDCC zu Problemen führen
# MAKEFLAGS += -j4

ifeq ($(TARGET_PLATFORM),sg)
	COMPILER := sdcc
	FLAGS := -DPLATFORM_SG $(DEMOFLAG) -DGAME_NAME=\"$(OUTPUT)\" $(DEBUGFLAG)
endif

LINKER := ihx2sms
RES2SRC := folder2c
MAINENTRY := main

FILES = assets/font.c assets/images.c assets/audio.c

ifeq ($(OS),Windows_NT)
	FILES_WIN = $(subst /,\,$(FILES))
    RM = del /Q
else
	FILES_WIN = $(FILES)
    RM = rm -f
endif

# Alle Header-Dateien automatisch finden
HEADERS := $(wildcard *.h) $(wildcard libs/*.h) $(wildcard src/*.h)

all: $(OUTPUT).sg

clean:
	$(RM) *.rel *.ihx *.sc *.o *.sym *.lst *.adb *.sms *.sg *.asm *.map *.lk *.cdb *.noi
	$(RM) $(subst /,$(if $(filter Windows_NT,$(OS)),\,/),assets/font.rel assets/images.rel assets/audio.rel assets/bpe_table.rel assets/bpe_texts.rel)
	$(RM) $(FILES_WIN)

# Asset-Generierung
assets/font.c:
	$(RES2SRC) assets/font assets/font

assets/images.c:
	$(RES2SRC) assets/images assets/images

assets/audio.c:
	$(RES2SRC) assets/audio assets/audio

# Generische Regel für .c -> .rel MIT Header-Abhängigkeiten
%.rel: %.c $(HEADERS)
	$(COMPILER) $(FLAGS) -c -mz80 $<

# Spezifische Regel für main.c mit Include-Pfaden
$(MAINENTRY).rel: $(MAINENTRY).c $(HEADERS)
	$(COMPILER) $(FLAGS) -c -mz80 --peep-file peep-rules.txt -Ilibs -Isrc $(MAINENTRY).c

# Spezifische Regeln für libs (falls besondere Flags nötig sind)
libs/strings.rel: libs/strings.c $(HEADERS)
	$(COMPILER) $(FLAGS) -c -mz80 libs/strings.c

libs/console.rel: libs/console.c $(HEADERS)
	$(COMPILER) $(FLAGS) -c -mz80 libs/console.c

# Asset-Regeln mit Abhängigkeiten
assets/font.rel: assets/font.c
	$(COMPILER) $(FLAGS) -c -mz80 -o assets/font.rel assets/font.c

assets/images.rel: assets/images.c
	$(COMPILER) $(FLAGS) -c -mz80 -o assets/images.rel assets/images.c

assets/audio.rel: assets/audio.c
	$(COMPILER) $(FLAGS) -c -mz80 -o assets/audio.rel assets/audio.c

assets/bpe_table.rel: assets/bpe_table.c assets/bpe_table.h
	$(COMPILER) $(FLAGS) -c -mz80 -o assets/bpe_table.rel assets/bpe_table.c

assets/bpe_texts.rel: assets/bpe_texts.c assets/bpe_texts.h
	$(COMPILER) $(FLAGS) -c -mz80 -o assets/bpe_texts.rel assets/bpe_texts.c

# IHX-Linking mit allen Abhängigkeiten
$(OUTPUT).ihx: libs/console.rel libs/strings.rel global.rel views.rel widgets.rel \
               $(MAINENTRY).rel libbasic.rel fbuffer.rel animation.rel \
               assets/font.rel assets/images.rel assets/audio.rel \
               assets/bpe_table.rel assets/bpe_texts.rel \
               bpe.rel engine.rel engine2.rel monster.rel
	$(COMPILER) $(FLAGS) -o $(OUTPUT).ihx -mz80 --no-std-crt0 --data-loc 0xC000 \
		crt0/crt0_sg.rel PSGlib/PSGlib.rel SGlib/SGlib.rel \
		console.rel global.rel strings.rel engine.rel engine2.rel monster.rel views.rel widgets.rel \
		$(MAINENTRY).rel assets/font.rel assets/images.rel assets/audio.rel \
		assets/bpe_table.rel assets/bpe_texts.rel bpe.rel \
		libbasic.rel fbuffer.rel animation.rel

$(OUTPUT).$(TARGET_PLATFORM): $(OUTPUT).ihx
	$(LINKER) $(OUTPUT).ihx $(OUTPUT).$(TARGET_PLATFORM)

run: $(OUTPUT).$(TARGET_PLATFORM)
ifeq ($(OS),Windows_NT)
	"$(OUTPUT).$(TARGET_PLATFORM)"
else
	./$(OUTPUT).$(TARGET_PLATFORM)
endif

.PHONY: all clean run
