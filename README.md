# 📌 Visão Geral
O Motor Balístico Multi-Física é uma biblioteca/módulo em C++ focado em alto desempenho e baixo consumo de memória, projetado para calcular a perda de velocidade aerodinâmica e a capacidade de penetração de projéteis contra blindagens homogêneas de aço (RHA).
O sistema suporta dois modelos físicos distintos dependendo do tipo de projétil:

* Fórmula de De Marre: Projetada para projéteis de calibre cheio e convencionais (AP, APCBC, APHE).

* Fórmula de Lanz-Odermatt: Projetada para projéteis subcalibrados de alta velocidade e penetradores de energia cinética (APDS, APFSDS).

# ⚡ Arquitetura e Otimizações de Memória

* Zero Allocation Loop: Durante a execução do loop principal de cálculo e renderização de tabelas, o consumo de memória alocada no Heap é de 0 bytes.

* Tipo enumerado otimizado (ShellType : uint8_t): O tipo da munição consome apenas 1 byte de memória.

* Eliminação de Cálculos Redundantes: A função printDistanceTable reutiliza o cálculo da velocidade remanescente, evitando chamadas repetidas à função exponencial ($\exp$).

* Sem Cópias Temporárias: Passagem de parâmetros por referência constante (const Shell&) e uso de Move Semantics (std::move / emplace_back) na inserção de novos tanques.

# 📐 Fórmulas Físicas e Algoritmos
## 1. Desaceleração Aerodinâmica (Perda de Velocidade)
A velocidade em função da distância é modelada pela decadência exponencial de arrasto:
$$v(d) = v_0 \cdot e^{-k \cdot d}$$

* $v_0$: Velocidade inicial na boca do canhão ($\text{m/s}$).

* $k$: Coeficiente de arrasto aerodinâmico (adimensional, tipicamente entre $0.00010$ e $0.00035$).

* $d$: Distância até o alvo ($\text{m}$).

## 2. Fórmula de De Marre (Projéteis Convencionais)
Aplicada para munições de calibre total (AP, APCBC, APHE). Este modelo relaciona a perda de velocidade de impacto diretamente com a perda de penetração nominal base.
Equação:
$$P(d, \theta) = P_0 \cdot \left( \frac{v(d)}{v_0} \right)^{1.43} \cdot \cos(\theta)$$

* $P(d, \theta)$: Penetração efetiva na distância $d$ e ângulo $\theta$ ($\text{mm}$).

* $P_0$: Penetração nominal a $0\text{ m}$ e $0^\circ$ ($\text{mm}$).

* $v(d)$: Velocidade no ponto de impacto ($\text{m/s}$).

* $v_0$: Velocidade inicial ($\text{m/s}$).

* $\theta$: Ângulo de impacto em relação à normal ($0^\circ =$ impacto perpendicular).

### 🧮 Exemplo Prático (De Marre)

* Munição: 85mm BR-365A (T-34-85)

* Velocidade Inicial ($v_0$): $792\text{ m/s}$

* Penetração Base ($P_0$): $135\text{ mm}$

* Arrasto ($k$): $0.000148$

* Alvo: a $500\text{ m}$ com ângulo de $30^\circ$

1. Velocidade a $500\text{ m}$:
   $$v(500) = 792 \cdot e^{-0.000148 \cdot 500} = 792 \cdot e^{-0.074} \approx 735.62\text{ m/s}$$

2. Razão de Velocidade com expoente De Marre ($1.43$):
   $$\left( \frac{735.62}{792} \right)^{1.43} = (0.9288)^{1.43} \approx 0.900$$

3. Penetração Angular a $30^\circ$ ($\cos(30^\circ) \approx 0.866$):
   $$P = 135 \cdot 0.900 \cdot 0.866 \approx 105.22\text{ mm}$$

## 3. Fórmula de Lanz-Odermatt (Projéteis Subcalibrados)
Aplicada para munições descarte de sabot e penetradores longos (APDS, APFSDS). O modelo é baseado na mecânica de penetração hidrodinâmica modificada pela resistência dos materiais.
Equação:
$$P(d, \theta) = L \cdot \sqrt{\frac{\rho_p}{\rho_t}} \cdot \exp\left( -\frac{Y_t}{\frac{1}{2} \rho_p \cdot v(d)^2} \right) \cdot \cos^{1.15}(\theta)$$

