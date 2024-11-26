#include <simlib.h>
#include <iostream>
#include <algorithm>

// deklarace obslužných linek
// deklarace tříd popisujících procesy a události
// popis simulačního experimentu ve funkci main()

Facility seller("Obsluha/Zaměstnanec");           // Obsluha s kapacitou 1
Store equipment(100);      // Sklad vybavení se 100 kusy
double x = 120;             // Střední hodnota mezi rezervacemi (v minutách) // TODO: bude jako parametr
double extra_time = 30 * 24 * 60;  // Extra 30 days for returns after the season ends

// Délka sezóny a mimo sezóny v minutách
const double MONTH = 30 * 24 * 60;           // Jeden měsíc v minutách
const double SEASON_DURATION = 6.5 * MONTH;  // Délka sezóny
// const double OFF_SEASON_DURATION = 5.5 * MONTH;  // Délka mimo sezóny

// Vytvoření nové rezervace s exponenciálním rozložením se středem x
double ReservationInterval() {
    return Exponential(x);
}

// Simulování domluveného termínu (exponenciálně např. 2 týdny dopředu)
double AgreedTime() {
    return Exponential(14 * 24 * 60);  // Exponenciální rozložení se střední hodnotou 14 dní
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
        // Enter(equipment, amount);
        // TODO: vybavení je sušené a je znovu k dispozici (vrátí se do skladu), zkontrolovat, jestli se opravdu vrátilo do skladu
    }
};

// Proces opravy vybavení
class EquipmentRepair : public Process {
    int amount;
public:
    EquipmentRepair(int n) : amount(n) {}
    void Behavior() {
        Wait(RepairTime());  // 1-2 týdny
        // Leave(equipment, amount);
        Enter(equipment, amount); // vybavení je opravené a je znovu k dispozici (vrátí se do skladu)
    }
};

// Třída představující rezervaci
class Reservation : public Process {
    // int requestedEquipment = 1;  // Počet požadovaného vybavení

    void Behavior() {      
        // Čekání do domluveného termínu
        Wait(AgreedTime());
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
        // Kontrola dostupnosti vybavení
        int requestedEquipment = RandomEquipmentAmount(); // Náhodný počet vybavení (1-30, průměrně 2)
        Enter(equipment, requestedEquipment);
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
            // Vybavení jde do opravy
            Leave(equipment, requestedEquipment);
            (new EquipmentRepair(requestedEquipment))->Activate();
        } else {
            // Vybavení je v pořádku
            Leave(equipment, requestedEquipment);
            // Sušení vybavení
            (new EquipmentDrying(requestedEquipment))->Activate();
        }

        // Uvolnění obsluhy
        Release(seller);
    }
};

// Generátor rezervací
class ReservationGenerator : public Event {
    void Behavior() {
        // Sezóna - generátor je aktivní
        double season_end = Time + SEASON_DURATION;
        if (Time < season_end) { // TODO: zkontrolovat, jestli tu nemá být místo if while cyklus
            (new Reservation)->Activate();
            Activate(Time + ReservationInterval());
            //Passivate();
        }
    }
};

// Generátor rezervací
// class ReservationGenerator : public Event {
//     void Behavior() {
//         (new Reservation)->Activate();
//         Activate(Time + ReservationInterval());
//     }
// };

int main() {
    // Inicializace simulace
    Init(0, SEASON_DURATION + extra_time);  // Simulace aktivní sezóny
    // Aktivace generátoru rezervací
    (new ReservationGenerator)->Activate();
    // Spuštění simulace
    Run();
    seller.Output();
    equipment.Output();
    return 0;
}
