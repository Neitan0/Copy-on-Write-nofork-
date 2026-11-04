CC = gcc
CFLAGS = -Wall -Wextra -O2

all: cow exportar

cow: main.c sim.c sim.h
	$(CC) $(CFLAGS) -o cow main.c sim.c

exportar: exportar.c sim.c sim.h
	$(CC) $(CFLAGS) -o exportar exportar.c sim.c

clean:
	rm -f cow exportar resultados.csv grafico_*.svg