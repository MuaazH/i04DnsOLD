#ifndef _LOGGER_H_
#define _LOGGER_H_
#include "../util/util.h"

enum LogApp {
    LOG_DEFAULT = 1,
    LOG_CONF,
    LOG_DNS,
    LOG_HTTP
};

void logInit();
void logDestroy();

/*
  This is the json format (64 bytes)
  TOTAL SIZE = 64 + sizeof(LogLine)
  TOTAL SIZE = 64 + 280 = 348
{time: "", app: , int: , msg: , p1: "", p2: "", p3:"", p4:""};
*/

#define LOG_MAX_PRARM_LEN 64
#define LOG_MAX_JSON_OBJ_SIZE 350
#define LOG_HISTORY_MAX_LINES 0x80

int logToJson(char *buf, int maxSize);

void logInfo(int app, int msg, const char *p1 = 0, const char *p2 = 0, const char *p3 = 0, const char *p4 = 0);
void logError(int app, int msg, const char *p1 = 0, const char *p2 = 0, const char *p3 = 0, const char *p4 = 0);
void logWarn(int app, int msg, const char *p1 = 0, const char *p2 = 0, const char *p3 = 0, const char *p4 = 0);
void logDebug(int app, int msg, const char *p1 = 0, const char *p2 = 0, const char *p3 = 0, const char *p4 = 0);

#endif

