#include <switch.h>
#ifdef DEBUG
#include <twili.h>
#endif
#include "main.h"
#include "Utils.h"
#include "Common.h"


void *mouse_png;
int mouse_png_size;
int english = 0;
char return_path[256];


// args:
// 0 : self NRO program path
// 1 : ONScripter game folder
// 2 : mode: 0 default; mode&1 fullscreen stretch; mode&2 outline; mode&4 english
// 3 : return nro path after game exit
int main(int argc, char *argv[])
{
#ifdef DEBUG
	twiliInitialize();
	twiliBindStdio();
#endif

	srand(time(NULL));
	romfsInit();
	strcpy(return_path, "sdmc:/onsemu/exefs/ONSBrowser.nro");
	utils::printInfo("ONScripter-Jh for Nintendo Switch\n\n");

	// argc = 3;
	// argv[1] = (char*)"sdmc:/onsemu/tuskihime";
	// argv[2] = (char*)"4";
	char path[256];
	int fullmode = 0;
	int outline = 0;

	if (envHasArgv() && argc > 1)
	{
		strcpy(path, argv[1]);
		if (argc > 2)
		{
			int setting = atoi(argv[2]);
			if(setting & 1)
				fullmode = 1;
			if(setting & 2)
				outline = 1;
			if(setting & 4)
				english = 1;
		}
		argv[2] = path;
		if (argc > 3)
			strcpy(return_path, argv[3]);
	}
	else
		ons_exit(0);

	argv[1] = (char *)"--root";
	argv[3] = (char *)"--compatible";
	argv[4] = (char *)"--fontcache";

	if (fullmode)
		argv[5] = (char *)"--fullscreen";
	else
		argv[5] = (char *)"--window";
	argc = 6;
	if (outline)
	{
		argv[argc++] = (char *)"--render-font-outline";
	}
	if (english)
	{
		argv[argc++] = (char *)"--enc:sjis";
	}
	// argv[argc++] = (char *)"--debug:1";
	
	//argv[6] = (char*)"sdmc:/onsemu/hanchan";
	//nsadec_main(argv[6]);

	mouse_png_size = 1699;
	mouse_png = (void *)malloc(mouse_png_size);
	FILE* f = fopen("romfs:/cursor/mouse.png", "rb");
    if (f)
    {
		fread(mouse_png, mouse_png_size, 1, f);
        fclose(f);

    } else {
        printf("romfs:/cursor/mouse.png open fail\n");
    }

	OnsMain(argc, argv);

	ons_exit(EXIT_SUCCESS);
	return 0;
}

void ons_exit(int flag)
{

	
    romfsExit();

#ifdef DEBUG
	twiliExit();
#endif
	char args[256];
	sprintf(args, "\"%s\"", return_path);
	envSetNextLoad(return_path, args);
	exit(EXIT_SUCCESS);
}