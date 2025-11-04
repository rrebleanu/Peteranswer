#include <iostream>
#include <cstring>    // pentru funcția strcspn()
#include <fstream>    // pentru lucrul cu fișiere (ifstream, ofstream)
#include <vector>     // pentru stocarea evenimentelor în vector
#include <string>
#include <conio.h>    // pentru _getch() (citire tastatură fără afișare)
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
// Clasa gestionează setările aplicației — caracterul secret și mesajul
// care apare sub formă de mască atunci când se tastează o petiție secretă.
class Configuratie {
    char caracterSecret;   // caracterul care indică o petiție validă (ex: '.')
    string mesajMasca;     // textul care se afișează mascat în locul tastelor reale
public:
    // Constructor implicit + cu parametri
    Configuratie(char c = '.', const string& mesaj = "Peter please answer the following question")
        : caracterSecret(c), mesajMasca(mesaj) {}

    // Constructor de copiere
    Configuratie(const Configuratie& other)
        : caracterSecret(other.caracterSecret), mesajMasca(other.mesajMasca) {}

    // Operator de copiere (=)
    Configuratie& operator=(const Configuratie& other) {
        if (this != &other) {
            caracterSecret = other.caracterSecret;
            mesajMasca = other.mesajMasca;
        }
        return *this;
    }

    // Destructor (nu e necesar special deoarece nu există resurse dinamice)
    ~Configuratie() = default;

    // Getteri const
    char getCaracterSecret() const { return caracterSecret; }
    const string& getMesajMasca() const { return mesajMasca; }

    // Funcție care încarcă setările dintr-un fișier extern (config.txt)
    bool incarcaDinFisier(const string& numeFisier) {
        ifstream fin(numeFisier);
        if (!fin.is_open()) return false;

        char c;
        string mesaj;
        fin >> c;           // citește caracterul secret
        fin.ignore();       // sare peste spațiu / newline
        getline(fin, mesaj); // citește linia următoare (mesajul mască)

        // dacă fișierul nu conține mesaj, folosim valoarea implicită
        if (mesaj.empty()) mesaj = "Peter please answer the following question";

        caracterSecret = c;
        mesajMasca = mesaj;
        return true;
    }

    // Supraincarcare operator << pentru afișare ușoară
    friend ostream& operator<<(ostream& out, const Configuratie& cfg) {
        out << "[Configuratie] Caracter secret: '" << cfg.caracterSecret
            << "', Mesaj masca: \"" << cfg.mesajMasca << "\"";
        return out;
    }
};

// ===================================================================
// === Clasa ISTORIC =================================================
// ===================================================================
// Această clasă păstrează toate evenimentele (succese sau eșecuri)
// care apar în timpul rulării aplicației.
class Istoric {
    vector<string> evenimente; // vector care reține toate sesiunile

