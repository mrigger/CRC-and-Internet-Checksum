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
	int error;
	frameList next;
} crcFrame;

typedef enum CRC_MODE { CRC1, CRC8, CRC16, CRC32, InternetChecksum, Error} MODE;

frameList startframe = NULL;

// �Լ� ����
MODE selectMode();
int selectProb();
void Tx(MODE mode);
void Rx(MODE mode);
uint8_t calcCRC1(uint8_t M[]);
uint8_t calcCRC8(uint8_t M[]);
uint16_t calcCRC16(uint8_t M[]);
uint32_t calcCRC32(uint8_t M[]);
uint16_t calcInternetChecksum(uint8_t M[]);

int main() {
	MODE mode;
	srand((unsigned int)time(NULL));
	
	mode = selectMode();	// ���� ��� �Է�
	if (mode == Error) return -1;
	Tx(mode);				// ����
	Rx(mode);				// ����

	return 0;
}

MODE selectMode() {
	MODE mode;
	int input;

	printf("<<< CRC and Internet Checksum >>>\n\n");
	printf("[ Select mode ]\n");
	printf("[1] CRC1  [2] CRC8  [3] CRC16  [4] CRC32 [5] InternetChecksum\n\n");
	printf("Mode: ");
	scanf("%d", &input);

	switch (input) {
	case 1: mode = CRC1; break;
	case 2: mode = CRC8; break;
	case 3: mode = CRC16; break;
	case 4: mode = CRC32; break;
	case 5: mode = InternetChecksum; break;
	default: printf("Incorrect input value\n"); return Error;
	}

	return mode;
}

int selectProb() {
	int input, prob;

	printf("\n[ Select probability ]\n");
	printf("[1] 0.1  [2] 0.001  [3] 0.00001  [4] 0.000000001\n\n");\
	printf("Mode: ");
	scanf("%d", &input);

	switch (input) {
	case 1: prob = 10; break;
	case 2: prob = 1000; break;
	case 3: prob = 100000; break;
	case 4: prob = 1000000000; break;
	default: printf("Incorrect input value [default: 0.1]\n"); prob = 10; break;
	}

	return prob;
}

void Tx(MODE mode) {
	uint8_t M[MAX_LENGTH];
	uint32_t crcValue;
	char* fileEnd;
	int lineLength;
	int line = 0;
	int crcLength;
	int noiseLevel;
	int error = 0;
	frameList newframe, curr;
	FILE *fp = fopen("sample.txt", "rb");
	
	noiseLevel = selectProb();
	while (1) {
		error = 0;
		memset(M, '\0', MAX_LENGTH);
		fileEnd = fgets(M, MAX_LENGTH, fp);		// �� ������ ���ڿ� �Է�
		if (!fileEnd) break;

		lineLength = strlen(M);
		M[lineLength - 1] = '\0';	// \n ����
		M[lineLength - 2] = '\0';	// \r ����
		lineLength -= 2;

		if (mode == CRC1) { crcValue = calcCRC1(M); crcLength = 1; }
		else if (mode == CRC8) { crcValue = calcCRC8(M); crcLength = 8; }
		else if (mode == CRC16) { crcValue = calcCRC16(M); crcLength = 16; }
		else if (mode == CRC32) { crcValue = calcCRC32(M); crcLength = 32; }
		else { crcValue = calcInternetChecksum(M); crcLength = 16; }

		// �Է� ��Ʈ��Ʈ�� ���� ����
		/*
		for (i = 0; i < lineLength; i++) {
			for (j = 7; j >= 0; j--) {
				prob = (int)(rand() % noiseLevel);
				if (prob == 1) {
					// printf("Line bit Error. [%d] \n", line);
					M[i] ^= (1 << j);
					error = 1;
				}
			}
		}
		*/
		/*
		// CRC �� ��������
		for (i = 0; i < crcLength; i++) {
			prob = (int)(rand() % noiseLevel);
			if (prob == 1) {
				//printf("CRC bit Error. [%d] \n", line);
				crcValue ^= (1 << i);
			}
		}
		*/

		// ����Ʈ�� ����
		if (!startframe) {
			startframe = (frameList)malloc(sizeof(crcFrame));
			strcpy(startframe->load, M);
			startframe->crcValue = crcValue;
			startframe->error = error;
			startframe->next = NULL;
			curr = startframe;
		}
		else {
			newframe = (frameList)malloc(sizeof(crcFrame));
			strcpy(newframe->load, M);
			newframe->crcValue = crcValue;
			newframe->error = error;
			newframe->next = NULL;
			curr->next = newframe;
			curr = curr->next;
		}
		line++;
	}

	printf("\n");
	fclose(fp);
}

