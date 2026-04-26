#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "zstring.h"
#include "scanner.h"
#include "lista.h"
#include "estructura.h"

int clisock;
struct sockaddr_in cliaddr;

void getOpts( int, int, char* argv[], char* opt[] );
void analizar( unsigned int, char* );
void parsear( char );
int esValidaIpV4( const char* );
//void crearCliente( void );
//int sintaxisCrearCliente( InfoCliente** );

int confIni( int flgMosMsj ) {
	int r = false;
	struct lexico lex = Token(1);

 	if( lex.atrib == E_parA ) {
		r = true;
	} 
	else  {
		if( true == flgMosMsj )
			mensaje_error( "Se esperaba un paréntesis abierto" );
	}
	
	return r;
}

int confFin( int flgMosMsj ) {
	int r = false;
	struct lexico lex = Token(1);

	if( lex.atrib == E_parC ) {
		lex = Token(1);

		if( lex.atrib == E_puncoma ) {
			r = true;
		}
		else {
			if( true == flgMosMsj )
				mensaje_error( "Se esperaba punto y coma" );
		}
	} 
	else {
		if( true == flgMosMsj ) 
			mensaje_error( "Se esperaba paréntesis de cierre" );
	}
	
	return r;
}

int confVacia( int flgMosMsj ) {
	int r = false;
	//struct lexico lex = Token(1);

	if( estado == E_puncoma ) {
		r = true;
	}
	else {
		if( true == flgMosMsj ) 
			mensaje_error( "Se esperaba punto y coma" );
	}
	
	return r;
}

int sintaxisVacia( void ){
	int r = confIni( false );
	if( false == r )
		return confVacia( true );
	
	return confFin( false );
}

int sintaxisEnviarMensaje( InfoMensaje **inf ) {
	int r = false;
	int long unsigned idOrigen 	= -1;
	int long unsigned idDestino	= -1;
	char *mensaje 				= zstr( (char*)"" );
	
	r = confIni( true );  				if( false == r ) return r;
	r = parIntLargo( false, &idOrigen );if( false == r ) return r;
	r = parIntLargo( true, &idDestino );if( false == r ) return r;
	r = parStr( true, &mensaje, " " );	if( false == r ) return r;
	r = confFin( true );
	
	if( true == r ) {
		(*inf) -> numlinea 		= numlinea;
		(*inf) -> idOrigen 		= idOrigen;
		(*inf) -> idDestino 	= idDestino;
		(*inf) -> mensaje 		= mensaje;
		(*inf) -> estado 		= ENVIADO;
	}
	return r;
}

int sintaxisBuscarCliente( InfoBuscarCliente **inf ) {
	int r = false;
	int long unsigned clave = 0;

	r = confIni( true );  				if( false == r ) return r;
	r = parIntLargo( false, &clave );	if( false == r ) return r;
	r = confFin( true );
	
	if( true == r ) {
		(*inf) -> numlinea 		= numlinea;
		(*inf) -> cui 			= clave;
		(*inf) -> id 			= clave;
	}
	return r;
}

int sintaxisCrearCliente( InfoCliente **inf ) {

	int r = false;
	char strIni[] = { "" };

	int long unsigned cui 		= 0;
	int long unsigned movil 	= 0;
	int long unsigned telefono	= 0;

	char *nombre 	= zstr( strIni );
	char *apellidos = zstr( strIni );
	char *domicilio = zstr( strIni );
	char *correoe 	= zstr( strIni );

 	r = confIni( true );  				if( false == r ) return r;
	r = parStr( false, &nombre, " " ); 	if( false == r ) return r;
	r = parStr( true, &apellidos, " " );if( false == r ) return r;
	r = parIntLargo( true, &cui );		if( false == r ) return r;
	r = parStr( true, &domicilio, " " );if( false == r ) return r;
	r = parIntLargo( true, &movil );	if( false == r ) return r; 
	r = parIntLargo( true, &telefono );	if( false == r ) return r;
	r = parStr( true, &correoe, "" ); 	if( false == r ) return r;
	r = confFin( true );

	//printf( "\n\t\tval:%s\n", domicilio ); 
	
	if( true == r ) {
		(*inf) -> numlinea 		= numlinea;
		(*inf) -> cui 			= cui;
		(*inf) -> nombre 		= nombre;
		(*inf) -> apellidos		= apellidos;
		(*inf) -> domicilio		= domicilio;
		(*inf) -> movil			= movil;
		(*inf) -> telefono		= telefono;
		(*inf) -> correoe		= correoe;
		
		r = true;
	}

	return r;
}

