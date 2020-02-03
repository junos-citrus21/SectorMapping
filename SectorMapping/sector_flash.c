#include "sector_include.h"

table* mapping;

void init(const int inclination, int* const megabyte)
{
	int i = 0;
	flash data;
	FILE* fp = fopen("flash_memory.txt", "wb");

	if (fp == NULL)
	{
		printf("파일을 열 수 없습니다.\n");
		return;
	}

	if (inclination <= 0)
	{
		printf("명령 구문이 올바르지 않습니다.\n");
		return;
	}

	//할당할 MB : 섹터 * 블록 * 사용자 의향
	*megabyte = SECTOR * BLOCK * inclination;

	//매핑 테이블 생성
	mapping = (table*)malloc(*megabyte * sizeof(table));

	//기본 매핑 테이블 부여, 물리 섹터 넘버는 아직 저장하지 않았으므로 FACTORY_RESET값 설정
	for (i = 0; i <= *megabyte - 1; i++)
	{
		(mapping + i)->lsn = i;
		(mapping + i)->psn = FACTORY_RESET;
	}

	//전부 hex값을 00으로 초기화합니다.
	memset(data.byte, 0x00, sizeof(data.byte));

	//파일의 처음으로 간다.
	fseek(fp, 0, SEEK_SET);

	//데이터를 파일에 쓴다.
	for (i = 0; i < *megabyte; i++)
	{
		fwrite(&data, sizeof(data), 1, fp);
	}

	printf("%d megabyte(s) flash memory\n", inclination);

	fclose(fp);
}

void flash_read(const int lsn, int* const megabyte)
{
	FILE* fp = fopen("flash_memory.txt", "rb");

	if (fp == NULL)
	{
		printf("파일을 열 수 없습니다.\n");
		return;
	}

	if (lsn <= -1)
	{
		printf("명령 구문이 올바르지 않습니다.\n");
		return;
	}
	else if (lsn > *megabyte - 1)
	{
		printf("%d 섹터가 없습니다.\n", lsn);
		return;
	}
	else if ((mapping + lsn)->psn == FACTORY_RESET)
	{
		printf("입력된 데이터가 없습니다.\n");
		return;
	}

	ftl_read(fp, lsn, megabyte);

	fclose(fp);
}

void flash_write(const int lsn, const char* string, int* const megabyte)
{
	clock_t start = clock();
	clock_t end;
	FILE* fp = fopen("flash_memory.txt", "rb+");

	if (fp == NULL)
	{
		printf("파일을 열 수 없습니다.\n");
		return;
	}

	if (lsn <= -1)
	{
		printf("명령 구문이 올바르지 않습니다.\n");
		return;
	}
	else if (lsn > *megabyte - 1)
	{
		printf("%d 섹터가 없습니다.\n", lsn);
		return;
	}
	
	ftl_write(fp, lsn, string, megabyte);
	end = clock();
	printf("실행 시간 : %f초\n", (float)(end - start) / CLOCKS_PER_SEC);

	fclose(fp);
}

void flash_erase(const int lbn, int* const megabyte)
{
	int i;
	int count = 0;
	flash data;
	FILE* fp = fopen("flash_memory.txt", "rb+");

	if (lbn <= -1)
	{
		printf("명령 구문이 올바르지 않습니다.\n");
		return;
	}
	else if (lbn > (*megabyte - 1) / SECTOR)
	{
		printf("%d 블록이 없습니다.\n", lbn);
		return;
	}

	memset(data.byte, 0x00, sizeof(data.byte));

	//처음에서 number만큼 블록을 이동한다.
	fseek(fp, SECTOR * lbn * sizeof(data), SEEK_SET);

	for (i = 0; i <= SECTOR - 1; i++)
	{
		fwrite(&data, sizeof(data), 1, fp);
		count++;
	}

	printf("%dth block erase, 횟수 : %d회\n", lbn, count);

	fclose(fp);
}

void ftl_read(FILE* fp, const int lsn, int* const megabyte)
{
	int i;
	flash data;

	//구조체 초기화
	memset(data.byte, 0x00, sizeof(data.byte));

	//처음에서 lsn만큼 섹터를 이동한다.
	fseek(fp, (mapping + lsn)->psn * sizeof(data), SEEK_SET);

	//구조체를 읽어온다.
	fread(&data, sizeof(data), 1, fp);
	printf("오프셋 : %d \n16진수 : ", (mapping + lsn)->psn);

	for (i = 0; i <= sizeof(data.byte) - 1; i++)
	{
		printf("%2X ", data.byte[i]);
	}

	printf("\n문자열 : ");

	for (i = 0; i <= sizeof(data.byte) - 1; i++)
	{
		printf("%2c ", data.byte[i]);
	}

	printf("\n");
}

