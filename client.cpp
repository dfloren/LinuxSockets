#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>  
#include <arpa/inet.h> 

#include "client.h"
#include "util.h"

int main(int argc, char const *argv[]) {
    if(argc != ARGC){
        printf("Usage: %s port ip-address\n", argv[0]);
        return 0;
    }

    int host_port = atoi(argv[1]);
    const char* host_ip = argv[2];

    char client_name[MAX_NAME_LENGTH] = "";
    getClientName(client_name);

    char logPath[MAX_NAME_LENGTH] = "./logs/";
    strcat(logPath, client_name);
    FILE* log_fp = fopen(logPath, "w");
    if(log_fp == NULL){
        perror("Failed to open log file");
        return 1;
    }

    int fd = socket(AF_INET, // an Internet socket
                    SOCK_STREAM, // reliable stream-like socket
                    0); // OS determine the protocol (TCP)

    if (fd < 0){
        perror("Failed to establish socket");
        return 1;
    }

    struct sockaddr_in serv_addr; 

    // setup server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host_ip);
    serv_addr.sin_port = htons(host_port);

    // bind socket to the server address
    if (connect(fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0){
        perror("Failed connecting to host");
        return 1;
    }

    fprintf(log_fp, "Using port %d\nUsing server address %s\nHost %s\n", host_port, host_ip, client_name);

    int trans_count = 0;
    char cmd_buff[MAX_MSG_LENGTH] = "";
    char msg_buff[MAX_MSG_LENGTH] = "";
    char server_reply[MAX_MSG_LENGTH] = "";

    strcat(client_name, ":"); // msg format: "machinename.pid:T/S<int>"

    // send & recv messages
    while(fgets(cmd_buff,MAX_MSG_LENGTH, stdin) != NULL){
        strtok(cmd_buff, "\n");

        if(cmd_buff[0] == 'S'){
            fprintf(log_fp, "Sleep %d units\n", atoi(&cmd_buff[1]));
            Sleep(atoi(&cmd_buff[1]));
            continue;
        }

        if(strcmp(cmd_buff, "quit") == 0)
            break;

        msg_buff[0] = '\0'; 
        strcat(msg_buff, client_name);
        strcat(msg_buff, cmd_buff);
        sendMsg(fd, msg_buff, sizeof(msg_buff));
        fprintf(log_fp, "%.2f Send (T %s)\n", getEpochS(), &cmd_buff[1]);
        trans_count++;

        // Block until server replies
        if(recv(fd , server_reply , MAX_MSG_LENGTH, 0) < 0)
            perror("Failed to read server reply");
        else{
            fprintf(log_fp, "%.2f Recv (D %s)\n", getEpochS(), &server_reply[1]);
        }
    }

    // close connection
    msg_buff[0] = '\0';
    strcat(msg_buff, "quit");
    sendMsg(fd, msg_buff, sizeof(msg_buff));
    fprintf(log_fp, "Sent %d transactions.\n", trans_count);

    close(fd);
    fclose(log_fp);

    return 0;
} 


void getClientName(char buf[]){
    char pid_buf[MAX_PID_LENGTH];
    gethostname(buf, MAX_NAME_LENGTH);
    snprintf(pid_buf, MAX_PID_LENGTH, ".%d", getpid());
    strcat(buf, pid_buf); // appends null-byte
}