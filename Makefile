SRC=./src
TARGET=./target
CC = gcc
LDFLAGS =
LIBS = -lpsapi
RM = rm -rf

all: clean default run

clean:
	$(RM) $(TARGET)/*.out

default: nightview.c

nightview.c:
	$(CC) $(CFLAGS) $(SRC)/nightview.c $(LIBS) -o $(TARGET)/nightview.exe

run:
	$(TARGET)/nightview.exe
