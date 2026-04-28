#ifndef __COLA_H
   #define __COLA_H

#include "lista.h"

void encolar( Nodo **ini, Nodo **fin, void *inf ){
    insertarAlFinal( ini, fin, inf );
}

void *descolar( Nodo **ini, Nodo **fin, int *errno ) {
    return removerAlFrente( ini, fin, errno );
}

#endif