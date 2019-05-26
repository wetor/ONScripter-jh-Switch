#include <switch.h>
#include "main.h"
#include "Utils.h"




int main(int argc, char *argv[]) {
#if defined(SWITCH)
	twiliInitialize();
#endif

	utils::printInfo("ONScripter-Jh for Nintendo Switch\n\n");

	//ons.MPV_VideoPlayer((char*)"/onsemu/yuanzhikong/mov/opening.mp4",true);
	
	argc = 5;
	argv[0] = (char*)"ons";
	argv[1] = (char*)"--root";
	argv[2] = (char*)"sdmc:/onsemu/Rewrite";
	argv[3] = (char*)"--compatible";
	argv[4] = (char*)"--fontcache";
	//argv[5] = (char*)"--fullscreen";	
	//argv[5] = (char*)"--debug:1";
	
	OnsMain(argc, argv);

#if defined(SWITCH)
	twiliExit();
#endif
	return 0;
}