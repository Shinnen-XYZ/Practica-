#!/bin/bash

gcc servidor.c lista.c scanner.c zstring.c -o servidor
gcc cliente.c lista.c scanner.c zstring.c -o cliente

echo "Compilación lista"
