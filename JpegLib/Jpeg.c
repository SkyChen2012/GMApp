
#include <malloc.h>
#include <math.h>

#include "Jpeg.h"
#include "MXTypes.h"


/*将YUV 420 转换成YUV444 UV插补*/
void ProcessUV(PBYTE pUVBuf,PBYTE pTmpUVBuf,int width,int height,int nStride)
{
	int nTmpUV = height * nStride / 4;
	int i,j;
	for(j = 0 ; j < height ; j++)
	{
		for(i = 0 ; i < width ; i++)
		{
			pUVBuf[j* nStride + i] = *(pTmpUVBuf + (j / 2) * (nStride / 2) + i / 2);
		}
	}
}

int QualityScaling(int quality)
/* Convert a user-specified quality rating to a percentage scaling factor
 * for an underlying quantization table, using our recommended scaling curve.
 * The input 'quality' factor should be 0 (terrible) to 100 (very good).
 */
{
  /* Safety limit on quality factor.  Convert 0 to 1 to avoid zero divide. */
  if (quality <= 0) quality = 1;
  if (quality > 100) quality = 100;

  /* The basic table is used as-is (scaling 100) for a quality of 50.
   * Qualities 50..100 are converted to scaling percentage 200 - 2*Q;
   * note that at Q=100 the scaling is 0, which will cause jpeg_add_quant_table
   * to make all the table entries 1 (hence, minimum quantization loss).
   * Qualities 1..50 are converted to scaling percentage 5000/Q.
   */
  if (quality < 50)
    quality = 5000 / quality;
  else
    quality = 200 - quality*2;

  return quality;
}


void DivBuff(PBYTE pBuf,int width,int height,int nStride,int xLen,int yLen)
{
	int xBufs = width / xLen;             //X轴方向上切割数量
	int yBufs = height / yLen;            //Y轴方向上切割数量
	int tmpBufLen  = xBufs * xLen * yLen;           //计算临时缓冲区长度
	BYTE* tmpBuf  = (BYTE *)malloc(tmpBufLen);           //创建临时缓冲
	int i;               //临时变量
	int j;
	int k; 
	int n;
	int bufOffset  = 0;               //切割开始的偏移量
	
	for (i = 0; i < yBufs; ++i)               //循环Y方向切割数量
	{
		n = 0;                   //复位临时缓冲区偏移量
		for (j = 0; j < xBufs; ++j)              //循环X方向切割数量  
		{   
			bufOffset = yLen * i * nStride + j * xLen;        //计算单元信号块的首行偏移量  
			for (k = 0; k < yLen; ++k)             //循环块的行数
			{
				memcpy(tmpBuf + n,pBuf +bufOffset,xLen);        //复制一行到临时缓冲
				n += xLen;                //计算临时缓冲区偏移量
				bufOffset += nStride;              //计算输入缓冲区偏移量
			}
		}
		memcpy(pBuf +i * tmpBufLen,tmpBuf,tmpBufLen);         //复制临时缓冲数据到输入缓冲
	} 
	free(tmpBuf);                 //删除临时缓冲
}

void SetQuantTable(const BYTE* std_QT,BYTE* QT, int Q)
{
	int tmpVal = 0;
	int	i;
	for (i = 0; i < DCTBLOCKSIZE; ++i)
	{
		tmpVal = (std_QT[i] * Q + 50L) / 100L;
		
		if (tmpVal < 1)                 //数值范围限定
		{
			tmpVal = 1L;
		}
		if (tmpVal > 255)
		{
			tmpVal = 255L;
		}
		QT[FZBT[i]] = (BYTE)tmpVal;
	} 
}

