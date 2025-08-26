#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_STATES 128
#define MAX_TRANS  1024
#define MAX_NAME   64

typedef struct {
    char from[MAX_NAME];
    char symbol[MAX_NAME];
    char to[MAX_NAME];
} Transition;

typedef struct {
    char states[MAX_STATES][MAX_NAME];
    int  n_states;
    char alphabet[MAX_STATES][MAX_NAME];
    int  n_symbols;
    char initial[MAX_NAME];
    char finals[MAX_STATES][MAX_NAME];
    int  n_finals;

    Transition trans[MAX_TRANS];
    int n_trans;
} DFA;

static void trim(char *s){
    // trim leading/trailing whitespace
    int i=0, j=strlen(s)-1;
    while (isspace((unsigned char)s[i])) i++;
    while (j>=i && isspace((unsigned char)s[j])) j--;
    int k=0;
    while (i<=j) s[k++] = s[i++];
    s[k]=0;
}

static void remove_comment(char *s){
    for (int i=0; s[i]; ++i){
        if (s[i]=='#'){ s[i]=0; break; }
    }
}

static int in_set(char arr[][MAX_NAME], int n, const char* v){
    for (int i=0;i<n;i++) if (strcmp(arr[i], v)==0) return 1;
    return 0;
}

static int is_final(DFA* dfa, const char* st){
    return in_set(dfa->finals, dfa->n_finals, st);
}

static const char* step(DFA* dfa, const char* st, const char* sym){
    for (int i=0;i<dfa->n_trans;i++){
        if (strcmp(dfa->trans[i].from, st)==0 &&
            strcmp(dfa->trans[i].symbol, sym)==0){
            return dfa->trans[i].to;
        }
    }
    return NULL;
}

static void split_list(char arr[][MAX_NAME], int *n, const char* inside){
    // inside is the content inside {...}
    char buf[4096]; strncpy(buf, inside, sizeof(buf)); buf[sizeof(buf)-1]=0;
    char *tok = strtok(buf, ",");
    while(tok){
        char x[MAX_NAME]; strncpy(x, tok, sizeof(x)); x[sizeof(x)-1]=0;
        trim(x);
        if (strlen(x)>0){
            strncpy(arr[*n], x, MAX_NAME); arr[*n][MAX_NAME-1]=0;
            (*n)++;
        }
        tok = strtok(NULL, ",");
    }
}

static void parse_conf(const char* path, DFA* dfa){
    memset(dfa, 0, sizeof(*dfa));
    FILE *f = fopen(path, "r");
    if (!f){ perror("Conf.txt"); exit(1); }
    char line[4096];
    int reading_delta = 0;

    while (fgets(line, sizeof(line), f)){
        remove_comment(line);
        trim(line);
        if (strlen(line)==0) continue;

        if (strstr(line, "δ") || strcasestr(line, "delta")){
            reading_delta = 1;
            continue;
        }
        if (reading_delta){
            if (line[0]=='}'){ reading_delta = 0; continue; }
            // (q0,a)->q1   or (q0, a)->q1
            char from[MAX_NAME], sym[MAX_NAME], to[MAX_NAME];
            if (sscanf(line, " ( %63[^,] , %63[^) ] ) -> %63s ", from, sym, to) == 3){
                trim(from); trim(sym); trim(to);
                strncpy(dfa->trans[dfa->n_trans].from, from, MAX_NAME);
                strncpy(dfa->trans[dfa->n_trans].symbol, sym, MAX_NAME);
                strncpy(dfa->trans[dfa->n_trans].to, to, MAX_NAME);
                dfa->n_trans++;
            } else {
                fprintf(stderr, "Transición mal formada: %s\n", line);
                exit(1);
            }
            continue;
        }

        // Estados Q
        char inside[4096];
        if (sscanf(line, "Q = { %4095[^}] }", inside)==1){
            split_list(dfa->states, &dfa->n_states, inside);
            continue;
        }
        // Alfabeto Σ | Sigma | S | Sig
        if (sscanf(line, "Σ = { %4095[^}] }", inside)==1 ||
            sscanf(line, "Sigma = { %4095[^}] }", inside)==1 ||
            sscanf(line, "Sig = { %4095[^}] }", inside)==1 ||
            sscanf(line, "S = { %4095[^}] }", inside)==1){
            split_list(dfa->alphabet, &dfa->n_symbols, inside);
            continue;
        }
        // Finales F
        if (sscanf(line, "F = { %4095[^}] }", inside)==1){
            split_list(dfa->finals, &dfa->n_finals, inside);
            continue;
        }
        // Inicial: línea sin '=' y sin llaves
        if (!strchr(line, '=') && !strchr(line, '{') && !strchr(line, '}')){
            strncpy(dfa->initial, line, MAX_NAME);
            dfa->initial[MAX_NAME-1]=0;
            continue;
        }
    }
    fclose(f);

    if (dfa->n_states==0 || dfa->n_symbols==0 || strlen(dfa->initial)==0 || dfa->n_finals==0){
        fprintf(stderr, "Configuración incompleta en Conf.txt\n");
        exit(1);
    }
}

int main(int argc, char** argv){
    if (argc!=3){
        fprintf(stderr, "Uso: %s Conf.txt Cadenas.txt\n", argv[0]);
        return 1;
    }
    DFA dfa;
    parse_conf(argv[1], &dfa);

    FILE* g = fopen(argv[2], "r");
    if (!g){ perror("Cadenas.txt"); return 1; }

    char buf[4096];
    while (fgets(buf, sizeof(buf), g)){
        remove_comment(buf);
        trim(buf);
        if (strlen(buf)==0) continue;
        const char* state = dfa.initial;
        int ok = 1;
        // Iteramos símbolo por símbolo; admitimos símbolos de longitud 1 o más (tokens).
        // Si el alfabeto son símbolos de 1 caracter, se recorre como chars.
        // Aquí asumimos caracteres individuales (común en AFD). Si deseas tokens, separa por espacios.
        for (size_t i=0; i<strlen(buf); ++i){
            char sym[2] = { buf[i], 0 };
            if (!in_set(dfa.alphabet, dfa.n_symbols, sym)){ ok = 0; break; }
            const char* next = step(&dfa, state, sym);
            if (!next){ ok = 0; break; }
            state = next;
        }
        printf("%s: %s\n", buf, (ok && is_final(&dfa, state)) ? "ACEPTA" : "NO ACEPTA");
    }
    fclose(g);
    return 0;
}
