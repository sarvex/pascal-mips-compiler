
# tools
LEX = flex
LEX_FLAGS =
YACC = bison
YACC_FLAGS = -d
CC = gcc
CC_FLAGS = -g -Wall

# files
LEX_INPUT = pascal.l
LEX_OUTPUT = $(OUTPUT)/lex.yy.c
YACC_INPUT = pascal.y
YACC_OUTPUT = $(OUTPUT)/pacal.tab.c
BINARY = opc
SOURCES := $(wildcard *.c)
OBJECTS = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(SOURCES)))

all: 
$(BINARY): $(OBJECTS)
	gcc $(CC_FLAGS) $(OBJECTS) -o $(BINARY)

$(LEX_OUTPUT):
	$(LEX) $(LEX_FLAGS) pascal.l -o $(LEX_OUTPUT)

yacc:
	$(YACC) $(YACC_FLAGS) pascal.y

.c.o:
	gcc -c $(CC_FLAGS) $<
clean:
	-rm -rf $(OUTPUT) $(BINARY)

.PHONY: all clean tests

