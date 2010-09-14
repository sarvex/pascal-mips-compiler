
# default target
all:

# tools
LEX = flex
LEX_FLAGS =
YACC = bison
YACC_FLAGS = -d
CC = gcc
CC_FLAGS = -g -Wall

# dirs
OUTPUT = output
OUTPUT:
	mkdir -p $@

# files
LEX_INPUT = pascal.l
LEX_OUTPUT = lex.yy.c
YACC_INPUT = pascal.y
YACC_OUTPUT = pascal.tab.c
YACC_SIDE_EFFECTS = pascal.tab.h
BINARY = opc
SOURCES := $(wildcard *.c) $(LEX_OUTPUT) $(YACC_OUTPUT)
OBJECTS = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(SOURCES)))
$(OBJECTS): | $(OUTPUT)


$(LEX_OUTPUT): $(LEX_INPUT)
	$(LEX) $(LEX_FLAGS) $(LEX_INPUT) -o $(LEX_OUTPUT)

$(YACC_OUTPUT): $(YACC_INPUT)
	$(YACC) $(YACC_FLAGS) $(YACC_INPUT) -o $(YACC_OUTPUT)


all: $(BINARY)
$(BINARY): $(OBJECTS)
	gcc $(CC_FLAGS) $(OBJECTS) -o $(BINARY)

.c.o:
	gcc -c $(CC_FLAGS) $<
clean:
	-rm -rf $(LEX_OUTPUT) $(YACC_OUTPUT) $(YACC_SIDE_EFFECTS) $(OUTPUT) $(BINARY)

.PHONY: all clean

