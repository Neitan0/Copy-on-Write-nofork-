/* exportar.c - gera CSV e graficos SVG para usar nos slides.
 * Nao faz parte da simulacao: so le os resultados de experimento(). */
#include <stdio.h>
#include "sim.h"

static void svg_line(const char *file, const char *title, const char *ylabel,
                     const int *xs, int n, const double *a, const double *b,
                     const char *la, const char *lb) {
    FILE *f = fopen(file, "w");
    if (!f) { perror(file); return; }
    const int W = 760, H = 460, L = 80, R = 30, T = 60, B = 70;
    double ymax = 0;
    for (int i = 0; i < n; i++) { if (a[i] > ymax) ymax = a[i]; if (b[i] > ymax) ymax = b[i]; }
    ymax *= 1.1; if (ymax <= 0) ymax = 1;
    int pw = W - L - R, ph = H - T - B;

    fprintf(f, "<svg xmlns='http://www.w3.org/2000/svg' width='%d' height='%d' viewBox='0 0 %d %d' font-family='Arial,Helvetica,sans-serif'>\n", W, H, W, H);
    fprintf(f, "<rect width='100%%' height='100%%' fill='white'/>\n");
    fprintf(f, "<text x='%d' y='32' font-size='20' font-weight='bold' text-anchor='middle'>%s</text>\n", W / 2, title);

    for (int g = 0; g <= 5; g++) {
        double v = ymax * g / 5.0;
        int y = T + ph - (int)(ph * g / 5.0);
        fprintf(f, "<line x1='%d' y1='%d' x2='%d' y2='%d' stroke='#ddd'/>\n", L, y, W - R, y);
        fprintf(f, "<text x='%d' y='%d' font-size='12' text-anchor='end'>%.0f</text>\n", L - 8, y + 4, v);
    }
    fprintf(f, "<line x1='%d' y1='%d' x2='%d' y2='%d' stroke='black'/>\n", L, T, L, T + ph);
    fprintf(f, "<line x1='%d' y1='%d' x2='%d' y2='%d' stroke='black'/>\n", L, T + ph, W - R, T + ph);
    fprintf(f, "<text x='%d' y='%d' font-size='13' text-anchor='middle'>%% de escritas nos acessos</text>\n", L + pw / 2, H - 15);
    fprintf(f, "<text x='20' y='%d' font-size='13' text-anchor='middle' transform='rotate(-90 20 %d)'>%s</text>\n", T + ph / 2, T + ph / 2, ylabel);

    const double *ser[2] = { a, b };
    const char *cor[2] = { "#1f77b4", "#d62728" };
    for (int k = 0; k < 2; k++) {
        fprintf(f, "<polyline fill='none' stroke='%s' stroke-width='3' points='", cor[k]);
        for (int i = 0; i < n; i++) {
            double x = L + (n > 1 ? pw * (double)i / (n - 1) : 0);
            double y = T + ph - ph * ser[k][i] / ymax;
            fprintf(f, "%.1f,%.1f ", x, y);
        }
        fprintf(f, "'/>\n");
        for (int i = 0; i < n; i++) {
            double x = L + (n > 1 ? pw * (double)i / (n - 1) : 0);
            double y = T + ph - ph * ser[k][i] / ymax;
            fprintf(f, "<circle cx='%.1f' cy='%.1f' r='4.5' fill='%s'/>\n", x, y, cor[k]);
        }
    }
    for (int i = 0; i < n; i++) {
        double x = L + (n > 1 ? pw * (double)i / (n - 1) : 0);
        fprintf(f, "<text x='%.1f' y='%d' font-size='12' text-anchor='middle'>%d%%</text>\n", x, T + ph + 20, xs[i]);
    }
    fprintf(f, "<rect x='%d' y='%d' width='14' height='14' fill='%s'/><text x='%d' y='%d' font-size='13'>%s</text>\n", L + 15, T + 8, cor[0], L + 36, T + 20, la);
    fprintf(f, "<rect x='%d' y='%d' width='14' height='14' fill='%s'/><text x='%d' y='%d' font-size='13'>%s</text>\n", L + 15, T + 30, cor[1], L + 36, T + 42, lb);
    fprintf(f, "</svg>\n");
    fclose(f);
}

int main(int argc, char **argv) {
    unsigned long long seed = escolher_seed(argc, argv);
    Linha r[NRATIOS];
    experimento(r, seed);

    double mem_cow[NRATIOS], mem_ing[NRATIOS], cp_cow[NRATIOS], cp_ing[NRATIOS];
    FILE *csv = fopen("resultados.csv", "w");
    if (csv) fprintf(csv, "escrita_pct,mem_cow_kb,mem_ingenuo_kb,economia_pct,copiado_cow_kb,copiado_ingenuo_kb,copias_cow,faltas_cow\n");

    for (int i = 0; i < NRATIOS; i++) {
        int mc = r[i].cow.frames * PAGE_KB;
        int mn = r[i].ingenuo.frames * PAGE_KB;
        if (csv) fprintf(csv, "%d,%d,%d,%.2f,%ld,%ld,%ld,%ld\n",
                         r[i].escrita_pct, mc, mn, 100.0 * (mn - mc) / mn,
                         r[i].cow.copied_kb, r[i].ingenuo.copied_kb,
                         r[i].cow.copies, r[i].cow.faults);
        mem_cow[i] = mc;  mem_ing[i] = mn;
        cp_cow[i] = (double)r[i].cow.copied_kb;
        cp_ing[i] = (double)r[i].ingenuo.copied_kb;
    }
    if (csv) fclose(csv);

    svg_line("grafico_memoria.svg", "Memoria usada: COW vs fork ingenuo", "Memoria usada (KB)",
             RATIOS, NRATIOS, mem_cow, mem_ing, "Copy-on-Write", "Copia completa (ingenuo)");
    svg_line("grafico_copiado.svg", "Dados copiados: COW vs fork ingenuo", "Dados copiados (KB)",
             RATIOS, NRATIOS, cp_cow, cp_ing, "Copy-on-Write", "Copia completa (ingenuo)");

    printf("Seed usada: %llu\n", seed);
    printf("Arquivos gerados: resultados.csv, grafico_memoria.svg, grafico_copiado.svg\n");
    return 0;
}