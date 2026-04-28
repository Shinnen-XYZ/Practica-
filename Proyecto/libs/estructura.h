#ifndef __ESTRUCTURA_H
#define __ESTRUCTURA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SER_PORT 1200

typedef unsigned long int iClave;

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

/* Solo prototipos — implementacion en missing.c */
int div_and_ceil(int n, int d);
int calcPack(size_t len);
int compararInfoCliente(void *dato, void *ptrClave);
int compararInfoClienteConId(void *dato, void *ptrClave);
int compararInfoMensaje(void *dato, void *ptrClave);
void imprimirInfoCliente(void *dato);
void imprimirInfoMensaje(void *dato);
char *serializarInfoCliente(void *dato);
char *serializarInfoMensaje(void *dato);

#endif