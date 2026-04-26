#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "libs/zstring.h"
#include "libs/lista.h"
#include "libs/estructura.h"

static Nodo *iniClientes = NULL, *finClientes = NULL;
static Nodo *iniMensajes = NULL, *finMensajes = NULL;
static pthread_mutex_t mtxClientes = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtxMensajes = PTHREAD_MUTEX_INITIALIZER;
static unsigned int contadorId = 0;

#define ARCHIVO_CLIENTES "clientes.dat"
#define ARCHIVO_MENSAJES "mensajes.dat"

/* GblflgImpLin viene de missing.c */

static char *escapar( const char *s ) {
    if (!s) return strdup("");
    size_t extra = 0;
    for (const char *p = s; *p; p++) if (*p == '#') extra++;
    char *r = malloc(strlen(s) + extra + 1);
    char *w = r;
    for (const char *p = s; *p; p++) {
        if (*p == '#') *w++ = '\\';
        *w++ = *p;
    }
    *w = '\0';
    return r;
}

static char *serializarCliente( InfoCliente *inf ) {
    char *nom = escapar(inf->nombre);
    char *ape = escapar(inf->apellidos);
    char *dom = escapar(inf->domicilio);
    char *cor = escapar(inf->correoe);
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%d#%u#%lu#%s#%s#%s#%lu#%lu#%s",
             inf->numlinea, inf->id, inf->cui,
             nom, ape, dom, inf->movil, inf->telefono, cor);
    free(nom); free(ape); free(dom); free(cor);
    return strdup(tmp);
}

static char *serializarMensaje( InfoMensaje *inf ) {
    char *msj = escapar(inf->mensaje);
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%d#%lu#%lu#%s#%u",
             inf->numlinea, inf->idOrigen, inf->idDestino, msj, inf->estado);
    free(msj);
    return strdup(tmp);
}

static void persistirClientes( void ) {
    FILE *f = fopen(ARCHIVO_CLIENTES, "w");
    if (!f) return;
    Nodo *n = iniClientes;
    while (n) {
        char *s = serializarCliente((InfoCliente*)n->dato);
        fprintf(f, "%s\n", s);
        free(s);
        n = n->ptr_sig;
    }
    fclose(f);
}

static void persistirMensajes( void ) {
    FILE *f = fopen(ARCHIVO_MENSAJES, "w");
    if (!f) return;
    Nodo *n = iniMensajes;
    while (n) {
        char *s = serializarMensaje((InfoMensaje*)n->dato);
        fprintf(f, "%s\n", s);
        free(s);
        n = n->ptr_sig;
    }
    fclose(f);
}

static char *leerPaquetes( int sock ) {
    size_t len = 0;
    read(sock, &len, sizeof(size_t));
    int pack = calcPack(len);
    char *acum = calloc(pack * BUFFER_LEN + 1, 1);
    char buf[BUFFER_LEN];
    for (int i = 0; i < pack; i++) {
        memset(buf, 0, BUFFER_LEN);
        read(sock, buf, BUFFER_LEN);
        strcat(acum, buf);
    }
    return acum;
}

static void enviarPaquetes( int sock, const char *str ) {
    size_t len = strlen(str);
    write(sock, &len, sizeof(size_t));
    int pack = calcPack(len);
    char buf[BUFFER_LEN];
    for (int i = 0; i < pack; i++) {
        int ini = (i == 0) ? 0 : i * (BUFFER_LEN - 1);
        memset(buf, 0, BUFFER_LEN);
        strncpy(buf, str + ini, BUFFER_LEN - 1);
        write(sock, buf, BUFFER_LEN);
    }
}

static InfoCliente *deserializarCliente( char *raw ) {
    InfoCliente *c = calloc(1, sizeof(InfoCliente));
    char *copia = strdup(raw);
    char *tok;
    tok = strtok(copia, "#"); c->numlinea  = tok ? atoi(tok)            : 0;
    tok = strtok(NULL,  "#"); /* id lo asigna el servidor */
    tok = strtok(NULL,  "#"); c->cui       = tok ? strtoul(tok,NULL,10) : 0;
    tok = strtok(NULL,  "#"); c->nombre    = tok ? strdup(tok)          : strdup("");
    tok = strtok(NULL,  "#"); c->apellidos = tok ? strdup(tok)          : strdup("");
    tok = strtok(NULL,  "#"); c->domicilio = tok ? strdup(tok)          : strdup("");
    tok = strtok(NULL,  "#"); c->movil     = tok ? strtoul(tok,NULL,10) : 0;
    tok = strtok(NULL,  "#"); c->telefono  = tok ? strtoul(tok,NULL,10) : 0;
    tok = strtok(NULL,  "#"); c->correoe   = tok ? strdup(tok)          : strdup("");
    free(copia);
    return c;
}

