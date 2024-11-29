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
	./$(EXEC) -c 2 -b 30 -s 6.5
	./$(EXEC) -c 2 -b 35 -s 6.5
	./$(EXEC) -c 2 -b 40 -s 6.5
	./$(EXEC) -c 2 -b 45 -s 6.5
	./$(EXEC) -c 2 -b 50 -s 6.5

experiment2:
	./$(EXEC) -c 2 -b 40 -s 6.5
	./$(EXEC) -c 4 -b 40 -s 6.5
	./$(EXEC) -c 10 -b 40 -s 6.5

experiment3:
	./$(EXEC) -c 2 -b 40 -s 6.5
	./$(EXEC) -c 2 -b 40 -s 3.5
	./$(EXEC) -c 2 -b 40 -s 8

zip:
	zip T6_xbarta51_xbabia01.zip *.cpp *.hpp Makefile dokumentace.pdf