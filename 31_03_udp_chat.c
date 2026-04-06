#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Su dung: %s <port_s> <ip_d> <port_d>\n", argv[0]);
        printf("Vi du: %s 8000 127.0.0.1 9000\n", argv[0]);
        return 1;
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        perror("Loi tao socket");
        return 1;
    }

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(port_s);

    if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("Loi bind()");
        return 1;
    }
    printf("Ung dung dang lang nghe tren cong %d...\n", port_s);
    printf("Se gui tin nhan den %s:%d\n", ip_d, port_d);

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(ip_d);
    dest_addr.sin_port = htons(port_d);

    fd_set readfds;
    char buf[BUF_SIZE];

    while (1) {
        FD_ZERO(&readfds);
        
        FD_SET(STDIN_FILENO, &readfds);
        
        FD_SET(sock, &readfds);

        int activity = select(sock + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Loi select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (fgets(buf, BUF_SIZE, stdin) != NULL) {
                buf[strcspn(buf, "\n")] = 0; 
                
                if (strlen(buf) > 0) {
                    sendto(sock, buf, strlen(buf), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                }
            }
        }

        if (FD_ISSET(sock, &readfds)) {
            struct sockaddr_in sender_addr;
            socklen_t sender_len = sizeof(sender_addr);
            
            int bytes_read = recvfrom(sock, buf, BUF_SIZE - 1, 0, (struct sockaddr *)&sender_addr, &sender_len);
            if (bytes_read > 0) {
                buf[bytes_read] = '\0';
                printf("\r[Ban be]: %s\n", buf);
            }
        }
    }

    close(sock);
    return 0;
}