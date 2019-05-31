#include "Common.h"

int sardec_main(int argc, char *argv[])
{
	SarReader cSR;
	unsigned long length, buffer_length = 0;
	unsigned char *buffer = NULL;
	char file_name[256], dir_name[256];
	unsigned int i, j, count;
	FILE *fp;
	struct stat file_stat;

	if (argc != 2) {
		fprintf(stderr, "Usage: sardec arc_file\n");
		exit(-1);
	}
	if (cSR.open(argv[1]) != 0) {
		fprintf(stderr, "can't open file %s\n", argv[1]);
		exit(-1);
	}
	count = cSR.getNumFiles();

	SarReader::FileInfo sFI;

	for (i = 0; i < count; i++) {
		sFI = cSR.getFileByIndex(i);

		length = cSR.getFileLength(sFI.name);

		if (length > buffer_length) {
			if (buffer) delete[] buffer;
			buffer = new unsigned char[length];
			buffer_length = length;
		}
		if (cSR.getFile(sFI.name, buffer) != length) {
			fprintf(stderr, "file %s can't be retrieved\n", sFI.name);
			continue;
		}

		strcpy(file_name, sFI.name);
		for (j = 0; j < strlen(file_name); j++) {
			if (file_name[j] == '\\') {
				file_name[j] = '/';
				strncpy(dir_name, file_name, j);
				dir_name[j] = '\0';

				/* If the directory does'nt exist, create it */
				if (stat(dir_name, &file_stat) == -1 && errno == ENOENT)
					mkdir(dir_name, 00755);
			}
		}

		printf("opening %s\n", file_name);
		if ((fp = fopen(file_name, "wb"))) {
			fwrite(buffer, 1, length, fp);
			fclose(fp);
		}
		else {
			printf(" ... falied\n");
		}
	}

	if (buffer) delete[] buffer;

	exit(0);
}

int nsadec_main(char* file)
{
	NsaReader cNR;
	unsigned int nsa_offset = 0;
	unsigned long length;
	unsigned char *buffer;
	char file_name[256], dir_name[256];
	unsigned int i, j, count;
	int archive_type = BaseReader::ARCHIVE_TYPE_NSA;
	FILE *fp;
	struct stat file_stat;
	/*if (argc >= 2) {
		while (argc > 2) {
			if (!strcmp(argv[1], "-ns2")) {
				archive_type = BaseReader::ARCHIVE_TYPE_NS2;
			}
			else if (!strcmp(argv[1], "-offset")) {
				nsa_offset = atoi(argv[2]);
				argc--;
				argv++;
			}

			argc--;
			argv++;
		}
	}
	if (argc != 2) {
		fprintf(stderr, "Usage: nsadec [-offset ##] [-ns2] arc_file\n");
		exit(-1);
	}*/
	cNR.openForConvert(file, archive_type, nsa_offset);
	count = cNR.getNumFiles();
	SarReader::FileInfo sFI;
	for (i = 0; i < count; i++) {
		sFI = cNR.getFileByIndex(i);
		length = cNR.getFileLength(sFI.name);
		//buffer = new unsigned char[length];
		/*unsigned int len;
		if ((len = cNR.getFile(sFI.name, buffer)) != length) {
			//fprintf( stderr, "file %s can't be retrieved\n", sFI.name );
			printf("file %s is not fully retrieved %d %lu\n", sFI.name, len, length);
			length = sFI.length;
			//continue;
		}*/

		//strcpy(file_name, sFI.name);

		//多级目录创建
		/*for (j = 0; j < strlen(file_name); j++) {
			if (file_name[j] == '\\') {
				file_name[j] = '/';
				strncpy(dir_name, file_name, j);
				dir_name[j] = '\0';

				// If the directory does'nt exist, create it

				if (stat(dir_name, &file_stat) == -1 && errno == ENOENT)
					mkdir(dir_name, 00755);
			}
		}*/

		printf("%d   %lu   %s\n", i, length, sFI.name);
		//写出文件
		/*if ((fp = fopen(file_name, "wb"))) {
			fwrite(buffer, 1, length, fp);
			fclose(fp);
		}
		else {
			printf("opening %s ... falied\n", file_name);
		}*/

		//delete[] buffer;
	}
	return 0;
}