int sintaxisLista( char **str ){
	int r = false;
	char *lista = zstr( (char*)"" );

	r = confIni( true );  				if( false == r ) return r;
	r = parId( false, &lista ); 		if( false == r ) return r;
	r = confFin( true );

	if( true == r ) {
		*str = zstr( lista );
	}
	
	return r;
}

int sintaxisObtenerMensajes( int long unsigned *idCliente ){
	int r = false;
	int long unsigned id = -1;

	r = confIni( true ); 			if( false == r ) return r;
	r = parIntLargo( false, &id );	if( false == r ) return r;
	r = confFin( true );

	if( true == r )
		*idCliente = id;

	return r;
}

void enviarMensaje( void ) {
	int res = -1;
	InfoMensaje *info = (InfoMensaje*)malloc( sizeof( InfoMensaje ) );
	
	if ( true == sintaxisEnviarMensaje( &info ) ) {

		char buffer[ BUFFER_LEN ] = "";

    	char *str = serializarInfoMensaje( info );
    	size_t len = strlen(str);
    	int pack = calcPack( len );
		char msj[ 100 ] = { '\0' };

		strcpy( buffer, "ENVIAR_MENSAJE" );
		write( clisock, buffer, sizeof( buffer ) );

		write( clisock, &len, sizeof( size_t ) );

    	for( int i = 0; i < pack; i++ ) {
        	int ini = (i==0) ? 0 : i * BUFFER_LEN - i;
        	strcpy( buffer, "" );
        	strcpy( buffer, zsubstr( str, ini, BUFFER_LEN - 1 ) );
        	write( clisock, buffer, sizeof( buffer ) );
    	}

		listen( clisock, 1 );
        read( clisock, &res, sizeof( res ) );

		if( res == -1 ) {
			sprintf( msj, "Usuario con id origen [%lu] no fue encontrado", info -> idOrigen );
			mensajeConLineaErr( "Imposible enviar mensaje" );
			mensaje_error( msj );
			printf( "\n" );
		}
		else if( res == -2 ) {
			sprintf( msj, "Usuario con id destino [%lu] no fue encontrado", info -> idDestino );
			mensajeConLineaErr( "Imposible enviar mensaje" );
			mensaje_error( msj );
			printf( "\n" );
		}
		else {
			sprintf( msj, "Mensaje enviado a servidor. Origen: [%lu], destino: [%lu]", info -> idOrigen, info -> idDestino );
			mensajeConLinea( msj );
		}
	}
	else {
 		mensajeConLineaErr( "Los parámetros son incorrectos para ENVIAR_MENSAJE" );
		recuperarse();
	}
}

