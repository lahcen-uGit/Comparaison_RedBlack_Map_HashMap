#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
using namespace std;

enum Couleur { ROUGE, NOIR };


class ArbreRougeNoir {
private:
    struct Noeud {
        int     valeur;
        Couleur couleur;
        Noeud  *gauche, *droite, *parent;

        Noeud(int v, Couleur c, Noeud* nil)
            : valeur(v), couleur(c), gauche(nil), droite(nil), parent(nil) {}
    };

    Noeud* racine;
    Noeud* nil;


    long long nbComparaisons = 0;   // comparaisons faites pendant la descente
    long long nbRotations    = 0;   // rotations faites par corrigerInsertion
    long long nbCas1         = 0;   // recolorations (oncle rouge)

    // --- compteurs de la SUPPRESSION ---
    mutable long long nbComparaisonsSupp = 0;   // comparaisons pendant la recherche du noeud a supprimer
    long long nbCas2Supp     = 0;               // recolorations (frere noir, deux neveux noirs)

    void rotationGauche(Noeud* x) {
        nbRotations++;
        Noeud* y = x->droite;

        x->droite = y->gauche;
        if (y->gauche != nil) y->gauche->parent = x;

        y->parent = x->parent;
        if (x->parent == nil)              racine = y;
        else if (x == x->parent->gauche)   x->parent->gauche = y;
        else                               x->parent->droite = y;

        y->gauche = x;
        x->parent = y;
    }

    void rotationDroite(Noeud* x) {
        nbRotations++;
        Noeud* y = x->gauche;

        x->gauche = y->droite;
        if (y->droite != nil) y->droite->parent = x;

        y->parent = x->parent;
        if (x->parent == nil)              racine = y;
        else if (x == x->parent->droite)   x->parent->droite = y;
        else                               x->parent->gauche = y;

        y->droite = x;
        x->parent = y;
    }

    void corrigerInsertion(Noeud* z) {
        while (z->parent->couleur == ROUGE) {
            Noeud* pere      = z->parent;
            Noeud* grandPere = pere->parent;

            if (pere == grandPere->gauche) {
                Noeud* oncle = grandPere->droite;

                if (oncle->couleur == ROUGE) {       // CAS 1
                    nbCas1++;
                    pere->couleur      = NOIR;
                    oncle->couleur     = NOIR;
                    grandPere->couleur = ROUGE;
                    z = grandPere;
                } else {
                    if (z == pere->droite) {         // CAS 2
                        rotationGauche(pere);
                        z    = pere;
                        pere = z->parent;
                    }
                    pere->couleur      = NOIR;       // CAS 3
                    grandPere->couleur = ROUGE;
                    rotationDroite(grandPere);
                }
            } else {
                Noeud* oncle = grandPere->gauche;

                if (oncle->couleur == ROUGE) {       // CAS 1
                    nbCas1++;
                    pere->couleur      = NOIR;
                    oncle->couleur     = NOIR;
                    grandPere->couleur = ROUGE;
                    z = grandPere;
                } else {
                    if (z == pere->gauche) {         // CAS 2
                        rotationDroite(pere);
                        z    = pere;
                        pere = z->parent;
                    }
                    pere->couleur      = NOIR;       // CAS 3
                    grandPere->couleur = ROUGE;
                    rotationGauche(grandPere);
                }
            }
        }
        racine->couleur = NOIR;
    }



    // remplace le sous-arbre u par le sous-arbre v
    void transplanter(Noeud* u, Noeud* v) {
        if (u->parent == nil)              racine = v;
        else if (u == u->parent->gauche)   u->parent->gauche = v;
        else                               u->parent->droite = v;
        v->parent = u->parent;             
    }

    Noeud* minimum(Noeud* n) const {
        while (n->gauche != nil) n = n->gauche;
        return n;
    }

