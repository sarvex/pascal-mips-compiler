
# default target
all:

# dirs
OUTPUT = output
$(OUTPUT):
	mkdir -p $@

# files
LEX_INPUT = pascal.l
LEX_OUTPUT = $(OUTPUT)/lex.yy.cpp
LEX_OBJECT = $(LEX_OUTPUT).o
$(LEX_OUTPUT): | $(OUTPUT)
YACC_INPUT = pascal.y
YACC_OUTPUT = $(OUTPUT)/y.tab.cpp
$(YACC_OUTPUT): | $(OUTPUT)
YACC_OBJECT = $(YACC_OUTPUT).o
BINARY = opc
SOURCES := $(wildcard *.cpp)
OBJECTS = $(addprefix $(OUTPUT)/,$(addsuffix .o,$(SOURCES)))
ALL_OBJECTS = $(OBJECTS) $(LEX_OBJECT) $(YACC_OBJECT)
$(ALL_OBJECTS): $(LEX_OUTPUT) $(YACC_OUTPUT)
DEPEND_FILES = $(addsuffix .d,$(ALL_OBJECTS))
-include $(DEPEND_FILES)

# tools
LEX = flex
LEX_FLAGS =
YACC = bison
YACC_FLAGS = -d -y
CC = g++
CC_FLAGS = -ggdb -Wall -I. -I$(OUTPUT)
CC_COMPILE = $(CC) $(CC_FLAGS) -c -o $@ -MMD -MP -MF $@.d
LINK = g++
LINK_FLAGS =
TEST = python test.py

# rulz
$(OBJECTS):
	$(CC_COMPILE) $(notdir $(basename $@))

$(LEX_OUTPUT): $(LEX_INPUT)
	$(LEX) $(LEX_FLAGS) -o$(LEX_OUTPUT) $(LEX_INPUT)
$(LEX_OBJECT):
	$(CC_COMPILE) $(LEX_OUTPUT)

$(YACC_OUTPUT): $(YACC_INPUT)
	$(YACC) $(YACC_FLAGS) $(YACC_INPUT) -o $(YACC_OUTPUT)
	sed -i 1i'#include "parser.h"' $(basename $(YACC_OUTPUT)).hpp
$(YACC_OBJECT):
	$(CC_COMPILE) $(YACC_OUTPUT)


all: $(BINARY)
$(BINARY): $(ALL_OBJECTS)
	$(LINK) $(LINK_FLAGS) $(ALL_OBJECTS) -o $(BINARY)

test: all
	$(TEST)

clean:
	rm -rf $(OUTPUT) $(BINARY)

# the | in this list is because old make doesn't have order-only rules
.PHONY: all clean |

