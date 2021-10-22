CC=gcc

SRC_PATH=src
OBJ_PATH=.obj
SRC=$(wildcard $(SRC_PATH)/*.c)
OBJ=$(patsubst $(SRC_PATH)/%,$(OBJ_PATH)/%,$(SRC:.c=.o))

ALIB_SRC_PATH=./alib/src
ALIB_SRC=$(wildcard $(ALIB_SRC_PATH)/*.c)
ALIB_OBJ=$(patsubst $(ALIB_SRC_PATH)/%,$(OBJ_PATH)/%,$(ALIB_SRC:.c=.o))

INC= -lm

LDFLAGS = -lm
CFLAGS = -ggdb3 -Og -Wall -Wextra -pedantic-errors -fstrict-aliasing -std=c99

all: debug

$(OBJ_PATH)/%.o: $(ALIB_SRC_PATH)/%.c
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

alib: $(ALIB_OBJ)

debug: $(OBJ) $(ALIB_OBJ)
	$(CC) -o bfc $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ)
	rm -f testing
