# Install
EXEC = test
CXX = g++

# Flags
CFLAGS += `sdl2-config --cflags`

SRC = main.cpp
OBJ = $(SRC:.c=.o)

UNAME_S := $(shell uname -s)
LIBS += -lm -ldl `sdl2-config --libs`

$(EXEC):
	rm -f $(EXEC)
	$(CXX) $(SRC) $(CFLAGS) -o $(EXEC) $(LIBS)
