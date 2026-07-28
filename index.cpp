#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <limits>
#include <array>

// Estrutura leve para munições
struct Shell {
    std::string name;
    double caliber_mm;       // Calibre em mm
    double mass_kg;          // Massa em kg
    double muzzleVel_ms;     // Velocidade inicial (m/s)
    double pen0m_mm;         // Penetração a 0m e 0 graus (mm)
    double dragCoeff;        // Coeficiente de arrasto aerodinâmico (k)
};

// Estrutura para os tanques
struct Tank {
    std::string name;
    std::vector<Shell> shells;
};

class BallisticEngine {
public:
    // Retorna velocidade estimada sem alocar variáveis adicionais
    static inline double getVelocityAtDistance(double muzzleVel, double distance_m, double dragCoeff) {
        return muzzleVel * std::exp(-dragCoeff * distance_m);
    }

    // Aplica a Fórmula de De Marre com variação de velocidade e ângulo
    static double calculatePenetration(const Shell& shell, double distance_m, double angle_degrees) {
        double currentVel = getVelocityAtDistance(shell.muzzleVel_ms, distance_m, shell.dragCoeff);
        double velocityRatio = std::pow(currentVel / shell.muzzleVel_ms, 1.43);
        double flatPenetration = shell.pen0m_mm * velocityRatio;

        // Converte ângulo para radianos (0° = impacto direto/perpendicular)
        double radians = angle_degrees * (M_PI / 180.0);
        return flatPenetration * std::cos(radians);
    }

    // Exibe tabela comparativa de distâncias
    static void printDistanceTable(const Shell& shell, double angle_degrees) {
        std::cout << "\n--- TABELA BALÍSTICA (" << shell.name << " @ " << angle_degrees << " deg) ---\n";
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
    // Garagem de tanques armazenada em memória contígua
    std::vector<Tank> garage = {
        {
            "T-34-85 (URSS)", 
            {
                {"85mm BR-365A (APHEBC)", 85.0, 9.2, 792.0, 135.0, 0.000148},
                {"85mm BR-365K (APHE)",   85.0, 9.2, 792.0, 142.0, 0.000160},
                {"85mm BR-365P (APCR)",   85.0, 5.0, 1050.0, 180.0, 0.000320}
            }
        },
        {
            "Tiger I (Alemanha)", 
            {
                {"88mm PzGr 39 (APCBC)", 88.0, 10.2, 773.0, 165.0, 0.000135},
                {"88mm PzGr 40 (APCR)",  88.0, 7.3,  930.0, 226.0, 0.000300}
            }
        },
        {
            "Sherman Firefly (Reino Unido)", 
            {
                {"76.2mm Shot Mk.8 (APCBC)", 76.2, 7.7, 884.0, 171.0, 0.000140},
                {"76.2mm Shot SV/Mk.1 (APDS)", 76.2, 3.3, 1204.0, 228.0, 0.000280}
            }
        }
    };

    while (true) {
        clearScreen();
        std::cout << "=============================================\n";
        std::cout << "   CALCULADORA BALÍSTICA DE BLINDAGEM (C++)  \n";
        std::cout << "=============================================\n";
        std::cout << "Escolha um tanque:\n";

        for (size_t i = 0; i < garage.size(); ++i) {
            std::cout << " [" << (i + 1) << "] " << garage[i].name << "\n";
        }
        std::cout << " [" << (garage.size() + 1) << "] + Cadastrar Novo Tanque\n";
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

            std::cout << "Calibre (mm): ";
            std::cin >> customShell.caliber_mm;

            std::cout << "Massa do Projetil (kg): ";
            std::cin >> customShell.mass_kg;

            std::cout << "Velocidade Inicial (m/s): ";
            std::cin >> customShell.muzzleVel_ms;

            std::cout << "Penetracao a 0m / 0 deg (mm): ";
            std::cin >> customShell.pen0m_mm;

            std::cout << "Coeficiente de Arrasto (~0.00015): ";
            std::cin >> customShell.dragCoeff;

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

        // Seleção de munição com ponteiro/referência
        clearScreen();
        std::cout << "Tanque: " << selectedTank->name << "\n";
        std::cout << "Escolha a municao:\n";
        for (size_t i = 0; i < selectedTank->shells.size(); ++i) {
            std::cout << " [" << (i + 1) << "] " << selectedTank->shells[i].name << "\n";
        }
        std::cout << "Opcao: ";

        int shellChoice;
        std::cin >> shellChoice;
        if (shellChoice < 1 || shellChoice > static_cast<int>(selectedTank->shells.size())) {
            continue;
        }

        // Aponta direto para a munição escolhida
        const Shell& selectedShell = selectedTank->shells[shellChoice - 1];

        // Leitura dos parâmetros
        double distance, angle;
        std::cout << "\nDistancia do alvo (m): ";
        std::cin >> distance;

        std::cout << "Angulo de impacto (graus, 0 = perpendicular): ";
        std::cin >> angle;

        // Processamento
        double currentVel = BallisticEngine::getVelocityAtDistance(selectedShell.muzzleVel_ms, distance, selectedShell.dragCoeff);
        double penResult = BallisticEngine::calculatePenetration(selectedShell, distance, angle);

        // Apresentação dos resultados
        clearScreen();
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "=============================================\n";
        std::cout << "            RESULTADO DO CÁLCULO             \n";
        std::cout << "=============================================\n";
        std::cout << "Tanque:               " << selectedTank->name << "\n";
        std::cout << "Municao:              " << selectedShell.name << "\n";
        std::cout << "Distancia do Alvo:    " << distance << " m\n";
        std::cout << "Angulo de Impacto:    " << angle << " deg\n";
        std::cout << "Velocidade no Alvo:   " << currentVel << " m/s (Perda: " << (selectedShell.muzzleVel_ms - currentVel) << " m/s)\n";
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