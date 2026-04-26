#ifndef __PILA_H
   #define __PILA_H

#include "lista.h"

void push( Nodo **ini, Nodo **fin, void *inf ){
    insertarAlFrente( ini, fin, inf );
}

void *pop( Nodo **ini, Nodo **fin, int *errno ) {
    return removerAlFrente( ini, fin, errno );
}

#endif