static InfoMensaje *deserializarMensaje( char *raw ) {
    InfoMensaje *m = calloc(1, sizeof(InfoMensaje));
    char *copia = strdup(raw);
    char *tok;
    tok = strtok(copia, "#"); m->numlinea  = tok ? atoi(tok)            : 0;
    tok = strtok(NULL,  "#"); m->idOrigen  = tok ? strtoul(tok,NULL,10) : 0;
    tok = strtok(NULL,  "#"); m->idDestino = tok ? strtoul(tok,NULL,10) : 0;
    tok = strtok(NULL,  "#"); m->mensaje   = tok ? strdup(tok)          : strdup("");
    tok = strtok(NULL,  "#"); m->estado    = tok ? (unsigned)atoi(tok)  : ENVIADO;
    free(copia);
    return m;
}

static void cmd_crearCliente( int sock, char *raw ) {
    InfoCliente *nuevo = deserializarCliente(raw);
    pthread_mutex_lock(&mtxClientes);
    void *encontrado = NULL;
    iClave clave = nuevo->cui;
    buscarDentroDeLista(iniClientes, compararInfoCliente, &clave, &encontrado);
    int respuesta;
    if (encontrado) {
        respuesta = -1;
        free(nuevo);
    } else {
        nuevo->id = ++contadorId;
        insertarAlFinal(&iniClientes, &finClientes, nuevo);
        persistirClientes();
        respuesta = (int)nuevo->id;
        printf("[Servidor] Cliente creado  id=%u  cui=%lu  %s %s\n",
               nuevo->id, nuevo->cui, nuevo->nombre, nuevo->apellidos);
    }
    pthread_mutex_unlock(&mtxClientes);
    write(sock, &respuesta, sizeof(respuesta));
}

static void cmd_enviarMensaje( int sock, char *raw ) {
    InfoMensaje *msg = deserializarMensaje(raw);
    pthread_mutex_lock(&mtxClientes);
    void *eo = NULL, *ed = NULL;
    iClave ko = msg->idOrigen, kd = msg->idDestino;
    buscarDentroDeLista(iniClientes, compararInfoClienteConId, &ko, &eo);
    buscarDentroDeLista(iniClientes, compararInfoClienteConId, &kd, &ed);
    pthread_mutex_unlock(&mtxClientes);
    int respuesta;
    if (!eo) {
        respuesta = -1;
    } else if (!ed) {
        respuesta = -2;
    } else {
        pthread_mutex_lock(&mtxMensajes);
        insertarAlFinal(&iniMensajes, &finMensajes, msg);
        persistirMensajes();
        pthread_mutex_unlock(&mtxMensajes);
        respuesta = 1;
        printf("[Servidor] Mensaje almacenado  %lu -> %lu\n",
               msg->idOrigen, msg->idDestino);
    }
    write(sock, &respuesta, sizeof(respuesta));
}

static void cmd_obtenerMensajes( int sock ) {
    unsigned long int idCliente = 0;
    read(sock, &idCliente, sizeof(idCliente));
    Nodo *ini = NULL, *fin = NULL;
    pthread_mutex_lock(&mtxMensajes);
    Nodo *n = iniMensajes;
    while (n) {
        InfoMensaje *m = (InfoMensaje*)n->dato;
        if (m->idDestino == idCliente)
            insertarAlFinal(&ini, &fin, m);
        n = n->ptr_sig;
    }
    pthread_mutex_unlock(&mtxMensajes);
    int total = contarElementosLista(ini);
    if (total < 0) total = 0;
    unsigned int uTotal = (unsigned int)total;
    write(sock, &uTotal, sizeof(uTotal));
    Nodo *cur = ini;
    while (cur) {
        char *s = serializarMensaje((InfoMensaje*)cur->dato);
        enviarPaquetes(sock, s);
        free(s);
        cur = cur->ptr_sig;
    }
}

