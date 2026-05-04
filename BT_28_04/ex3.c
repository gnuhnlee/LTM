#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT 9000
#define MAX_TOPICS 10
#define MAX_NAME_LEN 64

// Cấu trúc lưu trữ thông tin của mỗi client
typedef struct {
    int fd;
    char topics[MAX_TOPICS][MAX_NAME_LEN];
    int num_topics;
} ClientInfo;

ClientInfo clients[FD_SETSIZE];

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) { perror("socket() failed"); return 1; }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed"); return 1;
    }

    if (listen(listener, 10)) {
        perror("listen() failed"); return 1;
    }

    printf("SUB/UNSUB/PUB Server is listening on port %d...\n", PORT);

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    // Khởi tạo mảng quản lý client
    for (int i = 0; i < FD_SETSIZE; i++) {
        clients[i].fd = -1;
        clients[i].num_topics = 0;
    }

    char buf[512];

    while (1) {
        fdtest = fdread;
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret < 0) break;

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &fdtest)) {
                if (i == listener) {
                    // Chấp nhận kết nối mới
                    int client = accept(listener, NULL, NULL);
                    if (client < FD_SETSIZE) {
                        FD_SET(client, &fdread);
                        clients[client].fd = client;
                        clients[client].num_topics = 0;
                        printf("Client %d connected.\n", client);
                        send(client, "Welcome to our SUB/UNSUB/PUB server!\n", 38, 0);
                    } else {
                        close(client);
                    }
                } else {
                    // Nhận dữ liệu từ client
                    int n = recv(i, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        printf("Client %d disconnected.\n", i);
                        FD_CLR(i, &fdread);
                        close(i);
                        clients[i].fd = -1;
                        clients[i].num_topics = 0;
                    } else {
                        buf[n] = 0;
                        printf("Received from %d: %s", i, buf);

                        char cmd[10], topic[MAX_NAME_LEN], msg[256];

                        // Xử lý lệnh SUB <topic>
                        if (sscanf(buf, "SUB %s", topic) == 1) {
                            if (clients[i].num_topics < MAX_TOPICS) {
                                strcpy(clients[i].topics[clients[i].num_topics++], topic);
                                send(i, "Subscribed successfully!\n", 25, 0);
                            } else {
                                send(i, "Topic limit reached!\n", 21, 0);
                            }
                        }
                        //Xu ly lenh UNSUB <topic>
                        else if(sscanf(buf, "UNSUB %s", topic) == 1){
                            int k;
                            for (k = 0; k < clients[i].num_topics; k++){
                                if(strcmp(clients[i].topics[k], topic) == 0){
                                    strcpy(clients[i].topics[k],"");
                                    send(i, "Unsubscribed successfully!\n", 28, 0);
                                    break;
                                }
                            }
                        }
                        // Xử lý lệnh PUB <topic> <msg>
                        else if (sscanf(buf, "PUB %s %[^\n]", topic, msg) == 2) {
                            char send_buf[512];
                            sprintf(send_buf, "[%s]: %s\n", topic, msg);

                            // Chuyển tiếp cho các client đã SUB topic này
                            for (int j = 0; j < FD_SETSIZE; j++) {
                                if (clients[j].fd != -1) {
                                    for (int k = 0; k < clients[j].num_topics; k++) {
                                        if (strcmp(clients[j].topics[k], topic) == 0) {
                                            send(clients[j].fd, send_buf, strlen(send_buf), 0);
                                            break; 
                                        }
                                    }
                                }
                            }
                        } else {
                            send(i, "Invalid Command! Use SUB <topic> or PUB <topic> <msg>\n", 55, 0);
                        }
                    }
                }
            }
        }
    }

    close(listener);
    return 0;
}