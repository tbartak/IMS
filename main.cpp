#include <simlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include "Parser.hpp"

#define defaultBoatNum 40

Facility employee("Obsluha/Zaměstnanec"); // Obsluha s kapacitou 1
int boatNumber = defaultBoatNum;
Store *boat;                   // Sklad lodí se 40 kusy
double x;                                  // Střední hodnota mezi rezervacemi (v minutách) // TODO: bude jako parametr
bool bigReservation = false;            // Příznak, zda se jedná o velkou skupinku

// Délka sezóny a mimo sezóny v minutách
const double MONTH = 30 * 24 * 60;              // Jeden měsíc v minutách
double SEASON_DURATION;                         // Délka sezóny
double OFF_SEASON_DURATION; // Délka mimo sezóny
double ACTIVE_SEASON_COUNT;

std::vector<class Reservation*> reservations;  // Rezervace

// Vytvoření nové rezervace s exponenciálním rozložením se středem x
double ReservationInterval() {
    double currentTime = Time;

    double firstMonth = 1 * MONTH;
    double lastMonth = SEASON_DURATION - 1 * MONTH;

    if(currentTime <= firstMonth || currentTime >= lastMonth) {
        return Exponential(2 * x);
    }

    return Exponential(x);
}

double BigReservationInterval() {
    double currentTime = Time;

    double firstMonth = 1 * MONTH;
    double lastMonth = SEASON_DURATION - 1 * MONTH;

    if(currentTime <= firstMonth || currentTime >= lastMonth) {
        return Exponential(40 * x);
    }
    return Exponential(20 * x);
}

// Simulování domluveného termínu rezervace
double AgreedTime() {
    return Uniform(7 * 24 * 60, 30 * 24 * 60);  // 7-30 dní v minutách
}

// Simulování doby nafukování + poučení o používání
double InflatingTime(int amount) {
    int sum = 0;

    for(int i = 0; i < amount; i++) {
        sum += Uniform(3, 8);   // 3 to 8 minutes
    }
    
    return sum;  
}

// Simulování doby předvedení + vyfouknutí a sbalení
double DemonstratingTime(int amount) {
    int sum = 0;
    
    for(int i = 0; i < amount; i++) {
        sum += Exponential(10);  // Exponenciální rozložení se střední hodnotou 10 minut
    }
    
    return sum;
}

// Simulování předání zbytku vybavení + podpis smlouvy, doplacení, ...
double HandoverTime() {
    return Uniform(4, 10);  // 4 to 10 minutes
}

// Simulování doby výpůjčky
double BorrowTime() {
    return Uniform(2 * 24 * 60, 7 * 24 * 60);  // 2-7 dní v minutách
}

// Doba kontroly vybavení při vrácení
double CheckTime(int amount) {
    // Implementace doby kontroly
    int sum = 0;

    for (int i = 0; i < amount; i++) {
        sum += Uniform(5, 10); // 5 to 10 minutes
    }

    return sum;
}

// Simulování doby vysoušení vybavení
double DryingTime(int amount) {
    int sum = 0;

    for (int i = 0; i < amount; i++) {
        sum += Uniform(2, 15); // 2 to 15 minutes
    }

    return sum; 
}

// Simulování doby opravy vybavení
double RepairTime() {
    return Uniform(7 * 1440, 14 * 1440);  // 1-2 týdny
}

// Simulování doplatku za poškození vybavení
double DamageSettlementTime() {
    return Uniform(1, 2);  // 1 - 2 minuty
}

// Simulování náhodného počtu chtěného vybavení
int RandomBoatAmount() {
    int amount = 0;

    // Průměrně 2 kusy vybavení, pro velké skupinky 20 kusů
    bigReservation ? amount = int(Exponential(1)) + 10 : amount = int(Exponential(2)) + 1;
    
    // V případě velké skupinky se omezí počet vybavení na 40, jinak 30
    if(bigReservation) {
        bigReservation = false;
        return (amount > 40) ? 40 : amount;

    } else {
        return (amount > 10) ? 10 : amount;
    }
}

// Proces sušení vybavení
class boatDrying : public Process {
    int amount;
public:
    boatDrying(int n) : amount(n) {}
    void Behavior() {
        Wait(DryingTime(amount));
        Leave(*boat, amount);
    }
};

// Proces opravy vybavení
class boatRepair : public Process {
    int amount;
public:
    boatRepair(int n) : amount(n) {}
    void Behavior() {
        Wait(RepairTime());  // 1-2 týdny
        Leave(*boat, amount);
    }
};

// Třída představující rezervaci
class Reservation : public Process {
public:
    int requestedBoat;
    double reservationStart;
    double reservationEnd;

    Reservation(int boats, double start, double end)
        : requestedBoat(boats), reservationStart(start), reservationEnd(end) {}