void Rx(MODE mode) {
	uint32_t crcValue;
	uint8_t check[MAX_LENGTH];
	frameList curr;
	int line;

	curr = startframe;
	line = 0;
	int totalErr = 0;
	int realErr;
	int foundErr;
	int incorrect = 0;
	int notFound = 0;
	int result = 0;
	int errorline = 0;

	while (curr->next) {
		realErr = 0;
		foundErr = 0;
		memset(check, '\0', MAX_LENGTH);
		if (mode == CRC1) {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)(curr->crcValue << 7);
		}
		else if (mode == CRC8) {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)curr->crcValue;
		}
		else if (mode == CRC16) {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 8);
			check[strlen(check)] = (uint8_t)(curr->crcValue);
		}
		else if (mode == CRC32) {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 24);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 16);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 8);
			check[strlen(check)] = (uint8_t)(curr->crcValue);
		}
		else {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 8);
			check[strlen(check)] = (uint8_t)(curr->crcValue);
		}

		if (mode == CRC1) {
			crcValue = calcCRC1(check);
			if (curr->error == 1) {
				realErr = 1;
				printf("R: ERROR [%d] ", line);
				totalErr++;
			}
		}
		else if (mode == CRC8) {
			crcValue = calcCRC8(check);
			if (curr->error == 1) {
				realErr = 1;
				printf("R: ERROR [%d] ", line);
				totalErr++;
			}
		}
		else if (mode == CRC16) {
			crcValue = calcCRC16(check);
			if (curr->error == 1) {
				realErr = 1;
				printf("R: ERROR [%d] ", line);
				totalErr++;
			}
		}
		else if (mode == CRC32) {
			crcValue = calcCRC32(check);
			if (curr->error == 1) {
				realErr = 1;
				printf("R: ERROR [%d] ", line);
				totalErr++;
			}
		}
		else {
			crcValue = calcInternetChecksum(check);
			if (curr->error == 1) {
				realErr = 1;
				printf("R: ERROR [%d] ", line);
				totalErr++;
			}
		}

		if (crcValue != 0) {
			foundErr = 1;
			if(realErr == 0) printf("R: GOOD  [%d] [%s] ", line, curr->load);
			printf("F: ERROR [%d] ", line);
			errorline++;
		}

		if (realErr || foundErr) printf("\n");
		if (realErr && foundErr) result++;
		if (!realErr && foundErr) incorrect++;
		if (realErr && !foundErr) notFound++;

		line++;
		curr = curr->next;
	}

	printf("\n");
	printf("Total error line : %d\n", totalErr);
	printf("��Ȯ�ϰ� ã�� ���� : %d\n", result);
	printf("������ �ƴѵ� ������ ã�� ���� (CRC value error): %d\n", incorrect);
	printf("�����ε� �� ã�� ���� : %d\n", notFound);
	printf("Check error line : %d\n", errorline);
}

uint8_t calcCRC1(uint8_t M[]) {
	uint8_t crcValue = 0;
	uint8_t crcCalc = 0;
	int i, j;
	int input;
	int c0;
	int stringSize = strlen(M);

	for (i = 0; i < stringSize; i++)
	{
		input = 0;
		crcCalc = 0;
		for (j = 7; j >= 0; j--)
		{
			if ((M[i] & (1 << j))) 
				input = 1;
			else
				input = 0;

			c0 = ((crcValue & 0x01) ^ input); // c0 = c0 xor input

			crcCalc = c0;

			crcValue <<= 1;
			crcValue %= 2;

			crcValue &= 0;
			crcValue ^= crcCalc;
		}
	}

	return crcValue;
}

uint8_t calcCRC8(uint8_t M[]) {
	uint8_t crcValue = 0;
	uint8_t crcCalc = 0;
	int i, j;
	int input;
	int c0, c2, c1;
	int stringSize = strlen(M);

	for (i = 0; i < stringSize; i++)
	{
		input = 0;
		crcCalc = 0;
		for (j = 7; j >= 0; j--)
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
			crcValue = crcValue % 0x100;  // 1 0000 0000

			// ���� �־��ֱ� ���� 0, 1, 2��° reset (1111_1000)
			// ���� ���� ����
			crcValue &= 0xF8;
			crcValue ^= crcCalc;
		}
	}

	return crcValue;
}

