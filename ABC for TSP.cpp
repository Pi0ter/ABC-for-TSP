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

std::vector<Punkt> punkty;
std::vector<Bee> pszczoly;
vector<int> najlepszeRozwiazanie;
float najkrotszaDroga = (float)INT_MAX;
std::vector<std::vector<float>>  tablicaDystansow;

int procentZwiadowcow;
int procentZbieraczy;
// procentZwiadowcow + procent zbieraczy = 100
int procentSkautow;
//procentSkautow to osobny procent potrzebny do przetwarzania rozwiązań 

int limitIteracji;
int cykle;


/**************************************************************/

struct Punkt {
public:
    float X;
    float Y;
};

class Bee {
public:
    char rola = ' '; // Przyporządkowana rola pszczoły[F,O,S]
    std::vector<int> trasa; // Każda pszczoła zna wszystkie punkty do przebycia 
    double dystans =0;
    int iterator=0; // Licznik kolejnych iteracji w których pszczoła wskazywała na to samo rozwiązanie
    
};
void infoPszczola(Bee a);
/**
Prawdopodobnie do struktury lub tabeli struktur
*/
void wczytywaniePunktow(std::string a) {   
    float x = 0, y = 0, d = 0;
    std::fstream plik("/ali535.txt",ios::in);
   
    if (!plik) {
        std::cout << std::endl << "Brak pliku";
        throw("Brak pliku");
        return;
    }
   
    if (plik.is_open()) {
        while (plik >>d>> x >> y) {
            std::cout << std::endl << d;
            Punkt tempPunkt;
            tempPunkt.X = x;
            tempPunkt.Y = y;
            punkty.push_back(tempPunkt);
        }
    }

    //test
    for (auto x : punkty) {
        cout << x.X << " " << x.Y << endl;
    }
    plik.close();
}

/**
Dystans między punktami, pierwsza zmienna to punkt w kolejności na globalnej liście, druga to drugi punkt
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

/**
Oblicza dystans pomiędzy poszczególnymi punktami w tabeli i zpisuje je do tabeli
*/
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


/**
Oblicza dystans rozwiązania jakie prezentuje dana pszczoła
*/
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

