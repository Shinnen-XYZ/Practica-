#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "estructura.h"

/* ============================================================
   VARIABLE GLOBAL compartida entre servidor y cliente
   ============================================================ */
int GblflgImpLin = 1;

/* ============================================================
   FUNCIONES DE estructura.h — implementadas aqui una sola vez
   ============================================================ */
int div_and_ceil(int n, int d) {
    return (n % d == 0) ? (n / d) : (n / d + 1);
}

int calcPack(size_t len) {
    return div_and_ceil(len, BUFFER_LEN - 1);
}

int compararInfoCliente(void *dato, void *ptrClave) {
    InfoCliente *inf = (InfoCliente*)dato;
    iClave clave = *((iClave*)ptrClave);
    return (inf->cui == clave || inf->id == clave) ? 1 : 0;
}

int compararInfoClienteConId(void *dato, void *ptrClave) {
    InfoCliente *inf = (InfoCliente*)dato;
    iClave clave = *((iClave*)ptrClave);
    return (inf->id == clave) ? 1 : 0;
}

int compararInfoMensaje(void *dato, void *ptrClave) {
    InfoMensaje *inf = (InfoMensaje*)dato;
    iClave clave = *((iClave*)ptrClave);
    return (inf->idDestino == clave) ? 1 : 0;
}

void imprimirInfoCliente(void *dato) {
    InfoCliente *inf = (InfoCliente*)dato;
    printf("\n\t--------------------------");
    if (GblflgImpLin)
        printf("\n\tLinea:\t\t%d", inf->numlinea);
    printf("\n\tId:\t\t%d",       inf->id);
    printf("\n\tNo. Dpi:\t%lu",   inf->cui);
    printf("\n\tNombre:\t\t%s",   inf->nombre);
    printf("\n\tApellidos:\t%s",  inf->apellidos);
    printf("\n\tDomicilio:\t%s",  inf->domicilio);
    printf("\n\tMovil:\t\t%lu",   inf->movil);
    printf("\n\tTelefono:\t%lu",  inf->telefono);
    printf("\n\tCorreo:\t\t%s",   inf->correoe);
}

void imprimirInfoMensaje(void *dato) {
    InfoMensaje *inf = (InfoMensaje*)dato;
    printf("\n\t--------------------------");
    if (GblflgImpLin)
        printf("\n\tLinea:\t\t%d", inf->numlinea);
    printf("\n\tId_origen:\t%lu",  inf->idOrigen);
    printf("\n\tId_destino:\t%lu", inf->idDestino);
    printf("\n\tMensaje:\t%s",     inf->mensaje);
    printf("\n\tEstado:\t\t%d",    inf->estado);
}

/* ============================================================
   SERIALIZACIÓN — formato compatible con el protocolo
   ============================================================ */
static char *escapar_mc(const char *s) {
    if (!s) return strdup("");
    size_t extra = 0;
    for (const char *p = s; *p; p++) if (*p == '#') extra++;
    char *r = malloc(strlen(s) + extra + 1);
    char *w = r;
    for (const char *p = s; *p; p++) {
        if (*p == '#') *w++ = '\\';
        *w++ = *p;
    }
    *w = '\0';
    return r;
}

/* numlinea#id#cui#nombre#apellidos#domicilio#movil#telefono#correoe */
char *serializarInfoCliente(void *dato) {
    InfoCliente *inf = (InfoCliente*)dato;
    char *nom = escapar_mc(inf->nombre);
    char *ape = escapar_mc(inf->apellidos);
    char *dom = escapar_mc(inf->domicilio);
    char *cor = escapar_mc(inf->correoe);
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%d#%u#%lu#%s#%s#%s#%lu#%lu#%s",
             inf->numlinea, inf->id, inf->cui,
             nom, ape, dom, inf->movil, inf->telefono, cor);
    free(nom); free(ape); free(dom); free(cor);
    return strdup(tmp);
}

/* numlinea#idOrigen#idDestino#mensaje#estado */
char *serializarInfoMensaje(void *dato) {
    InfoMensaje *inf = (InfoMensaje*)dato;
    char *msj = escapar_mc(inf->mensaje);
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%d#%lu#%lu#%s#%u",
             inf->numlinea, inf->idOrigen, inf->idDestino,
             msj, inf->estado);
    free(msj);
    return strdup(tmp);
}

/* ============================================================
   UTILIDADES DE CADENAS
   ============================================================ */
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
    char *pos = strstr(str, rep);
    if (!pos) return str;
    size_t repLen  = strlen(rep);
    size_t withLen = strlen(with);
    char *res = malloc(strlen(str) - repLen + withLen + 1);
    size_t prefix = pos - str;
    strncpy(res, str, prefix);
    strcpy(res + prefix, with);
    strcpy(res + prefix + withLen, pos + repLen);
    return res;
}

char *zstrupr(char *str) {
    for (int i = 0; str[i]; i++)
        if (str[i] >= 'a' && str[i] <= 'z')
            str[i] -= 32;
    return str;
}

char *zscanf(void) {
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