//为float AA&N IDCT算法初始化量化表
void InitQTForAANDCT(JPEGINFO *pJpgInfo)
{
	UINT i = 0;           //临时变量
	UINT j = 0;
	UINT k = 0; 
	
	for (i = 0; i < DCTSIZE; i++)  //初始化亮度信号量化表
	{
		for (j = 0; j < DCTSIZE; j++)
		{
			pJpgInfo->YQT_DCT[k] = (FLOAT) (1.0 / ((DOUBLE) pJpgInfo->YQT[FZBT[k]] *
				aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
			++k;
		}
	} 
	
	k = 0;
	for (i = 0; i < DCTSIZE; i++)  //初始化色差信号量化表
	{
		for (j = 0; j < DCTSIZE; j++)
		{
			pJpgInfo->UVQT_DCT[k] = (FLOAT) (1.0 / ((DOUBLE) pJpgInfo->UVQT[FZBT[k]] *
				aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
			++k;
		}
	} 
}

//返回符号的长度
BYTE ComputeVLI(SHORT val)
{ 
	BYTE binStrLen = 0;
	val = abs(val); 
	//获取二进制码长度   
	if(val == 1)
	{
		binStrLen = 1;  
	}
	else if(val >= 2 && val <= 3)
	{
		binStrLen = 2;
	}
	else if(val >= 4 && val <= 7)
	{
		binStrLen = 3;
	}
	else if(val >= 8 && val <= 15)
	{
		binStrLen = 4;
	}
	else if(val >= 16 && val <= 31)
	{
		binStrLen = 5;
	}
	else if(val >= 32 && val <= 63)
	{
		binStrLen = 6;
	}
	else if(val >= 64 && val <= 127)
	{
		binStrLen = 7;
	}
	else if(val >= 128 && val <= 255)
	{
		binStrLen = 8;
	}
	else if(val >= 256 && val <= 511)
	{
		binStrLen = 9;
	}
	else if(val >= 512 && val <= 1023)
	{
		binStrLen = 10;
	}
	else if(val >= 1024 && val <= 2047)
	{
		binStrLen = 11;
	}
	
	return binStrLen;
}

//********************************************************************
// 方法名称:BuildVLITable 
//
// 方法说明:生成VLI表
//
// 参数说明:
//********************************************************************
void BuildVLITable(JPEGINFO *pJpgInfo)
{
	SHORT i   = 0;
	
	for (i = 0; i < DC_MAX_QUANTED; ++i)
	{
		pJpgInfo->pVLITAB[i] = ComputeVLI(i);
	}
	
	for (i = DC_MIN_QUANTED; i < 0; ++i)
	{
		pJpgInfo->pVLITAB[i] = ComputeVLI(i);
	}
}

//写文件开始标记
int WriteSOI(PBYTE pOut,int nDataLen)
{ 
	memcpy(pOut+nDataLen,&SOITAG,sizeof(SOITAG));
	return nDataLen+sizeof(SOITAG);
}

//写入文件结束标记
int WriteEOI(PBYTE pOut,int nDataLen)
{
	memcpy(pOut+nDataLen,&EOITAG,sizeof(EOITAG));
	return nDataLen + sizeof(EOITAG);
}

//写APP0段
int WriteAPP0(PBYTE pOut,int nDataLen)
{
	WORD temp1		= 0;
	CHAR  temp2		 = 0;
	BYTE   temp3	  = 0; 
	int			tempLen  = 0;
	JPEGAPP0 APP0;
	
	tempLen = nDataLen;
	temp1 = 0xE0FF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp1 = 0x1000;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp2 = 'J';
	memcpy(pOut+tempLen,&temp2,sizeof(CHAR));
	tempLen += sizeof(CHAR);
	
	temp2 = 'F';
	memcpy(pOut+tempLen,&temp2,sizeof(CHAR));
	tempLen += sizeof(CHAR);
	
	temp2 = 'I';
	memcpy(pOut+tempLen,&temp2,sizeof(CHAR));
	tempLen += sizeof(CHAR);
	
	temp2 = 'F';
	memcpy(pOut+tempLen,&temp2,sizeof(CHAR));
	tempLen += sizeof(CHAR);
	
	temp2 = 0;
	memcpy(pOut+tempLen,&temp2,sizeof(CHAR));
	tempLen += sizeof(CHAR);
	
	temp1 = 0x0101;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp3 = 0x00;
	memcpy(pOut+tempLen,&temp3,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	temp1 = 0x0100;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp1 = 0x0100;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp3 = 0x00;
	memcpy(pOut+tempLen,&temp3,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	temp3 = 0x00;
	memcpy(pOut+tempLen,&temp3,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	return tempLen;
}

//写入DQT段
int WriteDQT(JPEGINFO *pJpgInfo,PBYTE pOut,int nDataLen)
{
	UINT i = 0;
	JPEGDQT_8BITS DQT_Y;
	WORD temp1 = 0;
	BYTE temp2 = 0;
	int			tempLen  = 0;
	
	tempLen = nDataLen;
	
	temp1 = 0xDBFF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp1 = 0x4300;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);	
	
	temp2 = 0x00;
	memcpy(pOut+tempLen, &temp2, sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	for (i = 0; i < DCTBLOCKSIZE; i++)
	{
		temp2 = pJpgInfo->YQT[i];
		memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
		tempLen += sizeof(BYTE);	
	}    
	
	temp1 = 0xDBFF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp1 = 0x4300;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);	
	
	temp2 = 0x01;
	memcpy(pOut+tempLen, &temp2, sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	for (i = 0; i < DCTBLOCKSIZE; i++)
	{
		temp2 = pJpgInfo->UVQT[i];
		memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
		tempLen += sizeof(BYTE);
	}
	
	return tempLen;
}


// 将高8位和低8位交换
USHORT Intel2Moto(USHORT val)
{
	BYTE highBits = (BYTE)(val / 256);
	BYTE lowBits = (BYTE)(val % 256);
	
	return lowBits * 256 + highBits;
}

//写入SOF段
int WriteSOF(PBYTE pOut,int nDataLen,int width,int height)
{
	WORD  temp1 = 0;
	BYTE	temp2 = 0;
	int			tempLen  = 0;	
	JPEGSOF0_24BITS SOF;
	
	tempLen = nDataLen;
	
	temp1 = 0xC0FF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);	
	
	temp1 = 0x1100;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);	
	
	temp2 = 0x08;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp1 = Intel2Moto((USHORT)height);
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);	
	
	temp1 = Intel2Moto((USHORT)width);
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);	
	
	temp2 = 0x03;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x01;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x11;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x00;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x02;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x11;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x01;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x03;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x11;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x01;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	return tempLen;
}

//写1字节到文件
int WriteByte(BYTE val,PBYTE pOut,int nDataLen)
{   
	pOut[nDataLen] = val;
	return nDataLen + 1;
}

//写入DHT段
int WriteDHT(PBYTE pOut,int nDataLen)
{
	UINT i = 0;
	WORD temp1 = 0;
	BYTE temp2 = 0;
	int tempLen = 0;
	
	JPEGDHT DHT;
	
	tempLen = nDataLen;
	
	temp1 = 0xC4FF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);

	temp1 = Intel2Moto(19 + 12);
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);

	temp2 = 0x00;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	

	for (i = 0; i < 16; i++)
	{
		temp2 = STD_DC_Y_NRCODES[i + 1];
		memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
		tempLen += sizeof(BYTE);
	} 

	for (i = 0; i <= 11; i++)
	{
		tempLen = WriteByte(STD_DC_Y_VALUES[i],pOut,tempLen);  
	}  
	//------------------------------------------------

	temp1 = 0xC4FF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp1 = Intel2Moto(19 + 12);
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);

	temp2 = 0x01;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);

	for (i = 0; i < 16; i++)
	{
		temp2 = STD_DC_UV_NRCODES[i + 1];
		memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
		tempLen += sizeof(BYTE);
	}

	for (i = 0; i <= 11; i++)
	{
		tempLen = WriteByte(STD_DC_UV_VALUES[i],pOut,tempLen);  
	} 
	//----------------------------------------------------

	temp1 = 0xC4FF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);

	temp1 = Intel2Moto(19 + 162);
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);

	temp2 = 0x10;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);

	for (i = 0; i < 16; i++)
	{
		temp2 = STD_AC_Y_NRCODES[i + 1];
		memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
		tempLen += sizeof(BYTE);
	}

	for (i = 0; i <= 161; i++)
	{
		tempLen = WriteByte(STD_AC_Y_VALUES[i],pOut,tempLen);  
	}  
	//-----------------------------------------------------

	temp1 = 0xC4FF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp1 = Intel2Moto(19 + 162);
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp2 = 0x11;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	for (i = 0; i < 16; i++)
	{
		temp2 = STD_AC_UV_NRCODES[i + 1];
		memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
		tempLen += sizeof(BYTE);
	}

	for (i = 0; i <= 161; i++)
	{
		tempLen = WriteByte(STD_AC_UV_VALUES[i],pOut,tempLen);  
	}
	return tempLen;
}

