#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <random>
#include <chrono>


using namespace std;

struct Punkt; 
class Bee;   

//! Lista punktów wczytanych przez program
std::vector<Punkt> punkty;
//! Lista pszczół w roju
std::vector<Bee> pszczoly;
//! Lista punktów w kolejności odpowiadającej najlepszemu znanemu rozwiązaniu
vector<int> najlepszeRozwiazanie;
//! Długość najkrótszego znanego rozwiązania
float najkrotszaDroga = (float)INT_MAX;
//! Tablica dwu wymiarowa w której zawarte są dystanse między poszczególnymi punktami
std::vector<std::vector<float>>  tablicaDystansow;
//! Nazwa pliku w którym znajduje się lista punktów
std::string nazwaPliku;
//! Ilość pszczół w roju
int iloscPszczol;
//! Procent pszczół z zachowaniem obserwatora
int procentZwiadowcow;
//! Procent pszczół z zachowaniem zbieracza
int procentZbieraczy;
//! Procent pszczół z zachowaniem zwiadowcy
int procentSkautow;

//! Limit prób mutacji rozwiązania jakie podejmują pszczoły zbieracze
int limitIteracji;

//! Limit cykli tańca
int cykle;



//! Struktura każdego punktu w trasie
struct Punkt {
public:
    float X;
    float Y;
};

//! Klasa pszczoły
class Bee {
public:
    //! Przyporządkowana rola (zachowanie) pszczoły możliwe dane[F,O,S]
    char rola = ' '; 
    //! Każda pszczoła zna wszystkie punkty  we własnej trasie i przechowuje je w tej zmiennej
    std::vector<int> trasa; 
    //! Dystans posiadanej przez pszczołę trasy
    double dystans =0;
    //! Licznik kolejnych iteracji w których pszczoła wskazywała na to samo rozwiązanie
    int iterator=0;     
};

//! Funkcja wypisująca informacje o konkretnej pszczole
void infoPszczola(Bee a);

//! Funkcja wczytująca plik z punktami
/*!
 \param nazwaPliku nazwa pliku z którego wczytywana jest liczba punktów.
*/
void wczytywaniePunktow(std::string nazwaPliku) {

    std::ifstream plik(nazwaPliku);
    if (!plik) {
        cout << endl << "Brak pliku";
        throw("Brak pliku");
    }
    float x, y,d;

    while (plik >>d>> x >> y) {
        Punkt tempPunkt;
        tempPunkt.X = x;
        tempPunkt.Y = y;
        punkty.push_back(tempPunkt);
    }

 /*   
    for (auto x : punkty) {
        cout << x.X << " " << x.Y << endl;
    }*/

}


//! Funkcja obliczająca dystans pomiędzy dwoma punktami
/*!
 \param a id punkt na globalnej liście
 \param b id punktu na globalnej liście
 \param pnkty tablica punktów wczytanych przez program
*/
float dystansMiedzyPunktami(int a,int b, std::vector<Punkt> pnkty) {
    float ax, ay, bx, by, dystans;

    ax = pnkty.at(a).X;
    ay = pnkty.at(a).Y;

    bx = pnkty.at(b).X;
    by = pnkty.at(b).Y;

    dystans = sqrt((ax - bx)* (ax - bx) + (ay - by)* (ay - by));

    return dystans;
}