void buscarCliente( void ) {
	int resp = -1;
	InfoBuscarCliente *info = (InfoBuscarCliente*)malloc( sizeof( InfoBuscarCliente ) );
	
	if ( true == sintaxisBuscarCliente( &info ) ) {

		char buffer[ BUFFER_LEN ] = "";
		strcpy( buffer, "BUSCAR_CLIENTE" );

		write( clisock, buffer, sizeof( buffer ) );

		write( clisock, info, sizeof( InfoBuscarCliente ) );

		listen( clisock, 1 );
		read( clisock, &resp, sizeof( resp ) );
		
		char msj[ 100 ] = { '\0' }; 
		if( -1 ==  resp ) {
			sprintf( msj, "No. de DPI '%lu' no fue encontrado", info -> cui );
			mensajeConLineaErr( "Resultado de búsqueda" );
			mensaje_error( msj );
			printf( "\n" );
		} else if( resp > 0 ) {

			size_t len = -1;
			listen( clisock, 1 );
            read( clisock, &len, sizeof( len ) );
            int pack = calcPack( len );

            char *nwStr = (char*)malloc(1); nwStr[ 0 ] = '\0';
            for( int i = 0; i < pack; i++ ) {
                listen( clisock, 1 );
                read( clisock, buffer, BUFFER_LEN );
                nwStr = zstr( nwStr, (char*)buffer );
            }
			//printf( "nwstr: %s\n", nwStr );
            char *strNum = zstring_strtok( nwStr, "#" );
			char *strId  = zstring_strtok( NULL, "#" );
            char *strCui = zstring_strtok( NULL, "#" );
            char *strNom = zstring_strtok( NULL, "#" );
            char *strApe = zstring_strtok( NULL, "#" );
            char *strDom = zstring_strtok( NULL, "#" );
            char *strMov = zstring_strtok( NULL, "#" );
            char *strTel = zstring_strtok( NULL, "#" );
            char *strCor = zstring_strtok( NULL, "#" );

            InfoCliente *infCli = (InfoCliente*) malloc( sizeof( InfoCliente ) );

            char rep[] = "\\#";
            char with[] = "#";

            strDom = zstr_replace( strDom, rep, with );

            infCli -> numlinea = atoi( strNum );
			infCli -> id = atoi( strId );
            infCli -> cui = atoll( strCui );
            infCli -> nombre = strNom;
            infCli -> apellidos = strApe;
            infCli -> domicilio = zstr( strDom );
            infCli -> movil = atoll( strMov );
            infCli -> telefono = atoll( strTel );
            infCli -> correoe = strCor;

			mensajeConLinea( "Cliente encontrado" );
			GblflgImpLin = false;
			imprimirInfoCliente( infCli );
			GblflgImpLin = true;
			printf( "\n" );
		}

	} 
	else {
 		mensajeConLineaErr( "Los parámetros son incorrectos para BUSCAR_CLIENTE" );
		recuperarse();
	}
}

void crearCliente( void ) {

	InfoCliente *info = NULL;
	info = (InfoCliente*)malloc( sizeof( InfoCliente ) );
	
	int r = sintaxisCrearCliente( &info );

 	if ( true == r ) {

		int idCliente = -1;
		char buffer[ BUFFER_LEN ] = "";

    	char *str = serializarInfoCliente( info );
    	size_t len = strlen(str);

    	int pack = calcPack( len );

		strcpy( buffer, "CREAR_CLIENTE" );
		write( clisock, buffer, sizeof( buffer ) );

    	//printf( "\n >serializado { len: %lu, pack: %d, str: '%s' }\n", len, pack, str );

		write( clisock, &len, sizeof( size_t ) );

    	for( int i = 0; i < pack; i++ ) {
        	int ini = (i==0) ? 0 : i * BUFFER_LEN - i;
        	strcpy( buffer, "" );
        	strcpy( buffer, zsubstr( str, ini, BUFFER_LEN - 1 ) );
        	write( clisock, buffer, sizeof( buffer ) );
    	}

		listen( clisock, 1 );
		read( clisock, &idCliente, sizeof(idCliente) );
		
		char msj[ 100 ] = { '\0' } ; 
		if( -1 ==  idCliente ) {
			sprintf( msj, "No. de DPI '%lu' repetido, no será tomada en cuenta la instrucción", info -> cui );
			mensajeConLineaErr( "Error en instrucción" );
			mensaje_error( msj );
			printf( "\n" );
		} else if( idCliente > 0 ) {
			sprintf( msj, "Cliente '%lu' creado con identificador [ %d ]", info -> cui, idCliente );
			mensajeConLinea( msj );
		}

 	} else {
 		mensajeConLineaErr( "Los parámetros son incorrectos para CREAR_CLIENTE" );
		recuperarse();
	}
}

/* ---- ESTRESAR ---- */
typedef struct {
    unsigned long int idOrigen;
    unsigned long int idDestino;
    char             *mensaje;
    char             *ip;
    int               numero;
} ArgsEstresar;

