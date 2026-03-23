#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // tạo socket với IPv4
    struct sockaddr_in addr; // cấu hình CTDL chứa địa chỉ của server và client
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr)); // bắt tay 3 bước
    if (ret < 0) {
        perror("connect() failed");
        return 1;
    }
    char buffer[1024];
    while(1) {
        fgets(buffer, sizeof(buffer), stdin);
        send(client, buffer, strlen(buffer), 0);
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }
    }
    close(client); // đóng kết nối
    printf("Da dong ket noi.\n");
    return 0;
}