//写入SOS段
int WriteSOS(PBYTE pOut,int nDataLen)
{
	WORD temp1 = 0;
	BYTE	temp2 = 0;
	JPEGSOS_24BITS SOS;
	
	int			tempLen  = 0;	
	
	tempLen = nDataLen;
	
	temp1 = 0xDAFF;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);		
	
	temp1 = 0x0C00;
	memcpy(pOut+tempLen,&temp1,sizeof(WORD));
	tempLen += sizeof(WORD);
	
	temp2 = 0x03;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x01;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x00;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	temp2 = 0x02;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x11;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);
	
	temp2 = 0x03;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x11;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x00;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x3F;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	temp2 = 0x00;
	memcpy(pOut+tempLen,&temp2,sizeof(BYTE));
	tempLen += sizeof(BYTE);	
	
	return tempLen;
}

// 生成标准Huffman表
void BuildSTDHuffTab(BYTE* nrcodes,BYTE* stdTab,HUFFCODE* huffCode)
{
	BYTE i     = 0;             //临时变量
	BYTE j     = 0;
	BYTE k     = 0;
	USHORT code   = 0; 
	
	for (i = 1; i <= 16; i++)
	{ 
		for (j = 1; j <= nrcodes[i]; j++)
		{   
			huffCode[stdTab[k]].code = code;
			huffCode[stdTab[k]].length = i;
			++k;
			++code;
		}
		code*=2;
	} 
	
	for (i = 0; i < k; i++)
	{
		huffCode[i].val = stdTab[i];  
	}
}

