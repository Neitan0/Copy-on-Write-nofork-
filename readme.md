# Simulação de Copy-on-Write (COW) no fork()

Compara o `fork()` com COW contra o fork "ingênuo" (copia tudo na hora), variando a % de escritas.

## Arquivos
- `sim.h` / `sim.c`: simulação (tabelas de páginas, frames com contador de referências, fork, read/write)
- `main.c`: demo passo a passo e tabela de resultados no terminal
- `exportar.c`: gera `resultados.csv` e os gráficos `.svg` (só para os slides)

## Como rodar
```bash
make            # compila cow e exportar
./cow           # demo + tabela (sorteio diferente a cada execução)
./cow 42        # usa a semente 42 (resultado reproduzível)
./exportar      # gera CSV e gráficos SVG (também aceita a semente)
```