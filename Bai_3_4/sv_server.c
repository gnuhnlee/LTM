#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    // 1. Kiem tra tham so (Chi co Cong va File Log)
    if (argc != 3) {
        printf("Su dung: %s <cong> <file_log.txt>\n", argv[0]);
        exit(1);
    }

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    int opt = 1; 
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) { 
        perror("setsockopt failed"); 
        exit(EXIT_FAILURE); 
    }

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    if (listen(listener, 5) < 0) {
        perror("listen() failed");
        exit(1);
    }

    printf("Server dang lang nghe o cong %s, log luu tai '%s'...\n", argv[1], argv[2]);
    
    // 2. Vong lap vo han de phuc vu nhieu sinh vien (nhieu client) lien tiep
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        
        // Block o day de cho client ket noi
        int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client < 0) {
            perror("accept() failed");
            continue; 
        }

        // Lay IP cua client
        char *client_ip = inet_ntoa(client_addr.sin_addr);

        // Lay thoi gian hien tai
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char time_str[26];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

        // Nhan du lieu tu client
        char buf[256];
        int n = recv(client, buf, sizeof(buf) - 1, 0);
        
        if (n > 0) {
            buf[n] = 0; // Ket thuc chuoi
            
            // In ra man hinh de test
            printf("%s %s %s\n", client_ip, time_str, buf);

            // Ghi vao file log (Mo, ghi, roi dong luon)
            FILE *f_log = fopen(argv[2], "a");
            if (f_log != NULL) {
                fprintf(f_log, "%s %s %s\n", client_ip, time_str, buf);
                fclose(f_log);
            } else {
                perror("Khong the mo file log");
            }
        }

        // Dong socket rieng cua client nay, vong len cho client moi
        close(client);
    }

    close(listener);
    return 0;
}