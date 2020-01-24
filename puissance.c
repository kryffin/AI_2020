#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>

// Paramètres du jeu
#define LARGEUR_MAX 7         // nb max de fils pour un noeud (= nb max de coups possibles) = 7 car on ne peut insérer de jetons que par colonne (7 colonnes)

#define TEMPS 5        // temps de calcul pour un coup avec MCTS (en secondes)
#define COMPROMIS 2    // Constante c, qui est le compromis entre exploitation et exploration

#define GRILLE_LARGEUR 7
#define GRILLE_HAUTEUR 6

#define SUITE_GAGNANTE 4

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

/*
 
 joueur 0 : humain
 joueur 1 : ordinateur
 
 */

// Variables globales
int nombreSimulation[][];

// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition d'un état du jeu
typedef struct EtatSt {
    
    int joueur; // à qui de jouer ?
    
    /* par exemple, pour morpion: */
    char grille[GRILLE_LARGEUR][GRILLE_HAUTEUR];
    
} Etat;

// Definition d'un coup à jouer
typedef struct CoupSt {
    
    int colonne;
    
} Coup;

// Copier un état
Etat *copieEtat (Etat *src) {
    Etat *etat = (Etat *)malloc(sizeof(Etat)); //création de l'état qui recevra la copie
    
    etat->joueur = src->joueur; //copie du joueur
    
    //copie de la grille
    int i, j;
    for (i = 0; i < GRILLE_LARGEUR; i++)
        for (j = 0; j < GRILLE_HAUTEUR; j++)
            etat->grille[i][j] = src->grille[i][j];
    
    return etat;
}

// Etat initial
Etat *etat_initial () {
    Etat *etat = (Etat *)malloc(sizeof(Etat)); //création de l'état
    
    //remplissage de la grille sans jetons
    int i,j;
    for (i = 0; i < GRILLE_LARGEUR; i++)
        for (j = 0; j < GRILLE_HAUTEUR; j++)
            etat->grille[i][j] = ' ';
    
    return etat;
}

// Procédure affichant un état de jeu dans la sortie standard
void afficheJeu (Etat *etat) {
    
    //première ligne (indices des colonnes)
    int i,j;
    
    //boucle d'affichage de la grille
    for (j = GRILLE_HAUTEUR-1; j >= 0; j--) {
        for(i = 0; i < GRILLE_LARGEUR; i++) {
            printf(" %c |", etat->grille[i][j]);
        }
        printf("\b \n---------------------------\n");
    }
    
    for (i = 0; i < GRILLE_LARGEUR; i++)
        printf(" %d |", i);
    printf("\b \n\n");
    
}


// TODO: adapter la liste de paramètres au jeu
Coup *nouveauCoup (int i) {
    Coup *coup = (Coup *)malloc(sizeof(Coup)); //nouveau coup
    
    // TODO: à compléter avec la création d'un nouveau coup
    
    /* par exemple : */
    coup->colonne = i;
    
    return coup;
}

