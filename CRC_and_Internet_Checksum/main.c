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

// 함수 선언
uint8_t calcCRC1(uint8_t M[]);
uint8_t calcCRC8(uint8_t M[]);
uint16_t calcCRC16(uint8_t M[]);
uint32_t calcCRC32(uint8_t M[]);

int main() {
	// 송신부
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

	// 수신부
	uint8_t check[MAX_LENGTH];

	FILE *fp = fopen("sample.txt", "rb");
	
	srand((unsigned int)time(NULL));
	mode = CRC32;

	// 송신부
	while (1) {
		memset(M, '\0', MAX_LENGTH);
		fileEnd = fgets(M, MAX_LENGTH, fp);		// 한 라인의 문자열 입력
		if (!fileEnd) break;

		lineLength = strlen(M);
		M[lineLength - 1] = '\0';	// \n 삭제
		M[lineLength - 2] = '\0';	// \r 삭제
		lineLength -= 2;

		strcpy(origin, M);

		if (mode == CRC1) { crcValue = calcCRC1(M); crcLength = 1; }
		else if (mode == CRC8) { crcValue = calcCRC8(M); crcLength = 8; }
		else if (mode == CRC16) { crcValue = calcCRC16(M); crcLength = 16; }
		else if (mode == CRC32) { crcValue = calcCRC32(M); crcLength = 32; }

		noiseLevel = 1000;	// 0.001

		// 입력 비트스트림 에러 생성
		for (i = 0; i < lineLength; i++) {
			for (j = 7; j >= 0; j--) {
				prob = (int)(rand() % noiseLevel);
				if (prob == 1) {
					//printf("Line bit Error. [%d] \n", line);
					M[i] ^= (1 << j);
				}
			}
		}

		// CRC 값 에러생성
		for (i = 0; i < crcLength; i++) {
			prob = (int)(rand() % noiseLevel);
			if (prob == 1) {
				//printf("CRC bit Error. [%d] \n", line);
				crcValue ^= (1 << i);
			}
		}

		// 리스트에 연결
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

	// 수신부
	curr = startframe;
	origcurr = originframe;
	line = 0;
	int totalErr = 0;
	int realErr;
	int foundErr;
	int incorrect = 0;
	int notFound = 0;
	int result = 0;

	while(curr->next) {
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
		else if(mode == CRC32) {
			strcpy(check, curr->load);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 24);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 16);
			check[strlen(check)] = (uint8_t)(curr->crcValue >> 8);
			check[strlen(check)] = (uint8_t)(curr->crcValue);
		}
		else {}

		if (mode == CRC1) {
			crcValue = calcCRC1(check);
			if (strcmp(origcurr->load, curr->load)) {
				realErr = 1;
				printf("R: Miss correct. [%d] --- ", line);
				totalErr++;
			}
		}
		else if (mode == CRC8) {
			crcValue = calcCRC8(check);
			if (strcmp(origcurr->load, curr->load)) {
				realErr = 1;
				printf("R: Miss correct. [%d] --- ", line);
				totalErr++;
			}
		}
		else if (mode == CRC16) {
			crcValue = calcCRC16(check);
			if (strcmp(origcurr->load, curr->load)) {
				realErr = 1;
				printf("R: Miss correct. [%d] --- ", line);
				totalErr++;
			}
		}
		else if (mode == CRC32) {
			crcValue = calcCRC32(check);
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
		if (realErr && foundErr) result++;
		if (!realErr && foundErr) incorrect++;
		if (realErr && !foundErr) notFound++;

		line++;
		curr = curr->next;
		origcurr = origcurr->next;
	}

	printf("Check error line : %d\n", totalErr - result);
	printf("Total error line : %d\n", totalErr);
	printf("잘못 찾은 갯수 : %d\n", incorrect);
	printf("못 찾은 갯수 : %d\n", notFound);
	printf("일치하는 갯수 : %d\n", result);

	fclose(fp);
	return 0;
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

			// 값을 넣어주기 위해 0, 1, 2번째 reset (1111_1000)
			// 계산된 값을 넣음
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

			// C값 계산.
			c0 = (((crcValue & 0x8000) >> 15) ^ input);				// c0 = c15 xor input
			c15 = c0 ^ ((crcValue & 0x4000) >> 14);					// c15 = c15 xor input xor C14
			c8 = c0 ^ ((crcValue & 0x0080) >> 7);					// c8 = c15 xor input xor c7

			crcCalc = c0 + (c8 << 8) + (c15 << 15);

			crcValue = crcValue << 1;
			// crcValue %= 65536;
			// 1 0000 0000 0000 0000
			crcValue %= 0x10000;

			// 0,8,15 째 bit를 저장해야함 ==> 0111_1110_1111_1110
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

			// C값 계산.
			c0 = (((crcValue & 0x80000000) >> 31) ^ input);	// C31 xor input ==> C0
			c26 = c0 ^ ((crcValue & 0x02000000) >> 25);			// C31 xor input xor C25 = C26
			c23 = c0 ^ ((crcValue & 0x00400000) >> 22);			// C31 xor input xor C22 = C23
			c22 = c0 ^ ((crcValue & 0x00200000) >> 21);			// C31 xor input xor C21 = C22
			c16 = c0 ^ ((crcValue & 0x00008000) >> 15);			// C31 xor input xor C15 = C16
			c12 = c0 ^ ((crcValue & 0x00000800) >> 11);			// C31 xor input xor C11 = C12
			c11 = c0 ^ ((crcValue & 0x00000400) >> 10);			// C31 xor input xor C10 = C11
			c10 = c0 ^ ((crcValue & 0x00000200) >> 9);			// C31 xor input xor C9  = C10
			c8 = c0 ^ ((crcValue & 0x00000080) >> 7);			// C31 xor input xor C7  = C8
			c7 = c0 ^ ((crcValue & 0x00000040) >> 6);			// C31 xor input xor C6  = C7
			c5 = c0 ^ ((crcValue & 0x00000010) >> 4);			// C31 xor input xor C4  = C5
			c4 = c0 ^ ((crcValue & 0x00000008) >> 3);			// C31 xor input xor C3  = C4
			c2 = c0 ^ ((crcValue & 0x00000002) >> 1);			// C31 xor input xor C1  = C2
			c1 = c0 ^ (crcValue & 0x00000001);					// C31 xor input xor C0  = C1

			crcCalc = c0 + c26 * 0x2000000 + c23 * 0x400000 + c22 * 0x200000 + c16 * 0x8000
				+ c12 * 0x800 + c11 * 0x400 + c10 * 0x200 + c8 * 0x80 + c7 * 0x40 
				+ c5 * 0x10 + c4 * 0x08 + c2 * 0x02 + c1 * 0x01;

			crcValue <<= 1;
			crcValue %= 0x100000000;

			// 0,1,2,4,5,7,8,10,11,12,16,22,23,26 저장해야함. ==>  1111_1011_0011_1110_1110_0010_0100_1000 / FB3EE248
			crcValue &= 0xFB3EE248;
			crcValue ^= crcCalc;
		}
	}

	return crcValue;
}

