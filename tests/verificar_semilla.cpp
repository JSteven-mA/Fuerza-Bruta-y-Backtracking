#include <iostream>
#include <string>

long long calcularSemilla(const std::string& cadenaApellidos) {
    long long sumaASCII = 0;

    std::cout << "--- Desglose de codigos ASCII ---\n";
    for (char c : cadenaApellidos) {
        int valorASCII = static_cast<unsigned char>(c);
        sumaASCII += valorASCII;
        std::cout << "Carácter '" << c << "' : " << valorASCII << "\n";
    }

    long long semilla = sumaASCII % 100000;

    std::cout << "---------------------------------\n";
    std::cout << "Suma total ASCII: " << sumaASCII << "\n";
    std::cout << "Semilla calculada (Suma % 100000): " << semilla << "\n";

    return semilla;
}

int main() {
    // Apellidos ordenados alfabéticamente, en minúsculas, sin espacios ni tildes
    std::string cadenaApellidos = "merchanardila";

    std::cout << "========================================================\n";
    std::cout << " UTILIDAD DE VERIFICACION DE SEMILLA\n";
    std::cout << " Cadena ingresada: \"" << cadenaApellidos << "\"\n";
    std::cout << "========================================================\n\n";

    calcularSemilla(cadenaApellidos);

    return 0;
}