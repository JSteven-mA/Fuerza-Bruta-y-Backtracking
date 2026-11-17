#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstdint>

// Librería criptográfica local en src/third_party/
#include "third_party/picosha2.h"

// =============================================================================
// ESTRUCTURAS DE DATOS
// =============================================================================

// --- Módulo Fuerza Bruta ---
struct InstanciaFBConfig {
    int id;
    int longitud;
    std::string nombreAlfabeto;
    std::string alfabeto;
};

struct ResultadoFB {
    int id;
    int longitud;
    std::string nombreAlfabeto;
    std::string passGenerada;
    std::string hashSHA256;

    // Métricas Fuerza Bruta
    double tiempoMsFB;
    long long candidatosFB;
    bool encontradoFB;
    std::string passRecuperadaFB;

    // Métricas Diccionario
    double tiempoMsDic;
    long long candidatosDic;
    bool encontradoDic;
};

// --- Módulo Backtracking ---
struct PoliticaBT {
    int minLower;
    int minUpper;
    int minDigit;
    int minSymbol;
};

struct InstanciaBTConfig {
    int id;
    int n;
    PoliticaBT politica;
};

struct ResultadoBT {
    int id;
    int n;
    long long nodosSinPoda;
    long long nodosConPoda;
    double porcentajeReduccion;
    long long solucionesSinPoda;
    long long solucionesConPoda;
    double tiempoMsSinPoda;
    double tiempoMsConPoda;
    bool correctitudCoincide;
};

// =============================================================================
// 1. UTILIDADES Y GENERADOR LCG
// =============================================================================

std::string calcularHash(const std::string& texto) {
    return picosha2::hash256_hex_string(texto);
}

long long calcularSemillaEquipo(const std::string& apellidos) {
    long long sumaASCII = 0;
    for (char c : apellidos) {
        sumaASCII += static_cast<unsigned char>(c);
    }
    return sumaASCII % 100000;
}

std::string generarContrasenaLCG(long long& x, const std::string& alfabeto, int longitud) {
    std::string pass = "";
    for (int i = 0; i < longitud; ++i) {
        x = (1103515245LL * x + 12345LL) % 2147483648LL;
        pass += alfabeto[x % alfabeto.length()];
    }
    return pass;
}

// =============================================================================
// 2. MÓDULO FUERZA BRUTA Y DICCIONARIO
// =============================================================================

bool enumerarFuerzaBruta(const std::string& hashObjetivo, const std::string& alfabeto,
                        int longitudActual, std::string& candidato, long long& candidatosEvaluados) {
    if (candidato.length() == static_cast<size_t>(longitudActual)) {
        candidatosEvaluados++;
        return calcularHash(candidato) == hashObjetivo;
    }

    for (char c : alfabeto) {
        candidato.push_back(c);
        if (enumerarFuerzaBruta(hashObjetivo, alfabeto, longitudActual, candidato, candidatosEvaluados)) {
            return true;
        }
        candidato.pop_back();
    }
    return false;
}

void ejecutarFuerzaBruta(const std::string& hashObjetivo, const std::string& alfabeto, int longitudMaxima,
                          long long& candidatosEvaluados, double& tiempoMs, bool& encontrado, std::string& passEncontrada) {
    candidatosEvaluados = 0;
    std::string candidato = "";
    encontrado = false;

    auto inicio = std::chrono::high_resolution_clock::now();

    for (int longitud = 1; longitud <= longitudMaxima; ++longitud) {
        if (enumerarFuerzaBruta(hashObjetivo, alfabeto, longitud, candidato, candidatosEvaluados)) {
            encontrado = true;
            passEncontrada = candidato;
            break;
        }
    }

    auto fin = std::chrono::high_resolution_clock::now();
    tiempoMs = std::chrono::duration<double, std::milli>(fin - inicio).count();
}