/*
	[과제]

	1.	long text file을 noisy channel에서 전송하는 것을 simulate
		CRC 알고리즘이 어떻게 작동하고 performance가 어떤지 확인.

	1)	sample.txt사용 ( 한 라인마다 CRC algorithm의 input으로 사용되는 frame을 만듬 )
	
	2)	H/W Logic으로 구현된 CRC Algorithm을 사용
		각 standard CRC algorithm에 따라서 CRC checksum values를 계산
	
	3)	파일이 noisy channel에서 CRC value로 전송이 되는데, 0.1의 bit_error값을 가짐
		CRC Algorithm을 적용해서 수신된 각 프레임이 오류가 발생하였는지 발생하지 않았는지를 결정
		수신측이 결정한 accuracy를 check하기 위해 받은 frame을 original frame과 비교하고 accuracy를 구하여라.
		이 accuracy로 부터 전송된 프레임의 오류를 감지할 수 없는 확률을 구할 수 있다.
	
	4)	이론적으로 감지가능한 오류가 존재하는 프레임의 확률을 유도하고,
		오류가 존재하지 않는 프레임의 확률을 유도하고,
		감지할 수 없는 오류가 존재하는 프레임의 확률을 유도.
		이 결과를 위에서 얻은 결과와 비교

	5)	이 과정을 반복. 프레임에서 에러가 일어날 확률을 0.001, 0.00001로 변경하여 실행
		( 전형적인 이더넷 케이블에서 에러가 일어날 확률 = 0.000000001 )

	6)	이 실험 결과에 대해 observation과 comments를 하시오.

	2.	CRC를 대신해 Internet checksum algorithm을 사용하여 위의 절차를 똑같이 실행하고
		CRC와 Internet checksum algorithm을 비교
*/

/*
	<< set of standard CRC algorithm >>

	CRC-1 : x + 1
	CRC-8-CCITT : x^8 + x^2 + x + 1
	CRC-16-CCITT : x^16 + x^15 + x^8 + 1
	CRC-32-IEEE : x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
*/

