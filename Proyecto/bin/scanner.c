#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "zstring.h"
#include "scanner.h"
#include "cola.h"

struct Reservadas Rec_Res[7] = {
    { "CREAR_CLIENTE", 50 },
    { "ENVIAR_MENSAJE", 51 },
    { "ESTRESAR", 52 },
    { "OBTENER_MENSAJES", 53 },
    { "LISTA", 54 },
    { "LIMPIAR", 55 },
    { "BUSCAR_CLIENTE", 56 }
};

//variables globales del programa
char lexema[150];
char codigo[15];
char letra[2];
char token;
char *linea = NULL;

int ilinea = -1;
int numlinea = 1;
int estado;
int salir;
int Fin = 0;
FILE *file_in;

Nodo *iniMsjErr = NULL;
Nodo *finMsjErr = NULL;

int abrir_archivo(const char *nombre) {
    if ((file_in = fopen(nombre, "r")) == NULL)
        return 0;
    return 1;
}

void cerrar_archivo() {
    fclose(file_in);
    file_in = NULL;
}

void recuperarse(void) {
    int len = (linea != NULL) ? strlen(linea) : -1;

    do {
        Token(1);
        if (ilinea >= len) break;
    } while (!eol() && estado != E_puncoma);

    if (!estaVacia(iniMsjErr)) {
        int err = -1;

        do {
            char *msj = (char*)descolar(&iniMsjErr, &finMsjErr, &err);
            if (err == 0)
                mensaje_error(msj);
        } while (err == 0);

        printf("\n");
    }
}

void lee_car() {
    if (file_in == NULL)
        token = linea[++ilinea];
    else
        token = getc(file_in);

    if (token == '\n')
        numlinea++;
}

void ReturnFile2() {
    if (file_in == NULL)
        --ilinea;
    else
        ungetc(token, file_in);

    if (token == '\n')
        numlinea--;
}

void Identificador() {
    if (isalpha(token) || isdigit(token) || token=='_' || token=='-')
        estado = E_identificador;
    else {
        ReturnFile2();
        salir = 1;
        lexema[strlen(lexema) - 1] = '\0';
    }
}

void Inicial() {
    if ((token==' ')||(token=='\n')||(token=='\t'))
        lexema[strlen(lexema)-1]='\0';
    else if(isalpha(token))
        estado=E_identificador;
    else if(isdigit(token))
        estado=E_num;
    else if(token=='+')
        estado=E_mas;
    else if(token=='(')
        estado=E_parA;
    else if(token==')')
        estado=E_parC;
    else if(token=='\"')
        estado=E_com;
    else if(token==',')
        estado=E_coma;
    else if(token==';')
        estado=E_puncoma;
    else if(token==EOF)
        estado=E_FinArchivo;
    else {
        estado=E_ascii;
        salir=1;
    }
}

void numero() {
    if (isdigit(token))
        estado=E_num;
    else {
        ReturnFile2();
        salir = 1;
        lexema[strlen(lexema) - 1] = '\0';
    }
}

int identifica_reservada() {
    char lexema2[150];
    strcpy(lexema2, lexema);

    for (int i = 0; i < strlen(lexema); i++)
        lexema2[i] = toupper(lexema2[i]);

    for (int i = 0; i < 7; i++) {
        if (!strcasecmp(Rec_Res[i].reservada, lexema2)) {
            strcpy(lexema, lexema2);
            estado = Rec_Res[i].codigo;
            break;
        }
    }
    return estado;
}

lexico Token(int tipo) {
    lexico RegLex;

    estado = E_inicial;
    strcpy(lexema, "");
    strcpy(letra, "");
    Fin = 0;
    salir = 0;

    while (!salir && token != EOF) {
        lee_car();

        letra[0] = token;
        letra[1] = '\0';
        strcat(lexema, letra);

        switch(estado) {
            case E_inicial: Inicial(); break;
            case E_identificador: Identificador(); break;
            case E_num: numero(); break;
            default:
                salir = 1;
                lexema[strlen(lexema) - 1] = '\0';
                ReturnFile2();
                break;
        }
    }

    if (token == EOF)
        Fin = 1;

    if (estado == E_identificador && tipo == 1)
        estado = identifica_reservada();

    strcpy(RegLex.C_lex, lexema);
    RegLex.atrib = estado;

    return RegLex;
}

int eol() {
    return (token == '\n' || token == EOF);
}

void mensaje_error(const char *mensaje) {
    printf("ERROR: %s\n", mensaje);
}