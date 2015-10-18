/************** SYSTEM INCLUDE FILES **************************************************/
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "MXTypes.h"
#include "Bmp.h"
#include "JpegApp.h"

#define	 BMP_DEBUG

BYTE	BmpPicBuf[BMP_TOTAL_SIZE];
INT		nBmpBufLen	=	0;


//使用整数运算(定点数运算)来代替浮点运算
const int csY_coeff_16 = 1.164383 * (1 << 16);
const int csU_blue_16 = 2.017232 * (1 << 16);
const int csU_green_16 = (-0.391762) * (1 << 16);
const int csV_green_16 = (-0.812968) * (1 << 16);
const int csV_red_16 = 1.596027 * (1 << 16);
//颜色查表
static BYTE _color_table[256 * 3];
static const BYTE* color_table = &_color_table[256];
//查表
static int Ym_tableEx[256];
static int Um_blue_tableEx[256];
static int Um_green_tableEx[256];
static int Vm_green_tableEx[256];
static int Vm_red_tableEx[256];
//颜色饱和函数
inline long border_color(long color) {
	if (color > 255)
		return 255;
	else if (color < 0)
		return 0;
	else
		return color;
}
//采用查找表进行计算时，必须运行的初始化函数
void YUV422P_To_RGB24_init() {
	int i;
	for (i = 0; i < 256 * 3; ++i)
		_color_table[i] = border_color(i - 256);
	for (i = 0; i < 256; ++i) {
		Ym_tableEx[i] = (csY_coeff_16 * (i - 16)) >> 16;
		Um_blue_tableEx[i] = (csU_blue_16 * (i - 128)) >> 16;
		Um_green_tableEx[i] = (csU_green_16 * (i - 128)) >> 16;
		Vm_green_tableEx[i] = (csV_green_16 * (i - 128)) >> 16;
		Vm_red_tableEx[i] = (csV_red_16 * (i - 128)) >> 16;
	}
}
inline void YUVToRGB24_Table(BYTE *p, const BYTE Y0, const BYTE Y1,
		const BYTE U, const BYTE V) {
	int Ye0 = Ym_tableEx[Y0];
	int Ye1 = Ym_tableEx[Y1];
	int Ue_blue = Um_blue_tableEx[U];
	int Ue_green = Um_green_tableEx[U];
	int Ve_green = Vm_green_tableEx[V];
	int Ve_red = Vm_red_tableEx[V];
	int UeVe_green = Ue_green + Ve_green;
	*p = color_table[(Ye0 + Ve_red)];
	*(p + 1) = color_table[(Ye0 + UeVe_green)];
	*(p + 2) = color_table[(Ye0 + Ue_blue)];
	*(p + 3) = color_table[(Ye1 + Ve_red)];
	*(p + 4) = color_table[(Ye1 + UeVe_green)];
	*(p + 5) = color_table[(Ye1 + Ue_blue)];
}
int YUV420P_To_RGB24(BYTE* pY, BYTE* pU, BYTE* pV, BYTE* DstPic, int width,
		int height) {
	int y, x, x_uv;
	BYTE* pDstLine = DstPic;
	if ((width % 2) != 0 || (height % 2) != 0)
		return (-1);
	for (y = 0; y < height; ++y) {
		//DECODE_PlanarYUV211_Common_line(pDstLine, pY, pU, pV,width);
		for (x = 0; x < width; x += 2) {
			x_uv = x >> 1;
			YUVToRGB24_Table(&pDstLine[x * 3], pY[x], pY[x + 1], pU[x_uv],
					pV[x_uv]);
		}
		pDstLine += width * 3; //RGB888
		pY += width; //YUV422
		if (y % 2 == 1) {
			pU += width / 2;
			pV += width / 2;
		}
	}
	return 0;
}


static void convertyuv422torgb565(unsigned char *inbuf,unsigned char *outbuf,int width,int height)
{
  int rows,cols,rowwidth;
  int y,u,v,r,g,b,rdif,invgdif,bdif;
  int size;
  unsigned char *YUVdata,*RGBdata;
  int YPOS,UPOS,VPOS;

  YUVdata = inbuf;
  RGBdata = outbuf;

  rowwidth = width>>1;
  size=width*height*2;
  YPOS=0;
  UPOS=YPOS + size/2;
  VPOS=UPOS + size/4;

  for(rows=0;rows<height;rows++)
  {
    for(cols=0;cols<width;cols++) 
    {
		 u = YUVdata[UPOS] - 128;
		 v = YUVdata[VPOS] - 128;

		 rdif = v + ((v * 103) >> 8);
		 invgdif = ((u * 88) >> 8) +((v * 183) >> 8);
		 bdif = u +( (u*198) >> 8);

		 r = YUVdata[YPOS] + rdif;
		 g = YUVdata[YPOS] - invgdif;
		 b = YUVdata[YPOS] + bdif;
		 r=r>255?255:(r<0?0:r);
		 g=g>255?255:(g<0?0:g);
		 b=b>255?255:(b<0?0:b);
		    
		 *(RGBdata++) =( ((g & 0x1C) << 3) | ( b >> 3) );
		 *(RGBdata++) =( (r & 0xF8) | ( g >> 5) );

		 YPOS++;      
		 
		 if(cols & 0x01)
		 {
		    UPOS++;
		    VPOS++;      
		 } 
    }
    if((rows & 0x01)== 0)
    {
		 UPOS -= rowwidth;
		 VPOS -= rowwidth;
    }
  }

}

