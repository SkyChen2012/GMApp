#ifndef _JPEG_H_
#define _JPEG_H_

#include "MXTypes.h"

#define DCTSIZE	8
#define DCTBLOCKSIZE	64
#define DC_MAX_QUANTED  2047   //������DC�����ֵ
#define DC_MIN_QUANTED  -2048   //������DC����Сֵ

//Huffman��ṹ
typedef struct _HUFFCODE
{
	WORD code;  // huffman ����
	BYTE length;  // ���볤��
	WORD val;   // ���ֶ�Ӧ��ֵ
}HUFFCODE;

typedef struct _JPEGINFO
{
	// ���2��FDCT�任Ҫ���ʽ��������
	FLOAT YQT_DCT[DCTBLOCKSIZE];
	FLOAT UVQT_DCT[DCTBLOCKSIZE];
	// ���2��������
	BYTE YQT[DCTBLOCKSIZE]; 
	BYTE UVQT[DCTBLOCKSIZE]; 
	
	// ���VLI��
	BYTE VLI_TAB[4096];
	BYTE* pVLITAB;                        //VLI_TAB�ı���,ʹ�±���-2048-2048
	//���4��Huffman��
	HUFFCODE STD_DC_Y_HT[12];
	HUFFCODE STD_DC_UV_HT[12];
	HUFFCODE STD_AC_Y_HT[256];
	HUFFCODE STD_AC_UV_HT[256];
	BYTE bytenew; // The byte that will be written in the JPG file
	CHAR bytepos; //bit position in the byte we write (bytenew)
}JPEGINFO;

//�ļ���ʼ,��ʼ���Ϊ0xFFD8
const static WORD SOITAG = 0xD8FF;

//�ļ�����,�������Ϊ0xFFD9
const static WORD EOITAG = 0xD9FF;

//���� 8x8 Z�任��
const static BYTE FZBT[64] =
{
	0, 1, 5, 6, 14,15,27,28,
		2, 4, 7, 13,16,26,29,42,
		3, 8, 12,17,25,30,41,43,
		9, 11,18,24,31,40,44,53,
		10,19,23,32,39,45,52,54,
		20,22,33,38,46,51,55,60,
		21,34,37,47,50,56,59,61,
		35,36,48,49,57,58,62,63 
};

//��׼�����ź�����ģ��
const static BYTE std_Y_QT[64] = 
{
	16, 11, 10, 16, 24, 40, 51, 61,
		12, 12, 14, 19, 26, 58, 60, 55,
		14, 13, 16, 24, 40, 57, 69, 56,
		14, 17, 22, 29, 51, 87, 80, 62,
		18, 22, 37, 56, 68, 109,103,77,
		24, 35, 55, 64, 81, 104,113,92,
		49, 64, 78, 87, 103,121,120,101,
		72, 92, 95, 98, 112,100,103,99
};

//��׼ɫ���ź�����ģ��
const static BYTE std_UV_QT[64] = 
{
	17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99 ,99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99
};