// 8x8的浮点离散余弦变换
void FDCT(FLOAT* lpBuff)
{
	FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	FLOAT tmp10, tmp11, tmp12, tmp13;
	FLOAT z1, z2, z3, z4, z5, z11, z13;
	FLOAT* dataptr;
	int ctr;
	
	/* 第一部分，对行进行计算 */
	dataptr = lpBuff;
	for (ctr = DCTSIZE-1; ctr >= 0; ctr--)
	{
		tmp0 = dataptr[0] + dataptr[7];
		tmp7 = dataptr[0] - dataptr[7];
		tmp1 = dataptr[1] + dataptr[6];
		tmp6 = dataptr[1] - dataptr[6];
		tmp2 = dataptr[2] + dataptr[5];
		tmp5 = dataptr[2] - dataptr[5];
		tmp3 = dataptr[3] + dataptr[4];
		tmp4 = dataptr[3] - dataptr[4];
		
		/* 对偶数项进行运算 */    
		tmp10 = tmp0 + tmp3; /* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		dataptr[0] = tmp10 + tmp11; /* phase 3 */
		dataptr[4] = tmp10 - tmp11;
		
		z1 = (FLOAT)((tmp12 + tmp13) * (0.707106781)); /* c4 */
		dataptr[2] = tmp13 + z1; /* phase 5 */
		dataptr[6] = tmp13 - z1;
		
		/* 对奇数项进行计算 */
		tmp10 = tmp4 + tmp5; /* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;
		
		z5 = (FLOAT)((tmp10 - tmp12) * ( 0.382683433)); /* c6 */
		z2 = (FLOAT)((0.541196100) * tmp10 + z5); /* c2-c6 */
		z4 = (FLOAT)((1.306562965) * tmp12 + z5); /* c2+c6 */
		z3 = (FLOAT)(tmp11 * (0.707106781)); /* c4 */
		
		z11 = tmp7 + z3;  /* phase 5 */
		z13 = tmp7 - z3;
		
		dataptr[5] = z13 + z2; /* phase 6 */
		dataptr[3] = z13 - z2;
		dataptr[1] = z11 + z4;
		dataptr[7] = z11 - z4;
		
		dataptr += DCTSIZE; /* 将指针指向下一行 */
	}
	
	/* 第二部分，对列进行计算 */
	dataptr = lpBuff;
	for (ctr = DCTSIZE-1; ctr >= 0; ctr--)
	{
		tmp0 = dataptr[DCTSIZE*0] + dataptr[DCTSIZE*7];
		tmp7 = dataptr[DCTSIZE*0] - dataptr[DCTSIZE*7];
		tmp1 = dataptr[DCTSIZE*1] + dataptr[DCTSIZE*6];
		tmp6 = dataptr[DCTSIZE*1] - dataptr[DCTSIZE*6];
		tmp2 = dataptr[DCTSIZE*2] + dataptr[DCTSIZE*5];
		tmp5 = dataptr[DCTSIZE*2] - dataptr[DCTSIZE*5];
		tmp3 = dataptr[DCTSIZE*3] + dataptr[DCTSIZE*4];
		tmp4 = dataptr[DCTSIZE*3] - dataptr[DCTSIZE*4];
		
		/* 对偶数项进行运算 */    
		tmp10 = tmp0 + tmp3; /* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		dataptr[DCTSIZE*0] = tmp10 + tmp11; /* phase 3 */
		dataptr[DCTSIZE*4] = tmp10 - tmp11;
		
		z1 = (FLOAT)((tmp12 + tmp13) * (0.707106781)); /* c4 */
		dataptr[DCTSIZE*2] = tmp13 + z1; /* phase 5 */
		dataptr[DCTSIZE*6] = tmp13 - z1;
		
		/* 对奇数项进行计算 */
		tmp10 = tmp4 + tmp5; /* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;
		
		z5 = (FLOAT)((tmp10 - tmp12) * (0.382683433)); /* c6 */
		z2 = (FLOAT)((0.541196100) * tmp10 + z5); /* c2-c6 */
		z4 = (FLOAT)((1.306562965) * tmp12 + z5); /* c2+c6 */
		z3 = (FLOAT)(tmp11 * (0.707106781)); /* c4 */
		
		z11 = tmp7 + z3;  /* phase 5 */
		z13 = tmp7 - z3;
		
		dataptr[DCTSIZE*5] = z13 + z2; /* phase 6 */
		dataptr[DCTSIZE*3] = z13 - z2;
		dataptr[DCTSIZE*1] = z11 + z4;
		dataptr[DCTSIZE*7] = z11 - z4;
		
		++dataptr;   /* 将指针指向下一列 */
	}
}