* $L$: Comprimento do núcleo penetrador ($\text{mm}$).

* $\rho_p$: Densidade do material do penetrador ($\text{kg/m}^3$, ex: $17500\text{ kg/m}^3$ para ligas de Tungstênio).

* $\rho_t$: Densidade da blindagem de aço RHA ($7850\text{ kg/m}^3$).

* $Y_t$: Limite de escoamento/resistência da blindagem RHA ($1.8 \cdot 10^9\text{ Pa}$ ou $1.8\text{ GPa}$).

* $v(d)$: Velocidade no ponto de impacto ($\text{m/s}$).

* $\theta$: Ângulo de impacto em radianos ($\cos^{1.15}(\theta)$ modela a perda de eficiência de hastes longas em ângulos elevados).

### 🧮 Exemplo Prático (Lanz-Odermatt)

* Munição: 76.2mm Shot SV/Mk.1 APDS (Sherman Firefly)

* Comprimento do Núcleo ($L$): $145\text{ mm}$

* Densidade do Núcleo ($\rho_p$): $17000\text{ kg/m}^3$

* Velocidade Inicial ($v_0$): $1204\text{ m/s}$

* Arrasto ($k$): $0.000280$

* Alvo: a $500\text{ m}$ com ângulo de $0^\circ$ (perpendicular)

1. Velocidade a $500\text{ m}$:
   $$v(500) = 1204 \cdot e^{-0.000280 \cdot 500} = 1204 \cdot e^{-0.14} \approx 1046.7\text{ m/s}$$

2. Termo Hidrodinâmico:
   $$\sqrt{\frac{\rho_p}{\rho_t}} = \sqrt{\frac{17000}{7850}} = \sqrt{2.1656} \approx 1.4716$$

3. Pressão Dinâmica ($q$):
   $$q = 0.5 \cdot 17000 \cdot (1046.7)^2 \approx 9.312 \cdot 10^9\text{ Pa}$$

4. Correção de Resistência do Material:
   $$\exp\left( -\frac{1.8 \cdot 10^9}{9.312 \cdot 10^9} \right) = e^{-0.1933} \approx 0.8242$$

5. Penetração Total ($\theta = 0^\circ \implies \cos(0) = 1$):
   $$P = 145 \cdot 1.4716 \cdot 0.8242 \approx 175.87\text{ mm}$$

# 💻 Estrutura de Dados em C++
C++

```
#include <cstdint>
#include <string>
#include <vector>

// Enumeração compacta ocupando apenas 1 byte
enum class ShellType : uint8_t {
    CONVENTIONAL_DEMARRE, // AP, APCBC, APHE
    SUB_CALIBER_ODERMATT  // APDS, APFSDS
};

// Estrutura unificada de munição
struct Shell {
    std::string name;
    ShellType type;
    double caliber_mm;       
    double mass_kg;          
    double muzzleVel_ms;     
    double dragCoeff;        

    // De Marre
    double pen0m_mm = 0.0;   

    // Lanz-Odermatt
    double coreLength_mm = 0.0;    
    double coreDiameter_mm = 0.0;  
    double coreDensity_kgm3 = 0.0; 
};

// Estrutura do veículo
struct Tank {
    std::string name;
    std::vector<Shell> shells;
};
```

# 📊 Tabela Comparativa dos Modelos Físicos
| Característica | De Marre | Lanz-Odermatt |
| --- | --- | --- |
| Aplicações |AP, APCBC, APHE, HE | APDS, APFSDS, APCR|
| Mecânica Principal | Transferência de energia cinética global | Deformação e penetração hidrodinâmica |
| Variáveis Chave | Penetração base ($P_0$), velocidade inicial ($v_0$) | Comprimento do penetrador ($L$), densidade ($\rho_p$) |
| Comportamento Angular| Perda linear ($\cos \theta$) | Perda não-linear ($\cos^{1.15} \theta$) |
| Custo Computacional | 1 chamada std::pow, 1 std::cos | 1 std::sqrt, 1 std::exp, 1 std::pow |

