#include   <stdiog.h> 
#include   <dirent.h>
#include   <string.h> 
#include   <windows.h>
#define MAX 100

struct tanda{
	WCHAR artist[31];
	WCHAR title[31];
}standa[MAX];

typedef struct MP3INFOA
{
	char     identify[3];          // TAG
	char     Title[30];            // 歌曲名,30个字节   
	char     Artist[30];           // 歌手名,30个字节   
	char     Album[30];         // 所属唱片,30个字节   
	char     Year[4];             // 年份,4个字符   
	char     Comment[28];   // 注释,28个字节   
	char     reserved[3];
} MP3INFOA;

typedef struct MP3INFOW // UNICODE用来存储信息的结构体。
{
	WCHAR    Title[31];            // 歌曲名
	WCHAR    Artist[31];           // 歌手名
	WCHAR    Album[31];            // 所属唱片
	WCHAR    Year[5];              // 年份
	WCHAR    Comment[29];          // 注释
} MP3INFOW;

typedef struct FileList
{
	char   filename[260];
	struct   FileList   *next;
} FILENODE;

wchar_t* c2w(const char *str/*char to wchar*/)
{
	int length = strlen(str) + 1;
	wchar_t *t = (wchar_t*)malloc(sizeof(wchar_t)*length);
	memset(t, 0, length*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str), t, length);
	return t;
} 

BOOL GetMp3Info(const WCHAR *path, MP3INFOW *infw)
{
	char temp[31];
	FILE *rf;
	MP3INFOA infa;

	if (!path) return FALSE;

	if (NULL == (rf = _wfopen(path, L"rb"))) return FALSE;

	fseek(rf, -128, SEEK_END);
	fread(&infa, sizeof(MP3INFOA), 1, rf);
	fclose(rf); // 关闭文件

	if (infa.identify[0] != 'T' || infa.identify[1] != 'A' || infa.identify[2] != 'G') return FALSE;

	memset(infw, 0, sizeof(MP3INFOW));

	temp[30] = 0;
	memcpy(temp, infa.Title, 30);
	MultiByteToWideChar(CP_ACP, 0, temp, -1, infw->Title, 31);
	memcpy(temp, infa.Artist, 30);
	MultiByteToWideChar(CP_ACP, 0, temp, -1, infw->Artist, 31);
	memcpy(temp, infa.Album, 30);
	MultiByteToWideChar(CP_ACP, 0, temp, -1, infw->Album, 31);

	temp[4] = 0;
	memcpy(temp, infa.Year, 4);
	MultiByteToWideChar(CP_ACP, 0, temp, -1, infw->Year, 5);

	temp[28] = 0;
	memcpy(temp, infa.Comment, 28);
	MultiByteToWideChar(CP_ACP, 0, temp, -1, infw->Comment, 29);
	return TRUE;
}

BOOL SetMp3Info(const WCHAR *path, const MP3INFOW *infw)
{
	char identify[3]; // TAG
	FILE *rwf;
	MP3INFOA infa;

	if (!path || !infw) return FALSE;

	memset(&infa, 0, sizeof(MP3INFOA));
	infa.identify[0] = 'T';
	infa.identify[1] = 'A';
	infa.identify[2] = 'G';

	WideCharToMultiByte(CP_ACP, 0, infw->Title, -1, infa.Title, 30, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, infw->Artist, -1, infa.Artist, 30, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, infw->Album, -1, infa.Album, 30, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, infw->Year, -1, infa.Year, 4, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, infw->Comment, -1, infa.Comment, 28, NULL, NULL);

	if (NULL == (rwf = _wfopen(path, L"rb+"))) return FALSE;

	fseek(rwf, -128, SEEK_END);
	fread(identify, 3, 1, rwf);

	if (identify[0] == 'T' && identify[1] == 'A' && identify[2] == 'G')
	{
		fseek(rwf, -128, SEEK_END);
	}
	else
	{
		fseek(rwf, 0, SEEK_END);
	}
	fwrite(&infa, sizeof(MP3INFOA), 1, rwf);
	fclose(rwf);
	return TRUE;
}

