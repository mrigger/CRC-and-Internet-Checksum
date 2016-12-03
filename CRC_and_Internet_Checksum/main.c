#pragma warning(disable: 4996)

#include <stdio.h>
#include <string.h>

#define M_SIZE 1024

typedef struct crcFrame* frameList;
typedef struct crcframe {

	frameList next;
} crcFrame;

typedef enum CRC_MODE { CRC1, CRC5, CRC8, CRC16, CRC32 } MODE;

// �Լ� ����
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

		fgets(M, M_SIZE, fp);		// �� ������ ���ڿ� �Է�
		M[strlen(M) - 1] = '\0';	// \n ����
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
			���� ���� �� frame�� crcValue���� ����Ͽ���
			�׷��� ���� �� �� �������� ��Ʈ ������ ������ �ְ�
			������ �ϸ� �ǳ�?
			�׸��� ��?
		*/
		
	}


	// ����׿� �ڵ�
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

	
	// 8��Ʈ�Է�
	for (j = 7; j >= 0; j--) {
		// �� ��ġ�� ��Ʈ�� input���� ����
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

		// ���� �� �ڸ� �� shift
		crcValue = crcValue << 1;
		crcValue = crcValue % 32;

		// ���� �־��ֱ� ���� 0, 2, 4��° reset (0_1010)
		// ���� �� ����
		crcValue &= 0x0A;
		crcValue ^= crcCalc;
	}

	// ���� 5��Ʈ �Է�
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

