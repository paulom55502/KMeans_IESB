# Projeto de Programação Paralela --- K-means (K-médias)

**Disciplina:** Programação Paralela --- IESB\
**Tema:** 10 --- Agrupamento por K-médias\


## 1. Objetivo

Implementar o algoritmo K-means em três versões --- sequencial, paralela
com OpenMP e distribuída com MPI --- e comparar seus tempos de execução
para diferentes quantidades de pontos.

O K-means agrupa um conjunto de pontos em `K` grupos. Em cada iteração,
cada ponto é associado ao centróide mais próximo; em seguida, os
centróides são recalculados pela média dos pontos de cada grupo. O
processo termina quando os centróides deixam de mudar significativamente
ou quando o limite de iterações é atingido.

## 2. Organização do projeto

``` text
KMeans_IESB/
├── common/
│   └── kmeans_common.hpp
├── sequencial/
│   └── kmeans.cpp
├── openmp/
│   └── kmeans_openmp.cpp
├── mpi/
│   └── kmeans_mpi.cpp
├── scripts/
│   └── medir.sh
├── resultados/
│   └── medicoes.csv
├── Makefile
├── README.md
└── AUTORES.md
```

O arquivo `common/kmeans_common.hpp` reúne funções compartilhadas, como
estruturas de dados, geração determinística dos pontos e operações
comuns do algoritmo.

## 3. Requisitos

Ambiente Linux ou WSL2 com:

-   compilador C++ (`g++`);
-   `make`;
-   OpenMP (normalmente incluído no GCC);
-   OpenMPI (`mpicxx` e `mpirun`).

No Ubuntu/WSL2, instale as ferramentas com:

``` bash
sudo apt update
sudo apt install -y build-essential g++ make openmpi-bin libopenmpi-dev
```

## 4. Compilação

Na pasta raiz do projeto, execute:

``` bash
make
```

Para remover os executáveis gerados:

``` bash
make clean
```

## 5. Como executar

Os programas recebem os argumentos nesta ordem:

``` text
N D K MAX_ITER
```

-   `N`: quantidade de pontos;
-   `D`: quantidade de dimensões de cada ponto;
-   `K`: quantidade de grupos;
-   `MAX_ITER`: número máximo de iterações.

Exemplo de configuração usada nos testes: 100.000 pontos, 4 dimensões, 5
grupos e até 100 iterações.

### Versão sequencial

``` bash
./sequencial/kmeans 100000 4 5 100
```

### Versão OpenMP

``` bash
OMP_NUM_THREADS=4 ./openmp/kmeans_openmp 100000 4 5 100
```

Altere `OMP_NUM_THREADS` para testar diferentes quantidades de threads,
por exemplo 1, 2, 4 ou 8.

### Versão MPI

``` bash
mpirun -np 4 ./mpi/kmeans_mpi 100000 4 5 100
```

Altere `-np` para testar 1, 2 ou 4 processos. Em algumas instalações,
pode ser necessário permitir explicitamente a execução com mais
processos do que núcleos disponíveis; use essa opção somente se o MPI
apresentar esse aviso.

## 6. Estratégias de paralelização

### OpenMP

A distribuição dos pontos entre as threads é feita com uma região
paralela e `omp for schedule(static)`. Cada thread mantém acumuladores
locais para as somas e quantidades de pontos de cada grupo. Ao final da
etapa, esses resultados parciais são combinados para atualizar os
centróides.

O uso de acumuladores locais evita que várias threads atualizem
simultaneamente as mesmas variáveis, prevenindo condições de corrida
nessa etapa.

### MPI

Os pontos são divididos entre os processos. O processo principal gera os
dados e distribui partes dos pontos usando `MPI_Scatterv`, permitindo
partes de tamanhos diferentes. Os centróides são compartilhados com
`MPI_Bcast`.

Cada processo calcula as somas e quantidades locais dos grupos. Os
resultados parciais são combinados com `MPI_Allreduce`, de modo que
todos os processos obtenham os valores necessários para atualizar os
centróides. O tempo final considera o processo mais lento.

## 7. Medição de desempenho

O script automatiza os testes com três tamanhos de entrada e diferentes
números de threads/processos:

``` bash
bash scripts/medir.sh
```

Os resultados são gravados em:

``` text
resultados/medicoes.csv
```

O CSV contém a versão executada, tamanho da entrada, dimensões,
quantidade de grupos, limite de iterações, número de trabalhadores e
tempo em segundos.

### Resultados obtidos

A tabela apresenta os melhores tempos registrados nos testes realizados.
Os valores são de execuções individuais e podem variar conforme a
máquina e a carga do sistema.

  -----------------------------------------------------------------------
             Pontos        Sequencial   OpenMP (melhor)      MPI (melhor)
  ----------------- ----------------- ----------------- -----------------
             10.000        0,019272 s     0,006476 s (8     0,007726 s (4
                                               threads)        processos)

             50.000        0,156193 s     0,084580 s (4     0,056907 s (4
                                               threads)        processos)

            100.000        0,303943 s     0,085308 s (8     0,092780 s (4
                                               threads)        processos)
  -----------------------------------------------------------------------

Nos testes de validação, as três versões produziram os mesmos centróides
para a configuração verificada (`N=100000`, `D=4`, `K=5`,
`MAX_ITER=100`).

## 8. Observações sobre os resultados

O desempenho depende do tamanho da entrada e da quantidade de threads ou
processos. Para entradas menores, o custo de iniciar threads ou realizar
comunicação entre processos pode reduzir o ganho. Para entradas maiores,
a divisão do trabalho tende a compensar melhor esse custo.

Os tempos apresentados são referências obtidas no ambiente de teste do
grupo, não garantias de desempenho em outras máquinas. Para uma análise
mais robusta, recomenda-se executar cada configuração várias vezes e
comparar a média.

