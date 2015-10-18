#ifndef _JPEG_H_
#define _JPEG_H_

#include "MXTypes.h"

#define DCTSIZE	8
#define DCTBLOCKSIZE	64
#define DC_MAX_QUANTED  2047   //量化后DC的最大值
#define DC_MIN_QUANTED  -2048   //量化后DC的最小值

//Huffman码结构
typedef struct _HUFFCODE
{
	WORD code;  // huffman 码字
	BYTE length;  // 编码长度
	WORD val;   // 码字对应的值
}HUFFCODE;

typedef struct _JPEGINFO
{
	// 存放2个FDCT变换要求格式的量化表
	FLOAT YQT_DCT[DCTBLOCKSIZE];
	FLOAT UVQT_DCT[DCTBLOCKSIZE];
	// 存放2个量化表
	BYTE YQT[DCTBLOCKSIZE]; 
	BYTE UVQT[DCTBLOCKSIZE]; 
	
	// 存放VLI表
	BYTE VLI_TAB[4096];
	BYTE* pVLITAB;                        //VLI_TAB的别名,使下标在-2048-2048
	//存放4个Huffman表
	HUFFCODE STD_DC_Y_HT[12];
	HUFFCODE STD_DC_UV_HT[12];
	HUFFCODE STD_AC_Y_HT[256];
	HUFFCODE STD_AC_UV_HT[256];
	BYTE bytenew; // The byte that will be written in the JPG file
	CHAR bytepos; //bit position in the byte we write (bytenew)
}JPEGINFO;

//文件开始,开始标记为0xFFD8
const static WORD SOITAG = 0xD8FF;

//文件结束,结束标记为0xFFD9
const static WORD EOITAG = 0xD9FF;

//正向 8x8 Z变换表
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

//标准亮度信号量化模板
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

//标准色差信号量化模板
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
// 标准Huffman表 (cf. JPEG standard section K.3) 
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

//JFIF APP0段结构
#pragma pack(push,1)
typedef struct _JPEGAPP0
{
	WORD segmentTag;  //APP0段标记，必须为FFE0
	WORD length;    //段长度，一般为16，如果没有缩略图
	CHAR id[5];     //文件标记 "JFIF" + "\0"
	WORD ver;      //文件版本，一般为0101或0102
	BYTE densityUnit; //密度单位，0=无单位 1=点数/英寸 2=点数/厘米
	WORD densityX;   //X轴方向密度,通常写1
	WORD densityY;   //Y轴方向密度,通常写1
	BYTE thp;     //缩略图水平像素数,写0
	BYTE tvp;     //缩略图垂直像素数,写0
}JPEGAPP0;// = {0xE0FF,16,'J','F','I','F',0,0x0101,0,1,1,0,0};
#pragma pack(pop)

//JFIF DQT段结构(8 bits 量化表)
#pragma pack(push,1)
typedef struct _JPEGDQT_8BITS
{
	WORD segmentTag;  //DQT段标记，必须为0xFFDB
	WORD length;    //段长度,这里是0x4300
	BYTE tableInfo;  //量化表信息
	BYTE table[64];  //量化表(8 bits)
}JPEGDQT_8BITS;
#pragma pack(pop)

		//JFIF SOF0段结构(真彩)，其余还有SOF1-SOFF
#pragma pack(push,1)
typedef struct _JPEGSOF0_24BITS
{
	WORD segmentTag;  //SOF段标记，必须为0xFFC0
	WORD length;    //段长度，真彩图为17，灰度图为11
	BYTE precision;  //精度，每个信号分量所用的位数，基本系统为0x08
	WORD height;    //图像高度
	WORD width;     //图像宽度
	BYTE sigNum;   //信号数量，真彩JPEG应该为3，灰度为1
	BYTE YID;     //信号编号，亮度Y
	BYTE HVY;     //采样方式，0-3位是垂直采样，4-7位是水平采样
	BYTE QTY;     //对应量化表号
	BYTE UID;     //信号编号，色差U
	BYTE HVU;     //采样方式，0-3位是垂直采样，4-7位是水平采样
	BYTE QTU;     //对应量化表号
	BYTE VID;     //信号编号，色差V
	BYTE HVV;     //采样方式，0-3位是垂直采样，4-7位是水平采样
	BYTE QTV;     //对应量化表号
}JPEGSOF0_24BITS;// = {0xC0FF,0x0011,8,0,0,3,1,0x11,0,2,0x11,1,3,0x11,1};
#pragma pack(pop)

		//JFIF DHT段结构
#pragma pack(push,1)
typedef struct _JPEGDHT
{
	WORD segmentTag;  //DHT段标记，必须为0xFFC4
	WORD length;    //段长度
	BYTE tableInfo;  //表信息，基本系统中 bit0-3 为Huffman表的数量，bit4 为0指DC的Huffman表 为1指AC的Huffman表，bit5-7保留，必须为0
	BYTE huffCode[16];//1-16位的Huffman码字的数量，分别存放在数组[1-16]中
			//BYTE* huffVal;  //依次存放各码字对应的值
}JPEGDHT;
#pragma pack(pop)

		// JFIF SOS段结构（真彩）
#pragma pack(push,1)
typedef struct _JPEGSOS_24BITS
{
	WORD segmentTag;  //SOS段标记，必须为0xFFDA
	WORD length;    //段长度，这里是12
	BYTE sigNum;   //信号分量数，真彩图为0x03,灰度图为0x01
	BYTE YID;     //亮度Y信号ID,这里是1
	BYTE HTY;     //Huffman表号，bit0-3为DC信号的表，bit4-7为AC信号的表
	BYTE UID;     //亮度Y信号ID,这里是2
	BYTE HTU;
	BYTE VID;     //亮度Y信号ID,这里是3
	BYTE HTV;
	BYTE Ss;     //基本系统中为0
	BYTE Se;     //基本系统中为63
	BYTE Bf;     //基本系统中为0
}JPEGSOS_24BITS;// = {0xDAFF,0x000C,3,1,0,2,0x11,3,0x11,0,0x3F,0};
#pragma pack(pop)

//AC信号中间符号结构
typedef struct _ACSYM
{
	BYTE zeroLen;  //0行程
	BYTE codeLen;  //幅度编码长度
	SHORT amplitude;//振幅
}ACSYM;

//DC/AC 中间符号2描述结构
typedef struct _SYM2
{
	SHORT amplitude;//振幅
	BYTE codeLen;  //振幅长度(二进制形式的振幅数据的位数)
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

void ProcessUV(PBYTE pUVBuf,PBYTE pTmpUVBuf,int width,int height,int nStride);  //将UV值变成和Y一样的格式
int QualityScaling(int quality); //校正Quality值
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
