COMPILER = g++
CFLAGS = -std=c++11
FLAGS = -I/usr/local/include -lsimlib
EXEC = main
SRC = Parser.cpp main.cpp

all:
	$(COMPILER) $(CFLAGS) -o $(EXEC) $(SRC) $(FLAGS)

clean:
	rm -f $(EXEC)

run:
	./$(EXEC) -c 2 -b 40 -s 6.5

experiment1:
	./$(EXEC) -c 2 -b 40 -s 6.5
	./$(EXEC) -c 2 -b 41 -s 6.5

experiment2:
	./$(EXEC) -c 2 -b 40 -s 6.5