#include "Parser.hpp"

#include <iostream>

int Parser::parseArguments(int argc, char* argv[]) {
    int opt;

    while ((opt = getopt(argc, argv, "c:b:s:l:")) != -1) {
        switch (opt) {
            case 'c': // Zakaznici
                customers = std::stoi(optarg);
                break;
            case 'b': // Pocet lodi
                boatNumber = std::stoi(optarg);
                break;
            case 's': // Delka sezony
                seasonLength = std::stod(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-c customers] [-b boatNumber] [-s seasonLength] [-l simulationLength]" << std::endl;
                return -1;
        }
    }

    return 0;
}

int Parser::getCustomers() {
    return customers;
}

int Parser::getBoatNumber() {
    return boatNumber;
}

double Parser::getSeasonLength() {
    return seasonLength;
}