void
ConvertYUV2Bmp()
{
	BMPHEAD	bmp;
	int	cx, cy, extra, bpp, bytespp, ncolors, sizecolortable;
	DWORD mask;
	BYTE *in_Y   =	NULL;
	BYTE *in_U   =	NULL;
	BYTE *in_V	  =	NULL;
	int step;
	int offset;
#ifdef BMP_DEBUG
	INT	 time1,time2;
	FILE*	fpBMP = NULL;
	time1=GetTickCount();
#endif
	bYUV2JPG	=	FALSE;

	in_Y   =	RawPicBuf;
	in_U   =	in_Y + PHOTO_WIDTH * PHOTO_HEIGHT;
	in_V   =	in_U + PHOTO_WIDTH * PHOTO_HEIGHT / 4;
	YUV422P_To_RGB24_init();
	YUV420P_To_RGB24(in_Y,in_U,in_V,BmpPicBuf+BMP_HEAD_SIZE,PHOTO_WIDTH,PHOTO_HEIGHT);

//	convertyuv422torgb565(RawPicBuf,BmpPicBuf+BMP_HEAD_SIZE,PHOTO_WIDTH,PHOTO_HEIGHT);
#ifdef BMP_DEBUG
	time2=GetTickCount();
	printf("%s  time=%d\n",__FUNCTION__,time2-time1);
#endif	
	
	sizecolortable= 0;
	cx=PHOTO_WIDTH;
	cy=PHOTO_HEIGHT;
	bpp=24;
	bytespp = (bpp+7)/8;

	/* dword right padded*/
	step = bytespp*cx;  
	offset = step%4;  
	if (offset != 0)
	{  
		step += (4-offset);  
	}  
	ncolors = (bpp <= 8)? (1<<bpp): 0;
	/* fill out bmp header*/
	memset(&bmp, 0, sizeof(bmp));
	bmp.bfType[0] = 'B';
	bmp.bfType[1] = 'M';
	bmp.bfSize = cy*step + 54;
	bmp.bfOffBits = 54;
	bmp.BiSize = 40;
	bmp.BiWidth = (cx);
	bmp.BiHeight = (cy);
	bmp.BiPlanes = (1);
	bmp.BiBitCount = (bpp);
	bmp.BiCompression = 0;
	bmp.BiSizeImage =cy*step;
	bmp.BiClrUsed = ((bpp <= 8)? ncolors: 0);
	memcpy(BmpPicBuf,&bmp,sizeof(BMPHEAD));
	bYUV2JPG	=	TRUE;
	nBmpBufLen = BMP_TOTAL_SIZE;
	/*bmp.BiClrImportant = 0;*/
#ifdef BMP_DEBUG
	fpBMP = fopen(BMP_PIC_NAME,"wb");
	if (NULL == fpBMP)
	{
		return;
	}
	fwrite(BmpPicBuf,nBmpBufLen,1,fpBMP);
	fclose(fpBMP);
#endif
}

/*
void
ConvertYUV2Bmp()
{
	BMPHEAD	bmp;
	int	cx, cy, extra, bpp, bytespp, ncolors, sizecolortable;
	DWORD mask;
#ifdef BMP_DEBUG
	INT	 time1,time2;
	FILE*	fpBMP = NULL;
	time1=GetTickCount();
#endif
	convertyuv422torgb565(RawPicBuf,BmpPicBuf+BMP_HEAD_SIZE,PHOTO_WIDTH,PHOTO_HEIGHT);
#ifdef BMP_DEBUG
	time2=GetTickCount();
	printf("%s  time=%d\n",__FUNCTION__,time2-time1);
#endif	
	nBmpBufLen=BMP_HEAD_SIZE+PHOTO_WIDTH*PHOTO_HEIGHT*2;
	sizecolortable= 0;
	cx=PHOTO_WIDTH;
	cy=PHOTO_HEIGHT;
	bpp=16;
	bytespp = (bpp+7)/8;

	// dword right padded
	extra = (cx*bytespp) & 3;
	if (extra)
		extra = 4 - extra;
	ncolors = (bpp <= 8)? (1<<bpp): 0;
	//fill out bmp header
	memset(&bmp, 0, sizeof(bmp));
	bmp.bfType[0] = 'B';
	bmp.bfType[1] = 'M';
	bmp.bfSize = (70 + (long)(cx+extra)*cy*bytespp);
	bmp.bfOffBits = 70;
	bmp.BiSize = (56);
	bmp.BiWidth = (cx);
	bmp.BiHeight = (cy);
	bmp.BiPlanes = (1);
	bmp.BiBitCount = (bpp);
	bmp.BiCompression = ((bpp==16 || bpp==32)? BI_BITFIELDS: BI_RGB);
	bmp.BiSizeImage = ((long)(cx+extra)*cy*bytespp);
	bmp.BiClrUsed = ((bpp <= 8)? ncolors: 0);
	memcpy(BmpPicBuf,&bmp,sizeof(BMPHEAD));
	mask=RMASK565;
	memcpy(BmpPicBuf+sizeof(BMPHEAD),&mask,4);
	mask=GMASK565;
	memcpy(BmpPicBuf+sizeof(BMPHEAD)+4,&mask,4);
	mask=BMASK565;
	memcpy(BmpPicBuf+sizeof(BMPHEAD)+8,&mask,4);
	//bmp.BiClrImportant = 0;
#ifdef BMP_DEBUG
	fpBMP = fopen(BMP_PIC_NAME,"wb");
	if (NULL == fpBMP)
	{
		return;
	}
	

	// write header
	fwrite(BmpPicBuf,nBmpBufLen,1,fpBMP);
	fclose(fpBMP);
#endif

}
*/

