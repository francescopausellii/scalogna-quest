#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gamelib.h"

Giocatore giocatori[4];
short partita = 0;
static Giocatore *setGiocatore(Giocatore *g);
static void stampaGiocatore(Giocatore *g);
static char* stampaClasse(classeGiocatore classe);
static void prendiTesoro(Giocatore *giocatore);
static void avanza(Giocatore* giocatore);
static void indietreggia(Giocatore *giocatore);
static void stampaRisultato(int r);
static int apriPorta(Giocatore *giocatore);
static int affrontaAbitante(Giocatore *giocatore);
static int scappa(Giocatore *giocatore, AbitanteSegrete *abitante);
static int combatti(Giocatore *giocatore, AbitanteSegrete *abitante);
static int giocaPotereSpeciale(Giocatore *giocatore, AbitanteSegrete *abitante);
static AbitanteSegrete *setAbitante(AbitanteSegrete *a);

ZonaSegrete *pFirst; ZonaSegrete *pLast;
static short mappaFinita = 0;
static void generaMappa();
static void inserisciZona(int pos);
static void cancellaZona(int pos);
static void chiudiMappa();
static void stampaMappa();
static void stampaZona(ZonaSegrete *temp);
static char* stampaTipoZona(TipoZona zona);
static char* stampaTipoTesoro(TipoTesoro tesoro);
static char* stampaTipoPorta(TipoPorta porta);

// Utils
static int randomInt(int min, int max);
static void shuffle(Giocatore *giocatori, int n);

void impostaGioco(){
    memset(giocatori, 0, sizeof(Giocatore) * 4); //Inizializzo array di giocatori a NULL
    int n = -1;
    do{
        printf("\e[1;1H\e[2J");
        printf("\e[46m Impostazioni gioco \e[0m\n");
        printf("Inserisci il numero di giocatori (1-4)\n");
        if (n != -1 && (n < 1 || n > 4))
            printf("\e[41m Input non valido! Riprova: \e[0m\n");
        printf("--> ");
        scanf("%d", &n);

        while (getchar() != '\n');
        if(n == -1) n = 0;
    } while (n < 1 || n > 4);

    for (int i = 0; i < n; i++){
        printf("\e[1;1H\e[2J");
        Giocatore *giocatore = (Giocatore *) malloc(sizeof(Giocatore));
        printf("\e[43m Giocatore %d \e[0m\n", i + 1);
        printf("- Inserisci il nome: ");
        fgets(giocatore->nome, 32, stdin);
        giocatore->nome[strlen(giocatore->nome) - 1] = '\0';

        giocatore->classe = -1;
        do{
            if (giocatore->classe != -1 && (giocatore->classe < 1 || giocatore->classe > sizeof(classeGiocatore)))
                printf("\e[41m Input non valido! \e[0m\n");
            printf("- Scegli la tua classe (\e[1;32m[1] BARBARO\e[0m \e[1;33m[2] NANO\e[0m \e[1;34m[3] ELFO\e[0m \e[1;35m[4] MAGO\e[0m): ");
            scanf("%u", &giocatore->classe);

            while (getchar() != '\n');
            if(giocatore->classe==-1) giocatore->classe = 0;
        } while (giocatore->classe < 1 || giocatore->classe > sizeof(classeGiocatore));
        giocatore->classe -= 1;

        giocatore = setGiocatore(giocatore);

        int c = -1;
        do{
            if (c != -1 && (c < 1 || c > 3))
                printf("\e[41m Input non valido! \e[0m\n");
            printf("- Un punto mente può essere sacrificato per un punto vita, e viceversa.\n \e[1;35m[1]\e[0m punto mente\n \e[1;32m[2]\e[0m punto vita\n \e[1;31m[3]\e[0m rifiuta\n--> ");
            scanf("%d", &c);
            
            while (getchar() != '\n');
            if(c==-1) c = 0;
        } while (c < 1 || c > 3);

        switch(c){
            case 1:
                giocatore->mente-=1;
                giocatore->puntiVita+=1;
                break;
            case 2:
                giocatore->puntiVita-=1;
                giocatore->mente+=1;
                break;
        }

        giocatori[i] = *giocatore;
        //stampaGiocatore(&giocatori[i]);
    }
    
    int m = -1;
    do{
        printf("\e[1;1H\e[2J");
        if(pFirst!=NULL){
            printf("----------\n");
            stampaMappa(pFirst);
            printf("----------\n");
        }

        printf("\e[46m Impostazioni mappa \e[0m\n");
        printf("1) Genera 15 zone\n2) Inserisci zona\n3) Cancella zona\n4) Chiudi mappa\n");
        if(m >= 1 && m <= 4) m = -1;
        if (m != -1 && (m < 1 || m > 4)) printf("\e[41m Input non valido! \e[0m \n");
        printf("--> ");
        scanf("%d", &m);

        int p;
        switch(m){
            case 1:
                generaMappa();
                break;
            case 2:
                printf(" - Posizione in cui vuoi creare la zona: ");
                scanf("%d", &p);
                inserisciZona(p);
                break;
            case 3:
                printf(" - Posizione in cui vuoi eliminare la zona: ");
                scanf("%d", &p);
                cancellaZona(p);
                break;
            case 4:
                chiudiMappa();
                break;
        }
        
        while (getchar() != '\n');
    } while (mappaFinita == 0 || (m < 1 || m > 4));

}