static const DOUBLE aanScaleFactor[8] = {1.0, 1.387039845, 1.306562965, 1.175875602,1.0, 0.785694958, 0.541196100, 0.275899379};
// ��׼Huffman�� (cf. JPEG standard section K.3) 
static BYTE STD_DC_Y_NRCODES[17]={0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
static BYTE STD_DC_Y_VALUES[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
static BYTE STD_DC_UV_NRCODES[17]={0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
static BYTE STD_DC_UV_VALUES[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

static BYTE STD_AC_Y_NRCODES[17]={0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0X7D };
static BYTE STD_AC_Y_VALUES[162]= {
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
		0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
		0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
		0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
		0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
		0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
		0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
		0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
		0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
		0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
		0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
		0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
		0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
		0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
		0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
		0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
		0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
		0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
		0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0xf9, 0xfa };
	
static BYTE STD_AC_UV_NRCODES[17]={0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0X77};
static BYTE STD_AC_UV_VALUES[162]={
		0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
			0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
			0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
			0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
			0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
			0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
			0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
			0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
			0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
			0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
			0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
			0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
			0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
			0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
			0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
			0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
			0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
			0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
			0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
			0xf9, 0xfa };  

static USHORT mask[16]={1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768};

//JFIF APP0�νṹ
#pragma pack(push,1)
typedef struct _JPEGAPP0
{
	WORD segmentTag;  //APP0�α�ǣ�����ΪFFE0
	WORD length;    //�γ��ȣ�һ��Ϊ16�����û������ͼ
	CHAR id[5];     //�ļ���� "JFIF" + "\0"
	WORD ver;      //�ļ��汾��һ��Ϊ0101��0102
	BYTE densityUnit; //�ܶȵ�λ��0=�޵�λ 1=����/Ӣ�� 2=����/����
	WORD densityX;   //X�᷽���ܶ�,ͨ��д1
	WORD densityY;   //Y�᷽���ܶ�,ͨ��д1
	BYTE thp;     //����ͼˮƽ������,д0
	BYTE tvp;     //����ͼ��ֱ������,д0
}JPEGAPP0;// = {0xE0FF,16,'J','F','I','F',0,0x0101,0,1,1,0,0};
#pragma pack(pop)

//JFIF DQT�νṹ(8 bits ������)
#pragma pack(push,1)
typedef struct _JPEGDQT_8BITS
{
	WORD segmentTag;  //DQT�α�ǣ�����Ϊ0xFFDB
	WORD length;    //�γ���,������0x4300
	BYTE tableInfo;  //��������Ϣ
	BYTE table[64];  //������(8 bits)
}JPEGDQT_8BITS;
#pragma pack(pop)

		//JFIF SOF0�νṹ(���)�����໹��SOF1-SOFF
#pragma pack(push,1)
typedef struct _JPEGSOF0_24BITS
{
	WORD segmentTag;  //SOF�α�ǣ�����Ϊ0xFFC0
	WORD length;    //�γ��ȣ����ͼΪ17���Ҷ�ͼΪ11
	BYTE precision;  //���ȣ�ÿ���źŷ������õ�λ��������ϵͳΪ0x08
	WORD height;    //ͼ��߶�
	WORD width;     //ͼ����
	BYTE sigNum;   //�ź����������JPEGӦ��Ϊ3���Ҷ�Ϊ1
	BYTE YID;     //�źű�ţ�����Y
	BYTE HVY;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
	BYTE QTY;     //��Ӧ�������
	BYTE UID;     //�źű�ţ�ɫ��U
	BYTE HVU;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
	BYTE QTU;     //��Ӧ�������
	BYTE VID;     //�źű�ţ�ɫ��V
	BYTE HVV;     //������ʽ��0-3λ�Ǵ�ֱ������4-7λ��ˮƽ����
	BYTE QTV;     //��Ӧ�������
}JPEGSOF0_24BITS;// = {0xC0FF,0x0011,8,0,0,3,1,0x11,0,2,0x11,1,3,0x11,1};
#pragma pack(pop)

		//JFIF DHT�νṹ
#pragma pack(push,1)
typedef struct _JPEGDHT
{
	WORD segmentTag;  //DHT�α�ǣ�����Ϊ0xFFC4
	WORD length;    //�γ���
	BYTE tableInfo;  //����Ϣ������ϵͳ�� bit0-3 ΪHuffman���������bit4 Ϊ0ָDC��Huffman�� Ϊ1ָAC��Huffman��bit5-7����������Ϊ0
	BYTE huffCode[16];//1-16λ��Huffman���ֵ��������ֱ���������[1-16]��
			//BYTE* huffVal;  //���δ�Ÿ����ֶ�Ӧ��ֵ
}JPEGDHT;
#pragma pack(pop)

		// JFIF SOS�νṹ����ʣ�
#pragma pack(push,1)
typedef struct _JPEGSOS_24BITS
{
	WORD segmentTag;  //SOS�α�ǣ�����Ϊ0xFFDA
	WORD length;    //�γ��ȣ�������12
	BYTE sigNum;   //�źŷ����������ͼΪ0x03,�Ҷ�ͼΪ0x01
	BYTE YID;     //����Y�ź�ID,������1
	BYTE HTY;     //Huffman��ţ�bit0-3ΪDC�źŵı�bit4-7ΪAC�źŵı�
	BYTE UID;     //����Y�ź�ID,������2
	BYTE HTU;
	BYTE VID;     //����Y�ź�ID,������3
	BYTE HTV;
	BYTE Ss;     //����ϵͳ��Ϊ0
	BYTE Se;     //����ϵͳ��Ϊ63
	BYTE Bf;     //����ϵͳ��Ϊ0
}JPEGSOS_24BITS;// = {0xDAFF,0x000C,3,1,0,2,0x11,3,0x11,0,0x3F,0};
#pragma pack(pop)

//AC�ź��м���Žṹ
typedef struct _ACSYM
{
	BYTE zeroLen;  //0�г�
	BYTE codeLen;  //���ȱ��볤��
	SHORT amplitude;//���
}ACSYM;

//DC/AC �м����2�����ṹ
typedef struct _SYM2
{
	SHORT amplitude;//���
	BYTE codeLen;  //�������(��������ʽ��������ݵ�λ��)
}SYM2;

typedef struct _BMBUFINFO
{
    UINT imgWidth;
    UINT imgHeight;
	UINT buffWidth;
	UINT buffHeight;
    WORD BitCount;
    BYTE padSize;    
}BMBUFINFO;

void ProcessUV(PBYTE pUVBuf,PBYTE pTmpUVBuf,int width,int height,int nStride);  //��UVֵ��ɺ�Yһ���ĸ�ʽ
int QualityScaling(int quality); //У��Qualityֵ
void DivBuff(PBYTE pBuf,int width,int height,int nStride,int xLen,int yLen);
void SetQuantTable(const BYTE* std_QT,BYTE* QT, int Q);
void InitQTForAANDCT(JPEGINFO *pJpgInfo);
void BuildVLITable(JPEGINFO *pJpgInfo);
int WriteSOI(PBYTE pOut,int nDataLen);
int WriteEOI(PBYTE pOut,int nDataLen);
int WriteAPP0(PBYTE pOut,int nDataLen);
int WriteDQT(JPEGINFO *pJpgInfo,PBYTE pOut,int nDataLen);
int WriteSOF(PBYTE pOut,int nDataLen,int width,int height);
int WriteDHT(PBYTE pOut,int nDataLen);
int WriteSOS(PBYTE pOut,int nDataLen);
void BuildSTDHuffTab(BYTE* nrcodes,BYTE* stdTab,HUFFCODE* huffCode);
int ProcessData(JPEGINFO *pJpgInfo,BYTE* lpYBuf,BYTE* lpUBuf,BYTE* lpVBuf,int width,int height,PBYTE pOut,int nDataLen);

#endif
