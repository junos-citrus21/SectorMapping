#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define SHIFT (-3)
#define BASIC (-2)
#define FACTORY_RESET (-1) //SSD용 NULL값
#define FALSE (0)
#define TRUE (1)
#define SECTOR (32) //1개의 블록은 32개 섹터로 구성
#define BLOCK (64) //32 * 64 * 512 = 1048576바이트다. 즉 계산하면 1MB이다.
#define MAX (512) //기본 논리적 섹터 크기는 512 bytes이다.
#define SIZE (32) //하지만 512 bytes로 하면 파일의 크기가 너무 크므로, 임의로 줄인다.

typedef struct _flash {
	char byte[MAX / SIZE]; //기본 바이트 크기
} flash;

typedef struct _table {
	int lsn; //논리 섹터 넘버
	int psn; //물리 섹터 넘버
} table;

void init(const int inclination);
void flash_read(int lsn);
void flash_write(int lsn, char* string);
void flash_erase(int lbn);
void ftl_read(FILE* fp, int lsn);
void ftl_write(FILE* fp, int lsn, char* string);
void print_table(const int inclination);
void help();
