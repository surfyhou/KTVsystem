#include   <stdiog.h> 
#include   <dirent.h>
#include   <string.h> 
#include   <windows.h>

#define MAX 100 //max num of music
#define NOTFOUND 0
#define FOUND 1
#define _ERROR -1

struct titleandartists{
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

wchar_t* c2w(const char *str)
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

BOOL reflib(char oldpath[], const char *mlib){
	//library path changing
	FILE *fp;
	int _sofs = 1;//sum of songs	
	int len_of_op = strlen(oldpath);
	printf("Refresh library starts.\n");
	printf("Music library is at %s\nIf you want to change it, press 'y', or anykey else to continue\n", oldpath);
	if (getchar() == 'y'){
		scanf_s("%s", &oldpath, 261);
		len_of_op = strlen(oldpath);
	}
	//library path changing ends
	MP3INFOW *mp3infow = (MP3INFOW *)malloc(sizeof(MP3INFOW));
	FileList *filesNode;
	filesNode = getFiles(oldpath);

	if (filesNode == NULL)
	{
		printf("get file failed\n");
		return 0;
	}

	const char *filepath = strcat(oldpath, mlib);

	if ((fp = fopen(filepath, "r+")) == NULL){
		printf("Cannot open music library,strike any key to exit.\n");
		getchar();
		exit(1);
	}
	char newpath[260] = "C:\\Users\\qq160_000\\Desktop\\musicfortest\\";

	char *sofs = (char*)malloc(sizeof(char)* MAX);
	filesNode = filesNode->next->next;
	while (filesNode)
	{
		strcat(newpath, filesNode->filename);
		if ((filesNode->filename) != "zmusiclib.txt" && (GetMp3Info(c2w(newpath), mp3infow) == TRUE)) {
			_itoa(_sofs, sofs, 10);
			fprintf(fp, sofs);
			fwprintf(fp, mp3infow->Title);
			fprintf(fp, "|");
			fwprintf(fp, mp3infow->Artist);
			fprintf(fp, "\n");
			for (int count = len_of_op; count < 260; count++){
				newpath[count] = '\0';
			}
		}
		filesNode = filesNode->next;
		_sofs++;

	}
	//sofs--;
	//fwprintf(fp, mp3infow->Artist);
	fclose(fp);
	printf("Refresh music library success!\n///////////////////////////////////////////\n");
	return 1;


}

BOOL search(char str[], char strf[]){
	int i, j, flag = 0;
	for (j = 0; j < strlen(str); j++){
		if (str[j] == strf[0]){
			for (i = 1; i < strlen(strf) && str[i + j] == strf[i]; i++){
				if (i == strlen(strf) - 1) {
					flag++;
					return FOUND;
				}
			}
		}
	}
	if (flag == 0) return NOTFOUND;
	else return _ERROR;

}

BOOL schtit(char oldpath[], const char *mlib){
	bool flag_find = NOTFOUND;
	FILE *fpschtit;
	int count = 0;
	char str_to_temp[63];
	int got[MAX] = { 0 };
	char tosch_tit[31];
	const char *filepath = strcat(oldpath, mlib);

	printf("Please input the title to search musics.(completely title)\n");
	scanf("%s", &tosch_tit);
	if ((fpschtit = fopen(filepath, "r")) == NULL){
		printf("Cannot open music library, strike any key to exit.\n");
		getchar();
		exit(1);
	}
	int i_temp = 1;
	for (count = 1; !feof(fpschtit); count++) {
		fgets(str_to_temp, 63, fpschtit);
		if (search(str_to_temp, tosch_tit)){
			printf("Found the music:%s\n", tosch_tit);
			flag_find = FOUND;
		}
	}
	if (flag_find){
		//play that music and roll lysics.
		//play();
		//rolllysic();
	}
	else if (!flag_find){
		printf("Not found, press any key to the menu.\n");
		getchar();
		return 1;
	}
	else{
		printf("Unknown Error! press any key to exit!\n");
		getchar();
		exit(1);
	}
	//insert code here
	return 0;
}

BOOL schart(char oldpath[], const char *mlib){
	FILE *fpschart;
	int count = 0;
	bool flag_find = NOTFOUND;
	char str_to_temp[63];
	int got[MAX] = { 0 };
	char tosch_art[31];
	const char *filepath = strcat(oldpath, mlib);

	printf("Please input the artist's name to search his/her musics.\n");
	scanf("%s", &tosch_art);
	if ((fpschart = fopen(filepath, "r")) == NULL){
		printf("Cannot open music library, strike any key to exit.\n");
		getchar();
		exit(1);
	}
	
	for (count = 1; !feof(fpschart); count++) {
		fgets(str_to_temp, 63, fpschart);
		if (search(str_to_temp, tosch_art)){
			got[str_to_temp[0]-48]++;
			printf("%c:", str_to_temp[0]);
			for (int i = 1; str_to_temp[i] != '|'; i++) {
				printf("%c",str_to_temp[i]);
			}
			printf("\n");
			flag_find = FOUND;
		}
	}
	if (flag_find){
		printf("Please select the music you wanna listen.(input number)\n");
		int sign_of_music = 0;
		scanf("%d", &sign_of_music);
		//play that music and roll lysics.
		//play();
		//rolllysic();
		return 0;
	}
	else if (!flag_find){
		printf("Not found, press any key to the menu.\n");
		getchar();
		return 1;
	}
	else{
		printf("Unknown Error! press any key to exit!\n");
		getchar();
		exit(1);
	}
}

int  main(int argc, char argv[])
{
	int order;
	char oldpath[260] = "C:\\Users\\qq160_000\\Desktop\\musicfortest\\";
	const char *mlib = "zmusiclib.txt";
orderLOOP:	printf("Please press:\n1 for refresh the library.\n2 for search by title.\n3 for search by artists.\nor any other key to quit.\n");
	order = getchar();
	fflush(stdin);
	if (order == 49){
		reflib(oldpath, mlib);
		fflush(stdin);
		printf("///////////////////////////////////////\n");
		goto orderLOOP;
		//refresh the library
	}
	else if (order == 50){
		//search by title
		schtit(oldpath, mlib);
		fflush(stdin);
		printf("///////////////////////////////////////\n");
		goto orderLOOP;
	}
	else if (order == 51){
		//search by artists
		schart(oldpath, mlib);
		fflush(stdin);
		printf("///////////////////////////////////////\n");
		goto orderLOOP;
	}
	else{
		return 0;

	}


	system("pause");
	return 0;
}
