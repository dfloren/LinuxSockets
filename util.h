#ifndef UTIL_H
#define UTIL_H

void sendMsg(int fd, char* buf, int len);
double getEpochS();
double getRunTime(struct timeval start);

#endif