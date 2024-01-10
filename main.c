#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gamelib.h"

int main(){
    time_t t;
    srand((unsigned) time(&t)); 
    
    int c= -1;
    do{
        printf("\e[1;1H\e[2J");
        printf("█▀ █▀▀ ▄▀█ █░░ █▀█ █▀▀ █▄░█ ▄▀█ ▄▄ █▀█ █░█ █▀▀ █▀ ▀█▀\n"
               "▄█ █▄▄ █▀█ █▄▄ █▄█ █▄█ █░▀█ █▀█ ░░ ▀▀█ █▄█ ██▄ ▄█ ░█░\n");
        printf("1) Imposta gioco\n2) Gioca\n3) Termina gioco\n");
        if (c != -1 && (c < 1 || c > 3)) printf("\e[41m Input non valido! \e[0m \n");
        printf("--> ");
        scanf("%d", &c);
        switch (c){
            case 1:
                printf("c: %d\n", c);
                impostaGioco();
                break;
            case 2:
                gioca();
                break;
            case 3:
                terminaGioco();
                break;
        }

    } while (c != 3);

    return 0;
}