# Projeto de Programação Paralela — K-means (IESB 2026/2)

## 1. O que foi implementado
O K-means agrupa N pontos com D dimensões em K grupos. Em cada iteração:
1. cada ponto é associado ao centroide mais próximo (distância euclidiana ao quadrado);
2. são somadas as coordenadas por grupo e contabilizada a quantidade de pontos;
3. os centroides são atualizados pela média;
4. o algoritmo para quando o maior deslocamento dos centroides fica abaixo da tolerância ou atinge MAX_ITER.

Complexidade aproximada: **O(N × K × D × I)**, onde I é o número de iterações.

Os dados são gerados deterministicamente por índice, e os centroides iniciais são os primeiros K pontos. Assim, as versões recebem os mesmos dados e a mesma inicialização.

## 2. Ambiente recomendado no Windows
Use WSL2 + Ubuntu. No PowerShell (como administrador, se solicitado):
```powershell
wsl --install -d Ubuntu
```
Reinicie se o Windows solicitar e abra o aplicativo Ubuntu. Dentro do Ubuntu:
```bash
sudo apt update
sudo apt install -y build-essential g++ make openmpi-bin libopenmpi-dev
```
Entre na pasta do projeto. Se o ZIP estiver em Downloads do Windows, normalmente:
```bash
cd /mnt/c/Users/SEU_USUARIO/Downloads
unzip KMeans_IESB.zip
cd KMeans_IESB
```
Substitua `SEU_USUARIO` pelo nome da pasta de usuário do Windows.

## 3. Compilar
Na raiz do projeto:
```bash
make clean
make
```
Se `make` não estiver instalado:
```bash
sudo apt install make
```

## 4. Executar
Argumentos: `N D K MAX_ITER`. Exemplo:
```bash
./sequencial/kmeans 100000 4 5 100
OMP_NUM_THREADS=4 ./openmp/kmeans_openmp 100000 4 5 100
mpirun -np 4 ./mpi/kmeans_mpi 100000 4 5 100
```
No OpenMP, teste 1, 2, 4 e 8 threads:
```bash
for t in 1 2 4 8; do
  OMP_NUM_THREADS=$t ./openmp/kmeans_openmp 100000 4 5 100
done
```
No MPI, teste 1, 2 e 4 processos:
```bash
mpirun -np 1 ./mpi/kmeans_mpi 100000 4 5 100
mpirun -np 2 ./mpi/kmeans_mpi 100000 4 5 100
mpirun -np 4 ./mpi/kmeans_mpi 100000 4 5 100
```
Se o OpenMPI avisar que está sendo executado como root, não use root desnecessariamente. Em Ubuntu/WSL rode como seu usuário normal.

## 5. Organização
- `sequencial/`: referência sequencial.
- `openmp/`: versão com memória compartilhada.
- `mpi/`: versão com memória distribuída.
- `common/`: funções comuns para geração de dados, distância e parâmetros.
- `scripts/`: automação de medições.
- `resultados/`: CSV e gráficos gerados.

## 6. Como foi paralelizado
### OpenMP
O laço de atribuição percorre os pontos. Cada iteração lê os centroides e um ponto, e escreve somente nos acumuladores privados da thread. Ao final, os acumuladores locais são combinados. Isso evita que duas threads atualizem simultaneamente a mesma soma/contagem. Foi usado `schedule(static)` porque o trabalho por ponto é semelhante.

### MPI
Os pontos são divididos entre processos com `MPI_Scatterv`, permitindo partes de tamanhos diferentes quando N não é divisível pelo número de processos. Os centroides iniciais são enviados com `MPI_Bcast`. Cada processo calcula suas somas e contagens locais; `MPI_Allreduce` combina esses vetores e entrega o resultado a todos os processos, pois todos precisam atualizar os mesmos centroides.

## 7. Fórmulas de desempenho
- Speedup: `S = T_sequencial / T_paralelo`
- Eficiência: `E = S / P`, em que P é o número de threads/processos.
- Lei de Amdahl: `S_max = 1 / (f + (1-f)/P)`, em que f é a fração serial estimada/medida.

Faça comparação forte: mantenha N, D, K e MAX_ITER fixos e varie P. Registre também CPU, RAM, sistema operacional, compilador e versões.

**Não preencha resultados com valores inventados.** Rode os testes e guarde as saídas reais.

## 8. Limitações e cuidados
- A ordem das somas em ponto flutuante pode produzir pequenas diferenças entre versões.
- Compare centroides com tolerância numérica, não exija igualdade textual perfeita.
- Se um cluster ficar vazio, o código mantém seu centroide anterior.
- O gerador e a inicialização foram escolhidos para reprodutibilidade acadêmica; não representam necessariamente a melhor inicialização para dados reais.
- O cronômetro MPI começa antes da geração dos dados no processo raiz e inclui geração, distribuição e iterações. As versões sequencial e OpenMP também incluem geração e inicialização no tempo total.

## 9. Antes da entrega
- [ ] Compilar as três versões.
- [ ] Conferir que todas terminam sem erro.
- [ ] Comparar centroides (com tolerância).
- [ ] Rodar pelo menos três tamanhos de N.
- [ ] OpenMP: 1, 2, 4 e 8 threads.
- [ ] MPI: 1, 2 e 4 processos.
- [ ] Salvar resultados reais em CSV e gerar gráficos.
- [ ] Completar AUTORES.md e README com hardware e metodologia.