    Noeud* trouverNoeud(int valeur) const {
        Noeud* courant = racine;
        while (courant != nil) {
            nbComparaisonsSupp++;
            if      (valeur < courant->valeur) courant = courant->gauche;
            else if (valeur > courant->valeur) courant = courant->droite;
            else return courant;
        }
        return nil;
    }

    void corrigerSuppression(Noeud* x) {
        while (x != racine && x->couleur == NOIR) {
            if (x == x->parent->gauche) {
                Noeud* frere = x->parent->droite;

                if (frere->couleur == ROUGE) {                 // CAS 1 : frère rouge
                    frere->couleur     = NOIR;
                    x->parent->couleur = ROUGE;
                    rotationGauche(x->parent);
                    frere = x->parent->droite;
                }
                if (frere->gauche->couleur == NOIR &&
                    frere->droite->couleur == NOIR) {          // CAS 2 : frère noir, deux neveux noirs
                    nbCas2Supp++;
                    frere->couleur = ROUGE;
                    x = x->parent;
                } else {
                    if (frere->droite->couleur == NOIR) {      // CAS 3 : neveu gauche rouge seulement
                        frere->gauche->couleur = NOIR;
                        frere->couleur         = ROUGE;
                        rotationDroite(frere);
                        frere = x->parent->droite;
                    }
                    frere->couleur         = x->parent->couleur; // CAS 4 : neveu droit rouge
                    x->parent->couleur     = NOIR;
                    frere->droite->couleur = NOIR;
                    rotationGauche(x->parent);
                    x = racine;
                }
            } else {
                Noeud* frere = x->parent->gauche;

                if (frere->couleur == ROUGE) {                 // CAS 1
                    frere->couleur     = NOIR;
                    x->parent->couleur = ROUGE;
                    rotationDroite(x->parent);
                    frere = x->parent->gauche;
                }
                if (frere->droite->couleur == NOIR &&
                    frere->gauche->couleur == NOIR) {          // CAS 2
                    nbCas2Supp++;
                    frere->couleur = ROUGE;
                    x = x->parent;
                } else {
                    if (frere->gauche->couleur == NOIR) {      // CAS 3
                        frere->droite->couleur = NOIR;
                        frere->couleur         = ROUGE;
                        rotationGauche(frere);
                        frere = x->parent->gauche;
                    }
                    frere->couleur         = x->parent->couleur; // CAS 4
                    x->parent->couleur     = NOIR;
                    frere->gauche->couleur = NOIR;
                    rotationDroite(x->parent);
                    x = racine;
                }
            }
        }
        x->couleur = NOIR;
    }

  

    void supprimerTout(Noeud* n) {
        if (n == nil) return;
        supprimerTout(n->gauche);
        supprimerTout(n->droite);
        delete n;
    }

    int calculerHauteur(Noeud* n) const {
        if (n == nil) return 0;
        return 1 + max(calculerHauteur(n->gauche), calculerHauteur(n->droite));
    }

public:
    ArbreRougeNoir() {
        nil = new Noeud(0, NOIR, nullptr);
        nil->gauche = nil->droite = nil->parent = nil;
        racine = nil;
    }

    ~ArbreRougeNoir() {
        supprimerTout(racine);
        delete nil;
    }

    void inserer(int valeur) {
        Noeud* z = new Noeud(valeur, ROUGE, nil);

        Noeud* parent  = nil;
        Noeud* courant = racine;
        while (courant != nil) {
            nbComparaisons++;
            parent = courant;
            if      (valeur < courant->valeur) courant = courant->gauche;
            else if (valeur > courant->valeur) courant = courant->droite;
            else { delete z; return; }
        }

        z->parent = parent;
        if (parent == nil)                racine = z;
        else if (valeur < parent->valeur) parent->gauche = z;
        else                              parent->droite = z;

        corrigerInsertion(z);
    }

    bool chercher(int valeur) const {
        Noeud* courant = racine;
        while (courant != nil) {
            if      (valeur < courant->valeur) courant = courant->gauche;
            else if (valeur > courant->valeur) courant = courant->droite;
            else return true;
        }
        return false;
    }

