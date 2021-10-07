#include "CConfigFile.h"
#include "logger.h"
#include "dns/dnsServer.h"
#include "../base/heap_dbg.h"
#include "version.h"
#include <system.h>

static const char *configFileName = "./CONF";
static CConfigFile *pConfig = 0;

static void loadConfig() {

	bool reload;
	if (!pConfig) {
		NEW_OBJ(pConfig, CConfigFile, ((char *) configFileName));
		logInfo(LOG_CONF, 0x005); // Loading Config...
		reload = false;
	} else {
		logInfo(LOG_CONF, 0x006); // Reloading Config...
		reload = true;
	}
    if (pConfig->Load()) {
		if (reload) {
			logInfo(LOG_CONF, 0x021); // Reloading Config...OK
		} else {
			logInfo(LOG_CONF, 0x01F); // Loading Config...OK
		}
	} else {
		if (reload) {
			logError(LOG_CONF, 0x020); // Reloading Config...FAILED
		} else {
			logError(LOG_CONF, 0x01E); // Loading Config...FAILED
		}
	}

}

void runDnsSrv(void *pConfig) {
	dns::run((CConfigFile *) pConfig);
}

int main(int argc, char *argv[])
{
	logInit();
	logInfo(LOG_DEFAULT, 0x001, VERSION);
	//

	// Quit if you can't init the network
	logInfo(LOG_DEFAULT, 0x004); // Initializing Network...
	int netstatus = net_init();
	if (netstatus) {
		logInfo(LOG_DEFAULT, 0x01C); // Initializing Network...FAILED
		logError(LOG_DEFAULT, 0x003); // Shutting Down...
		logDestroy();
		return 2;
	}
	logInfo(LOG_DEFAULT, 0x01D); // Initializing Network...OK

	// laod shit
	loadConfig();

	// run shit
	runDnsSrv(pConfig);
    return 0;
}