void cleanBuffer () {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Demander à l'humain quel coup jouer
Coup *demanderCoup () {
    
    /* par exemple : */
    int i = -1; //init à -1 car si on entre autre chose qu'un entier i restera à -1 et le programme redemandera la colonne à choisir
    printf("Sur quelle colonne voulez-vous jouer votre jeton? ");
    scanf("%d", &i);
    fflush(0); //permet de vider le buffer du scanf, sinon scanf ne demandera plus notre input
    
    return nouveauCoup(i);
}

int coupJouable (Etat etat, Coup coup) {
    //parcours des lignes pour trouver la première libre, si aucune sur la hauteur de la grille le ocup n'est pas jouable
    int j;
    for (j = 0; j < GRILLE_HAUTEUR; j++) {
        if (etat.grille[coup.colonne][j] == ' ') {
            return 1;
        }
    }
    
    return 0;
}

// Modifier l'état en jouant un coup
// retourne 0 si le coup n'est pas possible
int jouerCoup (Etat *etat, Coup *coup) {
    
    // TODO: à compléter
    
    /* par exemple : */
    if (etat->grille[coup->colonne][GRILLE_HAUTEUR-1] != ' ') {
        return 0; //si l'emplacement le plus haut d'une colonne est déjà pris on ne peux pas jouer dans cette colonne
    } else {
        int j;
        
        //parcours des lignes pour trouver la première libre
        for (j = 0; j < GRILLE_HAUTEUR; j++) {
            if (etat->grille[coup->colonne][j] == ' ') {
                etat->grille[coup->colonne][j] = etat->joueur ? 'O' : 'X'; //on met un jeton à l'emplacement libre le plus bas de la colonne demandée O : humain; X : machine
                break;
            }
        }
        
        // à l'autre joueur de jouer
        etat->joueur = AUTRE_JOUEUR(etat->joueur);
        
        return 1; //le coup est jouable
    }
}

// Retourne une liste de coups possibles à partir d'un etat
// (tableau de pointeurs de coups se terminant par NULL)
Coup **coups_possibles (Etat *etat) {
    //coups sera de la taille : (1+8) * taille(coup)
    Coup **coups = (Coup **)malloc((1+LARGEUR_MAX) * sizeof(Coup *));
    
    int k = 0, i;
    for(i = 0; i < GRILLE_LARGEUR; i++) {
        Coup *coup = nouveauCoup(i);
        if (coupJouable(*etat, *coup)) {
            //si le coup en largeur i est jouable on l'ajoute au tableau des coups jouables
            coups[k] = coup;
            k++;
        }
    }
    
    coups[k] = NULL;
    
    return coups;
}


// Definition du type Noeud
typedef struct NoeudSt {
    
    int joueur; // joueur qui a joué pour arriver ici
    Coup *coup;   // coup joué par ce joueur pour arriver ici
    
    Etat *etat; // etat du jeu
    
    struct NoeudSt *parent; // noeud parent
    struct NoeudSt *enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
    int nb_enfants;    // nb d'enfants présents dans la liste
    
    // POUR MCTS:
    int nb_victoires;
    int nb_simus;
    
} Noeud;


// Créer un nouveau noeud en jouant un coup à partir d'un parent
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud *nouveauNoeud (Noeud *parent, Coup *coup) {
    Noeud *noeud = (Noeud *)malloc(sizeof(Noeud)); //création d'un noeud
    
    if (parent != NULL && coup != NULL) {
        noeud->etat = copieEtat(parent->etat);
        jouerCoup(noeud->etat, coup); //modification de l'état du noeud en l'état modifié par le coup à jouer
        noeud->coup = coup;
        noeud->joueur = AUTRE_JOUEUR(parent->joueur);
    } else {
        noeud->etat = NULL;
        noeud->coup = NULL;
        noeud->joueur = 0;
    }
    
    noeud->parent = parent;
    noeud->nb_enfants = 0; //nouveau noeud donc aucun enfant
    
    // POUR MCTS:
    noeud->nb_victoires = 0;
    noeud->nb_simus = 0;
    
    
    return noeud;
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud *ajouterEnfant(Noeud *parent, Coup *coup) {
    Noeud *enfant = nouveauNoeud(parent, coup) ;
    parent->enfants[parent->nb_enfants] = enfant;
    parent->nb_enfants++;
    return enfant;
}

void freeNoeud (Noeud *noeud) {
    if (noeud->etat != NULL)
        free(noeud->etat);
    
    while (noeud->nb_enfants > 0) {
        freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
        noeud->nb_enfants--;
    }
    
    if (noeud->coup != NULL)
        free(noeud->coup);
    
    free(noeud);
}

// Test si l'état est un état terminal
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin( Etat * etat ) {
    
    // TODO...
    
    /* par exemple    */
    
    // tester si un joueur a gagné
    int i, j, k, n = 0;
    for (i = 0; i < GRILLE_LARGEUR; i++) {
        for(j = 0; j < GRILLE_HAUTEUR; j++) {
            printf("TEST : %d %d\n", i, j);
            if (etat->grille[i][j] != ' ') {
                n++; // nb coups joués
                
                // lignes
                k = 0;
                while (k < SUITE_GAGNANTE && etat->grille[i+k][j] == etat->grille[i][j])
                    k++;
                if (k == SUITE_GAGNANTE)
                    return etat->grille[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;
                
                // colonnes
                k = 0;
                while (k < SUITE_GAGNANTE && etat->grille[i][j+k] == etat->grille[i][j])
                    k++;
                if (k == SUITE_GAGNANTE)
                    return etat->grille[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;
                
                // diagonales
                k = 0;
                while (k < SUITE_GAGNANTE && j+k < SUITE_GAGNANTE && etat->grille[i+k][j+k] == etat->grille[i][j])
                    k++;
                if (k == SUITE_GAGNANTE)
                    return etat->grille[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;
                
                k = 0;
                while (k < SUITE_GAGNANTE && j-k >= 0 && etat->grille[i+k][j-k] == etat->grille[i][j])
                    k++;
                if (k == SUITE_GAGNANTE)
                    return etat->grille[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;
            }
        }
    }
    
    // et sinon tester le match nul
    if (n == GRILLE_LARGEUR * GRILLE_HAUTEUR)
        return MATCHNUL;
    
    return NON;
}



// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, int tempsmax) {
    
    clock_t tic, toc;
    tic = clock();
    int temps;
    
    Coup ** coups;
    Coup * meilleur_coup ;
    
    // Créer l'arbre de recherche
    Noeud * racine = nouveauNoeud(NULL, NULL);
    racine->etat = copieEtat(etat);
    
    // créer les premiers noeuds:
    coups = coups_possibles(racine->etat);
    int k = 0;
    Noeud * enfant;
    while ( coups[k] != NULL) {
        enfant = ajouterEnfant(racine, coups[k]);
        k++;
    }
    
    
    meilleur_coup = coups[ rand()%k ]; // choix aléatoire
    
    /*  TODO :
     - supprimer la sélection aléatoire du meilleur coup ci-dessus
     - implémenter l'algorithme MCTS-UCT pour déterminer le meilleur coup ci-dessous
     
     int iter = 0;
     
     do {
     
     
     
     // à compléter par l'algorithme MCTS-UCT...
     
     
     
     
     toc = clock();
     temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
     iter ++;
     } while ( temps < tempsmax );
     
     / fin de l'algorithme  */
    
    // Jouer le meilleur premier coup
    jouerCoup(etat, meilleur_coup );
    
    // Penser à libérer la mémoire :
    freeNoeud(racine);
    free (coups);
}

// B-Valeur d'un noeud
// etat: etat dont on veut la B-valeur, N: nombre de simulation dans ce noeuds
double B_Value(Etat * etat){
    /*int signe = 1;
    // Si c'est au joueur humain de jouer, on part sur du négatif
    if(etat->joueur == 0){
        signe = -1;
    }
    double moyenne = 0;
    // TODO: faire la B_Value en calculant la moyenne et la racine
    return signe * moyenne + COMPROMIS * sqr()*/
}

// Retourne le nombre de simulation d'un état
int Nb_Simulation(Etat * etat){
    // TODO: trouver un moyen de récup le nombre de simulation d'un état
    // Stocker lors du MCTS dans un tableau global le nb simulation et l'indice = id du noeud ?
}

int main(void) {
    
    Coup * coup;
    FinDePartie fin = NON;
    
    // initialisation
    Etat * etat = etat_initial();
    
    // Choisir qui commence :
    printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
    scanf("%d", &(etat->joueur) );
    
    
    
    // boucle de jeu
    while (fin == NON) {
        printf("\n");
        afficheJeu(etat);
        
        if ( etat->joueur == 0 ) {
            // tour de l'humain
            
            do {
                coup = demanderCoup();
            } while (!jouerCoup(etat, coup));
            
        } else {
            // tour de l'Ordinateur
            
            //ordijoue_mcts( etat, TEMPS );
            int i = 6;
            do {
                coup = nouveauCoup(i--);
            } while (!jouerCoup(etat, coup));
            /*do {
                coup = demanderCoup();
            } while (!jouerCoup(etat, coup));*/
             
        }
        
        fin = testFin(etat);
        printf("%u\n", fin);
        free(coup);
    }
    
    printf("\n");
    afficheJeu(etat);
    
    if ( fin == ORDI_GAGNE )
        printf( "** L'ordinateur a gagné **\n");
    else if ( fin == MATCHNUL )
        printf(" Match nul !  \n");
    else
        printf( "** BRAVO, l'ordinateur a perdu  **\n");
    return 0;
}
