#ifndef _LOCALE_H_
#define _LOCALE_H_

struct LogMessage {
    const char *msg;
    const char *p1;
    const char *p2;
    const char *p3;
    const char *p4;
};

LogMessage log_messages[] {
/* 0x000 */	{"???", 0, 0, 0, 0},
/* 0x001 */ {"i04Dns", "version", 0, 0, 0},
/* 0x002 */ {"Elevating Privileges...", 0, 0, 0},
/* 0x003 */ {"Shutting Down...", 0, 0, 0},
/* 0x004 */ {"Initializing Network...", 0, 0, 0, 0},
/* 0x005 */ {"Loading Config...", 0, 0, 0, 0},
/* 0x006 */ {"Reloading Config...", 0, 0, 0, 0},
/* 0x007 */ {"Duplicate Name Ignored", "name", 0, 0, 0},
/* 0x008 */ {"Invalid Config Line", "line", 0, 0, 0},
/* 0x009 */ {"Dropping", "type", 0, 0, 0},
/* 0x00A */ {"Dropping", "name", 0, 0, 0},
/* 0x00B */ {"Answering From Local Records", 0, 0, 0, 0},
/* 0x00C */ {"Answering From Cache", 0, 0, 0, 0},
/* 0x00D */ {"Query Timedout", "id", 0, 0, 0},
/* 0x00E */ {"Forwarding", "query", "server", "id", 0},
/* 0x00F */ {"Server Overloaded", 0, 0, 0, 0},
/* 0x010 */ {"Query", "client", "name", 0, 0},
/* 0x011 */ {"Answer", "server", "id", "delay-ms", 0},
/* 0x012 */ {"Unknown Answer", "id", 0, 0, 0},
/* 0x013 */ {"Failed To Decode Msg", 0, 0, 0, 0},
/* 0x014 */ {"Refusing Because Queries Count Is Not One", "count", 0, 0, 0},
/* 0x015 */ {"Refusing", "type", 0, 0, 0},
/* 0x016 */ {"Refusing", "name", 0, 0, 0},
/* 0x017 */ {"Starting", 0, 0, 0, 0},
/* 0x018 */ {"Failed To Open Port", "port", 0, 0, 0},
/* 0x019 */ {"Port Open", "port", 0, 0, 0},
/* 0x01A */ {"Elevating Privileges...FAILED", 0, 0, 0, 0},
/* 0x01B */ {"Elevating Privileges...OK", 0, 0, 0, 0},
/* 0x01C */ {"Initializing Network...FAILED", 0, 0, 0, 0},
/* 0x01D */ {"Initializing Network...OK", 0, 0, 0, 0},
/* 0x01E */ {"Loading Config...FAILED", 0, 0, 0, 0},
/* 0x01F */ {"Loading Config...OK", 0, 0, 0, 0},
/* 0x020 */ {"Reloading Config...FAILED", 0, 0, 0, 0},
/* 0x021 */ {"Reloading Config...OK", 0, 0, 0, 0},
/* 0x022 */ {"Http Port Already Set, Ignoring Line", "line", 0, 0, 0},
/* 0x023 */ {"Http Port Not Set, Using Default Port", "port", 0, 0, 0},
/* 0x024 */ {"Http Port Changed, Restarting Server", 0, 0, 0},
/* 0x025 */ {"Web Interface File Already Set, Ignoring Line", "line", 0, 0, 0},
/* 0x026 */ {"Web Interface File Name Is Too Long, Ignoring Line", "file", "max", 0, 0},
/* 0x027 */ {"Web Interface File Not Set, Stopping Http Server", 0, 0, 0, 0},
/* 0x028 */ {"Loading Web Interface File...", 0, 0, 0, 0},
/* 0x029 */ {"Loading Web Interface File...FAILED", 0, 0, 0, 0},
/* 0x02A */ {"Loading Web Interface File...OK", 0, 0, 0, 0},
/* 0x02B */ {"Web Interface File Is Too Large", "max", 0, 0, 0},
/* 0x02C */ {"Could Not Read Web Interface File", "file", 0, 0, 0},
/* 0x02D */ {"Web Interface File Is Not Valid", "file", 0, 0, 0},
/* 0x02E */ {"Web Interface Admin Already Set, Ignoring Line", "line", 0, 0, 0},
/* 0x02F */ {"Web Interface Admin Is Too Long, Ignoring Line", "line", 0, 0, 0},
/* 0x030 */ {"Web Interface Admin Password Already Set, Ignoring Line", "line", 0, 0, 0},
/* 0x031 */ {"Web Interface Admin Password Is Too Long, Ignoring Line", "line", 0, 0, 0}

};

#endif

