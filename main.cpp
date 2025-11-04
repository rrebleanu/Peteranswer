#include <iostream>
#include <cstring>    // pentru funcția strcspn()
#include <fstream>    // pentru lucrul cu fișiere (ifstream, ofstream)
#include <vector>     // pentru stocarea evenimentelor în vector
#include <string>

// 🧩 bloc condițional pentru compatibilitate Windows/Linux
#ifdef _WIN32
    #include <conio.h>    // pentru _getch() (citire tastatură fără afișare)
#else
    #include <termios.h>
    #include <unistd.h>
    // Implementare alternativă a _getch() pentru Linux/macOS
    char _getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0) return 0;
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        if (tcsetattr(0, TCSANOW, &old) < 0) return 0;
        if (read(0, &buf, 1) < 0) return 0;
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
        return buf;
    }
#endif

#include <thread>     // pentru efecte de întârziere (sleep)
#include <chrono>     // folosit împreună cu thread pentru sleep_for()
#include <ctime>      // pentru funcții legate de timp (ctime)
#include <iomanip>    // pentru formatarea afișărilor (optional)
#include <sstream>    // pentru compunerea șirurilor cu timestamp
#include <limits>     // pentru curățarea bufferului de intrare
using namespace std;

// ===================================================================
// === Clasa CONFIGURATIE ============================================
// ===================================================================
class Configuratie {
    char caracterSecret;
    string mesajMasca;
public:
    Configuratie(char c = '.', const string& mesaj = "Peter please answer the following question")
        : caracterSecret(c), mesajMasca(mesaj) {}

    Configuratie(const Configuratie& other)
        : caracterSecret(other.caracterSecret), mesajMasca(other.mesajMasca) {}

    Configuratie& operator=(const Configuratie& other) {
        if (this != &other) {
            caracterSecret = other.caracterSecret;
            mesajMasca = other.mesajMasca;
        }
        return *this;
    }

    ~Configuratie() = default;

    char getCaracterSecret() const { return caracterSecret; }
    const string& getMesajMasca() const { return mesajMasca; }

    bool incarcaDinFisier(const string& numeFisier) {
        ifstream fin(numeFisier);
        if (!fin.is_open()) return false;

        char c;
        string mesaj;
        fin >> c;
        fin.ignore();
        getline(fin, mesaj);

        if (mesaj.empty()) mesaj = "Peter please answer the following question";

        caracterSecret = c;
        mesajMasca = mesaj;
        return true;
    }

    friend ostream& operator<<(ostream& out, const Configuratie& cfg) {
        out << "[Configuratie] Caracter secret: '" << cfg.caracterSecret
            << "', Mesaj masca: \"" << cfg.mesajMasca << "\"";
        return out;
    }
};

// ===================================================================
// === Clasa ISTORIC =================================================
// ===================================================================
class Istoric {
    vector<string> evenimente;

    string getTimestamp() const {
        time_t now = time(nullptr);
        char buf[26];
#ifdef _WIN32
        ctime_s(buf, sizeof(buf), &now);
#else
        ctime_r(&now, buf);
#endif
        buf[strcspn(buf, "\n")] = 0;
        return string(buf);
    }

public:
    Istoric() = default;
    Istoric(const Istoric& other) : evenimente(other.evenimente) {}
    Istoric& operator=(const Istoric& other) {
        if (this != &other) evenimente = other.evenimente;
        return *this;
    }
    ~Istoric() = default;

    void adaugaEveniment(const string& tip, const string& intrebare, const string& raspuns) {
        ostringstream os;
        os << "[" << getTimestamp() << "] [" << tip << "] Intrebare: '" << intrebare
           << "', Raspuns: '" << raspuns << "'";
        evenimente.push_back(os.str());
    }

    void afiseaza() const {
        cout << "\n=== Istoric Peter Answers ===\n";
        if (evenimente.empty()) {
            cout << "Nicio sesiune inregistrata.\n";
            return;
        }
        for (size_t i = 0; i < evenimente.size(); ++i)
            cout << i + 1 << ". " << evenimente[i] << "\n";
    }

