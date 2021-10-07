
#include "logger.h"
#include <iostream>
#include <stdio.h>
#include <cstdarg>
#include <ctime>
#include "../base/heap_dbg.h"
#include <system.h>
#include "locale.h"

struct LogLine {
	char time[16];
	int app;
	int lvl;
	int msg;
	char p1[64];
	char p2[64];
	char p3[64];
	char p4[64];
};

enum {
	LOG_LVL_INFO = 1,
	LOG_LVL_WARN,
	LOG_LVL_ERROR,
	LOG_LVL_DEBUG,
};

using namespace std;

LOCK logLock;
int hStart = 0;
int hCount = 0;
LogLine history[LOG_HISTORY_MAX_LINES];

void logInit() {
	hStart = 0;
	hCount = 0;
	mem_zero((void *) &history, sizeof(history));
	logLock = lock_create();
}

void logDestroy() {
	lock_destroy(logLock);
}

int logLineToJson(char *pBuf, LogLine *pLine) {
	bool p1 = pLine->p1[0];
	bool p2 = pLine->p2[0];
	bool p3 = pLine->p3[0];
	bool p4 = pLine->p4[0];
	sprintf(
		pBuf,
		"{\"time\": \"%s\", \"app\": %d, \"lvl\": %d, \"msg\": %d%s%s%s%s%s%s%s%s%s%s%s%s}",
		pLine->time,
		pLine->app,
		pLine->lvl,
		pLine->msg,
		p1 ? ", \"p1\": \"" : "", p1 ? pLine->p1 : "", p1 ? "\"" : "",
		p2 ? ", \"p2\": \"" : "", p2 ? pLine->p2 : "", p2 ? "\"" : "",
		p3 ? ", \"p3\": \"" : "", p3 ? pLine->p3 : "", p3 ? "\"" : "",
		p4 ? ", \"p4\": \"" : "", p4 ? pLine->p4 : "", p4 ? "\"" : ""
	);
	return str_length(pBuf);
}

int logToJson(char *buf, int maxSize) {
	int maxLen = maxSize - 3;
	if (maxLen <= 0) {
		buf[0] = 0;
		return 0;
	}
	lock_wait(logLock);
	int j = hStart;
	int len = 1; // make space for the [
	buf[0] = '[';
	for(int i = 0; i < hCount; i++)
	{
		int tmp = len + LOG_MAX_JSON_OBJ_SIZE;
		if (i) tmp++; // make space for the comma
		if (tmp < maxLen) {
			if (i) buf[len++] = ','; // put the comma
			int objLen = logLineToJson(buf + len, &history[j]);
			len += objLen;
		}
		j++;
		if (j >= LOG_HISTORY_MAX_LINES) {
			j = 0;
		}
	}
	buf[len++] = ']'; // put the site
	buf[len] = 0;
	lock_unlock(logLock);
	return len;
}

#define printParam(param, paramName) if (param) { \
			cout << "  " << (paramName ? paramName : "??"); \
			cout << " = " << param; \
		}

inline void println(const char *time, int app, int lvl, int msg, const char *p1, const char *p2, const char *p3, const char *p4) {
	cout << time << " ";
	switch (app) {
		case LOG_DNS:
			cout << "[ DNS ] "; break;
		case LOG_CONF:
			cout << "[ CFG ] "; break;
		case LOG_HTTP:
			cout << "[ WEB ] "; break;
		default:
			cout << "[ SRV ] "; break;
	}
	switch (lvl) {
		case LOG_LVL_INFO:
			cout << "[inf] "; break;
		case LOG_LVL_WARN:
			cout << "[wrn] "; break;
		case LOG_LVL_ERROR:
			cout << "[err] "; break;
		case LOG_LVL_DEBUG:
			cout << "[dbg] "; break;
	}
	cout << log_messages[msg].msg;
	printParam(p1, log_messages[msg].p1);
	printParam(p2, log_messages[msg].p2);
	printParam(p3, log_messages[msg].p3);
	printParam(p4, log_messages[msg].p4);
	cout << endl;
}

inline void copyParam(char *dst, const char *src) {
	if (!src) {
		dst[0] = 0;
		return;
	}
	int len = str_length(src);
	if (len >= LOG_MAX_PRARM_LEN) {
		len = LOG_MAX_PRARM_LEN - 1;
	}
	mem_copy((void *) dst, (void *) src, len);
	dst[len] = 0;
}

void log(int app, int lvl, int msg, const char *p1, const char *p2, const char *p3, const char *p4) {
	lock_wait(logLock);
	int i = hStart;
	if (hCount >= LOG_HISTORY_MAX_LINES) {
		hStart++;
		if (hStart >= LOG_HISTORY_MAX_LINES) {
			hStart = 0;
		}
	} else {
		i += hCount;
		hCount++;
	}
	LogLine *x = &history[i];
	time_t ctt = time(0);
	tm *time = localtime(&ctt);
	sprintf(x->time, "%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
	x->app = app;
	x->lvl = lvl;
	x->msg = msg;
	copyParam((char *) &x->p1, p1);
	copyParam((char *) &x->p2, p2);
	copyParam((char *) &x->p3, p3);
	copyParam((char *) &x->p4, p4);
	println(x->time, app, lvl, msg, p1, p2, p3, p4);
	lock_unlock(logLock);
}

void logInfo(int app, int msg, const char *p1, const char *p2, const char *p3, const char *p4) {
	log(app, LOG_LVL_INFO, msg, p1, p2, p3, p4);
}

void logError(int app, int msg, const char *p1, const char *p2, const char *p3, const char *p4) {
	log(app, LOG_LVL_ERROR, msg, p1, p2, p3, p4);
}

void logWarn(int app, int msg, const char *p1, const char *p2, const char *p3, const char *p4) {
	log(app, LOG_LVL_WARN, msg, p1, p2, p3, p4);
}

void logDebug(int app, int msg, const char *p1, const char *p2, const char *p3, const char *p4) {
#ifdef DEBUG_CONF
	log(app, LOG_LVL_DEBUG, msg, p1, p2, p3, p4);
#endif
}
