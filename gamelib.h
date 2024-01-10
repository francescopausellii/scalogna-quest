// Dichiarazione delle funzioni da chiamare in main.c
// Definizione dei tipi necessari alla libreria
void impostaGioco();
void gioca();
void terminaGioco();

typedef enum{
    BARBARO,
    NANO,
    ELFO,
    MAGO
} classeGiocatore;

typedef enum{
    CORRIDOIO,
    SCALA,
    SALA_BANCHETTO,
    MAGAZZINO,
    GIARDINO,
    POSTO_GUARDIA,
    PRIGIONE,
    CUCINA,
    ARMERIA,
    TEMPIO
} TipoZona;

typedef enum{
    NESSUN_TESORO,
    VELENO,
    GUARIGIONE,
    DOPPIA_GUARIGIONE
} TipoTesoro;

typedef enum{
    NESSUNA_PORTA,
    PORTA_NORMALE,
    PORTA_DA_SCASSINARE
} TipoPorta;

typedef struct ZonaSegrete{
    TipoZona zona;
    TipoTesoro tesoro;
    TipoPorta porta;
    struct ZonaSegrete *zonaSuccessiva;
    struct ZonaSegrete *zonaPrecedente;
} ZonaSegrete;

typedef struct{
    char nome[36];
    classeGiocatore classe;
    struct ZonaSegrete *posizione;
    unsigned char puntiVita;
    unsigned char dadiAttacco;
    unsigned char dadiDifesa;
    unsigned char mente;
    unsigned char potereSpeciale;
} Giocatore;

typedef struct{
    char nome[36];
    unsigned char puntiVita;
    unsigned char dadiAttacco;
    unsigned char dadiDifesa;
} AbitanteSegrete;