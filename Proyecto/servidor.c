#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs/zstring.h"
#include "libs/lista.h"
#include "libs/estructura.h"


int main() {

    int sersock, newsock;
    struct sockaddr_in seraddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    sersock = socket(AF_INET, SOCK_STREAM, 0);

    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(SER_PORT);
    seraddr.sin_addr.s_addr = INADDR_ANY;

    bind(sersock, (struct sockaddr*)&seraddr, sizeof(seraddr));
    listen(sersock, 5);

    printf("Servidor activo...\n");

    while(1) {
        newsock = accept(sersock, (struct sockaddr*)&cliaddr, &len);

        char buffer[1024];
        int n = read(newsock, buffer, sizeof(buffer)-1);

        if(n > 0) {
            buffer[n] = 0;
            printf("Mensaje: %s\n", buffer);
            write(newsock, "OK", 2);
        }

        close(newsock);
    }

    close(sersock);
    return 0;
}