//********************************************************************
// 方法名称:WriteBitsStream 
//
// 方法说明:写入二进制流
//
// 参数说明:
// value:需要写入的值
// codeLen:二进制长度
//********************************************************************
int WriteBitsStream(JPEGINFO *pJpgInfo,USHORT value,BYTE codeLen,PBYTE pOut,int nDataLen)
{ 
	CHAR posval;//bit position in the bitstring we read, should be<=15 and >=0 
	posval=codeLen-1;
	while (posval>=0)
	{
		if (value & mask[posval])
		{
			pJpgInfo->bytenew|=mask[pJpgInfo->bytepos];
		}
		posval--;
		pJpgInfo->bytepos--;
		if (pJpgInfo->bytepos<0) 
		{ 
			if (pJpgInfo->bytenew==0xFF)
			{
				nDataLen = WriteByte(0xFF,pOut,nDataLen);
				nDataLen = WriteByte(0,pOut,nDataLen);
			}
			else
			{
				nDataLen = WriteByte(pJpgInfo->bytenew,pOut,nDataLen);
			}
			pJpgInfo->bytepos=7;
			pJpgInfo->bytenew=0;
		}
	}
	return nDataLen;
}

//********************************************************************
// 方法名称:WriteBits 
//
// 方法说明:写入二进制流
//
// 参数说明:
// value:AC/DC信号的振幅
//********************************************************************
int WriteBits(JPEGINFO *pJpgInfo,HUFFCODE huffCode,PBYTE pOut,int nDataLen)
{  
	return WriteBitsStream(pJpgInfo,huffCode.code,huffCode.length,pOut,nDataLen); 
}