    // retourne true si la valeur existait et a été supprimée
    bool supprimer(int valeur) {
        Noeud* z = trouverNoeud(valeur);
        if (z == nil) return false;

        Noeud*  y = z;
        Couleur couleurOrigine = y->couleur;
        Noeud*  x;

        if (z->gauche == nil) {                 // 0 ou 1 enfant (droit)
            x = z->droite;
            transplanter(z, z->droite);
        } else if (z->droite == nil) {          // 1 enfant (gauche)
            x = z->gauche;
            transplanter(z, z->gauche);
        } else {                                // 2 enfants : on prend le successeur
            y = minimum(z->droite);
            couleurOrigine = y->couleur;
            x = y->droite;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplanter(y, y->droite);
                y->droite = z->droite;
                y->droite->parent = y;
            }
            transplanter(z, y);
            y->gauche = z->gauche;
            y->gauche->parent = y;
            y->couleur = z->couleur;
        }

        delete z;
        if (couleurOrigine == NOIR) corrigerSuppression(x);
        return true;
    }

    



    int hauteur() const { return calculerHauteur(racine); }

    long long comparaisons() const { return nbComparaisons; }
    long long rotations()    const { return nbRotations; }
    long long recolorations() const { return nbCas1; }
    long long comparaisonsSupp() const { return nbComparaisonsSupp; }
    long long recolorationsSupp() const { return nbCas2Supp; }
};

//  Outils de mesure


// retourne le temps actuel en millisecondes
double temps() {
    return chrono::duration<double, milli>(
        chrono::steady_clock::now().time_since_epoch()).count();
}

// les 3 ordres possibles pour les cles a inserer
enum Ordre { ALEATOIRE, CROISSANT, PRESQUE_TRIE };

const int POURCENTAGE_ECHANGES = 5;   // PRESQUE_TRIE : on fait n * 5 / 100 echanges au hasard


vector<int> genererCles(int n, Ordre ordre) {
    vector<int> cles;
    for (int i = 0; i < n; i++) {
        cles.push_back(i);
    }

    mt19937 gen(42);                            
    if (ordre == ALEATOIRE) {
        shuffle(cles.begin(), cles.end(), gen);
    } else if (ordre == PRESQUE_TRIE) {
        // on part de l'ordre croissant et on echange quelques paires au hasard
        int nbEchanges = n * POURCENTAGE_ECHANGES / 100;
        for (int k = 0; k < nbEchanges; k++) {
            int i = gen() % n;
            int j = gen() % n;
            swap(cles[i], cles[j]);
        }
    }
    return cles;
}

