#ifndef __ESTRUCTURA_H
#define __ESTRUCTURA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SER_PORT 1200

typedef unsigned long int iClave;

/* IMPORTANTE: solo declaración */
extern int GblflgImpLin;

struct infoCliente {
    int numlinea;
    unsigned int id;
    unsigned long int cui;
    char *nombre;
    char *apellidos;
    char *domicilio;
    unsigned long int movil;
    unsigned long int telefono;
    char *correoe;
};

struct infoBuscarCliente {
    int numlinea;
    unsigned int id;
    unsigned long int cui;
};

typedef struct infoMensaje {
    int numlinea;
    unsigned long int idOrigen;
    unsigned long int idDestino;
    char *mensaje;
    unsigned int estado;
} InfoMensaje;

typedef struct infoCliente InfoCliente;
typedef struct infoBuscarCliente InfoBuscarCliente;

enum { BUFFER_LEN = 20 };
enum { ENVIADO = 0, RECIBIDO = 1, TRASLADADO = 2 };

int div_and_ceil(int n, int d) {
    return (n % d == 0) ? (n / d) : (n / d + 1);
}

int calcPack(size_t len) {
    return div_and_ceil(len, BUFFER_LEN - 1);
}

int compararInfoCliente(void *dato, void *ptrClave) {
    InfoCliente *inf = (InfoCliente*)dato;
    iClave clave = *((iClave*)ptrClave);

    if (inf->cui == clave || inf->id == clave)
        return 1;

    return 0;
}

int compararInfoClienteConId(void *dato, void *ptrClave) {
    InfoCliente *inf = (InfoCliente*)dato;
    iClave clave = *((iClave*)ptrClave);

    if (inf->id == clave)
        return 1;

    return 0;
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
        printf("\n\tLínea:\t\t%d", inf->numlinea);

    printf("\n\tId:\t\t%d", inf->id);
    printf("\n\tNo. Dpi:\t%lu", inf->cui);
    printf("\n\tNombre:\t\t%s", inf->nombre);
    printf("\n\tApellidos:\t%s", inf->apellidos);
    printf("\n\tDomicilio:\t%s", inf->domicilio);
    printf("\n\tMóvil:\t\t%lu", inf->movil);
    printf("\n\tTeléfono:\t%lu", inf->telefono);
    printf("\n\tCorreo:\t\t%s", inf->correoe);
}

void imprimirInfoMensaje(void *dato) {
    InfoMensaje *inf = (InfoMensaje*)dato;

    printf("\n\t--------------------------");

    if (GblflgImpLin)
        printf("\n\tLínea:\t\t%d", inf->numlinea);

    printf("\n\tId_origen:\t%lu", inf->idOrigen);
    printf("\n\tId_destino:\t%lu", inf->idDestino);
    printf("\n\tMensaje:\t%s", inf->mensaje);
    printf("\n\tEstado:\t\t%d", inf->estado);
}

#endif