//! Oblicza dystans pomiędzy wszytskimi punktami w tabeli i zapisuje je do tabeli dystansów
void poszczegolneDystanse() {
    int liczbaWierszy = punkty.size();
    cout << endl << "Liczba wszystkch punktow wprowadzonych do systemu: " << liczbaWierszy;
    for (int i = 0; i < liczbaWierszy; i++) {
        tablicaDystansow.push_back(vector<float>(0));
    }

    for (int i = 0; i < liczbaWierszy; i++) {
        for (int j = 0; j < liczbaWierszy; j++) {
            tablicaDystansow[i].push_back(0);
            //tablicaDystansow[i][j] = dystansMiedzyPunktami(i, j, punkty);
        }
    }

    for (int i = 0; i < liczbaWierszy; i++) {
        for (int j = 0; j < liczbaWierszy; j++) {           
            tablicaDystansow[i][j] = dystansMiedzyPunktami(i, j, punkty);
        }
    }

}
//! Wypisuje wczytane punktu oraz tablicę dystansów.
void wypiszPunktyIDystanse() {

    cout << endl << "Wprowadzone punkty do systemu i ich numeracja";
    for (int i = 0; i < punkty.size(); i++) {
        cout << endl << i << ". " << punkty[i].X << " " << punkty[i].Y;
    }
    cout << endl << "Dystanse poszczegolnych punktow" << endl;

    for (int i = 0; i < punkty.size(); i++) {
        for (int j = 0; j < punkty.size(); j++) {
            cout << "\t" << tablicaDystansow[i][j];
        }
        cout << endl;
    }


}
//! Wypisuje aktualnie posiadane trasy przez każdą pszczołę w roju.
void wypiszTrasyroju() {

    cout << endl << "Trasy roju" << endl;
    for (auto x : pszczoly) {
        cout << endl;
        for (int y : x.trasa) {
            cout << " " << y;
        }
        cout << " - " << x.rola << x.dystans;
    }
}


//! Oblicza dystans rozwiązania jakie posiada dana pszczoła.
float dlugoscSciezki(vector<int> a) {
    float dlugosc = 0;
    int punktA, punktB;

    for (int i = 0; i < punkty.size()-1; i++) {
        punktA = a[i];
        punktB = a[i + 1];

        dlugosc += tablicaDystansow[punktA][punktB];
    }

    dlugosc += tablicaDystansow[a[punkty.size() - 1]][a[0]];

    return dlugosc;
}

//! Inicjalizacja roju pszczół za pomocą losowej ścieżki.
void inicjalizacjaRoju(int iloscPszczol) {    
    for (int i = 0; i < iloscPszczol; i++) {
        Bee temp;
        for (int i = 0; i < punkty.size(); i++) {
            temp.trasa.push_back(i);
        }                

        temp.dystans = dlugoscSciezki(temp.trasa);
        temp.iterator = 0;

        pszczoly.push_back(temp);
    }
}


//! Wprowadza podział roju na role według parametrów  (obserwator, zbieracz i zwiadowca).
void podzialRoju() {
    int liczbaPszczol = pszczoly.size();
    float lZwiad = float(procentZwiadowcow) / 100;
    float lZbier = float(procentZwiadowcow) / 100;
    
    lZwiad = floor(lZwiad * liczbaPszczol);
    lZbier = floor(lZbier * liczbaPszczol);

    for (int i = 0; i < lZwiad; i++) {
        pszczoly[i].rola = 'O'; 
    }

    for (int i = lZwiad; i < liczbaPszczol; i++) {
        pszczoly[i].rola = 'F';
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(pszczoly[i].trasa.begin(), pszczoly[i].trasa.end(), default_random_engine(seed));        
       
        pszczoly[i].dystans = dlugoscSciezki(pszczoly[i].trasa);        
    }    
}

//! Mutowanie ścieżek po przez zamianę kolejności dwóch losowych punktów.
vector<int> mutujSciezke(Bee *a) {
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> distrib(0, punkty.size() - 2);

    int temp = 0;
    int numerPunktuDoZamiany = 0;
    numerPunktuDoZamiany = distrib(mt);
    /*if (numerPunktuDoZamiany > 4) {
        throw("XD");
    }*/
    vector<int> zmutowana;

    for (auto x : a->trasa) {
        zmutowana.push_back(x);
    }

    temp = zmutowana[numerPunktuDoZamiany];
    zmutowana[numerPunktuDoZamiany] = zmutowana[numerPunktuDoZamiany +1];;
    zmutowana[numerPunktuDoZamiany +1] = temp;
    
    return zmutowana;
}

