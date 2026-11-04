/* Copy-on-Write: versao minima para observar em sala
 * Compilar e rodar: gcc -o cow_simples cow_simples.c && ./cow_simples
 */
#include <stdio.h>

#define N 4              /* paginas de cada processo */

int pai[N], filho[N];    /* tabela de paginas: pagina -> frame */
int donos[16];           /* quantos processos usam cada frame */
int proximo = 0;         /* proximo frame livre */

void mostrar(void) {
    printf("  PAI:   ");
    for (int i = 0; i < N; i++) printf("Pag%d->F%d  ", i, pai[i]);
    printf("\n  FILHO: ");
    for (int i = 0; i < N; i++) {
        if (filho[i] < 0) printf("Pag%d->--  ", i);   /* filho ainda nao existe */
        else              printf("Pag%d->F%d  ", i, filho[i]);
    }
    printf("\n  Donos: ");
    for (int f = 0; f < proximo; f++) printf("F%d=%d  ", f, donos[f]);
    printf("\n\n");
}

void escrever(const char *nome, int *tabela, int pag) {
    int f = tabela[pag];
    printf("%s escreve na pagina %d (frame F%d)\n", nome, pag, f);

    if (donos[f] > 1) {                 /* compartilhado: copia */
        int novo = proximo++;
        donos[f]--;
        donos[novo] = 1;
        tabela[pag] = novo;
        printf("  -> frame compartilhado: COPIA F%d para F%d\n", f, novo);
    } else {                            /* so um dono: escreve direto */
        printf("  -> unico dono: escreve direto, sem copiar\n");
    }
    mostrar();
}

int main(void) {
    for (int i = 0; i < N; i++) {       /* pai: cada pagina em um frame */
        pai[i] = proximo;
        filho[i] = -1;
        donos[proximo++] = 1;
    }
    printf("=== Pai criado ===\n");
    mostrar();

    for (int i = 0; i < N; i++) {       /* fork COW: filho aponta pros mesmos frames */
        filho[i] = pai[i];
        donos[pai[i]]++;
    }
    printf("=== Depois do fork (nada foi copiado) ===\n");
    mostrar();

    escrever("FILHO", filho, 1);
    escrever("PAI",   pai,   1);
    escrever("FILHO", filho, 1);
    escrever("PAI",   pai,   3);

    printf("Frames usados: %d (o fork ingenuo usaria %d)\n", proximo, 2 * N);
    return 0;
}
