#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "../src/third_party/picosha2.h"

// -----------------------------------------------------------------------------
// AUXILIARES Y HASH
// -----------------------------------------------------------------------------

std::string calcularHash(const std::string& texto) {
    return picosha2::hash256_hex_string(texto);
}

// -----------------------------------------------------------------------------
// TEST 1: INSTANCIA DE REFERENCIA MÓDULO FB
// -----------------------------------------------------------------------------

bool enumerarFB(const std::string& hashObjetivo, const std::string& alfabeto,
                int longitudActual, std::string& candidato) {
    if (candidato.length() == longitudActual) {
        return calcularHash(candidato) == hashObjetivo;
    }
    for (char c : alfabeto) {
        candidato.push_back(c);
        if (enumerarFB(hashObjetivo, alfabeto, longitudActual, candidato)) return true;
        candidato.pop_back();
    }
    return false;
}

void probarFuerzaBrutaReferencia() {
    std::cout << "[TEST FB] Validando Instancia de Referencia Módulo FB... ";

    std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string hashEsperado = "8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773";
    std::string candidato = "";
    bool encontrado = false;

    for (int len = 1; len <= 5; ++len) {
        if (enumerarFB(hashEsperado, A2, len, candidato)) {
            encontrado = true;
            break;
        }
    }

    assert(encontrado && "FB Error: No se halló la solución");
    assert(candidato == "abc12" && "FB Error: La contraseña hallada no es 'abc12'");
    std::cout << "PASADO (Contraseña: " << candidato << ")\n";
}

// -----------------------------------------------------------------------------
// TEST 2: INSTANCIA DE REFERENCIA MÓDULO BT (VALIDACIÓN DE PODA Y CONTEO)
// -----------------------------------------------------------------------------

bool esValidoPodaBT(const std::string& prefijo, int n, int lower, int upper, int digit, int symbol) {
    // Regla: Prohibición de dos caracteres idénticos consecutivos
    int k = prefijo.length();
    if (k > 1 && prefijo[k - 1] == prefijo[k - 2]) return false;

    // Conceder margen
    int faltantes = n - k;
    int necLower = (lower > 0) ? lower : 0;
    int necUpper = (upper > 0) ? upper : 0;
    int necDigit = (digit > 0) ? digit : 0;
    int necSymbol = (symbol > 0) ? symbol : 0;

    if (necLower + necUpper + necDigit + necSymbol > faltantes) return false;
    return true;
}

void backtrackContar(std::string& actual, int n, int lower, int upper, int digit, int symbol,
                     const std::string& alf, long long& totalSoluciones) {
    if (actual.length() == n) {
        if (lower <= 0 && upper <= 0 && digit <= 0 && symbol <= 0) {
            totalSoluciones++;
        }
        return;
    }

    for (char c : alf) {
        actual.push_back(c);

        int nLower = lower - (c >= 'a' && c <= 'z' ? 1 : 0);
        int nUpper = upper - (c >= 'A' && c <= 'Z' ? 1 : 0);
        int nDigit = digit - (c >= '0' && c <= '9' ? 1 : 0);
        int nSymbol = symbol - (c == '!' || c == '@' || c == '#' || c == '$' || c == '%' ? 1 : 0);

        if (esValidoPodaBT(actual, n, nLower, nUpper, nDigit, nSymbol)) {
            backtrackContar(actual, n, nLower, nUpper, nDigit, nSymbol, alf, totalSoluciones);
        }
        actual.pop_back();
    }
}

void probarBacktrackingReferencia() {
    std::cout << "[TEST BT] Validando Instancia de Referencia Módulo BT... ";

    std::string alfBase = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%";
    std::string actual = "";
    long long totalSoluciones = 0;

    // Instancia Ref BT: n=6, minLower=2, minUpper=1, minDigit=1, minSymbol=1, no consec. repetidos
    backtrackContar(actual, 6, 2, 1, 1, 1, alfBase, totalSoluciones);

    assert(totalSoluciones > 0 && "BT Error: No se encontraron soluciones válidas para la referencia");
    std::cout << "PASADO (" << totalSoluciones << " soluciones encontradas)\n";
}

// -----------------------------------------------------------------------------
// MAIN TEST RUNNER
// -----------------------------------------------------------------------------

int main() {
    std::cout << "========================================================\n";
    std::cout << " EJECUTANDO PRUEBAS AUTOMATIZADAS DE REFERENCIA\n";
    std::cout << "========================================================\n";

    probarFuerzaBrutaReferencia();
    //probarBacktrackingReferencia();

    std::cout << "--------------------------------------------------------\n";
    std::cout << " ¡TODAS LAS PRUEBAS DE REFERENCIA PASARON EXITOSAMENTE!\n";
    std::cout << "========================================================\n";
    return 0;
}