int WriteBits2(JPEGINFO *pJpgInfo,SYM2 sym,PBYTE pOut,int nDataLen)
{
	return WriteBitsStream(pJpgInfo,sym.amplitude,sym.codeLen,pOut,nDataLen); 
}

//********************************************************************
// 方法名称:BuildSym2 
//
// 方法说明:将信号的振幅VLI编码,返回编码长度和信号振幅的反码
//
// 参数说明:
// value:AC/DC信号的振幅
//********************************************************************
SYM2 BuildSym2(SHORT value)
{
	SYM2 Symbol;  
	
	Symbol.codeLen = ComputeVLI(value);              //获取编码长度
	Symbol.amplitude = 0;
	if (value >= 0)
	{
		Symbol.amplitude = value;
	}
	else
	{
		Symbol.amplitude = (SHORT)(pow(2,Symbol.codeLen)-1) + value;  //计算反码
	}
	
	return Symbol;
}

//********************************************************************
// 方法名称:RLEComp 
//
// 方法说明:使用RLE算法对AC压缩,假设输入数据1,0,0,0,3,0,5 
//     输出为(0,1)(3,3)(1,5),左位表示右位数据前0的个数
//          左位用4bits表示,0的个数超过表示范围则输出为(15,0)
//          其余的0数据在下一个符号中表示.
//
// 参数说明:
// lpbuf:输入缓冲,8x8变换信号缓冲
// lpOutBuf:输出缓冲,结构数组,结构信息见头文件
// resultLen:输出缓冲长度,即编码后符号的数量
//********************************************************************
void RLEComp(SHORT* lpbuf,ACSYM* lpOutBuf,BYTE *resultLen)
{  
	BYTE zeroNum     = 0;       //0行程计数器
	UINT EOBPos      = 0;       //EOB出现位置 
	const BYTE MAXZEROLEN = 15;          //最大0行程
	UINT i        = 0;      //临时变量
	UINT j        = 0;
	
	EOBPos = DCTBLOCKSIZE - 1;          //设置起始位置，从最后一个信号开始
	for (i = EOBPos; i > 0; i--)         //从最后的AC信号数0的个数
	{
		if (lpbuf[i] == 0)           //判断数据是否为0
		{
			--EOBPos;            //向前一位
		}
		else              //遇到非0，跳出
		{
			break;                   
		}
	}
	
	for (i = 1; i <= EOBPos; i++)         //从第二个信号，即AC信号开始编码
	{
		if (lpbuf[i] == 0 && zeroNum < MAXZEROLEN)     //如果信号为0并连续长度小于15
		{
			++zeroNum;   
		}
		else
		{   
			lpOutBuf[j].zeroLen = zeroNum;       //0行程（连续长度）
			lpOutBuf[j].codeLen = ComputeVLI(lpbuf[i]);    //幅度编码长度
			lpOutBuf[j].amplitude = lpbuf[i];      //振幅      
			zeroNum = 0;           //0计数器复位
			++(*resultLen);           //符号数量++
			++j;             //符号计数
		}
	} 
}

