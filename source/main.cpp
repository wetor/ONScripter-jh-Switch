


#include <switch.h>
#include <twili.h>
#include "main.h"
#include "Utils.h"
#include "Common.h"

extern u32 __nx_applet_type;
extern char *fake_heap_end;
extern "C"
{
	void __nx_win_init(void);
	void __libnx_init_time(void);

	void __appInit(void)
	{
		Result rc = smInitialize();
		if (R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

		rc = setsysInitialize();
		if (R_SUCCEEDED(rc))
		{
			SetSysFirmwareVersion fw;
			rc = setsysGetFirmwareVersion(&fw);
			if (R_SUCCEEDED(rc)) hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
			setsysExit();
		}

		rc = timeInitialize();
		if (R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_Time));

		__libnx_init_time();

		rc = fsInitialize();
		if (R_FAILED(rc)) fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

		fsdevMountSdmc();
	}
}




static void *ghaddr;
int main(int argc, char *argv[]) {
	twiliInitialize();

	srand(time(NULL));

	//if (R_FAILED(svcSetHeapSize(&ghaddr, 0x10000000))) exit(1);
	//fake_heap_end = (char*)ghaddr + 0x10000000;

	if (R_FAILED(appletInitialize()))	printf("test00\n");
	if (R_FAILED(hidInitialize()))		printf("test01\n");
	if (R_FAILED(accountInitialize()))	printf("test02\n");
	if (R_FAILED(ncmInitialize()))		printf("test03\n");
	if (R_FAILED(nsInitialize()))		printf("test04\n");
	if (R_FAILED(psmInitialize()))		printf("test05\n");
	if (R_FAILED(setInitialize()))		printf("test06\n");
	if (R_FAILED(setsysInitialize()))	printf("test07\n");
	if (R_FAILED(splInitialize()))		printf("test08\n");
	if (R_FAILED(bpcInitialize()))		printf("test09\n");
	if (R_FAILED(nifmInitialize()))		printf("test10\n");


	utils::printInfo("ONScripter-Jh for Nintendo Switch\n\n");
	//ons_exit(0);
	//ons.MPV_VideoPlayer((char*)"/onsemu/yuanzhikong/mov/opening.mp4",true);
	//char path[256];
	
	char path[256];
	int fullmode = 0;
	if (envHasArgv() && (argc > 1)) {
		strcpy(path, argv[1]);
		if (argc > 2) {
			fullmode = !strcmp(argv[2], "1");
		}
		argv[2] = path;
	}
	else
		argv[2] = (char*)"sdmc:/onsemu/hanchan";

	argv[1] = (char*)"--root";
	argv[3] = (char*)"--compatible";
	argv[4] = (char*)"--fontcache";
	argc = 6;
	if (fullmode) 
		argv[5] = (char*)"--fullscreen";
	else
		argv[5] = (char*)"--window";

	
	/*argv[5] = (char*)"sdmc:/onsemu/hanchan/arc.nsa";
	nsadec_main(argv[5]);*/

	OnsMain(argc, argv);


	ons_exit(0);
	

	
	//svcSetHeapSize(&ghaddr, ((u8*)envGetHeapOverrideAddr() + envGetHeapOverrideSize()) - (u8*)ghaddr);
	//twiliExit();
	return 0;
}



void ons_exit(int flag) {


	printf("exit1 %d\n", flag);

	envSetNextLoad("sdmc:/onsemu/ONScripter.nro", "sdmc:/onsemu/ONScripter.nro sdmc:/onsemu/ONScripter.nro");

	nifmExit();
	bpcExit();
	splExit();
	setsysExit();
	setExit();
	psmExit();
	nsExit();
	ncmExit();
	accountExit();
	hidExit();
	appletExit();

	//svcSetHeapSize(&ghaddr, ((u8*)envGetHeapOverrideAddr() + envGetHeapOverrideSize()) - (u8*)ghaddr);
	printf("exit4 %d\n", flag);
	//twiliExit();

	printf("exit5 %d\n", flag);
	exit(flag);

}