---

# 🧠 Custo de Memória (Memory Footprint)

O consumo de memória é dividido entre a **Stack** (pilha de execução), a **Heap** (memória dinâmica) e a área de dados estáticos (`.rodata`).

## A. Análise do Tamanho das Estruturas (`sizeof`)

Em arquiteturas de **64 bits** (utilizando compiladores padrão como GCC ou Clang):

| Estrutura / Tipo | Componentes Internos | Tamanho individual | Total da Estrutura |
| --- | --- | --- | --- |
| `ShellType` | `uint8_t` | 1 byte | **1 byte** |
| `Shell` | `std::string name`<br>

<br>`ShellType type` (+ 7 bytes padding de alinhamento)<br>

<br>8 variáveis do tipo `double` | 32 bytes<br>

<br>8 bytes<br>

<br>64 bytes | **104 bytes** por munição |
| `Tank` | `std::string name`<br>

<br>`std::vector<Shell> shells` | 32 bytes<br>

<br>24 bytes | **56 bytes** por tanque na Stack |

> **Nota de alinhamento:** O atributo `ShellType` foi reduzido para `uint8_t` (1 byte). O compilador insere 7 bytes de *padding* para garantir que os campos `double` subsequentes fiquem alinhados em endereços múltiplos de 8 bytes na memória.

---

## B. Consumo no Heap e Stack em Tempo de Execução

1. **Vetor Principal (`garage`):**
* Ao chamar `garage.reserve(10)`, o vetor reserva espaço no Heap para 10 elementos do tipo `Tank`:
$$10 \times 56\text{ bytes} = 560\text{ bytes}$$




2. **Vetores Internos de Munições (`shells`):**
* Com os 2 tanques iniciais (2 munições cada):
$$2 \text{ tanques} \times 2 \text{ munições} \times 104\text{ bytes} = 416\text{ bytes}$$




3. **Strings (`std::string`):**
* Devido à otimização **SSO** (*Small String Optimization*), strings curtas com até 15 caracteres não alocam no Heap; ficam guardadas na própria estrutura de 32 bytes.


4. **Constantes Estáticas (`.rodata`):**
* O vetor `static constexpr std::array<double, 8> distances` ocupa $8 \times 8 = 64\text{ bytes}$ no segmento de código de leitura.



**Consumo Total estimado da aplicação:** **< 2 KB de RAM**.

---

# ⚡ Custo de Processamento (Complexidade & CPU)

## A. Complexidade Algorítmica (Big-O)

* **Cálculo da Velocidade (`getVelocityAtDistance`):** $\mathcal{O}(1)$ — Executa 1 multiplicação e 1 exponencial.
* **Cálculo de Penetração (`calculatePenetrationFromVelocity`):** $\mathcal{O}(1)$ — Realiza cálculos aritméticos diretos e funções transcendentais.
* **Geração da Tabela (`printDistanceTable`):** $\mathcal{O}(1)$ contínuo (ou $\mathcal{O}(N)$ onde $N = 8$, tamanho fixo do array de distâncias).

---

## B. Otimizações de CPU Implementadas

1. **Reaproveitamento de Cálculo da Exponencial ($e^{-k \cdot d}$):**
* A função `std::exp()` exige entre **30 e 80 ciclos de clock** do processador.
* Ao passar a velocidade já calculada para `calculatePenetrationFromVelocity`, o código reduz pela metade a quantidade de chamadas a `std::exp()`, economizando cerca de **300 a 500 ciclos de CPU** na renderização de cada tabela balística.


2. **Custo Matemático por Algoritmo:**
* **De Marre:** Executa 1 `std::pow` ($v^{1.43}$) e 1 `std::cos`.
* **Lanz-Odermatt:** Executa 1 `std::sqrt`, 1 `std::exp` (correção de resistência do material) e 1 `std::pow` ($\cos^{1.15}$).


3. **Gargalo Real do Programa (I/O):**
* O custo computacional da matemática balística é insignificante (medido em nanosegundos).
* O maior gargalo de tempo durante a execução é a **E/S do terminal** (`std::cout`), que leva milissegundos para formatar e renderizar o texto no console.