// 处理DU(数据单元)
int ProcessDU(JPEGINFO *pJpgInfo,FLOAT* lpBuf,FLOAT* quantTab,HUFFCODE* dcHuffTab,HUFFCODE* acHuffTab,SHORT* DC,PBYTE pOut,int nDataLen)
{
	BYTE i    = 0;              //临时变量
	UINT j    = 0;
	SHORT diffVal = 0;                //DC差异值  
	BYTE acLen  = 0;               //熵编码后AC中间符号的数量
	SHORT sigBuf[DCTBLOCKSIZE];              //量化后信号缓冲
	ACSYM acSym[DCTBLOCKSIZE];              //AC中间符号缓冲 
	
	FDCT(lpBuf);                 //离散余弦变换
	
	for (i = 0; i < DCTBLOCKSIZE; i++)            //量化操作
	{          
		sigBuf[FZBT[i]] = (short)((lpBuf[i] * quantTab[i] + 16384.5) - 16384);
	}
	//-----------------------------------------------------
	//对DC信号编码，写入文件
	//DPCM编码 
	diffVal = sigBuf[0] - *DC;
	*DC = sigBuf[0];
	//搜索Huffman表，写入相应的码字
	if (diffVal == 0)
	{  
		nDataLen = WriteBits(pJpgInfo,dcHuffTab[0],pOut,nDataLen);  
	}
	else
	{   
		nDataLen = WriteBits(pJpgInfo,dcHuffTab[pJpgInfo->pVLITAB[diffVal]],pOut,nDataLen);  
		nDataLen = WriteBits2(pJpgInfo,BuildSym2(diffVal),pOut,nDataLen);    
	}
	//-------------------------------------------------------
	//对AC信号编码并写入文件
	for (i = 63; (i > 0) && (sigBuf[i] == 0); i--) //判断ac信号是否全为0
	{
		//注意，空循环
	}
	if (i == 0)                //如果全为0
	{
		nDataLen = WriteBits(pJpgInfo,acHuffTab[0x00],pOut,nDataLen);           //写入块结束标记  
	}
	else
	{ 
		RLEComp(sigBuf,&acSym[0],&acLen);         //对AC运行长度编码 
		for (j = 0; j < acLen; j++)           //依次对AC中间符号Huffman编码
		{   
			if (acSym[j].codeLen == 0)          //是否有连续16个0
			{   
				nDataLen = WriteBits(pJpgInfo,acHuffTab[0xF0],pOut,nDataLen);         //写入(15,0)    
			}
			else
			{
				nDataLen = WriteBits(pJpgInfo,acHuffTab[acSym[j].zeroLen * 16 + acSym[j].codeLen],pOut,nDataLen); //
				nDataLen = WriteBits2(pJpgInfo,BuildSym2(acSym[j].amplitude),pOut,nDataLen);    
			}   
		}
		if (i != 63)              //如果最后位以0结束就写入EOB
		{
			nDataLen = WriteBits(pJpgInfo,acHuffTab[0x00],pOut,nDataLen);
		}
	}
	return nDataLen;
}

