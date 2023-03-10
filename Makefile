MKDIR   := md
RMDIR   := rd /S /Q
CC      := gcc

BIN     := ./bin
OBJ     := ./obj
INCLUDE := ./include

SRC     := ./src
SRCS    := $(wildcard $(SRC)/*.c)
OBJS    := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
EXE     := $(BIN)/nightview.exe
CFLAGS  := -I$(INCLUDE)
LDLIBS  := -lpsapi

all: default run

clean:
	$(RMDIR) $(OBJ) $(BIN)

default: $(EXE)

$(EXE): $(OBJS) | $(BIN)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN) $(OBJ):
	$(MKDIR) $@

run: $(EXE)
	$<