//! Eksploatacja znanych rozwiązań przez pszczoły zbieracze.
/*!   
    Dopóki dana pszczoła nie osiągnie limitu iteracji będzie starała się po przez mutowanie ścieżki odnaleźć lepszą.
    Jeżeli jej to się nie uda, pszczoła zostaje zwiadowcą i porzuca dotychczasowe rozwiązanie.       
*/
void zwiad(Bee *a) {

    vector<int> zmutowana = mutujSciezke(a);
    float nowaDlugosc = dlugoscSciezki(zmutowana);

    if (nowaDlugosc < a->dystans) {
        a->trasa = zmutowana;
        a->dystans = nowaDlugosc;
        a->iterator = 0;

    }
    else {
        if (a->iterator >= limitIteracji) {
            a->rola = 'S';
        }
        else {
            a->iterator++ ; 
        }
    }
}

//! Generacja nowej losowej trasy dla pszczoły
void szukajNowejSciezki(Bee a){
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(a.trasa.begin(), a.trasa.end(), default_random_engine(seed));
    a.dystans = dlugoscSciezki(a.trasa);
    a.iterator = 0;
    
    a.rola = 'F';    
}


//! Część funkcji sortującej 
bool sortowanie(vector<float> a,vector<float> b) {
    return a[1] > b[1];
}

//! Funkcja przydziela najlepszą znaną trasę pszczole obserwatorowi w celu eksploracji.
 vector<int> kopiujNajlepszaTrase(Bee a) {
    vector<int> temp;

    for (auto x : a.trasa) {
        temp.push_back(x);
    }
       
    return temp;
}

 //! Główna pętla programu ralizująca komunikację.
 /*!
    W tej funkcji każda pszczoła z zachowaniem zbieracz eksploatuje własne rozwiązanie, czyli po  przez mutację próbuje znaleźć lepsze.
    Jeśli jej się to uda i osiągnięte roziązanie jest lepsze od globalnie najlepszego to zostaje ono podmienione.

    Wszystkie dotychczasowe rozwiązania z aktualnego cyklu są zapamiętywane a następnie ustalony procent najgorszych rozwiązań staje się zwiadowcami. 
 */
void taniec(){
    vector<vector<float>> dotychczasoweRozwiazania;
    
    
    for (int i = 0; i < (int)pszczoly.size(); i++) {
        //infoPszczola(pszczoly[i]);
        if (pszczoly[i].rola == 'F') {
            zwiad(&pszczoly[i]);
            if (najkrotszaDroga > pszczoly[i].dystans) {                
               najlepszeRozwiazanie  =  kopiujNajlepszaTrase(pszczoly[i]);

                najkrotszaDroga = pszczoly[i].dystans;

                cout << endl << "Nowa najlepsza trasa pszczola zbieracz id: " << i<< "\t\t\t[F]";
                infoPszczola(pszczoly[i]);
            }

            dotychczasoweRozwiazania.push_back(vector<float>(0));

            dotychczasoweRozwiazania[dotychczasoweRozwiazania.size() - 1].push_back(i);            
            dotychczasoweRozwiazania[dotychczasoweRozwiazania.size() - 1].push_back(pszczoly[i].dystans);
            
        
        }
        else {
            if (pszczoly[i].rola == 'S') {
                szukajNowejSciezki(pszczoly[i]);
                
            }
        }        
    }
    

    sort(dotychczasoweRozwiazania.begin(), dotychczasoweRozwiazania.end(), sortowanie);

    float lSkautow = (float)procentSkautow / 100;
    lSkautow = floor(lSkautow * dotychczasoweRozwiazania.size());    
    
    
    for (int i = 0; i < (int)lSkautow; i++) {
        int idPszcz = dotychczasoweRozwiazania[i][0];
        pszczoly[idPszcz].rola = 'S';
    }

}

//! Kopiowanie trasy pszczoły
vector<int> kopiujTrase(vector<int> a) {
    vector<int> temp;

    for (auto x : a) {
        temp.push_back(x);
    }
  
    return temp;
}