FILENODE*  getFiles(char *dir)
{
	DIR   *directory_pointer;
	struct   dirent   *entry;
	directory_pointer = opendir(dir);
	struct FileList start;
	struct FileList *filesNode;
	start.next = NULL;
	filesNode = &start;
	while ((entry = readdir(directory_pointer)) != NULL)
	{
		filesNode->next = (struct   FileList   *)malloc(sizeof(struct   FileList));
		filesNode = filesNode->next;
		strcpy(filesNode->filename, entry->d_name);
		filesNode->next = NULL;
	}
	closedir(directory_pointer);
	filesNode = start.next;
	return filesNode;
}

BOOL reflib(){
	//library path changing
	char oldpath[260] = "C:\\Users\\Surfy\\Desktop\\musicfortest\\";
	const char *mlib = "zmusiclib.txt";
	int len_of_op=strlen(oldpath);
	printf("Refresh library starts.\n");
	printf("Music library is at %s\nIf you want to change it, press 'y', or anykey else to continue\n", oldpath);
	if (getchar() == 'y'){
		scanf_s("%s", &oldpath,261);
		len_of_op = strlen(oldpath);
	}
	//library path changing ends

	int sofs = 0;//sum of songs
	MP3INFOW *mp3infow = (MP3INFOW *)malloc(sizeof(MP3INFOW));
	FileList *filesNode;
	filesNode = getFiles(oldpath);

	if (filesNode == NULL)
	{
		printf("get file failed");
		return 0;
	}
	FILE *fp;
	const char *filepath = strcat(oldpath, mlib);
	if ((fp = fopen(filepath, "r+")) == NULL){
		printf("Cannot open music library,strike any key to exit.");
		getchar();
		return 0;
	}
	char newpath[260] = "C:\\Users\\Surfy\\Desktop\\musicfortest\\";
	filesNode = filesNode->next->next;
	while (filesNode)
	{
		strcat(newpath, filesNode->filename);
		GetMp3Info(c2w(newpath), mp3infow);
		fwprintf(fp, mp3infow->Title);
		fprintf(fp, " | ");
		fwprintf(fp, mp3infow->Artist);
		fprintf(fp, "\n");
		filesNode = filesNode->next;
		for (int count = 36; count < 260; count++){
			newpath[count] = '\0';
		}
		
	}
	sofs--;
	fclose(fp);
	printf("Refresh music library success!\n///////////////////////////////////////////\n");
	return 1;
	
	
}

BOOL schtit(char oldpath[], const char *mlib){
	FILE *fpschtit;
	const char *filepath = strcat(oldpath, mlib);
	if ((fpschtit = fopen(filepath, "r")) == NULL){
		printf("Cannot open music library, strike any key to exit.\n");
		getchar();
		return 0;
	}



	return 0;
}

BOOL schart(char oldpath[], const char *mlib){
	FILE *fpschart;
	const char *filepath = strcat(oldpath, mlib);
	if ((fpschart = fopen(filepath, "r")) == NULL){
		printf("Cannot open music library, strike any key to exit.\n");
		getchar();
		return 0;
	}
	return 0;
}

int  main()
{
	        int order;
orderLOOP:	printf("Please press 1 for refresh the library, 2 for search by title, 3 for search by artists or q to quit.\n");
	        order = getchar();
			fflush(stdin);
			if (order == 'q'){
				return 0;
			}
			else if (order == 49){
				if (!reflib()){
					printf("refresh music library failed. strike anykey to exit.\n");
					getchar();
					exit(1);
				}
				fflush(stdin);
				goto orderLOOP;
					//refresh the library
	        }
	        else if (order == 50){
		        //search by title
				fflush(stdin);
				goto orderLOOP;
	        }
	        else if (order == 51){
		        //search by artists
				fflush(stdin);
				goto orderLOOP;
	        }
	        else{
		        printf("OOPS! please input a number between 1 and 3.\n\n\n");
		        goto orderLOOP;
	        }


	system("pause");
	return 0;
}
