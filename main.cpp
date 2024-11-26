#include <simlib.h>
#include <iostream>
#include <algorithm>

Facility seller("Obsluha/Zaměstnanec"); // Obsluha s kapacitou 1
Store equipment(40);                    // Sklad vybavení se 40 kusy
double x = 720;                         // Střední hodnota mezi rezervacemi (v minutách) // TODO: bude jako parametr

// Délka sezóny a mimo sezóny v minutách
const double MONTH = 30 * 24 * 60;              // Jeden měsíc v minutách
const double SEASON_DURATION = 6.5 * MONTH;     // Délka sezóny
const double OFF_SEASON_DURATION = 5.5 * MONTH; // Délka mimo sezóny

// Vytvoření nové rezervace s exponenciálním rozložením se středem x
double ReservationInterval() {
    return Exponential(x);
}

// Simulování domluveného termínu rezervace
double AgreedTime() {
    return Uniform(7 * 24 * 60, 30 * 24 * 60);  // 7-30 dní v minutách
}

// Simulování doby nafukování + poučení o používání
double InflatingTime() {
    return Uniform(3, 8);  // 3 to 8 minutes
}

// Simulování doby předvedení + vyfouknutí a sbalení
double DemonstratingTime() {
    return Exponential(10);  // Exponenciální rozložení se střední hodnotou 10 minut
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
double CheckTime() {
    // Implementace doby kontroly
    return Uniform(5, 10);  // 5 to 10 minutes
}

// Simulování doby vysoušení vybavení
double DryingTime() {
    return Uniform(2, 15);  // 2 to 15 minutes
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
int RandomEquipmentAmount() {
    int amount = int(Exponential(2)) + 1;  // Průměrně 2, minimálně 1
    return (amount > 30) ? 30 : amount;    // Maximálně 30
}

// Proces sušení vybavení
class EquipmentDrying : public Process {
    int amount;
public:
    EquipmentDrying(int n) : amount(n) {}
    void Behavior() {
        Wait(DryingTime());
        Leave(equipment, amount);
    }
};

// Proces opravy vybavení
class EquipmentRepair : public Process {
    int amount;
public:
    EquipmentRepair(int n) : amount(n) {}
    void Behavior() {
        Wait(RepairTime());  // 1-2 týdny
        Leave(equipment, amount);
    }
};

// Třída představující rezervaci
class Reservation : public Process {
    void Behavior() {      

        // Čekání do domluveného termínu
        Wait(AgreedTime());
        
        // Kontrola dostupnosti vybavení
        int requestedEquipment = RandomEquipmentAmount(); // Náhodný počet vybavení (1-30, průměrně 2)
        Enter(equipment, requestedEquipment); // TODO: přidat nějakou pravděpodobnost, že zákazník odejde, pokud není dostatek vybavení po nějakou dobu

        // Žádost o obsluhu prodavačem (zákazník přichází, čeká na obsluhu)
        Seize(seller);

        // Obsluha zákazníka:
        // Krok 1: Nafouknutí lodě a poučení o používání
        Wait(InflatingTime());
        // Krok 2: Předvedení lodě, vyfouknutí a sbalení
        Wait(DemonstratingTime());
        // Krok 3: Předání zbytku vybavení, podpis smlouvy, doplacení, ...
        Wait(HandoverTime());
        // Nakonec dojde k uvolnění prodavače
        Release(seller);

        // Vypůjčení vybavení
        // Čekání po dobu výpůjčky
        Wait(BorrowTime());

        // Vracení vybavení
        // Musí se čekat na uvolnění obsluhy
        Seize(seller);

        // Kontrola stavu vybavení
        Wait(CheckTime());        

        // Rozhodnutí o stavu vybavení (5% šance, že bude poškozené)
        if (Random() < 0.05) {  // 5% pravděpodobnost poškození
            // Vyřizování škod se zákazníkem
            Wait(DamageSettlementTime());
            // Uvolnění obsluhy
            Release(seller);
            // Vybavení jde do opravy
            (new EquipmentRepair(requestedEquipment))->Activate();
            // Leave(equipment, requestedEquipment);
        } else {
            // Sušení vybavení
            (new EquipmentDrying(requestedEquipment))->Activate();
            // Uvolnění obsluhy
            Release(seller);
            // Vybavení je v pořádku
            // Leave(equipment, requestedEquipment);
        }
    }
};

// Generátor rezervací
class ReservationGenerator : public Event {
    void Behavior() {
        // Sezóna - generátor je aktivní
        if (Time < SEASON_DURATION) { // TODO: zkontrolovat, jestli tu nemá být místo if while cyklus
            (new Reservation)->Activate();
            Activate(Time + ReservationInterval());
        }
    }
};

int main() {
    // Inicializace simulace
    double simulation_duration = SEASON_DURATION + OFF_SEASON_DURATION;
    Init(0, simulation_duration);  // Simulace aktivní sezóny
    // Aktivace generátoru rezervací
    (new ReservationGenerator)->Activate();
    // Spuštění simulace
    Run();
    seller.Output();
    equipment.Output();
    return 0;
}
