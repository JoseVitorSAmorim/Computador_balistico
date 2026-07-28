#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <limits>

// Estrutura para representar a munição
struct Shell {
    std::string name;
    double caliber_mm;       // Calibre em mm
    double mass_kg;          // Massa em kg
    double muzzleVel_ms;     // Velocidade inicial (m/s)
    double pen0m_mm;         // Penetração a 0m e 0 graus (mm)
    double dragCoeff;        // Coeficiente de arrasto aerodinâmico (k)
};

// Estrutura para representar o tanque e seu arsenal
struct Tank {
    std::string name;
    std::vector<Shell> shells;
};

class BallisticEngine {
public:
    // Calcula velocidade remanescente a uma certa distância
    static double getVelocityAtDistance(double muzzleVel, double distance_m, double dragCoeff) {
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
        std::cout << "\n--- TABELA BALÍSTICA DE PENETRAÇÃO (" << shell.name << " @ " << angle_degrees << " deg) ---\n";
        std::cout << std::left << std::setw(12) << "Distância" 
                  << std::setw(15) << "Velocidade" 
                  << std::setw(18) << "Penetração" << "\n";
        std::cout << "---------------------------------------------\n";

        std::vector<double> distances = {0, 100, 300, 500, 800, 1000, 1500, 2000};
        for (double d : distances) {
            double vel = getVelocityAtDistance(shell.muzzleVel_ms, d, shell.dragCoeff);
            double pen = calculatePenetration(shell, d, angle_degrees);

            std::cout << std::left << std::setw(12) << (std::to_string((int)d) + " m")
                      << std::setw(15) << (std::to_string((int)vel) + " m/s")
                      << std::setw(18) << (std::to_string(pen).substr(0, 5) + " mm") << "\n";
        }
        std::cout << "---------------------------------------------\n";
    }
};

// Limpa buffer de entrada
void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    // Banco de dados inicial de tanques e munições
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
        std::cout << "\n=============================================\n";
        std::cout << "   CALCULADORA BALÍSTICA DE BLINDAGEM (C++)  \n";
        std::cout << "=============================================\n";
        std::cout << "Escolha um tanque:\n";

        for (size_t i = 0; i < garage.size(); ++i) {
            std::cout << " [" << (i + 1) << "] " << garage[i].name << "\n";
        }
        std::cout << " [" << (garage.size() + 1) << "] + Cadastrar Novo Tanque / Munição Customizada\n";
        std::cout << " [0] Sair\n";
        std::cout << "Opção: ";

        int tankChoice;
        if (!(std::cin >> tankChoice) || tankChoice == 0) break;

        Tank selectedTank;

        // Opção de criar novo tanque/munição
        if (tankChoice == static_cast<int>(garage.size() + 1)) {
            clearInput();
            std::cout << "\n--- CADASTRAR NOVO TANQUE ---\n";
            std::cout << "Nome do Tanque: ";
            std::getline(std::cin, selectedTank.name);

            Shell customShell;
            std::cout << "Nome da Munição: ";
            std::getline(std::cin, customShell.name);

            std::cout << "Calibre (mm): ";
            std::cin >> customShell.caliber_mm;

            std::cout << "Massa do Projétil (kg): ";
            std::cin >> customShell.mass_kg;

            std::cout << "Velocidade Inicial na Boca (m/s): ";
            std::cin >> customShell.muzzleVel_ms;

            std::cout << "Penetração a 0m / 0 deg (mm): ";
            std::cin >> customShell.pen0m_mm;

            std::cout << "Coeficiente de Arrasto (padrão ~0.00015): ";
            std::cin >> customShell.dragCoeff;

            selectedTank.shells.push_back(customShell);
            garage.push_back(selectedTank); // Salva na garagem durante a execução
        } 
        else if (tankChoice > 0 && tankChoice <= static_cast<int>(garage.size())) {
            selectedTank = garage[tankChoice - 1];
        } 
        else {
            std::cout << "\nOpção inválida!\n";
            continue;
        }

        // Seleção de munição
        std::cout << "\nTanque selecionado: " << selectedTank.name << "\n";
        std::cout << "Escolha a munição:\n";
        for (size_t i = 0; i < selectedTank.shells.size(); ++i) {
            std::cout << " [" << (i + 1) << "] " << selectedTank.shells[i].name << "\n";
        }
        std::cout << "Opção: ";

        int shellChoice;
        std::cin >> shellChoice;
        if (shellChoice < 1 || shellChoice > static_cast<int>(selectedTank.shells.size())) {
            std::cout << "Munição inválida!\n";
            continue;
        }

        Shell selectedShell = selectedTank.shells[shellChoice - 1];

        // Leitura de parâmetros de combate
        double distance, angle;
        std::cout << "\nDigite a distância do alvo em metros (ex: 500): ";
        std::cin >> distance;

        std::cout << "Digite o ângulo de impacto da blindagem em graus (0 = reto/perpendicular): ";
        std::cin >> angle;

        // Cálculos
        double currentVel = BallisticEngine::getVelocityAtDistance(selectedShell.muzzleVel_ms, distance, selectedShell.dragCoeff);
        double penResult = BallisticEngine::calculatePenetration(selectedShell, distance, angle);

        // Exibição dos resultados
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n=============================================\n";
        std::cout << "            RESULTADO DO CÁLCULO             \n";
        std::cout << "=============================================\n";
        std::cout << "Tanque:               " << selectedTank.name << "\n";
        std::cout << "Munição:              " << selectedShell.name << "\n";
        std::cout << "Distância do Alvo:    " << distance << " m\n";
        std::cout << "Ângulo de Impacto:    " << angle << "°\n";
        std::cout << "Velocidade no Alvo:   " << currentVel << " m/s (Perda: " << (selectedShell.muzzleVel_ms - currentVel) << " m/s)\n";
        std::cout << "PENETRAÇÃO ESTIMADA:  " << penResult << " mm\n";
        std::cout << "=============================================\n";

        // Tabela de apoio
        BallisticEngine::printDistanceTable(selectedShell, angle);
    }

    std::cout << "\nCalculadora encerrada.\n";
    return 0;
}