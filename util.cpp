#include <cstddef>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

void sendMsg(int fd, char* msg_buff, int len){
        int sent = 0;
        while((len-sent) > 0)
            sent += send(fd, msg_buff+sent, len-sent, 0);
}

double getEpochS(){
    struct timeval clock;
    double seconds;
    gettimeofday(&clock, NULL);
    seconds = (double) clock.tv_sec + (double) clock.tv_usec/1000000;
    return seconds;
}

double getRunTime(struct timeval start){
    struct timeval end;
    gettimeofday(&end, NULL);
    double run_time = (double) (end.tv_sec - start.tv_sec) +
                        (double) (end.tv_usec - start.tv_usec) / 1000000;
    return run_time;
}