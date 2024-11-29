COMPILER = g++
CFLAGS = -std=c++11
FLAGS = -I/usr/local/include -L/usr/local/lib -lsimlib
EXEC = main
SRC = Parser.cpp main.cpp

all:
	$(COMPILER) $(CFLAGS) -o $(EXEC) $(SRC) $(FLAGS)

clean:
	rm -f $(EXEC)

run:
	./$(EXEC) -c 2 -b 40 -s 6.5