# Práctica 1: Fuerza Bruta y Backtracking

**Contraseñas bajo ataque y bajo diseño: enumeración exhaustiva y construcción con poda**

* **Curso:** Análisis y Diseño de Algoritmos
* **Estudiante:** Jaisson Steven Merchán Ardila
* **Lenguaje:** C++17
* **Biblioteca de hash:** `picosha2` (v. 20140213, Licencia MIT)

\---

## Descripción del Proyecto

Este proyecto implementa y contrasta dos paradigmas fundamentales de búsqueda exhaustiva aplicados al dominio de cadenas de caracteres sobre un alfabeto finito, abordando el problema desde dos perspectivas opuestas:

1. **Módulo de Fuerza Bruta (FB) — Rol del Analista/Atacante:**
Dado el hash criptográfico **SHA-256** de una contraseña sintética, el algoritmo enumera sistemáticamente en orden lexicográfico todas las cadenas posibles de longitud $1 \\dots L$ sobre un alfabeto $\\Sigma$ hasta encontrar una coincidencia (resistencia a preimagen). Además, implementa un **ataque por diccionario** sobre un conjunto predefinido de 500 palabras para contrastar la eficiencia frente a la exhaustividad.
2. **Módulo de Backtracking (BT) — Rol del Diseñador de Políticas:**
Dado un alfabeto de 67 símbolos ($\\Sigma = \\Sigma\_L \\cup \\Sigma\_U \\cup \\Sigma\_D \\cup \\Sigma\_S$), una longitud fija $n$ y una política de contraseñas $\\pi = (m\_L, m\_U, m\_D, m\_S)$ junto con la prohibición de caracteres idénticos consecutivos, el algoritmo construye las contraseñas carácter por carácter y cuenta cuántas cumplen las reglas. Implementa una función de factibilidad incremental para **podar** ramas infactibles de forma temprana, comparando su rendimiento frente a una versión de referencia **sin poda**.

> \*\*Nota Ética:\*\* Todas las contraseñas objetivo son sintéticas y fueron generadas mediante un generador lineal congruencial (LCG) determinista con fines estrictamente académicos. Este software no ha sido utilizado contra ningún sistema real.

\---

## Estructura del Proyecto

```
├── resources/
│   └── diccionario.txt            # Diccionario de 500 entradas únicas para ataque no exhaustivo
├── results/
│   ├── reporte\_resultados\_fb.txt  # Reporte detallado de ejecución secundaria (Corrida B)
│   ├── tabla\_resultados\_bt.csv    # Métricas exportadas del Módulo Backtracking
│   └── tabla\_resultados\_fb.csv    # Métricas exportadas del Módulo Fuerza Bruta (Corrida A)
├── src/
│   ├── third\_party/
│   │   └── picosha2.h             # Biblioteca header-only para cálculo de SHA-256 (Licencia MIT)
│   └── main.cpp                   # Código fuente principal (LCG, Módulo FB, Módulo BT y exportación CSV)
├── tests/
│   ├── test\_referencias.cpp       # Pruebas unitarias con instancias de referencia (ej. "abc12")
│   └── verificar\_semilla.cpp      # Utilidad para validar el cálculo de la semilla ASCII
└── README.md
```

\---

## Requisitos, Compilación y Ejecución

### Requisitos

* Compilador compatible con el estándar **C++17** (GCC `g++` 7+, Clang 5+ o MSVC).
* Uso de librerías estándar `<chrono>`, `<filesystem>` y `<fstream>`.

### Compilación

Desde la **raíz del proyecto** (importante para mantener las rutas relativas hacia `resources/` y `results/`):

```bash
# Compilar el programa principal con optimización O2
g++ -std=c++17 -O2 -o ada\_p1 src/main.cpp

# Compilar las pruebas y utilidades (opcional)
g++ -std=c++17 -O2 -o tests/verificar\_semilla tests/verificar\_semilla.cpp
g++ -std=c++17 -O2 -o tests/test\_referencias tests/test\_referencias.cpp
```

### Ejecución

