#ifndef PROBE_H
#define PROBE_H

#include <string>
#include <msgpack.h>

using namespace std;

struct  eventRecord
{
    long   timeSec;
    long   timeNanoSec;
    int    rank;
    string jobId;
    string username;
    string jobMS;
    string eventType;
    string eventName;
    string eventPosition;
    string eventCount;
    string eventCategory;
    string eventVarValue;
};

void atExit(void);
void atExit(int sigNum);
void atQuickExit(void);
static int openSocketTCP(char *ip, char *port);
static int openSocketUDP(char *ip, char *port);
static void closeSocket();
static void getEnvData();
static int getEnvData(const char *eventRate, const char *eventPriority, const char *eventCategory);
string countEvents(const char *tag);
int sendInstRecordTCP(const char *tag);
int sendInstRecordMSGQUEUE(const char *tag);
int sendInstRecordPOSIX(const char *tag);
int sendInstRecordUDP(const char *tag);

extern "C"
int sendInstRecord(const char *args, const char *name, const char *type, const char *category, unsigned int priority,
               unsigned int position, unsigned int samplingRate);
extern "C"
int accessDataInst(const char *args, const char *name, const char *type, const char *category, unsigned int priority,unsigned int position, unsigned int samplingRate,const char *varType, void *varAdd);




#endif
