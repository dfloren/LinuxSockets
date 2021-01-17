#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <vector>
#include <sys/time.h>

#include "server.h"
#include "util.h"

using std::vector;

int main(int argc, char const *argv[]) {
    if(argc != ARGC){
        printf("Usage: %s port\n", argv[0]);
        return 0;
    }

    int host_port = atoi(argv[1]);

    int fd = socket(AF_INET, // an Internet socket
                    SOCK_STREAM, // reliable stream-like socket
                    0); // OS determine the protocol (TCP)

    int option = 1; // so socket can be reused immediately. Avoid TIME_WAIT state
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    if (fd < 0)
        return 1;

    struct sockaddr_in serv_addr; 

    // setup server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(host_port);

    // bind socket to the server address
    if (bind(fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Failed to establish socket");
        return 1;
    }

    printf("Using port %d\n", host_port);
    if (listen(fd, SOMAXCONN) < 0)
        return 1;

    struct sockaddr_in client_address;
    int len = sizeof(client_address);

    int tx_count = 0;

    // server timeout and timekeeping
    struct timeval timeout, start;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    start.tv_sec = 0;
    start.tv_usec = 0;

    vector<clientInfo> clientList;

    fd_set socket_set, copy_set;
    FD_ZERO(&socket_set);

    // add listener to original set
    FD_SET(fd, &socket_set);

    while(1) {
        copy_set = socket_set;
        int socketCount = select(FD_SETSIZE, &copy_set, NULL, NULL, &timeout);
        if(socketCount == 0){
            printf("Program exited due to %d seconds of inactivity.\n", TIMEOUT);
            break;
        }

        // Linux implementation of select() modifies &timeout. Must reset timer.
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;

        char buf[MAX_MSG_LENGTH];
        for(int sock=0; sock < FD_SETSIZE; sock++){
            if(FD_ISSET(sock, &copy_set)){
                if(sock == fd){ // pending connection request
                    int client_fd = accept(fd, (struct sockaddr*) &client_address, (socklen_t*) &len);
                    FD_SET(client_fd, &socket_set);
                }
                else{ // pending message
                    int read_bytes = recv(sock, buf, MAX_MSG_LENGTH, 0);
                    if(read_bytes > 0)
                        buf[read_bytes] = '\0';

                    if(strcmp(buf, "quit") == 0){
                        FD_CLR(sock, &socket_set);
                        close(sock);
                        continue;
                    }

                    if(tx_count < 1){ // start the clock. Inefficient check if compiler not smart
                        gettimeofday(&start, NULL);
                    }

                    // msg format: "machinename.pid:T<int>"
                    char* msg = strtok(buf, ":");
                    while(msg[0] != 'T'){
                        msg = strtok(NULL, "");
                    }

                    // simulate transaction
                    tx_count++;
                    printf("%.2f: # %d (T %d) from %s\n", getEpochS(), tx_count, atoi(msg+1), buf);
                    fflush(stdout);
                    Trans(atoi(msg+1));

                    // send done reply
                    char server_reply[MAX_MSG_LENGTH] = "";
                    sprintf(server_reply, "D%d", tx_count);
                    sendMsg(sock, server_reply, sizeof(server_reply));
                    
                    printf("%.2f: # %d (Done) from %s\n", getEpochS(), tx_count, buf);
                    fflush(stdout);

                    // update client stats
                    bool found = false;
                    for(int i=0; i < (int)clientList.size(); i++){
                        if(strcmp(buf, clientList[i].machineName) == 0){
                            found = true;
                            clientList[i].completed++;
                            break;
                        }
                    }
                    if(!found){
                        clientInfo newClient;
                        strcat(newClient.machineName, buf);
                        newClient.completed++;
                        clientList.push_back(newClient);
                    }
                }
            }
        }
    }
    
    double run_time = (tx_count < 1)? ZERO : getRunTime(start) - TIMEOUT;
    printf("\nSUMMARY\n");
    for(int i=0; i < (int)clientList.size(); i++)
        printf("%d transactions from %s\n", clientList[i].completed, clientList[i].machineName);
    printf("%.1f transactions/second    (%d/%.2f)\n", (double)tx_count/ run_time, tx_count, run_time);

    close(fd);

    return 0;
}