static void *hiloEstresar( void *arg ) {
    ArgsEstresar *a = (ArgsEstresar*)arg;
    int sock;
    struct sockaddr_in addr;
    addr.sin_port        = htons(SER_PORT);
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(a->ip);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if ( sock < 0 || connect(sock,(struct sockaddr*)&addr,sizeof(addr)) < 0 ) {
        printf("[Estres] Hilo %d: no pudo conectarse\n", a->numero);
        free(a);
        return NULL;
    }
    InfoMensaje info;
    info.numlinea  = 0;
    info.idOrigen  = a->idOrigen;
    info.idDestino = a->idDestino;
    info.mensaje   = a->mensaje;
    info.estado    = ENVIADO;
    char buffer[BUFFER_LEN] = "";
    char *str = serializarInfoMensaje(&info);
    size_t len = strlen(str);
    int pack   = calcPack(len);
    strcpy(buffer, "ENVIAR_MENSAJE");
    write(sock, buffer, sizeof(buffer));
    write(sock, &len, sizeof(size_t));
    for ( int i = 0; i < pack; i++ ) {
        int ini = (i==0) ? 0 : i*(BUFFER_LEN-1);
        memset(buffer, 0, BUFFER_LEN);
        strncpy(buffer, str+ini, BUFFER_LEN-1);
        write(sock, buffer, sizeof(buffer));
    }
    int res = -99;
    read(sock, &res, sizeof(res));
    printf("[Estres] Hilo %d: respuesta=%d\n", a->numero, res);
    strcpy(buffer, "SALIR");
    write(sock, buffer, sizeof(buffer));
    close(sock);
    free(a);
    return NULL;
}

int sintaxisEstresar( unsigned int *nPet,
                      unsigned long int *idO,
                      unsigned long int *idD,
                      char **msg ) {
    int r = false;
    unsigned long int tmp = 0;
    char *m = zstr((char*)"");
    r = confIni(true);              if(!r) return r;
    r = parIntLargo(false, &tmp);   if(!r) return r;  *nPet = (unsigned)tmp;
    r = parIntLargo(true,  idO);    if(!r) return r;
    r = parIntLargo(true,  idD);    if(!r) return r;
    r = parStr(true, &m, "");       if(!r) return r;
    r = confFin(true);
    if(r) *msg = m;
    return r;
}

void estresar( void ) {
    unsigned int      nPet = 0;
    unsigned long int idO  = 0, idD = 0;
    char             *msg  = NULL;
    if ( !sintaxisEstresar(&nPet, &idO, &idD, &msg) ) {
        mensajeConLineaErr("Parametros incorrectos para ESTRESAR");
        recuperarse();
        return;
    }
    char msj[100];
    sprintf(msj, "Enviando %u peticiones concurrentes...", nPet);
    mensajeConLinea(msj);
    pthread_t *hilos = malloc(nPet * sizeof(pthread_t));
    for ( unsigned int i = 0; i < nPet; i++ ) {
        ArgsEstresar *args = malloc(sizeof(ArgsEstresar));
        args->idOrigen  = idO;
        args->idDestino = idD;
        args->mensaje   = msg;
        args->numero    = i + 1;
        args->ip        = zstr((char*)"127.0.0.1");
        if ( pthread_create(&hilos[i], NULL, hiloEstresar, args) != 0 ) {
            perror("pthread_create");
            free(args);
            hilos[i] = 0;
        }
    }
    for ( unsigned int i = 0; i < nPet; i++ )
        if (hilos[i]) pthread_join(hilos[i], NULL);
    free(hilos);
    sprintf(msj, "Estres completado: %u mensajes enviados", nPet);
    mensajeConLinea(msj);
}
/* ---- FIN ESTRESAR ---- */

void lista( void ) {
	char buffer[ BUFFER_LEN ] = "";
	char *idLista = zstr( (char*)"" );

 	if ( true == sintaxisLista( &idLista ) ) {
		
		strcpy( buffer, "LISTA|" ); strcat( buffer, idLista );
		write( clisock, buffer, sizeof( buffer ) );
		mensajeConLinea( zstr( (char*)"Lista '", zstrupr( idLista ), (char*)"' generada" ) );
		//printf( "idLista: '%s'\n", idLista );

 	} else {
 		mensajeConLineaErr( "Los parámetros son incorrectos para LISTA" );
		recuperarse();
	}
}

void limpiar( void ) {
	char buffer[ BUFFER_LEN ] = "";

 	if ( true == sintaxisVacia() ) {
		
		strcpy( buffer, "LIMPIAR" ); 
		write( clisock, buffer, sizeof( buffer ) );

		mensajeConLinea( "Limpiar pantalla" );

 	} else {
 		mensajeConLinea( "Los parámetros son incorrectos para LIMPIAR\n" );
		recuperarse();
	}
}