void gioca(){
    if(mappaFinita){
        partita = 1;
        shuffle(giocatori, 4);
        for (int i = 0; i < 4; i++) giocatori[i].posizione = pFirst;

        int currplayer = 0, nturno = 1;
        while(partita){
            int c = -1;
            int tesoro = 0;
            int azioniPerTurno = 5;
            //-----
            int aliveplayer = 1;
            for (int i = 0; i < 4; i++) {
                if (strcmp(giocatori[i].nome, "") != 0) {
                    aliveplayer = 0;  // Almeno un giocatore è vivo
                    break;
                }
            }

            if (aliveplayer) {
                printf("\e[1;1H\e[2J");
                printf("Tutti i giocatori sono morti. La partita è finita.\nPremi Enter per tornare al menu principale...\n");
                while (getchar() != '\n');
                break;
            }
            //-----
            if(strcmp(giocatori[currplayer].nome, "") != 0){
                do{
                    printf("\e[1;1H\e[2J");
                    printf("----------\n\e[0;107m TURNO %d \e[0m\e[0m di \e[1;33m%s\e[0m [%d/5] turni rimasti\n", nturno, giocatori[currplayer].nome, azioniPerTurno);
                    switch(c){
                        case 1:
                            avanza(&giocatori[currplayer]);
                            tesoro = 0;
                            break;
                        case 2:
                            indietreggia(&giocatori[currplayer]);
                            tesoro = 0;
                            break;
                        case 3:
                            stampaGiocatore(&giocatori[currplayer]);
                            break;
                        case 4:
                            printf("Zona attuale"); stampaZona(giocatori[currplayer].posizione);
                            break;
                        case 5:
                            if(!tesoro){ prendiTesoro(&giocatori[currplayer]); tesoro = 1; }
                            else{ printf("Hai già preso il \e[4;33mTESORO\e[0m della zona!\n"); }
                            break;
                    }
                    
                    if (giocatori[currplayer].puntiVita == 0) {
                        //printf("\e[1;1H\e[2J");
                        printf("Giocatore \e[1;33m%s\e[0m, sei morto, hai perso il gioco. Addio!\nPremi Enter per passare al turno successivo...\n", giocatori[currplayer].nome);
                        memset(&giocatori[currplayer], 0, sizeof(Giocatore));
                        while (getchar() != '\n');
                        break;
                    }

                    if (giocatori[currplayer].posizione->zonaSuccessiva == NULL) {
                        printf("\e[1;1H\e[2J");
                        printf("Giocatore \e[1;33m%s\e[0m, hai vinto il gioco!\nPremi Enter per tornare al menu principale...\n", giocatori[currplayer].nome);
                        while (getchar() != '\n');
                        partita = 0;  // Imposta partita a 0 per uscire dal ciclo e terminare il gioco
                        break;
                    }
                    printf("----------\n");
                    printf("\e[0;32m[1]\e[0m Avanza\n\e[0;33m[2]\e[0m Indietreggia\n\e[0;34m[3]\e[0m Stampa Giocatore\n\e[0;35m[4]\e[0m Stampa Zona\n\e[0;31m[5]\e[0m Prendi Tesoro\n\e[0;36m[6]\e[0m Passa\n");
                    if (c != -1 && (c < 1 || c > 6)) printf("\e[41m Input non valido! \e[0m\n");
                    printf("--> ");
                    scanf("%d", &c);

                    while (getchar() != '\n');
                    if (c == -1) c = 0; //input sbagliato
                    else if(c == 6) break; //il giocatore passa il turno
                    
                    //if(c!=3 && c!=4) azioniPerTurno--;
                } while (azioniPerTurno > 0 && (c >= 1 && c <= 6));

                nturno+=1;

            }
            currplayer = (currplayer + 1) % 4;
        }
    }
}
    
