/* sim.c - implementacao da simulacao (sem nenhuma geracao de arquivos) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sim.h"

const int RATIOS[NRATIOS] = { 0, 10, 25, 50, 75, 90, 100 };

/* gerador pseudo-aleatorio simples (mesmo resultado em qualquer SO) */
static unsigned long long rng_state;
static void rng_seed(unsigned long long s) { rng_state = s ? s : 88172645463325252ULL; }
static unsigned long long rng_next(void) {
    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 7;
    rng_state ^= rng_state << 17;
    return rng_state;
}

static int alloc_frame(Sim *s) {
    if (s->mem.used >= MAX_FRAMES) {
        fprintf(stderr, "Memoria fisica esgotada!\n");
        exit(1);
    }
    int f = s->mem.used++;
    s->mem.refcount[f] = 1;
    return f;
}

/* Cria o pai com npages paginas, cada uma em um frame proprio */
void sim_init(Sim *s, Mode mode, int npages, int verbose) {
    memset(s, 0, sizeof(*s));
    s->mode = mode;
    s->verbose = verbose;
    s->proc[0].n = npages;
    s->proc[1].n = npages;
    for (int i = 0; i < npages; i++) {
        PTE *p = &s->proc[0].pt[i];
        p->frame = alloc_frame(s);
        p->cow = 0;
        p->writable = 1;
    }
}

/* fork: COW compartilha os frames; ingenuo copia tudo */
void sim_fork(Sim *s) {
    int n = s->proc[0].n;
    for (int i = 0; i < n; i++) {
        PTE *pai = &s->proc[0].pt[i];
        PTE *fil = &s->proc[1].pt[i];
        if (s->mode == MODE_COW) {
            pai->cow = 1; pai->writable = 0;
            *fil = *pai;                        /* aponta pro mesmo frame */
            s->mem.refcount[pai->frame]++;
        } else {
            fil->frame = alloc_frame(s);        /* frame novo + copia */
            fil->cow = 0; fil->writable = 1;
            s->st.copied_kb += PAGE_KB;
        }
    }
    if (s->verbose)
        printf("  fork(): %s | frames em uso = %d (%d KB) | dados copiados no fork = %ld KB\n",
               s->mode == MODE_COW ? "COW" : "ingenuo", s->mem.used,
               s->mem.used * PAGE_KB, s->st.copied_kb);
}

/* Leitura ou escrita de um processo (0 = pai, 1 = filho) */
void sim_access(Sim *s, int who, int page, int is_write) {
    const char *nome = who == 0 ? "PAI  " : "FILHO";
    PTE *pte = &s->proc[who].pt[page];

    if (!is_write) {
        s->st.reads++;
        if (s->verbose)
            printf("  %s le    pag %d -> frame %d (sem custo extra)\n", nome, page, pte->frame);
        return;
    }
    s->st.writes++;

    if (s->mode == MODE_COW && !pte->writable && pte->cow) {
        s->st.faults++;                          /* falta COW */
        int old = pte->frame;
        if (s->mem.refcount[old] > 1) {          /* frame compartilhado: copia */
            int novo = alloc_frame(s);
            s->mem.refcount[old]--;
            pte->frame = novo;
            s->st.copies++;
            s->st.copied_kb += PAGE_KB;
            if (s->verbose)
                printf("  %s ESCREVE pag %d -> FALTA COW: copia frame %d -> %d (refs do antigo: %d)\n",
                       nome, page, old, novo, s->mem.refcount[old]);
        } else if (s->verbose) {                 /* unico dono: so libera */
            printf("  %s ESCREVE pag %d -> FALTA COW: unico dono do frame %d, so libera escrita (sem copia)\n",
                   nome, page, old);
        }
        pte->writable = 1;
        pte->cow = 0;
    } else if (s->verbose) {
        printf("  %s ESCREVE pag %d -> frame %d (escrita direta)\n", nome, page, pte->frame);
    }
}

void sim_finish(Sim *s) { s->st.frames = s->mem.used; }

/* Roda uma simulacao completa e devolve as metricas */
static Stats run(Mode mode, int wpct, unsigned long long seed) {
    Sim s;
    sim_init(&s, mode, NPAGES, 0);
    sim_fork(&s);
    rng_seed(seed);                              /* mesma sequencia nos 2 modos */
    for (int i = 0; i < NACCESS; i++) {
        int who   = (int)(rng_next() % 2);
        int page  = (int)(rng_next() % NPAGES);
        int write = (int)(rng_next() % 100) < wpct;
        sim_access(&s, who, page, write);
    }
    sim_finish(&s);
    return s.st;
}

/* Roda COW e ingenuo para cada % de escritas */
void experimento(Linha out[NRATIOS], unsigned long long seed) {
    for (int i = 0; i < NRATIOS; i++) {
        out[i].escrita_pct = RATIOS[i];
        out[i].cow     = run(MODE_COW,   RATIOS[i], seed);
        out[i].ingenuo = run(MODE_NAIVE, RATIOS[i], seed);
    }
}

unsigned long long escolher_seed(int argc, char **argv) {
    if (argc > 1) return strtoull(argv[1], NULL, 10);
    return (unsigned long long)time(NULL);
}