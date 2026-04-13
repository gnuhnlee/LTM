#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    int opt = 1; 
    // Thiết lập SO_REUSEADDR để giải phóng cổng ngay khi server tắt 
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }

    int ret = bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind() failed");
        exit(1);
    }

    ret = listen(listener, 5);
    if (ret < 0) {
        perror("listen() failed");
        exit(1);
    }

    printf("Waiting for client\n");
    
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client < 0) {
        perror("accept() failed");
        exit(1);
    }

    printf("Client connected: %d\n", client);
    printf("IP: %s\n", inet_ntoa(client_addr.sin_addr));
    printf("Port: %d\n", ntohs(client_addr.sin_port));

    FILE *f_chao = fopen(argv[2], "r");
        if (f_chao != NULL) {
            char dong_chao[256];
            // Doc tung dong trong tep cho den het (EOF)
            while (fgets(dong_chao, sizeof(dong_chao), f_chao) != NULL) {
                send(client, dong_chao, strlen(dong_chao), 0);
            }
            fclose(f_chao);
        } else {
            printf("Canh bao: Khong the mo tep chao '%s'\n", argv[2]);
            char default_msg[] = "Server xin chao!\n";
            send(client, default_msg, strlen(default_msg), 0);
        }

    FILE *f_luu = fopen(argv[3], "a"); // Che do "a" (append)
        if (f_luu == NULL) {
            printf("Loi: Khong the mo tep luu '%s'\n", argv[3]);
        }

    char buf[256];
    while (1) {
        int n = recv(client, buf, sizeof(buf) - 1, 0); 
        
        if (n <= 0) {
            printf("Disconnected\n");
            break;
        }

        buf[n] = 0; // Them ky tu ket thuc xau
        printf("%d bytes received: %s", n, buf);
        
        // Ghi noi dung client gui vao file
        if (f_luu != NULL) {
            fputs(buf, f_luu);
            fflush(f_luu);
        }
    }

    // Dong file sau khi client ngat ket noi
    if (f_luu != NULL) {
        fclose(f_luu);
    }

    close(client);
    close(listener);
}