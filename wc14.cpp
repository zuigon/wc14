#include <sys/socket.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <vector>

#include <string>
#include <sstream>


int GetIntVal(std::string strConvert) {
    int intReturn;
    intReturn = atoi(strConvert.c_str());
    return(intReturn);
}

struct klijent {
    int client_socket;
};

std::vector<klijent *> klijenti;

int main(int argc, char* argv[]){

    int port    = 8014;
    int quiet = 0;

    // char root = "/var/www/wc14_html";
    char root[] = "./html";

    int pokp=0;
    for(int i=1; i<argc; i++){
        std::string ar = argv[i];
        if(ar == "-q"){
            quiet=1;
            printf(">> -q\n");
        } else
        if(ar == "-p" && argv[i+1]){
            printf(">> -p\n");
            pokp = 1;
        } else
        if(pokp==1){
            port = GetIntVal(ar);
            printf(">> port: %d\n", port);
        }
    }


    int fail;

    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in sin = {0};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;
    fail = bind(s, (struct sockaddr *) &sin, sizeof(sin));
    if (fail){
        printf("Ne mogu koristiti port %d\n", port);
        return -1;
    }
    fail = listen(s, 10);
    if (fail){
        printf("Ne mogu slusati na portu %d\n", port);
        return -1;
    }

    printf("Pokrenut na 0.0.0.0:%d\n", port); fflush(stdout);


    fd_set sockets;
    unsigned int i;
    int max;

    while (1){
        FD_ZERO(&sockets);
        FD_SET(s, &sockets);
        max = s;
        for (i=0;i<klijenti.size();i++){
            struct klijent *k = klijenti[i];
            FD_SET(k->client_socket, &sockets);
            if (max < k->client_socket)
                max = k->client_socket;
        }
        if (select(max+1, &sockets, NULL, NULL, NULL)>0){
            for (i=0;i<klijenti.size();i++){
                struct klijent *k = klijenti[i];
                if (FD_ISSET(k->client_socket, &sockets)){
                    char buff[8192];
                    int total;
                    total = recv(k->client_socket, buff, sizeof(buff), 0);
                    if (total>0){
                        buff[total] = 0;
                        if (strstr(buff, "\r\n\r\n")){
                            char *a = strstr(buff, "\r\n");
                            if (a){
                                *a = 0;
                                printf("buff: %s\n", buff);
                                a = strchr(buff, ' ');
                                if (a){
                                    a++;
                                    char *b = strchr(a, ' ');
                                    if (b){
                                        *b = 0;
                                        if(quiet==0) printf("Zatrazen je %s\r\n", a);
                                        char outbuff[8192];
                                        if (!strcmp(a, "/"))
                                            a = "/index.html";

                                        sprintf(outbuff, "%s%s", root, a);
                                        FILE *stream = fopen(outbuff, "rb");
                                        if (stream){
                                            int velicina;

                                            fseek(stream, 0, SEEK_END);
                                            velicina = ftell(stream);
                                            fseek(stream, 0, SEEK_SET);

                                            sprintf(outbuff, "HTTP/1.1 200 OK\r\nServer: Neki moj\r\nContent-Length: %d\r\n\r\n", velicina);
                                            send(k->client_socket, outbuff, strlen(outbuff), 0);
                                            while (!feof(stream)){
                                                int i = fread(outbuff, 1, sizeof(outbuff), stream);
                                                if (i>0)
                                                    send(k->client_socket, outbuff, i, 0);
                                            }
                                            if(quiet==0)
                                                printf("gotov s citanjem\n");
                                            fclose(stream);
                                        }
                                        else {
                                            sprintf(outbuff, "HTTP/1.1 404 Not found\r\nServer: Neki moj\r\nContent-Length: 4\r\nNEMA");
                                            send(k->client_socket, outbuff, strlen(outbuff), 0);
                                        }
                                        close(k->client_socket);
                                        klijenti.erase(klijenti.begin() + i);
                                    }
                                }
                            }
                        }
                    }
                    else {
                        klijenti.erase(klijenti.begin() + i); i--;
                    }
                }
            }
            if (FD_ISSET(s, &sockets)){
                struct sockaddr_in client = {0};
                unsigned int client_len = sizeof(client);
                int novi_s = accept(s, (struct sockaddr *)&client, &client_len);
                klijent *novi_klijent = new klijent();
                novi_klijent->client_socket = novi_s;
                klijenti.push_back(novi_klijent);
            }
        }
    }

    return 0;
}

