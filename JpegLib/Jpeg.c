
#include <malloc.h>
#include <math.h>

#include "Jpeg.h"
#include "MXTypes.h"


/*��YUV 420 ת����YUV444 UV�岹*/
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
	int xBufs = width / xLen;             //X�᷽�����и�����
	int yBufs = height / yLen;            //Y�᷽�����и�����
	int tmpBufLen  = xBufs * xLen * yLen;           //������ʱ����������
	BYTE* tmpBuf  = (BYTE *)malloc(tmpBufLen);           //������ʱ����
	int i;               //��ʱ����
	int j;
	int k; 
	int n;
	int bufOffset  = 0;               //�иʼ��ƫ����
	
	for (i = 0; i < yBufs; ++i)               //ѭ��Y�����и�����
	{
		n = 0;                   //��λ��ʱ������ƫ����
		for (j = 0; j < xBufs; ++j)              //ѭ��X�����и�����  
		{   
			bufOffset = yLen * i * nStride + j * xLen;        //���㵥Ԫ�źſ������ƫ����  
			for (k = 0; k < yLen; ++k)             //ѭ���������
			{
				memcpy(tmpBuf + n,pBuf +bufOffset,xLen);        //����һ�е���ʱ����
				n += xLen;                //������ʱ������ƫ����
				bufOffset += nStride;              //�������뻺����ƫ����
			}
		}
		memcpy(pBuf +i * tmpBufLen,tmpBuf,tmpBufLen);         //������ʱ�������ݵ����뻺��
	} 
	free(tmpBuf);                 //ɾ����ʱ����
}

