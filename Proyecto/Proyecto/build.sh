#!/bin/bash
# Ejecutar desde la carpeta raiz del Proyecto: bash build.sh

set -e

echo "=== Compilando servidor ==="
gcc servidor.c \
    bin/lista.c \
    bin/scanner.c \
    bin/zstring.c \
    -I libs \
    -o servidor \
    -lpthread \
    -Wall -Wno-unused-result

echo "=== Compilando cliente ==="
gcc cliente.c \
    bin/lista.c \
    bin/scanner.c \
    bin/zstring.c \
    -I libs \
    -o cliente \
    -lpthread \
    -Wall -Wno-unused-result

echo ""
echo "Compilacion lista."
echo "  Terminal 1:  ./servidor"
echo "  Terminal 2:  ./cliente localhost"
echo "  Terminal 3:  ./cliente localhost -f entrada.txt"