#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <conio.h>
#include <thread>
#include <chrono>
#include <limits>

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
        out << "Caracter secret: " << cfg.caracterSecret << ", Mesaj masca: " << cfg.mesajMasca;
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
    Petitie(const string& text = "", const Configuratie& c = Configuratie()) : continut(text), cfg(c) {}
    void setContinut(const string& text) { continut = text; }
    const string& getContinut() const { return continut; }
    bool esteValida() const { return !continut.empty() && continut[0] == cfg.getCaracterSecret(); }
    string getRaspuns() const {
        if (esteValida()) return continut.substr(1);
        else return "The petition is wrong again. I'll never answer you";
    }
    friend ostream& operator<<(ostream& out, const Petitie& p) {
        out << "Petitie: " << p.continut << " (valid: " << (p.esteValida() ? "da" : "nu") << ")";
        return out;
    }
};

// ================= Clasa Simulator =================
class Simulator {
    Configuratie cfg;
    Istoric istoric;
    void eraseLastPrintedChar() { cout << '\b' << ' ' << '\b'; cout.flush(); }
public:
    Simulator() : cfg(), istoric() {}
    string citesteInputMascat() {
        string input;
        const string& mask = cfg.getMesajMasca();
        size_t maskIndex = 0;
        char c = _getch();

        if (c == cfg.getCaracterSecret()) {
            input.push_back(c);
            while (true) {
                c = _getch();
                if (c == 0 || c == -32) { _getch(); continue; }
                if (c == '\r') break;
                if (c == 8) { if (!input.empty()) { input.pop_back(); eraseLastPrintedChar(); if(maskIndex>0) maskIndex--; } continue; }
                input.push_back(c);
                if (maskIndex < mask.size()) { cout << mask[maskIndex]; maskIndex++; }
                else cout << '.';
                cout.flush();
                this_thread::sleep_for(chrono::milliseconds(20));
            }
        } else {
            input.push_back(c);
            cout << c;
            while (true) {
                c = _getch();
                if (c == 0 || c == -32) { _getch(); continue; }
                if (c == '\r') break;
                if (c == 8) { if (!input.empty()) { input.pop_back(); eraseLastPrintedChar(); } continue; }
                input.push_back(c); cout << c; cout.flush();
            }
        }
        cout << "\n";
        return input;
    }

    void proceseazaPetitie(Petitie& p, const string& intrebare) {
        string raspuns = p.getRaspuns();
        if (p.esteValida()) istoric.adaugaEveniment("[SUCCES] Intrebare: '" + intrebare + "', Raspuns: '" + raspuns + "'");
        else istoric.adaugaEveniment("[ESEC] Intrebare: '" + intrebare + "', Raspuns: Esec");
        cout << raspuns << "\n"; // afișează doar răspunsul efectiv
    }

    void ruleazaDinFisier(const string& fisier) {
        ifstream fin(fisier);
        if (!fin.is_open()) { cout << "Nu s-a putut deschide " << fisier << "\n"; return; }
        string liniePetitie, linieIntrebare;
        while (getline(fin, liniePetitie) && getline(fin, linieIntrebare)) {
            Petitie p(liniePetitie, cfg);
            cout << "Petitie: " << liniePetitie << "\nIntrebare: " << linieIntrebare << "\n";
            proceseazaPetitie(p, linieIntrebare);
        }
        fin.close();
    }

    void ruleazaDinStdin() {
        while (true) {
            cout << "\nIntrodu petitia: ";
            string textPetitie = citesteInputMascat();
            Petitie p(textPetitie, cfg);

            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Introdu intrebarea reala: " << flush;
            string intrebare;
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
int main() {
    Simulator simulator;
    cout << "Alege modul de introducere a datelor:\n1 = Citire din tastatura.txt\n2 = Introducere manuala (interactiv)\nAlege 1 sau 2: ";
    int optiune;
    if (!(cin >> optiune)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); optiune = 1; }
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // curata buffer
    if (optiune == 1) simulator.ruleazaDinFisier("tastatura.txt");
    else simulator.ruleazaDinStdin();
    simulator.afiseazaIstoric();
    return 0;
}