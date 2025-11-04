#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
// #include <conio.h> și alte librării non-standard sunt eliminate pentru stabilitate maximă

using namespace std;

// ================= Clasa Configuratie =================
class Configuratie {
    const char caracterSecret;
    const string mesajMasca;
public:
    Configuratie(char c = '.', const string& mesaj = "Peter please answer the following question")
        : caracterSecret(c), mesajMasca(mesaj) {}
    char getCaracterSecret() const { return caracterSecret; }
    const string& getMesajMasca() const { return mesajMasca; }

    friend ostream& operator<<(ostream& out, const Configuratie& cfg) {
        out << "Caracter secret: " << cfg.caracterSecret
            << ", Mesaj masca: " << cfg.mesajMasca;
        return out;
    }
};

// ================= Clasa Istoric =================
class Istoric {
    vector<string> evenimente;
public:
    void adaugaEveniment(const string& ev) { evenimente.push_back(ev); }

    void afiseaza() const {
        cout << "\n=== Istoric Peter Answers ===\n";
        if (evenimente.empty()) { cout << "Nicio sesiune inregistrata.\n"; return; }
        for (size_t i = 0; i < evenimente.size(); ++i)
            cout << i + 1 << ". " << evenimente[i] << "\n";
    }

    friend ostream& operator<<(ostream& out, const Istoric& ist) {
        for (const auto& ev : ist.evenimente)
            out << ev << "\n";
        return out;
    }
};

// ================= Clasa Petitie =================
class Petitie {
    string continut;
    Configuratie cfg;
public:
    Petitie(const string& text = "", const Configuratie& c = Configuratie())
        : continut(text), cfg(c) {}

    void setContinut(const string& text) { continut = text; }
    const string& getContinut() const { return continut; }

    bool esteValida() const { return !continut.empty() && continut[0] == cfg.getCaracterSecret(); }

    string getRaspuns() const {
        if (esteValida()) return continut.substr(1);
        else return "Imi pare rau, nu pot raspunde la intrebarea ta.";
    }

    friend ostream& operator<<(ostream& out, const Petitie& p) {
        out << "Petitie: " << p.continut
            << " (valid: " << (p.esteValida() ? "da" : "nu") << ")";
        return out;
    }
};

// ================= Clasa Simulator =================
class Simulator {
    Configuratie cfg;
    Istoric istoric;

public:
    Simulator() : cfg(), istoric() {}

    string citesteInput() {
        string input;
        // Folosim getline pentru a citi o linie completa
        getline(cin, input);
        return input;
    }

    void proceseazaPetitie(Petitie& p, const string& intrebare) {
        string raspuns = p.getRaspuns();

        // Simulare gandire
        cout << "\nPeter se gandeste..." << flush;
        this_thread::sleep_for(chrono::milliseconds(600));
        cout << "\r" << string(60, ' ') << "\r";

        if (p.esteValida())
            istoric.adaugaEveniment("[SUCCES] Intrebare: '" + intrebare + "', Raspuns: '" + raspuns + "'");
        else
            istoric.adaugaEveniment("[ESEC] Intrebare: '" + intrebare + "', Raspuns: Esec");

        cout << raspuns << "\n";
    }

    void ruleazaDinFisier(const string& fisier) {
        ifstream fin(fisier);
        if (!fin.is_open()) { cout << "Nu s-a putut deschide " << fisier << "\n"; return; }

        string liniePetitie, linieIntrebare;
        while (getline(fin, liniePetitie) && getline(fin, linieIntrebare)) {
            Petitie p(liniePetitie, cfg);
            cout << "\n--- Sesiune din fisier ---\n";
            cout << "Petitie: " << liniePetitie << "\nIntrebare: " << linieIntrebare << "\n";
            proceseazaPetitie(p, linieIntrebare);
        }
        fin.close();
    }

    void ruleazaDinStdin() {
        while (true) {
            cout << "\nIntrodu petitia: ";
            string textPetitie = citesteInput();
            Petitie p(textPetitie, cfg);

            cout << "Introdu intrebarea reala: " << flush;
            string intrebare;
            // Folosim direct getline pentru a citi intrebarea
            getline(cin, intrebare);

            proceseazaPetitie(p, intrebare);

            cout << "Doriti o noua sesiune? (da/nu): ";
            string rasp;
            getline(cin, rasp);
            if (rasp != "da" && rasp != "Da" && rasp != "DA") break;
        }
    }

    void afiseazaIstoric() const { cout << istoric; }
};

// ================= Main =================
// ================= Main (CORECTAT PENTRU STABILITATE MSAN) =================
int main() {
    Simulator simulator;

    cout << "Alege modul de introducere a datelor:\n";
    cout << "1 = Citire din tastatura.txt\n";
    cout << "2 = Introducere manuala (interactiv)\n";
    cout << "Alege 1 sau 2: ";

    int optiune = 1; // Presupunem implicit 1

    // Încercăm să citim un singur număr. Acesta este cel mai stabil mod.
    if (!(cin >> optiune)) {
        // Dacă citirea eșuează (cum se întâmplă la MSAN), forțăm opțiunea 1 și curățăm.
        cin.clear();
        optiune = 1;
    }

    // Curățăm buffer-ul rămas înainte de a trece la logica programului
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (optiune == 1) {
        simulator.ruleazaDinFisier("tastatura.txt");
    } else {
        simulator.ruleazaDinStdin();
    }

    simulator.afiseazaIstoric();
    return 0;
}