void terminaGioco(){
}

static Giocatore *setGiocatore(Giocatore *g){
    switch (g->classe){
        case 0:
            g->dadiAttacco = 3;
            g->dadiDifesa = 2;
            g->puntiVita = 8;
            g->mente = randomInt(1, 2);
            g->potereSpeciale = 0;
            break;
        case 1:
            g->dadiAttacco = 2;
            g->dadiDifesa = 2;
            g->puntiVita = 7;
            g->mente = randomInt(2, 3);
            g->potereSpeciale = 1;
            break;
        case 2:
            g->dadiAttacco = 2;
            g->dadiDifesa = 2;
            g->puntiVita = 6;
            g->mente = randomInt(3, 4);
            g->potereSpeciale = 1;
            break;
        case 3:
            g->dadiAttacco = 1;
            g->dadiDifesa = 2;
            g->puntiVita = 4;
            g->mente = randomInt(4, 5);
            g->potereSpeciale = 3;
            break;
    }
    return g;
}

static void stampaGiocatore(Giocatore *g){
    printf("+---\nNome: %s \e[47m %s \e[0m\n", g->nome, stampaClasse(g->classe));
    printf("Punti vita: %d\n", g->puntiVita);
    printf("Dadi attacco: %d\n", g->dadiAttacco);
    printf("Dadi difesa: %d\n", g->dadiDifesa);
    printf("Mente: %d\n", g->mente);
    printf("Potere speciale: %d\n---+\n", g->potereSpeciale);
}

static char* stampaClasse(classeGiocatore classe) {
    switch (classe) {
        case BARBARO: return "Barbaro"; break;
        case NANO: return "Nano"; break;
        case ELFO: return "Elfo"; break;
        case MAGO: return "Mago"; break;
    }

    return NULL;
}

//Partita
static void prendiTesoro(Giocatore *giocatore){
    switch(giocatore->posizione->tesoro){
        case 1:
            giocatore->puntiVita-=2;
            printf("Hai trovato \e[4;32mVELENO!\e[0m perdi 2 punti vita.\n");
            break;
        case 2:
            giocatore->puntiVita+=1;
            printf("Hai trovato \e[4;36mGUARIGIONE!\e[0m guadagni 1 punto vita.\n");
            break;
        case 3:
            giocatore->puntiVita+=2;
            printf("Hai trovato \e[4;34mDOPPIA GUARIGIONE!\e[0m guadagni 2 punti vita.\n");
            break;
        default:
            printf("\e[4;31mNESSUN TESORO\e[0m in questa zona!\n");
            break;
    }
}

static void avanza(Giocatore *giocatore){
    int resPorta = apriPorta(giocatore);
    stampaRisultato(resPorta);
    if(resPorta!=2 && giocatore->posizione->zonaSuccessiva != NULL){
        giocatore->posizione = giocatore->posizione->zonaSuccessiva;
        int dado = randomInt(1, 3);
        if(dado==1){
            int res = affrontaAbitante(giocatore);
            stampaRisultato(res);
        }
    }

    if(giocatore->posizione->zonaSuccessiva==NULL){
        int res = affrontaAbitante(giocatore);
        stampaRisultato(res);
    }
}

static void indietreggia(Giocatore *giocatore){
    if(giocatore->posizione->zonaPrecedente != NULL){
        giocatore->posizione = giocatore->posizione->zonaPrecedente;
        int dado = randomInt(1, 3);
        if(dado==1){
            int res = affrontaAbitante(giocatore);
            stampaRisultato(res);
        }
        stampaRisultato(7);
    } else{
        printf("Ti trovi nella prima stanza delle segrete!\n");
    }
}

static void stampaRisultato(int r){
    switch(r){
         case 0:
            printf("Sei avanzato alla zona successiva.\n");
            break;
        case 1:
            printf("Hai scassinato la porta con successo! Sei avanzato alla zona successiva.\n");
            break;
        case 2:
            printf("Hai sbagliato nel tentativo di scassinare la porta. Ricominci il percorso dalla prima stanza delle segrete.\n");
            break;
        case 3:
            printf("Hai sbagliato nel tentativo di scassinare la porta. Perdi un punto vita.\n");
            break;
        case 4:
            printf("Sei scappato! Sei tornato alla zona precedente.\n");
            break;
        case 5:
            printf("Sei stato sconfitto! Hai perso il gioco.\n");
            break;
        case 6:
            printf("Hai sconfitto l'abitante! Prosegui con il gioco.\n");
            break;
        case 7:
            printf("Sei tornato nella zona precedente.\n");
            break;
    }
}