    void Behavior() {      

        Wait(reservationStart - Time);
        Enter(*boat, requestedBoat);

        // Žádost o obsluhu prodavačem (zákazník přichází, čeká na obsluhu)
        Seize(employee);

        // Obsluha zákazníka:
        // Krok 1: Nafouknutí lodě a poučení o používání
        Wait(InflatingTime(requestedBoat));
        // Krok 2: Předvedení lodě, vyfouknutí a sbalení
        Wait(DemonstratingTime(requestedBoat));
        // Krok 3: Předání zbytku vybavení, podpis smlouvy, doplacení, ...
        Wait(HandoverTime());
        // Nakonec dojde k uvolnění prodavače
        Release(employee);

        // Vypůjčení vybavení
        // Čekání po dobu výpůjčky
        Wait(reservationEnd - reservationStart);

        // Vracení vybavení
        // Musí se čekat na uvolnění obsluhy
        Seize(employee);

        // Kontrola stavu vybavení
        Wait(CheckTime(requestedBoat));    
        
        int broken = 0;
        int wet = 0; 

        for(int i = 0; i < requestedBoat; i++) {
            Random() < 0.05 ? broken++ : wet++;
        }
        
        if (wet > 0) {
            // Sušení vybavení
            (new boatDrying(wet))->Activate();
        }

        // Rozhodnutí o stavu vybavení (5% šance, že bude poškozené)
        if (broken > 0) {  // 5% pravděpodobnost poškození
            // Vyřizování škod se zákazníkem
            Wait(DamageSettlementTime());
            // Usušení rozbitých lodí před opravou
            Wait(DryingTime(broken));
            // Uvolnění obsluhy
            Release(employee);
            // Vybavení jde do opravy
            (new boatRepair(broken))->Activate();
        } else {
            // Uvolnění obsluhy
            Release(employee);
        }

        // Remove the reservation from the list
        auto it = std::find(reservations.begin(), reservations.end(), this);
        if (it != reservations.end()) {
            reservations.erase(it);
        }
    }
};

bool IsAvailable(int requestedBoat, double start, double end, std::vector<Reservation*>& reservations) {
    int boatsInUse = 0;

    for (auto& res : reservations) {
        // Najdi rezervace, které se překrývají s novou rezervací
        if (!(res->reservationEnd <= start || res->reservationStart >= end)) {
            boatsInUse += res->requestedBoat;
        }
    }
    
    return (boatsInUse + requestedBoat) <= boatNumber;
}

double FindEarliestAvailableTime(int requestedBoat, double desiredStart, double desiredEnd, std::vector<Reservation*>& reservations) {
    // Najdi nejbližší čas, kdy je dostatek lodí k dispozici
    double currentTime = Time;

    while (currentTime < desiredStart) {
        if (IsAvailable(requestedBoat, currentTime, currentTime + (desiredEnd - desiredStart), reservations)) {
            return currentTime;
        }
        currentTime += 60; // Přeskoč na další hodinu
    }

    return -1; // Nenalezeno
}

void CreateReservation() {
    int requestedBoat = RandomBoatAmount();
    double start = Time + AgreedTime();
    double duration = BorrowTime();
    double end = start + duration;

    if (IsAvailable(requestedBoat, start, end, reservations)) {
        auto reservation = new Reservation(requestedBoat, start, end);
        reservations.push_back(reservation);
        reservation->Activate(start);

    } else {
        // Pokud není dostatek lodí k dispozici, najdi nejbližší čas, kdy je dostatek lodí k dispozici
        double earliestAvailable = FindEarliestAvailableTime(requestedBoat, start, end, reservations);
        if (earliestAvailable >= 0) {
            auto reservation = new Reservation(requestedBoat, earliestAvailable, earliestAvailable + duration);
            reservations.push_back(reservation);
            reservation->Activate(earliestAvailable);
        }
    }
}

// Generátor rezervací (pro jednotlivce/malé skupinky)
class ReservationGenerator : public Event {
    void Behavior() {
        // Sezóna - generátor je aktivní
        if (Time < SEASON_DURATION) {
            CreateReservation();
            Activate(Time + ReservationInterval());
        }
    }
};

// Generátor rezervací (pro velké skupiny)
class BigReservationGenerator : public Event {
    void Behavior() {
        // Sezóna - generátor je aktivní
        if (Time < SEASON_DURATION) {
            bigReservation = true;
            CreateReservation();
            Activate(Time + BigReservationInterval());
        }
    }
};

int main(int argc, char* argv[]) {
    Parser argParser;

    if(argParser.parseArguments(argc, argv) == -1) {
        return -1;
    }

    x = 1440 / argParser.getCustomers(); // 1440 minut = 1 den
    SEASON_DURATION = argParser.getSeasonLength() * MONTH;

    if (SEASON_DURATION > 12 * MONTH)
    {
        SEASON_DURATION = 12 * MONTH;
    }
    OFF_SEASON_DURATION = 12 * MONTH - SEASON_DURATION;
    boatNumber = argParser.getBoatNumber();

    boat = new Store("Lodě", boatNumber);  // Sklad lodí (v základu se 40 kusy)

    // Inicializace simulace
    double simulation_duration = SEASON_DURATION + OFF_SEASON_DURATION;
    Init(0, simulation_duration);  // Simulace aktivní sezóny
    
    // Aktivace generátoru rezervací
    (new ReservationGenerator)->Activate();
    (new BigReservationGenerator)->Activate();

    // Spuštění simulace
    Run();
    employee.Output();
    boat->Output();
    return 0;
}