/**
Inicjalizacja roju pszczół
*/
void inicjalizacjaRoju(int iloscPszczol) {
    //inicjalizacja
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


/**
Przyporządkowanie podziału na rolę pszczół (zwiadowca, zbieracz i skaut?)
*/
void podzialRoju() {
    int liczbaPszczol = pszczoly.size();
    float lZwiad = float(procentZwiadowcow) / 100;
    float lZbier = float(procentZwiadowcow) / 100;
    //cout << lZbier << " " << lZwiad;
    lZwiad = floor(lZwiad * liczbaPszczol);
    lZbier = floor(lZbier * liczbaPszczol);
    //cout <<endl<< lZbier << " " << lZwiad;
    for (int i = 0; i < lZwiad; i++) {
        pszczoly[i].rola = 'O'; 
        //cout << endl << pszczoly[i].rola;
    }

    for (int i = lZwiad; i < liczbaPszczol; i++) {
        pszczoly[i].rola = 'F'; //Forager / Zbieracz 
        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(pszczoly[i].trasa.begin(), pszczoly[i].trasa.end(), default_random_engine(seed));        
       
        pszczoly[i].dystans = dlugoscSciezki(pszczoly[i].trasa);
        //cout <<endl<< pszczoly[i].dystans;
    }    
}

/**
Mutowanie ścieżek po przez zmianę kolejności odwiedzanych punktów (zwykła zamiana wybranych PUNKTOW w ścieżce a=b b=a)
*/
vector<int> mutujSciezke(Bee *a) {
    random_device rd;
    mt19937 mt(rd());
    uniform_int_distribution<int> distrib(0, punkty.size() - 2);

    int temp = 0;
    int numerPunktuDoZamiany = 0;
    numerPunktuDoZamiany = distrib(mt);
  
    vector<int> zmutowana;

    for (auto x : a->trasa) {
        zmutowana.push_back(x);
    }

    temp = zmutowana[numerPunktuDoZamiany];
    zmutowana[numerPunktuDoZamiany] = zmutowana[numerPunktuDoZamiany +1];;
    zmutowana[numerPunktuDoZamiany +1] = temp;
    
    return zmutowana;
}

/**
    Przy pomocy mutujSciezke()#{? nie wiem czy nie dało by się tego w jednej funkcji}
    próbuje znaleźć lepszą drogę po przez ponowne obliczenie dystansu pomiędzy punktami w nowej kolejności
    Jeśli dystans jest większy niż poprzedni przez określoną ilość iteracji pszczoły, pszczoła z zbieracza zmienia się w zwiadowcę. [brak postępu]

    ? nie wiem czy to dobra nazwa funkcji - może się mylić z funkcją szukaj
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
            a->iterator++;
        }
    }
}

/**
    Pszczoła zwiadowca bierze nową losową (chyba z innej funkcji) trasę i zmienia się w pszczołę zbieracza.
*/
void szukajNowejSciezki(Bee a){
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(a.trasa.begin(), a.trasa.end(), default_random_engine(seed));
    a.dystans = dlugoscSciezki(a.trasa);
    a.iterator = 0;    
    a.rola = 'F';
    
}

/**
    Wybiera losowe trasy dla pszczół zwiadowców
    Określana jest najlepsza dotychczasowa trasa
    Pszczoły z najgorszymi trasami zmieniają się w zwiadowców


    Potrzebna będzie możliwość przekazanie parametrów przekonującej pszczoły do globalnej(?) zmiennej jako nalepszego rozwiązania 
    > dystans najlepszego rozwiązania [chyba żeby obliczać go gdy będzie potrzebny na bierząco osobną funkcją]
    > ścieżka tej trasy między poszczególnymi punktami
*/

bool sortowanie(vector<float> a,vector<float> b) {
    return a[1] > b[1];
}

 vector<int> kopiujNajlepszaTrase(Bee a) {
    vector<int> temp;

    for (auto x : a.trasa) {
        temp.push_back(x);
    }

    /*for (int i = 0; i < a->trasa.size(); i++) {
        temp.push_back(a->trasa[i]);
    }*/

    return temp;
}


void taniec(){
    vector<vector<float>> dotychczasoweRozwiazania;
    
    
    for (int i = 0; i < (int)pszczoly.size(); i++) {
        //infoPszczola(pszczoly[i]);
        if (pszczoly[i].rola == 'F') {
            zwiad(&pszczoly[i]);
            if (najkrotszaDroga > pszczoly[i].dystans) {

                //najlepszeRozwiazanie = pszczoly[i].trasa;
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


vector<int> kopiujTrase(vector<int> a) {
    vector<int> temp;

    for (auto x : a) {
        temp.push_back(x);
    }
    //for (int i = 0; i < a->trasa.size(); i++) {
    //    temp.push_back(a->trasa[i]);
    //}

    return temp;
}


/**
    Zbiera pszczoły zbieraczy aby "pracowały" nad najlepszym (najbardziej obiecującym) aktualnie rozwiązaniem
    Nazwa robocza
*/
void wezwanieDoNajlepszegoRozwiazania(vector<Bee> pszczoly) {
    for (auto x : pszczoly) {
        if (x.rola == 'O') {
            //x.trasa = najlepszeRozwiazanie;
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


void infoPszczola(Bee a) {
    cout << endl << "> Rola: " << a.rola << " Dystans: " << a.dystans << " iteracja: " << a.iterator<<" Trasa: [";
    for (auto x : a.trasa) {
        cout << x << " > ";
    }
    cout << "]";
}


int main()
{
    std::string nazwaPliku= "bier127.txt";


    procentZbieraczy = 30; // w % 
    procentZwiadowcow = 70; 
    procentSkautow = 20;

    limitIteracji = 100; // Limit mutacji trasy dla pojedyńczej pszczoły
    cykle = 500;        // Limit cykli algorytmu 


    int iloscPszczol = 1000;


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


    /*for (auto x : pszczoly) {
        cout << x.iterator<<" ";
    }*/


    //for (auto x : pszczoly) {
    //    cout << endl;
    //    for (auto y : x.trasa) {
    //        cout << " " << y;
    //    }
    //}

    //cout<<endl<<dlugoscSciezki(pszczoly[1]);
    
}
