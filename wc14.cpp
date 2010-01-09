#include <sys/socket.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <vector>

struct klijent {
  int client_socket;
};

std::vector<klijent *> klijenti;

int main(void){
  int fail;

  int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  struct sockaddr_in sin = {0};
  sin.sin_family = AF_INET;
  sin.sin_port = htons(8011);
  sin.sin_addr.s_addr = INADDR_ANY;
  fail = bind(s, (struct sockaddr *) &sin, sizeof(sin));
  if (fail){
    printf("Ne mogu koristiti port ..\n");
    return -1;
  }
  fail = listen(s, 10);
  if (fail){
    printf("Ne mogu slusati na portu ..\n");
    return -1;
  }


  fd_set sockets;
  unsigned int i;
  int max;

  while (1){
    FD_ZERO(&sockets);     FD_SET(s, &sockets);
    max = s;
    for (i=0;i<klijenti.size();i++){
      struct klijent *k = klijenti[i];        FD_SET(k->client_socket, &sockets);
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
                a = strchr(buff, ' ');
                if (a){
                  a++;
                  char *b = strchr(a, ' ');
                  if (b)
                  if (b){
                    *b = 0;
                    printf("Zatrazen je %s\r\n", a);
                    char outbuff[8192];
                    if (!strcmp(a, "/"))
                      a = "/index.html";
                    sprintf(outbuff, "/var/www/wc14_html%s", a);
                    FILE *stream = fopen(outbuff, "rb");
                    if (stream){
                      fseek(stream, 0 , SEEK_END);
                      int velicina = ftell(stream);
                      fseek(stream, 0, SEEK_SET);

                      sprintf(outbuff, "HTTP/1.1 200 OK\r\nServer: Neki moj\r\nContent-Length: %d\r\n\r\n", velicina);
                      send(k->client_socket, outbuff, strlen(outbuff), 0);
                      while (!feof(stream)){
                        int i = fread(outbuff, 1, sizeof(outbuff), stream);
                        if (i>0){
                          send(k->client_socket, outbuff, i, 0);
                        }
                      }
                      printf("gotov s citanjem");
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

