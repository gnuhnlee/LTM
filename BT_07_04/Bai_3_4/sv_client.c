#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // 1. Kiem tra tham so
    if (argc != 3) {
        printf("Su dung: %s <dia chi IP> <cong>\n", argv[0]);
        return 1;
    }

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr; 
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr)); 
    if (ret < 0) {
        perror("connect() failed");
        return 1;
    }

    // Cac bien luu thong tin
    char mssv[20], name[50], dob[20];
    float gpa;
    char buffer[256]; // Chi giu lai 1 bien buffer nay

    printf("Nhap thong tin sinh vien:\n");
    
    printf("- MSSV: ");
    scanf("%s", mssv);
    getchar(); 

    printf("- Ho ten: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("- Ngay sinh (YYYY-MM-DD): ");
    scanf("%s", dob);

    printf("- Diem TB: ");
    scanf("%f", &gpa);

    sprintf(buffer, "%s %s %s %.2f", mssv, name, dob, gpa);

    send(client, buffer, strlen(buffer), 0);
    printf("Da gui: %s\n", buffer);

    close(client);
    return 0;
}