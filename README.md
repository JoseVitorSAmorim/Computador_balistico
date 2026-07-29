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
| Custo Computacional | 1 chamada std::pow, 1 std::cos | 1 std::sqrt, 1 std::exp, 1 std::pow |

