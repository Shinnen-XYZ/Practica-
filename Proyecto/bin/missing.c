#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ===== FUNCIONES QUE NO EXISTEN ===== */

unsigned long int parIntLargo(char *str) {
    return strtoul(str, NULL, 10);
}

char *parStr(char *str) {
    return str;
}

char *parId(char *str) {
    return str;
}

char *zsubstr(char *str, int ini, int len) {
    char *res = (char*)malloc(len + 1);
    strncpy(res, str + ini, len);
    res[len] = '\0';
    return res;
}

char *zstring_strtok(char *str, const char *delim) {
    return strtok(str, delim);
}

char *zstr_replace(char *str, char *rep, char *with) {
    return str;
}

char *zstrupr(char *str) {
    for (int i = 0; str[i]; i++)
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] -= 32;
    return str;
}

char *zscanf() {
    char *buffer = (char*)malloc(256);
    fgets(buffer, 256, stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    return buffer;
}

void mensajeConLinea(char *str) {
    printf("%s\n", str);
}

void mensajeConLineaErr(char *str) {
    printf("ERROR: %s\n", str);
}

char *serializarInfoCliente(void *info) {
    return "cliente";
}

char *serializarInfoMensaje(void *info) {
    return "mensaje";
}

/* ===== VARIABLE GLOBAL (SOLO AQUÍ) ===== */

int GblflgImpLin = 1;