void ejecutarDiccionario(const std::string& hashObjetivo, const std::string& rutaDiccionario,
                         long long& candidatosEvaluados, double& tiempoMs, bool& encontrado) {
    std::ifstream archivo(rutaDiccionario);
    candidatosEvaluados = 0;
    encontrado = false;
    std::string palabra;

    auto inicio = std::chrono::high_resolution_clock::now();

    if (archivo.is_open()) {
        while (archivo >> palabra) {
            candidatosEvaluados++;
            if (calcularHash(palabra) == hashObjetivo) {
                encontrado = true;
                break;
            }
        }
        archivo.close();
    }

    auto fin = std::chrono::high_resolution_clock::now();
    tiempoMs = std::chrono::duration<double, std::milli>(fin - inicio).count();
}

// =============================================================================
// 3. MÓDULO BACKTRACKING (CON PODA VS SIN PODA)
// =============================================================================

inline bool esLower(char c)  { return (c >= 'a' && c <= 'z'); }
inline bool esUpper(char c)  { return (c >= 'A' && c <= 'Z'); }
inline bool esDigit(char c)  { return (c >= '0' && c <= '9'); }
inline bool esSymbol(char c) { return (c == '!' || c == '@' || c == '#' || c == '$' || c == '%'); }

// Validador de hojas para exploración Sin Poda
bool esValidoHojaSinPoda(const std::string& pass, const PoliticaBT& pol) {
    for (size_t i = 1; i < pass.length(); ++i) {
        if (pass[i] == pass[i - 1]) return false;
    }
    int cLower = 0, cUpper = 0, cDigit = 0, cSymbol = 0;
    for (char c : pass) {
        if (esLower(c))       cLower++;
        else if (esUpper(c))  cUpper++;
        else if (esDigit(c))  cDigit++;
        else if (esSymbol(c)) cSymbol++;
    }
    return (cLower >= pol.minLower && cUpper >= pol.minUpper &&
            cDigit >= pol.minDigit && cSymbol >= pol.minSymbol);
}

// BT Sin Poda (Exploración Exhaustiva Completa)
void btSinPoda(std::string& actual, int n, const PoliticaBT& pol, const std::string& alf,
               long long& nodosGenerados, long long& soluciones) {
    nodosGenerados++;
    if (actual.length() == static_cast<size_t>(n)) {
        if (esValidoHojaSinPoda(actual, pol)) {
            soluciones++;
        }
        return;
    }
    for (char c : alf) {
        actual.push_back(c);
        btSinPoda(actual, n, pol, alf, nodosGenerados, soluciones);
        actual.pop_back();
    }
}

// Función de Bounding / Poda para BT Con Poda
bool esPrometedorBT(const std::string& prefijo, int n, int lowerReq, int upperReq, int digitReq, int symbolReq) {
    int k = prefijo.length();

    // Regla 1: Caracteres consecutivos no pueden ser idénticos
    if (k > 1 && prefijo[k - 1] == prefijo[k - 2]) return false;

    // Regla 2: Posiciones faltantes suficientes para cumplir la política
    int faltantes = n - k;
    int necLower  = (lowerReq > 0)  ? lowerReq  : 0;
    int necUpper  = (upperReq > 0)  ? upperReq  : 0;
    int necDigit  = (digitReq > 0)  ? digitReq  : 0;
    int necSymbol = (symbolReq > 0) ? symbolReq : 0;

    if (necLower + necUpper + necDigit + necSymbol > faltantes) return false;

    return true;
}

// BT Con Poda
void btConPoda(std::string& actual, int n, int lowerReq, int upperReq, int digitReq, int symbolReq,
              const std::string& alf, long long& nodosVisitados, long long& soluciones) {
    nodosVisitados++;

    if (actual.length() == static_cast<size_t>(n)) {
        if (lowerReq <= 0 && upperReq <= 0 && digitReq <= 0 && symbolReq <= 0) {
            soluciones++;
        }
        return;
    }

    for (char c : alf) {
        actual.push_back(c);

        int nLower  = lowerReq  - (esLower(c)  ? 1 : 0);
        int nUpper  = upperReq  - (esUpper(c)  ? 1 : 0);
        int nDigit  = digitReq  - (esDigit(c)  ? 1 : 0);
        int nSymbol = symbolReq - (esSymbol(c) ? 1 : 0);

        if (esPrometedorBT(actual, n, nLower, nUpper, nDigit, nSymbol)) {
            btConPoda(actual, n, nLower, nUpper, nDigit, nSymbol, alf, nodosVisitados, soluciones);
        }

        actual.pop_back();
    }
}