uint16_t calcCRC16(uint8_t M[]) {
	uint16_t crcValue = 0;
	uint16_t crcCalc = 0;
	int i, j;
	int input;
	int c0, c8, c15;
	int stringSize = strlen(M);

	for (i = 0; i < stringSize; i++)
	{
		input = 0;
		crcCalc = 0;
		for (j = 7; j >= 0; j--)
		{
			if ((M[i] & (1 << j)))
				input = 1;
			else
				input = 0;

			c0 = (((crcValue >> 15) & 1) ^ input);				// c0 = c15 xor input
			c15 = c0 ^ ((crcValue >> 14) & 1);					// c15 = c15 xor input xor C14
			c8 = c0 ^ ((crcValue >> 7) & 1);;					// c8 = c15 xor input xor c7

			crcCalc = c0 + (c8 << 8) + (c15 << 15);

			crcValue = crcValue << 1;
			crcValue %= 0x10000;

			// ���� �־��ֱ� ���� 0, 8, 15��° reset (0111_1110_1111_1110)
			crcValue &= 0x7EFE;
			crcValue ^= crcCalc;
		}
	}
	return crcValue;
}

uint32_t calcCRC32(uint8_t M[]) {
	uint32_t crcValue = 0;
	uint32_t crcCalc = 0;
	int i, j;
	int input;
	int stringSize = strlen(M);
	uint32_t c26, c23, c22, c16, c12, c11, c10, c8, c7, c5, c4, c2, c1, c0;

	for (i = 0; i < stringSize; i++)
	{
		crcCalc = 0;
		for (j = 7; j >= 0; j--)
		{
			if ((M[i] & (1 << j))) 
				input = 1;
			else
				input = 0;

			c0 = (((crcValue & 0x80000000) >> 31) ^ input);		// c0 = c31 xor input
			c26 = c0 ^ ((crcValue & 0x02000000) >> 25);			// c26 = c31 xor input xor c25
			c23 = c0 ^ ((crcValue & 0x00400000) >> 22);			// c23 = c31 xor input xor c22
			c22 = c0 ^ ((crcValue & 0x00200000) >> 21);			// c22 = c31 xor input xor c21
			c16 = c0 ^ ((crcValue & 0x00008000) >> 15);			// c16 = c31 xor input xor c15
			c12 = c0 ^ ((crcValue & 0x00000800) >> 11);			// c12 = c31 xor input xor c11
			c11 = c0 ^ ((crcValue & 0x00000400) >> 10);			// c11 = c31 xor input xor c10
			c10 = c0 ^ ((crcValue & 0x00000200) >> 9);			// c10 = C31 xor input xor c9
			c8 = c0 ^ ((crcValue & 0x00000080) >> 7);			// c8 = c31 xor input xor c7
			c7 = c0 ^ ((crcValue & 0x00000040) >> 6);			// c7 = c31 xor input xor c6
			c5 = c0 ^ ((crcValue & 0x00000010) >> 4);			// c5 = c31 xor input xor c4
			c4 = c0 ^ ((crcValue & 0x00000008) >> 3);			// c4 = c31 xor input xor c3
			c2 = c0 ^ ((crcValue & 0x00000002) >> 1);			// c2 = c31 xor input xor c1
			c1 = c0 ^ (crcValue & 0x00000001);					// c1 = c31 xor input xor c0

			crcCalc = c0 + c26 * 0x4000000 + c23 * 0x800000 + c22 * 0x400000 + c16 * 0x10000 + c12 * 0x1000 + c11 * 0x800
				+ c10 * 0x400 + c8 * 0x100 + c7 * 0x80 + c5 * 0x20 + c4 * 0x10 + c2 * 0x04 + c1 * 0x02;
			crcValue <<= 1;
			crcValue %= 0x100000000;

			// ���� �־��ֱ� ���� 0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26��° reset (1111_1011_0011_1110_1110_0010_0100_1000)
			crcValue &= 0xFB3EE248;
			crcValue ^= crcCalc;
		}
	}

	return crcValue;
}

uint16_t calcInternetChecksum(uint8_t M[]) {
	uint32_t checksum = 0;
	uint16_t word1 = 0;
	uint16_t word2 = 0;
	int i;
	int stringSize = strlen(M);

	for (i = 0; i < stringSize; i = i + 2) {
		word1 = (uint16_t)checksum;
		word2 = M[i];
		word2 <<= 8;
		word2 += M[i + 1];

		checksum = word1 + word2;

		if (checksum / 0x10000) {
			checksum %= 0x10000;
			checksum += 1;
		}
	}
	
	return (uint16_t)(0xFFFF ^ checksum);
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

