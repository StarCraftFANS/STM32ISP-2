#ifndef __FIFO_H__
#define __FIFO_H__


#define FifoType    volatile unsigned short

#define FifoSize    (1024*20)

#define FifoFull    1
#define FifoEmpty   2
#define FifoNormal  3

typedef struct 	
{ 
	volatile unsigned short front;  //����ͷ
	volatile unsigned short rear;   //����β
	volatile unsigned short count;  //������
	FifoType Buf[FifoSize];		  	
}FIFO_ST;

extern FIFO_ST MyFifo;

extern void FifoInit(FIFO_ST *Queue);
extern unsigned char FifoIn(FIFO_ST *Queue,FifoType Idat);
extern unsigned char FifoOut(FIFO_ST *Queue,FifoType *Odat);

#endif
