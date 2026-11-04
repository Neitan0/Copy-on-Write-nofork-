/* main.c - demonstracao e tabela de resultados no terminal */
#include <stdio.h>
#include "sim.h"

static void demo(void) {
    printf("==============================================================\n");
    printf(" PARTE 1: DEMONSTRACAO PASSO A PASSO (8 paginas, COW)\n");
    printf("==============================================================\n");

    Sim s;
    sim_init(&s, MODE_COW, 8, 1);
    printf("  Pai criado com 8 paginas -> frames 0..7\n");
    sim_fork(&s);

    /* {quem (0=pai, 1=filho), pagina, escrita?} */
    int ops[][3] = {
        {0, 2, 0}, {1, 2, 0}, {1, 2, 1}, {1, 2, 1},
        {0, 5, 1}, {1, 5, 1}, {0, 7, 0}, {1, 0, 1}
    };
    int nops = (int)(sizeof(ops) / sizeof(ops[0]));
    for (int i = 0; i < nops; i++)
        sim_access(&s, ops[i][0], ops[i][1], ops[i][2]);
    sim_finish(&s);

    printf("\n  Resultado: %d frames = %d KB (ingenuo usaria 16 frames = 64 KB)\n",
           s.st.frames, s.st.frames * PAGE_KB);
    printf("  %ld copias apos o fork = %ld KB copiados (ingenuo copiou 32 KB no fork)\n",
           s.st.copies, s.st.copied_kb);
    printf("  %ld faltas COW tratadas pelo SO\n\n", s.st.faults);
}

static void tabela(unsigned long long seed) {
    Linha r[NRATIOS];
    experimento(r, seed);

    printf("==============================================================\n");
    printf(" PARTE 2: EXPERIMENTO (%d paginas por processo, %d acessos, seed %llu)\n", NPAGES, NACCESS, seed);
    printf("==============================================================\n");
    printf("%-8s | %-10s %-10s %-9s | %-12s %-12s | %-8s %-8s\n",
           "Escrita", "Mem COW", "Mem Ing.", "Economia", "Copiado COW", "Copiado Ing.", "Copias", "Faltas");
    printf("%-8s | %-10s %-10s %-9s | %-12s %-12s | %-8s %-8s\n",
           "(%)", "(KB)", "(KB)", "(%)", "(KB)", "(KB)", "COW", "COW");
    printf("---------+--------------------------------+---------------------------+-----------------\n");

    for (int i = 0; i < NRATIOS; i++) {
        int mc = r[i].cow.frames * PAGE_KB;
        int mn = r[i].ingenuo.frames * PAGE_KB;
        double eco = 100.0 * (mn - mc) / mn;
        printf("%-8d | %-10d %-10d %7.1f%% | %-12ld %-12ld | %-8ld %-8ld\n",
               r[i].escrita_pct, mc, mn, eco,
               r[i].cow.copied_kb, r[i].ingenuo.copied_kb,
               r[i].cow.copies, r[i].cow.faults);
    }
    printf("\nCada pagina/frame = %d KB. \"Faltas\" = vezes que o SO foi acionado por escrita em pagina compartilhada.\n", PAGE_KB);
}

int main(int argc, char **argv) {
    demo();
    tabela(escolher_seed(argc, argv));
    return 0;
}