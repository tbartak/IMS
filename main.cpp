#include <simlib.h>
#include <iostream>
#include <algorithm>

Facility employee("Obsluha/Zaměstnanec"); // Obsluha s kapacitou 1
Store boat("Lodě", 40);                    // Sklad lodí se 40 kusy
double x = 720;                         // Střední hodnota mezi rezervacemi (v minutách) // TODO: bude jako parametr
bool bigReservation = false;            // Příznak, zda se jedná o velkou skupinku

// Délka sezóny a mimo sezóny v minutách
const double MONTH = 30 * 24 * 60;              // Jeden měsíc v minutách
const double SEASON_DURATION = 6.5 * MONTH;     // Délka sezóny
const double OFF_SEASON_DURATION = 5.5 * MONTH; // Délka mimo sezóny

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
    bigReservation ? amount = int(Exponential(20)) + 10 : amount = int(Exponential(2)) + 1;
    
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
        Leave(boat, amount);
    }
};

// Proces opravy vybavení
class boatRepair : public Process {
    int amount;
public:
    boatRepair(int n) : amount(n) {}
    void Behavior() {
        Wait(RepairTime());  // 1-2 týdny
        Leave(boat, amount);
    }
};

// Třída představující rezervaci
class Reservation : public Process {
    void Behavior() {      

        // Čekání do domluveného termínu
        Wait(AgreedTime());
        
        // Kontrola dostupnosti vybavení
        int requestedboat = RandomBoatAmount(); // Náhodný počet vybavení (1-30, průměrně 2)
        Enter(boat, requestedboat);

        // Žádost o obsluhu prodavačem (zákazník přichází, čeká na obsluhu)
        Seize(employee);

        // Obsluha zákazníka:
        // Krok 1: Nafouknutí lodě a poučení o používání
        Wait(InflatingTime(requestedboat));
        // Krok 2: Předvedení lodě, vyfouknutí a sbalení
        Wait(DemonstratingTime(requestedboat));
        // Krok 3: Předání zbytku vybavení, podpis smlouvy, doplacení, ...
        Wait(HandoverTime());
        // Nakonec dojde k uvolnění prodavače
        Release(employee);

        // Vypůjčení vybavení
        // Čekání po dobu výpůjčky
        Wait(BorrowTime());

        // Vracení vybavení
        // Musí se čekat na uvolnění obsluhy
        Seize(employee);

        // Kontrola stavu vybavení
        Wait(CheckTime(requestedboat));    
        
        int broken = 0;
        int wet = 0; 

        for(int i = 0; i < requestedboat; i++) {
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
    }
};

// Generátor rezervací (pro jednotlivce/malé skupinky)
class ReservationGenerator : public Event {
    void Behavior() {
        // Sezóna - generátor je aktivní
        if (Time < SEASON_DURATION) {
            (new Reservation)->Activate();
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
            (new Reservation)->Activate();
            Activate(Time + BigReservationInterval());
        }
    }
};

int main() {
    // Inicializace simulace
    double simulation_duration = SEASON_DURATION + OFF_SEASON_DURATION;
    Init(0, simulation_duration);  // Simulace aktivní sezóny
    
    // Aktivace generátoru rezervací
    (new ReservationGenerator)->Activate();
    (new BigReservationGenerator)->Activate();

    // Spuštění simulace
    Run();
    employee.Output();
    boat.Output();
    return 0;
}