void ftl_write(FILE* fp, const int lsn, const char* string, int* const megabyte)
{
	int i, j;
	int type = BASIC;
	int indicator;
	int count = 0;
	int check = 0; //옮겨쓴 횟수 저장
	int next_block_lsn = 0; //옮겨 쓸 블록
	const int limit = (int)(lsn / SECTOR) + 1; //섹터가 다른 블록으로 넘어까지 않도록 제한 설정
	flash data;

	fseek(fp, lsn * sizeof(data), SEEK_SET);
	fread(data.byte, sizeof(data), 1, fp);

	//변수 indicator는 논리 섹터 넘버가 현재 가리키는 위치 지시자이다.
	for (indicator = lsn; (mapping + lsn)->psn <= limit * SECTOR; indicator++)
	{
		count++;

		//한 블록에서 모든 섹터가 전부 사용했을경우
		if (count == SECTOR)
		{
			//복사용 빈 블록을 처음부터 차례대로 검사한다.
			for (i = 0; i <= *megabyte - 1; i++)
			{
				//같은 블록이면 건너 뜀
				if (lsn / SECTOR == i)
				{
					continue;
				}

				count = 0;

				//한 블록에서 빈 섹터 검사하는 구역
				for (j = 0; j <= SECTOR - 1; j++)
				{
					fseek(fp, (i*SECTOR + j) * sizeof(data), SEEK_SET);
					fread(data.byte, sizeof(data), 1, fp);

					if (data.byte[0] == 0x00)
					{
						count++;
						continue;
					}
					else
					{
						break;
					}
				}

				//이 블록을 복사할 블록으로 설정한다.
				if (count == SECTOR)
				{
					next_block_lsn = i * SECTOR;

					break;
				}
			}

			count = 0;

			//빈 블록을 설정한 곳으로 복사한다.
			for (indicator = (limit - 1)*SECTOR; indicator <= limit * SECTOR - 1; indicator++)
			{
				count++;
				//예전 데이터를 읽어온다.
				fseek(fp, (mapping + indicator)->psn * sizeof(data), SEEK_SET);
				fread(data.byte, sizeof(data), 1, fp);

				if (data.byte[0] == 0x00)
				{
					continue;
				}

				//NULL값이 아니면 check값 +1, 새로운 블록으로 데이터를 복사한다.
				check++;
				fseek(fp, (mapping + next_block_lsn)->lsn * sizeof(data), SEEK_SET); //다음 블록으로 순서대로 이동
				fwrite(data.byte, sizeof(data), 1, fp); //다음 블록으로 복사
				(mapping + indicator)->psn = (mapping + next_block_lsn)->lsn;
				next_block_lsn++;

				if (count == SECTOR)
				{
					break;
				}
			}

			type = SHIFT;
			break;
		}
		//한 블록에서 모든 섹터를 사용 안했을 경우
		else
		{
			if (indicator >= limit * SECTOR)
			{
				indicator = (limit - 1)*SECTOR;
			}

			fseek(fp, indicator * sizeof(data), SEEK_SET);
			fread(data.byte, sizeof(data), 1, fp);

			//데이터가 기록되어있을 경우 다음 섹터에 쓰고, 다음 블록 안 넘을때 까지 쓴다.
			if (data.byte[0] != 0x00)
			{
				continue;
			}
			else
			{
				(mapping + lsn)->psn = indicator; //매핑테이블 최신
				//TODO : 옮겨쓰고 나서, 같은 섹터에 데이터를 쓸때, 기존데이터는 지우고, 매핑테이블을 -1로 바꾼다. 
				type = BASIC;

				break;
			}
		}
	}

	//명령에서 입력한 데이터 넣는 구역
	if (type == SHIFT)
	{
		fseek(fp, next_block_lsn * sizeof(data), SEEK_SET);
		fwrite(string, strlen(string), 1, fp);
		(mapping + indicator)->psn = (mapping + next_block_lsn)->lsn;

		//이전 블록은 지운다.
		flash_erase(lsn / SECTOR, megabyte);
	}
	else if (type == BASIC)
	{
		fseek(fp, indicator * sizeof(data), SEEK_SET);
		fwrite(string, strlen(string), 1, fp);
	}

	check++;
	printf("done, 쓰기 : %d회\n", check);
}

void print_table(const int inclination, int* const megabyte)
{
	//이 함수는 기본 바이트 크기가 16바이트로 설정되어 있다.
	assert(MAX / SIZE == 16);

	int i;

	if (inclination <= -1 || inclination >= *megabyte / SECTOR)
	{
		printf("값이 올바르지 않습니다.\n");

		return;
	}

	printf("| %4d번 블록 매핑 테이블                        |\n", inclination);
	printf("| %10s %10s || %10s %10s |\n", "lsn", "psn", "lsn", "psn");

	for (i = inclination * SECTOR; i <= (inclination + 1) * SECTOR - 1 - 16; i++)
	{
		printf("| %10d %10d || %10d %10d |\n", mapping[i].lsn, mapping[i].psn, mapping[i + 16].lsn, mapping[i + 16].psn);
	}
}