//! Funkcja zwołuje wszystkie pszczoły obserwatorów, aby te zaczęły eksplorację najlepszego znanego rozwiązania
void wezwanieDoNajlepszegoRozwiazania(vector<Bee> pszczoly) {
    for (auto x : pszczoly) {
        if (x.rola == 'O') {
            
            x.trasa = kopiujTrase(najlepszeRozwiazanie);            
            

            x.trasa = mutujSciezke(&x);
            x.dystans = dlugoscSciezki(x.trasa);
            if (x.dystans < dlugoscSciezki(najlepszeRozwiazanie)) {

                cout << endl << "Nowa najlepsza trasa pszczola zwiadowca \t\t\t[O]"  ;
                infoPszczola(x);


                //najlepszeRozwiazanie = x.trasa;
                najlepszeRozwiazanie = kopiujNajlepszaTrase(x);
                najkrotszaDroga = x.dystans;
            }

        }        

    }


}

//! Funkcja udostępnia informację o pszczole
void infoPszczola(Bee a) {
    cout << endl << "> Rola: " << a.rola << " Dystans: " << a.dystans << " iteracja: " << a.iterator<<" Trasa: [";
    for (auto x : a.trasa) {
        cout << x << " > ";
    }
    cout << "]";
}


int main()
{

    //std::string nazwaPliku = "ali535.txt";
    //procentZbieraczy = 40; // w % 
    //procentZwiadowcow = 60; 
    //procentSkautow = 20;

    //limitIteracji = 100; // Limit mutacji trasy dla pojedyńczej pszczoły
    //cykle = 500;        // Limit cykli algorytmu 



    // iloscPszczol = 2000;

    
    while (1) {

        std::cout << "Pliku do zaladowania (w formacie nazwa.txt): ";
        getline(std::cin, nazwaPliku);

        std::cout << "Liczba pszczol w roju: ";
        std::cin >> iloscPszczol;
        std::cout<<std::endl;

        std::cout << "Liczba zbieraczy w %: ";
        std::cin >> procentZbieraczy;
        std::cout << std::endl;
        
        std::cout << "Liczba obserwatorow w %: ";
        std::cin >> procentZwiadowcow;
        std::cout << std::endl;

        if ((100 - procentZbieraczy) < procentZwiadowcow) {
            procentZwiadowcow = 100 - procentZbieraczy;
        }


        std::cout << "Liczba zwiadowcow w %: ";
        std::cin >> procentSkautow;
        std::cout << std::endl;

        std::cout << "Limit iteracji: ";
        std::cin >> limitIteracji;
        std::cout << std::endl;

        std::cout << "Liczba cykli algorytmu: ";
        std::cin >> cykle;
        std::cout << std::endl;


        clock_t czasInicjacji = clock();
        wczytywaniePunktow(nazwaPliku);
        poszczegolneDystanse();
        wypiszPunktyIDystanse();
        czasInicjacji = clock() - czasInicjacji;


        cout<< endl << "  Inicjacja zajela: "<< (float)czasInicjacji/CLOCKS_PER_SEC<<" sekund"<<
            endl<<"Czesc wlasciwa: " << endl << endl;

        clock_t czas = clock();


        inicjalizacjaRoju(iloscPszczol);
        podzialRoju();
        //wypiszTrasyroju();
        for (int i = 0; i < cykle; i++) {
            taniec();          

            wezwanieDoNajlepszegoRozwiazania(pszczoly);
            cout << ".";
        }
        czas = clock() - czas;

        cout << endl << "Rozwiazanie znaleziono w " << (float)czas/CLOCKS_PER_SEC << " sekund(y)";

        cout << endl << "Najlepsze rozwiazanie: " << endl;    
    
        cout << "[ ";
        for (auto x : najlepszeRozwiazanie) {
                cout << " > " << x;
        }
        cout << "] Dystans: " << dlugoscSciezki(najlepszeRozwiazanie);


        cout << endl  << endl;

        system("pause");
    }
      
}