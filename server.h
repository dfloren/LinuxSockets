#ifndef SERVER_H
#define SERVER_H

#define ARGC 2
#define MAX_MSG_LENGTH 512
#define MAX_NAME_LENGTH 256
#define TIMEOUT 30
#define ZERO 0.0000001

struct clientInfo{
    int completed = 0;
    char machineName[MAX_NAME_LENGTH] = "";
};

void Trans( int n ); // tands.cpp

#endif