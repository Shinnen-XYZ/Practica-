#ifndef ZSTRING_H
#define ZSTRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// función base
char *zstr_impl(char *str1, const char *sep, char *str2);

// macros para soportar 1, 2 y 3 parámetros
#define GET_MACRO(_1,_2,_3,NAME,...) NAME
#define zstr(...) GET_MACRO(__VA_ARGS__, zstr3, zstr2, zstr1)(__VA_ARGS__)

char *zstr1(char *str);
char *zstr2(char *str1, char *str2);
char *zstr3(char *str1, const char *sep, char *str2);

/* Funciones de missing.c que necesitan prototipo en 64-bit */
char *zsubstr(char *str, int ini, int len);
char *zstring_strtok(char *str, const char *delim);
char *zstr_replace(char *str, char *rep, char *with);
char *zstrupr(char *str);

#endif