static int apriPorta(Giocatore *giocatore){
    if(giocatore->posizione->porta == 2){ //da scassinare
        int dado = randomInt(1, 6);
        if (dado <= giocatore->mente){
            return 1; //scassinata
        } else{
            int caso = randomInt(1, 100);
            if (caso < 10) {
                giocatore->posizione = pFirst;
                return 2;
            } else if (caso < 60) {
                giocatore->puntiVita--;
                return 3;
            } else {
                int res = affrontaAbitante(giocatore);
                return res;
            }
        }
    }
    return 0;
}

static int affrontaAbitante(Giocatore *giocatore){
    printf("\e[1;1H\e[2J");
    printf("E' comparso un abitante delle segrete!\n");
    AbitanteSegrete *abitante = (AbitanteSegrete *) malloc(sizeof(AbitanteSegrete));
    abitante = setAbitante(abitante);
    int c = -1;
    do{
        printf("----------\n\e[0;107m Abitante delle segrete \e[0m\n");
        if(c==1){
            int cm = combatti(giocatore, abitante);
            if(cm == 5 || cm == 6) return cm;
        } else if(c==2){
            int ps = giocaPotereSpeciale(giocatore, abitante);
            if(ps) break;
        } else if(c==3){
            int s = scappa(giocatore, abitante);
            if(s==1) return 4;
            if(s==5) return 5;
        }
        printf("----------\n");
        printf("\e[0;32m[1]\e[0m Combatti\n\e[0;33m[2]\e[0m Gioca Potere Speciale\n\e[0;34m[3]\e[0m Scappa\n");

        printf("--> ");
        scanf("%d", &c);
        while (getchar() != '\n');
        if (c == -1) c = 0; //input sbagliato
        printf("\e[1;1H\e[2J");
    } while (c >= 1 && c <= 3);
    return 6;
}

static int scappa(Giocatore *giocatore,  AbitanteSegrete *abitante){
    int res;
    int dado = randomInt(1, 6);
    if (dado <= giocatore->mente){
        if (giocatore->posizione->zonaPrecedente != NULL) giocatore->posizione = giocatore->posizione->zonaPrecedente;
        printf("Sei scappato dal combattimento!\n");
        res = 1;
    } else{
        int adadi = abitante->dadiAttacco;
        int gdadi = giocatore->dadiDifesa / 2;

        for(int i=0; i<adadi; i++){
            if(gdadi>0){
                int gdif = randomInt(1, 6);
                if(gdif!=4 && gdif!=5){
                    if(giocatore->puntiVita >= 1) giocatore->puntiVita-=1;
                    else return 5;
                    printf("Hai perso \e[1;91m-1\e[0m punti vita!\n - %s: %d punti vita\n", giocatore->nome, giocatore->puntiVita);
                } else{
                    printf("Hai difeso \e[0;96m1\e[0m punto vita!\n - %s: %d punti vita\n", giocatore->nome, giocatore->puntiVita);
                }
                gdadi--;
            } else{
                if(giocatore->puntiVita >= 1) giocatore->puntiVita-=1;
                else return 5;
                printf("Hai perso \e[1;91m-1\e[0m punti vita!\n - %s: %d punti vita\n", giocatore->nome, giocatore->puntiVita);
            }
        }
    }
    if(giocatore->puntiVita == 0) res = 5;
    return res;
}