void lancerBenchmark(int n, Ordre mode) {
    cout << "\n========== n = " << n << " | cles ";
    if (mode == ALEATOIRE)         cout << "dans un ordre ALEATOIRE";
    else if (mode == CROISSANT)    cout << "dans l'ordre CROISSANT";
    else                           cout << "PRESQUE TRIEES (" << POURCENTAGE_ECHANGES << "% d'echanges au hasard)";
    cout << " ==========\n";

    vector<int> cles = genererCles(n, mode);
    vector<int> ordre = genererCles(n, ALEATOIRE);   // recherche et suppression dans le desordre

    double debut;
    int trouves = 0;                            // pour verifier que les recherches marchent

    // ---------- notre arbre rouge-noir ----------
    ArbreRougeNoir arbre;

    debut = temps();
    for (int i = 0; i < n; i++) arbre.inserer(cles[i]);
    double insArbre = temps() - debut;

    debut = temps();
    for (int i = 0; i < n; i++) {
        if (arbre.chercher(ordre[i])) trouves++;
    }
    double rechArbre = temps() - debut;

    // on lit les compteurs et la hauteur AVANT la suppression
    long long comp = arbre.comparaisons();
    long long rot  = arbre.rotations();
    long long cas1 = arbre.recolorations();
    int hauteur    = arbre.hauteur();

    debut = temps();
    for (int i = 0; i < n; i++) arbre.supprimer(ordre[i]);
    double supArbre = temps() - debut;

    // ---------- std::map ----------
    map<int, int> m;

    debut = temps();
    for (int i = 0; i < n; i++) m.insert({cles[i], cles[i]});
    double insMap = temps() - debut;

    debut = temps();
    for (int i = 0; i < n; i++) {
        if (m.count(ordre[i]) == 1) trouves++;
    }
    double rechMap = temps() - debut;

    debut = temps();
    for (int i = 0; i < n; i++) m.erase(ordre[i]);
    double supMap = temps() - debut;

    // ---------- std::unordered_map ----------
    unordered_map<int, int> u;

    debut = temps();
    for (int i = 0; i < n; i++) u.insert({cles[i], cles[i]});
    double insHash = temps() - debut;

    debut = temps();
    for (int i = 0; i < n; i++) {
        if (u.count(ordre[i]) == 1) trouves++;
    }
    double rechHash = temps() - debut;

    debut = temps();
    for (int i = 0; i < n; i++) u.erase(ordre[i]);
    double supHash = temps() - debut;

    // ---------- affichage des temps ----------
  
    cout << fixed << setprecision(2);
    cout << "\n" << left << setw(26) << "Structure" << right
         << setw(16) << "Insertion(ms)" << setw(20) << "Insertion(ns)"
         << setw(16) << "Recherche(ms)" << setw(20) << "Recherche(ns)"
         << setw(16) << "Suppression(ms)" << setw(20) << "Suppression(ns)" << "\n";

    cout << left << setw(26) << "ArbreRougeNoir (maison)" << right
         << setw(16) << insArbre << setw(20) << insArbre * 1e6
         << setw(16) << rechArbre << setw(20) << rechArbre * 1e6
         << setw(16) << supArbre << setw(20) << supArbre * 1e6 << "\n";
    cout << left << setw(26) << "std::map" << right
         << setw(16) << insMap << setw(20) << insMap * 1e6
         << setw(16) << rechMap << setw(20) << rechMap * 1e6
         << setw(16) << supMap << setw(20) << supMap * 1e6 << "\n";
    cout << left << setw(26) << "std::unordered_map" << right
         << setw(16) << insHash << setw(20) << insHash * 1e6
         << setw(16) << rechHash << setw(20) << rechHash * 1e6
         << setw(16) << supHash << setw(20) << supHash * 1e6 << "\n";

    // ---------- affichage du cout de l'insertion ----------
    cout << "\nCout de l'insertion dans notre arbre :\n";
    cout << "  nombre d'insertions               : " << n << "\n";
    cout << "  nombre de comparaisons (total)    : " << comp
         << "   | par insertion : " << (double)comp / n
         << "   (log2(n) = " << log2((double)n) << ")\n";
    cout << "  nombre de rotations (total)       : " << rot
         << "   | par insertion : " << (double)rot / n << "\n";
    cout << "  nombre de recolorations (cas 1)   : " << cas1
         << "   | par insertion : " << (double)cas1 / n << "\n";
    cout << "  hauteur de l'arbre                : " << hauteur
         << "   (borne max 2*log2(n+1) = " << 2 * log2((double)n + 1) << ")\n";

    cout << "\nVerification : " << trouves << " elements trouves sur " << 3 * n << "\n";
}

int main() {
    while (true) {
        cout << "\nNombre d'elements a inserer (0 pour quitter) : ";
        long long n;
        if (!(cin >> n)) {                 
            cout << "Entree invalide.\n";
            break;
        }
        if (n == 0) break;
        if (n < 0 || n > 20000000) {
            cout << "Choisis un nombre positif\n";
            continue;
        }

        lancerBenchmark((int)n, ALEATOIRE);
        lancerBenchmark((int)n, CROISSANT);
        lancerBenchmark((int)n, PRESQUE_TRIE);
    }
    return 0;
}