    void salveazaInFisier(const string& numeFisier) const {
        ofstream fout(numeFisier, ios::app);
        if (!fout.is_open()) return;
        for (const auto& ev : evenimente)
            fout << ev << "\n";
        fout.close();
    }
};

// ===================================================================
// === Clasa PETITIE =================================================
// ===================================================================
class Petitie {
    string continut;
    const Configuratie& cfg;

public:
    Petitie(const string& text, const Configuratie& c) : continut(text), cfg(c) {}

    void setContinut(const string& text) { continut = text; }
    const string& getContinut() const { return continut; }

    bool esteValida() const { return !continut.empty() && continut[0] == cfg.getCaracterSecret(); }

    string getRaspuns() const {
        if (esteValida())
            return continut.substr(1);
        else
            return "The petition is wrong again. I'll never answer you";
    }
};

// ===================================================================
// === Clasa SIMULATOR ===============================================
// ===================================================================
class Simulator {
    Configuratie cfg;
    Istoric istoric;

    void eraseLastPrintedChar() {
        cout << '\b' << ' ' << '\b';
        cout.flush();
    }

public:
    Simulator() = default;

    string citesteInputMascat() {
        string input;
        const string& mask = cfg.getMesajMasca();
        size_t maskIndex = 0;
        char c = _getch();

        if (c == cfg.getCaracterSecret()) {
            input.push_back(c);
            while (true) {
                c = _getch();
                if (c == '\r') break;
                if (c == 8) {
                    if (!input.empty()) {
                        input.pop_back();
                        eraseLastPrintedChar();
                        if (maskIndex > 0) maskIndex--;
                    }
                    continue;
                }
                input.push_back(c);
                cout << (maskIndex < mask.size() ? mask[maskIndex++] : '.');
                cout.flush();
                this_thread::sleep_for(chrono::milliseconds(20));
            }
        } else {
            input.push_back(c);
            cout << c;
            while (true) {
                c = _getch();
                if (c == '\r') break;
                if (c == 8) {
                    if (!input.empty()) { input.pop_back(); eraseLastPrintedChar(); }
                    continue;
                }
                input.push_back(c);
                cout << c;
                cout.flush();
            }
        }
        cout << "\n";
        return input;
    }

    void proceseazaPetitie(Petitie& p, const string& intrebare) {
        string raspuns = p.getRaspuns();
        if (p.esteValida())
            istoric.adaugaEveniment("SUCCES", intrebare, raspuns);
        else
            istoric.adaugaEveniment("ESEC", intrebare, "Esec");

        cout << raspuns << "\n";
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
            cout << "Introdu intrebarea: ";
            string intrebare;
            getline(cin, intrebare);

            proceseazaPetitie(p, intrebare);

            cout << "Doriti o noua sesiune? (da/nu): ";
            string rasp;
            getline(cin, rasp);
            if (rasp != "da" && rasp != "Da" && rasp != "DA") break;
        }
    }

    void afiseazaIstoric() const { istoric.afiseaza(); }
    void salveazaIstoric() const { istoric.salveazaInFisier("istoric.txt"); }

    bool incarcaConfiguratie(const string& fisier) { return cfg.incarcaDinFisier(fisier); }

    void afiseazaConfiguratie() const { cout << cfg << "\n"; }
};

// ===================================================================
// === MAIN ===========================================================
// ===================================================================
int main() {
    Simulator simulator;

    if (simulator.incarcaConfiguratie("config.txt"))
        simulator.afiseazaConfiguratie();
    else
        cout << "Nu s-a gasit config.txt. Se foloseste configuratia implicita.\n";

    cout << "\nAlege modul de introducere a datelor:\n"
            "1 = Citire din tastatura.txt\n"
            "2 = Introducere manuala (interactiv)\n"
            "Alege 1 sau 2: ";

    int optiune;
    if (!(cin >> optiune)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        optiune = 1;
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (optiune == 1)
        simulator.ruleazaDinFisier("tastatura.txt");
    else
        simulator.ruleazaDinStdin();

    simulator.afiseazaIstoric();
    simulator.salveazaIstoric();

    cout << "\nIstoricul a fost salvat in fisierul 'istoric.txt'.\n";
    return 0;
}
