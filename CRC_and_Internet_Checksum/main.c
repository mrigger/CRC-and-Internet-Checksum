#pragma warning(disable: 4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#define MAX_LENGTH 1024

typedef struct crcframe* frameList;
typedef struct crcframe {
	uint8_t load[MAX_LENGTH];
	uint32_t crcValue;
	frameList next;
} crcFrame;

typedef enum CRC_MODE { CRC1, CRC5, CRC8, CRC16, CRC32, InternetChecksum} MODE;

// �Լ� ����
uint8_t calcCRC1(uint8_t M[]);
uint8_t calcCRC8(uint8_t M[]);
uint16_t calcCRC16(uint8_t M[]);
uint32_t calcCRC32(uint8_t M[]);

int main() {
	// �۽ź�
	char fileEnd;
	uint8_t origin[MAX_LENGTH];
	uint8_t M[MAX_LENGTH];
	uint32_t crcValue;
	MODE mode;
	int lineLength;
	int crcLength;
	int i, j;
	int noiseLevel, prob, line = 0;
	frameList startframe = NULL, newframe, curr;
	frameList originframe = NULL, orignew, origcurr;

	// ���ź�
	uint8_t check[MAX_LENGTH];

	FILE *fp = fopen("sample.txt", "rb");
	
	srand((unsigned int)time(NULL));

	mode = CRC8;

	// �۽ź�
	while (1) {
		memset(M, '\0', MAX_LENGTH);
		fileEnd = fgets(M, MAX_LENGTH, fp);		// �� ������ ���ڿ� �Է�
		if (!fileEnd) break;

		lineLength = strlen(M);
		M[lineLength - 1] = '\0';	// \n ����
		M[lineLength - 2] = '\0';	// \r ����
		strcpy(origin, M);

		if (mode == CRC1) {}
		else if (mode == CRC8) { crcValue = calcCRC8(M); crcLength = 8; }
		else if (mode == CRC16) {}
		else {}

		noiseLevel = 10;	// 0.001

		// �Է� ��Ʈ��Ʈ�� ���� ����
		for (i = 0; i < lineLength; i++) {
			for (j = 7; j >= 0; j--) {
				prob = (int)(rand() % noiseLevel);
				if (prob == 1) {
					printf("Line bit Error. [%d] \n", line);
					M[i] ^= (1 << j);
				}
			}
		}

		// CRC �� ��������
		for (i = 0; i < crcLength; i++) {
			prob = (int)(rand() % noiseLevel);
			if (prob == 1) {
				printf("CRC bit Error. [%d] \n", line);
				crcValue ^= (1 << i);
			}
		}

		// ����Ʈ�� ����
		if (!startframe) {
			startframe = (frameList)malloc(sizeof(crcFrame));
			strcpy(startframe->load, M);
			startframe->crcValue = crcValue;
			startframe->next = NULL;
			curr = startframe;

			originframe = (frameList)malloc(sizeof(crcFrame));
			strcpy(originframe->load, origin);
			originframe->crcValue = 0;
			originframe->next = NULL;
			origcurr = originframe;
		}
		else {
			newframe = (frameList)malloc(sizeof(crcFrame));
			strcpy(newframe->load, M);
			newframe->crcValue = crcValue;
			newframe->next = NULL;
			curr->next = newframe;
			curr = curr->next;

			orignew = (frameList)malloc(sizeof(crcFrame));
			strcpy(orignew->load, origin);
			orignew->crcValue = 0;
			orignew->next = NULL;
			origcurr->next = orignew;
			origcurr = origcurr->next;
		}
		line++;
	}

	printf("\n");

	// ���ź�
	curr = startframe;
	origcurr = originframe;
	line = 0;
	int totalErr = 0;
	int realErr;
	int foundErr;
	int incorrect = 0;
	int notFound = 0;

	while(curr->next) {
		realErr = 0;
		foundErr = 0;
		memset(check, '\0', MAX_LENGTH);
		if (mode == CRC1) {}
		else if (mode == CRC8) {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)curr->crcValue;
		}
		else if (mode == CRC16) {}
		else {}

		if (mode == CRC8) {
			crcValue = calcCRC8(check);
			if (strcmp(origcurr->load, curr->load)) {
				realErr = 1;
				printf("R: Miss correct. [%d] --- ", line);
				totalErr++;
			}
		}

		if (crcValue != 0) {
			foundErr = 1;
			printf("F: Miss correct. [%d]", line);
		}

		if (realErr || foundErr) printf("\n");
		if (!realErr && foundErr) incorrect++;
		if (realErr && !foundErr) notFound++;

		line++;
		curr = curr->next;
		origcurr = origcurr->next;
	}

	printf("Check error line : %d\n", totalErr - incorrect - notFound);
	printf("Total error line : %d\n", totalErr);
	printf("�߸� ã�� ���� : %d\n", incorrect);
	printf("�� ã�� ���� : %d\n", notFound);

	fclose(fp);
	return 0;
}