void obtenerMensajes(void){
	int res = false;
	char buffer[ BUFFER_LEN ] = "";
	unsigned int long idCliente;
	unsigned int rSer = false;

	res = sintaxisObtenerMensajes( &idCliente );

 	if ( true == res ) {
		
		strcpy( buffer, "OBTENER_MENSAJES" );
		write( clisock, buffer, sizeof( buffer ) );
		write( clisock, &idCliente, sizeof( idCliente ) );

		listen( clisock, 1 );
        read( clisock, &rSer, sizeof( rSer ) );

		if( false == rSer ) {
			mensajeConLineaErr( "No hay mensajes para este usuario" );
		}
		else {

			mensajeConLinea( "Mensaje/s recibido/s" );

			for( unsigned int j = 1; j <= rSer; j++ ) {

				size_t len = -1;
				listen( clisock, 1 );
				read( clisock, &len, sizeof( len ) );
				int pack = calcPack( len );

				char *nwStr = (char*)malloc(1); nwStr[ 0 ] = '\0';
				for( int i = 0; i < pack; i++ ) {
					listen( clisock, 1 );
					read( clisock, buffer, BUFFER_LEN );
					nwStr = zstr( nwStr, (char*)buffer );
				}
				//printf( "nwstr: %s\n", nwStr );
				
				char *strNum = zstring_strtok( nwStr, "#" );
				char *strIdO  = zstring_strtok( NULL, "#" );
				char *strIdD = zstring_strtok( NULL, "#" );
				char *strMsj = zstring_strtok( NULL, "#" );
				char *strEst = zstring_strtok( NULL, "#" );
				
				InfoMensaje *infMsj = (InfoMensaje*) malloc( sizeof( InfoMensaje ) );

				char rep[] = "\\#";
				char with[] = "#";

				strMsj = zstr_replace( strMsj, rep, with );

				infMsj -> numlinea = atoi( strNum );
				infMsj -> idOrigen = atoll( strIdO );
				infMsj -> idDestino = atoll( strIdD );
				infMsj -> mensaje = strMsj;
				infMsj -> estado = atoi( strEst );
				
				GblflgImpLin = false;
				imprimirInfoMensaje( infMsj );
				GblflgImpLin = true;
			}
			printf( "\n" );
		}
		

 	} else {
 		mensajeConLineaErr( "Los parámetros son incorrectos para ENVIAR_MENSAJES" );
		recuperarse();
	}
}

int esValidaIpV4( const char* buffer ) {
    if (NULL == buffer) return false;

    register const      char*   pos     = buffer;
    register unsigned   char    ch      = *pos;
    register unsigned   short   count   = 0;

    if( 0 == strcasecmp( buffer, "localhost" ) ) {
        return true;
	}

    while( ch != '\0' ) {
        if (!((ch >= '0' && ch <= '9') || ch == '.')) return false;

        if (ch == '.')
            if (++count > 3) return false;

        ch = *++pos;
    }

    if (count == 3 && *--pos != '.') return true;

    return false;
}

void initSocket( char *ip ) {

	if( NULL == ip )
		ip = zstr( (char*)"127.0.0.1" );

	cliaddr.sin_port 		= htons(SER_PORT);
    cliaddr.sin_family		= AF_INET;
    cliaddr.sin_addr.s_addr	= inet_addr( ip );

    clisock = socket( AF_INET, SOCK_STREAM, 0 );
    
	if( clisock < 0 ) {
        perror( "\n SOCKET" );
		exit(EXIT_FAILURE);
    }

    if( connect( clisock,(struct sockaddr*)&cliaddr,sizeof(cliaddr)) < 0 ) {
        perror( "\n CONEXIÓN" );
        exit(EXIT_FAILURE);
    }

    printf( "\033[1;35m Cliente conectado a [ %s ]\n", ip );
	printf( "\033[0m" );
}

void initSesion( char *ip ) {
	//const char *strIni = "\n=== Ingrese el archivo: ";
	//const char *strIni = zstr( (char*)"cliente@", ip, (char*)"> " );
	//printf( "\033[1;31m" ); 
	//printf( " %s", strIni );
	//printf( "\033[0m" );
	const char *strIni = zstr( (char*)"cliente" ) ;
	printf( "\033[1;32m %s", strIni ); 
	printf( "\033[0m@" );
	printf( "\033[01;34m%s", ip ); 
	printf( "\033[0m> " );
}