```bash
# Ejecutar el experimento completo (genera los archivos CSV en results/)
./ada\_p1

# Ejecutar verificación de semilla o pruebas de referencia
./tests/verificar\_semilla
./tests/test\_referencias
```



\---

## Modelamiento y Complejidad Algorítmica

### Alfabetos Definidos

* **$A\_1$ (Minúsculas):** ${a \\dots z}$, $|\\Sigma| = 26$.
* **$A\_2$ (Alfanumérico minúsculas):** ${a \\dots z, 0 \\dots 9}$, $|\\Sigma| = 36$.
* **$\\Sigma\_{BT}$ (Completo):** 26 minúsculas + 26 mayúsculas + 10 dígitos + 5 símbolos (`! @ # $ %`), $|\\Sigma| = 67$.

### Resumen de Complejidad

|Algoritmo|Complejidad Temporal (Peor Caso)|Complejidad Espacial|Notas Clave|
|-|-|-|-|
|**Fuerza Bruta (FB)**|$\\Theta(|\\Sigma|^L \\cdot c\_h)$|
|**Ataque Diccionario**|$\\Theta(|D|\\cdot c\_h)$|
|**Backtracking Sin Poda**|$\\Theta(n \\cdot|\\Sigma|^n)$|
|**Backtracking Con Poda**|$O(|\\Sigma|\\cdot V)$|

*(Donde $c\_h$ es el costo $\\Theta(1)$ de calcular un bloque SHA-256 de 512 bits para mensajes $\\le 55$ bytes, y $V$ es el número de nodos visitados).*

\---

## 📊 Resumen de Resultados Experimentales

Las contraseñas objetivo de FB fueron generadas con la semilla **1355** (`"ardilamerchan"` $\\pmod{100000}$) mediante un LCG ($x\_{i+1} = (1103515245 x\_i + 12345) \\bmod 2^{31}$).

### 1\. Módulo de Fuerza Bruta vs. Diccionario

* **Efectividad:** Fuerza Bruta logró **5/5 aciertos** (100%), mientras que el diccionario obtuvo **0/5 aciertos** (0%) debido a que las contraseñas sintéticas uniformes no siguen el sesgo humano.
* **El Muro Exponencial:** Con un costo promedio de $\\approx 13.43,\\mu\\text{s}$ por candidato (Corrida A), el límite de tratabilidad (< 1 hora para un barrido completo) se alcanza en **$n = 6$** ($\\approx 71.9\\text{ min}$ para $A\_1$ y $\\approx 8.4\\text{ h}$ para $A\_2$).

|#|Alfabeto, $n$|Contraseña|Candidatos Evaluados|% Espacio $S\_L$|Tiempo FB (A)|Tiempo Diccionario (A)|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|**1**|$A\_1, 4$|`mpip`|239 554|50.4%|3.08 s|6.27 ms|
|**2**|$A\_2, 4$|`i3c1`|458 920|26.6%|5.78 s|28.80 ms|
|**3**|$A\_1, 5$|`uhope`|9 747 665|78.9%|141.93 s|6.60 ms|
|**4**|$A\_2, 5$|`folmz`|10 793 582|17.4%|144.80 s|6.19 ms|
|**5**|$A\_1, 6$|`kzclex`|142 637 506|44.4%|1 906.08 s|7.93 ms|

### 2\. Módulo de Backtracking (Con Poda vs. Sin Poda)

Todas las configuraciones evaluadas poseen holgura cero ($s = n - \\sum m\_c = 0$), maximizando el impacto de la regla de capacidad ($R\_2$) y de no repetición consecutiva ($R\_1$):

|#|$n$|Política $(m\_L, m\_U, m\_D, m\_S)$|Nodos Sin Poda|Nodos Con Poda|Reducción de Nodos|Aceleración Tiempo|Soluciones Válidas|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|**1**|3|`(1, 1, 1, 0)`|305 320|43 015|**85.91%**|4.34x|40 560|
|**2**|4|`(1, 1, 1, 1)`|20 456 441|890 720|**95.65%**|14.26x|811 200|
|**3**|4|`(2, 1, 1, 0)`|20 456 441|2 183 361|**89.33%**|7.39x|2 068 560|

\---