void SetQuantTable(const BYTE* std_QT,BYTE* QT, int Q)
{
	int tmpVal = 0;
	int	i;
	for (i = 0; i < DCTBLOCKSIZE; ++i)
	{
		tmpVal = (std_QT[i] * Q + 50L) / 100L;
		
		if (tmpVal < 1)                 //��ֵ��Χ�޶�
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

//Ϊfloat AA&N IDCT�㷨��ʼ��������
void InitQTForAANDCT(JPEGINFO *pJpgInfo)
{
	UINT i = 0;           //��ʱ����
	UINT j = 0;
	UINT k = 0; 
	
	for (i = 0; i < DCTSIZE; i++)  //��ʼ�������ź�������
	{
		for (j = 0; j < DCTSIZE; j++)
		{
			pJpgInfo->YQT_DCT[k] = (FLOAT) (1.0 / ((DOUBLE) pJpgInfo->YQT[FZBT[k]] *
				aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
			++k;
		}
	} 
	
	k = 0;
	for (i = 0; i < DCTSIZE; i++)  //��ʼ��ɫ���ź�������
	{
		for (j = 0; j < DCTSIZE; j++)
		{
			pJpgInfo->UVQT_DCT[k] = (FLOAT) (1.0 / ((DOUBLE) pJpgInfo->UVQT[FZBT[k]] *
				aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
			++k;
		}
	} 
}

//���ط��ŵĳ���
BYTE ComputeVLI(SHORT val)
{ 
	BYTE binStrLen = 0;
	val = abs(val); 
	//��ȡ�������볤��   
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
// ��������:BuildVLITable 
//
// ����˵��:����VLI��
//
// ����˵��:
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

//д�ļ���ʼ���
int WriteSOI(PBYTE pOut,int nDataLen)
{ 
	memcpy(pOut+nDataLen,&SOITAG,sizeof(SOITAG));
	return nDataLen+sizeof(SOITAG);
}

//д���ļ��������
int WriteEOI(PBYTE pOut,int nDataLen)
{
	memcpy(pOut+nDataLen,&EOITAG,sizeof(EOITAG));
	return nDataLen + sizeof(EOITAG);
}

//дAPP0��
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

//д��DQT��
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


// ����8λ�͵�8λ����
USHORT Intel2Moto(USHORT val)
{
	BYTE highBits = (BYTE)(val / 256);
	BYTE lowBits = (BYTE)(val % 256);
	
	return lowBits * 256 + highBits;
}

//д��SOF��
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

//д1�ֽڵ��ļ�
int WriteByte(BYTE val,PBYTE pOut,int nDataLen)
{   
	pOut[nDataLen] = val;
	return nDataLen + 1;
}

//д��DHT��
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

//д��SOS��
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

// ���ɱ�׼Huffman��
void BuildSTDHuffTab(BYTE* nrcodes,BYTE* stdTab,HUFFCODE* huffCode)
{
	BYTE i     = 0;             //��ʱ����
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

// 8x8�ĸ�����ɢ���ұ任
void FDCT(FLOAT* lpBuff)
{
	FLOAT tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	FLOAT tmp10, tmp11, tmp12, tmp13;
	FLOAT z1, z2, z3, z4, z5, z11, z13;
	FLOAT* dataptr;
	int ctr;
	
	/* ��һ���֣����н��м��� */
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
		
		/* ��ż����������� */    
		tmp10 = tmp0 + tmp3; /* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		dataptr[0] = tmp10 + tmp11; /* phase 3 */
		dataptr[4] = tmp10 - tmp11;
		
		z1 = (FLOAT)((tmp12 + tmp13) * (0.707106781)); /* c4 */
		dataptr[2] = tmp13 + z1; /* phase 5 */
		dataptr[6] = tmp13 - z1;
		
		/* ����������м��� */
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
		
		dataptr += DCTSIZE; /* ��ָ��ָ����һ�� */
	}
	
	/* �ڶ����֣����н��м��� */
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
		
		/* ��ż����������� */    
		tmp10 = tmp0 + tmp3; /* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		dataptr[DCTSIZE*0] = tmp10 + tmp11; /* phase 3 */
		dataptr[DCTSIZE*4] = tmp10 - tmp11;
		
		z1 = (FLOAT)((tmp12 + tmp13) * (0.707106781)); /* c4 */
		dataptr[DCTSIZE*2] = tmp13 + z1; /* phase 5 */
		dataptr[DCTSIZE*6] = tmp13 - z1;
		
		/* ����������м��� */
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
		
		++dataptr;   /* ��ָ��ָ����һ�� */
	}
}

//********************************************************************
// ��������:WriteBitsStream 
//
// ����˵��:д���������
//
// ����˵��:
// value:��Ҫд���ֵ
// codeLen:�����Ƴ���
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
// ��������:WriteBits 
//
// ����˵��:д���������
//
// ����˵��:
// value:AC/DC�źŵ����
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
// ��������:BuildSym2 
//
// ����˵��:���źŵ����VLI����,���ر��볤�Ⱥ��ź�����ķ���
//
// ����˵��:
// value:AC/DC�źŵ����
//********************************************************************
SYM2 BuildSym2(SHORT value)
{
	SYM2 Symbol;  
	
	Symbol.codeLen = ComputeVLI(value);              //��ȡ���볤��
	Symbol.amplitude = 0;
	if (value >= 0)
	{
		Symbol.amplitude = value;
	}
	else
	{
		Symbol.amplitude = (SHORT)(pow(2,Symbol.codeLen)-1) + value;  //���㷴��
	}
	
	return Symbol;
}

//********************************************************************
// ��������:RLEComp 
//
// ����˵��:ʹ��RLE�㷨��ACѹ��,������������1,0,0,0,3,0,5 
//     ���Ϊ(0,1)(3,3)(1,5),��λ��ʾ��λ����ǰ0�ĸ���
//          ��λ��4bits��ʾ,0�ĸ���������ʾ��Χ�����Ϊ(15,0)
//          �����0��������һ�������б�ʾ.
//
// ����˵��:
// lpbuf:���뻺��,8x8�任�źŻ���
// lpOutBuf:�������,�ṹ����,�ṹ��Ϣ��ͷ�ļ�
// resultLen:������峤��,���������ŵ�����
//********************************************************************
void RLEComp(SHORT* lpbuf,ACSYM* lpOutBuf,BYTE *resultLen)
{  
	BYTE zeroNum     = 0;       //0�г̼�����
	UINT EOBPos      = 0;       //EOB����λ�� 
	const BYTE MAXZEROLEN = 15;          //���0�г�
	UINT i        = 0;      //��ʱ����
	UINT j        = 0;
	
	EOBPos = DCTBLOCKSIZE - 1;          //������ʼλ�ã������һ���źſ�ʼ
	for (i = EOBPos; i > 0; i--)         //������AC�ź���0�ĸ���
	{
		if (lpbuf[i] == 0)           //�ж������Ƿ�Ϊ0
		{
			--EOBPos;            //��ǰһλ
		}
		else              //������0������
		{
			break;                   
		}
	}
	
	for (i = 1; i <= EOBPos; i++)         //�ӵڶ����źţ���AC�źſ�ʼ����
	{
		if (lpbuf[i] == 0 && zeroNum < MAXZEROLEN)     //����ź�Ϊ0����������С��15
		{
			++zeroNum;   
		}
		else
		{   
			lpOutBuf[j].zeroLen = zeroNum;       //0�г̣��������ȣ�
			lpOutBuf[j].codeLen = ComputeVLI(lpbuf[i]);    //���ȱ��볤��
			lpOutBuf[j].amplitude = lpbuf[i];      //���      
			zeroNum = 0;           //0��������λ
			++(*resultLen);           //��������++
			++j;             //���ż���
		}
	} 
}

// ����DU(���ݵ�Ԫ)
int ProcessDU(JPEGINFO *pJpgInfo,FLOAT* lpBuf,FLOAT* quantTab,HUFFCODE* dcHuffTab,HUFFCODE* acHuffTab,SHORT* DC,PBYTE pOut,int nDataLen)
{
	BYTE i    = 0;              //��ʱ����
	UINT j    = 0;
	SHORT diffVal = 0;                //DC����ֵ  
	BYTE acLen  = 0;               //�ر����AC�м���ŵ�����
	SHORT sigBuf[DCTBLOCKSIZE];              //�������źŻ���
	ACSYM acSym[DCTBLOCKSIZE];              //AC�м���Ż��� 
	
	FDCT(lpBuf);                 //��ɢ���ұ任
	
	for (i = 0; i < DCTBLOCKSIZE; i++)            //��������
	{          
		sigBuf[FZBT[i]] = (short)((lpBuf[i] * quantTab[i] + 16384.5) - 16384);
	}
	//-----------------------------------------------------
	//��DC�źű��룬д���ļ�
	//DPCM���� 
	diffVal = sigBuf[0] - *DC;
	*DC = sigBuf[0];
	//����Huffman��д����Ӧ������
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
	//��AC�źű��벢д���ļ�
	for (i = 63; (i > 0) && (sigBuf[i] == 0); i--) //�ж�ac�ź��Ƿ�ȫΪ0
	{
		//ע�⣬��ѭ��
	}
	if (i == 0)                //���ȫΪ0
	{
		nDataLen = WriteBits(pJpgInfo,acHuffTab[0x00],pOut,nDataLen);           //д���������  
	}
	else
	{ 
		RLEComp(sigBuf,&acSym[0],&acLen);         //��AC���г��ȱ��� 
		for (j = 0; j < acLen; j++)           //���ζ�AC�м����Huffman����
		{   
			if (acSym[j].codeLen == 0)          //�Ƿ�������16��0
			{   
				nDataLen = WriteBits(pJpgInfo,acHuffTab[0xF0],pOut,nDataLen);         //д��(15,0)    
			}
			else
			{
				nDataLen = WriteBits(pJpgInfo,acHuffTab[acSym[j].zeroLen * 16 + acSym[j].codeLen],pOut,nDataLen); //
				nDataLen = WriteBits2(pJpgInfo,BuildSym2(acSym[j].amplitude),pOut,nDataLen);    
			}   
		}
		if (i != 63)              //������λ��0������д��EOB
		{
			nDataLen = WriteBits(pJpgInfo,acHuffTab[0x00],pOut,nDataLen);
		}
	}
	return nDataLen;
}

//********************************************************************
// ��������:ProcessData 
//
// ����˵��:����ͼ������FDCT-QUANT-HUFFMAN
//
// ����˵��:
// lpYBuf:����Y�ź����뻺��
// lpUBuf:ɫ��U�ź����뻺��
// lpVBuf:ɫ��V�ź����뻺��
//********************************************************************
int ProcessData(JPEGINFO *pJpgInfo,BYTE* lpYBuf,BYTE* lpUBuf,BYTE* lpVBuf,int width,int height,PBYTE pOut,int nDataLen)
{ 
	size_t yBufLen = 0;           //����Y���峤��
	size_t uBufLen = 0;           //ɫ��U���峤��          
	size_t vBufLen = 0;           //ɫ��V���峤��
	FLOAT dctYBuf[DCTBLOCKSIZE];            //Y�ź�FDCT������ʱ����
	FLOAT dctUBuf[DCTBLOCKSIZE];            //U�ź�FDCT������ʱ���� 
	FLOAT dctVBuf[DCTBLOCKSIZE];            //V�ź�FDCT������ʱ���� 
	UINT mcuNum   = 0;             //���MCU������ 
	SHORT yDC   = 0;             //Y�źŵĵ�ǰ���DC
	SHORT uDC   = 0;             //U�źŵĵ�ǰ���DC
	SHORT vDC   = 0;             //V�źŵĵ�ǰ���DC 
	BYTE yCounter  = 0;             //YUV�źŸ��Ե�д�������
	BYTE uCounter  = 0;
	BYTE vCounter  = 0;
	UINT i    = 0;             //��ʱ����              
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
		/ (DCTBLOCKSIZE * 3);         //����MCU������
	
	for (p = 0;p < mcuNum; p++)        //��������MCU��д��
	{
		yCounter = 1;//MCUIndex[SamplingType][0];   //��������ʽ��ʼ�����źż�����
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

