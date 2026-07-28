#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <limits>
#include <array>

// Enum para identificar a física da munição
enum class ShellType {
    CONVENTIONAL_DEMARRE, // AP, APCBC, APHE (Fórmula de De Marre)
    SUB_CALIBER_ODERMATT  // APDS, APFSDS (Fórmula de Lanz-Odermatt)
};

// Estrutura unificada de Munição
struct Shell {
    std::string name;
    ShellType type;
    double caliber_mm;       // Calibre nominal do canhão (mm)
    double mass_kg;          // Massa total do projétil (kg)
    double muzzleVel_ms;     // Velocidade inicial (m/s)
    double dragCoeff;        // Arrasto aerodinâmico (k)

    // Específico para De Marre
    double pen0m_mm = 0.0;   

    // Específico para Lanz-Odermatt (Subcalibrados)
    double coreLength_mm = 0.0;    // Comprimento do núcleo penetrador (L)
    double coreDiameter_mm = 0.0;  // Diâmetro do núcleo penetrador (D)
    double coreDensity_kgm3 = 0.0; // Densidade do penetrador (ex: 17500 kg/m³)
};

struct Tank {
    std::string name;
    std::vector<Shell> shells;
};

class BallisticEngine {
public:
    static inline double getVelocityAtDistance(double muzzleVel, double distance_m, double dragCoeff) {
        return muzzleVel * std::exp(-dragCoeff * distance_m);
    }

    // Calcula penetração selecionando a fórmula física adequada
    static double calculatePenetration(const Shell& shell, double distance_m, double angle_degrees) {
        double currentVel = getVelocityAtDistance(shell.muzzleVel_ms, distance_m, shell.dragCoeff);
        double radians = angle_degrees * (M_PI / 180.0);

        if (shell.type == ShellType::CONVENTIONAL_DEMARRE) {
            // === FÓRMULA DE DE MARRE ===
            double velocityRatio = std::pow(currentVel / shell.muzzleVel_ms, 1.43);
            double flatPenetration = shell.pen0m_mm * velocityRatio;
            
            return flatPenetration * std::cos(radians);
        } 
        else {
            // === FÓRMULA DE LANZ-ODERMATT (APDS / APFSDS) ===
            constexpr double rho_t = 7850.0;   // Densidade do Aço RHA (kg/m³)
            constexpr double Y_t = 1.8e9;      // Resistência do aço RHA (~1.8 GPa)

            // Pressão dinâmica q = 0.5 * rho_p * v^2
            double dynamicPressure = 0.5 * shell.coreDensity_kgm3 * std::pow(currentVel, 2);
            
            // Fator hidrodinâmico e correção de resistência do material
            double hydrodynamicFactor = std::sqrt(shell.coreDensity_kgm3 / rho_t);
            double strengthCorrection = std::exp(-Y_t / dynamicPressure);

            // Penetração plana P = L * sqrt(rho_p / rho_t) * exp(-Y_t / q)
            double flatPenetration = shell.coreLength_mm * hydrodynamicFactor * strengthCorrection;

            // Penetradores longos perdem eficiência em ângulos (exponencial de inclinação Lanz-Odermatt)
            return flatPenetration * std::pow(std::cos(radians), 1.15);
        }
    }

    static void printDistanceTable(const Shell& shell, double angle_degrees) {
        std::cout << "\n--- TABELA BALÍSTICA (" << shell.name 
                  << " | " << (shell.type == ShellType::CONVENTIONAL_DEMARRE ? "De Marre" : "Lanz-Odermatt")
                  << " @ " << angle_degrees << " deg) ---\n";
        std::cout << std::left << std::setw(12) << "Distancia" 
                  << std::setw(15) << "Velocidade" 
                  << std::setw(18) << "Penetracao" << "\n";
        std::cout << "---------------------------------------------\n";

        // Array estático na memória de código
        static constexpr std::array<double, 8> distances = {0, 100, 300, 500, 800, 1000, 1500, 2000};

        for (double d : distances) {
            double vel = getVelocityAtDistance(shell.muzzleVel_ms, d, shell.dragCoeff);
            double pen = calculatePenetration(shell, d, angle_degrees);

             // Formatação direta no stream sem criar std::string intermediárias
            std::cout << std::left << std::setw(4) << static_cast<int>(d) << " m       "
                      << std::setw(4) << static_cast<int>(vel) << " m/s        "
                      << std::fixed << std::setprecision(1) << pen << " mm\n";
        }
        std::cout << "---------------------------------------------\n";
    }
};