void parsear( char cfin ) {
	int len		= ( linea != NULL ) ? strlen( linea ) : -1;
	ilinea 		= -1;
	numlinea 	= 1;
	token 		= '\0';
	
	//printf( "linea:'%s'\n", linea );
	while( token != cfin ) { //while( !Fin ) {
		//printf( "Fin: %d\n", Fin );
		lexico lex = Token(1);
		//printf( "token:'%c', token(int):%d, i:%d, len:%lu\n", token, (int)token, ilinea, strlen(linea) );
		//printf( "\n\tlexema: %s\n", lexema );
		if( ilinea >= len )
			break;

		switch( lex.atrib ) {
			case 50: crearCliente();
					 break;

			case 51: enviarMensaje();
					 break;

			case 52: estresar();
					 break;

			case 53: obtenerMensajes();
					 break;

			case 54: lista();
					 break;

			case 55: limpiar();
					 break;

			case 56: buscarCliente();
					 break;

			default:
				printf( " Comando desconocido: { %s }\n", lex.C_lex );
				recuperarse();
				break;
		};
	}
}

void getOpts( int ini, int argc, char* argv[], char* opt[] ) {
	int i;
	int flag = false;

	opt[ 0 ] = opt[ 1 ] = NULL;

	for( i = ini; i < argc; i++ ) {
        char const *opcion = argv[ i ];

		if( strcmp( opcion, "-f" ) == 0 ) {
            if( i + 1 < argc )
                opt[ 0 ] = zstr( argv[ i + 1 ] );
        } else {
			if( 0 == strcasecmp( opcion, "localhost" ) )
				opt[ 1 ] = zstr( (char*)"127.0.0.1" );
			else {
				if( i == 0 || strcmp( argv[ i - 1 ], "-f" ) != 0 ) {
					flag = esValidaIpV4( opcion );
					if( true == flag ) {
						opt[ 1 ] = zstr( (char*)opcion );
					} else 
						opt[ 1 ] = NULL;
				}
			}
        }
    }
}

void analizar( unsigned int flg, char *cmd ) {
	
	char *opts[ 2 ];
	int argc = 0, j, errno = 0;
	char delim[] = " ";
	Nodo *ptr_ini = NULL, *ptr_fin = NULL;
	char *tmpCmd = zstr( cmd );

	char *ptr = strtok( cmd, delim );
	while( ptr != NULL ) {
		argc++;
		insertarAlFinal( &ptr_ini, &ptr_fin, ptr );
		ptr = strtok( NULL, delim );
	}

	char* args[ argc ];
	for( j = 0; j < argc; j++ )
		args[ j ] = (char*)removerAlFrente( &ptr_ini, &ptr_fin, &errno );
	
	getOpts( 0, argc, args, opts );

	if( true == flg )
		opts[ 0 ] = tmpCmd;

	if( opts[ 0 ] != NULL ) {
		if( true == abrir_archivo( opts[ 0 ] ) ) {
			parsear( EOF );
			cerrar_archivo();
		} else
			printf( " > Archivo '%s' no existe\n", opts[ 0 ] );

	} 
	else {
		linea = zstr( tmpCmd );
		parsear( '\n' );
	}
}

int
main( int argc, char* args[] ) {
	// ./cliente 127.0.0.1 -f entrada0 
	// ./cliente -f entrada0 127.0.0.1
	// ./cliente localhost
	// ./cliente localhost -f entrada0

	char* opts[ 2 ];
	char *cmd;
	int primeraVuelta = true;
	
	getOpts( 1, argc, args, opts );
	
	initSocket( opts[ 1 ] );

	//printf( "opts[0] = '%s'\n", opts[ 0 ] );
	//printf( "opts[1] = '%s'\n", opts[ 1 ] );
	
	for(;;) {

		if( true == primeraVuelta ) {

			primeraVuelta = false;

			if( opts[ 0 ] != NULL ) {
				analizar( 1, opts[ 0 ] );
				continue;
			} else {
				initSesion( opts[ 1 ] );
    			cmd = zscanf();
			}

		} else {
			initSesion( opts[ 1 ] );
    		cmd = zscanf();
		}

    	if( !strcasecmp( cmd, "salir" ) ) {
			char buffer[ BUFFER_LEN ] = "SALIR";
			write( clisock, buffer, sizeof( buffer ) );
    		break;
		}

		if( cmd != NULL && strlen( cmd ) > 0 )
			analizar( 0, cmd );
	}

	close( clisock );

	return EXIT_SUCCESS;
}