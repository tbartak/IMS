#include <simlib.h>
#include <iostream>

// deklarace obslužných linek
// deklarace tříd popisujících procesy a události
// popis simulačního experimentu ve funkci main()

Facility seller;           // Prodavač s kapacitou 1
Store equipment(100);      // Sklad vybavení se 100 kusy
double x = 6 * 60;             // Střední hodnota mezi rezervacemi (v minutách)

// Délka sezóny a mimo sezóny v minutách
const double MONTH = 30 * 24 * 60;           // Jeden měsíc v minutách
const double SEASON_DURATION = 6.5 * MONTH;  // Délka sezóny
// const double OFF_SEASON_DURATION = 5.5 * MONTH;  // Délka mimo sezóny

// Doba kontroly vybavení při vrácení
double CheckTime() {
    // Implementace doby kontroly
    return Exponential(10);  // Exponenciální rozdělení se střední hodnotou 10 minut
}

// Funkce pro domluvený termín
double AgreedTime() {
    // Implementace domluveného termínu
    return Uniform(100, 200);  // Například uniformní rozložení mezi 100 a 200 minutami
}

// Proces sušení vybavení
class EquipmentDrying : public Process {
    int amount;
public:
    EquipmentDrying(int n) : amount(n) {}
    void Behavior() {
        Wait(Uniform(2, 15));
        Leave(equipment, amount);
    }
};

// Proces opravy vybavení
class EquipmentRepair : public Process {
    int amount;
public:
    EquipmentRepair(int n) : amount(n) {}
    void Behavior() {
        Wait(Uniform(7 * 1440, 14 * 1440));  // 1-2 týdny
        Leave(equipment, amount);
    }
};

// Třída představující rezervaci
class Reservation : public Process {
    int requestedEquipment = 1;  // Počet požadovaného vybavení

    void Behavior() {      
        // Čekání do domluveného termínu
        Wait(AgreedTime());
        // Žádost o obsluhu prodavačem
        Seize(seller);
        // Doba obsluhy
        Wait(Exponential(20));  // Exponenciální rozložení se střední hodnotou 20 minut
        // Uvolnění prodavače
        Release(seller);

        // Kontrola dostupnosti vybavení
        Enter(equipment, requestedEquipment);

        // Vypůjčení vybavení
        double borrowTime = Normal(5 * 1440, 1 * 1440);  // Střed 5 dní, sigma 1 den
        if (borrowTime < 0) borrowTime = 0;  // Ensure non-negative time to borrow
        Wait(borrowTime);

        // Vracení vybavení
        Seize(seller);
        // The seller needs to dry the equipment (before checking condition)
        Wait(Uniform(2, 15));  // 2 to 15 minutes
        // Doba kontroly vybavení
        Wait(CheckTime());
        

        // Rozhodnutí o stavu vybavení
        if (Random() < 0.05) {  // 5% pravděpodobnost poškození
            // Uvolnění prodavače
            Release(seller);
            // Vybavení jde do opravy
            (new EquipmentRepair(requestedEquipment))->Activate(); // TODO: přidat že prvně se ještě prodavač zdrží na vysoušení lodi
        } else {
            // Uvolnění prodavače
            Release(seller);
            // Sušení vybavení
            (new EquipmentDrying(requestedEquipment))->Activate(); // TODO: přidat že prvně se ještě prodavač zdrží na vysoušení lodi
        }
    }
};

// Generátor rezervací
class ReservationGenerator : public Event {
    void Behavior() {
        // Sezóna - generátor je aktivní
        double season_end = Time + SEASON_DURATION;
        if (Time < season_end) {
            (new Reservation)->Activate();
            Activate(Time + Exponential(x));
            //Passivate();
        }
    }
};

// Generátor rezervací
// class ReservationGenerator : public Event {
//     void Behavior() {
//         (new Reservation)->Activate();
//         Activate(Time + Exponential(x));
//     }
// };

int main() {
    // Inicializace simulace
    double extra_time = 10 * 24 * 60;  // Extra 10 days for returns after the season ends
    Init(0, SEASON_DURATION + extra_time);  // Simulace aktivní sezóny
    // Aktivace generátoru rezervací
    (new ReservationGenerator)->Activate();
    // Spuštění simulace
    Run();
    seller.Output();
    equipment.Output();
    return 0;
}