    // Funcție privată care returnează data și ora curentă sub formă de text
    string getTimestamp() const {
        time_t now = time(nullptr);
        char buf[26];
#ifdef _WIN32
        ctime_s(buf, sizeof(buf), &now); // pentru Windows
#else
        ctime_r(&now, buf);              // pentru Linux/Mac
#endif
        buf[strcspn(buf, "\n")] = 0;     // elimină caracterul '\n' de la final
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

    // Adaugă un nou eveniment în lista istorică
    void adaugaEveniment(const string& tip, const string& intrebare, const string& raspuns) {
        ostringstream os;
        os << "[" << getTimestamp() << "] [" << tip << "] Intrebare: '" << intrebare
           << "', Raspuns: '" << raspuns << "'";
        evenimente.push_back(os.str());
    }

    // Afișează toate evenimentele în consolă
    void afiseaza() const {
        cout << "\n=== Istoric Peter Answers ===\n";
        if (evenimente.empty()) {
            cout << "Nicio sesiune inregistrata.\n";
            return;
        }
        for (size_t i = 0; i < evenimente.size(); ++i)
            cout << i + 1 << ". " << evenimente[i] << "\n";
    }

    // Salvează istoricul în fișier (adaugă la final)
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
// Reprezintă un mesaj trimis către Peter. Poate fi valid (dacă începe
// cu caracterul secret) sau invalid (altfel).
class Petitie {
    string continut;          // textul petiției
    const Configuratie& cfg;  // referință la configurație (pentru acces la caracterul secret)

public:
    Petitie(const string& text, const Configuratie& c) : continut(text), cfg(c) {}

    void setContinut(const string& text) { continut = text; }
    const string& getContinut() const { return continut; }

    // Petiția este validă doar dacă primul caracter este cel secret
    bool esteValida() const { return !continut.empty() && continut[0] == cfg.getCaracterSecret(); }

    // Răspunsul generat de Peter
    string getRaspuns() const {
        if (esteValida())
            return continut.substr(1); // răspunsul este textul după caracterul secret
        else
            return "The petition is wrong again. I'll never answer you";
    }
};

// ===================================================================
// === Clasa SIMULATOR ===============================================
// ===================================================================
// Este clasa principală care controlează toată logica programului.
class Simulator {
    Configuratie cfg;   // configurația curentă (caracter secret + mesaj mască)
    Istoric istoric;    // istoricul sesiunilor

    // Mică funcție ajutătoare pentru a șterge ultimul caracter afișat (la backspace)
    void eraseLastPrintedChar() {
        cout << '\b' << ' ' << '\b';
        cout.flush();
    }

public:
    Simulator() = default;

    // Funcție care citește o petiție de la tastatură în mod mascat (fără să se vadă textul)
    string citesteInputMascat() {
        string input;
        const string& mask = cfg.getMesajMasca(); // textul de mască
        size_t maskIndex = 0;
        char c = _getch(); // citim primul caracter fără să fie afișat

        // Dacă primul caracter este cel secret => afișăm mască
        if (c == cfg.getCaracterSecret()) {
            input.push_back(c);
            while (true) {
                c = _getch();
                if (c == '\r') break; // Enter
                if (c == 8) { // Backspace
                    if (!input.empty()) {
                        input.pop_back();
                        eraseLastPrintedChar();
                        if (maskIndex > 0) maskIndex--;
                    }
                    continue;
                }
                input.push_back(c);
                cout << (maskIndex < mask.size() ? mask[maskIndex++] : '.'); // afișăm mască
                cout.flush();
                this_thread::sleep_for(chrono::milliseconds(20)); // mic efect vizual
            }
        } else {
            // Dacă nu e caracter secret, afișăm direct ce se tastează
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

    // Procesează o petiție și afișează răspunsul lui Peter
    void proceseazaPetitie(Petitie& p, const string& intrebare) {
        string raspuns = p.getRaspuns();
        if (p.esteValida())
            istoric.adaugaEveniment("SUCCES", intrebare, raspuns);
        else
            istoric.adaugaEveniment("ESEC", intrebare, "Esec");

        cout << raspuns << "\n";
    }

    // Citește datele dintr-un fișier de intrare (tastatura.txt)
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

    // Modul interactiv — utilizatorul tastează manual datele
    void ruleazaDinStdin() {
        while (true) {
            cout << "\nIntrodu petitia: ";
            string textPetitie = citesteInputMascat(); // citire secretă
            Petitie p(textPetitie, cfg);

            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Introdu intrebarea: ";
            string intrebare;
            getline(cin, intrebare);

            proceseazaPetitie(p, intrebare);

            // întreabă dacă utilizatorul vrea o nouă sesiune
            cout << "Doriti o noua sesiune? (da/nu): ";
            string rasp;
            getline(cin, rasp);
            if (rasp != "da" && rasp != "Da" && rasp != "DA") break;
        }
    }

    // Afișare și salvare istoric
    void afiseazaIstoric() const { istoric.afiseaza(); }
    void salveazaIstoric() const { istoric.salveazaInFisier("istoric.txt"); }

    // Încarcă configurația din fișier extern
    bool incarcaConfiguratie(const string& fisier) { return cfg.incarcaDinFisier(fisier); }

    void afiseazaConfiguratie() const { cout << cfg << "\n"; }
};

// ===================================================================
// === MAIN ===========================================================
// ===================================================================
// Punctul de intrare al programului — gestionează meniul și modul de rulare
int main() {
    Simulator simulator;

    // Încearcă să încarce setările din fișierul config.txt
    if (simulator.incarcaConfiguratie("config.txt"))
        simulator.afiseazaConfiguratie();
    else
        cout << "Nu s-a gasit config.txt. Se foloseste configuratia implicita.\n";

    // Meniu pentru alegerea modului de introducere a datelor
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

    // Rulează în funcție de alegerea utilizatorului
    if (optiune == 1)
        simulator.ruleazaDinFisier("tastatura.txt");
    else
        simulator.ruleazaDinStdin();

    // La final: afișează și salvează istoricul
    simulator.afiseazaIstoric();
    simulator.salveazaIstoric();

    cout << "\nIstoricul a fost salvat in fisierul 'istoric.txt'.\n";
    return 0;
}
