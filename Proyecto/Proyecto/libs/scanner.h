//scanner.h
#ifndef __SCANNER_H
#define __SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*constantes*/
#define false              	0
#define true               	1
#define E_identificador    	2  /*estado identificador*/
#define E_num              	3  //estado numerico
#define E_mas              	4  // estado signo "+"
#define E_puncoma          	5  // estado ";"
#define E_apos				6  // estado "'"
#define E_igual	      	  	7  // estado "="
#define E_asteris	      	8  // estado "*"
#define E_corcheteA	      	9  // estado "["
#define E_corcheteC	     	10  // estado "]"
#define E_parA            	11  // estado "("
#define E_parC            	12  // estado ")"
#define E_LlaveA	     	13  // estado "{"
#define E_LlaveC	     	14  // estado "}"
#define E_coma	     	 	15  // estado ","
#define E_preg	     	 	16  // estado "?"
#define E_or		     	17  // estado "�"
#define E_punto	     	 	18	 // estado "."
#define E_FinArchivo      	19  // estado fin archivo
#define E_ascii           	20  // estado cualquier ascii
#define E_inicial         	21  // estado inicial
#define E_texto           	22  // texto
#define E_guion           	23  // estado "-"
#define E_space           	24  // espacio en blanco
#define E_com             	25  // estado " \" "

struct lexico {
	char C_lex[ 150 ];
	int atrib;
};

//Estructura para guardar las Palabras Rervadas
struct Reservadas {
	char reservada[ 35 ];	//nombre de la reservada
	int codigo;         	//codigo
};

typedef struct lexico lexico;

//variables globales del programa
extern char lexema[ 150 ];  // Guarda el componente lexico
extern char codigo[ 15 ];
extern char letra[ 2 ];
extern char token;     // Caracter le�do desde el teclado
extern int salir;      // Bandera de Lectura
extern int Fin;
extern int numlinea;
extern int estado;     // Estado actual del scaner
extern FILE *file_in;  // Archivo de entrada
extern int ilinea;
extern char *linea;

void ReturnFile2();
void Identificador();
int abrir_archivo(const char*);
void lee_car();
void ReturnfileLexema();
void Inicial();
void numero();
int identifica_reservada();
void cerrar_archivo();
lexico Token(int);
int eol();
void recuperarse(void);
int parId( int, char ** );
int parStr( int, char**, const char* );
int parIntLargo( int, int long unsigned* );
void mensaje_error( const char* );
void mensajeConLinea( const char* );
void mensajeConLineaErr( const char* );
#endif