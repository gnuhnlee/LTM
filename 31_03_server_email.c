#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    int opt = 1; 
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }

    int ret = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    ret = listen(listener, 5);
    if (ret < 0) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for client...\n");
    
    int client_sockets[30]; 
    for (int i = 0; i < 30; i++) {
        client_sockets[i] = 0; 
    }
    fd_set readfds; 

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        int max_sd = listener;

        for (int i = 0; i < 30; i++) {
            int sd = client_sockets[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            printf("Lỗi select()\n");
        }

        if (FD_ISSET(listener, &readfds)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int new_socket = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
            
            if (new_socket >= 0) {
                printf("Client moi ket noi: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                
                char *greeting = "Ho ten va MSSV (VD: Le Hong Nhung 20235393):\n";
                send(new_socket, greeting, strlen(greeting), 0);

                for (int i = 0; i < 30; i++) {
                    if (client_sockets[i] == 0) {
                        client_sockets[i] = new_socket;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < 30; i++) {
            int sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) {
                char buf[512];
                int valread = recv(sd, buf, sizeof(buf) - 1, 0);

                if (valread == 0) {
                    printf("Client %d ngat ket noi.\n", sd);
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    buf[valread] = '\0';
                    
                    char *tokens[20];
                    int count = 0;
                    
                    char *token = strtok(buf, " \r\n");
                    while (token != NULL) {
                        tokens[count++] = token;
                        token = strtok(NULL, " \r\n");
                    }

                    if (count >= 2) {
                        char email[256] = "";
                        char *mssv = tokens[count - 1];
                        char *ten = tokens[count - 2];
                        
                        for(int j = 0; ten[j]; j++){
                            ten[j] = tolower(ten[j]);
                        }
                        strcat(email, ten);
                        strcat(email, ".");
                        
                        for (int k = 0; k < count - 2; k++) {
                            char initial[2] = {tolower(tokens[k][0]), '\0'};
                            strcat(email, initial);
                        }
                        
                        strcat(email, mssv);
                        strcat(email, "@sis.hust.edu.vn\n");

                        send(sd, email, strlen(email), 0);
                    } else {
                        char *err = "Sai dinh dang. Vui long nhap lai!\n";
                        send(sd, err, strlen(err), 0);
                    }
                }
            }
        }
    }

    return 0;
}