static int combatti(Giocatore *giocatore, AbitanteSegrete *abitante){
    int res = 1;
    int gdadi = giocatore->dadiAttacco;
    int adadi = abitante->dadiDifesa;

    for(int i=0; i<gdadi; i++){
        int attacco = randomInt(1, 6);
        if(attacco >= 1 && attacco <= 3){ // Il giocatore può attaccare
            if(adadi>0){
                int adif = randomInt(1, 6);
                if(adif!=6){
                    if(abitante->puntiVita >= 1) abitante->puntiVita-=1;
                    else return 6;
                    printf("%s ha perso \e[1;91m-1\e[0m punto vita!\n - %s: %d punti vita\n", abitante->nome, abitante->nome, abitante->puntiVita);
                } else{ //l'abitante può difendersi
                    printf("%s ha difeso \e[0;96m1\e[0m punto vita!\n - %s: %d punti vita\n", abitante->nome, abitante->nome, abitante->puntiVita);
                }
                adadi--;
            } else {
                if (abitante->puntiVita >= 1) abitante->puntiVita -= 1;
                else return 6;
                printf("%s ha perso \e[1;91m-1\e[0m punto vita!\n - %s: %d punti vita\n", abitante->nome, abitante->nome, abitante->puntiVita);
            }
        }
    }

    gdadi = giocatore->dadiDifesa;
    adadi = abitante->dadiAttacco;
    for(int i=0; i<adadi; i++){
        int attacco = randomInt(1, 6);
        if(attacco>=1 && attacco<=3){
            if(gdadi>0){
                int gdif = randomInt(1, 6);
                if(gdif!=4 && gdif!=5){
                    if(giocatore->puntiVita >= 1) giocatore->puntiVita-=1;
                    else return 5;
                    printf("Hai perso \e[1;91m-1\e[0m punti vita!\n - %s: %d punti vita\n", giocatore->nome, giocatore->puntiVita);
                } else{
                    printf("Hai difeso \e[0;96m1\e[0m punto vita!\n - %s: %d punti vita\n", giocatore->nome, giocatore->puntiVita);
                }
                gdadi--;
            } else{
                if(giocatore->puntiVita >= 1) giocatore->puntiVita-=1;
                else return 5;
                printf("Hai perso \e[1;91m-1\e[0m punti vita!\n - %s: %d punti vita\n", giocatore->nome, giocatore->puntiVita);
            }
        }
            
    }
    if(giocatore->puntiVita==0) res = 5;
    else if(abitante->puntiVita==0) res = 6;

    return res;
}

static int giocaPotereSpeciale(Giocatore *giocatore, AbitanteSegrete *abitante){
    if(giocatore->potereSpeciale>0){
        giocatore->potereSpeciale-=1;
        abitante->puntiVita = 0;
        printf("Potere speciale rimasto: %d\n",giocatore->potereSpeciale);
        return 1;
    } else{
        printf("Hai terminato gli attacchi speciali!\n");
        return 0;
    }
}

static AbitanteSegrete *setAbitante(AbitanteSegrete *a){
    char *names[] = {
        "Alan Turing", "Grace Hopper", "Linus Torvalds", "Ada Lovelace", "Tim Berners-Lee",
        "John McCarthy", "Donald Knuth", "Guido van Rossum", "Richard Stallman", "Bill Gates"
    };

    strcpy(a->nome, names[randomInt(0, sizeof(names)/sizeof(names[0])-1)]);
    a->puntiVita = randomInt(5, 10);
    a->dadiAttacco = randomInt(1, 3);
    a->dadiDifesa = randomInt(1, 3);
    return a;
}

//Mappa
static void generaMappa(){
    ZonaSegrete *temp;
    for(int i = 0; i < 15; i++) {
        temp = (ZonaSegrete*) malloc(sizeof(ZonaSegrete));
        temp->zona = (TipoZona) randomInt(0, 9);
        temp->tesoro = (TipoTesoro) randomInt(0, 3);
        temp->porta = (TipoPorta) randomInt(0, 2);

        temp->zonaPrecedente = pLast;

        if(pLast != NULL) {
            pLast->zonaSuccessiva = temp;
        }

        pLast = temp;
        temp->zonaSuccessiva = NULL;

        if(pFirst == NULL) {
            pFirst = pLast;
        }
    }
}

static void inserisciZona(int pos){
    ZonaSegrete *nuovaZona = (ZonaSegrete*) malloc(sizeof(ZonaSegrete));
    nuovaZona->zona = (TipoZona) randomInt(0, 9);
    nuovaZona->tesoro = (TipoTesoro) randomInt(0, 3);
    nuovaZona->porta = (TipoPorta) randomInt(0, 2);
    nuovaZona->zonaSuccessiva = NULL;
    nuovaZona->zonaPrecedente = NULL;

    if(pFirst==NULL || pos == 0) {
        if(pFirst != NULL) {
            pFirst->zonaPrecedente = nuovaZona;
            nuovaZona->zonaSuccessiva = pFirst;
        }
        pFirst = nuovaZona;
        if(pLast == NULL) {
            pLast = nuovaZona;
        }
    } else{
        ZonaSegrete *temp = pFirst;
        for(int i = 0; i < pos - 1 && temp != NULL; i++) {
            temp = temp->zonaSuccessiva;
        }

        if(temp == NULL || temp->zonaSuccessiva == NULL) {
            pLast->zonaSuccessiva = nuovaZona;
            nuovaZona->zonaPrecedente = pLast;
            pLast = nuovaZona;
        } else {
            temp->zonaSuccessiva->zonaPrecedente = nuovaZona;
            nuovaZona->zonaSuccessiva = temp->zonaSuccessiva;
            temp->zonaSuccessiva = nuovaZona;
            nuovaZona->zonaPrecedente = temp;
        }
    }
}

