#include "sector_include.h"

void help()
{
	printf("+--------------------------------------+\n");
	printf("| 명령어 도움말            <섹터 매핑> |\n");
	printf("+--------------------------------------+\n");
	printf("| 1. 메모리 생성 : init 정수 (MB)      |\n");
	printf("| 2. 섹터 읽기 : read 정수             |\n");
	printf("| 3. 섹터 쓰기 : write 정수 문자열     |\n");
	printf("| 4. 블록 삭제 : erase 정수            |\n");
	printf("| 5. 블록 매핑 테이블 출력 : print 정수|\n");
	printf("| 6. 도움말 : help                     |\n");
	printf("| 7. 종료 : exit                       |\n");
	printf("+--------------------------------------+\n");
}

int main(void)
{
	int psn;
	int megabyte = 0;
	char command[SIZE];
	char string[MAX / SIZE + 1];
	
	help();

	while (TRUE)
	{
		printf(">>>");
		scanf(" %s", command);

		if (strcmp(command, "init") == 0 || strcmp(command, "i") == 0)
		{
			scanf("%d", &psn);
			init(psn, &megabyte);
		}
		else if (strcmp(command, "read") == 0 || strcmp(command, "r") == 0)
		{
			scanf("%d", &psn);
			flash_read(psn, &megabyte);
		}
		else if (strcmp(command, "write") == 0 || strcmp(command, "w") == 0)
		{
			scanf("%d ", &psn);
			scanf("%s", string);
			flash_write(psn, string, &megabyte);
		}
		else if (strcmp(command, "erase") == 0 || strcmp(command, "e") == 0)
		{
			scanf("%d", &psn);
			flash_erase(psn, &megabyte);
		}
		else if (strcmp(command, "print") == 0 || strcmp(command, "p") == 0)
		{
			scanf("%d", &psn);
			print_table(psn, &megabyte);
		}
		else if ((strcmp(command, "help") == 0)) help();
		else if ((strcmp(command, "exit") == 0) || (strcmp(command, "quit") == 0)) break;
		else printf("잘못된 명령입니다.\n");
	}

	return 0;
}