static void cmd_buscarCliente( int sock ) {
    InfoBuscarCliente info;
    read(sock, &info, sizeof(InfoBuscarCliente));
    pthread_mutex_lock(&mtxClientes);
    void *resultado = NULL;
    iClave clave = info.cui;
    buscarDentroDeLista(iniClientes, compararInfoCliente, &clave, &resultado);
    pthread_mutex_unlock(&mtxClientes);
    int respuesta;
    if (!resultado) {
        respuesta = -1;
        write(sock, &respuesta, sizeof(respuesta));
    } else {
        InfoCliente *c = (InfoCliente*)resultado;
        respuesta = (int)c->id;
        write(sock, &respuesta, sizeof(respuesta));
        char *s = serializarCliente(c);
        enviarPaquetes(sock, s);
        free(s);
    }
}

static void cmd_lista( int sock, char *codigo ) {
    if (strcasecmp(codigo, "C") == 0) {
        printf("[Servidor] Lista de clientes:\n");
        pthread_mutex_lock(&mtxClientes);
        GblflgImpLin = 0;
        imprimirLista(iniClientes, imprimirInfoCliente);
        GblflgImpLin = 1;
        pthread_mutex_unlock(&mtxClientes);
    } else if (strcasecmp(codigo, "M") == 0) {
        printf("[Servidor] Lista de mensajes:\n");
        pthread_mutex_lock(&mtxMensajes);
        GblflgImpLin = 0;
        imprimirLista(iniMensajes, imprimirInfoMensaje);
        GblflgImpLin = 1;
        pthread_mutex_unlock(&mtxMensajes);
    }
}

typedef struct { int sock; } ArgsHilo;

static void *atenderCliente( void *arg ) {
    ArgsHilo *args = (ArgsHilo*)arg;
    int sock = args->sock;
    free(args);
    char cmd[BUFFER_LEN];
    while (1) {
        memset(cmd, 0, BUFFER_LEN);
        int n = read(sock, cmd, BUFFER_LEN);
        if (n <= 0) break;
        printf("[Servidor] Comando: '%s'\n", cmd);
        if      (strncmp(cmd, "CREAR_CLIENTE",   13) == 0) { char *r = leerPaquetes(sock); cmd_crearCliente(sock, r);  free(r); }
        else if (strncmp(cmd, "ENVIAR_MENSAJE",  14) == 0) { char *r = leerPaquetes(sock); cmd_enviarMensaje(sock, r); free(r); }
        else if (strncmp(cmd, "OBTENER_MENSAJES",16) == 0) { cmd_obtenerMensajes(sock); }
        else if (strncmp(cmd, "BUSCAR_CLIENTE",  14) == 0) { cmd_buscarCliente(sock); }
        else if (strncmp(cmd, "LISTA",            5) == 0) { char *sep = strchr(cmd,'|'); if(sep) cmd_lista(sock, sep+1); }
        else if (strncmp(cmd, "LIMPIAR",          7) == 0) { printf("[Servidor] LIMPIAR\n"); }
        else if (strncmp(cmd, "SALIR",            5) == 0) { printf("[Servidor] Cliente desconectado\n"); break; }
        else { printf("[Servidor] Desconocido: '%s'\n", cmd); }
    }
    close(sock);
    return NULL;
}

int main( void ) {
    int sersock;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t cliLen = sizeof(cliaddr);
    int opt = 1;

    sersock = socket(AF_INET, SOCK_STREAM, 0);
    if (sersock < 0) { perror("socket"); return EXIT_FAILURE; }
    setsockopt(sersock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    seraddr.sin_family      = AF_INET;
    seraddr.sin_port        = htons(SER_PORT);
    seraddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sersock, (struct sockaddr*)&seraddr, sizeof(seraddr)) < 0) {
        perror("bind"); return EXIT_FAILURE;
    }
    listen(sersock, 10);
    printf("[Servidor] Escuchando en puerto %d...\n", SER_PORT);

    while (1) {
        int newsock = accept(sersock, (struct sockaddr*)&cliaddr, &cliLen);
        if (newsock < 0) { perror("accept"); continue; }
        printf("[Servidor] Conexion desde %s\n", inet_ntoa(cliaddr.sin_addr));
        ArgsHilo *args = malloc(sizeof(ArgsHilo));
        args->sock = newsock;
        pthread_t hilo;
        if (pthread_create(&hilo, NULL, atenderCliente, args) != 0) {
            perror("pthread_create"); free(args); close(newsock);
        } else {
            pthread_detach(hilo);
        }
    }
    close(sersock);
    return EXIT_SUCCESS;
}