uint8_t calcCRC8(uint8_t M[])
{
	uint8_t crcValue = 0;
	uint8_t crcCalc = 0;
	int input;
	int c0, c2, c1;
	int stringSize = strlen(M);

	for (int i = 0; i < stringSize; i++)
	{
		input = 0;
		crcCalc = 0;
		for (int j = 7; j >= 0; j--)
		{
			if ((M[i] & (1 << j)))
				input = 1;
			else
				input = 0;

			c0 = (((crcValue & 0x80) >> 7) ^ input);	// c0 = c7 xor input
			c2 = c0 ^ ((crcValue & 0x02) >> 1);			// c2 = c7 xor input xor c1
			c1 = c0 ^ ((crcValue & 0x01));				// c1 = c7 xor input xor c0 

			crcCalc = c0 + (c1 << 1) + (c2 << 2);

			crcValue = crcValue << 1;
			crcValue = crcValue % 256;

			// ���� �־��ֱ� ���� 0, 1, 2��° reset (1111_1000)
			// ���� ���� ����
			crcValue &= 0xF8;
			crcValue ^= crcCalc;
		}
	}

	return crcValue;
}

/*
	[����]

	1.	long text file�� noisy channel���� �����ϴ� ���� simulate
		CRC �˰����� ��� �۵��ϰ� performance�� ��� Ȯ��.

	1)	sample.txt��� ( �� ���θ��� CRC algorithm�� input���� ���Ǵ� frame�� ���� )
	
	2)	H/W Logic���� ������ CRC Algorithm�� ���
		�� standard CRC algorithm�� ���� CRC checksum values�� ���
	
	3)	������ noisy channel���� CRC value�� ������ �Ǵµ�, 0.1�� bit_error���� ����
		CRC Algorithm�� �����ؼ� ���ŵ� �� �������� ������ �߻��Ͽ����� �߻����� �ʾҴ����� ����
		�������� ������ accuracy�� check�ϱ� ���� ���� frame�� original frame�� ���ϰ� accuracy�� ���Ͽ���.
		�� accuracy�� ���� ���۵� �������� ������ ������ �� ���� Ȯ���� ���� �� �ִ�.
	
	4)	�̷������� ���������� ������ �����ϴ� �������� Ȯ���� �����ϰ�,
		������ �������� �ʴ� �������� Ȯ���� �����ϰ�,
		������ �� ���� ������ �����ϴ� �������� Ȯ���� ����.
		�� ����� ������ ���� ����� ��

	5)	�� ������ �ݺ�. �����ӿ��� ������ �Ͼ Ȯ���� 0.001, 0.00001�� �����Ͽ� ����
		( �������� �̴��� ���̺��� ������ �Ͼ Ȯ�� = 0.000000001 )

	6)	�� ���� ����� ���� observation�� comments�� �Ͻÿ�.

	2.	CRC�� ����� Internet checksum algorithm�� ����Ͽ� ���� ������ �Ȱ��� �����ϰ�
		CRC�� Internet checksum algorithm�� ��
*/

/*
	<< set of standard CRC algorithm >>

	CRC-1 : x + 1
	CRC-8-CCITT : x^8 + x^2 + x + 1
	CRC-16-CCITT : x^16 + x^15 + x^8 + 1
	CRC-32-IEEE : x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
*/