//********************************************************************
// 方法名称:ProcessData 
//
// 方法说明:处理图像数据FDCT-QUANT-HUFFMAN
//
// 参数说明:
// lpYBuf:亮度Y信号输入缓冲
// lpUBuf:色差U信号输入缓冲
// lpVBuf:色差V信号输入缓冲
//********************************************************************
int ProcessData(JPEGINFO *pJpgInfo,BYTE* lpYBuf,BYTE* lpUBuf,BYTE* lpVBuf,int width,int height,PBYTE pOut,int nDataLen)
{ 
	size_t yBufLen = 0;           //亮度Y缓冲长度
	size_t uBufLen = 0;           //色差U缓冲长度          
	size_t vBufLen = 0;           //色差V缓冲长度
	FLOAT dctYBuf[DCTBLOCKSIZE];            //Y信号FDCT编码临时缓冲
	FLOAT dctUBuf[DCTBLOCKSIZE];            //U信号FDCT编码临时缓冲 
	FLOAT dctVBuf[DCTBLOCKSIZE];            //V信号FDCT编码临时缓冲 
	UINT mcuNum   = 0;             //存放MCU的数量 
	SHORT yDC   = 0;             //Y信号的当前块的DC
	SHORT uDC   = 0;             //U信号的当前块的DC
	SHORT vDC   = 0;             //V信号的当前块的DC 
	BYTE yCounter  = 0;             //YUV信号各自的写入计数器
	BYTE uCounter  = 0;
	BYTE vCounter  = 0;
	UINT i    = 0;             //临时变量              
	UINT j    = 0;                 
	UINT k    = 0;
	UINT p    = 0;
	UINT m    = 0;
	UINT n    = 0;
	UINT s    = 0; 

	yBufLen = malloc_usable_size(lpYBuf);
	uBufLen = malloc_usable_size(lpUBuf);
	vBufLen = malloc_usable_size(lpVBuf);
	
	mcuNum = (height * width * 3)
		/ (DCTBLOCKSIZE * 3);         //计算MCU的数量
	
	for (p = 0;p < mcuNum; p++)        //依次生成MCU并写入
	{
		yCounter = 1;//MCUIndex[SamplingType][0];   //按采样方式初始化各信号计数器
		uCounter = 1;//MCUIndex[SamplingType][1];
		vCounter = 1;//MCUIndex[SamplingType][2];
		
		for (; i < yBufLen; i += DCTBLOCKSIZE)
		{
			for (j = 0; j < DCTBLOCKSIZE; j++)
			{
				dctYBuf[j] = (FLOAT)(lpYBuf[i + j] - 128);
			}   
			if (yCounter > 0)
			{    
				--yCounter;
				nDataLen = ProcessDU(pJpgInfo,dctYBuf,pJpgInfo->YQT_DCT,pJpgInfo->STD_DC_Y_HT,pJpgInfo->STD_AC_Y_HT,&yDC,pOut,nDataLen);
			}
			else
			{
				break;
			}
		}  
		//------------------------------------------------------------------  
		for (; m < uBufLen; m += DCTBLOCKSIZE)
		{
			for (n = 0; n < DCTBLOCKSIZE; n++)
			{
				dctUBuf[n] = (FLOAT)(lpUBuf[m + n] - 128);
			}    
			if (uCounter > 0)
			{    
				--uCounter;
				nDataLen = ProcessDU(pJpgInfo,dctUBuf,pJpgInfo->UVQT_DCT,pJpgInfo->STD_DC_UV_HT,pJpgInfo->STD_AC_UV_HT,&uDC,pOut,nDataLen);
			}
			else
			{
				break;
			}
		}  
		//-------------------------------------------------------------------  
		for (; s < vBufLen; s += DCTBLOCKSIZE)
		{
			for (k = 0; k < DCTBLOCKSIZE; k++)
			{
				dctVBuf[k] = (FLOAT)(lpVBuf[s + k] - 128);
			}
			if (vCounter > 0)
			{
				--vCounter;
				nDataLen = ProcessDU(pJpgInfo,dctVBuf,pJpgInfo->UVQT_DCT,pJpgInfo->STD_DC_UV_HT,pJpgInfo->STD_AC_UV_HT,&vDC,pOut,nDataLen);
			}
			else
			{
				break;
			}
		}
		if (0 == (p%100))
		{
			usleep(1*1000);
			WatchDog();
			
		}
		
	} 
	return nDataLen;
}

