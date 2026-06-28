# Linear Hashing

Projeto em C++ que implementa uma tabela de hash dinâmica usando hashing linear com tratamento de overflow por páginas e redimensionamento controlado por fator de carga.

## Visão geral

O objetivo é demonstrar o comportamento de uma tabela de hash que cresce linearmente durante a inserção de dados. O algoritmo usa:

- tamanho fixo de página para cada bucket
- páginas de overflow quando um bucket fica cheio
- redistribuição incremental de dados quando o fator de carga máximo é ultrapassado
- crescimento em níveis com ponteiro de divisão (`split pointer`)

O repositório inclui também um gerador de testes e scripts de experimento que coletam métricas como capacidade, páginas de overflow e acessos a páginas para buscas bem-sucedidas e não bem-sucedidas.

## Estrutura do repositório

- `src/main.cpp` - implementação do hashing linear e programa principal de experimento
- `src/gen.cpp` - gerador de arquivos de teste
- `tests/` - arquivos de teste gerados
- `results/` - artefatos de saída, por exemplo `table.csv`
- `bin/` - executáveis e binários compilados
- `compile.sh` / `execute.sh` - scripts de compilação e execução
- `scripts/` - arquivos utilizdos para plotagem de gráficos e escrita de tabelas latex
- `images/`, `latex/`, `used_images/` - documentação e resultados auxiliares

## Compilação

A partir da raiz do repositório:

```bash
mkdir -p bin
g++ src/gen.cpp -o bin/gen -Ofast
g++ src/main.cpp -o bin/hashing -O2
```

Se desejar, também é possível utilizar o script de compilação `compile.sh`. 

## Gerar testes

Execute o gerador para criar arquivos em `tests/`:

```bash
./bin/gen
```

O programa pergunta:

1. Quantos arquivos de teste devem ser gerados?
2. Quantos valores cada teste terá?
3. Quantos valores serão buscados em cada teste?

Cada arquivo de teste contém:

- primeira linha: número de valores `n` a inserir
- segunda linha: `n` valores únicos a inserir
- terceira linha: número de buscas contidas `q1`
- quarta linha: `q1` valores presentes na tabela
- quinta linha: número de buscas não contidas `q2`
- sexta linha: `q2` valores ausentes da tabela

## Executar o algoritmo

Use o programa principal com a sintaxe:

```bash
./bin/hashing <pageSize> <alfaMax> [csvOutputPath] [inputFile]
```

Exemplos:

- usar tamanho de página 10 e `alfaMax` 0.75 com entrada padrão:
  ```bash
  ./bin/hashing 10 0.75 < tests/000.in
  ```

- gravar resultados em CSV:
  ```bash
  ./bin/hashing 10 0.75 results/table.csv tests/000.in < tests/000.in
  ```

Quando `csvOutputPath` é informado, o programa cria o arquivo com cabeçalho se necessário e adiciona uma nova linha de métricas.

## Formato de saída

O programa informa as seguintes métricas:

- `n` - número de valores inseridos
- `searchIn` - número de buscas por valores presentes
- `searchOut` - número de buscas por valores ausentes
- `pageSize` - tamanho fixo da página
- `alfaMax` - carga máxima permitida
- `capacity` - número de buckets/páginas alocados
- `overflow` - número de páginas de overflow criadas
- `alfa` - fator de carga atual após inserções
- `diskAccessIn` - acessos a páginas em buscas bem-sucedidas
- `diskAccessOut` - acessos a páginas em buscas não bem-sucedidas
- `input` - nome opcional do arquivo de entrada

Cabeçalho CSV:

```text
n;searchIn;searchOut;pageSize;alfaMax;capacity;overflow;alfa;diskAccessIn;diskAccessOut;input
```

## Observações

- A função de hash baseia-se em nível: `h(x,l) = x % (m * 2^l)`, onde `l` é o nível e `m` o tamanho inicial da estrutura.
- Overflow ocorre quando um bucket ultrapassa o tamanho de página fixo.
- A estrutura é adequada para experimentos e análise de comportamento de hashing linear, não para produção.
