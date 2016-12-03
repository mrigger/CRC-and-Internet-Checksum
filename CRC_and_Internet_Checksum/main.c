#pragma warning(disable: 4996)

#include <stdio.h>
#include <string.h>

#define M_SIZE 1024

typedef struct crcFrame* frameList;
typedef struct crcframe {

	frameList next;
} crcFrame;

typedef enum CRC_MODE { CRC1, CRC5, CRC8, CRC16, CRC32 } MODE;

// 함수 선언
unsigned long long calcCRC5(unsigned char M);
unsigned long long calcCRC8(unsigned char M[]);

int main() {
	unsigned char M[M_SIZE] = { 653 };
	unsigned long long crcValue = 0;
	unsigned long long crc[M_SIZE];
	MODE mode;
	int stringSize;
	int i;

	FILE *fp = fopen("sample.txt", "rb");
	
	mode = CRC5;

	while (1) {
		if (fp == EOF)
			break;

		fgets(M, M_SIZE, fp);		// 한 라인의 문자열 입력
		M[strlen(M) - 1] = '\0';	// \n 삭제
		stringSize = strlen(M) - 1;
		

		if (mode == CRC1) {

		}
		else if (mode == CRC5) {
			// (8 + 5) bit = 13
			for (i = 0; i < stringSize; i++) {
				crc[i] = calcCRC5(M[i]);
			}
		}
		else if (mode == CRC8) {
			for (i = 0; i < stringSize; i++) {
				crc[i] = calcCRC8(M[i]);
			}
		}
		else if (mode == CRC16) {

		}
		else {

		}

		/*
			지금 현재 한 frame의 crcValue값을 계산하였음
			그러면 이제 이 한 프레임의 비트 값에서 에러를 주고
			저장을 하면 되나?
			그리고 비교?
		*/
		
	}


	// 디버그용 코드
	// result = calcCRC5(653);
	// printf("%lld %lld\n", result);

	fclose(fp);
	return 0;
}

unsigned long long calcCRC5(unsigned char M) {
	int i, j;
	
	unsigned long long crcValue = 0;
	unsigned long long crcCalc = 0;
	unsigned long long output = 0;
	int input;
	int c0, c2, c4;

	
	// 8비트입력
	for (j = 7; j >= 0; j--) {
		// 각 위치의 비트를 input으로 받음
		if ((M & (1 << j)))
			input = 1;
		else
			input = 0;
		
		output = output << 1;
		output += input;

		c0 = (((crcValue & 0x10) >> 4) ^ input);	// c0 = c4 xor input
		c2 = c0 ^ ((crcValue & 0x02) >> 1);			// c2 = c4 xor input xor c1
		c4 = c0 ^ ((crcValue & 0x08) >> 3);			// c4 = c4 xor input xor c3

		crcCalc = c0 + (c2 << 2) + (c4 << 4);

		// 먼저 한 자리 씩 shift
		crcValue = crcValue << 1;
		crcValue = crcValue % 32;

		// 값을 넣어주기 위해 0, 2, 4번째 reset (0_1010)
		// 계산된 값 넣음
		crcValue &= 0x0A;
		crcValue ^= crcCalc;
	}

	// 남은 5비트 입력
	output = output << 5;
	output += crcValue;

	return output;
}

unsigned long long calcCRC8(unsigned char M[])
{
	unsigned long long crcValue = 0;
	unsigned long long crcCalc = 0;
	int input;
	int c0, c2, c1;
	int stringSize = strlen(M) - 1;

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

			// 값을 넣어주기 위해 0, 1, 2번째 reset (1111_1000)
			// 계산된 값을 넣음
			crcValue &= 0xF8;
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