// Limpa a tela usando códigos ANSI 
void clearScreen() { 
    std::cout << "\033[2J\033[1;1H"; 
}

// Limpa buffer do std::cin
void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Aguarda o usuário antes de limpar a tela
void pauseAndContinue() {
    std::cout << "\nPressione ENTER para continuar...";
    clearInput();
    std::cin.get();
}

int main() {
    // Garagem cadastrada com munições convencionais e subcalibradas
    std::vector<Tank> garage = {
        {
            "T-34-85 (URSS)", 
            {
                {"85mm BR-365A (APHEBC)", ShellType::CONVENTIONAL_DEMARRE, 85.0, 9.2, 792.0, 0.000148, 135.0},
                {"85mm BR-365P (APCR)",   ShellType::SUB_CALIBER_ODERMATT, 85.0, 5.0, 1050.0, 0.000320, 0.0, 110.0, 28.0, 15000.0}
            }
        },
        {
            "Sherman Firefly (Reino Unido)", 
            {
                {"76.2mm Shot Mk.8 (APCBC)",   ShellType::CONVENTIONAL_DEMARRE, 76.2, 7.7, 884.0, 0.000140, 171.0},
                {"76.2mm Shot SV/Mk.1 (APDS)", ShellType::SUB_CALIBER_ODERMATT, 76.2, 3.3, 1204.0, 0.000280, 0.0, 145.0, 28.0, 17000.0}
            }
        }
    };

    while (true) {
        clearScreen();
        std::cout << "=============================================\n";
        std::cout << "   CALCULADORA BALÍSTICA MULTI-FÍSICA (C++)  \n";
        std::cout << "=============================================\n";
        std::cout << "Escolha um tanque:\n";

        for (size_t i = 0; i < garage.size(); ++i) {
            std::cout << " [" << (i + 1) << "] " << garage[i].name << "\n";
        }
        std::cout << " [" << (garage.size() + 1) << "] + Cadastrar Novo Tanque e Municao\n";
        std::cout << " [0] Sair\n";
        std::cout << "Opcao: ";

        int tankChoice;
        if (!(std::cin >> tankChoice) || tankChoice == 0) break;

        // PONTEIRO: Evita copiar a estrutura 'Tank' inteira
        const Tank* selectedTank = nullptr;

        if (tankChoice == static_cast<int>(garage.size() + 1)) {
            clearInput();
            Tank newTank;
            Shell customShell;

            std::cout << "\n--- CADASTRAR NOVO TANQUE ---\n";
            std::cout << "Nome do Tanque: ";
            std::getline(std::cin, newTank.name);

            std::cout << "Nome da Municao: ";
            std::getline(std::cin, customShell.name);

            std::cout << "Tipo de Municao:\n";
            std::cout << " [1] Convencional (AP, APCBC, APHE) - Formula De Marre\n";
            std::cout << " [2] Subcalibrada (APDS, APFSDS)   - Formula Lanz-Odermatt\n";
            std::cout << "Opcao: ";
            int typeChoice;
            std::cin >> typeChoice;

            std::cout << "Calibre Nominal (mm): ";
            std::cin >> customShell.caliber_mm;

            std::cout << "Massa Total do Projetil (kg): ";
            std::cin >> customShell.mass_kg;

            std::cout << "Velocidade Inicial (m/s): ";
            std::cin >> customShell.muzzleVel_ms;

            std::cout << "Coeficiente de Arrasto (~0.00015): ";
            std::cin >> customShell.dragCoeff;

            if (typeChoice == 2) {
                customShell.type = ShellType::SUB_CALIBER_ODERMATT;
                std::cout << "Comprimento do Núcleo Penetrador L (mm): ";
                std::cin >> customShell.coreLength_mm;
                std::cout << "Diametro do Núcleo D (mm): ";
                std::cin >> customShell.coreDiameter_mm;
                std::cout << "Densidade do Penetrador (ex: 17500 para Tungstênio): ";
                std::cin >> customShell.coreDensity_kgm3;
            } else {
                customShell.type = ShellType::CONVENTIONAL_DEMARRE;
                std::cout << "Penetracao Base a 0m / 0 deg (mm): ";
                std::cin >> customShell.pen0m_mm;
            }

            newTank.shells.push_back(customShell);

            // Movemos a memória diretamente para a garagem (zero cópia de strings)
            garage.push_back(std::move(newTank));
            selectedTank = &garage.back();
        } 
        else if (tankChoice > 0 && tankChoice <= static_cast<int>(garage.size())) {
            // Aponta diretamente para o tanque existente na garagem
            selectedTank = &garage[tankChoice - 1];
        } 
        else {
            continue;
        }

        // Seleção de munição
        clearScreen();
        std::cout << "Tanque: " << selectedTank->name << "\n";
        std::cout << "Escolha a municao:\n";
        for (size_t i = 0; i < selectedTank->shells.size(); ++i) {
            std::cout << " [" << (i + 1) << "] " << selectedTank->shells[i].name 
                      << (selectedTank->shells[i].type == ShellType::SUB_CALIBER_ODERMATT ? " [Subcalibrada]" : " [Convencional]") << "\n";
        }
        std::cout << "Opcao: ";

        int shellChoice;
        std::cin >> shellChoice;
        if (shellChoice < 1 || shellChoice > static_cast<int>(selectedTank->shells.size())) {
            continue;
        }

        // Aponta direto para a munição escolhida
        const Shell& selectedShell = selectedTank->shells[shellChoice - 1];

        // Parâmetros de disparo
        double distance, angle;
        std::cout << "\nDistancia do alvo (m): ";
        std::cin >> distance;

        std::cout << "Angulo de impacto (graus, 0 = perpendicular): ";
        std::cin >> angle;

        // Processamento
        double currentVel = BallisticEngine::getVelocityAtDistance(selectedShell.muzzleVel_ms, distance, selectedShell.dragCoeff);
        double penResult = BallisticEngine::calculatePenetration(selectedShell, distance, angle);

        // Exibição
        clearScreen();
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "=============================================\n";
        std::cout << "            RESULTADO DO CÁLCULO             \n";
        std::cout << "=============================================\n";
        std::cout << "Tanque:               " << selectedTank->name << "\n";
        std::cout << "Municao:              " << selectedShell.name << "\n";
        std::cout << "Algoritmo Aplicado:   " << (selectedShell.type == ShellType::CONVENTIONAL_DEMARRE ? "De Marre (Full-Caliber)" : "Lanz-Odermatt (Sub-Caliber)") << "\n";
        std::cout << "Distancia do Alvo:    " << distance << " m\n";
        std::cout << "Angulo de Impacto:    " << angle << " deg\n";
        std::cout << "Velocidade no Alvo:   " << currentVel << " m/s\n";
        std::cout << "PENETRACAO ESTIMADA:  " << penResult << " mm\n";
        std::cout << "=============================================\n";

        BallisticEngine::printDistanceTable(selectedShell, angle);
        
        // Pausa para leitura do jogador antes de limpar o terminal
        pauseAndContinue();
    }

    clearScreen();
    std::cout << "Calculadora encerrada.\n";
    return 0;
}