#!/usr/bin/env python3
import sys
import re

def _strip_comments(line: str) -> str:
    line = line.strip()
    if "#" in line:
        line = line.split("#", 1)[0].rstrip()
    return line

def parse_conf(path):
    with open(path, encoding="utf-8") as f:
        raw_lines = [ _strip_comments(l) for l in f.readlines() ]
    # remove empty lines
    lines = [l for l in raw_lines if l]

    if len(lines) < 5:
        raise ValueError("Archivo de configuración incompleto.")

    # Estados
    m = re.search(r"Q\s*=\s*\{([^}]*)\}", " ".join(lines))
    if not m:
        raise ValueError("No se pudo leer el conjunto de estados Q.")
    estados = [s.strip() for s in m.group(1).split(",") if s.strip()]

    # Alfabeto: admite varias etiquetas
    whole = " ".join(lines)
    m = re.search(r"(Σ|Sigma|Sig|S)\s*=\s*\{([^}]*)\}", whole)
    if not m:
        raise ValueError("No se pudo leer el alfabeto Σ.")
    alfabeto = [s.strip() for s in m.group(2).split(",") if s.strip()]

    # Estado inicial: primera línea que no contiene '=' y no es llave
    inicial = None
    for l in lines:
        if ("=" not in l) and ("{" not in l) and ("}" not in l):
            inicial = l.strip()
            break
    if inicial is None:
        raise ValueError("No se encontró el estado inicial.")

    # Finales
    m = re.search(r"F\s*=\s*\{([^}]*)\}", whole)
    if not m:
        raise ValueError("No se pudo leer el conjunto de estados finales F.")
    finales = { s.strip() for s in m.group(1).split(",") if s.strip() }

    # Transiciones
    trans = {}
    inside = False
    for l in lines:
        if "δ" in l or "delta" in l.lower():
            inside = True
            continue
        if inside and l.startswith("}"):
            break
        if inside:
            if "->" not in l:
                continue
            # formato: (q0,a)->q1
            m = re.match(r"\(\s*([^,]+)\s*,\s*([^)]+)\s*\)\s*->\s*(\S+)", l)
            if not m:
                raise ValueError(f"Transición mal formada: {l}")
            q, a, q2 = m.group(1).strip(), m.group(2).strip(), m.group(3).strip()
            trans[(q, a)] = q2

    return estados, set(alfabeto), inicial, finales, trans

def eval_cadena(cad, inicial, finales, trans, alfabeto):
    estado = inicial
    for ch in cad.strip():
        if ch not in alfabeto:
            return False
        nxt = trans.get((estado, ch))
        if nxt is None:
            return False
        estado = nxt
    return estado in finales

def main():
    if len(sys.argv) != 3:
        print("Uso: python afd.py Conf.txt Cadenas.txt")
        sys.exit(1)
    conf_path, cadenas_path = sys.argv[1], sys.argv[2]
    estados, alfabeto, inicial, finales, trans = parse_conf(conf_path)

    with open(cadenas_path, encoding="utf-8") as f:
        cadenas = [l.strip() for l in f if l.strip()]

    for cad in cadenas:
        ok = eval_cadena(cad, inicial, finales, trans, alfabeto)
        print(f"{cad}: {'ACEPTA' if ok else 'NO ACEPTA'}")

if __name__ == "__main__":
    main()
