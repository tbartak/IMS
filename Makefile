COMPILER = g++
FLAGS = -I/usr/local/include -L/usr/local/lib -lsimlib
EXEC = main
SRC = main.cpp

all:
	$(COMPILER) -o $(EXEC) $(SRC) $(FLAGS)

clean:
	rm -f $(EXEC)

run:
	./$(EXEC)