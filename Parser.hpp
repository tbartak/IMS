#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <getopt.h>

class Parser {
private:
    int customers = 2;
    int boatNumber = 40;
    double seasonLength = 6.5;

public:
    int parseArguments(int argc, char* argv[]);
    int getCustomers();
    int getBoatNumber();
    double getSeasonLength();
};

#endif