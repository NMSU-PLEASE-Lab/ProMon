#ifndef PROBE_H
#define PROBE_H

#include <msgpack.h>


void atExit(void);
void atExitSignal(int sigNum);
void atQuickExit(void);
static int openSocketTCP(char *ip, char *port);
static int openSocketUDP(char *ip, char *port);
static void closeSocket();
static void getEnvData();
//static int getEnvData(const char *eventRate, const char *eventPriority, const char *eventCategory);
//string countEvents(const char *tag);
int sendInstRecordTCP(msgpack_sbuffer* msgpack_buffer);
int sendInstRecordMSGQUEUE(msgpack_sbuffer* msgpack_buffer);
int sendInstRecordPOSIX(msgpack_sbuffer* msgpack_buffer);
int sendInstRecordUDP(msgpack_sbuffer* msgpack_buffer);

extern int sendInstRecord(const char *args, const char *name, const char *type, const char *category, unsigned int priority,
               unsigned int position, unsigned int samplingRate);
extern int accessDataInst(const char *args, const char *name, const char *type, const char *category, unsigned int priority,unsigned int position, unsigned int samplingRate,const char *varType, void *varAdd);


#endif
