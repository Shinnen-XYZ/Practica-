#!/bin/bash
set -e

echo "=== Compilando servidor ==="
gcc -w servidor.c bin/lista.c bin/zstring.c bin/missing.c -I libs -o servidor -lpthread

echo "=== Compilando cliente ==="
gcc -w cliente.c bin/lista.c bin/zstring.c bin/scanner.c bin/missing.c -I libs -o cliente -lpthread

echo ""
echo "Compilacion lista."
echo "  Terminal 1:  ./servidor"
echo "  Terminal 2:  ./cliente localhost"
echo "  Terminal 3:  ./cliente localhost -f entrada.txt"