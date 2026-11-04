/* sim.h - nucleo da simulacao de Copy-on-Write (COW) no fork() */
#ifndef SIM_H
#define SIM_H

#define MAX_PAGES  1024
#define MAX_FRAMES 4096
#define PAGE_KB    4      /* tamanho de cada pagina/frame em KB */

/* Parametros do experimento */
#define NPAGES   256      /* paginas por processo */
#define NACCESS  1000     /* acessos sorteados */
#define NRATIOS  7        /* quantidade de % de escrita testadas */

extern const int RATIOS[NRATIOS];   /* % de escritas: 0, 10, 25, ... 100 */

typedef enum { MODE_COW, MODE_NAIVE } Mode;

typedef struct {
    int frame;      /* frame fisico */
    int cow;        /* 1 = pagina compartilhada (COW) */
    int writable;   /* 1 = escrita permitida */
} PTE;              /* entrada da tabela de paginas */

typedef struct {
    PTE pt[MAX_PAGES];
    int n;
} Process;

typedef struct {
    int refcount[MAX_FRAMES];   /* quantos processos usam cada frame */
    int used;                   /* frames alocados */
} Memory;

typedef struct {
    long reads, writes;
    long faults;       /* faltas COW tratadas pelo SO */
    long copies;       /* paginas copiadas apos o fork */
    long copied_kb;    /* dados copiados (fork + execucao) */
    int  frames;       /* frames usados no final */
} Stats;

typedef struct {
    Mode mode;
    Memory mem;
    Process proc[2];   /* 0 = pai, 1 = filho */
    Stats st;
    int verbose;       /* 1 = imprime cada operacao */
} Sim;

void sim_init(Sim *s, Mode mode, int npages, int verbose);
void sim_fork(Sim *s);
void sim_access(Sim *s, int who, int page, int is_write);
void sim_finish(Sim *s);

/* Resultado de um experimento: COW vs ingenuo para uma % de escritas */
typedef struct {
    int   escrita_pct;
    Stats cow, ingenuo;
} Linha;

void experimento(Linha out[NRATIOS], unsigned long long seed);

/* Semente: argumento da linha de comando (ex.: ./cow 42) ou, se nao houver, o relogio */
unsigned long long escolher_seed(int argc, char **argv);

#endif