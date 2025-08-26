# AFD (DFA) — C & Python

Repositorio de referencia para **Diseño e Implementación de un AFD** que lee dos archivos:
- `Conf.txt` → configuración del autómata
- `Cadenas.txt` → cadenas de prueba (una por línea)

Incluye implementaciones en **Python** y **C**, más ejemplos y un **Makefile** para compilar C.

## Estructura
```
afd-repo/
├─ README.md
├─ Conf.txt                # ejemplo de configuración
├─ Cadenas.txt             # ejemplo de entradas
├─ python/
│  └─ afd.py
└─ c/
   ├─ main.c
   └─ Makefile
```

## Formato de `Conf.txt`
```
Q = {q0,q1,q2}          # Estados
Σ = {a,b}               # Alfabeto (también se acepta 'Sigma' o 'S =')
q0                        # Estado inicial (una sola línea con el nombre)
F = {q2}                # Estados de aceptación
δ = {                    # Función de transición (una por línea)
(q0,a)->q1
(q0,b)->q0
(q1,a)->q1
(q1,b)->q2
(q2,a)->q2
(q2,b)->q2
}
```
- Se ignoran líneas vacías y comentarios que empiecen con `#`.
- Los nombres de estado/símbolo no deben contener espacios.

## `Cadenas.txt`
Una cadena por línea (sin espacios). Se ignorarán líneas vacías.

## Uso — Python
```bash
cd python
python3 afd.py ../Conf.txt ../Cadenas.txt
```
Salida ejemplo:
```
aab: ACEPTA
abb: ACEPTA
bbb: NO ACEPTA
```

## Uso — C
```bash
cd c
make           # compila a ./afd
./afd ../Conf.txt ../Cadenas.txt
```
## Pruebas rápidas
Con los archivos de ejemplo del repo:
```bash
# Python
cd python && python3 afd.py ../Conf.txt ../Cadenas.txt

# C
cd ../c && make && ./afd ../Conf.txt ../Cadenas.txt
```

## Notas
- Ambos programas validan símbolos desconocidos y transiciones faltantes.
- Si una transición no existe, la cadena es **NO ACEPTA**.
