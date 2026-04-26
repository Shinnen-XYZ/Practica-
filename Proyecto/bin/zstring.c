#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libs/zstring.h"

char *zstr1(char *str) {
    char *res = (char*)malloc(strlen(str) + 1);
    strcpy(res, str);
    return res;
}

char *zstr2(char *str1, char *str2) {
    char *res = (char*)malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(res, str1);
    strcat(res, str2);
    return res;
}

char *zstr3(char *str1, const char *sep, char *str2) {
    char *res = (char*)malloc(strlen(str1) + strlen(sep) + strlen(str2) + 1);
    strcpy(res, str1);
    strcat(res, sep);
    strcat(res, str2);
    return res;
}