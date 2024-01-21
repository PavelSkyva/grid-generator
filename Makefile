RM=rm -f
CC=gcc
CFLAGS= -Wall -Wextra -std=c17 -pedantic -lm


EX= grid-generator

#sem vepsat vzdy kazdy novy soubor (s priponou .o)
SRC= grid-generator.o

all: $(SRC)
		$(CC) $(CFLAGS) -o $(EX) $(SRC)

clean: #smaz vsechny object files a binarky 
		$(RM) ${SRC} ${TSRC} ${EX} ${TEX}