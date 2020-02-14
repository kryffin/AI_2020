#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

// Paramètres du jeu
#define LARGEUR_MAX 7         // nb max de fils pour un noeud (= nb max de coups possibles) = 7 car on ne peut insérer de jetons que par colonne (7 colonnes)

#define TEMPS 10        // temps de calcul pour un coup avec MCTS (en secondes)
#define COMPROMIS sqrt(2)    // Constante c, qui est le compromis entre exploitation et exploration

#define GRILLE_LARGEUR 7
#define GRILLE_HAUTEUR 6

#define SUITE_GAGNANTE 4

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

int AFFICHAGE = 0;

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
    
    return nouveauCoup(i);
}

int coupJouable (Etat * etat, Coup * coup) {
    //parcours des lignes pour trouver la première libre, si aucune sur la hauteur de la grille le ocup n'est pas jouable
    if (coup->colonne >= 0 && coup->colonne < 7)
        for (int j = 0; j < GRILLE_HAUTEUR; j++)
            if (etat->grille[coup->colonne][j] == ' ')
                return 1;
    
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

Etat* jouerCoupTest (Etat *etat, Coup *coup) {

    // TODO: à compléter

    /* par exemple : */
    if (etat->grille[coup->colonne][GRILLE_HAUTEUR-1] != ' ') {
        return NULL; //si l'emplacement le plus haut d'une colonne est déjà pris on ne peux pas jouer dans cette colonne
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
        printf("avant: %d ",etat->joueur);
        etat->joueur = AUTRE_JOUEUR(etat->joueur);
        printf("après: %d ",etat->joueur);
        afficheJeu(etat);
        return etat; //le coup est jouable
    }
}

// Retourne une liste de coups possibles à partir d'un etat
// (tableau de pointeurs de coups se terminant par NULL)
Coup **coups_possibles (Etat *etat) {
    //coups sera de la taille : (1+7) * taille(coup)
    Coup **coups = (Coup **)malloc((1+LARGEUR_MAX) * sizeof(Coup *));
    
    int k = 0, i;
    for(i = 0; i < GRILLE_LARGEUR; i++) {
        Coup *coup = nouveauCoup(i);
        if (coupJouable(etat, coup)) {
            //si le coup en largeur i est jouable on l'ajoute au tableau des coups jouables
            coups[k] = coup;
            k++;
        } else {
            free(coup);
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
                while (k < SUITE_GAGNANTE && i+k < GRILLE_LARGEUR && etat->grille[i+k][j] == etat->grille[i][j])
                    k++;
                if (k == SUITE_GAGNANTE)
                    return etat->grille[i][j] == 'O'? ORDI_GAGNE : HUMAIN_GAGNE;
                
                // colonnes
                k = 0;
                while (k < SUITE_GAGNANTE && j+k < GRILLE_HAUTEUR && etat->grille[i][j+k] == etat->grille[i][j])
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
double B_Value(Noeud * noeud){
    //le signe est + au tour de l'ordinateur et - au tour de l'humain
    int signe = -1;
    if(noeud->joueur == 0)
    	signe = 1;

    //moyenne des victoires du noeud
    double moyenne;
    if (noeud->nb_simus == 0)
    	moyenne = 0;
    else
    	moyenne = noeud->nb_victoires / noeud->nb_simus;

	//DEBUG
    //if(AFFICHAGE)printf("%f + %d * %f (-> ln : %f)\n", signe * moyenne, COMPROMIS, sqrt(log(noeuds->parent->nb_simus)/noeuds->nb_simus), log(noeuds->parent->nb_simus));

    double res;
    if (noeud->parent->nb_simus == 0)
    	res = (signe * moyenne);
    else
    	res = (signe * moyenne) + (COMPROMIS * sqrt(log(noeud->parent->nb_simus)/noeud->nb_simus));

    return res;
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

void afficherProgres (clock_t temps, clock_t tempsmax) {
	int progress = (int) (((float)((float)temps / (float)tempsmax)) * 20);
    int i;
    printf("{");
    for (i = 0; i < 20; i++) {
    	i <= progress ? printf("=") : printf(" ");
    }
    printf("}\r");
}

// Retourne 0 si l'état du noeud n'est pas final (la partie n'est pas terminée), 1 sinon
int estFinale (Noeud * noeud) {
	return testFin(noeud->etat) == NON ? 0 : 1;
}

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, clock_t tempsmax) {
    
    //calcul du temps restant
    clock_t tic, toc;
    tic = clock();
    clock_t temps;

    int iter = 0; //nombre d'itérations
    
    // Coups possible et meilleur coup final
    Coup ** coups;
    Coup * meilleur_coup ;
    
    // Création de l'arbre de recherche
    Noeud * racine = nouveauNoeud(NULL, NULL);
   	racine->etat = copieEtat(etat);

    int i, k, indMax; //i compteur sur les fils, k sur les coups et indMax indice de la plus grande B_value
    double bMax; //B_value max lors de la recherche de la plus grande B_value

    Noeud * enfant; //enfant à développer
    Noeud * cur; //noeud courant parcouru

    do{

    	cur = racine; //on démarre à la racine de l'arbre

    	/*
    	1. Sélectionner récursivement à partir de la racine le nœud avec la plus grande
    	B-valeur jusqu’à arriver à un nœud terminal ou un dont tous les fils n’ont
    	pas été développés
    	*/

    	//DEBUG
    	if(AFFICHAGE)printf("\n\t1. SELECTION\n");

        //tant que le fils courant a des enfants et que tous les enfants sont développés
        while (cur->nb_enfants > 0 && tousFilsDeveloppes(cur)) {
        	//DEBUG
        	if(AFFICHAGE)printf("\t\tParcours des fils :\n");

        	bMax = B_Value(cur->enfants[0]);
			indMax = 0;

        	//parcours des fils du noeud courant et sélection de la meilleure B-valeur
        	for (i = 0; i < cur->nb_enfants; i++) {
        		//DEBUG
        		if(AFFICHAGE)printf("\t\t\tcur_nb_enfants : %u | Noeud_nb_simus : %u | B-valeur (%d) : %f\n", cur->nb_enfants, cur->enfants[i]->nb_simus, i, B_Value(cur->enfants[i]));

    			//sauvegarde de l'indice du noeud ayant la plus grande B-valeur
        		if (B_Value(cur->enfants[i]) > bMax)
        			indMax = i;
        	}
        	//DEBUG
        	if(AFFICHAGE)printf("\t\tB-valeur max (%d) : %f\n", indMax, bMax);

        	cur = cur->enfants[indMax];

        	//DEBUG
        	if(AFFICHAGE)printf("\t\tNouveau cur -> nb_enfants : %u, nb_simus : %u\n", cur->nb_enfants, cur->nb_simus);
        }

        /*
        2. Développer un fils choisi aléatoirement parmi les fils non développés
        */

        // DEBUG AFFICHE_ETAT
        //if(AFFICHAGE)afficheJeu(cur->etat);

        //DEBUG
    	if(AFFICHAGE)printf("\n\t2. DEVELOPPEMENT\n");
        
        if (!estFinale(cur)) {
        	if (cur->nb_enfants < 1) {
        		//développement des fils du noeud
	        	coups = coups_possibles(cur->etat);
	        	k = 0;

	        	//DEBUG
	        	if(AFFICHAGE)printf("\t\tDéveloppement des fils du noeud (le noeud n'a pas encore de fils)\n");

	        	while (coups[k] != NULL) {
	        		//DEBUG
	        		if(AFFICHAGE)printf("\t\t\tDéveloppement du fils %u du noeud (coup : %u)\n", k, coups[k]->colonne);

	        		enfant = ajouterEnfant(cur, coups[k]);
	        		jouerCoup(enfant->etat, coups[k]);
	        		k++;
	        	}
	        	
	        	free(coups);

	        	//selection d'un enfant au hasard
    			int r = rand() % cur->nb_enfants;
    			enfant = cur->enfants[r];
    			cur = cur->enfants[r];

    			//DEBUG
        		if(AFFICHAGE)printf("\t\tChoix aléatoire d'un fils (le noeud n'avait pas d'enfant) : %u\n", r);
        	} else {
        		//le noeud a déjà des enfants, on en cherche un à développer

        		int nonDev[LARGEUR_MAX]; //tableau des indices des fils non développés
        		int j = 0;
        		for (i = 0; i < cur->nb_enfants; i++) {
        			if (cur->enfants[i]->nb_simus < 1) {
        				nonDev[j] = i;
        				j++;
        			}
        		}

        		//choix aléatoire d'un fils non développé
        		int r = rand() % j;
        		enfant = cur->enfants[nonDev[r]];
        		cur = cur->enfants[nonDev[r]];

        		//DEBUG
        		if(AFFICHAGE)printf("\t\tChoix aléatoire d'un fils non développé (le noeud a déjà des enfants) : %u\n", r);
        	}
        }
                
        /*
        3. Simuler la fin de la partie avec une marche aléatoire (de tous les fils crées?)
        */

        //DEBUG
        if(AFFICHAGE)printf("\n\t3. SIMULATION\n");
        
        Etat * etatAleatoire = copieEtat(enfant->etat);

        //tant que l'état du jeu de l'enfant est interminé
	    while(testFin(etatAleatoire) == NON) {
	        //generation des coups possibles
	        coups = coups_possibles(etatAleatoire);

	        k = 0;
	        while (coups[k] != NULL) {
	        	k++;
	        }

	        //choix aléatoire d'un coup parmis ceux possible et affectation du coup sur l'état
	        int r = rand() % k;
	        jouerCoup(etatAleatoire, coups[r]);
            
            k = 0;
	        while (coups[k] != NULL) {
                free(coups[k]);
	        	k++;
	        }
	        free(coups);

	        //DEBUG
	        if(AFFICHAGE)printf("\t\t\tJe joue le coup %u (coup : %u)\n", r, coups[r]->colonne);
	    }
	    
	    
	    //DEBUG
	    if(AFFICHAGE)printf("\t\tSimulation terminée\n");

        //DEBUG
        if(AFFICHAGE)printf("\n\t4. MISE A JOUR\n");

		//recompense de 1 si l'ordi est gagnant, 0 sinon
        int recompense = 0;
        if (testFin(etatAleatoire) == ORDI_GAGNE)
        	recompense = 1;

	    free(etatAleatoire);

        
        //DEBUG
       if(AFFICHAGE)printf("\t\tMise à jour d'une simulation avec en récompense %u\n", recompense);

        while (cur != NULL) {
        	//DEBUG
        	if(AFFICHAGE)printf("\t\t\tMise à jour d'un noeud\n");

        	cur->nb_simus++;
        	cur->nb_victoires = cur->nb_victoires + recompense;
        	cur = cur->parent;
        }

        //DEBUG
        if(AFFICHAGE)printf("\t\tFin de la remontada\n");
        
        //DEBUG
        if(AFFICHAGE)printf("\n\t- GESTION TEMPS\n");
        
        // calcul du temps
        toc = clock();
        temps = (toc - tic) / CLOCKS_PER_SEC;

        iter++;

        //DISPLAY?
        if(!AFFICHAGE)afficherProgres(temps, tempsmax);
                
    } while(temps < tempsmax);

    //recherche du meilleur coup à jouer
	bMax = B_Value(racine->enfants[0]);
	indMax = 0;
    for (i = 0; i < racine->nb_enfants; i++) {
    	if (B_Value(racine->enfants[i]) > bMax)
    		indMax = i;
    }
    meilleur_coup = racine->enfants[indMax]->coup;

    //application du meilleur coup sur le jeu actuel
    jouerCoup(etat, meilleur_coup);

    printf("\rItérations : %d       \n", iter);
    
    //free nécessaires
    freeNoeud(racine);
}

void creerTestEtat(){
    FILE* fichier = NULL;
    Etat * etat = etat_initial();
    etat->joueur = -1;
    fichier = fopen("test.txt", "r+");
    Coup *coup;
    int cpt = 0;
    if (fichier != NULL){
        char col = NULL;
        while(col != EOF){
            col = fgetc(fichier);
            cpt++;
        }
        int tab[cpt];
        cpt = 0;
        rewind(fichier);
        col = NULL;
        while(col != EOF){
            col = fgetc(fichier);
            int col1 = atoi(&col);
            tab[cpt] = col1;
            cpt++;
        }
        fclose(fichier);
        int cpt1 = 0;
        while(cpt1 < cpt-1){
            int col1 = tab[cpt1];
            printf("%d \n",col1);
            coup = nouveauCoup(col1);
            //etat->joueur = AUTRE_JOUEUR(etat->joueur);
            jouerCoupTest(etat,coup);
            cpt1++;
        }
        afficheJeu(etat);
        FinDePartie fin = testFin(etat);
        if ( fin == ORDI_GAGNE )
            printf( "** L'ordinateur a gagné **\n");
        else if ( fin == MATCHNUL )
            printf(" Match nul !  \n");
        else
            printf( "** BRAVO, l'ordinateur a perdu  **\n");
    }
}

int main (int argc, char **argv) {
    
    if(argc == 2)
    AFFICHAGE = atoi(argv[1]);
    
    /*srand(time(NULL));
    Coup * coup;
    FinDePartie fin = NON;
    
    // initialisation
    Etat * etat = etat_initial();
    
    etat->joueur = -1;
    
    // Choisir qui commence :
    do {
        
        printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
        scanf("%d", &(etat->joueur));
        
    } while (etat->joueur < 0 || etat->joueur > 1);

    // boucle de jeu
    while (fin == NON) {
        printf("\n");
        afficheJeu(etat);
        
        if ( etat->joueur == 0 ) {
            // tour de l'humain
            coup = NULL;
            do {
                
                if (coup != NULL) free(coup); //free du coup avant d'en redemander un nouveau
                coup = demanderCoup();
                
            } while (!coupJouable(etat, coup));
            
            jouerCoup(etat, coup);
            
            free(coup);
            
        } else {
            // tour de l'Ordinateur
            
            ordijoue_mcts( etat, TEMPS );
             
        }
        
        fin = testFin(etat);
    }
    
    printf("\n");
    afficheJeu(etat);
    free(etat);
    
    if ( fin == ORDI_GAGNE )
        printf( "** L'ordinateur a gagné **\n");
    else if ( fin == MATCHNUL )
        printf(" Match nul !  \n");
    else
        printf( "** BRAVO, l'ordinateur a perdu  **\n");*/

    creerTestEtat();
    return 0;
}