// =============================================================================
// 4. MAIN PRINCIPAL (EJECUCIÓN INTEGRADA Y EXPORTACIÓN)
// =============================================================================

int main() {
    std::filesystem::create_directories("results");

    std::string A1 = "abcdefghijklmnopqrstuvwxyz";
    std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string alfBT = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%";

    std::cout << " MODULO 1: FUERZA BRUTA Y DICCIONARIO\n";


    // 1.1 Prueba de Referencia
    std::string hashRef = "8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773";
    long long candFBRef = 0, candDicRef = 0;
    double tFBRef = 0.0, tDicRef = 0.0;
    bool encFBRef = false, encDicRef = false;
    std::string passFBRef = "";

    ejecutarFuerzaBruta(hashRef, A2, 5, candFBRef, tFBRef, encFBRef, passFBRef);
    ejecutarDiccionario(hashRef, "resources/diccionario.txt", candDicRef, tDicRef, encDicRef);

    std::cout << "[Referencia] FB   : " << (encFBRef ? passFBRef : "No hallada") << " | " << tFBRef << " ms | Eval: " << candFBRef << "\n";
    std::cout << "[Referencia] Dicc : " << (encDicRef ? "SI" : "NO") << " | " << tDicRef << " ms | Eval: " << candDicRef << "\n\n";

    // 1.2 Instancias del Equipo con LCG
    std::string apellidosEquipo = "ardilamerchan";
    long long semilla = calcularSemillaEquipo(apellidosEquipo);
    std::cout << "Equipo: \"" << apellidosEquipo << "\" | Semilla LCG: " << semilla << "\n\n";

    std::vector<InstanciaFBConfig> configsFB = {
        {1, 4, "A1", A1}, {2, 4, "A2", A2}, {3, 5, "A1", A1}, {4, 5, "A2", A2}, {5, 6, "A1", A1}
    };

    std::vector<ResultadoFB> resultadosFB;
    long long x_lcg = semilla;

    for (const auto& inst : configsFB) {
        ResultadoFB res;
        res.id = inst.id;
        res.longitud = inst.longitud;
        res.nombreAlfabeto = inst.nombreAlfabeto;

        res.passGenerada = generarContrasenaLCG(x_lcg, inst.alfabeto, inst.longitud);
        res.hashSHA256 = calcularHash(res.passGenerada);

        ejecutarFuerzaBruta(res.hashSHA256, inst.alfabeto, inst.longitud,
                            res.candidatosFB, res.tiempoMsFB, res.encontradoFB, res.passRecuperadaFB);

        ejecutarDiccionario(res.hashSHA256, "resources/diccionario.txt",
                           res.candidatosDic, res.tiempoMsDic, res.encontradoDic);

        resultadosFB.push_back(res);

        std::cout << "FB Instancia " << res.id << " [" << res.nombreAlfabeto << ", n=" << res.longitud << "] Pass: " << res.passGenerada << "\n"
                  << "  - FB : " << res.tiempoMsFB << " ms | Candidatos: " << res.candidatosFB << "\n"
                  << "  - Dic: Encontrada: " << (res.encontradoDic ? "SI" : "NO") << " | " << res.tiempoMsDic << " ms\n";
    }

    // -------------------------------------------------------------------------
    // MÓDULO 2: BACKTRACKING
    // -------------------------------------------------------------------------

    std::cout << " MODULO 2: BACKTRACKING (CON PODA VS SIN PODA)\n";


    std::vector<InstanciaBTConfig> configsBT = {
        {1, 3, {1, 1, 1, 0}},
        {2, 4, {1, 1, 1, 1}},
        {3, 4, {2, 1, 1, 0}}
    };

    std::vector<ResultadoBT> resultadosBT;

    for (const auto& inst : configsBT) {
        ResultadoBT res;
        res.id = inst.id;
        res.n = inst.n;

        std::string actSin = "", actCon = "";

        // Sin Poda
        res.nodosSinPoda = 0; res.solucionesSinPoda = 0;
        auto iniSin = std::chrono::high_resolution_clock::now();
        btSinPoda(actSin, inst.n, inst.politica, alfBT, res.nodosSinPoda, res.solucionesSinPoda);
        auto finSin = std::chrono::high_resolution_clock::now();
        res.tiempoMsSinPoda = std::chrono::duration<double, std::milli>(finSin - iniSin).count();

        // Con Poda
        res.nodosConPoda = 0; res.solucionesConPoda = 0;
        auto iniCon = std::chrono::high_resolution_clock::now();
        btConPoda(actCon, inst.n, inst.politica.minLower, inst.politica.minUpper,
                  inst.politica.minDigit, inst.politica.minSymbol, alfBT, res.nodosConPoda, res.solucionesConPoda);
        auto finCon = std::chrono::high_resolution_clock::now();
        res.tiempoMsConPoda = std::chrono::duration<double, std::milli>(finCon - iniCon).count();

        // Métricas
        res.porcentajeReduccion = (1.0 - (static_cast<double>(res.nodosConPoda) / static_cast<double>(res.nodosSinPoda))) * 100.0;
        res.correctitudCoincide = (res.solucionesSinPoda == res.solucionesConPoda);

        resultadosBT.push_back(res);

        std::cout << "BT Instancia " << res.id << " (n=" << res.n << ")\n"
                  << "  - Nodos Sin Poda : " << res.nodosSinPoda << " (" << res.tiempoMsSinPoda << " ms)\n"
                  << "  - Nodos Con Poda : " << res.nodosConPoda << " (" << res.tiempoMsConPoda << " ms)\n"
                  << "  - Reduccion Espacio: " << std::fixed << std::setprecision(2) << res.porcentajeReduccion << " %\n"
                  << "  - Correctitud    : " << (res.correctitudCoincide ? "PASADO (Coinciden soluciones)" : "ERROR") << "\n";

    }

    // -------------------------------------------------------------------------
    // EXPORTACIÓN A ARCHIVOS (RESULTS/)
    // -------------------------------------------------------------------------

    // Exportar CSV FB
    std::ofstream csvFB("results/tabla_resultados_fb.csv");
    if (csvFB.is_open()) {
        csvFB << "Instancia,Longitud,Alfabeto,Contrasena,SHA256,FB_Tiempo_ms,FB_Candidatos,FB_Encontrado,Dic_Tiempo_ms,Dic_Encontrado\n";
        for (const auto& r : resultadosFB) {
            csvFB << r.id << "," << r.longitud << "," << r.nombreAlfabeto << ","
                  << r.passGenerada << "," << r.hashSHA256 << ","
                  << std::fixed << std::setprecision(4) << r.tiempoMsFB << ","
                  << r.candidatosFB << "," << (r.encontradoFB ? "true" : "false") << ","
                  << r.tiempoMsDic << "," << (r.encontradoDic ? "true" : "false") << "\n";
        }
        csvFB.close();
    }

    // Exportar CSV BT
    std::ofstream csvBT("results/tabla_resultados_bt.csv");
    if (csvBT.is_open()) {
        csvBT << "Instancia,n,Nodos_Sin_Poda,Nodos_Con_Poda,Porcentaje_Reduccion,Soluciones_Sin_Poda,Soluciones_Con_Poda,Tiempo_ms_Sin_Poda,Tiempo_ms_Con_Poda,Correctitud_OK\n";
        for (const auto& r : resultadosBT) {
            csvBT << r.id << "," << r.n << "," << r.nodosSinPoda << "," << r.nodosConPoda << ","
                << std::fixed << std::setprecision(2) << r.porcentajeReduccion << ","
                << r.solucionesSinPoda << "," << r.solucionesConPoda << ","
                << std::fixed << std::setprecision(4) << r.tiempoMsSinPoda << ","
                << r.tiempoMsConPoda << "," << (r.correctitudCoincide ? "true" : "false") << "\n";
        }
        csvBT.close();
    }

    std::cout << "\n Archivos generados con exito en 'results/':\n";
    std::cout << "  - results/tabla_resultados_fb.csv\n";
    std::cout << "  - results/tabla_resultados_bt.csv\n";

    return 0;
}