static void cancellaZona(int pos){
    if(pFirst == NULL) {
        printf("La lista è vuota\n");
        return;
    }

    ZonaSegrete *temp = pFirst;
    for(int i = 0; i < pos && temp != NULL; i++) {
        temp = temp->zonaSuccessiva;
    }

    if(temp == NULL) {
        temp = pLast;
    }

    if(temp->zonaPrecedente != NULL) {
        temp->zonaPrecedente->zonaSuccessiva = temp->zonaSuccessiva;
    } else {
        pFirst = temp->zonaSuccessiva;
    }

    if(temp->zonaSuccessiva != NULL) {
        temp->zonaSuccessiva->zonaPrecedente = temp->zonaPrecedente;
    } else {
        pLast = temp->zonaPrecedente;
    }

    free(temp);
}

static void chiudiMappa(){
    ZonaSegrete *temp = pFirst;
    int count = 0;

    while(temp != NULL && count < 15) {
        count++;
        temp = temp->zonaSuccessiva;
    }

    if(count >= 15){
        mappaFinita = 1;
    } else{
        mappaFinita = 0;
    }
}

static void stampaMappa(){
    ZonaSegrete *temp = pFirst;
    int i = 0;
    while(temp != NULL) {
        printf("%d", i);
        stampaZona(temp);
        //printf("\e[4;37mZona %d\e[0m: %s, Tesoro: %s, Porta: %s\n", i, stampaTipoZona(temp->zona), stampaTipoTesoro(temp->tesoro), stampaTipoPorta(temp->porta));
        //printf("Indirizzo di Zona %d: %p\n", i, temp);
        //printf("Indirizzi di Zona %d: zonaSuccessiva: %p, zonaPrecedente: %p\n", i, temp->zonaSuccessiva, temp->zonaPrecedente);
        temp = temp->zonaSuccessiva;
        i++;
    }
}

static void stampaZona(ZonaSegrete *temp){
    printf("] \e[1;95mZona %s\e[0m: ", stampaTipoZona(temp->zona));
    printf("%s, ", !temp->tesoro ? stampaTipoTesoro(temp->tesoro) : "Tesoro presente");
    printf("%s\n", !temp->porta ? stampaTipoPorta(temp->porta) : "Porta presente");
}

static char* stampaTipoZona(TipoZona zona) {
    switch (zona) {
        case CORRIDOIO: return "Corridoio"; break;
        case SCALA: return "Scala"; break;
        case SALA_BANCHETTO: return "Sala banchetto"; break;
        case MAGAZZINO: return "Magazzino"; break;
        case GIARDINO: return "Giardino"; break;
        case POSTO_GUARDIA: return "Posto guardia"; break;
        case PRIGIONE: return "Prigione"; break;
        case CUCINA: return "Cucina"; break;
        case ARMERIA: return "Armeria"; break;
        case TEMPIO: return "Tempio"; break;
    }

    return NULL;
}

static char* stampaTipoTesoro(TipoTesoro tesoro) {
    switch (tesoro) {
        case NESSUN_TESORO: return "Nessun tesoro"; break;
        case VELENO: return "Veleno"; break;
        case GUARIGIONE: return "Guarigione"; break;
        case DOPPIA_GUARIGIONE: return "Doppia guarigione"; break;
    }

    return NULL;
}

static char* stampaTipoPorta(TipoPorta porta) {
    switch (porta) {
        case NESSUNA_PORTA: return "Nessuna porta"; break;
        case PORTA_NORMALE: return "Porta normale"; break;
        case PORTA_DA_SCASSINARE: return "Porta da scassinare"; break;
    }

    return NULL;
}

//Utils
static int randomInt(int min, int max){
    return min + rand() % (max - min + 1); // Restituisci un numero casuale tra min e max (inclusi)
}

static void shuffle(Giocatore *giocatori, int n) {
  Giocatore temp;

  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (rand() % 2 == 0) {
        temp = giocatori[i];
        giocatori[i] = giocatori[j];
        giocatori[j] = temp;
      }
    }
  }
}