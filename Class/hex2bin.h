#pragma once
 
#ifndef __HEX_H_
#define __HEX_H_
 
typedef struct{
 
int len; //bin�ļ���С
UINT startAddress; //ˢд����ʼ��ַ
BYTE *pContent;//ת���������
 
}HexToBinData;
 
typedef struct{
 
BYTE data[16];//����
BYTE len;//���ݳ���
UINT pos;//ƫ�Ƶ�ַ
BYTE type;//����
 
}HexLinData;
 
int ConvertHexToBin(const char *str,HexToBinData *pData);
int GetStrFrHex(CString &FileSrc,CString &File);
 
#endif
//���������ǽ�hex�ļ���strֱ��ת��Ϊbin�ļ��������洢��HexToBinData�ṹ���У����ɹ��򷵻�0��ʧ���򷵻ط�0.