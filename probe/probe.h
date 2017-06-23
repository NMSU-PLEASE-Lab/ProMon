#ifndef PROBE_H
#define PROBE_H

#include <string>

using namespace std;

void atExit(void);
void atExit(int sigNum);
void atQuickExit(void);
static int openSocketTCP(char *ip, char *port);
static int openSocketUDP(char *ip, char *port);
static void closeSocket();
static void getEnvData();
static int getEnvData(const char* eventRate, const char* eventPriority, const char* eventCategory);
string countEvents(const char *tag);
int sendInstRecordTCP(const char *tag);
int sendInstRecordMSGQUEUE(const char *tag);
int sendInstRecordPOSIX(const char *tag);
int sendInstRecordUDP(const char *tag);

extern "C"
int sendInstRecord(const char *args, const char *tag);

extern "C" 
int accessDataInst(const char *args, const char *tag, void* varAdd);


#endif
