#ifndef MOX_PIO_H
#define	MOX_PIO_H

#ifndef _GNUC_
	#define _GNUC_
#endif

#ifdef _GNUC_
	#ifndef	PACKED
		#define	PACKED	__attribute__((packed, aligned(1)))
	#endif	// PACKED
#else
	#define PACKED
#endif	// _GNUC_

#define	PIO_IOC_MAGIC				'p'
#define	PIO_IOC_OPEN				(_IO(PIO_IOC_MAGIC, 0))
#define	PIO_IOC_CLOSE				(_IO(PIO_IOC_MAGIC, 1))
#define	PIO_IOC_READ				(_IO(PIO_IOC_MAGIC, 2))
#define	PIO_IOC_WRITE				(_IO(PIO_IOC_MAGIC, 3))

#define	PIO_IOC_MAXNR				4

#define	MOX_PIO_MAJOR				242
#define	MOX_PIO_NAME				"mox_pio"

#define	MOX_PIOA					0
#define	MOX_PIOB					1
#define	MOX_PIOC					2
#define	MOX_PIOD					3
#define	MOX_PIOE					4
#define	MOX_PIOF					5

#define	MOX_PIO_PIN0				(1 << 0)
#define	MOX_PIO_PIN1				(1 << 1)
#define	MOX_PIO_PIN2				(1 << 2)
#define	MOX_PIO_PIN3				(1 << 3)
#define	MOX_PIO_PIN4				(1 << 4)
#define	MOX_PIO_PIN5				(1 << 5)
#define	MOX_PIO_PIN6				(1 << 6)
#define	MOX_PIO_PIN7				(1 << 7)
#define	MOX_PIO_PIN8				(1 << 8)
#define	MOX_PIO_PIN9				(1 << 9)
#define	MOX_PIO_PIN10				(1 << 10)
#define	MOX_PIO_PIN11				(1 << 11)
#define	MOX_PIO_PIN12				(1 << 12)
#define	MOX_PIO_PIN13				(1 << 13)
#define	MOX_PIO_PIN14				(1 << 14)
#define	MOX_PIO_PIN15				(1 << 15)
#define	MOX_PIO_PIN16				(1 << 16)
#define	MOX_PIO_PIN17				(1 << 17)
#define	MOX_PIO_PIN18				(1 << 18)
#define	MOX_PIO_PIN19				(1 << 19)
#define	MOX_PIO_PIN20				(1 << 20)
#define	MOX_PIO_PIN21				(1 << 21)
#define	MOX_PIO_PIN22				(1 << 22)
#define	MOX_PIO_PIN23				(1 << 23)
#define	MOX_PIO_PIN24				(1 << 24)
#define	MOX_PIO_PIN25				(1 << 25)
#define	MOX_PIO_PIN26				(1 << 26)
#define	MOX_PIO_PIN27				(1 << 27)
#define	MOX_PIO_PIN28				(1 << 28)
#define	MOX_PIO_PIN29				(1 << 29)
#define	MOX_PIO_PIN30				(1 << 30)
#define	MOX_PIO_PIN31				(1 << 31)

#define	MOX_PIO_FUN_INPUT			0
#define	MOX_PIO_FUN_OUTPUT			1

typedef	struct _PIOOpenParam
{
	unsigned int					nPin		PACKED;
	unsigned char					ucPort		PACKED;
	unsigned char					ucFun		PACKED;	
	unsigned char					ucValue		PACKED;
} PIOOpenParam;

typedef	struct _PIOCloseParam
{
	unsigned int					nPin		PACKED;
	unsigned char					ucPort		PACKED;
	unsigned char					ucFun		PACKED;	
	unsigned char					ucValue		PACKED;
} PIOCloseParam;

typedef	struct _PIOReadParam
{
	unsigned int					nPin		PACKED;
	unsigned char					ucPort		PACKED;
	unsigned char					ucValue		PACKED;	//  0, 1
} PIOReadParam;

typedef	struct _PIOWriteParam
{
	unsigned int					nPin		PACKED;
	unsigned char					ucPort		PACKED;
	unsigned char					ucValue		PACKED;	//  0, 1
} PIOWriteParam;

#endif	// MOX_PIO_H
