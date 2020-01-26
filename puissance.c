#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

// Paramètres du jeu
#define LARGEUR_MAX 7         // nb max de fils pour un noeud (= nb max de coups possibles) = 7 car on ne peut insérer de jetons que par colonne (7 colonnes)

#define TEMPS 5        // temps de calcul pour un coup avec MCTS (en secondes)
#define COMPROMIS 0.2f    // Constante c, qui est le compromis entre exploitation et exploration

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

// Demander à l'humain quel coup jouer
Coup *demanderCoup () {
    
    /* par exemple : */
    int i = -1; //init à -1 car si on entre autre chose qu'un entier i restera à -1 et le programme redemandera la colonne à choisir
    printf("Sur quelle colonne voulez-vous jouer votre jeton? ");
    scanf("%d", &i);
    fflush(0); //permet de vider le buffer du scanf, sinon scanf ne demandera plus notre input
    
    return nouveauCoup(i);
}

int coupJouable (Etat * etat, Coup * coup) {
    //parcours des lignes pour trouver la première libre, si aucune sur la hauteur de la grille le ocup n'est pas jouable
    int j;
    for (j = 0; j < GRILLE_HAUTEUR; j++) {
        if (etat->grille[coup->colonne][j] == ' ') {
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
        if (coupJouable(etat, coup)) {
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

// B-Valeur d'un noeud
// noeuds: noeuds dont on veut la B-valeur
double B_Value(Noeud * noeuds){
    //le signe est + au tour de l'ordinateur et - au tour de l'humain
    int signe = 1;
    if(noeuds->joueur == 0)
    	signe = -1;

    //moyenne des victoires du noeud
    double moyenne = noeuds->nb_victoires / noeuds->nb_simus;

	//DEBUG
    //printf("%f + %d * %f (-> ln : %f)\n", signe * moyenne, COMPROMIS, sqrt(log(noeuds->parent->nb_simus)/noeuds->nb_simus), log(noeuds->parent->nb_simus));

    return (signe * moyenne) + (COMPROMIS * sqrt(log(noeuds->parent->nb_simus)/noeuds->nb_simus));
}
	
//un fils est développé s'il a été simulé au moins une fois
int tousFilsDeveloppes(Noeud * noeud) {
	int i, cpt = 0;
	for (i = 0; i < noeud->nb_enfants; i++) {
		if (noeud->enfants[i]->nb_simus > 0) {
			cpt++;
		}
	}

	return cpt == noeud->nb_enfants ? 1 : 0;
}

void developpementAleatoire (Noeud * noeud) {
	Coup * coupAleatoire;

	//parcours aléatoire jusqu'à une fin de partie
    while(testFin(noeud->etat) == NON) {
        //recherche d'un coup aléatoire jouable
        do {
            coupAleatoire = nouveauCoup(rand()%7);
        } while(!coupJouable(noeud->etat, coupAleatoire));

        //création d'un fils dans lequel on joue ce coup
        jouerCoup(noeud->etat,coupAleatoire);
    }

    switch (testFin(noeud->etat)) {
    	case ORDI_GAGNE:
    		noeud->nb_victoires++;
    		break;

    	case HUMAIN_GAGNE:
    		noeud->nb_victoires--;
    		break;

    	default:
    		break;
    }

    noeud->nb_simus++;
}

void afficherProgres (clock_t temps, clock_t tempsmax) {
	int progress = (int) (((float)((float)temps / (float)tempsmax)) * 20);
    int i;
    printf("{");
    for (i = 0; i < 20; i++) {
    	i <= progress ? printf("=") : printf(" ");
    }
    printf("}\r");
}

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, clock_t tempsmax) {
    
    clock_t tic, toc;
    tic = clock();
    clock_t temps;
    int iter = 0;
    
    // Coup possible
    Coup ** coups;
    Coup * meilleur_coup ;
    
    // Créer l'arbre de recherche
    Noeud * racine = nouveauNoeud(NULL, NULL);
   	racine->etat = copieEtat(etat);
    
    //DEBUG
    //printf("Création des premiers noeuds\n");

    // créer les premiers noeuds:
    coups = coups_possibles(racine->etat);
    int k = 0;
    Coup * coupAlea;
    Noeud * enfant;

    //initialisation des 7 (max) premiers fils de la racine
    while (coups[k] != NULL) {
    	//DEBUG
    	//printf("\tCréation d'un enfant\n");

        enfant = ajouterEnfant(racine, coups[k]);

        developpementAleatoire(enfant);

        racine->nb_simus++; //incrémentation manuelle du nombre de simus de la racine pour l'initialisation

		//DEBUG
        //printf("\tEtat de la partie finie : %d\n", testFin(enfant->etat));
        //printf("\tB-Valeur : %f\n", B_Value(enfant));
        //printf("\tN() : %d ; r() : %d ; parent(N()) : %d\n\n", enfant->nb_simus, enfant->nb_victoires, enfant->parent->nb_simus);


        k++;
    }

    //DEBUG
    //printf("\nAlgo MCTS : \n");
    
    int i;
    Noeud * cur;
    double bMax;
    int indMax;
    do{

    	cur = racine;

    	/*
    	1. Sélectionner récursivement à partir de la racine le nœud avec la plus grande
    	B-valeur jusqu’à arriver à un nœud terminal ou un dont tous les fils n’ont
    	pas été développés
    	*/

    	//DEBUG
    	//printf("\n\t1. SELECTION\n");

        //tant que le fils courant a des enfants ET qu'ils ont tous été développés
        while (cur->nb_enfants > 0 && tousFilsDeveloppes(cur)) {
        	//DEBUG
        	//printf("\t\tParcours des fils :\n");

        	bMax = B_Value(cur->enfants[0]);
			indMax = 0;

        	//parcours des fils du noeud courant et sélection de la meilleure B-valeur
        	for (i = 0; i < cur->nb_enfants; i++) {

        		//DEBUG
        		//printf("\t\t\tB-valeur (%d) : %f\n", i, B_Value(cur->enfants[i]));

    			//sauvegarde de l'indice du noeud ayant la plus grande B-valeur
        		if (B_Value(cur->enfants[i]) > bMax)
        			indMax = i;
        	}
        	//DEBUG
        	//printf("\t\tB-valeur max (%d) : %f\n", indMax, bMax);

        	cur = cur->enfants[indMax];
        }

        /*
        2. Développer un fils choisi aléatoirement parmi les fils non développés
        */

        //DEBUG
    	//printf("\n\t2. DEVELOPPEMENT\n");

    	//développement des fils du noeud choisi
        k = 0;
        coups = coups_possibles(cur->etat);
        while (coups[k] != NULL && coupJouable(cur->etat, coups[k])) {
        	//DEBUG
        	//printf("\t\tDeveloppement du coup %d\n", k);

            enfant = ajouterEnfant(cur, coups[k]);
            k++;
        }
        
        /*
        3. Simuler la fin de la partie avec une marche aléatoire (de tous les fils crées?)
        */

        //DEBUG
        //printf("\n\t3. SIMULATION\n");

        for (i = 0; i < cur->nb_enfants; i++) {
        	//DEBUG
        	//printf("\t\tSimulation du fils %d\n", i);

        	developpementAleatoire(cur->enfants[i]);
        }

        /*
        */

        //DEBUG
        //printf("\n\t4. MISE A JOUR\n");
        
        //DEBUG
        //printf("\n\t- GESTION TEMPS\n");
        
        // calcul du temps
        toc = clock();
        temps = (toc - tic) / CLOCKS_PER_SEC;
        iter++;

        //DEBUG
        //printf("\t\ttemps : %d | temps max : %d\n", temps, tempsmax);

        //DISPLAY?
        afficherProgres(temps, tempsmax);
    } while(temps < tempsmax);

	bMax = B_Value(racine->enfants[0]);
	indMax = 0;
    for (i = 0; i < racine->nb_enfants; i++) {
    	if (B_Value(racine->enfants[i]) > bMax)
    		indMax = i;
    }

    meilleur_coup = racine->enfants[indMax]->coup;

    jouerCoup(etat, meilleur_coup);

    printf("\rItérations : %d       \n", iter);
    
    
    //meilleur_coup = coups[ rand()%k ]; // choix aléatoire
    
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
    //jouerCoup(etat, meilleur_coup );
    
    // Penser à libérer la mémoire :
    freeNoeud(racine);
    free(coups);
}

int main(void) {
    srand(time(NULL));
    Coup * coup;
    FinDePartie fin = NON;
    
    // initialisation
    Etat * etat = etat_initial();
    
    etat->joueur = -1;
    
    // Choisir qui commence :
    do {
        printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
        scanf("%d", &(etat->joueur));
        fflush(0);
    } while (etat->joueur < 0 || etat->joueur > 1);
    
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
            
            ordijoue_mcts( etat, TEMPS );
             
        }
        
        fin = testFin(etat);
    }

    free(coup);
    
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
